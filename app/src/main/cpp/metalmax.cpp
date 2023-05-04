#include <jni.h>
#include <__threading_support>
#include <unistd.h>
#include <android/bitmap.h>
#include "native_gl.h"
#include "native_sles.h"
#include "charsets.h"
#include "palette_data.h"
#include "opt/mem_opt.h"

volatile int key, funcKey;

const unsigned char up = 0b0001;
const unsigned char down = 0b0010;
const unsigned char left = 0b0100;
const unsigned char right = 0b1000;

const unsigned char a = 0b0001;
const unsigned char b = 0b0010;
const unsigned char ta = 0b0100;
const unsigned char tb = 0b1000;

const unsigned char map_val = 0b0001;
const unsigned char music_val = 0b0010;
unsigned char needChange = 0;

void onLoop() {
    if (key & up) {
        onUp();
    }
    if (key & down) {
        onDown();
    }
    if (key & left) {
        onLeft();
    }
    if (key & right) {
        onRight();
    }
    if (funcKey & a) {
        needChange |= map_val;
    } else if (needChange & map_val) {
        needChange &= ~map_val;
        changeMap();
    }
    if (funcKey & b) {
        needChange |= music_val;
    } else if (needChange & music_val) {
        needChange &= ~music_val;
        changeMusic();
    }
}

//定义线程函数
[[noreturn]] void *logic_thread(void *arg) {
    while (true) {
        onLoop();
        if (isColorDebugMode()) {
            usleep(100 * 1000);
        }
        usleep(16 * 1000);
    }
}

void commonTest(JNIEnv *env, jclass clazz) {
    //test code
    initFirstBuffer();
}

void init(JNIEnv *env, jclass clazz) {
    initGL();
    initSL();
}

void onChange(JNIEnv *env, jclass clazz, jint width, jint height) {
    onGLSurfaceChange(width, height);
}

void onDrawFrame(JNIEnv *env, jclass clazz) {
    onGLDraw();
}

void onKeyEvent(JNIEnv *env, jclass clazz, jint newKey) {
    key = newKey;
}

void onFuncKeyEvent(JNIEnv *env, jclass clazz, jint newKey) {
    funcKey = newKey;
}

jboolean nativeCharToJavaBmp(JNIEnv *env, jclass clazz, jobject bitmap, jstring text) {
    const char *textInChar = env->GetStringUTFChars(text, JNI_FALSE);
    unsigned char * nativeBmp = getStringImg(textInChar);
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
            data[dataIdx] = palette1[nativeBmp[dataIdx]];
            dataIdx++;
        }
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    return true;
}

static JNINativeMethod methods[] = {
        {"commonTest",   "()V",   (void *) &commonTest},
        {"onKeyEvent",     "(I)V",  (void *) &onKeyEvent},
        {"onFuncKeyEvent", "(I)V",  (void *) &onFuncKeyEvent},
        {"glInit",         "()V",   (void *) &init},
        {"glOnChange",     "(II)V", (void *) &onChange},
        {"glOnDrawFrame",  "()V",   (void *) &onDrawFrame},
        {"getCharImg", "(Landroid/graphics/Bitmap;Ljava/lang/String;)Z",   (void *) &nativeCharToJavaBmp},
};

extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_initNativeMethod(JNIEnv *env, jclass clazz) {
    (*env).RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0]));
    pthread_t id;
    //创建函数线程，并且指定函数线程要执行的函数
    pthread_create(&id, NULL, logic_thread, NULL);
}