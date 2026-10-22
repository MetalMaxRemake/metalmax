#ifndef metalmax_game_state_h
#define metalmax_game_state_h
#include <stdint.h>
namespace metalmax {
namespace game_state {
constexpr unsigned k_inventory_size=32;
struct player {
    uint32_t experience;
    uint16_t hp,max_hp,attack,defense;
    uint8_t level,speed,active,equipment_state;
    uint8_t equipment[8];
};
struct state {
    player players[3];
    uint32_t money;
    uint8_t inventory[k_inventory_size];
    uint8_t events[32],treasures[12];
    uint8_t map,x,y,direction;
};
static_assert(sizeof(player)==24,"Player layout changed; update save version");
static_assert(sizeof(state)==156,"State layout changed; update save version");
state &get();
void new_game();
bool event(unsigned id);
void set_event(unsigned id,bool value);
bool has_item(unsigned id);
bool add_item(unsigned id);
bool remove_item(unsigned slot);
bool equip(unsigned slot,unsigned player=0);
bool use_healing_item(unsigned slot,unsigned player=0);
void award(uint32_t experience,uint32_t money);
void set_save_directory(const char *directory);
bool save();
bool load();
bool has_save();
bool spend(uint32_t amount);
}
} // namespace metalmax
#endif
