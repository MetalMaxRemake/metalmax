//
// Created by youngpark on 2023/5/11.
//

#ifndef METALMAX_MONSTER_H
#define METALMAX_MONSTER_H

#include "../global.h"

#define HP  0
#define ATTACK  1
#define DEFENSE 2
#define ARMOR   3
#define SPEED   4
#define HIT_RATE 5
#define LEVEL   6
#define EXP 7
#define GOLD    8
#define DROP    9
#define ATTR    10
#define RESIS   11
#define ABILITY 12
#define ATK_MODE    13

byte *renderMonster(int monsterId, int rawIdx, int columnIdx, byte *result);
byte getMonsterProperty(int monsterId, int type);

#endif //METALMAX_MONSTER_H
