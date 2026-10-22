//
// Created by parkyu on 2023/5/11.
//

#include <cstdlib>
#include <cstdio>
#include "render_debug.h"
#include "logic.h"
#include "../charset/charsets.h"
#include "../monster/monster.h"
#include "render_battle.h"
#include "../audio/native_sound.h"
#include "../maps/map.h"
#include "status/character.h"
#include "../maps/map_data/map_data.h"
#include "../audio/mm_sound.h"
#include "../graphic/native_graphic.h"
#include "../graphic/palette_data.h"
#include "../monster/monster_data/monster_data.h"

namespace metalmax {
const char chinese_demo[4] = {0, 1, 2, 3};

inline void debug_render::render_debug_menu(uint8_t *screen_buffer) {
    int start = 70;
    int text_height = 7;
    render_asci_text(screen_buffer, "  ENTER BATTLE   ", 10, start + text_height * 0);
    render_asci_text(screen_buffer, "  >NEXT MUSIC    ", 10, start + text_height * 1);
    render_asci_text(screen_buffer, "  <PRE MUSIC     ", 10, start + text_height * 2);
    render_asci_text(screen_buffer, "  >NEXT MAP      ", 10, start + text_height * 3);
    render_asci_text(screen_buffer, "  <PRE MAP       ", 10, start + text_height * 4);
    render_asci_text(screen_buffer, "  CHANGE PALETTE ", 10, start + text_height * 5);
    render_asci_text(screen_buffer, "  RESET PALETTE  ", 10, start + text_height * 6);
    render_asci_text(screen_buffer, "  GOD MODE       ", 10, start + text_height * 7);
    render_asci_text(screen_buffer, "  EXIT           ", 10, start + text_height * 8);
    render_select_pos(screen_buffer, 13, start + (text_height * select_pos));
}

inline void debug_render::process_selection() {
    character *player = get_default_player();
    if (select_pos == 0) {
        pop();
        auto *battle = new battle_render(rand() % monster_count, true);
        push(battle);
    } else if (select_pos == 1) {
        change_audio(get_audio_idx() + 1);
    } else if (select_pos == 2) {
        change_audio(get_audio_idx() - 1);
    } else if (select_pos == 3) {
        int map_id = (get_current_map() + 1) % MAP_COUNT;
        int map_height = map_size[map_id * 2];
        int map_width = map_size[map_id * 2 + 1];
        player->set_pos(map_width / 2, map_height / 2);
        change_map(map_id, map_width / 2, map_height / 2);
    } else if (select_pos == 4) {
        int map_id = (get_current_map() + MAP_COUNT - 1) % MAP_COUNT;
        int map_height = map_size[map_id * 2];
        int map_width = map_size[map_id * 2 + 1];
        player->set_pos(map_width / 2, map_height / 2);
        change_map(map_id, map_width / 2, map_height / 2);
    } else if (select_pos == 5) {
        int *current_palette = native_graphic::get_palette_buffer();
        if (!current_palette) { return; }
        //fixme
        for (int i = 0; i < palette::palette_size; i++) {
            unsigned int dd = current_palette[i];
            unsigned int a = (dd & 0xff000000) >> 24;
            unsigned int r = (dd & 0x00ff0000) >> 16;
            unsigned int g = 0;
            unsigned int b = (dd & 0x000000ff) >> 0;
            current_palette[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
        native_graphic::apply_new_palette(current_palette);
    } else if (select_pos == 6) {
        int *current_palette = native_graphic::get_palette_buffer();
        if (!current_palette) { return; }
        //fixme
        for (int i = 0; i < palette::palette_size; i++) {
            int dd = palette::palette_rgb[i];
            int a = (dd & 0xff000000) >> 24;
            int b = (dd & 0x00ff0000) >> 16;
            int g = (dd & 0x0000ff00) >> 8;
            int r = (dd & 0x000000ff) >> 0;
            current_palette[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
        native_graphic::apply_new_palette(current_palette);
    } else if (select_pos == 7) {
        get_default_player()->god_mode = !get_default_player()->god_mode;
    } else if (select_pos == 8) {
        pop();
    }
}

uint8_t *debug_render::render(uint8_t *screen_buffer) {
    render_debug_info(screen_buffer);
    render_debug_menu(screen_buffer);
    return screen_buffer;
}

void debug_render::render_debug_info(uint8_t *screen_buffer) const {
    render_asci_text(screen_buffer, "METAL MAX 1 VERSION 0.10", 10, 10);
    render_zh_text(screen_buffer, chinese_demo, 4, 10, 24);
    char info[128];
    snprintf(info, sizeof(info), "MUSIC=%d, MAP=%d, GOD MODE=%s ",
             get_audio_idx(),
             get_current_map(),
             get_default_player()->god_mode ? "T" : "F");
    render_asci_text(screen_buffer, info, 10, 36);
    int map_id = get_current_map();
    snprintf(info, sizeof(info), "POS X=%d, Y=%d, TILE=%d ",
             get_default_player()->x,
             get_default_player()->y,
             get_tile_idx(get_default_player()->x,
                        get_default_player()->y));
    render_asci_text(screen_buffer, info, 10, 44);
}

void debug_render::process_key_click(uint8_t direct_key, uint8_t function_key) {
    render_effect(effect_push_button);
    if (direct_key & up) {
        select_pos--;
        if (select_pos < 0) {
            select_pos = 8;
        }
    }
    if (direct_key & down) {
        select_pos++;
        if (select_pos > 8) {
            select_pos = 0;
        }
    }
    if (function_key & a) {
        process_selection();
    }
    if (function_key & b) {
        pop();
    }
}

void debug_render::tik_logic() {

}

debug_render::debug_render() {
    log_d("DebugRender", "new");
}

debug_render::~debug_render() {
    log_d("DebugRender", "delete");
}
} // namespace metalmax
