//
// Created by parkyu on 2023/5/11.
//
#include <android/log.h>
#include <__threading_support>
#include <unistd.h>
#include "global.h"

char *get_current_time_str() {
    static char buffer[30];
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm *local = localtime(&tv.tv_sec);

    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
             local->tm_year + 1900,
             local->tm_mon + 1,
             local->tm_mday,
             local->tm_hour,
             local->tm_min,
             local->tm_sec,
             tv.tv_usec / 1000);

    return buffer;
}


void logd(char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (false) {
        get_current_time_str();
    }
    __android_log_print(ANDROID_LOG_DEBUG, tag, fmt, args);
    va_end(args);
}

void loge(char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (false) {
        get_current_time_str();
    }
    __android_log_print(ANDROID_LOG_ERROR, tag, fmt, args);
    va_end(args);
}

volatile char worker_thread_running = false;

void (*taskStack[10])();

int stackTop = 0;

void *workerThread(void *args) {
    worker_thread_running = true;
    while (worker_thread_running) {
        if (stackTop == 0) {
            usleep(1000 * 100);
        } else {
            stackTop--;
            taskStack[stackTop]();
        }
    }
    return nullptr;
}

void postTask(void(*task)()) {
    taskStack[stackTop++] = task;
}

void startWorkerThread() {
    pthread_t id;
    //创建函数线程，并且指定函数线程要执行的函数
    pthread_create(&id, nullptr, workerThread, nullptr);
}

void stopWorkerThread() {
    worker_thread_running = false;
}