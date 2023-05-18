//
// Created by parkyu on 2023/5/12.
//

#include <cstdio>
#include "render_battle.h"
#include "../monster/monster.h"
#include "../opt/mem_opt.h"
#include "../charset/charsets.h"
#include "../audio/native_sound.h"

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
    if(enterBattle) {
        if(getAudioIdx() != 4) {
            changeAudio(4);
        }
        renderBackground(screenBuffer);
        renderMonster(monsterIdx, 10, 20, screenBuffer);
        char info[110];
        sprintf(info, "CURRENT MONSTER:%d", monsterIdx);
        renderAsciText(screenBuffer, R"(PUSH "UP" OR "DOWN" TO SWITCH)", 10, 138);
        renderAsciText(screenBuffer, R"(PRESS "B" TO EXIT)", 10, 146);
        renderAsciText(screenBuffer, info, 10, 154);
    } else {
        if(getAudioIdx() != 23) {
            changeAudio(23);
        }
        splashClk++;
        if(splashClk >= 60 * 2) {
            enterBattle = true;
        }
        if(splashClk % 3 == 0 && splashCount < 5) {
            splashCount++;
            __memset_aarch64(screenBuffer, 8, 256 * 256);
        }
    }
    return screenBuffer;
}

bool upPusheda = false, downPusheda = false;

bool BattleRender::processKey(byte directKey, byte functionKey) {
    if(!enterBattle) {
        return true;
    }
    if (directKey & up) {
        upPusheda = true;
    } else if(upPusheda) {
        upPusheda = false;
        monsterIdx--;
    }
    if (directKey & down) {
        downPusheda = true;
    } else if(downPusheda) {
        downPusheda = false;
        monsterIdx++;
    }
    if(functionKey & b) {
        pop();
    }
    return true;
}