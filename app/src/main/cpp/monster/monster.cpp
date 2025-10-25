//
// Created by youngpark on 2023/5/11.
//

#include "monster.h"
#include "monster_data/monster_data.h"
#include "../graphic/graphic_process.h"
#include "../graphic/bitmap_render.h"

uint8_t *renderMonster(int monsterId, int rawIdx, int columnIdx, uint8_t *result) {
    int monster_width = monster_size[monsterId * 2 + 1];
    int monster_height = monster_size[monsterId * 2];
    return bitmap_render::render_bitmap(monster_model[monsterId],
                        monster_width, monster_height,
                        columnIdx, rawIdx, result);
}

//health	attack	defense	armor护甲	speed	hitRate	battleLevel	experience	gold	dropsItem	attribute	resistance	ability	attackMode
//stride = 14


uint8_t getMonsterProperty(int monsterId, int type) {
    return monster_properties[monsterId * MONSTER_PROPERTY_STRIDE + type];
}