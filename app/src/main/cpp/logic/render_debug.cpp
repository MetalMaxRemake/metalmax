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

const char chinese_demo[4] = {0, 1, 2, 3};

byte *selectIcon;

int selectPos = 0;

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

void renderDebugMenu(byte *screenBuffer) {
    int start = 70;
    renderAsciText(screenBuffer, "  ENTER BATTLE", 10, start + 8*0);
    renderAsciText(screenBuffer, "  >NEXT MUSIC ", 10, start + 8*1);
    renderAsciText(screenBuffer, "  <PRE MUSIC  ", 10, start + 8*2);
    renderAsciText(screenBuffer, "  >NEXT MAP   ", 10, start + 8*3);
    renderAsciText(screenBuffer, "  <PRE MAP    ", 10, start + 8*4);
    renderAsciText(screenBuffer, "  EXIT", 10, start + 8*5);
    renderSelectPos(screenBuffer, 13, start + (8 * selectPos));
}

void processSelection() {
    Character* player = getDefaultPlayer();
    if (selectPos == 0) {
        pop();
        BattleRender *battleRender = new BattleRender;
        push(battleRender);
    } else if (selectPos == 1) {
        changeAudio(getAudioIdx() + 1);
    } else if (selectPos == 2) {
        changeAudio(getAudioIdx() - 1);
    }else if (selectPos == 3) {
        //todo this is debug code
        int mapId = getCurrentMap() + 1;
        int map_height = map_size[mapId * 2];
        int map_width = map_size[mapId * 2 + 1];
        player->setPos(map_width / 2, map_height / 2);
        changeMap(mapId, 0, 0);
    }else if (selectPos == 4) {
        //todo this is debug code
        int mapId = getCurrentMap() - 1;
        int map_height = map_size[mapId * 2];
        int map_width = map_size[mapId * 2 + 1];
        player->setPos(map_width / 2, map_height / 2);
        changeMap(mapId, 0, 0);
    } else if (selectPos == 5) {
        pop();
    }
}

byte *DebugRender::render(byte *screenBuffer) {
    renderAsciText(screenBuffer, "METAL MAX 1 VERSION 0.5", 10, 10);
    renderZhText(screenBuffer, chinese_demo, 4, 10, 34);
    char info[30];
    sprintf(info, "MUSIC %d, MAP %d", getAudioIdx(), getCurrentMap());
    renderAsciText(screenBuffer, info, 10, 46);
    renderDebugMenu(screenBuffer);
    return screenBuffer;
}

void DebugRender::processKeyClick(byte directKey, byte functionKey) {
    renderCache(0);
    if (directKey & up) {
        selectPos--;
        if (selectPos < 0) {
            selectPos = 0;
        }
    }
    if (directKey & down) {
        selectPos++;
        if (selectPos > 5) {
            selectPos = 5;
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
    __memset_aarch64(selectIcon, 3, 8*8);
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
