//
// Created by youngpark on 2023/5/20.
//

#ifndef METALMAX_BITMAP_RENDER_H
#define METALMAX_BITMAP_RENDER_H

#include "../global.h"

namespace bitmap_render {
    uint8_t *render_bitmap(uint8_t *bitmap_buffer,
                           int bitmap_width, int bitmap_height,
                           int render_x, int render_y,
                           uint8_t *screen_buffer);

    uint8_t *render_bitmap_with_transport(uint8_t *bitmap_buffer,
                                          int bitmap_width, int bitmap_height,
                                          int x, int y,
                                          uint8_t *screen_buffer);

    uint8_t *render_bitmap_with_src_scroll(uint8_t *originData, uint8_t direct,
                                           int width, int height,
                                           int x, int y,
                                           uint8_t *result);
}

#endif //METALMAX_BITMAP_RENDER_H
