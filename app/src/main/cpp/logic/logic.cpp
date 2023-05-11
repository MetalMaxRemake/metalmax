//
// Created by parkyu on 2023/5/9.
//

#include <unistd.h>
#include <__threading_support>
#include "logic.h"
#include "../maps/map.h"
#include "../charset/charsets.h"
#include "../graphic/native_gl.h"

const byte up = 0b0001;
const byte down = 0b0010;
const byte left = 0b0100;
const byte right = 0b1000;

const byte a = 0b0001;
const byte b = 0b0010;
const byte ta = 0b0100;
const byte tb = 0b1000;

byte * screenBuffer[2];
byte bufferIdx = 0;

int (*renderProxys[10])(byte *screenBuffer);

void setRender(int (*renderProxy)(byte *screenBuffer), int idx) {
    renderProxys[idx] = renderProxy;
}

int posX, posY;
int mapId = 0;

#define SCENE_BATTLE 0
#define SCENE_MAP 1
byte scene;

void tikLogic();
void processKey();
void initLogicThread();

void initLogic() {
    posX = 0;
    posY = 0;
    mapId = 0;
    scene = SCENE_MAP;
    refreshCurrentMap(mapId);
    tikLogic();
    initLogicThread();
}

/**
 * very important
 */
void tikLogic() {
    processKey();
    if (scene == SCENE_BATTLE) {
        //todo impl
    } else if (scene == SCENE_MAP) {
        screenBuffer[bufferIdx] = renderMap(posX, posY, screenBuffer[bufferIdx]);
        for (int i = 0; i < 10; i++) {
            if (renderProxys[i] != nullptr) {
                renderProxys[i](screenBuffer[bufferIdx]);
            }
        }
    }
    updateScreenBuffer(screenBuffer[bufferIdx]);
    bufferIdx++;
    bufferIdx = bufferIdx % 2;
}

byte needChange = 0;

byte* statusMachine;

void refreshSprite() {
//    currentSpriteBuffer = getSpriteImage(1, 0, nullptr);
}

byte directKey;
byte functionKey;

void updateDirectKey(byte key) {
    directKey = key;
}

void updateFunctionKey(byte key) {
    functionKey = key;
}

void processKey() {
    if (directKey & up) {
        posX--;
    }
    if (directKey & down) {
        posX++;
    }
    if (directKey & right) {
        posY++;
    }
    if (directKey & left) {
        posY--;
    }
}

//定义线程函数
[[noreturn]] void *logic_thread(void *arg) {
    while (true) {
        tikLogic();
        usleep(16 * 1000);
    }
}

void initLogicThread() {
    pthread_t id;
    //创建函数线程，并且指定函数线程要执行的函数
    pthread_create(&id, NULL, logic_thread, NULL);
}

void changeMap() {
//    if (mapId >= MAP_COUNT) {
//        mapId = 0;
//    }
//    posX = 0;
//    posY = 0;
//    startPerf();
//    refreshCurrentMap(mapId);
//    finishPerf("refreshCurrentMap");
//    refreshMap();
//    mapId++;
}