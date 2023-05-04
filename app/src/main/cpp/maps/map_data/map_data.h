//
// Created by youngpark on 2023/5/3.
//

#ifndef METALMAX_MAP_DATA_H
#define METALMAX_MAP_DATA_H

#define MAP_COUNT 3

#include "map_00.h"
#include "map_01.h"
#include "map_02.h"
#include "map_03.h"
#include "map_04.h"
#include "map_05.h"
#include "map_06.h"
#include "map_07.h"
#include "map_08.h"
#include "map_09.h"

extern "C" unsigned char *map_data[MAP_COUNT];
extern "C" unsigned short map_size[MAP_COUNT * 2];

#endif //METALMAX_MAP_DATA_H
