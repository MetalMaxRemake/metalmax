#include "tank_data.h"
#include "../data/resource_decode.h"
#include <cstring>

namespace {
#include "tank_data/tank_initial.h"
#include "tank_data/tank_meta.h"
}

namespace metalmax::tank_data {
initial_tank_t initial_tank(unsigned tank_id) {
    initial_tank_t result={};
    if (tank_id>=k_tank_count) { return result; }
    const uint8_t *p=k_tank_initial+tank_id*20;
    memcpy(result.equipment,p,k_equipment_count);
    result.equipment_state=p[6];
    result.shells=p[7];
    result.defense=resource_decode::u16(p+8);
    result.max_defense=resource_decode::u16(p+10);
    result.weight=resource_decode::u16(p+12);
    result.sp=resource_decode::u16(p+14);
    result.map=p[16];
    result.x=p[17];
    result.y=p[18];
    result.slot=p[19];
    return result;
}

uint8_t defense_up_step() { return k_tank_meta[0]; }
uint8_t shells_up_step() { return k_tank_meta[1]; }
uint8_t max_shells() { return k_tank_meta[2]; }
uint8_t weapon_shell_capacity(unsigned index) { return index<8 ? k_tank_meta[3+index] : 0; }

const char *tank_name(unsigned tank_id) {
    static const char *names[]={"NO.1","NO.2","NO.3","NO.4","NO.5","NO.6","NO.7","NO.8"};
    return tank_id<k_tank_count ? names[tank_id] : "";
}

const char *equipment_slot_name(unsigned slot) {
    static const char *names[]={"主炮","副炮","S-E","C装置","发动机","底盘"};
    return slot<k_equipment_count ? names[slot] : "";
}
}
