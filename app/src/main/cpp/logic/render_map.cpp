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

byte *MapRender::render(byte *screenBuffer) {
    screenBuffer = renderMap(posX, posY, screenBuffer);
    renderAsciText(screenBuffer, "COPYRIGHT:    CREA-TECH", 10, 200);
    renderAsciText(screenBuffer, "PUBLISH:      DATA EAST", 10, 208);
    renderAsciText(screenBuffer, "REMAKE AUTHOR: PARK-671", 10, 216);
    renderAsciText(screenBuffer, "THANKS:       AFOOLLOVE", 10, 224);
    return screenBuffer;
}

byte taPushed = 0;
byte tbPushed = 0;
byte moved = 0;
bool aPushedb = false;

bool MapRender::processKey(byte directKey, byte functionKey) {
    moved = 0;
    if (directKey & up) {
        posX--;
        moved = 1;
    }
    if (directKey & down) {
        posX++;
        moved = 1;
    }
    if (directKey & right) {
        posY++;
        moved = 1;
    }
    if (directKey & left) {
        posY--;
        moved = 1;
    }
    if (functionKey & a) {
        aPushedb = true;
    } else if (aPushedb) {
        aPushedb = false;
        DebugRender *debugRender = new DebugRender;
        push(debugRender);
    }
    if (functionKey & ta) {
        taPushed = 1;
    } else if (taPushed) {
        taPushed = 0;
        mapId++;
        mapId = mapId % 240;
        updateMap(mapId, 0, 0);
    }

    if (functionKey & tb) {
        tbPushed = 1;
    } else if (tbPushed) {
        tbPushed = 0;
        changeAudio(getAudioIdx() + 1);
    }
    return true;
}

MapRender::~MapRender() {
    logd("MapRender", "delete");
}
