//
// Created by parkyu on 2023/6/10.
//

#ifndef metalmax_active_tile_bmp_h
#define metalmax_active_tile_bmp_h

#include "../global.h"

namespace metalmax {
    constexpr unsigned k_water_block_count = 49;

    extern unsigned short water_block_id[k_water_block_count];

extern uint8_t wave[3][16 * 4];

extern uint8_t water[3][16 * 16];

    void render_wave(uint8_t direct, uint8_t status, uint8_t *screen_buffer, int x, int y);

    bool tile_is_water(unsigned short tile_id);

    bool tile_is_pure_water(unsigned short tile_id);

} // namespace metalmax
#endif //METALMAX_ACTIVE_TILE_BMP_H
