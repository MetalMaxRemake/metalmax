//
// Created by parkyu on 2023/5/11.
//

#include <cstdlib>
#include "render_map.h"
#include "../maps/map.h"
#include "render_debug.h"
#include "../audio/native_sound.h"
#include "render_battle.h"
#include "../charset/charsets.h"

void MapRender::updateMap(int newMapId, int x, int y) {
    this->mapId = newMapId;
    posX = x * 16;
    posY = y * 16;
    refreshCurrentMap(mapId);
}

MapRender::MapRender() {
}

byte *MapRender::render(byte *screenBuffer) {
    screenBuffer = renderMap(posX, posY, screenBuffer);
    if (showDebug) {
        drawCopyRight(screenBuffer);
    }
    return screenBuffer;
}

void MapRender::drawCopyRight(byte *screenBuffer) const {
    renderAsciText(screenBuffer, "REMAKE: PARK-671", 10, 216);
    renderAsciText(screenBuffer, "THANKS: AFOOLLOVE", 10, 224);
}

void MapRender::onUnFocus() {
    showDebug = false;
}

void MapRender::onFocus() {
    showDebug = true;
    if (getAudioIdx() != 2) {
        changeAudio(2);
    }
}

void MapRender::processKeyClick(byte directKey, byte functionKey) {
    renderCache(0);
    if (functionKey & a) {
        DebugRender *debugRender = new DebugRender;
        push(debugRender);
    }
    if (functionKey & ta) {
        mapId++;
        mapId = mapId % 240;
        updateMap(mapId, 0, 0);
    }
    if (functionKey & tb) {
        changeAudio(getAudioIdx() + 1);
    }
}

bool MapRender::processKey(byte directKey, byte functionKey) {
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
    return functionKey == 0;
}

MapRender::~MapRender() {
    logd("MapRender", "delete");
}
