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
public:
    BattleRender();
    byte * render(byte *screenBuffer);
    void tikLogic();
    void processKeyClick(byte directKey, byte functionKey);
};

#endif //METALMAX_RENDER_BATTLE_H
