//
// Created by parkyu on 2023/5/11.
//

#include <cstdlib>
#include <__threading_support>
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
#include "../opt/mem_opt.h"
#include "../graphic/palette_data.h"
#include "../audio/mm_sound.h"

static const byte HUMAN_PASS = 0b0001;
static const byte CAR_PASS = 0b0010;

static volatile int posX, posY;
static volatile int mapId;

void MapRender::updateMap(int newMapId, int x, int y) {
    if (newMapId < 0) {
        newMapId = 0;
    }
    renderEffect(ENTER_ENTRANCE);
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

pthread_mutex_t changeMapMutex;

byte *MapRender::render(byte *screenBuffer) {
    if (skipFrame) {
        __memset_aarch64(screenBuffer, BLACK, 256 * 256);
        return screenBuffer;
    }
    pthread_mutex_lock(&changeMapMutex);
    Character *player = getDefaultPlayer();
    posX = player->renderX - 127;
    posY = player->renderY - 127;
    screenBuffer = renderMap(posX, posY, screenBuffer);
    renderBitmapColorOffset(player->currentBitmap, 0,
                            16, 16,
                            player->renderX - posX, player->renderY - posY, screenBuffer);
    pthread_mutex_unlock(&changeMapMutex);
    if (showDebug) {
        drawCopyRight(screenBuffer);
    }
    return screenBuffer;
}

void MapRender::tikLogic() {
    Character *player = getDefaultPlayer();
    player->tik();
    if (skipFrame) {
        skipFrame--;
    }
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
    renderEffect(PUSH_BUTTON_EFFECT);
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

struct Pos {
    int mapId;
    int x, y;
};

Pos lastPosStack[10];
int lastPosStackTop = 0;

bool MapRender::processKey(byte directKey, byte functionKey) {
    Character *player = getDefaultPlayer();
    if (player->steping || directKey == 0) {
        return functionKey == 0;
    }
    int targetX = player->x;
    int targetY = player->y;
    if (directKey & up) {
        targetY--;
        player->direct = up;
    }
    if (directKey & down) {
        targetY++;
        player->direct = down;
    }
    if (directKey & right) {
        targetX++;
        player->direct = right;
    }
    if (directKey & left) {
        targetX--;
        player->direct = left;
    }
    //fixme tile_map.c 's feature data was TOTALLY WRONG!!!
    if (canHumanPass(targetX, targetY) || true) {
        if (checkEntrance(player, targetX, targetY)) {

        } else if (checkOutOfMap(player, targetX, targetY)) {

        } else {
            player->x = targetX;
            player->y = targetY;
        }
    }
    return functionKey == 0;
}

bool MapRender::checkOutOfMap(Character *player, int targetX, int targetY) {
    int map_height = map_size[mapId * 2];
    int map_width = map_size[mapId * 2 + 1];
    if (targetX < 0
        || targetY < 0
        || targetX > map_width
        || targetY > map_height) {
        skipFrame = 5;
        pthread_mutex_lock(&changeMapMutex);
        --lastPosStackTop;
        player->setPos(lastPosStack[lastPosStackTop].x, lastPosStack[lastPosStackTop].y);
        updateMap(lastPosStack[lastPosStackTop].mapId, 0, 0);
        pthread_mutex_unlock(&changeMapMutex);
        return true;
    }
    return false;
}

bool MapRender::checkEntrance(Character *player, int targetX, int targetY) {
    for (int i = 0; i < entrance_count[mapId]; i++) {
        byte entrance_x = entrances[mapId][i * 5];
        byte entrance_y = entrances[mapId][i * 5 + 1];
        if (targetX == entrance_x && targetY == entrance_y) {
            skipFrame = 5;
            int nextMapId = entrances[mapId][i * 5 + 4];
            bool isBack = false;
            for (int j = 0; j < lastPosStackTop; j++) {
                if (lastPosStack[j].mapId == nextMapId) {
                    lastPosStackTop = j;
                    isBack = true;
                    break;
                }
            }
            if (!isBack) {
                lastPosStack[lastPosStackTop].x = targetX;
                lastPosStack[lastPosStackTop].y = targetY;
                lastPosStack[lastPosStackTop++].mapId = mapId;
            }
            pthread_mutex_lock(&changeMapMutex);
            player->setPos(entrances[mapId][i * 5 + 2], entrances[mapId][i * 5 + 3]);
            updateMap(nextMapId, 0, 0);
            pthread_mutex_unlock(&changeMapMutex);
            return true;
        }
    }
    return false;
}

MapRender::~MapRender() {
    logd("MapRender", "delete");
}
