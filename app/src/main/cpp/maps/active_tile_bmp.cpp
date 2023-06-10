//
// Created by parkyu on 2023/6/0.
//

#include "active_tile_bmp.h"
#include "../logic/status/character.h"
#include "../graphic/bitmap_render.h"
#include "../logic/logic.h"
#include "../graphic/palette_data.h"

unsigned short water_block_id[WATER_BLOCK_COUNT] = {
        13,15,31,35,39,47,85,112,154,190,
        207,218,277,281,295,305,344,398,545,547,
        576,626,664,677,679,683,784,917,922,929,
        1040,1453,1467,1497,1545,1556,1572,1591,1601,1761,
        1764,1792,1823,1829,1834,1837,1839,1864,1881,
};

unsigned short pure_water_block_id[32] = {
        13,15,31,35,39,47,85,112,154,218,
        344,398,545,547,677,679,683,1040,1497,1545,
        1556,1572,1761,1764,1792,1823,1829,1834,1837,1839,
        1864,1881,
};

bool isWater(unsigned short tileId) {
        for(int i = 0;i<WATER_BLOCK_COUNT;i++) {
                if (tileId == water_block_id[i]) {
                        return true;
                }
        }
        return false;
}

bool isPureWater(unsigned short tileId) {
        for(int i = 0;i<32;i++) {
                if (tileId == pure_water_block_id[i]) {
                        return true;
                }
        }
        return false;
}

/**
 * @param direct 复用一下方向键的方向吧
 * @param screenBuffer
 */
void renderWave(byte direct, byte status, byte *screenBuffer, int x, int y) {
        renderBitmapWithScroll(wave[status], direct,
                               16, 4,
                               x, y,
                               screenBuffer);
}

//upper side, need to scroll & use
//8 / 0
byte wave[3][16 * 4] = {
        {
                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                35, 8, 35, 35, 8, 8, 35, 8, 8, 35, 8, 8, 35, 8, 35, 8,
                35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
                35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
        },
        {
                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                8, 35, 8, 35, 8, 35, 8, 8, 35, 8, 8, 35, 8, 8, 8, 35,
                35, 8, 35, 35, 35, 8, 35, 35, 8, 35, 35, 8, 35, 8, 35, 8,
                35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
        },
        {
                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                8, 8, 8, 35, 8, 8, 8, 8, 8, 35, 8, 8, 35, 8, 8, 8,
                35, 8, 35, 8, 35, 8, 35, 35, 8, 8, 8, 8, 8, 8, 35, 8,
                35, 35, 8, 35, 35, 35, 35, 8, 8, 35, 8, 8, 35, 35, 8, 35,
        }
};

byte water[3][16 * 16] = {
        {
                11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 11, 11, 11, 11, 11, 10,
                11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 11, 11, 11, 11, 11, 10,
                10, 11, 11, 11, 11, 11, 11, 11, 10, 11, 10, 11, 11, 11, 10, 10,
                10, 10, 10, 11, 11, 11, 10, 10, 10, 10, 10, 11, 10, 10, 11, 11,
                11, 10, 10, 10, 10, 10, 10, 11, 11, 11, 10, 10, 11, 11, 11, 11,
                11, 11, 10, 11, 11, 10, 11, 11, 11, 11, 11, 10, 11, 11, 11, 11,
                11, 10, 10, 11, 11, 10, 11, 11, 11, 11, 11, 10, 10, 10, 11, 11,
                10, 11, 11, 10, 10, 10, 10, 10, 11, 11, 10, 10, 11, 11, 10, 10,
                11, 11, 11, 11, 10, 10, 11, 10, 10, 10, 10, 11, 11, 11, 11, 10,
                10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 10, 10, 11, 11, 11, 11,
                11, 10, 11, 11, 11, 11, 11, 11, 11, 11, 10, 11, 10, 10, 10, 11,
                10, 10, 10, 10, 11, 11, 11, 11, 11, 10, 10, 10, 10, 11, 11, 10,
                11, 11, 11, 10, 10, 11, 11, 10, 10, 10, 11, 11, 11, 11, 11, 11,
                11, 11, 11, 10, 10, 10, 10, 10, 11, 10, 11, 11, 11, 11, 11, 11,
                10, 10, 10, 10, 11, 11, 10, 10, 11, 11, 10, 10, 10, 10, 11, 11,
                10, 10, 11, 11, 11, 11, 11, 11, 10, 10, 10, 10, 11, 11, 10, 10,
        },
        {
                11, 11, 10, 10, 11, 11, 11, 11, 11, 11, 11, 10, 10, 11, 11, 11,
                11, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 10, 11, 10, 11, 11,
                10, 11, 11, 11, 10, 10, 11, 11, 11, 10, 10, 10, 10, 10, 11, 10,
                11, 11, 11, 11, 11, 11, 10, 10, 10, 10, 11, 11, 11, 10, 10, 11,
                11, 11, 11, 11, 11, 10, 11, 11, 10, 11, 11, 11, 11, 11, 10, 11,
                10, 11, 11, 11, 10, 10, 11, 11, 10, 10, 11, 11, 11, 11, 10, 10,
                11, 10, 10, 10, 11, 10, 10, 10, 10, 10, 10, 10, 11, 10, 10, 11,
                11, 11, 10, 11, 11, 10, 10, 11, 11, 11, 10, 10, 10, 10, 11, 11,
                11, 11, 11, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 10, 10, 11,
                10, 10, 11, 11, 10, 11, 11, 11, 11, 11, 11, 11, 11, 10, 11, 10,
                11, 11, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 11,
                11, 11, 11, 11, 11, 10, 10, 11, 11, 11, 10, 10, 10, 11, 11, 11,
                11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 11, 11, 10, 11, 11, 11,
                11, 11, 11, 10, 10, 10, 11, 11, 11, 10, 10, 11, 11, 10, 10, 10,
                10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 10, 11,
                11, 11, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 11, 11, 11,
        },
        {
                11, 11, 10, 10, 10, 11, 11, 11, 11, 11, 11, 10, 10, 11, 10, 11,
                10, 10, 11, 11, 11, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11,
                11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 11, 11, 11, 11, 10, 10,
                11, 11, 11, 11, 11, 11, 10, 10, 11, 11, 11, 11, 11, 11, 11, 10,
                10, 10, 11, 11, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 10,
                11, 11, 10, 10, 11, 11, 10, 10, 10, 10, 10, 11, 11, 11, 10, 10,
                11, 11, 11, 10, 11, 11, 10, 10, 11, 11, 10, 10, 10, 10, 10, 11,
                11, 11, 11, 11, 10, 10, 10, 11, 11, 11, 11, 11, 11, 10, 10, 10,
                10, 10, 10, 10, 11, 10, 11, 11, 11, 11, 11, 11, 11, 11, 10, 11,
                11, 11, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 10, 10, 10,
                11, 11, 11, 11, 11, 11, 10, 10, 11, 11, 10, 10, 10, 10, 11, 11,
                11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 11, 11, 11, 10, 11, 11,
                10, 11, 11, 11, 11, 10, 10, 10, 10, 11, 11, 11, 11, 11, 10, 10,
                11, 10, 10, 10, 10, 10, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10,
                11, 11, 11, 10, 11, 11, 11, 11, 11, 11, 11, 10, 11, 10, 11, 11,
                11, 11, 11, 10, 10, 11, 11, 11, 11, 11, 11, 10, 10, 10, 11, 11,
        }
};