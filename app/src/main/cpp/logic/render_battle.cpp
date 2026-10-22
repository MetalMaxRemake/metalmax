#include "render_battle.h"
#include "status/game_state.h"
#include "../data/game_data.h"
#include "../monster/monster.h"
#include "../charset/utf8_text.h"
#include "../audio/native_sound.h"
#include "../graphic/palette_data.h"
#include "../tank/tank_data.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace metalmax {
    namespace {
        const char *player_name(unsigned player) {
            static const char *names[] = {"猎人", "机械师", "士兵"};
            return player < 3 ? names[player] : "";
        }

        void render_item(uint8_t *screen, unsigned id, int x, int y, int width) {
            render_utf_8_text(screen, id ? game_data::item(id).name : "空", x, y, width, 16);
        }

        int first_alive_player() {
            for (unsigned i = 0; i < 3; i++) {
                const auto &player = game_state::get().players[i];
                if (player.active && player.hp > 0) { return i; }
            }
            return -1;
        }
    }

battle_render::battle_render(int id, bool browse, int event) : preview(browse), result_event(event) {
    monster_idx = (id % 131 + 131) % 131;
    enemy_hp = game_data::monster(monster_idx).hp;
    current_player = std::max(0, first_alive_player());
    selected_player = current_player;
    snprintf(message, sizeof(message), "遇到了 %s！", game_data::monster(monster_idx).name);
}

    bool battle_render::tank_battle() const {
        int tank_id = game_state::get().riding_tank;
        return tank_id >= 0 && tank_id < static_cast<int>(game_state::k_tank_count) &&
               game_state::get().tanks[tank_id].owned && game_state::get().tanks[tank_id].sp > 0;
    }

    bool battle_render::any_player_alive() const { return first_alive_player() >= 0; }

    int battle_render::next_player(int start) const {
        for (int offset = 1; offset <= 3; offset++) {
            int index = (start + offset) % 3;
            const auto &player = game_state::get().players[index];
            if (player.active && player.hp > 0) { return index; }
        }
        return first_alive_player();
    }

void battle_render::tik_logic() {
    if (!enter_battle && ++splash_clk >= 60) { enter_battle = true; }
    int track = enter_battle ? 4 : 23;
    if (get_audio_idx() != track) { change_audio(track); }
}

    void battle_render::on_focus() {}

void battle_render::finish(bool win) {
    if (resolved) { return; }
    resolved = true;
    victory = win;
    if (win) {
        const auto &monster = game_data::monster(monster_idx);
        game_state::award(monster.experience, monster.gold);
        if (result_event >= 0) { game_state::set_event(result_event, true); }
        snprintf(message, sizeof(message), "胜利！经验 %u  金钱 %u", monster.experience,
                 monster.gold);
    } else {
        game_state::get().money /= 2;
        for (auto &player: game_state::get().players) {
            if (player.active) { player.hp = player.max_hp; }
        }
        if (game_state::get().riding_tank >=
            0) { game_state::get().tanks[game_state::get().riding_tank].sp = 0; }
        snprintf(message, sizeof(message), "战斗失败，失去一半金钱。");
    }
}

    void battle_render::player_attack() {
        auto &player = game_state::get().players[current_player];
        const auto &enemy = game_data::monster(monster_idx);
        int damage = std::max(1, static_cast<int>(player.attack) - enemy.defense / 2) + rand() % 4;
        enemy_hp = std::max(0, enemy_hp - damage);
        snprintf(message, sizeof(message), "%s 攻击，造成 %d 伤害。", player_name(current_player),
                 damage);
    }

    void battle_render::tank_attack() {
        int tank_id = game_state::get().riding_tank;
        auto &tank = game_state::get().tanks[tank_id];
        const auto &enemy = game_data::monster(monster_idx);
        int base = std::max<int>(1, game_state::tank_attack(tank_id));
        int damage = std::max(1, base - enemy.defense / 2) + rand() % 6;
        if (tank.shells > 0) { tank.shells--; }
        enemy_hp = std::max(0, enemy_hp - damage);
        snprintf(message, sizeof(message), "%s 炮击，造成 %d 伤害。", tank_data::tank_name(tank_id),
                 damage);
    }

    void battle_render::enemy_turn(bool defending) {
        if (enemy_hp <= 0) {
            finish(true);
            return;
        }
        const auto &enemy = game_data::monster(monster_idx);
        if (tank_battle()) {
            int tank_id = game_state::get().riding_tank;
            auto &tank = game_state::get().tanks[tank_id];
            int damage = std::max(1, static_cast<int>(enemy.attack) -
                                     static_cast<int>(game_state::tank_defense(tank_id)) / 3) +
                         rand() % 5;
            if (defending) { damage = std::max(1, damage / 2); }
            if (tank.sp >= damage) {
                tank.sp -= damage;
                snprintf(message, sizeof(message), "%s 受到 %d SP 伤害。",
                         tank_data::tank_name(tank_id), damage);
            } else {
                int spill = damage - tank.sp;
                tank.sp = 0;
                int target = first_alive_player();
                if (target >= 0) {
                    auto &player = game_state::get().players[target];
                    player.hp = std::max(0, player.hp - spill);
                }
                snprintf(message, sizeof(message), "%s 装甲耗尽，乘员受到 %d 伤害。",
                         tank_data::tank_name(tank_id), spill);
            }
        } else {
            int target = current_player;
            if (target < 0 || target >= 3 || !game_state::get().players[target].active ||
                game_state::get().players[target].hp <= 0) { target = first_alive_player(); }
            if (target < 0) {
                finish(false);
                return;
            }
            auto &player = game_state::get().players[target];
            int damage =
                    std::max(1, static_cast<int>(enemy.attack) - player.defense / 2) + rand() % 4;
            if (defending) { damage = std::max(1, damage / 2); }
            player.hp = std::max(0, player.hp - damage);
            snprintf(message, sizeof(message), "%s 受到 %d 伤害。", player_name(target), damage);
    }
        if (!any_player_alive()) { finish(false); }
        current_player = std::max(0, next_player(current_player));
        selected_player = current_player;
    }

    void battle_render::turn(int action) {
        if (!any_player_alive()) {
            finish(false);
            return;
        }
        if (tank_battle()) {
            if (action == 0) { tank_attack(); }
            else if (action == 1) {
                snprintf(message, sizeof(message), "%s 防御。",
                         tank_data::tank_name(game_state::get().riding_tank));
            }
            else if (action == 4) {
                game_state::exit_tank();
                snprintf(message, sizeof(message), "离开战车，转为步战。");
                return;
            }
            if (enemy_hp <= 0) {
                finish(true);
                return;
            }
            enemy_turn(action == 1);
            return;
        }
        if (game_state::get().players[current_player].hp <= 0) {
            current_player = next_player(current_player);
        }
        if (action == 0) { player_attack(); }
        else if (action == 1) {
            snprintf(message, sizeof(message), "%s 防御。", player_name(current_player));
        }
        if (enemy_hp <= 0) {
            finish(true);
            return;
        }
        enemy_turn(action == 1);
    }

    void battle_render::use_player_item(bool equip_only) {
        if (selected_player >= 3 ||
            selected_item >= static_cast<int>(game_state::k_player_inventory_size)) { return; }
        unsigned id = game_state::get().players[selected_player].inventory[selected_item];
        if (!id) {
            snprintf(message, sizeof(message), "这里没有物品。");
            return;
        }
        if (equip_only) {
            if (game_state::equip(selected_item, selected_player)) {
                snprintf(message, sizeof(message), "%s 装备完成。", player_name(selected_player));
            }
            else { snprintf(message, sizeof(message), "不能装备这件物品。"); }
            mode = k_action_mode;
        return;
    }
        if (game_state::use_healing_item(selected_item, selected_player)) {
            snprintf(message, sizeof(message), "%s 使用了 %s。", player_name(selected_player),
                     game_data::item(id).name);
            mode = k_action_mode;
            enemy_turn(false);
        } else {
            snprintf(message, sizeof(message), "战斗中不能使用这件物品。");
        }
    }

    void battle_render::use_tank_item(bool equip_only) {
        int tank_id = game_state::get().riding_tank;
        if (tank_id < 0 ||
            selected_item >= static_cast<int>(game_state::k_tank_inventory_size)) { return; }
        unsigned id = game_state::get().tanks[tank_id].inventory[selected_item];
        if (!id) {
            snprintf(message, sizeof(message), "这里没有车载物品。");
            return;
        }
        if (equip_only) {
            if (game_state::equip_tank(selected_item, tank_id, selected_tank_slot)) {
                snprintf(message, sizeof(message), "%s 装备完成。", tank_data::tank_name(tank_id));
            } else {
                snprintf(message, sizeof(message), "不能装备到这个位置。");
            }
            mode = k_action_mode;
            return;
        }
        if (game_data::item(id).kind == 8) {
            auto &tank = game_state::get().tanks[tank_id];
            tank.sp = std::min<uint16_t>(tank.max_sp, static_cast<uint16_t>(tank.sp + 100));
            game_state::remove_tank_item(selected_item, tank_id);
            snprintf(message, sizeof(message), "%s 使用了 %s，SP恢复。",
                     tank_data::tank_name(tank_id), game_data::item(id).name);
            mode = k_action_mode;
            enemy_turn(false);
        } else {
            snprintf(message, sizeof(message), "战斗中不能使用这件车载物品。");
        }
}

uint8_t *battle_render::render(uint8_t *screen) {
    memset(screen, palette::k_palette_black, global_config::k_screen_buffer_size);
    if (!enter_battle) {
        if ((splash_clk / 4) % 2) { memset(screen, 8, global_config::k_screen_buffer_size); }
        return screen;
    }
    render_monster(monster_idx, 20, 24, screen);
    const auto &monster = game_data::monster(monster_idx);
    char line[180];
    render_utf_8_text(screen, monster.name, 180, 32, 255, 16);
    snprintf(line, sizeof(line), "敌人 HP %d/%d", enemy_hp, monster.hp);
    render_utf_8_text(screen, line, 180, 52, 255, 16);
    if (preview) {
        snprintf(line, sizeof(line), "攻击 %d  防御 %d", monster.attack, monster.defense);
        render_utf_8_text(screen, line, 180, 76, 255, 16);
        render_utf_8_text(screen, "上/下 浏览怪物  B 返回", 16, 224, 423, 16);
        return screen;
    }
    int y = 76;
    if (tank_battle()) {
        int tank_id = game_state::get().riding_tank;
        const auto &tank = game_state::get().tanks[tank_id];
        snprintf(line, sizeof(line), "%s SP %u/%u  炮弹 %u", tank_data::tank_name(tank_id), tank.sp,
                 tank.max_sp, tank.shells);
        render_utf_8_text(screen, line, 180, y, 255, 16);
        y += 18;
    }
    for (unsigned i = 0; i < 3; i++) {
        const auto &player = game_state::get().players[i];
        if (!player.active) { continue; }
        snprintf(line, sizeof(line), "%c %s HP %u/%u",
                 i == static_cast<unsigned>(current_player) ? '>' : ' ',
                 player_name(i), player.hp, player.max_hp);
        render_utf_8_text(screen, line, 180, y, 255, 16);
        y += 18;
    }
    memset(screen + 156 * global_config::k_screen_width, palette::k_palette_black,
           100 * global_config::k_screen_width);
    render_utf_8_text(screen, message, 16, 164, 423, 32);
    if (resolved) {
        render_utf_8_text(screen, "A/B 返回地图", 16, 232, 423, 16);
        return screen;
    }
    if (mode == k_action_mode) {
        const char *tank_actions[] = {"炮击", "防御", "车载", "车装", "下车", "逃跑"};
        const char *foot_actions[] = {"攻击", "防御", "物品", "装备", "逃跑"};
        const char **actions = tank_battle() ? tank_actions : foot_actions;
        int count = tank_battle() ? 6 : 5;
        for (int i = 0; i < count; i++) {
            snprintf(line, sizeof(line), "%c%s", i == selected ? '>' : ' ', actions[i]);
            render_utf_8_text(screen, line, 16 + (i % 3) * 140, 214 + (i / 3) * 18, 130, 16);
        }
        return screen;
    }
    if (mode == k_player_item_mode || mode == k_player_equip_mode) {
        selected_player = std::max(0, std::min(2, selected_player));
        render_utf_8_text(screen, mode == k_player_item_mode ? "战斗物品" : "战斗装备", 16, 206,
                          120, 16);
        snprintf(line, sizeof(line), "%s", player_name(selected_player));
        render_utf_8_text(screen, line, 140, 206, 100, 16);
        const auto &player = game_state::get().players[selected_player];
        for (unsigned i = 0; i < game_state::k_player_inventory_size; i++) {
            snprintf(line, sizeof(line), "%c",
                     i == static_cast<unsigned>(selected_item) ? '>' : ' ');
            render_utf_8_text(screen, line, 16 + (i % 4) * 110, 224 + (i / 4) * 16, 16, 16);
            render_item(screen, player.inventory[i], 32 + (i % 4) * 110, 224 + (i / 4) * 16, 94);
        }
        return screen;
    }
    int tank_id = game_state::get().riding_tank;
    const auto &tank = game_state::get().tanks[tank_id];
    render_utf_8_text(screen, mode == k_tank_item_mode ? "车载物品" : "战车装备", 16, 206, 120, 16);
    if (mode == k_tank_equip_mode) {
        snprintf(line, sizeof(line), "槽:%s  Y切换",
                 game_state::tank_equipment_slot_name(selected_tank_slot));
        render_utf_8_text(screen, line, 140, 206, 220, 16);
    }
    for (unsigned i = 0; i < game_state::k_tank_inventory_size; i++) {
        snprintf(line, sizeof(line), "%c", i == static_cast<unsigned>(selected_item) ? '>' : ' ');
        render_utf_8_text(screen, line, 16 + (i % 4) * 110, 224 + (i / 4) * 16, 16, 16);
        render_item(screen, tank.inventory[i], 32 + (i % 4) * 110, 224 + (i / 4) * 16, 94);
    }
    return screen;
}

void battle_render::process_key_click(uint8_t direct, uint8_t function) {
    if (!enter_battle) { return; }
    if (preview) {
        if (direct & up) { monster_idx = (monster_idx + 130) % 131; }
        if (direct & down) { monster_idx = (monster_idx + 1) % 131; }
        enemy_hp = game_data::monster(monster_idx).hp;
        if (function & b) { pop(); }
        return;
    }
    if (resolved) {
        if (function & (a | b)) { pop(); }
        return;
    }
    if (mode == k_action_mode) {
        int count = tank_battle() ? 6 : 5;
        if (direct & (left | up)) { selected = (selected + count - 1) % count; }
        if (direct & (right | down)) { selected = (selected + 1) % count; }
        if (function & b) {
            selected = count - 1;
            return;
        }
        if (!(function & a)) { return; }
        if (!tank_battle()) {
            if (selected == 2) {
                mode = k_player_item_mode;
                selected_item = 0;
                selected_player = current_player;
                return;
            }
            if (selected == 3) {
                mode = k_player_equip_mode;
                selected_item = 0;
                selected_player = current_player;
                return;
            }
            if (selected == 4) {
                if (result_event < 0 && rand() % 100 < 70) { pop(); }
                else { enemy_turn(true); }
                return;
            }
            turn(selected);
            return;
        }
        if (selected == 2) {
            mode = k_tank_item_mode;
            selected_item = 0;
            return;
        }
        if (selected == 3) {
            mode = k_tank_equip_mode;
            selected_item = 0;
            selected_tank_slot = 0;
            return;
        }
        if (selected == 5) {
            if (result_event < 0 && rand() % 100 < 70) { pop(); }
            else { enemy_turn(true); }
            return;
        }
        turn(selected);
        return;
    }
    if (function & b) {
        mode = k_action_mode;
        return;
    }
    if (direct & left) {
        if (mode == k_player_item_mode || mode == k_player_equip_mode) {
            selected_player = (selected_player + 2) % 3;
        }
    }
    if (direct & right) {
        if (mode == k_player_item_mode || mode == k_player_equip_mode) {
            selected_player = (selected_player + 1) % 3;
        }
    }
    if (direct & up) { selected_item = std::max(0, selected_item - 1); }
    if (direct & down) {
        int max_item = (mode == k_tank_item_mode || mode == k_tank_equip_mode) ?
                       game_state::k_tank_inventory_size - 1 : game_state::k_player_inventory_size -
                                                               1;
        selected_item = std::min(max_item, selected_item + 1);
    }
    if ((mode == k_tank_equip_mode) && (function & key_y)) {
        selected_tank_slot = (selected_tank_slot + 1) % game_state::k_tank_equipment_count;
    }
    if (!(function & a)) { return; }
    if (mode == k_player_item_mode) { use_player_item(false); }
    else if (mode == k_player_equip_mode) { use_player_item(true); }
    else if (mode == k_tank_item_mode) { use_tank_item(false); }
    else if (mode == k_tank_equip_mode) { use_tank_item(true); }
}
} // namespace metalmax
