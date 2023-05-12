//
// Created by parkyu on 2023/5/12.
//

#include "render_battle.h"
#include "../monster/monster.h"
#include "../opt/mem_opt.h"

void renderBackground(byte *screenBuffer) {
    __memset_aarch64(screenBuffer, 3, 256 * 256);
}

void renderHumanMenu(byte *screenBuffer) {
    __memset_aarch64(screenBuffer, 3, 256 * 256);
}

void renderCarMenu(byte *screenBuffer) {
    __memset_aarch64(screenBuffer, 3, 256 * 256);
}

void renderInfoMenu(byte *screenBuffer) {
    __memset_aarch64(screenBuffer, 3, 256 * 256);
}

int monsterIdx = 0;

byte * BattleRender::render(byte *screenBuffer) {
    renderBackground(screenBuffer);
    renderMonster(monsterIdx, 10,20, screenBuffer);
    return screenBuffer;
}

bool upPushed = false, downPushed = false;

bool BattleRender::processKey(byte directKey, byte functionKey) {
    if (directKey & up) {
        upPushed = true;
    } else if(upPushed) {
        upPushed = false;
        monsterIdx--;
    }
    if (directKey & down) {
        downPushed = true;
    } else if(downPushed) {
        downPushed = false;
        monsterIdx++;
    }
    if(functionKey & b) {
        pop();
    }
    return true;
}