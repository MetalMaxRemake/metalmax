//
// Created by youngpark on 2023/5/11.
//

#include "monster.h"
#include "monster_data/monster_data.h"
#include "../opt/mem_opt.h"


byte *renderMonster(int monsterId, int x, int y, byte *result) {
    int monster_width = monster_size[monsterId * 2 + 1];
    int monster_height = monster_size[monsterId * 2];

    int offset = x * 256 + y;
    for (int i = 0; i < monster_height; i++) {
        for (int j = 0; j < monster_width; j++) {
            result[i * 256 + j + offset] = monster_model[monsterId][i*monster_width + j];
        }
    }
    return result;
}