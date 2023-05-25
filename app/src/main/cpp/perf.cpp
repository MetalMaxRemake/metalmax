//
// Created by parkyu on 2023/5/8.
//

#include "perf.h"
#include <android/log.h>
#include <ctime>

clock_t start_clock, finish_clock;

extern "C" void startPerf() {
    start_clock = clock();
}

extern "C" void finishPerf(const char *tag) {
    finish_clock = clock();
    __android_log_print(ANDROID_LOG_ERROR,
                        tag,
                        "time cost:%.3fms",
                        (((double) (finish_clock - start_clock)) / CLOCKS_PER_SEC) * 1000);
}

extern "C" long getDuration() {
    finish_clock = clock();
    return finish_clock - start_clock;
}