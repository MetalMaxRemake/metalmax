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
#include "../graphic/native_graphic.h"
#include "../monster/monster_data/monster_data.h"

#define ANIMATION_DURATION 20

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
    renderEffect(ENTER_ENTRANCE);
    refreshMusic();
    posX = x * 16;
    posY = y * 16;
    refreshCurrentMap(mapId);
}

MapRender::MapRender() {
    logd("MapRender", "new!");
    srand((unsigned) time(nullptr));
}

int MapRender::getMapId() {
    return mapId;
}

pthread_mutex_t changeMapMutex;

byte *MapRender::render(byte *screenBuffer) {
    if (entranceAnimation > ANIMATION_DURATION / 2) {
        fadeOut();
    } else if (entranceAnimation) {
        fadeIn();
    } else if (!isOriginPalette) {
        isOriginPalette = true;
        resetPalette();
    }
    pthread_mutex_lock(&changeMapMutex);
    Character *player = getDefaultPlayer();
    posX = player->renderX - 127;
    posY = player->renderY - 127;
    screenBuffer = renderMap(posX, posY, screenBuffer);
    renderBitmapColorOffset(player->currentBitmap, 0,
                            16, 16,
                            127, 127, screenBuffer);
    pthread_mutex_unlock(&changeMapMutex);
    if (showDebug) {
        drawCopyRight(screenBuffer);
    }
    return screenBuffer;
}

void MapRender::resetPalette() const {
    int *currentPalette = getCurrentPalette();
    for (int i = 0; i < 256; i++) {
        int dd = palette[i];
        int b = (dd & 0x00FF0000) >> 16;
        int g = (dd & 0x0000FF00) >> 8;
        int r = (dd & 0x000000FF) >> 0;
        currentPalette[i] = 0xff000000 | (r << 16) | (g << 8) | b;
    }
    refreshPalette(currentPalette);
}

void MapRender::fadeIn() const {
    int *currentPalette = getCurrentPalette();
    for (int i = 0; i < 256; i++) {
        unsigned int origindd = palette[i];
        unsigned int o_b = (origindd & 0x00FF0000) >> 16;
        unsigned int o_g = (origindd & 0x0000FF00) >> 8;
        unsigned int o_r = (origindd & 0x000000FF) >> 0;

        unsigned int dd = currentPalette[i];
        unsigned int b = (dd & 0x00FF0000) >> 16;
        unsigned int g = (dd & 0x0000FF00) >> 8;
        unsigned int r = (dd & 0x000000FF) >> 0;
        if (b <= 255) {
            b += (o_b / ANIMATION_DURATION);
        }
        if (g <= 255) {
            g += (o_g / ANIMATION_DURATION);
        }
        if (r <= 255) {
            r += (o_r / ANIMATION_DURATION);
        }
        currentPalette[i] = 0xff000000 | (r << 16) | (g << 8) | b;
    }
    refreshPalette(currentPalette);
}

void MapRender::fadeOut() const {
    int *currentPalette = getCurrentPalette();
    for (int i = 0; i < 256; i++) {
        unsigned int dd = currentPalette[i];
        unsigned int b = (dd & 0x00FF0000) >> 16;
        unsigned int g = (dd & 0x0000FF00) >> 8;
        unsigned int r = (dd & 0x000000FF) >> 0;
        int step = 20;
        if (b >= step) {
            b -= step;
        } else {
            b = 0;
        }
        if (g >= step) {
            g -= step;
        } else {
            g = 0;
        }
        if (r >= step) {
            r -= step;
        } else {
            r = 0;
        }
        currentPalette[i] = 0xff000000 | (r << 16) | (g << 8) | b;
    }
    refreshPalette(currentPalette);
}

void MapRender::tikLogic() {
    Character *player = getDefaultPlayer();
    player->tik();
    if (entranceAnimation) {
        isOriginPalette = false;
        entranceAnimation--;
    }
}

void MapRender::drawCopyRight(byte *screenBuffer) const {
    renderAsciText(screenBuffer, "REMAKE: PARK-671", 10, 216);
    renderAsciText(screenBuffer, "THANKS: AFOOLLOVE", 10, 224);
}

void MapRender::onUnFocus() {
    showDebug = false;
}

void MapRender::refreshMusic() const {
    byte currentMusic = map_music[mapId] - 3;
    if (getAudioIdx() != currentMusic) {
        changeAudio(currentMusic);
    }
}

void MapRender::onFocus() {
    showDebug = true;
    refreshMusic();
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
    if (entranceAnimation) {
        return true;
    }
    if (player->steping || directKey == 0 || entranceAnimation) {
        return functionKey == 0;
    }
    int targetX = player->x;
    int targetY = player->y;
    int nextDirect = direct_down;
    if (directKey & up) {
        targetY--;
        nextDirect = direct_up;
    }
    if (directKey & down) {
        targetY++;
        nextDirect = direct_down;
    }
    if (directKey & right) {
        targetX++;
        nextDirect = direct_right;
    }
    if (directKey & left) {
        targetX--;
        nextDirect = direct_left;
    }
    //fixme tile_map.c 's feature data was TOTALLY WRONG!!!
    if (canHumanPass(targetX, targetY) || true) {
        if (checkEntrance(player, targetX, targetY)) {
            player->direct = nextDirect;
        } else if (checkOutOfMap(player, targetX, targetY)) {
            player->direct = nextDirect;
        } else {
            player->x = targetX;
            player->y = targetY;
            triggerMonster();
        }
    } else {
        player->direct = nextDirect;
    }
    return functionKey == 0;
}

void MapRender::triggerMonster() const {
    if (getDefaultPlayer()->godMode || rand() % 100 > 10) {
        return;
    }
    int monsterGroupId = 0;
    if (mapId == 0) {
        int monsterArea = (getDefaultPlayer()->y / 16) * 16 + (getDefaultPlayer()->x / 16);
        monsterGroupId = monster_distribution_world[monsterArea];
    } else if (mapId >= 0x80) {
        monsterGroupId = monster_distribution_4_samll_maps[mapId - 0x80];
    }
    if (monsterGroupId == 0) {
        return;
    }
    int hitMonsterIdx = rand() % 0x0A;
    int monsterId = monster_group[monsterGroupId * 0x0A + hitMonsterIdx];
    if (monsterId == 0) {
        return;
    }
    BattleRender *battleRender = new BattleRender(monsterId);
    push(battleRender);
}

bool MapRender::checkOutOfMap(Character *player, int targetX, int targetY) {
    byte startX = movable_offset[mapId * 2];
    byte startY = movable_offset[mapId * 2 + 1];
    byte endX = startX + movable_size[mapId * 2];
    byte endY = startY + movable_size[mapId * 2 + 1];
    if (targetX < startX
        || targetY < startY
        || targetX > endX
        || targetY > endY) {
        entranceAnimation = ANIMATION_DURATION;
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
    int currentOnEntranceMapId = -1;
    int nextMapId = -1;
    int matchedIdx = -1;
    for (int i = 0; i < entrance_count[mapId]; i++) {
        byte entrance_x = entrances[mapId][i * 5];
        byte entrance_y = entrances[mapId][i * 5 + 1];
        if (player->x == entrance_x && player->y == entrance_y) {
            currentOnEntranceMapId = entrances[mapId][i * 5 + 4];
        }
        if (targetX == entrance_x && targetY == entrance_y) {
            matchedIdx = i;
            nextMapId = entrances[mapId][i * 5 + 4];
        }
    }
    if (matchedIdx != -1 && nextMapId != currentOnEntranceMapId) {
        entranceAnimation = ANIMATION_DURATION;
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
        player->setPos(entrances[mapId][matchedIdx * 5 + 2], entrances[mapId][matchedIdx * 5 + 3]);
        updateMap(nextMapId, 0, 0);
        pthread_mutex_unlock(&changeMapMutex);
        return true;
    }
    return false;
}

MapRender::~MapRender() {
    logd("MapRender", "delete");
}
