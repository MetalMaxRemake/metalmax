//
// Created by parkyu on 2023/5/13.
//

#include <string.h>
#include "render_splash.h"
#include "../graphic/bitmap_render.h"
#include "../graphic/palette_data.h"
#include "render_map.h"
#include "../audio/native_sound.h"
#include "../charset/charsets.h"
#include "../charset/utf8_text.h"
#include "../data/game_data.h"
#include "status/game_state.h"

namespace metalmax {
static volatile int count = 0;
static volatile int splash_scene = 0;

static volatile int logo_y = 0;
static volatile int percent = 0;

void splash_render::process_key_click(uint8_t direct_key, uint8_t function_key) {
    if (splash_scene >= 5) {
        if (direct_key & (up | down)) { start_selection ^= 1; }
        if (!(function_key & a)) { return; }
        if (start_selection && !game_state::load()) { return; }
        if (!start_selection) { game_state::new_game(); }
        auto &s_state = game_state::get();
        const int map_id = s_state.map, x = s_state.x, y = s_state.y;
        pop();
        auto *map_view = new map_render;
        map_view->update_map(map_id, x, y);
        push(map_view);
    } else {
        splash_scene++;
        if (splash_scene == 5) {
            change_audio(1);
        }
    }
}

/**
 * @param clk time unit:second
 */
void scene_tik(float clk) {
    if (count > 60 * clk) {
        splash_scene++;
        count = 0;
    } else {
        count++;
    }
}

void splash_render::tik_logic() {
    if (splash_scene == 0) {
        scene_tik(2);
    } else if (splash_scene == 1) {
        percent++;
        int height = 32;
        if (percent >= height) {
            percent = 0;
            splash_scene++;
        }
    } else if (splash_scene == 2) {
        scene_tik(0.5);
    } else if (splash_scene == 3) {
        int height = 32;
        if (logo_y == 0) {
            logo_y = (global_config::k_screen_height / 2) - height / 2;
        } else if (logo_y > 52) {
            logo_y--;
        } else {
            splash_scene++;
        }
    } else if (splash_scene == 4) {
        int height = 106;
        if (percent < height) {
            percent++;
        } else {
            splash_scene = 5;
        }
    }
}

uint8_t *splash_render::render(uint8_t *screen_buffer) {
    memset(screen_buffer,
           palette::k_palette_black,
           global_config::k_screen_width * global_config::k_screen_height);
    if (splash_scene == 0) {
        int width = 143;
        int height = 79;
        game_data::draw_image(game_data::k_splash_image,(global_config::k_screen_width / 2) - width / 2,(global_config::k_screen_height / 2) - height / 2,screen_buffer,height);
        return screen_buffer;
    } else if (splash_scene == 1) {
        int width = 208;
        int height = 32;
        game_data::draw_image(game_data::k_logo_image,(global_config::k_screen_width / 2) - width / 2 + 4,(global_config::k_screen_height / 2) - height / 2,screen_buffer,percent);
        return screen_buffer;
    } else if (splash_scene == 2) {
        int width = 208;
        int height = 32;
        game_data::draw_image(game_data::k_logo_image,(global_config::k_screen_width / 2) - width / 2 + 4,(global_config::k_screen_height / 2) - height / 2,screen_buffer,height);
        return screen_buffer;
    } else if (splash_scene == 3) {
        int width = 208;
        int height = 32;
        game_data::draw_image(game_data::k_logo_image,(global_config::k_screen_width / 2) - width / 2 + 4,logo_y,screen_buffer,height);
        return screen_buffer;
    } else if (splash_scene == 4) {
        int width = 248;
        int height = 106;
        if (percent < height) {
            game_data::draw_image(game_data::k_logo_image,(global_config::k_screen_width/2)-208/2+4,52,screen_buffer);
        }
        render_asci_text(screen_buffer, "PUSH START", (global_config::k_screen_width / 2) - 32, 150);
        render_asci_text(screen_buffer, "1996 DATA EAST", (global_config::k_screen_width / 2) - 48,
                       180);
        render_asci_text(screen_buffer, "1996 CREA-TECH", (global_config::k_screen_width / 2) - 48,
                       190);
        render_asci_text(screen_buffer, "2023 PARK REMAKE", (global_config::k_screen_width / 2) - 56,
                       200);
        game_data::draw_image(game_data::k_logo_2_image,(global_config::k_screen_width / 2) - width / 2,10,screen_buffer,percent);
        return screen_buffer;
    } else {
        render_utf_8_text(screen_buffer, "重装机兵", 164, 64, 256, 20);
        render_utf_8_text(screen_buffer, start_selection == 0 ? "> 开始新游戏" : "  开始新游戏", 164,
                       104, 256, 20);
        render_utf_8_text(screen_buffer, start_selection == 1 ? "> 继续游戏" : "  继续游戏", 164, 136,
                       256, 20);
        render_utf_8_text(screen_buffer, game_state::has_save() ? "上/下选择  A 确认" : "没有保存记录",
                       164, 176, 256, 20);
        return screen_buffer;
    }
}

splash_render::~splash_render() {

}
} // namespace metalmax
