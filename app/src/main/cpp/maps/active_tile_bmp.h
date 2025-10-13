//
// Created by parkyu on 2023/6/10.
//

#ifndef METALMAX_ACTIVE_TILE_BMP_H
#define METALMAX_ACTIVE_TILE_BMP_H

#include "../global.h"

#define WATER_BLOCK_COUNT 49

extern unsigned short water_block_id[WATER_BLOCK_COUNT];

extern uint8_t wave[3][16 * 4];

extern uint8_t water[3][16 * 16];

extern void renderWave(uint8_t direct, uint8_t status, uint8_t *screen_buffer, int x, int y);

bool tile_is_water(unsigned short tileId);

bool tile_is_pure_water(unsigned short tileId);

#endif //METALMAX_ACTIVE_TILE_BMP_H
