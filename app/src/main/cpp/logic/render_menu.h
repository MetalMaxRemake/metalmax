//
// Created by parkyu on 2023/5/11.
//

#ifndef metalmax_render_menu_h
#define metalmax_render_menu_h

#include "render.h"
#include "render_base_menu.h"

#define map_menu 0
#define start_menu 1

namespace metalmax {
class menu_render : public base_menu_render {
private:
    int select_pos = 0;
    void process_selection();
public:
    menu_render(int menu_type);
    uint8_t * render(uint8_t *screen_buffer);
    void tik_logic();
    void process_key_click(uint8_t direct_key, uint8_t function_key);
};

} // namespace metalmax
#endif //METALMAX_RENDER_MENU_H
