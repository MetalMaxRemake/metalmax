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
#include "../graphic/palette_data.h"

void renderBackground(byte *screenBuffer) {
    __memset_aarch64(screenBuffer, BLACK, 256 * 256);
}

void renderHumanMenu(byte *screenBuffer) {

}

void renderCarMenu(byte *screenBuffer) {

}

void renderInfoMenu(byte *screenBuffer) {

}

BattleRender::BattleRender(int monsterId) {
    monsterIdx = monsterId;
}

void BattleRender::tikLogic() {
    if(enterBattle) {
        if(getAudioIdx() != 4) {
            changeAudio(4);
        }
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
        }
    }
}

byte * BattleRender::render(byte *screenBuffer) {
    if(enterBattle) {
        renderBackground(screenBuffer);
        renderMonster(monsterIdx, 10, 20, screenBuffer);
        renderAsciText(screenBuffer, R"(PUSH "UP" OR "DOWN" TO SWITCH)", 10, 138);
        renderAsciText(screenBuffer, R"(PRESS "B" TO EXIT)", 10, 146);
        char info[110];
        sprintf(info, "CURRENT MONSTER:%d", monsterIdx);
        renderAsciText(screenBuffer, info, 10, 154);

        sprintf(info, "HP:%d ATTACK:%d DEFENSE:%d",
                getMonsterProperty(monsterIdx, HP),
                getMonsterProperty(monsterIdx, ATTACK),
                getMonsterProperty(monsterIdx, DEFENSE)
                );
        renderAsciText(screenBuffer, info, 10, 162);
    } else {
        if(splashClk % 3 == 0 && splashCount < 5) {
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
    if(monsterIdx < 0) {
        monsterIdx = 0;
    }
    monsterIdx %= monster_count;
    if(functionKey & b) {
        pop();
    }
}