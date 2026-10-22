#ifndef metalmax_game_state_h
#define metalmax_game_state_h
#include <stdint.h>
namespace metalmax {
namespace game_state {
constexpr unsigned k_inventory_size=32;
    constexpr unsigned k_player_inventory_size = 8;
    constexpr unsigned k_tank_count = 8;
    constexpr unsigned k_tank_equipment_count = 6;
    constexpr unsigned k_tank_inventory_size = 8;
    constexpr unsigned k_player_name_size = 16;
struct player {
    uint32_t experience;
    uint16_t hp,max_hp,attack,defense;
    uint8_t level,speed,active,equipment_state;
    uint8_t equipment[8];
    uint8_t inventory[k_player_inventory_size];
};
    struct tank {
        uint16_t defense, max_defense, weight, sp, max_sp;
        uint8_t map, x, y, direction, owned, riding, pulled, ok, slot, equipment_state, shells;
        uint8_t equipment[k_tank_equipment_count];
        uint8_t inventory[k_tank_inventory_size];
};
struct state {
    player players[3];
    tank tanks[k_tank_count];
    uint32_t money;
    uint8_t inventory[k_inventory_size];
    uint8_t events[32],treasures[12];
    uint8_t map,x,y,direction;
    int8_t riding_tank, pulled_tank;
    char player_names[3][k_player_name_size];
};
    static_assert(sizeof(player) == 32, "Player layout changed; update save version");
    static_assert(sizeof(tank) == 36, "Tank layout changed; update save version");
    static_assert(sizeof(state) == 520, "State layout changed; update save version");
state &get();
void new_game();

    const char *player_name(unsigned player);

    void set_player_name(unsigned player, const char *name);
bool event(unsigned id);
void set_event(unsigned id,bool value);
bool has_item(unsigned id);

    bool has_item(unsigned id, unsigned player);
bool add_item(unsigned id);

    bool add_item_to_player(unsigned id, unsigned player);
bool remove_item(unsigned slot);

    bool remove_item(unsigned slot, unsigned player);
bool equip(unsigned slot,unsigned player=0);
bool use_healing_item(unsigned slot,unsigned player=0);

    bool use_item(unsigned slot, unsigned player = 0);

    bool discard_item(unsigned slot, unsigned player = 0);

    unsigned free_item_slots();

    int equipment_slot(unsigned id);

    const char *equipment_slot_name(unsigned slot);

    bool can_equip(unsigned id, unsigned player);

    bool add_tank_item(unsigned id, unsigned tank_id);

    bool remove_tank_item(unsigned slot, unsigned tank_id);

    int tank_equipment_slot(unsigned id, unsigned tank_id);

    const char *tank_equipment_slot_name(unsigned slot);

    bool can_equip_tank(unsigned id, unsigned tank_id, unsigned slot);

    bool equip_tank(unsigned slot, unsigned tank_id, unsigned target_slot);

    uint16_t tank_attack(unsigned tank_id);

    uint16_t tank_defense(unsigned tank_id);

    uint16_t tank_capacity(unsigned tank_id);

    uint16_t tank_load(unsigned tank_id);

    void enter_tank(unsigned tank_id);

    void exit_tank();

    void pull_tank(unsigned tank_id);

    void unpull_tank();

    int tank_at(unsigned map, unsigned x, unsigned y);

    bool move_tank(unsigned tank_id, unsigned map, unsigned x, unsigned y, uint8_t direction);
void award(uint32_t experience,uint32_t money);
void set_save_directory(const char *directory);
bool save();
bool load();
bool has_save();
bool spend(uint32_t amount);
}
} // namespace metalmax
#endif
