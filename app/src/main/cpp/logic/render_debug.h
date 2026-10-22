//
// Created by parkyu on 2023/5/11.
//

#ifndef metalmax_render_debug_h
#define metalmax_render_debug_h

#include "logic.h"
#include "render.h"
#include "render_base_menu.h"

namespace metalmax {
class debug_render : public base_menu_render {
private:
    int select_pos = 0;
    void process_selection();
    void render_debug_menu(uint8_t *screen_buffer);
    void render_debug_info(uint8_t *screen_buffer) const;
public:
    debug_render();
    ~debug_render();
    uint8_t * render(uint8_t *screen_buffer);
    void tik_logic();
    void process_key_click(uint8_t direct_key, uint8_t function_key);
};

} // namespace metalmax
#endif //METALMAX_RENDER_DEBUG_H
