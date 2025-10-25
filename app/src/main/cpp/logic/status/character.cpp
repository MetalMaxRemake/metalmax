//
// Created by youngpark on 2023/5/21.
//

#include <cstdlib>
#include <string.h>
#include "character.h"
#include "../../sprite/sprite.h"
#include "../../graphic/graphic_process.h"

uint8_t direct_up = 0;
uint8_t direct_down = 1;
uint8_t direct_left = 2;
uint8_t direct_right = 3;

static Character *warrior;

Character::Character() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            bitmap[i][j] = (uint8_t *) malloc(16 * 16);
        }
    }
    memcpy(bitmap[direct_up][0], getSpriteImage(0, 3, nullptr), 256);//up1
    memcpy(bitmap[direct_up][1], getSpriteImage(0, 3, nullptr), 256);//up2
    memcpy(bitmap[direct_down][0], getSpriteImage(0, 0, nullptr), 256);//down1
    memcpy(bitmap[direct_down][1], getSpriteImage(0, 0, nullptr), 256);//down2
    memcpy(bitmap[direct_left][0], getSpriteImage(0, 1, nullptr), 256);//left1
    memcpy(bitmap[direct_left][1], getSpriteImage(0, 2, nullptr), 256);//left2
    memcpy(bitmap[direct_right][0], getSpriteImage(0, 1, nullptr), 256);//right1
    memcpy(bitmap[direct_right][1], getSpriteImage(0, 2, nullptr), 256);//right2

    bitmap[direct_up][1] = horizontalFlip(bitmap[direct_up][1], 16, 16);
    bitmap[direct_down][1] = horizontalFlip(bitmap[direct_down][1], 16, 16);
    bitmap[direct_right][0] = horizontalFlip(bitmap[direct_right][0], 16, 16);
    bitmap[direct_right][1] = horizontalFlip(bitmap[direct_right][1], 16, 16);
    direct = direct_down;
    halfStep = 0;
    currentBitmap = bitmap[direct][halfStep];
}

//x is left or right, y is up or down
void Character::setPos(int nx, int ny) {
    x = nx;
    y = ny;
    renderX = x * 16;
    renderY = y * 16;
    halfStep = 0;
}

void Character::tik() {
    steping = true;
    halfStep = 0;
    if (x * 16 > renderX) {
        renderX++;
        direct = direct_right;
        if (abs(x * 16 - renderX) > 8) {
            halfStep = 1;
        }
    } else if (x * 16 < renderX) {
        renderX--;
        direct = direct_left;
        if (abs(x * 16 - renderX) > 8) {
            halfStep = 1;
        }
    } else if (y * 16 > renderY) {
        renderY++;
        direct = direct_down;
        if (abs(y * 16 - renderY) > 8) {
            halfStep = 1;
        }
    } else if (y * 16 < renderY) {
        renderY--;
        direct = direct_up;
        if (abs(y * 16 - renderY) > 8) {
            halfStep = 1;
        }
    } else if (abs(x * 16 - renderX) <= 1 || abs(y * 16 - renderY) <= 1) {
        //这样写是为了两帧之间没有gap，防止卡顿
        steping = false;
    }
    currentBitmap = bitmap[direct][halfStep];
}

Character *getDefaultPlayer() {
    if (warrior == nullptr) {
        warrior = new Character;
        warrior->setPos(91, 232);
    }
    return warrior;
}