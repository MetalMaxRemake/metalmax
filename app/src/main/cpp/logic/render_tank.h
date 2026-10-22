#ifndef metalmax_render_tank_h
#define metalmax_render_tank_h

#include "render_base_menu.h"

namespace metalmax {
    class tank_status_render : public base_menu_render {
    public:
        uint8_t *render(uint8_t *screen) override;

        void process_key_click(uint8_t direct, uint8_t function) override;

    private:
        unsigned selected_tank = 0;
    };

    class tank_equipment_render : public base_menu_render {
    public:
        uint8_t *render(uint8_t *screen) override;

        void process_key_click(uint8_t direct, uint8_t function) override;

    private:
        unsigned selected_tank = 0;
        unsigned selected_slot = 0;
        unsigned selected_item = 0;
    };
}

#endif
