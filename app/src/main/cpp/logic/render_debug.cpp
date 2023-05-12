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
#include "../audio/native_sles.h"

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
    renderAsciText(screenBuffer, "  ENTER BATTLE", 10, 50);
    renderAsciText(screenBuffer, "  NEXT MUSIC", 10, 58);
    renderAsciText(screenBuffer, "  PRE MUSIC", 10, 66);
    renderAsciText(screenBuffer, "  EXIT", 10, 74);
    renderSelectPos(screenBuffer, 13, 50 + (8 * selectPos));
}

void processSelection() {
    if (selectPos == 0) {
        pop();
        BattleRender *battleRender = new BattleRender;
        push(battleRender);
    } else if (selectPos == 1) {
        changeAudio(getAudioIdx() + 1);
    } else if (selectPos == 2) {
        changeAudio(getAudioIdx() - 1);
    } else if (selectPos == 3) {
        pop();
    }
}

byte *DebugRender::render(byte *screenBuffer) {
//    renderMonster(0, 10,20, screenBuffer);
    renderAsciText(screenBuffer, "METAL MAX 1 VERSION 0.3", 10, 10);
    renderAsciText(screenBuffer, "ARM AARCH64 SIMD", 10, 18);
    renderAsciText(screenBuffer, "OPENGL ES 2.0", 10, 26);
    renderZhText(screenBuffer, chinese_demo, 4, 10, 34);
    renderDebugMenu(screenBuffer);
    return screenBuffer;
}

bool upPushed = false, downPushed = false, aPushed = false;

bool DebugRender::processKey(byte directKey, byte functionKey) {
    if (directKey & up) {
        upPushed = true;
    } else if (upPushed) {
        upPushed = false;
        selectPos--;
        if (selectPos < 0) {
            selectPos = 0;
        }
    }
    if (directKey & down) {
        downPushed = true;
    } else if (downPushed) {
        downPushed = false;
        selectPos++;
        if (selectPos > 3) {
            selectPos = 3;
        }
    }
    if (functionKey & a) {
        aPushed = true;
    } else if (aPushed) {
        aPushed = false;
        processSelection();
    }
    if (functionKey & b) {
        pop();
    }
    return true;
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
