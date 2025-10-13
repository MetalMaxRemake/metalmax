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

void renderBackground(uint8_t *screenBuffer) {
    __memset_aarch64(screenBuffer, palette::k_palette_black, global_config::k_screen_width * global_config::k_screen_height);
}

void renderHumanMenu(uint8_t *screenBuffer) {

}

void renderCarMenu(uint8_t *screenBuffer) {

}

void renderInfoMenu(uint8_t *screenBuffer) {

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

uint8_t * BattleRender::render(uint8_t *screenBuffer) {
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
            __memset_aarch64(screenBuffer, 8, global_config::k_screen_width * global_config::k_screen_height);
        }
    }
    return screenBuffer;
}

void BattleRender::processKeyClick(uint8_t directKey, uint8_t functionKey) {
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