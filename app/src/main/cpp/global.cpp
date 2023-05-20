//
// Created by parkyu on 2023/5/11.
//
#include <android/log.h>
#include <__threading_support>
#include <unistd.h>
#include "global.h"

void logd(char *tag, char *msg) {
    __android_log_print(ANDROID_LOG_DEBUG, tag, "%s", msg);
}

void loge(char *tag, char *msg) {
    __android_log_print(ANDROID_LOG_ERROR, tag, "%s", msg);
}

volatile char worker_thread_running = false;
void (*taskStack[10])();
int stackTop = 0;
void * workerThread(void * args) {
    worker_thread_running = true;
    while(worker_thread_running) {
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