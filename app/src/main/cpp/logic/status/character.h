//
// Created by youngpark on 2023/5/21.
//

#ifndef METALMAX_CHARACTER_H
#define METALMAX_CHARACTER_H

#include "../../global.h"

extern byte direct_up;
extern byte direct_down;
extern byte direct_left;
extern byte direct_right;

class Character {
public:
    Character();
    int name;
    bool godMode = false;
    int hp;
    int x, y;
    int renderX, renderY;
    bool steping;
    byte direct;//0-up 1-down 2-left 3-right
    int halfStep;
    int bitmapId;
    byte* bitmap[4][2];
    byte* currentBitmap;
    void tik();
    void setPos(int x, int y);
};

Character *getDefaultPlayer();

#endif //METALMAX_CHARACTER_H
