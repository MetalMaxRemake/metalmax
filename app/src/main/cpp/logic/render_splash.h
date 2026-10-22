//
// Created by parkyu on 2023/5/13.
//

#ifndef metalmax_render_splash_h
#define metalmax_render_splash_h

#include "../global.h"
#include "render.h"

namespace metalmax {
class splash_render : public base_render {
private:
public:

    uint8_t *render(uint8_t *screen_buffer);

    void tik_logic();

    void process_key_click(uint8_t direct_key, uint8_t function_key);

    ~splash_render();

    int start_selection = 0;
};

} // namespace metalmax
#endif //METALMAX_RENDER_SPLASH_H
