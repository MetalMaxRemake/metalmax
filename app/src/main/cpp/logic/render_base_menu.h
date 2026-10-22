//
// Created by parkyu on 2023/6/7.
//

#ifndef metalmax_render_base_menu_h
#define metalmax_render_base_menu_h

#include "render.h"

namespace metalmax {
class base_menu_render : public base_render {
private:
    uint8_t *select_icon;
protected:
    virtual void render_select_pos(uint8_t *screen_buffer, int x, int y) {
        int img_width = global_config::k_screen_width;
        int offset = y * img_width + x;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                int current_pos = i * img_width + j;
                screen_buffer[current_pos + offset] = select_icon[i * 8 + j];
            }
        }
    }
public:
    base_menu_render();
    ~base_menu_render();
};

} // namespace metalmax
#endif //METALMAX_RENDER_BASE_MENU_H
