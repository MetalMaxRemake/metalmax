//
// Created by parkyu on 2023/5/9.
//

#include "logic.h"
#include "maps/map.h"
#include "charsets.h"
#include "native_gl.h"

const char chinese_demo[4] = {0,1,2,3};
const char *version_string = "PARK_671 TEST VERSION 0.2";
const char *graphic_string = "OPENGL ES 2.0";

byte * screenBuffer;

int posX, posY;

void initFirstBuffer() {
    posX = 0;
    posY = 0;
    refreshCurrentMap(0);
    screenBuffer = getImage(0, 0, screenBuffer);
    drawText(screenBuffer, version_string, 10, 10);
    drawText(screenBuffer, graphic_string, 10, 20);
    drawZhText(screenBuffer, chinese_demo, 4, 10, 30);
    updateScreenBuffer(screenBuffer);
}

byte needChange = 0;

byte* statusMachine;

void refreshMap() {
//    if (debugColorModeEnable) {
//        debugColorMode();
//        return;
//    }
//    currentMapBuffer = getImage(posX, posY, (unsigned char *)currentMapBuffer);
}

void refreshSprite() {
//    currentSpriteBuffer = getSpriteImage(1, 0, nullptr);
}

void mixBuffer() {
}

void onRight() {
    posY++;
    refreshMap();
}

void onLeft() {
    posY--;
    refreshMap();
}

void onUp() {
    posX--;
    refreshMap();
}

void onDown() {
    posX++;
    refreshMap();
}

int mapId = 0;

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