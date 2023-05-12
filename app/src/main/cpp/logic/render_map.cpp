//
// Created by parkyu on 2023/5/11.
//

#include <cstdlib>
#include "render_map.h"
#include "../maps/map.h"
#include "render_debug.h"
#include "../audio/native_sles.h"
#include "render_battle.h"

void MapRender::updateMap(int newMapId, int x, int y) {
    this->mapId = newMapId;
    posX = x * 16;
    posY = y * 16;
    refreshCurrentMap(mapId);
}

byte * MapRender::render(byte *screenBuffer) {
    if (top() != this) {
        return screenBuffer;
    } else {
        return renderMap(posX, posY, screenBuffer);
    }
}

byte taPushed = 0;
byte tbPushed = 0;
byte moved = 0;

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
    if(moved && random() % 1000 == 1) {
        BattleRender *battleRender = new BattleRender;
        push(battleRender);
    }
    if(functionKey & a) {
        DebugRender *debugRender = new DebugRender;
        push(debugRender);
    }
    if(functionKey & ta) {
        taPushed = 1;
    } else if(taPushed) {
        taPushed = 0;
        mapId++;
        mapId = mapId % 240;
        updateMap(mapId, 0, 0);
    }

    if(functionKey & tb) {
        tbPushed = 1;
    } else if(tbPushed) {
        tbPushed = 0;
        changeAudio(getAudioIdx() + 1);
    }
    return true;
}

MapRender::~MapRender() {
    logd("MapRender", "delete");
}
