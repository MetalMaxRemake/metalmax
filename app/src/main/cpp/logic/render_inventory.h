#ifndef metalmax_render_inventory_h
#define metalmax_render_inventory_h

#include "render_base_menu.h"

namespace metalmax {
    class inventory_render : public base_menu_render {
    public:
        explicit inventory_render(unsigned player = 0) : selected_player(player) {}

        uint8_t *render(uint8_t *screen) override;

        void process_key_click(uint8_t direct, uint8_t function) override;

    private:
        int selected = 0;
        unsigned selected_player = 0;
    };

    class equipment_render : public base_menu_render {
    public:
        explicit equipment_render(unsigned player = 0) : selected_player(player) {}

        uint8_t *render(uint8_t *screen) override;

        void process_key_click(uint8_t direct, uint8_t function) override;

    private:
        int selected = 0;
        unsigned selected_player = 0;
    };

    class status_render : public base_menu_render {
    public:
        uint8_t *render(uint8_t *screen) override;

        void process_key_click(uint8_t direct, uint8_t function) override;

    private:
        unsigned selected_player = 0;
    };
} // namespace metalmax
#endif
