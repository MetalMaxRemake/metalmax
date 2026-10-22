#include "map.h"
#include "map_data/map_data.h"
#include "tile_bmp.h"
#include "../graphic/bitmap_render.h"
#include "../logic/story_runtime.h"

namespace metalmax {
static int current_map = 0;

void refresh_current_map(int map) {
    if (map >= 0 && map < MAP_COUNT) { current_map = map; }
}

unsigned short get_tile_idx(int x, int y) {
    const int width = map_size[current_map * 2 + 1];
    const int height = map_size[current_map * 2];
    if (x < 0 || y < 0 || x >= width || y >= height) { return map_fill[current_map]; }
    return story_runtime::tile(current_map, x, y, short_map_data[current_map][y * width + x]);
}

void fill_one_tile_in_screen(int x, int y, int id, uint8_t *screen) {
    if (id >= 0 && id < 1904) {
        bitmap_render::render_bitmap(bitmaps[id], 16, 16, x, y, screen);
    }
}

uint8_t *render_map(int x, int y, uint8_t *screen) {
    const int first_x = x >= 0 ? x / 16 : (x - 15) / 16;
    const int first_y = y >= 0 ? y / 16 : (y - 15) / 16;
    for (int ty = first_y; ty * 16 < y + 256; ty++) {
        for (int tx = first_x; tx * 16 < x + 455; tx++) {
            fill_one_tile_in_screen(tx * 16 - x, ty * 16 - y, get_tile_idx(tx, ty), screen);
        }
    }
    return screen;
}
} // namespace metalmax
