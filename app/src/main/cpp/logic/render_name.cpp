#include "render_name.h"
#include "render_map.h"
#include "render_dialogue.h"
#include "story_runtime.h"
#include "status/game_state.h"
#include "../sprite/map_npcs.h"
#include "../data/game_data.h"
#include "../charset/utf8_text.h"
#include "../graphic/palette_data.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace metalmax {
    namespace {
        constexpr int k_letter_count = 28;

        char letter_at(int index) {
            return index >= 0 && index < 26 ? static_cast<char>('A' + index) : '\0';
        }
    }

    void name_render::start_opening() {
        game_state::new_game();
        game_state::set_player_name(0, name);
        auto &state = game_state::get();
        state.map = 3;
        state.x = 7;
        state.y = 9;
        state.direction = 1;
        auto *player = get_default_player();
        player->set_pos(state.x, state.y);
        player->direct = state.direction;
        story_runtime::player_hide_all();
        pop();
        auto *map_view = new map_render;
        map_view->update_map(3, state.x, state.y);
        clear_map_history();
        push_map_history(1, 8, 12);
        push_map_history(2, 2, 10);
        push(map_view);
        map_npcs::load(3, 240);
    }

    uint8_t *name_render::render(uint8_t *screen) {
        memset(screen, palette::k_palette_black, global_config::k_screen_buffer_size);
        render_utf_8_text(screen, "给主角起名", 156, 34, 220, 18);
        char line[80];
        snprintf(line, sizeof(line), "%s", name);
        render_utf_8_text(screen, line, 176, 66, 180, 18);
        static const char *letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        for (int i = 0; i < 26; i++) {
            char text[3] = {letters[i], 0, 0};
            int x = 112 + (i % 7) * 36, y = 104 + (i / 7) * 24;
            render_utf_8_text(screen, text, x, y, 28, 16);
            if (selected == i) { render_select_pos(screen, x - 12, y + 2); }
        }
        render_utf_8_text(screen, selected == 26 ? "> DEL" : "  DEL", 112, 208, 80, 16);
        render_utf_8_text(screen, selected == 27 ? "> END" : "  END", 220, 208, 80, 16);
        render_utf_8_text(screen, "A 输入  B 删除", 112, 232, 220, 16);
        return screen;
    }

    void name_render::process_key_click(uint8_t direct, uint8_t function) {
        if (direct & left) { selected = std::max(0, selected - 1); }
        if (direct & right) { selected = std::min(k_letter_count - 1, selected + 1); }
        if (direct & up) { selected = std::max(0, selected - 7); }
        if (direct & down) { selected = std::min(k_letter_count - 1, selected + 7); }
        if (function & b) {
            size_t len = strlen(name);
            if (len > 0) { name[len - 1] = 0; }
            return;
        }
        if (!(function & a)) { return; }
        if (selected < 26) {
            size_t len = strlen(name);
            if (len < 8) {
                name[len] = letter_at(selected);
                name[len + 1] = 0;
            }
        } else if (selected == 26) {
            size_t len = strlen(name);
            if (len > 0) { name[len - 1] = 0; }
        } else {
            if (!*name) { snprintf(name, sizeof(name), "HUNTER"); }
            start_opening();
        }
    }
}
