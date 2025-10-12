//
// Created by parkyu on 2023/5/12.
//

#ifndef METALMAX_RENDER_BATTLE_H
#define METALMAX_RENDER_BATTLE_H

#include "logic.h"
#include "render.h"

class BattleRender : public BaseRender {
private:
    volatile bool enterBattle = false;
    volatile int splashClk = 0;
    volatile int splashCount = 0;
    int monsterIdx = 0;
public:
    BattleRender(int monsterId);
    uint8_t * render(uint8_t *screenBuffer);
    void tikLogic();
    void processKeyClick(uint8_t directKey, uint8_t functionKey);
};

#endif //METALMAX_RENDER_BATTLE_H
