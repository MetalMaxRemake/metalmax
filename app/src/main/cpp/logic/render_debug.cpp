//
// Created by parkyu on 2023/5/11.
//

#include <cstdlib>
#include "render_debug.h"
#include "logic.h"
#include "../charset/charsets.h"
#include "../monster/monster.h"
#include "../opt/mem_opt.h"
#include "render_battle.h"
#include "../audio/native_sound.h"
#include "../maps/map.h"
#include "status/character.h"
#include "../maps/map_data/map_data.h"
#include "../audio/mm_sound.h"
#include "../graphic/native_graphic.h"
#include "../graphic/palette_data.h"
#include "../monster/monster_data/monster_data.h"

const char chinese_demo[4] = {0, 1, 2, 3};

byte *selectIcon;

void renderSelectPos(byte *screenBuffer, int x, int y) {
    int img_width = 256;
    int offset = y * img_width + x;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int currentPos = i * img_width + j;
            screenBuffer[currentPos + offset] = selectIcon[i * 8 + j];
        }
    }
}

inline void DebugRender::renderDebugMenu(byte *screenBuffer) {
    int start = 70;
    int textHeight = 7;
    renderAsciText(screenBuffer, "  ENTER BATTLE   ", 10, start + textHeight * 0);
    renderAsciText(screenBuffer, "  >NEXT MUSIC    ", 10, start + textHeight * 1);
    renderAsciText(screenBuffer, "  <PRE MUSIC     ", 10, start + textHeight * 2);
    renderAsciText(screenBuffer, "  >NEXT MAP      ", 10, start + textHeight * 3);
    renderAsciText(screenBuffer, "  <PRE MAP       ", 10, start + textHeight * 4);
    renderAsciText(screenBuffer, "  CHANGE PALETTE ", 10, start + textHeight * 5);
    renderAsciText(screenBuffer, "  RESET PALETTE  ", 10, start + textHeight * 6);
    renderAsciText(screenBuffer, "  GOD MODE       ", 10, start + textHeight * 7);
    renderAsciText(screenBuffer, "  EXIT           ",10, start + textHeight * 8);
    renderSelectPos(screenBuffer, 13, start + (textHeight * selectPos));
}

inline void DebugRender::processSelection() {
    Character *player = getDefaultPlayer();
    if (selectPos == 0) {
        pop();
        BattleRender *battleRender = new BattleRender(rand() % monster_count);
        push(battleRender);
    } else if (selectPos == 1) {
        changeAudio(getAudioIdx() + 1);
    } else if (selectPos == 2) {
        changeAudio(getAudioIdx() - 1);
    } else if (selectPos == 3) {
        int mapId = getCurrentMap() + 1;
        int map_height = map_size[mapId * 2];
        int map_width = map_size[mapId * 2 + 1];
        player->setPos(map_width / 2, map_height / 2);
        changeMap(mapId, 0, 0);
    } else if (selectPos == 4) {
        int mapId = getCurrentMap() - 1;
        int map_height = map_size[mapId * 2];
        int map_width = map_size[mapId * 2 + 1];
        player->setPos(map_width / 2, map_height / 2);
        changeMap(mapId, 0, 0);
    } else if (selectPos == 5) {
        int *currentPalette = getCurrentPalette();
        for (int i = 0; i < 256; i++) {
            unsigned int dd = currentPalette[i];
            unsigned int a = (dd & 0xFF000000) >> 24;
            unsigned int r = (dd & 0x00FF0000) >> 16;
            unsigned int g = 0;
            unsigned int b = (dd & 0x000000FF) >> 0;
            currentPalette[i] = (a<<24) | (r << 16) | (g << 8) | b;
        }
        refreshPalette(currentPalette);
    } else if (selectPos == 6) {
        int *currentPalette = getCurrentPalette();
        for (int i = 0; i < 256; i++) {
            int dd = palette[i];
            int a = (dd & 0xFF000000) >> 24;
            int b = (dd & 0x00FF0000) >> 16;
            int g = (dd & 0x0000FF00) >> 8;
            int r = (dd & 0x000000FF) >> 0;
            currentPalette[i] = (a<<24) | (r << 16) | (g << 8) | b;
        }
        refreshPalette(currentPalette);
    } else if (selectPos == 7) {
        getDefaultPlayer()->godMode = !getDefaultPlayer()->godMode;
    } else if (selectPos == 8) {
        pop();
    }
}

byte *DebugRender::render(byte *screenBuffer) {
    renderDebugInfo(screenBuffer);
    renderDebugMenu(screenBuffer);
    return screenBuffer;
}

void DebugRender::renderDebugInfo(byte *screenBuffer) const {
    renderAsciText(screenBuffer, "METAL MAX 1 VERSION 0.7", 10, 10);
    renderZhText(screenBuffer, chinese_demo, 4, 10, 24);
    char info[30];
    sprintf(info, "MUSIC=%d, MAP=%d, GOD MODE=%s",
            getAudioIdx(),
            getCurrentMap(),
            getDefaultPlayer()->godMode?"T":"F");
    renderAsciText(screenBuffer, info, 10, 36);
    sprintf(info, "POS X=%d, Y=%d", getDefaultPlayer()->x, getDefaultPlayer()->y);
    renderAsciText(screenBuffer, info, 10, 44);
}

void DebugRender::processKeyClick(byte directKey, byte functionKey) {
    renderEffect(PUSH_BUTTON_EFFECT);
    if (directKey & up) {
        selectPos--;
        if (selectPos < 0) {
            selectPos = 8;
        }
    }
    if (directKey & down) {
        selectPos++;
        if (selectPos > 8) {
            selectPos = 0;
        }
    }
    if (functionKey & a) {
        processSelection();
    }
    if (functionKey & b) {
        pop();
    }
}

void DebugRender::tikLogic() {

}

DebugRender::DebugRender() {
    logd("DebugRender", "new");
    selectIcon = (byte *) malloc(8 * 8);
    __memset_aarch64(selectIcon, 3, 8 * 8);
    for (int i = 0; i < 4; i++) {
        __memset_aarch64(selectIcon + (i * 8), 8, i + 1);
    }
    for (int i = 4; i < 8; i++) {
        __memset_aarch64(selectIcon + (i * 8), 8, 8 - i);
    }
}

DebugRender::~DebugRender() {
    logd("DebugRender", "delete");
    free(selectIcon);
}
