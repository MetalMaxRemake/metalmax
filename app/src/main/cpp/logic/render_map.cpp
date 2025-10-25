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
#include "../graphic/palette_data.h"
#include "../audio/mm_sound.h"
#include "../graphic/native_graphic.h"
#include "../monster/monster_data/monster_data.h"
#include "render_menu.h"
#include "../maps/active_tile_bmp.h"

#define ANIMATION_DURATION 20

static const uint8_t HUMAN_PASS = 0b0001;
static const uint8_t CAR_PASS = 0b0010;

static volatile int map_render_x, map_render_y;
static volatile int mapId;

void MapRender::updateMap(int newMapId, int x, int y) {
    if (newMapId < 0) {
        newMapId = 0;
    }
    mapId = newMapId;
    mapId = mapId % 240;
    renderEffect(EFFECT_ENTER_ENTRANCE);
    refreshMusic();
    map_render_x = x * 16;
    map_render_y = y * 16;
    refreshCurrentMap(mapId);
}

MapRender::MapRender() {
    LOGD("MapRender", "new!");
    srand((unsigned) time(nullptr));
}

int MapRender::getMapId() {
    return mapId;
}

pthread_mutex_t changeMapMutex;

uint8_t *MapRender::render(uint8_t *screenBuffer) {
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
    //x is left or right, y is up or down
    map_render_x = player->renderX - global_config::k_screen_width / 2;
    map_render_y = player->renderY - global_config::k_screen_height / 2;
    screenBuffer = renderMap(map_render_x, map_render_y, screenBuffer);
    renderWater(screenBuffer);
    renderDoor(screenBuffer);
    renderSprite(screenBuffer);
    renderPlayers(screenBuffer, player);
    pthread_mutex_unlock(&changeMapMutex);
    return screenBuffer;
}

void MapRender::renderPlayers(uint8_t *screenBuffer, Character *player) const {
    bitmap_render::render_bitmap_with_transport(player->currentBitmap,
                                                16, 16,
                                                global_config::k_screen_width / 2,
                                                global_config::k_screen_height / 2,
                                                screenBuffer);
}

int water_status_map[4] = {1, 2, 3, 2};

uint8_t water_status = 0;
int water_status_clk = 0;

void MapRender::renderWater(uint8_t *screenBuffer) const {
    //convert render pos to tile pos
    int tile_start_index_x = map_render_x / 16;
    int tile_end_index_x = tile_start_index_x + global_config::k_tile_width;
    int tile_start_index_y = map_render_y / 16;
    int tile_end_index_y = tile_start_index_y + global_config::k_tile_height;
    for (int tile_index_x = tile_start_index_x - 1;
         tile_index_x < tile_end_index_x;
         tile_index_x++) {
        for (int tile_index_y = tile_start_index_y - 1;
             tile_index_y < tile_end_index_y;
             tile_index_y++) {
            if (tile_is_pure_water(getTileIdx(tile_index_x, tile_index_y))) {
                int render_x = tile_index_x * 16 - map_render_x;
                int render_y = tile_index_y * 16 - map_render_y;

                if (render_x > -16 && render_x < global_config::k_screen_width + 17 &&
                    render_y > -16 && render_y < global_config::k_screen_height + 17) {
                    bitmap_render::render_bitmap(water[water_status_map[water_status] - 1],
                                                 global_config::k_tile_size,
                                                 global_config::k_tile_size,
                                                 render_x,
                                                 render_y,
                                                 screenBuffer);
                    uint8_t direct = 0;
                    if (!tile_is_water(getTileIdx(tile_index_x, tile_index_y - 1))) {
                        direct |= up;
                    }
                    if (!tile_is_water(getTileIdx(tile_index_x, tile_index_y + 1))) {
                        direct |= down;
                    }
                    if (!tile_is_water(getTileIdx(tile_index_x - 1, tile_index_y))) {
                        direct |= left;
                    }
                    if (!tile_is_water(getTileIdx(tile_index_x + 1, tile_index_y))) {
                        direct |= right;
                    }
                    renderWave(direct,
                               water_status_map[water_status] - 1,
                               screenBuffer,
                               render_x,
                               render_y);
                }
            }
        }
    }
}

void MapRender::renderSprite(uint8_t *screenBuffer) const {
    int spriteCount = map_sprite_count[mapId];
    for (int i = 0; i < spriteCount; i++) {
        uint8_t x = map_sprite[mapId][i * 3];
        uint8_t y = map_sprite[mapId][i * 3 + 1] - 1;
        int renderY = y * 16 - map_render_y;
        int renderX = x * 16 - map_render_x;
        if (renderX > -16
            && renderX < global_config::k_screen_width + 16
            && renderY > -16
            && renderY < global_config::k_screen_height + 16) {
            uint8_t spriteBmpId = map_sprite[mapId][i * 3 + 2];
            bitmap_render::render_bitmap_with_transport(sprites[spriteBmpId],
                                                        global_config::k_tile_size,
                                                        global_config::k_tile_size,
                                                        renderX,
                                                        renderY,
                                                        screenBuffer);
        }
    }
}

void MapRender::renderDoor(uint8_t *screenBuffer) const {
    Character *player = getDefaultPlayer();
    if (player->inDoor) {
        int map_width = map_size[mapId * 2 + 1];
        int x = player->x;
        int y = player->y;
        unsigned short fillTileId = -1;
        for (int i = y - 2; i <= y + 2; i++) {
            unsigned short tileId = short_map_data[mapId][i * map_width + x];
            if (tileId == 37 || tileId == 128 || tileId == 162) {
                fillTileId = short_map_data[mapId][(i - 1) * map_width + x];
            }
        }
        for (int i = y - 1; i <= y + 1; i++) {
            unsigned short tileId = short_map_data[mapId][i * map_width + x];
            if (tileId == 37 || tileId == 72
                || tileId == 128 || tileId == 137
                || tileId == 162 || tileId == 193) {
                int renderY = i * 16;
                int renderX = x * 16;
                fillOneTileInScreen(renderX - map_render_x, renderY - map_render_y, fillTileId,
                                    screenBuffer);
            }
        }

    }
}

void MapRender::resetPalette() const {
    int *currentPalette = native_graphic::getPaletteBuffer();
    for (int i = 0; i < palette::palette_size; i++) {
        int dd = palette::palette_rgb[i];
        int b = (dd & 0x00FF0000) >> 16;
        int g = (dd & 0x0000FF00) >> 8;
        int r = (dd & 0x000000FF) >> 0;
        currentPalette[i] = 0xff000000 | (r << 16) | (g << 8) | b;
    }
    native_graphic::refreshPalette(currentPalette);
}

void MapRender::fadeIn() const {
    int *currentPalette = native_graphic::getPaletteBuffer();
    for (int i = 0; i < palette::palette_size; i++) {
        unsigned int origindd = palette::palette_rgb[i];
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
    native_graphic::refreshPalette(currentPalette);
}

void MapRender::fadeOut() const {
    int *currentPalette = native_graphic::getPaletteBuffer();
    for (int i = 0; i < palette::palette_size; i++) {
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
    native_graphic::refreshPalette(currentPalette);
}

void MapRender::tikLogic() {
    Character *player = getDefaultPlayer();
    player->tik();
    if (entranceAnimation) {
        isOriginPalette = false;
        entranceAnimation--;
    }
    water_status_clk++;
    if (water_status_clk > 60) {
        water_status_clk = 0;
        water_status++;
        water_status %= 4;
    }
}

void MapRender::onUnFocus() {
}

void MapRender::refreshMusic() const {
    uint8_t currentMusic = map_music[mapId] - 3;
    if (getAudioIdx() != currentMusic) {
        changeAudio(currentMusic);
    }
}

void MapRender::onFocus() {
    refreshMusic();
}

void MapRender::processKeyClick(uint8_t directKey, uint8_t functionKey) {
    renderEffect(EFFECT_PUSH_BUTTON);
    if (functionKey & a) {
        MenuRender *menuRender = new MenuRender(MAP_MENU);
        push(menuRender);
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
    if (functionKey & keyX) {
        DebugRender *debugRender = new DebugRender;
        push(debugRender);
    }
}

bool canHumanPass(int targetX, int targetY) {
    unsigned short tileIdx = getTileIdx(targetX, targetY);
    uint8_t currentTileFeature = feature[tileIdx];
    return currentTileFeature & HUMAN_PASS;
}

struct Pos {
    int mapId;
    int x, y;
};

Pos lastPosStack[10];
int lastPosStackTop = 0;

bool MapRender::processKey(uint8_t directKey, uint8_t functionKey) {
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
            triggerDoor(player);
        }
    } else {
        player->direct = nextDirect;
    }
    return functionKey == 0;
}

void MapRender::triggerDoor(Character *player) const {
    int map_width = map_size[mapId * 2 + 1];
    unsigned short tileId = short_map_data[mapId][getDefaultPlayer()->y * map_width +
                                                  getDefaultPlayer()->x];
    if ((tileId == 37 || tileId == 72
         || tileId == 128 || tileId == 137
         || tileId == 162 || tileId == 193
        )
        && (!player->inDoor)) {
        renderEffect(EFFECT_DOOR);
        player->inDoor = true;
    } else if (player->inDoor) {
        renderEffect(EFFECT_DOOR);
        player->inDoor = false;
    }
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
    uint8_t startX = movable_offset[mapId * 2];
    uint8_t startY = movable_offset[mapId * 2 + 1];
    uint8_t endX = startX + movable_size[mapId * 2];
    uint8_t endY = startY + movable_size[mapId * 2 + 1];
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
        uint8_t entrance_x = entrances[mapId][i * 5];
        uint8_t entrance_y = entrances[mapId][i * 5 + 1];
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
    LOGD("MapRender", "delete");
}
