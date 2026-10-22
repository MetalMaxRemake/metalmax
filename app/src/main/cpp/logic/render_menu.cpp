#include "render_menu.h"
#include "render_inventory.h"
#include "render_dialogue.h"
#include "map_interaction.h"
#include "status/game_state.h"
#include "../charset/utf8_text.h"
#include "../graphic/palette_data.h"
#include <cstring>
#include <algorithm>

namespace metalmax {
menu_render::menu_render(int) {}

void menu_render::tik_logic() {}

void menu_render::process_selection() {
    switch (select_pos) {
        case 0:
            interact_with_npc();
            break;
        case 1:
            push(new inventory_render(true));
            break;
        case 2:
            push(new status_render);
            break;
        case 3:
            investigate_map();
            break;
        case 4:
            push(new inventory_render);
            break;
        case 5:
            push(new dialogue_render(game_state::save() ? "战斗记录保存了。" : "保存失败。"));
            break;
    }
}

void menu_render::process_key_click(uint8_t direct, uint8_t function) {
    if (direct & up) { select_pos = std::max(0, select_pos - 1); }
    if (direct & down) { select_pos = std::min(5, select_pos + 1); }
    if (function & b) {
        pop();
        return;
    }
    if (function & a) { process_selection(); }
}

uint8_t *menu_render::render(uint8_t *screen) {
    memset(screen + 144 * global_config::k_screen_width, palette::k_palette_black,
           112 * global_config::k_screen_width);
    static const char *labels[] = {"交谈", "装备", "状态", "调查", "物品", "保存"};
    for (int i = 0; i < 6; i++) {
        render_utf_8_text(screen, labels[i], 32, 152 + i * 16, 128, 16);
        if (i == select_pos) { render_select_pos(screen, 16, 154 + i * 16); }
    }
    return screen;
}
} // namespace metalmax
