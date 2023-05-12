//
// Created by parkyu on 2023/5/12.
//

#ifndef METALMAX_RENDER_BATTLE_H
#define METALMAX_RENDER_BATTLE_H

#include "logic.h"
#include "render.h"

class BattleRender : public BaseRender {
private:
    int pos;
public:
    byte * render(byte *screenBuffer);
    bool processKey(byte directKey, byte functionKey);
};

#endif //METALMAX_RENDER_BATTLE_H
