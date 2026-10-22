#include "map_interaction.h"
#include "render.h"
#include "render_dialogue.h"
#include "render_shop.h"
#include "render_computer.h"
#include "status/character.h"
#include "status/game_state.h"
#include "../sprite/map_npcs.h"
#include "../computer/computer_data.h"
#include <cstdio>

using namespace metalmax;
static void facing(int &x,int &y) {
    auto *p=get_default_player();x=p->x;y=p->y;
    static const int dx[4]={0,0,-1,1},dy[4]={-1,1,0,0};x+=dx[p->direct];y+=dy[p->direct];
}
void open_computer(int function,int parameter) {
    if (function==20) { push(new dialogue_render(game_state::save() ? "战斗记录保存了。" : "保存失败。")); }
    else if (function==17) {
        if (game_state::load()) { auto &s=game_state::get();change_map(s.map,s.x,s.y);push(new dialogue_render("战斗记录读取了。")); }
        else { push(new dialogue_render("没有可读取的记录。")); }
    } else if (function==2 || function==15) {
        for (auto &p:game_state::get().players) { if (p.active) { p.hp=p.max_hp; } }
        push(new dialogue_render("生命恢复了。"));
    } else if (function==21) {
        auto &p=game_state::get().players[0];uint32_t next=p.level<99 ? game_data::experience(p.level+1) : p.experience;
        char text[100];snprintf(text,sizeof(text),"下次升级还需要 %u 经验。",next>p.experience ? next-p.experience : 0);push(new dialogue_render(text));
    } else { push(new dialogue_render("这项功能尚未开放。")); }
}
void interact_with_npc() {
    int x,y;facing(x,y);auto *npc=map_npcs::at(x,y);
    if (!npc) { push(new dialogue_render("这里没有可以交谈的人。"));return; }
    npc->face_player();unsigned mode=npc->data.talk1;
    if (mode&128) { open_computer((mode&124)>>2,npc->data.talk2); }
    else if (mode&64) {
        unsigned type=(mode&60)>>2;
        if (type==6) {
            const auto shop=game_data::shop(type,npc->data.talk2);
            uint32_t cost=shop.count ? game_data::shop_entry_at(shop.offset).item : 0;
            if (game_state::spend(cost)) {
                for (auto &p:game_state::get().players) { if (p.active) { p.hp=p.max_hp; } }push(new dialogue_render("休息后生命恢复了。"));
            } else { push(new dialogue_render("金钱不足。")); }
        } else if (type==2 || type==3 || type==11 || type==14) { push(new shop_render(type,npc->data.talk2)); }
        else { push(new dialogue_render("这项服务尚未开放。")); }
    } else if (mode&60) { push(new dialogue_render((mode&60)>>2,npc->data.talk2)); }
    else if (npc->data.talk2) { push(new dialogue_render(npc->data.talk2,npc)); }
    else { push(new dialogue_render("……")); }
}
void investigate_map() {
    int x,y;facing(x,y);int map=get_current_map();
    for (unsigned i=0;i<game_data::treasure_count && i<128;i++) {
        const auto &t=game_data::treasure(i);
        if (t.map!=map || t.x!=x || t.y!=y) { continue; }
        if (game_state::get().treasures[i/8]&(1<<(i%8))) { push(new dialogue_render("这里已经空了。"));return; }
        if (t.item>=222) { push(new dialogue_render("这件宝藏尚不能领取。"));return; }
        if (!game_state::add_item(t.item)) { push(new dialogue_render("物品栏已满。"));return; }
        game_state::get().treasures[i/8]|=1<<(i%8);
        char text[128];snprintf(text,sizeof(text),"找到了 %s。",game_data::item(t.item).name);push(new dialogue_render(text));return;
    }
    for (unsigned i=0;i<computer_count;i++) {
        if (computers[i*4]==x && computers[i*4+1]==y && computers[i*4+2]==map) {
            // ComputerEditor.type is a separate control-table index. It must
            // not be decoded as Sprite.talk1's service/interaction bitfield.
            push(new computer_render(computers[i*4+3]));return;
        }
    }
    if (map_npcs::at(x,y)) { interact_with_npc();return; }
    push(new dialogue_render("没有发现什么。"));
}
