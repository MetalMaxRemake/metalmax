//
// Created by parkyu on 2023/5/9.
//

#include <unistd.h>
#include <__threading_support>
#include <cstdlib>
#include <typeinfo>
#include "logic.h"
#include "../maps/map.h"
#include "../charset/charsets.h"
#include "../graphic/native_graphic.h"
#include "render_map.h"
#include "render.h"
#include "render_splash.h"

const byte up = 0b0001;
const byte down = 0b0010;
const byte left = 0b0100;
const byte right = 0b1000;

byte tik_clk = 16;

BaseRender *renderStack[10];
MapRender *mapRender;
int stackIdx = 0;
volatile bool logicRunning = true;

template<class Base, typename Derived>

bool instanceOf(const Derived &object) {
    return !dynamic_cast<Base *>(object);
}

void push(BaseRender *baseRender) {
    bool isMapRender = instanceOf<MapRender>(baseRender);
    if (isMapRender) {
        mapRender = (MapRender *) baseRender;
    }
    if (stackIdx > 0 && top() != nullptr) {
        top()->onUnFocus();
    }
    renderStack[stackIdx++] = baseRender;
    if (stackIdx > 0 && top() != nullptr) {
        top()->onFocus();
    }
}

BaseRender *top() {
    return renderStack[stackIdx - 1];
}

void pop() {
    if (stackIdx > 0 && top() != nullptr) {
        top()->onUnFocus();
    }
    delete top();
    stackIdx--;
    if (stackIdx > 0 && top() != nullptr) {
        top()->onFocus();
    }
}

int getCurrentMap() {
    if (mapRender != nullptr) {
        return mapRender->getMapId();
    }
    return -1;
}

void changeMap(int mapId, int x, int y) {
    if (mapRender != nullptr) {
        mapRender->updateMap(mapId, x, y);
    }
}

void tikLogic();

void processKey();

void processLogic();

void initLogicThread();

byte *renderScreen(byte *buffer);

void initLogic() {
    if (!logicRunning) {
        return;
    }
    setRenderCallback(&renderScreen);
    SplashRender *splashRender = new SplashRender;
    push(splashRender);
    tikLogic();
    initLogicThread();
}

/**
 * very important
 */
void tikLogic() {
    processKey();
    processLogic();
}

void renderFps(byte *currentBuffer) {
    char fpsInfo[110];
    sprintf(fpsInfo, "FPS %d", getFps());
    renderAsciText(currentBuffer, fpsInfo, 0, 0);
}

byte *renderScreen(byte *screenBuffer) {
    for (int i = 0; i < stackIdx; i++) {
        if (renderStack[i] != nullptr) {
            screenBuffer = renderStack[i]->render(screenBuffer);
        }
    }
    renderFps(screenBuffer);
    return screenBuffer;
}

byte directKey = 0;
byte functionKey = 0;

byte last_directKey = 0;
byte last_functionKey = 0;

void updateDirectKey(byte key) {
    directKey = key;
}

void updateFunctionKey(byte key) {
    functionKey = key;
}

void processLogic() {
    top()->tikLogic();
}

void processKey() {
    if (top()->processKey(directKey, functionKey)) {
        last_directKey = directKey;
        last_functionKey = functionKey;
        return;
    }
    byte diff_directKey = 0;
    byte diff_functionKey = 0;
    if (directKey > last_directKey) {
        diff_directKey = (directKey | last_directKey) & ~(directKey & last_directKey);
    }
    if (functionKey > last_functionKey) {
        diff_functionKey = (functionKey | last_functionKey) & ~(functionKey & last_functionKey);
    }
    if (diff_directKey | diff_functionKey) {
        top()->processKeyClick(diff_directKey, diff_functionKey);
    }
    last_directKey = directKey;
    last_functionKey = functionKey;
}

//定义线程函数
void *logic_thread(void *arg) {
    logicRunning = true;
    while (logicRunning) {
        tikLogic();
        usleep(tik_clk * 1000);
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