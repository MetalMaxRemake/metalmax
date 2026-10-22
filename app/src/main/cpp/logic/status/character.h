//
// Created by youngpark on 2023/5/21.
//

#ifndef metalmax_character_h
#define metalmax_character_h

#include "../../global.h"

namespace metalmax {

    static constexpr uint8_t direct_up = 0;
    static constexpr uint8_t direct_down = 1;
    static constexpr uint8_t direct_left = 2;
    static constexpr uint8_t direct_right = 3;

    static constexpr int k_pixels_per_tick = 2;

    class character {
    public:
        character();

        int name = 0;
        bool god_mode = false;
        int hp = 80;
        //x is left or right, y is up or down
        int16_t x = 91, y = 232;
        //x is left or right, y is up or down
        int16_t render_x = 1456, render_y = 3712;
        bool steping = false;
        uint8_t direct;//0-up 1-down 2-left 3-right
        bool in_door = false;
        int half_step;
        int bitmap_id = 9;

        uint8_t *current_bitmap();

        void tik();

        void set_pos(int x, int y);

    };

    character *get_default_player();

} // namespace metalmax
#endif //METALMAX_CHARACTER_H
