#ifndef PALETTE_DATA_H
#define PALETTE_DATA_H

namespace palette {
    constexpr uint8_t k_palette_transport = 35;
    constexpr uint8_t k_palette_black = 3;
    constexpr uint8_t k_palette_white = 8;

    constexpr size_t palette_size = 256;
    extern int palette_rgb[256];
}

#endif