//
// Created by parkyu on 2023/5/9.
//

#include "logic.h"

const char chinese_demo[4] = {0,1,2,3};
const char *version_string = "PARK_671 TEST VERSION 0.1";
const char *graphic_string = "OPENGL ES 2.0";

void initFirstBuffer() {
    currentMapBuffer = getImage(0, 0, (unsigned char *)currentMapBuffer);
    currentSpriteBuffer = getSpriteImage(0, 0, (unsigned char *)currentSpriteBuffer);
    versionBuffer = getStringImg(version_string);
    graphicBuffer = getStringImg(graphic_string);
    chineseBuffer = getZhStringImg(chinese_demo,4);
}

byte needChange = 0;

byte* statusMachine;

void enterNewMap(int mapId, int x, int y) {
    statusMachine
}


void refreshMap() {
    if (debugColorModeEnable) {
        debugColorMode();
        return;
    }
    currentMapBuffer = getImage(posX, posY, (unsigned char *)currentMapBuffer);
}

void refreshSprite() {
    currentSpriteBuffer = getSpriteImage(1, 0, nullptr);
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
    if (mapId >= MAP_COUNT) {
        mapId = 0;
    }
    posX = 0;
    posY = 0;
    startPerf();
    refreshCurrentMap(mapId);
    finishPerf("refreshCurrentMap");
    refreshMap();
    mapId++;
}