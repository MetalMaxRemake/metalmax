//
// Created by youngpark on 2023/5/21.
//

#include <cstdlib>
#include <string.h>
#include "character.h"
#include "../../graphic/graphic_process.h"
#include "../../data/game_data.h"

namespace metalmax {

    static character *warrior;

    static int step_toward(int value, int target) {
        int delta = target - value;
        if (delta > k_pixels_per_tick) { return value + k_pixels_per_tick; }
        if (delta < -k_pixels_per_tick) { return value - k_pixels_per_tick; }
        return target;
    }

    character::character() {
        direct = direct_down;
        half_step = 0;
    }

    uint8_t *character::current_bitmap() {
        static uint8_t frame[256];
        // ROM bank1A:A127 identifies the on-foot player as image09. Its CHR
        // tiles all come from the fixed04/05 banks, independent of the map bank.
        game_data::decode_sprite(0, bitmap_id, direct, half_step, frame);
        return frame;
    }

//x is left or right, y is up or down
    void character::set_pos(int nx, int ny) {
        x = nx;
        y = ny;
        render_x = x * 16;
        render_y = y * 16;
        half_step = 0;
        steping = false;
    }

    void character::tik() {
        int target_x = x * 16;
        int target_y = y * 16;
        int dx = target_x - render_x;
        int dy = target_y - render_y;
        if (!dx && !dy) {
            half_step = 0;
            steping = false;
            return;
        }
        steping = true;
        if (dx) {
            direct = dx > 0 ? direct_right : direct_left;
            render_x = step_toward(render_x, target_x);
        } else {
            direct = dy > 0 ? direct_down : direct_up;
            render_y = step_toward(render_y, target_y);
        }
        int remaining = abs(target_x - render_x) + abs(target_y - render_y);
        half_step = remaining > 8 ? 1 : 0;
        steping = remaining != 0;
    }

    character *get_default_player() {
        if (warrior == nullptr) {
            warrior = new character;
            warrior->set_pos(91, 232);
        }
        return warrior;
    }
} // namespace metalmax
