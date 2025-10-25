#include <jni.h>
#include <unistd.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include "graphic/native_graphic.h"
#include "audio/native_sound.h"
#include "charset/charsets.h"
#include "logic/logic.h"

const char *BRIDGE_TAG = "bridge";

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_park_metalmax_NativeBridge_initNativeWindowNative(JNIEnv *env,
                                                           jclass clazz,
                                                           jobject surface) {
    LOGD(BRIDGE_TAG, "initNativeWindow");
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (nativeWindow == nullptr) {
        return JNI_FALSE;
    }
    if (!native_graphic::initGraphic(nativeWindow)) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_releaseNativeWindowNative(JNIEnv *env, jclass clazz) {
    LOGD(BRIDGE_TAG, "releaseNativeWindow");
    native_graphic::releaseGraphic();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_slInitNative(JNIEnv *env, jclass clazz) {
    LOGD(BRIDGE_TAG, "slInit");
    initSL();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_slReleaseNative(JNIEnv *env, jclass clazz) {
    LOGD(BRIDGE_TAG, "slRelease");
    releaseSL();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_initLogicNative(JNIEnv *env, jclass clazz) {
    LOGD(BRIDGE_TAG, "initLogic");
    initLogic();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_onKeyEventNative(JNIEnv *env, jclass clazz, jint key) {
    LOGD(BRIDGE_TAG, "onKeyEvent");
    updateDirectKey(key);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_onFuncKeyEventNative(JNIEnv *env, jclass clazz, jint key) {
    LOGD(BRIDGE_TAG, "onFuncKeyEvent");
    updateFunctionKey(key);
}
