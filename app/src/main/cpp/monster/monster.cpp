//
// Created by youngpark on 2023/5/11.
//

#include "monster.h"
#include "monster_data/monster_data.h"
#include "../opt/mem_opt.h"
#include "../graphic/graphic_process.h"
#include "../graphic/bitmap_render.h"

byte *renderMonster(int monsterId, int rawIdx, int columnIdx, byte *result) {
    int monster_width = monster_size[monsterId * 2 + 1];
    int monster_height = monster_size[monsterId * 2];
    return renderBitmap(monster_model[monsterId],
                        monster_width, monster_height,
                        columnIdx, rawIdx, result);
}