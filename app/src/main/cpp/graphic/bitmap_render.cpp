//
// Created by youngpark on 2023/5/20.
//

#include <string.h>
#include "bitmap_render.h"
#include "palette_data.h"
#include "../logic/logic.h"

namespace bitmap_render {

    uint8_t *render_bitmap(uint8_t *bitmap_buffer,
                           int bitmap_width,
                           int bitmap_height,
                           int render_x,
                           int render_y,
                           uint8_t *screen_buffer) {
        int src_start_offset = render_x < 0 ? -render_x : 0;
        int dst_start_idx = render_x < 0 ? 0 : render_x;
        int src_copy_length =
                (dst_start_idx + bitmap_width) >= global_config::k_screen_width ? (
                        global_config::k_screen_width - dst_start_idx) : bitmap_width - src_start_offset;
        int dst_start_offset =
                (render_y * global_config::k_screen_width + render_x) + src_start_offset;
        for (int line_index = 0;
             line_index < bitmap_height;
             line_index++) {
            if (line_index + render_y < 0 ||
                line_index + render_y >= global_config::k_screen_height) {
                //this line is out of screen
                continue;
            }
            memcpy(
                    screen_buffer + dst_start_offset + line_index * global_config::k_screen_width,
                    bitmap_buffer + src_start_offset + line_index * bitmap_width,
                    src_copy_length);
        }
        return screen_buffer;
    }

    uint8_t *render_bitmap_with_transport(uint8_t *bitmap_buffer,
                                          int bitmap_width,
                                          int bitmap_height,
                                          int render_x,
                                          int render_y,
                                          uint8_t *screen_buffer) {
        int src_start_offset = render_x < 0 ? -render_x : 0;
        int dst_start_idx = render_x < 0 ? 0 : render_x;
        int src_copy_length =
                (dst_start_idx + bitmap_width) >= global_config::k_screen_width ? (
                        global_config::k_screen_width - dst_start_idx) : bitmap_width - src_start_offset;
        int dst_start_offset =
                (render_y * global_config::k_screen_width + render_x) + src_start_offset;
        for (int line_index = 0;
             line_index < bitmap_height;
             line_index++) {
            if (line_index + render_y < 0 ||
                line_index + render_y >= global_config::k_screen_height) {
                //this line is out of screen
                continue;
            }
            uint8_t *src = bitmap_buffer + src_start_offset + line_index * bitmap_width;
            uint8_t *dst =
                    screen_buffer + dst_start_offset + line_index * global_config::k_screen_width;
            for (int copy_idx = 0; copy_idx < src_copy_length; copy_idx++) {
                if (src[copy_idx] == palette::k_palette_transport) {
                    continue;
                }
                dst[copy_idx] = src[copy_idx];
            }
        }
        return screen_buffer;
    }

    uint8_t *render_bitmap_with_src_scroll(uint8_t *originData, uint8_t direct,
                                           int width, int height,
                                           int x, int y,
                                           uint8_t *result) {
        if (direct & up) {
            bitmap_render::render_bitmap_with_transport(originData,
                                                        width, height,
                                                        x, y,
                                                        result);
        }
        if (direct & down) {
            int realWidth = width;//16
            int startX = 0;
            if (x < 0) {
                realWidth += x;//16 + -3 = 13
                startX = -x;//3
            } else if (x > global_config::k_screen_width) {
                realWidth -= (x - global_config::k_screen_width);
            }

            int offset = y * global_config::k_screen_width + x;
            for (int i = 0; i < height; i++) {
                if ((15 - i) + y < 0 || (15 - i) + y > global_config::k_screen_height) {
                    continue;
                }
                for (int j = startX; j < startX + realWidth; j++) {
                    if (originData[i * width + j] == palette::k_palette_transport) {
                        //skip trans color!
                        continue;
                    } else {
                        result[(15 - i) * global_config::k_screen_width + offset + j] = originData[
                                i * width + j];
                    }
                }
            }
        }
        if (direct & left) {
            int offset = y * global_config::k_screen_width + x;
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    if (j + y < 0 || j + y > global_config::k_screen_height) {
                        continue;
                    }
                    if (i + x < 0 || i + x > (global_config::k_screen_width - 1)) {
                        continue;
                    }
                    if (originData[i * width + j] == palette::k_palette_transport) {
                        //skip trans color!
                        continue;
                    } else {
                        result[j * global_config::k_screen_width + offset + i] = originData[
                                i * width +
                                j];
                    }
                }
            }
        }
        if (direct & right) {
            int offset = y * global_config::k_screen_width + x;
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    if (j + y < 0 || j + y > global_config::k_screen_height) {
                        continue;
                    }
                    if ((width - 1 - i) + x < 0 ||
                        (width - 1 - i) + x > (global_config::k_screen_width - 1)) {
                        continue;
                    }
                    if (originData[i * width + j] == palette::k_palette_transport) {
                        //skip trans color!
                        continue;
                    } else {
                        result[j * global_config::k_screen_width + offset +
                               (width - 1 - i)] = originData[i * width + j];
                    }
                }
            }
        }
        return result;
    }
}
