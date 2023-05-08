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
#include "sound/mm_sound.h"
#include "opt/mem_opt.h"
#include <android/log.h>
#include <unistd.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))

#define SAMPLE_RATE 16000
constexpr const uint64_t kFramesToBuffer = 1024;

#define CACHE_SIZE 3

volatile short cachedData[CACHE_SIZE][kFramesToBuffer];
volatile int cacheIdx = 0;
volatile int readIdx = 0;

struct Nsf2WavOptions;

struct Nsf2WavOptions {
    Nsf2WavOptions(const xgm::NSF &nsf)
            : length_ms(nsf.default_playtime),
              fade_ms(nsf.default_fadetime)
    {}

    int32_t length_ms;
    int32_t fade_ms;
    int channels = 1;
    double samplerate = SAMPLE_RATE;
    int track = 2;
    bool quiet = false;
};

pthread_mutex_t soundMutex;
xgm::NSFPlayer player;
int track = 0;

[[noreturn]] void *convertNsf2PCM(void *) {
    xgm::NSF nsf;
    xgm::NSFPlayerConfig config;
    Nsf2WavOptions options(nsf);
    nsf.SetDefaults(options.length_ms, options.fade_ms, nsf.default_loopnum);
    // audio samples, native machine format
    std::unique_ptr<int16_t[]> buf(new int16_t[kFramesToBuffer]);

    nsf.Load((unsigned char *)all_sound, all_sound_size);
    int count = nsf.GetSongNum();
    __android_log_print(ANDROID_LOG_INFO, "test", "mm_song_num:%d", count);

    config["MASTER_VOLUME"] = 128; /* default volume = 128 */
    config["PLAY_ADVANCE"] = 1;

    player.SetConfig(&config);
    player.UpdateInfinite();
    if(!player.Load(&nsf)) {
        fprintf(stderr,"Error with player load\n");
        return nullptr;
    }

    player.SetPlayFreq(options.samplerate);
    player.SetChannels(options.channels);
    player.SetSong(track);
    player.Reset();
    int fc; /* current # of frames to decode */
    while (true) {
        if(cacheIdx - readIdx >= CACHE_SIZE - 1) {
            usleep(1000 * 10);
            continue;
        }
        fc = kFramesToBuffer;
        pthread_mutex_lock(&soundMutex);
        player.Render((short *)cachedData[(cacheIdx++) % CACHE_SIZE], fc);
        pthread_mutex_unlock(&soundMutex);
    }
    return nullptr;
}

char isInited = 0;

extern "C" void initSL() {
    //曾经这里有一堆opensl的屎山代码，怎么调都有bug（破音，降调，加速...
    //后来我弃暗投明选择了传回java用audio tracker
    //以后有缘我再尝试opensl吧
    if(isInited) {
        return;
    }
    isInited = 1;
    pthread_t id;
    //创建函数线程，并且指定函数线程要执行的函数
    pthread_create(&id, nullptr, convertNsf2PCM, nullptr);
}

extern "C" void changeMusic() {
    track++;
    if(track >= 96) {
        track = 0;
    }
    pthread_mutex_lock(&soundMutex);
    player.Reset();
    player.SetSong(track);
    player.Reset();
    pthread_mutex_unlock(&soundMutex);
}

extern "C" short *getBuffer() {
    if(readIdx >= cacheIdx) {
        return nullptr;
    }
    return (short *)cachedData[(readIdx++) % CACHE_SIZE];
}