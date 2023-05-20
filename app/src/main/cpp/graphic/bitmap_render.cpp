//
// Created by youngpark on 2023/5/20.
//

#include "bitmap_render.h"
#include "../opt/mem_opt.h"

/**
 *
 * @param originData
 * @param width
 * @param height
 * @param x - [0,width]
 * @param y - [0,height]
 * @param result
 * @return
 */
byte *renderBitmap(byte *originData,
                   int width, int height,
                   int x, int y,
                   byte *result) {
    int offset = y * 256 + x;
    for (int i = 0; i < height; i++) {
        __memcpy_aarch64_simd(result + i * 256 + offset,
                              originData + i * width,
                              width);
    }
    return result;
}