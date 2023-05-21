//
// Created by youngpark on 2023/5/21.
//

#include <cstdlib>
#include "player.h"
#include "../../sprite/sprite.h"
#include "../../opt/mem_opt.h"
#include "../../graphic/graphic_process.h"

byte direct_up = 0;
byte direct_down = 1;
byte direct_left = 2;
byte direct_right = 3;

Character *warrior;

Character::Character() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            bitmap[i][j] = (byte *) malloc(16 * 16);
        }
    }
    __memcpy_aarch64_simd(bitmap[direct_up][0], getSpriteImage(0, 3, nullptr), 256);//up1
    __memcpy_aarch64_simd(bitmap[direct_up][1], getSpriteImage(0, 3, nullptr), 256);//up2
    __memcpy_aarch64_simd(bitmap[direct_down][0], getSpriteImage(0, 0, nullptr), 256);//down1
    __memcpy_aarch64_simd(bitmap[direct_down][1], getSpriteImage(0, 0, nullptr), 256);//down2
    __memcpy_aarch64_simd(bitmap[direct_left][0], getSpriteImage(0, 1, nullptr), 256);//left1
    __memcpy_aarch64_simd(bitmap[direct_left][1], getSpriteImage(0, 2, nullptr), 256);//left2
    __memcpy_aarch64_simd(bitmap[direct_right][0], getSpriteImage(0, 1, nullptr), 256);//right1
    __memcpy_aarch64_simd(bitmap[direct_right][1], getSpriteImage(0, 2, nullptr), 256);//right2

    bitmap[direct_up][1] = horizontalFlip(bitmap[direct_up][1], 16, 16);
    bitmap[direct_down][1] = horizontalFlip(bitmap[direct_down][1], 16, 16);
    bitmap[direct_right][0] = horizontalFlip(bitmap[direct_right][0], 16, 16);
    bitmap[direct_right][1] = horizontalFlip(bitmap[direct_right][1], 16, 16);

}

void Character::setPos(int nx, int ny) {
    x = nx;
    y = ny;
    renderX = x * 16;
    renderY = y * 16;
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