#include <jni.h>
#include <unistd.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include "graphic/native_graphic.h"
#include "audio/native_sound.h"
#include "charset/charsets.h"
#include "logic/logic.h"
#include "logic/status/game_state.h"

using namespace metalmax;
const char *bridge_tag = "bridge";

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_park_metalmax_NativeBridge_initNativeWindowNative(JNIEnv *env,
                                                           jclass clazz,
                                                           jobject surface) {
    log_d(bridge_tag, "initNativeWindow");
    ANativeWindow *native_window = ANativeWindow_fromSurface(env, surface);
    if (native_window == nullptr) {
        return JNI_FALSE;
    }
    if (!native_graphic::init_graphic(native_window)) {
        ANativeWindow_release(native_window);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_releaseNativeWindowNative(JNIEnv *env, jclass clazz) {
    log_d(bridge_tag, "releaseNativeWindow");
    native_graphic::release_graphic();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_slInitNative(JNIEnv *env, jclass clazz) {
    log_d(bridge_tag, "slInit");
    init_sl();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_slReleaseNative(JNIEnv *env, jclass clazz) {
    log_d(bridge_tag, "slRelease");
    release_sl();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_initLogicNative(JNIEnv *env, jclass clazz) {
    log_d(bridge_tag, "initLogic");
    init_logic();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_onKeyEventNative(JNIEnv *env, jclass clazz, jint key) {
    log_d(bridge_tag, "onKeyEvent");
    update_direct_key(key);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_onFuncKeyEventNative(JNIEnv *env, jclass clazz, jint key) {
    log_d(bridge_tag, "onFuncKeyEvent");
    update_function_key(key);
}

extern "C" JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_setSaveDirectoryNative(JNIEnv *env, jclass, jstring directory) {
    const char *path = env->GetStringUTFChars(directory, nullptr);
    if (path) {
        game_state::set_save_directory(path);
        env->ReleaseStringUTFChars(directory, path);
    }
}
extern "C" JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_setPausedNative(JNIEnv *, jclass, jboolean paused) {
    set_logic_paused(paused);
}
