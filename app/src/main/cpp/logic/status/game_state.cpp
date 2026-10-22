#include "game_state.h"
#include "character.h"
#include "../../data/game_data.h"
#include "../../maps/map_data/map_data.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <unistd.h>

namespace metalmax::game_state {
static state s_state;
static bool initialized=false;
static char save_path[1024];
static uint32_t crc(const uint8_t *bytes,unsigned length) {
    uint32_t value=0xffffffff;
    for (unsigned i=0;i<length;i++) {
        value^=bytes[i];
        for (int bit=0;bit<8;bit++) { value=(value>>1)^(0xedb88320u&-(value&1)); }
    }
    return ~value;
}
void new_game() {
    memset(&s_state,0,sizeof(s_state));
    s_state.money=game_data::initial_money();s_state.x=91;s_state.y=232;s_state.direction=1;
    unsigned slot=0;
    for (unsigned i=0;i<3;i++) {
        const auto &p=game_data::initial_player(i);auto &s=s_state.players[i];
        s.hp=p.hp;s.max_hp=p.max_hp;s.attack=p.attack;s.defense=p.defense;
        s.level=p.level;s.speed=p.speed;s.experience=p.experience;s.active=p.team!=0;
        memcpy(s.equipment,p.equipment,8);s.equipment_state=p.equipment_state;
        if (s.active) {
            for (unsigned j=0;j<8 && slot<k_inventory_size;j++) {
                if (p.inventory[j]) { s_state.inventory[slot++]=p.inventory[j]; }
            }
        }
    }
    initialized=true;
}
state &get() { if (!initialized) { new_game(); } return s_state; }
bool event(unsigned id) { return id<256 && (get().events[id/8]&(1<<(id%8))); }
void set_event(unsigned id,bool value) {
    if (id>=256) { return; }
    if (value) { get().events[id/8]|=1<<(id%8); } else { get().events[id/8]&=~(1<<(id%8)); }
}
bool has_item(unsigned id) {
    for (unsigned i=0;i<k_inventory_size;i++) { if (get().inventory[i]==id && id) { return true; } }
    for (const auto &p:get().players) {
        for (uint8_t item:p.equipment) { if (p.active && id && item==id) { return true; } }
    }
    return false;
}
bool add_item(unsigned id) {
    if (!id || id>=222) { return false; }
    for (unsigned i=0;i<k_inventory_size;i++) { if (!get().inventory[i]) { get().inventory[i]=id;return true; } }
    return false;
}
bool remove_item(unsigned slot) {
    if (slot>=k_inventory_size || !get().inventory[slot]) { return false; }
    for (unsigned i=slot;i+1<k_inventory_size;i++) { get().inventory[i]=get().inventory[i+1]; }
    get().inventory[k_inventory_size-1]=0;return true;
}
static int equipment_slot(unsigned id) {
    if (id>=41 && id<=64) { return 0; }
    if (id>=1 && id<=8) { return 2; }
    if (id>=9 && id<=16) { return 1; }
    if (id>=17 && id<=23) { return 3; }
    if (id>=24 && id<=28) { return 4; }
    if (id>=29 && id<=34) { return 5; }
    return -1;
}
bool equip(unsigned slot,unsigned player) {
    if (slot>=k_inventory_size || player>=3) { return false; }
    unsigned id=get().inventory[slot];if (!id || id>=222) { return false; }
    const auto &item=game_data::item(id);auto &p=get().players[player];
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
    if (old) { get().inventory[slot]=old; } else { remove_item(slot); }return true;
}
bool use_healing_item(unsigned slot,unsigned player) {
    if (slot>=k_inventory_size || player>=3) { return false; }
    unsigned id=get().inventory[slot];auto &p=get().players[player];
    // Initial remake rules; exact per-item ROM effect handlers remain archived.
    int amount=id==169 ? 30 : id==170 ? 60 : id==171 ? 120 : id==172 ? p.max_hp : 0;
    if (!amount || !p.active || p.hp<=0 || p.hp>=p.max_hp) { return false; }
    p.hp=std::min<int>(p.max_hp,p.hp+amount);remove_item(slot);return true;
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
    uint32_t header[4]={0x4d4d5356,2,sizeof(s_state),crc(reinterpret_cast<const uint8_t*>(&s_state),sizeof(s_state))};
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
    }
    for (uint8_t id:candidate.inventory) { if (id>=222) { return false; } }
    return true;
}
bool load() {
    if (!has_save()) { return false; }
    uint32_t header[4]={};state candidate={};uint8_t payload[244]={};
    FILE *f=fopen(save_path,"rb");if (!f) { return false; }
    bool ok=fread(header,sizeof(header),1,f)==1 && header[0]==0x4d4d5356
        && ((header[1]==1 && header[2]==244) || (header[1]==2 && header[2]==sizeof(s_state)));
    if (ok) { ok=fread(payload,header[2],1,f)==1 && fgetc(f)==EOF; }
    fclose(f);
    if (!ok || header[3]!=crc(payload,header[2])) { return false; }
    if (header[1]==2) { memcpy(&candidate,payload,sizeof(candidate)); }
    else {
        auto read32=[&](unsigned offset) { return uint32_t(payload[offset])|(uint32_t(payload[offset+1])<<8)|(uint32_t(payload[offset+2])<<16)|(uint32_t(payload[offset+3])<<24); };
        for (unsigned i=0;i<3;i++) {
            unsigned base=i*40;auto &p=candidate.players[i];
            for (unsigned j=0;j<4;j++) { if (read32(base+j*4)>65535) { return false; } }
            if (read32(base+16)>99 || read32(base+20)>255) { return false; }
            p.hp=read32(base);p.max_hp=read32(base+4);p.attack=read32(base+8);p.defense=read32(base+12);
            p.level=read32(base+16);p.speed=read32(base+20);p.experience=read32(base+24);p.active=payload[base+28];
            memcpy(p.equipment,payload+base+29,8);p.equipment_state=payload[base+37];
        }
        candidate.money=read32(120);
        for (unsigned i=0;i<32;i++) { if (payload[125+i*2]) { return false; }candidate.inventory[i]=payload[124+i*2]; }
        memcpy(candidate.events,payload+188,32);memcpy(candidate.treasures,payload+220,12);
        if (payload[237] || payload[239] || payload[241]) { return false; }
        candidate.map=payload[236];candidate.x=payload[238];candidate.y=payload[240];candidate.direction=payload[242];
    }
    if (!valid(candidate)) { return false; }
    s_state=candidate;initialized=true;get_default_player()->set_pos(s_state.x,s_state.y);get_default_player()->direct=s_state.direction;return true;
}
} // namespace metalmax::game_state
