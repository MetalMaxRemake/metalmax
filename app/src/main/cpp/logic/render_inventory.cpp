#include "render_inventory.h"
#include "render_dialogue.h"
#include "status/game_state.h"
#include "../charset/utf8_text.h"
#include "../graphic/palette_data.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace metalmax {
    namespace {
        const char *player_name(unsigned player) {
            static const char *names[] = {"猎人", "机械师", "士兵"};
            return player < 3 ? names[player] : "";
        }

        int first_active_player() {
            for (unsigned i = 0; i < 3; i++) {
                if (game_state::get().players[i].active) { return i; }
            }
            return -1;
        }

        unsigned normalize_player(unsigned player) {
            if (player < 3 && game_state::get().players[player].active) { return player; }
            int first = first_active_player();
            return first >= 0 ? static_cast<unsigned>(first) : 0;
        }

        unsigned step_player(unsigned player, int direction) {
            if (first_active_player() < 0) { return 0; }
            int current = static_cast<int>(normalize_player(player));
            for (int i = 0; i < 3; i++) {
                current = (current + direction + 3) % 3;
                if (game_state::get().players[current].active) { return static_cast<unsigned>(current); }
            }
            return static_cast<unsigned>(current);
        }

        void clear_panel(uint8_t *screen) {
            memset(screen + 32 * global_config::k_screen_width, palette::k_palette_black,
                   224 * global_config::k_screen_width);
        }

        void render_header(uint8_t *screen, const char *title, unsigned player, int y = 40) {
            char line[180];
            snprintf(line, sizeof(line), "%s  %s  金钱 %u", title, player_name(player),
                     game_state::get().money);
            render_utf_8_text(screen, line, 16, y, 423, 16);
        }

        void render_item_name(uint8_t *screen, unsigned id, int x, int y, int width) {
            render_utf_8_text(screen, id ? game_data::item(id).name : "空", x, y, width, 16);
        }

        bool selected_item_exists(unsigned player, unsigned slot) {
            return player < 3 && slot < game_state::k_player_inventory_size &&
                   game_state::get().players[player].inventory[slot];
        }
    }

uint8_t *inventory_render::render(uint8_t *screen) {
    clear_panel(screen);
    if (first_active_player() < 0) {
        render_utf_8_text(screen, "当前没有已入队角色。", 16, 48, 423, 16);
        return screen;
    }
    selected_player = normalize_player(selected_player);
    render_header(screen, "物品", selected_player);
    render_utf_8_text(screen, "←/→ 队员  A 使用/装备  Y 丢弃  B 返回", 16, 56, 423, 16);
    const auto &player = game_state::get().players[selected_player];
    for (unsigned i = 0; i < game_state::k_player_inventory_size; i++) {
        unsigned id = player.inventory[i];
        char line[160];
        snprintf(line, sizeof(line), "%c %u  ", i == static_cast<unsigned>(selected) ? '>' : ' ',
                 i + 1);
        render_utf_8_text(screen, line, 16, 80 + i * 18, 56, 16);
        render_item_name(screen, id, 72, 80 + i * 18, 180);
        if (id) {
            const auto &item = game_data::item(id);
            const char *kind = game_state::can_equip(id, selected_player) ? "装备" :
                               item.kind == 3 ? "道具" : "物品";
            snprintf(line, sizeof(line), "%s  价值%u", kind, item.price);
            render_utf_8_text(screen, line, 260, 80 + i * 18, 170, 16);
        }
    }
    return screen;
}

void inventory_render::process_key_click(uint8_t direct,uint8_t function) {
    if (first_active_player() < 0) {
        if (function) { pop(); }
        return;
    }
    selected_player = normalize_player(selected_player);
    if (direct & left) {
        selected_player = step_player(selected_player, -1);
        selected = 0;
    }
    if (direct & right) {
        selected_player = step_player(selected_player, 1);
        selected = 0;
    }
    if (direct & up) { selected = std::max(0, selected - 1); }
    if (direct & down) {
        selected = std::min(static_cast<int>(game_state::k_player_inventory_size) - 1,
                            selected + 1);
    }
    if (function & b) {
        pop();
        return;
    }
    if (function & key_y) {
        if (game_state::discard_item(selected, selected_player)) {
            push(new dialogue_render("已经丢弃。"));
        }
        else { push(new dialogue_render("这件物品不能丢弃。")); }
        return;
    }
    if (function & a) {
        unsigned id = selected_item_exists(selected_player, selected) ?
                      game_state::get().players[selected_player].inventory[selected] : 0;
        bool was_equipment = game_state::can_equip(id, selected_player);
        if (game_state::use_item(selected, selected_player)) {
            push(new dialogue_render(was_equipment ? "装备完成。" : "使用完成。"));
        } else {
            push(new dialogue_render("现在不能使用这件物品。"));
        }
    }
}

    uint8_t *equipment_render::render(uint8_t *screen) {
        clear_panel(screen);
        if (first_active_player() < 0) {
            render_utf_8_text(screen, "当前没有已入队角色。", 16, 48, 423, 16);
            return screen;
        }
        selected_player = normalize_player(selected_player);
        render_header(screen, "装备", selected_player);
        render_utf_8_text(screen, "←/→ 队员  A 装备选中物品  B 返回", 16, 56, 423, 16);
        const auto &player = game_state::get().players[selected_player];
        char line[180];
        snprintf(line, sizeof(line), "攻击 %u  防御 %u  速度 %u", player.attack, player.defense,
                 player.speed);
        render_utf_8_text(screen, line, 16, 76, 423, 16);
        for (unsigned i = 0; i < 6; i++) {
            snprintf(line, sizeof(line), "%s:", game_state::equipment_slot_name(i));
            render_utf_8_text(screen, line, 16, 100 + i * 16, 70, 16);
            render_item_name(screen, player.equipment[i], 88, 100 + i * 16, 170);
        }
        render_utf_8_text(screen, "携带物品", 260, 100, 160, 16);
        for (unsigned i = 0; i < game_state::k_player_inventory_size; i++) {
            unsigned id = player.inventory[i];
            snprintf(line, sizeof(line), "%c %u ", i == static_cast<unsigned>(selected) ? '>' : ' ',
                     i + 1);
            render_utf_8_text(screen, line, 260, 120 + i * 16, 48, 16);
            render_item_name(screen, id, 308, 120 + i * 16, 130);
        }
        return screen;
    }

    void equipment_render::process_key_click(uint8_t direct, uint8_t function) {
        if (first_active_player() < 0) {
            if (function) { pop(); }
            return;
        }
        selected_player = normalize_player(selected_player);
        if (direct & left) {
            selected_player = step_player(selected_player, -1);
            selected = 0;
        }
        if (direct & right) {
            selected_player = step_player(selected_player, 1);
            selected = 0;
        }
    if (direct&up) { selected=std::max(0,selected-1); }
        if (direct & down) {
            selected = std::min(static_cast<int>(game_state::k_player_inventory_size) - 1,
                                selected + 1);
        }
    if (function&b) { pop();return; }
    if (function&a) {
        if (game_state::equip(selected, selected_player)) {
            push(new dialogue_render("装备完成。"));
        }
        else { push(new dialogue_render("这个队员不能装备这件物品。")); }
    }
}

uint8_t *status_render::render(uint8_t *screen) {
    clear_panel(screen);
    if (first_active_player() < 0) {
        render_utf_8_text(screen, "当前没有已入队角色。", 16, 48, 423, 16);
        return screen;
    }
    selected_player = normalize_player(selected_player);
    render_header(screen, "状态", selected_player);
    render_utf_8_text(screen, "←/→ 队员  B 返回", 16, 56, 423, 16);
    const auto &player = game_state::get().players[selected_player];
    char line[180];
    uint32_t next = player.level < 99 ? game_data::experience(player.level + 1) : player.experience;
    snprintf(line, sizeof(line), "等级 %u  HP %u/%u  速度 %u", player.level, player.hp,
             player.max_hp, player.speed);
    render_utf_8_text(screen, line, 16, 82, 423, 16);
    snprintf(line, sizeof(line), "攻击 %u  防御 %u  经验 %u", player.attack, player.defense,
             player.experience);
    render_utf_8_text(screen, line, 16, 102, 423, 16);
    snprintf(line, sizeof(line), "下次升级 %u",
             next > player.experience ? next - player.experience : 0);
    render_utf_8_text(screen, line, 16, 122, 423, 16);
    render_utf_8_text(screen, "装备", 16, 150, 120, 16);
    for (unsigned i = 0; i < 6; i++) {
        snprintf(line, sizeof(line), "%s:", game_state::equipment_slot_name(i));
        render_utf_8_text(screen, line, 16, 170 + i * 14, 70, 14);
        render_item_name(screen, player.equipment[i], 88, 170 + i * 14, 150);
    }
    render_utf_8_text(screen, "物品", 260, 150, 120, 16);
    for (unsigned i = 0; i < game_state::k_player_inventory_size; i++) {
        snprintf(line, sizeof(line), "%u ", i + 1);
        render_utf_8_text(screen, line, 260, 170 + i * 14, 32, 14);
        render_item_name(screen, player.inventory[i], 292, 170 + i * 14, 130);
    }
    return screen;
}

    void status_render::process_key_click(uint8_t direct, uint8_t function) {
        if (first_active_player() < 0) {
            if (function) { pop(); }
            return;
        }
        selected_player = normalize_player(selected_player);
        if (direct & left) { selected_player = step_player(selected_player, -1); }
        if (direct & right) { selected_player = step_player(selected_player, 1); }
        if (function & b) { pop(); }
}
} // namespace metalmax
