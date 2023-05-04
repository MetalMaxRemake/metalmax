#include <jni.h>
#include <__threading_support>
#include <unistd.h>
#include "native_gl.h"
#include "native_sles.h"

volatile int key, funcKey;

const int up = 0b0001;
const int down = 0b0010;
const int left = 0b0100;
const int right = 0b1000;

const int a = 0b0001;
const int b = 0b0010;
const int ta = 0b0100;
const int tb = 0b1000;

char needChangeMap = 0;

void onLoop(){
    if (key & up) {
        onUp();
    } if (key & down) {
        onDown();
    } if (key & left) {
        onLeft();
    } if (key & right) {
        onRight();
    }
    if(funcKey &a) {
        needChangeMap = 1;
    } else if(needChangeMap) {
        needChangeMap = 0;
        changeMap();
    }
    if(funcKey &b) {

    }
}

//定义线程函数
[[noreturn]] void* logic_thread(void *arg)
{
    while (true) {
        onLoop();
        if(isColorDebugMode()) {
            usleep(100 * 1000);
        }
        usleep(16 * 1000);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_changeBuffer(JNIEnv *env, jclass thiz) {
//    changeBuffer();
    pthread_t id;
    //创建函数线程，并且指定函数线程要执行的函数
    pthread_create(&id, NULL, logic_thread, NULL);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_init(JNIEnv *env, jclass thiz) {
    initGL();
    initSL();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_onChange(JNIEnv *env, jclass thiz, jint width,
                                             jint height) {
    onGLSurfaceChange(width, height);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_onDrawFrame(JNIEnv *env, jclass thiz) {
    onGLDraw();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_onKeyEvent(JNIEnv *env, jclass clazz, jint newKey) {
    key = newKey;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_onFuncKeyEvent(JNIEnv *env, jclass clazz, jint newKey) {
    funcKey = newKey;
}