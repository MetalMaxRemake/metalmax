//
// Created by youngpark on 2023/5/12.
//

#ifndef metalmax_monster_data_h
#define metalmax_monster_data_h

#include <stdint.h>

#define monster_property_empty 255
#define monster_property_stride 14

extern const int monster_count;
extern const int monster_size[];
extern const unsigned char monster_group[930];

extern const uint8_t monster_distribution_world[256];
extern const uint8_t monster_distribution_4_samll_maps[112];
extern const uint8_t monster_properties[];

#endif //METALMAX_MONSTER_DATA_H
