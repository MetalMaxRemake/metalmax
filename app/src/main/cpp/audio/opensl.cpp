//
// Created by parkyu on 2023/5/15.
//

#include "opensl.h"
#include "native_sound.h"
#include "../global.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <assert.h>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <atomic>

#define k_sample_rate 16000

// engine interfaces
static SLObjectItf engine_object = nullptr;
static SLEngineItf engine_engine;

// output mix interfaces
static SLObjectItf output_mix_object = nullptr;
static SLEnvironmentalReverbItf output_mix_environmental_reverb = nullptr;

// buffer queue player interfaces
static SLObjectItf bq_player_object = nullptr;
static SLPlayItf bq_player_play;
static SLAndroidSimpleBufferQueueItf bq_player_buffer_queue;
static SLEffectSendItf bq_player_effect_send;
static SLVolumeItf bq_player_volume;
static SLmilliHertz bq_player_sample_rate = 0;

static pthread_mutex_t audio_engine_lock = PTHREAD_MUTEX_INITIALIZER;

static const SLEnvironmentalReverbSettings reverb_settings =
        SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

std::atomic<bool> is_running(false);
static short output_buffers[2][1024];
static unsigned output_index = 0;

void bq_player_callback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    if (!is_running) {
        return;
    }
    assert(bq == bq_player_buffer_queue);
    assert(nullptr == context);
    SLresult result;
    short *source = get_audio_buffer();
    short *buffer = output_buffers[output_index];
    output_index ^= 1;
    if (source) { memcpy(buffer, source, 1024 * 2); } else { memset(buffer, 0, 1024 * 2); }
    if (!is_running) { return; }
    result = (*bq_player_buffer_queue)->Enqueue(bq_player_buffer_queue,
                                             buffer,
                                             1024 * 2);
    if (result != SL_RESULT_SUCCESS) { log_e("opensl", "Enqueue failed: %u", result); }
}

void create_engine() {
    SLresult result;

    // create engine
    result = slCreateEngine(&engine_object, 0, nullptr, 0, nullptr, nullptr);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // realize the engine
    result = (*engine_object)->Realize(engine_object, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the engine interface, which is needed in order to create other objects
    result = (*engine_object)->GetInterface(engine_object, SL_IID_ENGINE, &engine_engine);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engine_engine)->CreateOutputMix(engine_engine, &output_mix_object, 1, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // realize the output mix
    result = (*output_mix_object)->Realize(output_mix_object, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*output_mix_object)->GetInterface(output_mix_object, SL_IID_ENVIRONMENTALREVERB,
                                              &output_mix_environmental_reverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*output_mix_environmental_reverb)->SetEnvironmentalReverbProperties(
                output_mix_environmental_reverb, &reverb_settings);
        (void) result;
    }

}

static short *silent_buffer;

void create_buffer_queue_audio_player(int sample_rate) {
    SLresult result;
    silent_buffer = (short *) malloc(sizeof(short) * 1024);
    memset(silent_buffer, 0, sizeof(short) * 1024);
    if (sample_rate >= 0) {
        bq_player_sample_rate = sample_rate * 1000;
    }

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_48,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    /*
     * Enable Fast Audio when possible:  once we set the same rate to be the native, fast audio path
     * will be triggered
     */
    if (bq_player_sample_rate) {
        format_pcm.samplesPerSec = bq_player_sample_rate;       //sample rate in mili second
    }
    SLDataSource audio_src = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, output_mix_object};
    SLDataSink audio_snk = {&loc_outmix, nullptr};

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ };

    result = (*engine_engine)->CreateAudioPlayer(engine_engine, &bq_player_object, &audio_src, &audio_snk,
                                                bq_player_sample_rate ? 2 : 3, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // realize the player
    result = (*bq_player_object)->Realize(bq_player_object, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the play interface
    result = (*bq_player_object)->GetInterface(bq_player_object, SL_IID_PLAY, &bq_player_play);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the buffer queue interface
    result = (*bq_player_object)->GetInterface(bq_player_object, SL_IID_BUFFERQUEUE,
                                             &bq_player_buffer_queue);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // register callback on the buffer queue
    result = (*bq_player_buffer_queue)->RegisterCallback(bq_player_buffer_queue, bq_player_callback,
                                                      nullptr);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the effect send interface
    bq_player_effect_send = nullptr;
    if (0 == bq_player_sample_rate) {
        result = (*bq_player_object)->GetInterface(bq_player_object, SL_IID_EFFECTSEND,
                                                 &bq_player_effect_send);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;
    }

    // get the volume interface
    result = (*bq_player_object)->GetInterface(bq_player_object, SL_IID_VOLUME, &bq_player_volume);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // set the player's state to playing
    result = (*bq_player_play)->SetPlayState(bq_player_play, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
}

void *opensl_thread(void *) {
    create_engine();
    is_running = true;
    int sample_rate = k_sample_rate;
    create_buffer_queue_audio_player(sample_rate);
    SLresult result;
    (*bq_player_volume)->EnableStereoPosition(bq_player_volume, SL_BOOLEAN_TRUE);
    (*bq_player_volume)->SetStereoPosition(bq_player_volume, 1);
    result = (*bq_player_buffer_queue)->Enqueue(bq_player_buffer_queue, silent_buffer, 1024 * 2);
    if (SL_RESULT_SUCCESS != result) {
        return nullptr;
    }
    return nullptr;
}

void start_sl_engine() {
    log_d("opensl", "startSLEngine()");
    pthread_mutex_lock(&audio_engine_lock);
    if (!is_running) { opensl_thread(nullptr); }
    pthread_mutex_unlock(&audio_engine_lock);
}

void stop_sl_engine() {
    pthread_mutex_lock(&audio_engine_lock);
    is_running = false;
    if (bq_player_object) {
        (*bq_player_play)->SetPlayState(bq_player_play, SL_PLAYSTATE_STOPPED);
        (*bq_player_object)->Destroy(bq_player_object);
        bq_player_object = nullptr;
    }
    if (output_mix_object) {
        (*output_mix_object)->Destroy(output_mix_object);
        output_mix_object = nullptr;
    }
    if (engine_object) {
        (*engine_object)->Destroy(engine_object);
        engine_object = nullptr;
    }
    free(silent_buffer);
    silent_buffer = nullptr;
    pthread_mutex_unlock(&audio_engine_lock);
}
