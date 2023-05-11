#include <jni.h>
#include <__threading_support>
#include <unistd.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include "native_gl.h"
#include "native_sles.h"
#include "charsets.h"
#include "palette_data.h"
#include "opt/mem_opt.h"
#include "logic.h"

typedef unsigned char byte;

volatile int key, funcKey;

const byte up = 0b0001;
const byte down = 0b0010;
const byte left = 0b0100;
const byte right = 0b1000;

const byte a = 0b0001;
const byte b = 0b0010;
const byte ta = 0b0100;
const byte tb = 0b1000;

const byte map_val = 0b0001;
const byte music_val = 0b0010;

//todo
int audioIdx = 0;

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
//    if (funcKey & a) {
//        needChange |= map_val;
//    } else if (needChange & map_val) {
//        needChange &= ~map_val;
//        changeMap();
//    }
//    if (funcKey & b) {
//        needChange |= music_val;
//    } else if (needChange & music_val) {
//        needChange &= ~music_val;
//        changeAudio(audioIdx++);
//    }
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

void getAudioBufferJNI(JNIEnv *env, jclass clazz, jshortArray array) {
    short *buffer = getAudioBuffer();
    if(buffer != nullptr) {
        env->SetShortArrayRegion(array, 0, 1024, buffer);
    }
}

void slInit(JNIEnv *env, jclass clazz) {
    initSL();
}

volatile int window_height, window_width;

//定义线程函数
[[noreturn]] void *gl_thread(void *arg) {
    while (true) {
        onGLDraw();
        usleep(1000*16);
    }
}

void initNativeWindow(JNIEnv *env, jclass clazz, jobject surface) {
    ANativeWindow* mANativeWindow = ANativeWindow_fromSurface(env, surface);

    window_height = ANativeWindow_getHeight(mANativeWindow);
    window_width = ANativeWindow_getWidth(mANativeWindow);
    ANativeWindow_setBuffersGeometry(mANativeWindow,
                                     window_width,
                                     window_height,
                                     WINDOW_FORMAT_RGBA_8888);
    initEGL(mANativeWindow);
    pthread_t id;
    initGL();
    onGLSurfaceChange(window_width, window_height);
    onGLDraw();
    pthread_create(&id, NULL, gl_thread, NULL);
}

static JNINativeMethod methods[] = {
        {"commonTest",   "()V",   (void *) &commonTest},
        {"onKeyEvent",     "(I)V",  (void *) &onKeyEvent},
        {"onFuncKeyEvent", "(I)V",  (void *) &onFuncKeyEvent},
        {"glInit",         "()V",   (void *) &init},
        {"glOnChange",     "(II)V", (void *) &onChange},
        {"glOnDrawFrame",  "()V",   (void *) &onDrawFrame},
        {"getAudioBuffer", "([S)V",   (void *) &getAudioBufferJNI},
        {"slInit", "()V",   (void *) &slInit},
        {"initNativeWindow", "(Landroid/view/Surface;)V",   (void *) &initNativeWindow},
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

__attribute__((constructor)) static void onDlOpen(void) {
    __android_log_print(ANDROID_LOG_INFO, "test", "on dlopen");
}