//
// Created by youngpark on 2023/5/11.
//

#ifndef metalmax_monster_h
#define metalmax_monster_h

#include "../global.h"

namespace metalmax {
enum monster_property_index {
    k_hp_property = 0,
    k_attack_property = 1,
    k_defense_property = 2,
    k_armor_property = 3,
    k_speed_property = 4,
    k_hit_rate_property = 5,
    k_level_property = 6,
    k_exp_property = 7,
    k_gold_property = 8,
    k_drop_property = 9,
    k_attr_property = 10,
    k_resis_property = 11,
    k_ability_property = 12,
    k_atk_mode_property = 13
};

uint8_t *render_monster(int monster_id, int raw_idx, int column_idx, uint8_t *result);
uint8_t get_monster_property(int monster_id, int type);

} // namespace metalmax
#endif //METALMAX_MONSTER_H
