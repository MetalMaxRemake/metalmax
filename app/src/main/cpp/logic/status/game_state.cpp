#include "game_state.h"
#include "character.h"
#include "../../data/game_data.h"
#include "../../maps/map_data/map_data.h"
#include "../../tank/tank_data.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <unistd.h>

namespace metalmax::game_state {
static state s_state;
static bool initialized=false;
static char save_path[1024];
    static constexpr uint32_t k_save_magic = 0x4d4d5356;
    static constexpr uint32_t k_save_version = 5;
static uint32_t crc(const uint8_t *bytes,unsigned length) {
    uint32_t value=0xffffffff;
    for (unsigned i=0;i<length;i++) {
        value^=bytes[i];
        for (int bit=0;bit<8;bit++) { value=(value>>1)^(0xedb88320u&-(value&1)); }
    }
    return ~value;
}

    static bool add_to_candidate(state &candidate, uint8_t id) {
        if (!id || id >= 222) { return false; }
        for (auto &p: candidate.players) {
            if (!p.active) { continue; }
            for (uint8_t &item: p.inventory) {
                if (!item) {
                    item = id;
                    return true;
                }
            }
        }
        for (uint8_t &item: candidate.inventory) {
            if (!item) {
                item = id;
                return true;
            }
        }
        return false;
    }

    static void migrate_flat_inventory(state &candidate, const uint8_t *items, unsigned count) {
        memset(candidate.inventory, 0, sizeof(candidate.inventory));
        for (unsigned i = 0; i < count; i++) { add_to_candidate(candidate, items[i]); }
    }

    static void init_tanks(state &target) {
        target.riding_tank = -1;
        target.pulled_tank = -1;
        for (unsigned i = 0; i < k_tank_count; i++) {
            auto &tank = target.tanks[i];
            memset(&tank, 0, sizeof(tank));
            const auto initial = tank_data::initial_tank(i);
            tank.defense = initial.defense;
            tank.max_defense = initial.max_defense;
            tank.weight = initial.weight;
            tank.sp = initial.sp;
            tank.max_sp = initial.sp;
            tank.map = initial.map;
            tank.x = initial.x;
            tank.y = initial.y;
            tank.direction = 1;
            tank.owned = initial.map < 240 ? 1 : 0;
            tank.ok = 1;
            tank.slot = initial.slot;
            tank.equipment_state = initial.equipment_state;
            tank.shells = initial.shells;
            memcpy(tank.equipment, initial.equipment, k_tank_equipment_count);
        }
    }

    static void init_player_names(state &target) {
        static const char *names[3] = {"HUNTER", "MECHANIC", "SOLDIER"};
        for (unsigned i = 0; i < 3; i++) {
            snprintf(target.player_names[i], k_player_name_size, "%s", names[i]);
        }
    }
void new_game() {
    memset(&s_state,0,sizeof(s_state));
    s_state.money=game_data::initial_money();s_state.x=91;s_state.y=232;s_state.direction=1;
    for (unsigned i=0;i<3;i++) {
        const auto &p=game_data::initial_player(i);auto &s=s_state.players[i];
        s.hp=p.hp;s.max_hp=p.max_hp;s.attack=p.attack;s.defense=p.defense;
        s.level=p.level;s.speed=p.speed;s.experience=p.experience;s.active=p.team!=0;
        memcpy(s.equipment,p.equipment,8);s.equipment_state=p.equipment_state;
        memcpy(s.inventory, p.inventory, k_player_inventory_size);
    }
    init_tanks(s_state);
    init_player_names(s_state);
    initialized=true;
}
state &get() { if (!initialized) { new_game(); } return s_state; }

    const char *player_name(unsigned player) {
        return player < 3 && get().player_names[player][0] ? get().player_names[player] : "";
    }

    void set_player_name(unsigned player, const char *name) {
        if (player >= 3) { return; }
        snprintf(get().player_names[player], k_player_name_size, "%s",
                 name && *name ? name : "HUNTER");
    }
bool event(unsigned id) { return id<256 && (get().events[id/8]&(1<<(id%8))); }
void set_event(unsigned id,bool value) {
    if (id>=256) { return; }
    if (value) { get().events[id/8]|=1<<(id%8); } else { get().events[id/8]&=~(1<<(id%8)); }
}
bool has_item(unsigned id) {
    if (!id) { return false; }
    for (const auto &p:get().players) {
        if (!p.active) { continue; }
        for (uint8_t item: p.inventory) { if (item == id) { return true; }}
        for (uint8_t item: p.equipment) { if (item == id) { return true; }}
    }
    for (unsigned i = 0; i < k_inventory_size; i++) {
        if (get().inventory[i] == id && id) { return true; }
    }
    for (const auto &tank: get().tanks) {
        if (!tank.owned) { continue; }
        for (uint8_t item: tank.inventory) { if (item == id) { return true; }}
        for (uint8_t item: tank.equipment) { if (item == id) { return true; }}
    }
    return false;
}

    bool has_item(unsigned id, unsigned player) {
        if (!id || player >= 3) { return false; }
        const auto &p = get().players[player];
        if (!p.active) { return false; }
        for (uint8_t item: p.inventory) { if (item == id) { return true; }}
        for (uint8_t item: p.equipment) { if (item == id) { return true; }}
        return false;
    }

    static bool compact_remove(uint8_t *items, unsigned count, unsigned slot) {
        if (slot >= count || !items[slot]) { return false; }
        for (unsigned i = slot; i + 1 < count; i++) { items[i] = items[i + 1]; }
        items[count - 1] = 0;
        return true;
    }

    bool add_item_to_player(unsigned id, unsigned player) {
        if (!id || id >= 222 || player >= 3 || !get().players[player].active) { return false; }
        for (uint8_t &item: get().players[player].inventory) {
            if (!item) {
                item = id;
                return true;
            }
    }
    return false;
}
bool add_item(unsigned id) {
    if (!id || id>=222) { return false; }
    const auto &item = game_data::item(id);
    if (item.kind >= 4 && item.kind <= 8) {
        if (get().riding_tank >= 0 && add_tank_item(id, get().riding_tank)) { return true; }
        for (unsigned tank_id = 0; tank_id < k_tank_count; tank_id++) {
            if (add_tank_item(id, tank_id)) { return true; }
        }
    }
    for (unsigned p = 0; p < 3; p++) { if (add_item_to_player(id, p)) { return true; }}
    for (unsigned i=0;i<k_inventory_size;i++) { if (!get().inventory[i]) { get().inventory[i]=id;return true; } }
    return false;
}
bool remove_item(unsigned slot) {
    return compact_remove(get().inventory, k_inventory_size, slot);
}

    bool remove_item(unsigned slot, unsigned player) {
        if (player >= 3) { return false; }
        return compact_remove(get().players[player].inventory, k_player_inventory_size, slot);
    }

    int equipment_slot(unsigned id) {
    if (id>=41 && id<=64) { return 0; }
    if (id>=1 && id<=8) { return 2; }
    if (id>=9 && id<=16) { return 1; }
    if (id>=17 && id<=23) { return 3; }
    if (id>=24 && id<=28) { return 4; }
    if (id>=29 && id<=34) { return 5; }
    return -1;
}

    const char *equipment_slot_name(unsigned slot) {
        static const char *names[] = {"武器", "衣服", "头部", "鞋子", "护甲", "手套", "装备7",
                                      "装备8"};
        return slot < 8 ? names[slot] : "";
    }

    bool can_equip(unsigned id, unsigned player) {
        if (!id || id >= 222 || player >= 3 || !get().players[player].active) { return false; }
        const auto &item = game_data::item(id);
        return equipment_slot(id) >= 0 && (item.can_equip & (0x80 >> player));
    }
bool equip(unsigned slot,unsigned player) {
    if (slot >= k_player_inventory_size || player >= 3) { return false; }
    auto &p = get().players[player];
    unsigned id = p.inventory[slot];
    if (!id || id >= 222) { return false; }
    const auto &item = game_data::item(id);
    const int target=equipment_slot(id);
    if (target<0 || !(item.can_equip&(0x80>>player)) || !p.active) { return false; }
    int actual_target=-1;
    for (unsigned i=0;i<8;i++) {
        if (p.equipment[i] && equipment_slot(p.equipment[i])==target) { actual_target=i;break; }
    }
    if (actual_target<0) {
        for (unsigned i=0;i<8;i++) { if (!p.equipment[i]) { actual_target=i;break; } }
    }
    if (actual_target<0) { return false; }
    unsigned old=p.equipment[actual_target];
    unsigned old_value=old<222 ? game_data::item(old).value : 0;
    p.equipment[actual_target]=id;p.equipment_state|=0x80>>actual_target;
    if (target==0) { p.attack=std::max(1,p.attack-static_cast<int>(old_value)+static_cast<int>(item.value)); }
    else { p.defense=std::max(0,p.defense-static_cast<int>(old_value)+static_cast<int>(item.value)); }
    if (old) { p.inventory[slot] = old; } else { remove_item(slot, player); }
    return true;
}
bool use_healing_item(unsigned slot,unsigned player) {
    if (slot >= k_player_inventory_size || player >= 3) { return false; }
    unsigned id = get().players[player].inventory[slot];
    auto &p = get().players[player];
    // Initial remake rules; exact per-item ROM effect handlers remain archived.
    int amount=id==169 ? 30 : id==170 ? 60 : id==171 ? 120 : id==172 ? p.max_hp : 0;
    if (!amount || !p.active || p.hp<=0 || p.hp>=p.max_hp) { return false; }
    p.hp = std::min<int>(p.max_hp, p.hp + amount);
    remove_item(slot, player);
    return true;
}

    bool use_item(unsigned slot, unsigned player) {
        if (player >= 3 || slot >= k_player_inventory_size ||
            !get().players[player].active) { return false; }
        unsigned id = get().players[player].inventory[slot];
        if (can_equip(id, player)) { return equip(slot, player); }
        return use_healing_item(slot, player);
    }

    bool discard_item(unsigned slot, unsigned player) {
        if (player >= 3 || slot >= k_player_inventory_size) { return false; }
        unsigned id = get().players[player].inventory[slot];
        if (!id || !game_data::item(id).discardable) { return false; }
        return remove_item(slot, player);
    }

    unsigned free_item_slots() {
        unsigned count = 0;
        for (const auto &p: get().players) {
            if (!p.active) { continue; }
            for (uint8_t item: p.inventory) { if (!item) { count++; }}
        }
        for (uint8_t item: get().inventory) { if (!item) { count++; }}
        for (const auto &tank: get().tanks) {
            if (!tank.owned) { continue; }
            for (uint8_t item: tank.inventory) { if (!item) { count++; }}
        }
        return count;
    }

    bool add_tank_item(unsigned id, unsigned tank_id) {
        if (!id || id >= 222 || tank_id >= k_tank_count ||
            !get().tanks[tank_id].owned) { return false; }
        for (uint8_t &item: get().tanks[tank_id].inventory) {
            if (!item) {
                item = id;
                return true;
            }
        }
        return false;
    }

    bool remove_tank_item(unsigned slot, unsigned tank_id) {
        if (tank_id >= k_tank_count) { return false; }
        return compact_remove(get().tanks[tank_id].inventory, k_tank_inventory_size, slot);
    }

    int tank_equipment_slot(unsigned id, unsigned tank_id) {
        const auto &item = game_data::item(id);
        if (!id || id >= 222) { return -1; }
        if (item.kind == 4) {
            uint8_t slot_mask = tank_id < k_tank_count ? get().tanks[tank_id].slot : 0xe0;
            if ((slot_mask & 0x80) && (item.can_equip & 0x80)) { return 0; }
            if ((slot_mask & 0x40) && (item.can_equip & 0x40)) { return 1; }
            if ((slot_mask & 0x20) && (item.can_equip & 0x20)) { return 2; }
            return -1;
        }
        if (item.kind == 5) { return 3; }
        if (item.kind == 6) { return 4; }
        if (item.kind == 7) { return 5; }
        return -1;
    }

    const char *tank_equipment_slot_name(unsigned slot) {
        return tank_data::equipment_slot_name(slot);
    }

    bool can_equip_tank(unsigned id, unsigned tank_id, unsigned slot) {
        if (tank_id >= k_tank_count || slot >= k_tank_equipment_count ||
            !get().tanks[tank_id].owned) { return false; }
        const auto &item = game_data::item(id);
        if (!id || id >= 222) { return false; }
        if (slot <= 2) {
            static const uint8_t hole_masks[3] = {0x80, 0x40, 0x20};
            return item.kind == 4 && (get().tanks[tank_id].slot & hole_masks[slot]) &&
                   (item.can_equip & hole_masks[slot]);
        }
        return (slot == 3 && item.kind == 5) || (slot == 4 && item.kind == 6) ||
               (slot == 5 && item.kind == 7);
    }

    bool equip_tank(unsigned slot, unsigned tank_id, unsigned target_slot) {
        if (tank_id >= k_tank_count || slot >= k_tank_inventory_size ||
            target_slot >= k_tank_equipment_count) { return false; }
        auto &tank = get().tanks[tank_id];
        unsigned id = tank.inventory[slot];
        if (!can_equip_tank(id, tank_id, target_slot)) { return false; }
        unsigned old = tank.equipment[target_slot];
        tank.equipment[target_slot] = id;
        tank.equipment_state |= 0x80 >> target_slot;
        if (old) { tank.inventory[slot] = old; } else { remove_tank_item(slot, tank_id); }
        return true;
    }

    uint16_t tank_attack(unsigned tank_id) {
        if (tank_id >= k_tank_count) { return 0; }
        uint32_t attack = 0;
        const auto &tank = get().tanks[tank_id];
        for (unsigned i = 0; i < 3; i++) {
            unsigned id = tank.equipment[i];
            if (id && (tank.equipment_state & (0x80 >> i))) { attack += game_data::item(id).value; }
        }
        return std::min<uint32_t>(65535, attack);
    }

    uint16_t tank_defense(unsigned tank_id) {
        if (tank_id >= k_tank_count) { return 0; }
        uint32_t defense = get().tanks[tank_id].defense;
        const auto &tank = get().tanks[tank_id];
        for (unsigned i = 0; i < k_tank_equipment_count; i++) {
            unsigned id = tank.equipment[i];
            if (id && (tank.equipment_state & (0x80 >> i))) {
                defense += game_data::item(id).value;
            }
        }
        return std::min<uint32_t>(65535, defense);
    }

    uint16_t tank_capacity(unsigned tank_id) {
        if (tank_id >= k_tank_count) { return 0; }
        unsigned engine = get().tanks[tank_id].equipment[4];
        return engine ? game_data::item(engine).value * 10 : 0;
    }

    uint16_t tank_load(unsigned tank_id) {
        if (tank_id >= k_tank_count) { return 0; }
        uint32_t load = get().tanks[tank_id].weight;
        for (uint8_t id: get().tanks[tank_id].equipment) {
            if (id) { load += game_data::item(id).can_equip; }
        }
        for (uint8_t id: get().tanks[tank_id].inventory) {
            if (id) { load += game_data::item(id).can_equip; }
        }
        return std::min<uint32_t>(65535, load);
    }

    void enter_tank(unsigned tank_id) {
        if (tank_id >= k_tank_count || !get().tanks[tank_id].owned) { return; }
        if (get().riding_tank >= 0 && get().riding_tank < static_cast<int>(k_tank_count)) {
            get().tanks[get().riding_tank].riding = 0;
        }
        get().riding_tank = tank_id;
        get().tanks[tank_id].riding = 1;
        get().tanks[tank_id].pulled = 0;
        if (get().pulled_tank == static_cast<int>(tank_id)) { get().pulled_tank = -1; }
        get().tanks[tank_id].map = get().map;
        get().tanks[tank_id].x = get().x;
        get().tanks[tank_id].y = get().y;
        get().tanks[tank_id].direction = get().direction;
    }

    void exit_tank() {
        if (get().riding_tank >= 0 && get().riding_tank < static_cast<int>(k_tank_count)) {
            auto &tank = get().tanks[get().riding_tank];
            tank.riding = 0;
            tank.map = get().map;
            tank.x = get().x;
            tank.y = get().y;
            tank.direction = get().direction;
        }
        get().riding_tank = -1;
    }

    void pull_tank(unsigned tank_id) {
        if (tank_id >= k_tank_count || !get().tanks[tank_id].owned) { return; }
        if (get().pulled_tank >= 0 && get().pulled_tank < static_cast<int>(k_tank_count)) {
            get().tanks[get().pulled_tank].pulled = 0;
        }
        get().pulled_tank = tank_id;
        get().tanks[tank_id].pulled = 1;
    }

    void unpull_tank() {
        if (get().pulled_tank >= 0 && get().pulled_tank < static_cast<int>(k_tank_count)) {
            get().tanks[get().pulled_tank].pulled = 0;
        }
        get().pulled_tank = -1;
    }

    int tank_at(unsigned map, unsigned x, unsigned y) {
        for (unsigned i = 0; i < k_tank_count; i++) {
            const auto &tank = get().tanks[i];
            if (tank.owned && !tank.riding && tank.map == map && tank.x == x && tank.y == y) {
                return i;
            }
        }
        return -1;
    }

    bool move_tank(unsigned tank_id, unsigned map, unsigned x, unsigned y, uint8_t direction) {
        if (tank_id >= k_tank_count || !get().tanks[tank_id].owned) { return false; }
        auto &tank = get().tanks[tank_id];
        tank.map = map;
        tank.x = x;
        tank.y = y;
        tank.direction = direction & 3;
        return true;
}
bool spend(uint32_t amount) {
    if (get().money<amount) { return false; }
    get().money-=amount;return true;
}
void award(uint32_t experience,uint32_t money) {
    get().money=std::min(0xffffffu,get().money+std::min(money,0xffffffu));
    for (auto &p:get().players) {
        if (!p.active || p.hp<=0) { continue; }
        p.experience=std::min(0xffffffu,p.experience+std::min(experience,0xffffffu));
        while (p.level<99 && p.experience>=game_data::experience(p.level+1)) {
            p.level++;p.max_hp=std::min<unsigned>(65535,p.max_hp+5);p.hp=std::min<int>(p.max_hp,p.hp+5);p.attack=std::min<unsigned>(65535,p.attack+2);p.defense=std::min<unsigned>(65535,p.defense+1);
        }
    }
}
void set_save_directory(const char *directory) {
    if (directory && *directory) { snprintf(save_path,sizeof(save_path),"%s/metalmax-v1.sav",directory); }
}
bool has_save() { return *save_path && access(save_path,R_OK)==0; }
bool save() {
    if (!*save_path) { return false; }
    auto *player=get_default_player();get().x=player->x;get().y=player->y;get().direction=player->direct;
    uint32_t header[4] = {k_save_magic, k_save_version, sizeof(s_state),
                          crc(reinterpret_cast<const uint8_t *>(&s_state), sizeof(s_state))};
    char temporary[1100];snprintf(temporary,sizeof(temporary),"%s.tmp",save_path);
    FILE *f=fopen(temporary,"wb");if (!f) { return false; }
    bool ok=fwrite(header,sizeof(header),1,f)==1 && fwrite(&s_state,sizeof(s_state),1,f)==1;
    if (fflush(f) || fsync(fileno(f))) { ok=false; }
    if (fclose(f)) { ok=false; }
    return ok && rename(temporary,save_path)==0;
}
static bool valid(const state &candidate) {
    if (candidate.map>=240 || candidate.direction>=4 || candidate.money>0xffffff
        || candidate.x>=map_size[candidate.map*2+1] || candidate.y>=map_size[candidate.map*2]) { return false; }
    for (const auto &p:candidate.players) {
        if (p.hp>p.max_hp || !p.max_hp || p.level<1 || p.level>99 || p.active>1) { return false; }
        for (uint8_t id:p.equipment) { if (id>=222) { return false; } }
        for (uint8_t id: p.inventory) { if (id >= 222) { return false; }}
    }
    for (uint8_t id:candidate.inventory) { if (id>=222) { return false; } }
    for (const auto &tank: candidate.tanks) {
        if (tank.map >= 240 && tank.map != 255) { return false; }
        if (tank.direction >= 4 || tank.owned > 1 || tank.riding > 1 || tank.pulled > 1 ||
            tank.ok > 1) {
            return false;
        }
        if (tank.sp > 9999 || tank.max_sp > 9999) { return false; }
        for (uint8_t id: tank.equipment) { if (id >= 222) { return false; }}
        for (uint8_t id: tank.inventory) { if (id >= 222) { return false; }}
    }
    if (candidate.riding_tank >= static_cast<int>(k_tank_count) ||
        candidate.pulled_tank >= static_cast<int>(k_tank_count)) { return false; }
    return true;
}
bool load() {
    if (!has_save()) { return false; }
    uint32_t header[4] = {};
    state candidate = {};
    uint8_t payload[sizeof(state)] = {};
    FILE *f=fopen(save_path,"rb");if (!f) { return false; }
    bool ok = fread(header, sizeof(header), 1, f) == 1 && header[0] == k_save_magic
              && ((header[1] == 1 && header[2] == 244) || (header[1] == 2 && header[2] == 156)
                  || (header[1] == 3 && header[2] == 180) || (header[1] == 4 && header[2] == 472)
                  || (header[1] == 5 && header[2] == sizeof(s_state)));
    if (ok) { ok=fread(payload,header[2],1,f)==1 && fgetc(f)==EOF; }
    fclose(f);
    if (!ok || header[3]!=crc(payload,header[2])) { return false; }
    if (header[1] == 5) { memcpy(&candidate, payload, sizeof(candidate)); }
    else if (header[1] == 4) {
        memcpy(&candidate, payload, 472);
        init_player_names(candidate);
    } else if (header[1] == 3) {
        memcpy(&candidate, payload, 180);
        init_tanks(candidate);
        init_player_names(candidate);
    } else if (header[1] == 2) {
        init_tanks(candidate);
        init_player_names(candidate);
        for (unsigned i = 0; i < 3; i++) {
            unsigned base = i * 24;
            auto &p = candidate.players[i];
            p.experience = uint32_t(payload[base]) | (uint32_t(payload[base + 1]) << 8) |
                           (uint32_t(payload[base + 2]) << 16) |
                           (uint32_t(payload[base + 3]) << 24);
            p.hp = uint16_t(payload[base + 4]) | (uint16_t(payload[base + 5]) << 8);
            p.max_hp = uint16_t(payload[base + 6]) | (uint16_t(payload[base + 7]) << 8);
            p.attack = uint16_t(payload[base + 8]) | (uint16_t(payload[base + 9]) << 8);
            p.defense = uint16_t(payload[base + 10]) | (uint16_t(payload[base + 11]) << 8);
            p.level = payload[base + 12];
            p.speed = payload[base + 13];
            p.active = payload[base + 14];
            p.equipment_state = payload[base + 15];
            memcpy(p.equipment, payload + base + 16, 8);
        }
        candidate.money = uint32_t(payload[72]) | (uint32_t(payload[73]) << 8) |
                          (uint32_t(payload[74]) << 16) | (uint32_t(payload[75]) << 24);
        migrate_flat_inventory(candidate, payload + 76, 32);
        memcpy(candidate.events, payload + 108, 32);
        memcpy(candidate.treasures, payload + 140, 12);
        candidate.map = payload[152];
        candidate.x = payload[153];
        candidate.y = payload[154];
        candidate.direction = payload[155];
    } else {
        init_tanks(candidate);
        init_player_names(candidate);
        auto read32=[&](unsigned offset) { return uint32_t(payload[offset])|(uint32_t(payload[offset+1])<<8)|(uint32_t(payload[offset+2])<<16)|(uint32_t(payload[offset+3])<<24); };
        uint8_t legacy_inventory[k_inventory_size] = {};
        for (unsigned i=0;i<3;i++) {
            unsigned base=i*40;auto &p=candidate.players[i];
            for (unsigned j=0;j<4;j++) { if (read32(base+j*4)>65535) { return false; } }
            if (read32(base+16)>99 || read32(base+20)>255) { return false; }
            p.hp=read32(base);p.max_hp=read32(base+4);p.attack=read32(base+8);p.defense=read32(base+12);
            p.level=read32(base+16);p.speed=read32(base+20);p.experience=read32(base+24);p.active=payload[base+28];
            memcpy(p.equipment,payload+base+29,8);p.equipment_state=payload[base+37];
        }
        candidate.money=read32(120);
        for (unsigned i = 0; i < 32; i++) {
            if (payload[125 + i * 2]) { return false; }
            legacy_inventory[i] = payload[124 + i * 2];
        }
        migrate_flat_inventory(candidate, legacy_inventory, 32);
        memcpy(candidate.events,payload+188,32);memcpy(candidate.treasures,payload+220,12);
        if (payload[237] || payload[239] || payload[241]) { return false; }
        candidate.map=payload[236];candidate.x=payload[238];candidate.y=payload[240];candidate.direction=payload[242];
    }
    if (!valid(candidate)) { return false; }
    s_state=candidate;initialized=true;get_default_player()->set_pos(s_state.x,s_state.y);get_default_player()->direct=s_state.direction;return true;
}
} // namespace metalmax::game_state
