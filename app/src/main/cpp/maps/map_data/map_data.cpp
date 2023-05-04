//
// Created by youngpark on 2023/5/3.
//

#include "map_data.h"

unsigned char *map_data[MAP_COUNT] = {
        (unsigned char *) map_00, (unsigned char *) map_01, (unsigned char *) map_02
};

unsigned short map_size[MAP_COUNT * 2] = {
        256,256,
        15,24,
        16,16
};