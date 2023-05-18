#include <jni.h>
#include <__threading_support>
#include <unistd.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include "graphic/native_graphic.h"
#include "audio/native_sound.h"
#include "charset/charsets.h"
#include "graphic/palette_data.h"
#include "opt/mem_opt.h"
#include "logic/logic.h"

void commonTest(JNIEnv *env, jclass clazz) {
    //test code
    initLogic();
}

void onKeyEvent(JNIEnv *env, jclass clazz, jint key) {
    updateDirectKey(key);
}

void onFuncKeyEvent(JNIEnv *env, jclass clazz, jint key) {
    updateFunctionKey(key);
}

jboolean nativeCharToJavaBmp(JNIEnv *env, jclass clazz, jobject bitmap, jstring text) {
    const char *textInChar = env->GetStringUTFChars(text, JNI_FALSE);
    byte * nativeBmp = getStringImg(textInChar);
    AndroidBitmapInfo bitmapInfo;
    AndroidBitmap_getInfo(env, bitmap, &bitmapInfo);
    void *pixels;
    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) {
        return false;
    }
    int *data = (int *)pixels;
    int dataIdx = 0;
    for(int i = 0;i<bitmapInfo.height;i++) {
        for(int j = 0;j<bitmapInfo.width;j++) {
            data[dataIdx] = palette[nativeBmp[dataIdx]];
            dataIdx++;
        }
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    return true;
}

void slInit(JNIEnv *env, jclass clazz) {
    initSL();
}

void slRelease(JNIEnv *env, jclass clazz) {
    releaseSL();
}

void initNativeWindow(JNIEnv *env, jclass clazz, jobject surface) {
    ANativeWindow* mANativeWindow = ANativeWindow_fromSurface(env, surface);
    initGraphic(mANativeWindow);
}

void releaseNativeWindow(JNIEnv *env, jclass clazz) {
    releaseGraphic();
}

static JNINativeMethod methods[] = {
        {"commonTest",   "()V",   (void *) &commonTest},
        {"onKeyEvent",     "(I)V",  (void *) &onKeyEvent},
        {"onFuncKeyEvent", "(I)V",  (void *) &onFuncKeyEvent},
        {"slInit", "()V",   (void *) &slInit},
        {"slRelease", "()V",   (void *) &slRelease},
        {"initNativeWindow", "(Landroid/view/Surface;)V",   (void *) &initNativeWindow},
        {"releaseNativeWindow", "()V",   (void *) &releaseNativeWindow},
        {"getCharImg", "(Landroid/graphics/Bitmap;Ljava/lang/String;)Z",   (void *) &nativeCharToJavaBmp},
};

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_initNativeMethod(JNIEnv *env, jclass clazz) {
    (*env).RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0]));
}

__attribute__((constructor)) static void onDlOpen(void) {
    logd("test", "on dlopen");
}