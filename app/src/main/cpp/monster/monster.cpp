#include "monster.h"
#include "monster_data/monster_data.h"
#include "../data/game_data.h"
namespace metalmax {
uint8_t *render_monster(int id,int y,int x,uint8_t *screen) {
    if (id>=0 && id<131) { game_data::draw_image(game_data::k_monster_image+id,x,y,screen); }
    return screen;
}
uint8_t get_monster_property(int id,int type) {
    return id>=0 && id<131 && type>=0 && type<14 ? monster_properties[id*14+type] : 0;
}
} // namespace metalmax
