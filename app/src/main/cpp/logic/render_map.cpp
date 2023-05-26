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
#include "../graphic/bitmap_render.h"
#include "../sprite/sprite.h"
#include "status/character.h"
#include "../maps/tile_bmp.h"
#include "../maps/map_data/map_data.h"

static const byte HUMAN_PASS = 0b0001;
static const byte CAR_PASS = 0b0010;

static volatile int posX, posY;
static volatile int mapId;

void MapRender::updateMap(int newMapId, int x, int y) {
    if (newMapId < 0) {
        newMapId = 0;
    }
    mapId = newMapId;
    mapId = mapId % 240;
    posX = x * 16;
    posY = y * 16;
    refreshCurrentMap(mapId);
}

MapRender::MapRender() {
    logd("MapRender", "new!");
}

int MapRender::getMapId() {
    return mapId;
}

byte *MapRender::render(byte *screenBuffer) {
    Character *player = getDefaultPlayer();
    posX = player->renderX - 127;
    posY = player->renderY - 127;
    screenBuffer = renderMap(posX, posY, screenBuffer);
    renderBitmapColorOffset(player->currentBitmap, 0,
                            16, 16,
                            player->renderX - posX, player->renderY - posY, screenBuffer);
    if (showDebug) {
        drawCopyRight(screenBuffer);
    }
    return screenBuffer;
}

void MapRender::tikLogic() {
    Character *player = getDefaultPlayer();
    player->tik();
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
        //todo this is debug code
        int map_height = map_size[mapId * 2];
        int map_width = map_size[mapId * 2 + 1];
        getDefaultPlayer()->setPos(map_width / 2, map_height / 2);
        updateMap(mapId, 0, 0);
    }
    if (functionKey & tb) {
        changeAudio(getAudioIdx() + 1);
    }
}

bool canHumanPass(int targetX, int targetY) {
    unsigned short tileIdx = getTileIdx(targetX, targetY);
    byte currentTileFeature = feature[tileIdx];
    return currentTileFeature & HUMAN_PASS;
}

bool MapRender::processKey(byte directKey, byte functionKey) {
    Character *player = getDefaultPlayer();
    if (player->steping) {
        return functionKey == 0;
    }
    int targetX = player->x;
    int targetY = player->y;
    if (directKey & up) {
        targetY--;
    }
    if (directKey & down) {
        targetY++;
    }
    if (directKey & right) {
        targetX++;
    }
    if (directKey & left) {
        targetX--;
    }
    //fixme tile_map.c 's feature data was TOTALLY WRONG!!!
    if (canHumanPass(targetX, targetY) || true) {
        player->x = targetX;
        player->y = targetY;
    }
    return functionKey == 0;
}

MapRender::~MapRender() {
    logd("MapRender", "delete");
}
