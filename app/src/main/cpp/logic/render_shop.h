#ifndef metalmax_render_shop_h
#define metalmax_render_shop_h
#include "render_base_menu.h"
#include "../data/game_data.h"
namespace metalmax {
class shop_render : public base_menu_render {
public:
    shop_render(int type,int page);
    uint8_t *render(uint8_t *screen) override;
    void process_key_click(uint8_t direct,uint8_t function) override;
private:
    game_data::shop_data_t data={};
    int selected=0;
    int shop_type = 0, page = 0;
};
} // namespace metalmax
#endif
