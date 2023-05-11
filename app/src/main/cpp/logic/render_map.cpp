//
// Created by parkyu on 2023/5/11.
//

#include "render_map.h"
#include "../maps/map.h"
#include "render_debug.h"

void MapRender::updateMap(int newMapId, int x, int y) {
    this->mapId = newMapId;
    posX = x * 16;
    posY = y * 16;
    refreshCurrentMap(mapId);
}

byte * MapRender::render(byte *screenBuffer) {
    return renderMap(posX, posY, screenBuffer);
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
    if(functionKey & a) {
        DebugRender *debugRender = new DebugRender;
        push(debugRender);
    }
    return true;
}

MapRender::~MapRender() {
    logd("MapRender", "delete");
}
