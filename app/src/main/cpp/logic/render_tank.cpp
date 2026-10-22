#include "render_tank.h"
#include "render_dialogue.h"
#include "status/game_state.h"
#include "../charset/utf8_text.h"
#include "../graphic/palette_data.h"
#include "../tank/tank_data.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace metalmax {
    namespace {
        void clear_panel(uint8_t *screen) {
            memset(screen + 32 * global_config::k_screen_width, palette::k_palette_black,
                   224 * global_config::k_screen_width);
        }

        int first_owned_tank() {
            for (unsigned i = 0; i < game_state::k_tank_count; i++) {
                if (game_state::get().tanks[i].owned) { return i; }
            }
            return -1;
        }

        unsigned normalize_tank(unsigned tank_id) {
            if (tank_id < game_state::k_tank_count &&
                game_state::get().tanks[tank_id].owned) { return tank_id; }
            int first = first_owned_tank();
            return first >= 0 ? static_cast<unsigned>(first) : 0;
        }

        unsigned step_tank(unsigned tank_id, int direction) {
            if (first_owned_tank() < 0) { return 0; }
            int current = static_cast<int>(normalize_tank(tank_id));
            for (int i = 0; i < static_cast<int>(game_state::k_tank_count); i++) {
                current =
                        (current + direction + game_state::k_tank_count) % game_state::k_tank_count;
                if (game_state::get().tanks[current].owned) { return static_cast<unsigned>(current); }
            }
            return static_cast<unsigned>(current);
        }

        void item_text(uint8_t *screen, unsigned id, int x, int y, int width) {
            render_utf_8_text(screen, id ? game_data::item(id).name : "空", x, y, width, 16);
        }

        void header(uint8_t *screen, const char *title, unsigned tank_id) {
            char line[160];
            snprintf(line, sizeof(line), "%s  %s", title, tank_data::tank_name(tank_id));
            render_utf_8_text(screen, line, 16, 40, 423, 16);
        }
    }

    uint8_t *tank_status_render::render(uint8_t *screen) {
        clear_panel(screen);
        if (first_owned_tank() < 0) {
            render_utf_8_text(screen, "当前没有可用战车。", 16, 48, 423, 16);
            return screen;
        }
        selected_tank = normalize_tank(selected_tank);
        header(screen, "战车状态", selected_tank);
        render_utf_8_text(screen, "←/→ 战车  B 返回", 16, 56, 423, 16);
        const auto &tank = game_state::get().tanks[selected_tank];
        char line[180];
        snprintf(line, sizeof(line), "SP %u/%u  防御 %u/%u", tank.sp, tank.max_sp, tank.defense,
                 tank.max_defense);
        render_utf_8_text(screen, line, 16, 82, 423, 16);
        snprintf(line, sizeof(line), "攻击 %u  总防御 %u", game_state::tank_attack(selected_tank),
                 game_state::tank_defense(selected_tank));
        render_utf_8_text(screen, line, 16, 102, 423, 16);
        snprintf(line, sizeof(line), "载重 %u/%u  炮弹 %u", game_state::tank_load(selected_tank),
                 game_state::tank_capacity(selected_tank), tank.shells);
        render_utf_8_text(screen, line, 16, 122, 423, 16);
        snprintf(line, sizeof(line), "位置 M%u (%u,%u)%s%s", tank.map, tank.x, tank.y,
                 tank.riding ? "  乘坐中" : "", tank.pulled ? "  牵引中" : "");
        render_utf_8_text(screen, line, 16, 142, 423, 16);
        render_utf_8_text(screen, "装备", 16, 170, 120, 16);
        for (unsigned i = 0; i < game_state::k_tank_equipment_count; i++) {
            snprintf(line, sizeof(line), "%s:", game_state::tank_equipment_slot_name(i));
            render_utf_8_text(screen, line, 16, 190 + i * 10, 70, 10);
            item_text(screen, tank.equipment[i], 88, 190 + i * 10, 160);
        }
        return screen;
    }

    void tank_status_render::process_key_click(uint8_t direct, uint8_t function) {
        if (first_owned_tank() < 0) {
            if (function) { pop(); }
            return;
        }
        selected_tank = normalize_tank(selected_tank);
        if (direct & left) { selected_tank = step_tank(selected_tank, -1); }
        if (direct & right) { selected_tank = step_tank(selected_tank, 1); }
        if (function & b) { pop(); }
    }

    uint8_t *tank_equipment_render::render(uint8_t *screen) {
        clear_panel(screen);
        if (first_owned_tank() < 0) {
            render_utf_8_text(screen, "当前没有可用战车。", 16, 48, 423, 16);
            return screen;
        }
        selected_tank = normalize_tank(selected_tank);
        header(screen, "战车装备", selected_tank);
        render_utf_8_text(screen, "←/→ 战车  上/下 物品  Y 槽位  A 装备  B 返回", 16, 56, 423, 16);
        const auto &tank = game_state::get().tanks[selected_tank];
        char line[160];
        render_utf_8_text(screen, "装备槽", 16, 82, 120, 16);
        for (unsigned i = 0; i < game_state::k_tank_equipment_count; i++) {
            snprintf(line, sizeof(line), "%c %s:", i == selected_slot ? '>' : ' ',
                     game_state::tank_equipment_slot_name(i));
            render_utf_8_text(screen, line, 16, 104 + i * 16, 90, 16);
            item_text(screen, tank.equipment[i], 112, 104 + i * 16, 140);
        }
        render_utf_8_text(screen, "车载物品", 270, 82, 150, 16);
        for (unsigned i = 0; i < game_state::k_tank_inventory_size; i++) {
            snprintf(line, sizeof(line), "%c %u ", i == selected_item ? '>' : ' ', i + 1);
            render_utf_8_text(screen, line, 270, 104 + i * 16, 48, 16);
            item_text(screen, tank.inventory[i], 318, 104 + i * 16, 120);
        }
        return screen;
    }

    void tank_equipment_render::process_key_click(uint8_t direct, uint8_t function) {
        if (first_owned_tank() < 0) {
            if (function) { pop(); }
            return;
        }
        selected_tank = normalize_tank(selected_tank);
        if (direct & left) {
            selected_tank = step_tank(selected_tank, -1);
            selected_slot = 0;
            selected_item = 0;
        }
        if (direct & right) {
            selected_tank = step_tank(selected_tank, 1);
            selected_slot = 0;
            selected_item = 0;
        }
        if (direct & up) { selected_item = selected_item ? selected_item - 1 : 0; }
        if (direct & down) {
            selected_item = std::min<unsigned>(game_state::k_tank_inventory_size - 1,
                                               selected_item + 1);
        }
        if (function & key_y) {
            selected_slot = (selected_slot + 1) % game_state::k_tank_equipment_count;
        }
        if (function & b) {
            pop();
            return;
        }
        if (function & a) {
            if (game_state::equip_tank(selected_item, selected_tank, selected_slot)) {
                push(new dialogue_render("战车装备完成。"));
            } else {
                push(new dialogue_render("不能装备到这个位置。"));
            }
        }
    }
}
