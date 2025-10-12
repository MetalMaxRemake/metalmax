#include <jni.h>
#include <unistd.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include "graphic/native_graphic.h"
#include "audio/native_sound.h"
#include "charset/charsets.h"
#include "opt/mem_opt.h"
#include "logic/logic.h"


extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_initNativeWindow(JNIEnv *env,
                                                     jclass clazz,
                                                     jobject surface,
                                                     jint width,
                                                     jint height) {
    LOGD("bridge", "initNativeWindow:%d, %d", width, height);
    ANativeWindow *mANativeWindow = ANativeWindow_fromSurface(env, surface);
    initGraphic(mANativeWindow, width, height);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_releaseNativeWindow(JNIEnv *env, jclass clazz) {
    releaseGraphic();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_slInit(JNIEnv *env, jclass clazz) {
    initSL();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_slRelease(JNIEnv *env, jclass clazz) {
    releaseSL();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_commonTest(JNIEnv *env, jclass clazz) {
    initLogic();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_onKeyEvent(JNIEnv *env, jclass clazz, jint key) {
    updateDirectKey(key);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_onFuncKeyEvent(JNIEnv *env, jclass clazz, jint key) {
    updateFunctionKey(key);
}