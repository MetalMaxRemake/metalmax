#include "map_interaction.h"
#include "render.h"
#include "render_dialogue.h"
#include "render_shop.h"
#include "render_computer.h"
#include "render_tank.h"
#include "status/character.h"
#include "status/game_state.h"
#include "../sprite/map_npcs.h"
#include "../computer/computer_data.h"
#include "../computer/teleport_data.h"
#include "../tank/tank_data.h"
#include "../audio/native_sound.h"
#include <cstdio>
#include <cstdlib>

using namespace metalmax;
static void facing(int &x,int &y) {
    auto *p=get_default_player();x=p->x;y=p->y;
    static const int dx[4]={0,0,-1,1},dy[4]={-1,1,0,0};x+=dx[p->direct];y+=dy[p->direct];
}

static int first_owned_tank() {
    if (game_state::get().riding_tank >= 0) { return game_state::get().riding_tank; }
    for (unsigned i = 0; i < game_state::k_tank_count; i++) {
        if (game_state::get().tanks[i].owned) { return i; }
    }
    return -1;
}

static void recover_party() {
    for (auto &p: game_state::get().players) { if (p.active) { p.hp = p.max_hp; }}
}

static void repair_tanks(bool fill_shells) {
    for (auto &tank: game_state::get().tanks) {
        if (!tank.owned) { continue; }
        tank.ok = 1;
        tank.sp = tank.max_sp;
        if (fill_shells) { tank.shells = tank_data::max_shells(); }
    }
}

static bool teleport_destination(int index, int &map, int &x, int &y) {
    return index >= 0 && teleport_data::location(static_cast<unsigned>(index), map, x, y);
}

static void teleport_to(int index) {
    int map, x, y;
    if (!teleport_destination(index, map, x, y)) {
        push(new dialogue_render("传送失败。"));
        return;
    }
    change_map(map, x, y);
    push(new dialogue_render("传送完成。"));
}

static void run_gamble(bool free_play) {
    if (!free_play && !game_state::spend(10)) {
        push(new dialogue_render("金钱不足。"));
        return;
    }
    if (rand() % 2) {
        game_state::get().money = std::min(0xffffffu, game_state::get().money + 50);
        push(new dialogue_render("中奖了，得到50G。"));
    } else {
        push(new dialogue_render("很遗憾，没有中奖。"));
    }
}

static void customize_tank(bool engine) {
    int tank_id = first_owned_tank();
    if (tank_id < 0) {
        push(new dialogue_render("没有可改装的战车。"));
        return;
    }
    if (!game_state::spend(100)) {
        push(new dialogue_render("金钱不足。"));
        return;
    }
    auto &tank = game_state::get().tanks[tank_id];
    if (engine) {
        tank.max_sp = std::min<unsigned>(9999, tank.max_sp + 100);
        tank.sp = tank.max_sp;
    }
    else {
        tank.max_defense = std::min<unsigned>(9999, tank.max_defense + 10);
        tank.defense = std::min<unsigned>(tank.max_defense, tank.defense + 10);
    }
    push(new dialogue_render(engine ? "发动机改装完成。" : "底盘改装完成。"));
}
void open_computer(int function,int parameter) {
    bool map_computer = parameter < 0;
    if (function >= 0x40 && function <= 0x7c && (function & 3) == 0) {
        push(new shop_render((function - 0x40) / 4, map_computer ? 0 : parameter));
        return;
    }
    if (function >= 0x80 && function <= 0xe0 && (function & 3) == 0) {
        function = (function - 0x80) / 4;
    }
    if (map_computer && function >= 0 && function < 16) { teleport_to(function); }
    else if (function == 20) {
        push(new dialogue_render(game_state::save() ? "战斗记录保存了。" : "保存失败。"));
    }
    else if (function==17) {
        if (game_state::load()) { auto &s=game_state::get();change_map(s.map,s.x,s.y);push(new dialogue_render("战斗记录读取了。")); }
        else { push(new dialogue_render("没有可读取的记录。")); }
    } else if (function == 0) {
        repair_tanks(true);
        push(new dialogue_render("战车补给全满。"));
    } else if (function == 1) {
        push(new dialogue_render(game_state::save() ? "老爹保存了战斗记录。" : "保存失败。"));
    } else if (function==2 || function==15) {
        recover_party();
        push(new dialogue_render(function == 15 ? "按摩后身体轻松了。" : "生命恢复了。"));
    } else if (function == 3) {
        customize_tank(false);
    } else if (function == 4) {
        game_state::get().money = std::min(0xffffffu, game_state::get().money + 20);
        push(new dialogue_render("交易完成。"));
    } else if (function == 5) {
        push(new dialogue_render("赏金首情报已记录。"));
    } else if (function == 6) {
        push(new dialogue_render(game_state::save() ? "保管所记录了队伍状态。" : "保管失败。"));
    } else if (function == 7) {
        push(new dialogue_render("底盘改装可提升防御上限。"));
    } else if (function == 8) {
        customize_tank(true);
    } else if (function == 9) {
        repair_tanks(false);
        push(new dialogue_render("战车修理完成。"));
    } else if (function == 10) {
        game_state::spend(std::min<uint32_t>(game_state::get().money, 10));
        push(new dialogue_render("谢谢捐赠。"));
    } else if (function == 11) {
        push(new dialogue_render("激光炮系统已检查。"));
    } else if (function == 12) {
        int tank_id = first_owned_tank();
        if (tank_id >= 0) {
            game_state::get().tanks[tank_id].owned = 0;
            game_state::get().money += 500;
            push(new dialogue_render("战车卖出了。"));
        }
        else { push(new dialogue_render("没有可出售的战车。")); }
    } else if (function == 13) {
        teleport_to(map_computer ? 0 : parameter);
    } else if (function == 14) {
        repair_tanks(false);
        push(new dialogue_render("战车清洗完成。"));
    } else if (function == 16) {
        game_state::unpull_tank();
        push(new dialogue_render("出租战车已回收。"));
    } else if (function == 18) {
        run_gamble(false);
    } else if (function == 19) {
        run_gamble(true);
    } else if (function==21) {
        auto &p=game_state::get().players[0];uint32_t next=p.level<99 ? game_data::experience(p.level+1) : p.experience;
        char text[100];snprintf(text,sizeof(text),"下次升级还需要 %u 经验。",next>p.experience ? next-p.experience : 0);push(new dialogue_render(text));
    } else if (function == 22 || function == 23) {
        game_state::set_event(0xd0 + function, true);
        push(new dialogue_render("控制终端操作完成。"));
    } else if (function == 24) {
        game_state::set_event(0xb8, true);
        push(new dialogue_render("密码确认。"));
    } else if (function >= 0x10 && function < 0x40) {
        run_gamble((function & 1) == 0);
    } else {
        game_state::set_event(function & 255, true);
        push(new dialogue_render("计算机操作完成。"));
    }
}
void interact_with_npc() {
    int x,y;facing(x,y);auto *npc=map_npcs::at(x,y);
    if (!npc) { push(new dialogue_render("这里没有可以交谈的人。"));return; }
    npc->face_player();unsigned mode=npc->data.talk1;
    if (mode&128) { open_computer((mode&124)>>2,npc->data.talk2); }
    else if (mode&64) {
        unsigned type=(mode&60)>>2;
        if (type >= 0 && type <= 15 && type != 6) { push(new shop_render(type, npc->data.talk2)); }
        else if (type == 6) {
            const auto shop=game_data::shop(type,npc->data.talk2);
            uint32_t cost=shop.count ? game_data::shop_entry_at(shop.offset).item : 0;
            if (game_state::spend(cost)) {
                for (auto &p:game_state::get().players) { if (p.active) { p.hp=p.max_hp; } }push(new dialogue_render("休息后生命恢复了。"));
            } else { push(new dialogue_render("金钱不足。")); }
        } else { push(new shop_render(type, npc->data.talk2)); }
    } else if (mode&60) { push(new dialogue_render((mode&60)>>2,npc->data.talk2)); }
    else if (npc->data.talk2) { push(new dialogue_render(npc->data.talk2,npc)); }
    else { push(new dialogue_render("……")); }
}
void investigate_map() {
    int x,y;facing(x,y);int map=get_current_map();
    if (game_state::get().riding_tank >= 0) {
        int tank_id = game_state::get().riding_tank;
        game_state::exit_tank();
        char text[64];
        snprintf(text, sizeof(text), "离开了 %s。", tank_data::tank_name(tank_id));
        push(new dialogue_render(text));
        return;
    }
    int tank_id = game_state::tank_at(map, x, y);
    if (tank_id >= 0) {
        if (game_state::get().pulled_tank == tank_id) {
            game_state::unpull_tank();
            push(new dialogue_render("停止牵引。"));
        } else {
            game_state::pull_tank(tank_id);
            char text[64];
            snprintf(text, sizeof(text), "开始牵引 %s。", tank_data::tank_name(tank_id));
            push(new dialogue_render(text));
        }
        return;
    }
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
            open_computer(computers[i * 4 + 3], -1);
            return;
        }
    }
    if (map_npcs::at(x,y)) { interact_with_npc();return; }
    push(new dialogue_render("没有发现什么。"));
}
