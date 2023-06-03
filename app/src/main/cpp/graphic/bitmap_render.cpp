//
// Created by youngpark on 2023/5/20.
//

#include "bitmap_render.h"
#include "../opt/mem_opt.h"
#include "palette_data.h"

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

/**
 * warning: this func is slower than upper!
 * warning: trans bitmap MUST use this func!
 * @param originData
 * @param colorOffset
 * @param width
 * @param height
 * @param x
 * @param y
 * @param result
 * @return
 */
byte *renderBitmapColorOffset(byte *originData, int colorOffset,
                              int width, int height,
                              int x, int y,
                              byte *result) {
    int offset = y * 256 + x;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if(originData[i * width + j] + colorOffset == TRANSPARENT) {
                //skip trans color!
                continue;
            } else {
                result[i * 256 + offset + j] = originData[i * width + j] + colorOffset;
            }
        }
    }
    return result;
}