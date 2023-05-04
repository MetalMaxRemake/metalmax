#include <jni.h>
#include <__threading_support>
#include <unistd.h>
#include "native_gl.h"
#include "native_sles.h"

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
        needChange |= map_val;
    } else if(needChange & map_val) {
        needChange &= ~map_val;
        changeMap();
    }
    if(funcKey &b) {
        needChange |= music_val;
    } else if(needChange & music_val) {
        needChange &= ~music_val;
        changeMusic();
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
Java_com_park_metalmax_NativeBridge_changeBuffer() {
//    changeBuffer();
    pthread_t id;
    //创建函数线程，并且指定函数线程要执行的函数
    pthread_create(&id, NULL, logic_thread, NULL);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_init() {
    initGL();
    initSL();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_onChange(jint width, jint height) {
    onGLSurfaceChange(width, height);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_onDrawFrame() {
    onGLDraw();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_onKeyEvent(jint newKey) {
    key = newKey;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_park_metalmax_NativeBridge_onFuncKeyEvent(jint newKey) {
    funcKey = newKey;
}