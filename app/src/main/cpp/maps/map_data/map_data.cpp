//
// Created by youngpark on 2023/5/3.
//

#include "map_data.h"

unsigned char *byte_map_data[BYTE_MAP_COUNT] = {
        (unsigned char *) map_00, (unsigned char *) map_01, (unsigned char *) map_02
};

unsigned short *short_map_data[SHORT_MAP_COUNT] = {
        (unsigned short *) map_03, (unsigned short *) map_04,(unsigned short *) map_05,
        (unsigned short *) map_06,(unsigned short *) map_07,(unsigned short *) map_08,
        (unsigned short *) map_09
};

unsigned short map_size[MAP_COUNT * 2] = {
        256,256,
        15,24,
        16,16,
        16,12,
        16,19,
        16,15,
        16,12,
        10,12,
        11,14,
        11,14
};