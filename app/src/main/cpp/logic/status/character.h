//
// Created by youngpark on 2023/5/21.
//

#ifndef METALMAX_CHARACTER_H
#define METALMAX_CHARACTER_H

#include "../../global.h"

extern uint8_t direct_up;
extern uint8_t direct_down;
extern uint8_t direct_left;
extern uint8_t direct_right;

class Character {
public:
    Character();
    int name;
    bool godMode = false;
    int hp;
    int x, y;
    int renderX, renderY;
    bool steping;
    uint8_t direct;//0-up 1-down 2-left 3-right
    bool inDoor = false;
    int halfStep;
    int bitmapId;
    uint8_t* bitmap[4][2];
    uint8_t* currentBitmap;
    void tik();
    void setPos(int x, int y);
};

Character *getDefaultPlayer();

#endif //METALMAX_CHARACTER_H
