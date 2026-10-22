// Generated module-local resource dispatch.
#include "../data/game_data.h"
#include "../data/resource_decode.h"
#include <cstring>
#include <algorithm>
namespace {
#include "status/player_data/player_0.h"
#include "status/player_data/player_1.h"
#include "status/player_data/player_2.h"
#include "status/player_data/initial_money.h"
#include "status/player_data/experience.h"
}
namespace metalmax::game_data {player_data_t initial_player(unsigned i){player_data_t r={};const uint8_t *p=nullptr;switch(i){case 0:p=k_player_0;break;case 1:p=k_player_1;break;case 2:p=k_player_2;break; default:return r;}r.hp=resource_decode::u16(p);r.max_hp=resource_decode::u16(p+2);r.attack=resource_decode::u16(p+4);r.defense=resource_decode::u16(p+6);r.level=p[8];r.speed=p[9];r.experience=resource_decode::u24(p+10);r.team=p[13];memcpy(r.equipment,p+14,8);memcpy(r.inventory,p+22,8);r.equipment_state=p[30];return r;}uint32_t initial_money(){return resource_decode::u24(k_initial_money);}uint32_t experience(unsigned i){return i<100?resource_decode::u24(k_experience+i*3):0;}}
