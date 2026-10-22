//
// Created by youngpark on 2023/5/20.
//

#ifndef metalmax_bitmap_render_h
#define metalmax_bitmap_render_h

#include "../global.h"

namespace metalmax::bitmap_render {
    uint8_t *render_bitmap(const uint8_t *bitmap_buffer,
                           int bitmap_width, int bitmap_height,
                           int render_x, int render_y,
                           uint8_t *screen_buffer);

    uint8_t *render_bitmap_with_transport(const uint8_t *bitmap_buffer,
                                          int bitmap_width, int bitmap_height,
                                          int x, int y,
                                          uint8_t *screen_buffer);

    uint8_t *render_bitmap_with_src_scroll(const uint8_t *origin_data, uint8_t direct,
                                           int width, int height,
                                           int x, int y,
                                           uint8_t *result);
}
namespace bitmap_render = metalmax::bitmap_render;
#endif //METALMAX_BITMAP_RENDER_H
