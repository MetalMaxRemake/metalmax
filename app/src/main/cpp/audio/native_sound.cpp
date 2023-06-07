//
// Created by youngpark on 2023/4/4.
//

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <queue>
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <stdbool.h>
#include "xgm/xgm.h"
#include "../opt/mem_opt.h"
#include "mm_sound.h"
#include "opensl.h"
#include "../global.h"
#include <android/log.h>
#include <unistd.h>

#define SAMPLE_RATE 16000
constexpr const uint64_t kFramesToBuffer = 1024;

#define CACHE_SIZE 3
#define PRE_CACHE_LEN 10

volatile short cachedData[CACHE_SIZE][kFramesToBuffer];
volatile int cacheIdx = 0;
volatile int readIdx = 0;
volatile bool mixCache = false;

struct Nsf2WavOptions;

struct Nsf2WavOptions {
    Nsf2WavOptions(const xgm::NSF &nsf)
            : length_ms(nsf.default_playtime),
              fade_ms(nsf.default_fadetime) {}

    int32_t length_ms;
    int32_t fade_ms;
    int channels = 1;
    double samplerate = SAMPLE_RATE;
};

pthread_mutex_t soundMutex;
pthread_mutex_t cacheIdxMutex;
xgm::NSFPlayer player1, player2;
xgm::NSF nsf;

/**
 * mix buffer2 into buffer1
 * @param buffer1
 * @param buffer2
 * @param len
 */
void mixSoundBuffer(short *buffer1, short *buffer2, int len) {
    for (int i = 0; i < len; i++) {
        int pcm = ((int) buffer1[i]) + ((int) buffer2[i]);
        if (pcm > 32767) {
            pcm = 32767;
        } else if (pcm < -32768) {
            pcm = -32768;
        }
        buffer1[i] = (short) pcm;
    }
}

void *nsfPlayerThread(void *) {
    xgm::NSFPlayerConfig config;
    Nsf2WavOptions options(nsf);
    nsf.SetDefaults(options.length_ms, options.fade_ms, nsf.default_loopnum);
    // audio samples, native machine format
    std::unique_ptr<int16_t[]> buf(new int16_t[kFramesToBuffer]);

    nsf.Load((unsigned char *) all_sound, all_sound_size);
    nsf.Load((unsigned char *) all_sound, all_sound_size);
    int count = nsf.GetSongNum();
    __android_log_print(ANDROID_LOG_INFO, "nsfPlayerThread", "mm_song_num:%d", count);

    config["MASTER_VOLUME"] = 128; /* default volume = 128 */
    config["PLAY_ADVANCE"] = 1;

    player1.SetConfig(&config);
    player1.UpdateInfinite();
    if (!player1.Load(&nsf)) {
        fprintf(stderr, "Error with player1 load\n");
        return nullptr;
    }
    player1.SetPlayFreq(options.samplerate);
    player1.SetChannels(options.channels);

    player2.SetConfig(&config);
    player2.UpdateInfinite();
    if (!player2.Load(&nsf)) {
        fprintf(stderr, "Error with player2 load\n");
        return nullptr;
    }
    player2.SetPlayFreq(options.samplerate);
    player2.SetChannels(options.channels);

    //start play!
    player1.SetSong(0);
    player1.Reset();
    int fc; /* current # of frames to decode */
    while (true) {
        if (cacheIdx - readIdx >= CACHE_SIZE - 1) {
            usleep(1000 * 10);
            continue;
        }
        fc = kFramesToBuffer;
        pthread_mutex_lock(&soundMutex);
        pthread_mutex_lock(&cacheIdxMutex);
        player1.Render((short *) cachedData[(cacheIdx) % CACHE_SIZE], fc);
        if (mixCache) {
            short effectBuffer[kFramesToBuffer];
            player2.Render((short *) effectBuffer, fc);
            mixSoundBuffer((short *) cachedData[(cacheIdx) % CACHE_SIZE],
                           (short *) effectBuffer,
                           fc);
        }
        cacheIdx++;
        pthread_mutex_unlock(&soundMutex);
        pthread_mutex_unlock(&cacheIdxMutex);
    }
    return nullptr;
}

char isInited = 0;

extern "C" void initSL() {
    startSLEngine();
    if (isInited) {
        return;
    }
    isInited = 1;
    pthread_t id;
    //创建函数线程，并且指定函数线程要执行的函数
    pthread_create(&id, nullptr, nsfPlayerThread, nullptr);
}

extern "C" void releaseSL() {
    stopSLEngine();
}

extern "C" int getAudioCount() {
    return nsf.GetSongNum();
}

int player1AudioId = 0, player2AudioId = 63;
/**
 * 23 - 遇到挂物
 * 63 - 按键
 * [23, 91]
 */
extern "C" void changeAudio(int audioIdx) {
    pthread_mutex_lock(&soundMutex);
    player1.SetSong(audioIdx);
    player1AudioId = audioIdx;
    player1.Reset();
    pthread_mutex_unlock(&soundMutex);
}

void *renderEffectThread(void *) {
    pthread_mutex_lock(&soundMutex);
    player2.SetSong(player2AudioId);
    player2.Reset();
    mixCache = true;
    pthread_mutex_unlock(&soundMutex);
    return nullptr;
}

extern "C" void renderEffect(int effectAudioId) {
    player2AudioId = effectAudioId;
    pthread_t id;
    //创建函数线程，并且指定函数线程要执行的函数
    pthread_create(&id, nullptr, renderEffectThread, nullptr);

}

extern "C" int getAudioIdx() {
    return player1AudioId;
}

extern "C" short *getAudioBuffer() {
    if (readIdx >= cacheIdx) {
        return nullptr;
    }
    if (readIdx > 100000) {
        pthread_mutex_lock(&cacheIdxMutex);
        logd("native_sound", "reset cacheIdx");
        if (readIdx < cacheIdx) {
            cacheIdx %= CACHE_SIZE;
            readIdx %= CACHE_SIZE;
            if (readIdx >= cacheIdx) {
                cacheIdx += CACHE_SIZE;
            }
        }
        pthread_mutex_unlock(&cacheIdxMutex);
    }
    return (short *) cachedData[(readIdx++) % CACHE_SIZE];
}