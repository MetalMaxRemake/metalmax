//
// Created by youngpark on 2023/5/11.
//

#include "monster.h"
#include "monster_0.h"
#include "../opt/mem_opt.h"

int monster_width = 96;
int monster_height = 128;

byte *renderMonster(int monsterId, int x, int y, byte *result) {
    int offset = x * 256 + y;
    for (int i = 0; i < monster_height; i++) {
        for (int j = 0; j < monster_width; j++) {
            result[i * 256 + j + offset] = monster_0[i][j];
        }
    }
    return result;
}