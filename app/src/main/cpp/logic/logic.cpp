//
// Created by parkyu on 2023/5/9.
//

#include <unistd.h>
#include <__threading_support>
#include "logic.h"
#include "../maps/map.h"
#include "../charset/charsets.h"
#include "../graphic/native_gl.h"
#include "render_map.h"
#include "render.h"

byte *screenBuffer[2];
byte bufferIdx = 0;

BaseRender *renderStack[10];
int stackIdx = 0;

void push(BaseRender *baseRender) {
    renderStack[stackIdx++] = baseRender;
}

BaseRender *top() {
    return renderStack[stackIdx - 1];
}

void pop() {
    delete top();
    stackIdx--;
}

#define SCENE_BATTLE 0
#define SCENE_MAP 1
byte scene;

void tikLogic();

void processKey();

void initLogicThread();

void processRender();

void initLogic() {
    MapRender *mapRender = new MapRender;
    mapRender->updateMap(0, 0, 0);
    push(mapRender);
    scene = SCENE_MAP;
    tikLogic();
    initLogicThread();
}

/**
 * very important
 */
void tikLogic() {
    processKey();
    processRender();
}

void processRender() {
    if (scene == SCENE_BATTLE) {
        //todo impl
    } else if (scene == SCENE_MAP) {
        for (int i = 0; i < stackIdx; i++) {
            if (renderStack[i] != nullptr) {
                screenBuffer[bufferIdx] = renderStack[i]->render(screenBuffer[bufferIdx]);
            }
        }
    }
    updateScreenBuffer(screenBuffer[bufferIdx]);
    bufferIdx++;
    bufferIdx = bufferIdx % 2;
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
    top()->processKey(directKey, functionKey);
}

volatile bool logicRunning = true;
//定义线程函数
void *logic_thread(void *arg) {
    logicRunning = true;
    while (logicRunning) {
        tikLogic();
        usleep(16 * 1000);
    }
    return nullptr;
}

void releaseLogicThread() {
    logicRunning = false;
}

void initLogicThread() {
    pthread_t id;
    //创建函数线程，并且指定函数线程要执行的函数
    pthread_create(&id, NULL, logic_thread, NULL);
}