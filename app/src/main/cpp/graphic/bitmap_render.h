//
// Created by youngpark on 2023/5/20.
//

#ifndef METALMAX_BITMAP_RENDER_H
#define METALMAX_BITMAP_RENDER_H

#include "../global.h"

byte *renderBitmap(byte *originData,
                   int width, int height,
                   int x, int y,
                   byte *result);

#endif //METALMAX_BITMAP_RENDER_H
