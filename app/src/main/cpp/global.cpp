//
// Created by parkyu on 2023/5/11.
//
#include <android/log.h>
#include <__threading_support>
#include <unistd.h>
#include "global.h"

void logd(char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    __android_log_print(ANDROID_LOG_DEBUG, tag, fmt, args);
    va_end(args);
}

void loge(char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    __android_log_print(ANDROID_LOG_ERROR, tag, fmt, args);
    va_end(args);
}
