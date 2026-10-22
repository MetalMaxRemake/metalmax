#ifndef metalmax_tank_data_h
#define metalmax_tank_data_h
#include <stdint.h>

namespace metalmax::tank_data {
constexpr unsigned k_tank_count=8;
constexpr unsigned k_equipment_count=6;
constexpr unsigned k_inventory_size=8;
enum tank_equipment_slot : uint8_t {
    k_main_gun_slot=0,
    k_secondary_gun_slot=1,
    k_special_slot=2,
    k_c_unit_slot=3,
    k_engine_slot=4,
    k_chassis_slot=5
};
struct initial_tank_t {
    uint16_t defense,max_defense,weight,sp;
    uint8_t equipment[k_equipment_count];
    uint8_t equipment_state,shells,map,x,y,slot;
};
initial_tank_t initial_tank(unsigned tank_id);
uint8_t defense_up_step();
uint8_t shells_up_step();
uint8_t max_shells();
uint8_t weapon_shell_capacity(unsigned index);
const char *tank_name(unsigned tank_id);
const char *equipment_slot_name(unsigned slot);
}

#endif //METALMAX_TANK_DATA_H
