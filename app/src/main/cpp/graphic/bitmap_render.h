//
// Created by youngpark on 2023/5/20.
//

#ifndef METALMAX_BITMAP_RENDER_H
#define METALMAX_BITMAP_RENDER_H

#include "../global.h"

uint8_t *renderBitmap(uint8_t *originData,
                   int width, int height,
                   int x, int y,
                   uint8_t *result);
uint8_t *renderBitmapWithTrans(uint8_t *originData,
                            int width, int height,
                            int x, int y,
                            uint8_t *result);
uint8_t *renderBitmapWithScroll(uint8_t *originData, uint8_t direct,
                             int width, int height,
                             int x, int y,
                             uint8_t *result);

#endif //METALMAX_BITMAP_RENDER_H
