//
// Created by youngpark on 2023/4/24.
//

#ifndef metalmax_map_h
#define metalmax_map_h

#include "../global.h"

namespace metalmax {
unsigned char *render_map(int render_y, int render_x, unsigned char *screen_buffer);
unsigned short get_tile_idx(int x, int y);
void refresh_current_map(int map_id);
void fill_one_tile_in_screen(int pos_x, int pos_y, int bmp_idx, uint8_t *result);

} // namespace metalmax
#endif //METALMAX_MAP_H
