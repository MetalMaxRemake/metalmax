#ifndef metalmax_render_name_h
#define metalmax_render_name_h

#include "render_base_menu.h"

namespace metalmax {
    class name_render : public base_menu_render {
    public:
        uint8_t *render(uint8_t *screen) override;

        void process_key_click(uint8_t direct, uint8_t function) override;

    private:
        char name[9] = "HUNTER";
        int selected = 27;

        void start_opening();
    };
}

#endif
