#include "bitmap_render.h"
#include "palette_data.h"
#include "../logic/logic.h"
#include <algorithm>
#include <cstring>

namespace metalmax::bitmap_render {
    static uint8_t *
    blit(const uint8_t *source, int width, int height, int x, int y, uint8_t *screen,
         bool transparent) {
        if (!source || !screen || width <= 0 || height <= 0) { return screen; }
        int x0 = std::max(0, -x), y0 = std::max(0, -y);
        int x1 = std::min(width, static_cast<int>(global_config::k_screen_width) - x);
        int y1 = std::min(height, static_cast<int>(global_config::k_screen_height) - y);
        if (x0 >= x1 || y0 >= y1) { return screen; }
        for (int row = y0; row < y1; row++) {
            const uint8_t *src = source + row * width + x0;
            uint8_t *dst = screen + (row + y) * global_config::k_screen_width + x + x0;
            if (!transparent) { memcpy(dst, src, x1 - x0); }
            else {
                for (int col = 0; col < x1 - x0; col++) {
                    if (src[col] != palette::k_palette_transport) { dst[col] = src[col]; }
                }
            }
        }
        return screen;
    }

    uint8_t *render_bitmap(const uint8_t *source, int width, int height, int x, int y,
                           uint8_t *screen) {
        return blit(source, width, height, x, y, screen, false);
    }

    uint8_t *
    render_bitmap_with_transport(const uint8_t *source, int width, int height, int x, int y,
                                 uint8_t *screen) {
        return blit(source, width, height, x, y, screen, true);
    }

    uint8_t *
    render_bitmap_with_src_scroll(const uint8_t *source, uint8_t direction, int width, int height,
                                  int x, int y, uint8_t *screen) {
        if (!source || width <= 0 || height <= 0) { return screen; }
        for (int sy = 0; sy < height; sy++) {
            for (int sx = 0; sx < width; sx++) {
                uint8_t color = source[sy * width + sx];
                if (color == palette::k_palette_transport) { continue; }
                for (int dir = 0; dir < 4; dir++) {
                    if (!(direction & (1 << dir))) { continue; }
                    int dx = sx, dy = sy;
                    if (dir == 1) { dy = 15 - sy; }
                    if (dir == 2) {
                        dx = sy;
                        dy = sx;
                    }
                    if (dir == 3) {
                        dx = 15 - sy;
                        dy = sx;
                    }
                    dx += x;
                    dy += y;
                    if (dx >= 0 && dx < global_config::k_screen_width && dy >= 0 &&
                        dy < global_config::k_screen_height) {
                        screen[dy * global_config::k_screen_width + dx] = color;
                    }
                }
        }
        }
        return screen;
    }
}
