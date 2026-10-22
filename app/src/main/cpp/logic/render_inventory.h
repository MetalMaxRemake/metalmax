#ifndef metalmax_render_inventory_h
#define metalmax_render_inventory_h
#include "render_base_menu.h"
namespace metalmax {
class inventory_render : public base_menu_render {
public:
    explicit inventory_render(bool equip_only=false):equipment(equip_only) {}
    uint8_t *render(uint8_t *screen) override;
    void process_key_click(uint8_t direct,uint8_t function) override;
private:
    int selected=0;
    bool equipment=false;
};
class status_render : public base_render {
public:
    uint8_t *render(uint8_t *screen) override;
    void process_key_click(uint8_t,uint8_t function) override { if (function) { pop(); } }
};
} // namespace metalmax
#endif
