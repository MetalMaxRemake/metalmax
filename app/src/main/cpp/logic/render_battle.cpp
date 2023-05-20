//
// Created by parkyu on 2023/5/12.
//

#include <cstdio>
#include "render_battle.h"
#include "../monster/monster.h"
#include "../opt/mem_opt.h"
#include "../charset/charsets.h"
#include "../audio/native_sound.h"
#include "../monster/monster_data/monster_data.h"
#include "../graphic/graphic_process.h"

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

BattleRender::BattleRender() {

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

void BattleRender::processKeyClick(byte directKey, byte functionKey) {
    if(!enterBattle) {
        return;
    }
    if (directKey & up) {
        monsterIdx--;
    }
    if (directKey & down) {
        monsterIdx++;
    }
    if(functionKey & b) {
        pop();
    }
}