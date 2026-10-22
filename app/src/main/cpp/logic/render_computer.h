//
// Created by parkyu on 2023/6/7.
//

#ifndef metalmax_render_computer_h
#define metalmax_render_computer_h

#include "render_base_menu.h"

namespace metalmax {
class computer_render : public base_menu_render {
private:
    int type;
public:
    computer_render(int type);
    uint8_t * render(uint8_t *screen_buffer);
    void tik_logic();
    void process_key_click(uint8_t direct_key, uint8_t function_key);
};

} // namespace metalmax
#endif //METALMAX_RENDER_COMPUTER_H
