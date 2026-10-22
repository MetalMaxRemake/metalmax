#include "render_map.h"
#include "render_menu.h"
#include "render_debug.h"
#include "render_battle.h"
#include "status/game_state.h"
#include "../maps/map.h"
#include "../maps/map_data/map_data.h"
#include "../maps/active_tile_bmp.h"
#include "../graphic/bitmap_render.h"
#include "../graphic/native_graphic.h"
#include "../graphic/palette_data.h"
#include "../sprite/map_npcs.h"
#include "../audio/native_sound.h"
#include "../audio/mm_sound.h"
#include "../monster/monster_data/monster_data.h"
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <ctime>

namespace metalmax {
struct last_position {
    uint8_t map, x, y;
};
static std::vector<last_position> history;
static unsigned water_tick = 0;

map_render::map_render() { srand(static_cast<unsigned>(time(nullptr))); }

map_render::~map_render() {}

void map_render::update_map(int map, int x, int y) {
    map_id = (map % 240 + 240) % 240;
    auto *p = get_default_player();
    p->set_pos(std::max(0, std::min(x, static_cast<int>(map_size[map_id * 2 + 1]) - 1)),
              std::max(0, std::min(y, static_cast<int>(map_size[map_id * 2]) - 1)));
    refresh_current_map(map_id);
    map_npcs::load(map_id);
    encounter_cooldown = 30;
    game_state::get().map = map_id;
    game_state::get().x = p->x;
    game_state::get().y = p->y;
    refresh_music();
}

int map_render::get_map_id() { return map_id; }

void map_render::refresh_music() const {
    int track = static_cast<int>(map_music[map_id]) - 3;
    if (track >= 0 && track != get_audio_idx()) { change_audio(track); }
}

void map_render::on_focus() { refresh_music(); }

void map_render::on_un_focus() {}

void map_render::reset_palette() const { native_graphic::set_global_fade_percent(100); }

void map_render::render_players(uint8_t *screen, character *p) const {
    bitmap_render::render_bitmap_with_transport(p->current_bitmap(), 16, 16, p->render_x - camera_x,
                                                p->render_y - camera_y, screen);
}

void map_render::render_sprite(uint8_t *screen) const { map_npcs::render(screen, camera_x, camera_y); }

void map_render::render_water(uint8_t *screen) const {
    const int states[4] = {0, 1, 2, 1};
    int frame = states[(water_tick / 60) % 4];
    int first_x = camera_x / 16 - 1, first_y = camera_y / 16 - 1;
    for (int y = first_y; y < first_y + global_config::k_tile_height + 3; y++) {
        for (int x = first_x; x < first_x + global_config::k_tile_width + 3; x++) {
            if (!tile_is_pure_water(get_tile_idx(x, y))) { continue; }
            int sx = x * 16 - camera_x, sy = y * 16 - camera_y;
            game_data::draw_image(game_data::k_water_image+frame,sx,sy,screen);
            uint8_t shore = 0;
            if (!tile_is_water(get_tile_idx(x, y - 1))) { shore |= up; }
            if (!tile_is_water(get_tile_idx(x, y + 1))) { shore |= down; }
            if (!tile_is_water(get_tile_idx(x - 1, y))) { shore |= left; }
            if (!tile_is_water(get_tile_idx(x + 1, y))) { shore |= right; }
            render_wave(shore, frame, screen, sx, sy);
        }
    }
}

void map_render::trigger_door(character *p) const {
    unsigned attr = game_data::map_attribute(map_id,p->x,p->y);
    bool door = (attr & 252) == 0x3c;
    if (door != p->in_door) {
        render_effect(effect_door);
        p->in_door = door;
    }
}

uint8_t *map_render::render(uint8_t *screen) {
    auto *p = get_default_player();
    camera_x = p->render_x - (global_config::k_screen_width - 16) / 2;
    camera_y = p->render_y - (global_config::k_screen_height - 16) / 2;
    render_map(camera_x, camera_y, screen);
    render_water(screen);
    render_sprite(screen);
    render_players(screen, p);
    return screen;
}

void map_render::tik_logic() {
    auto *p = get_default_player();
    p->tik();
    map_npcs::tick();
    water_tick++;
    if (entrance_animation) {
        entrance_animation--;
        if (entrance_animation == 15) { enter_another_map(p); }
        int percent = entrance_animation >= 15 ? (entrance_animation - 15) * 100 / 15 :
                      (15 - entrance_animation) * 100 / 15;
        native_graphic::set_global_fade_percent(percent);
        if (!entrance_animation) { reset_palette(); }
    }
}

void map_render::process_key_click(uint8_t, uint8_t function) {
    if (entrance_animation) { return; }
    if (function & a) {
        push(new menu_render(map_menu));
        return;
    }
    if (function & key_x) {
        push(new debug_render);
        return;
    }
    if (function & ta) {
        int map = (map_id + 1) % 240;
        update_map(map, map_size[map * 2 + 1] / 2, map_size[map * 2] / 2);
        history.clear();
    }
    if (function & tb) { change_audio(get_audio_idx() + 1); }
}

bool map_render::process_key(uint8_t direct, uint8_t function) {
    auto *p = get_default_player();
    if (entrance_animation) { return true; }
    if (p->steping || !direct) { return !function; }
    int dir = direct & up ? 0 : direct & down ? 1 : direct & left ? 2 : 3;
    static const int dx[4] = {0, 0, -1, 1}, dy[4] = {-1, 1, 0, 0};
    p->direct = dir;
    p->half_step=0;
    int x = p->x + dx[dir], y = p->y + dy[dir];
    if (may_enter_another_map(p, x, y) || may_exit_to_last_map(p, x, y)) { return true; }
    if (!map_npcs::passable(map_id, x, y)) { return !function; }
    if (map_npcs::blocks(x, y) && !map_npcs::push_npc(x, y, dir)) { return !function; }
    p->x = x;
    p->y = y;
    p->steping = true;
    trigger_door(p);
    trigger_monster();
    return !function;
}

bool map_render::may_enter_another_map(character *p, int x, int y) {
    const auto &range_t = game_data::map_entrances(map_id);
    for (unsigned i = 0; i < range_t.count; i++) {
        const auto &e = game_data::entrance_at(range_t.offset + i);
        if (e.x != x || e.y != y || (p->x == x && p->y == y)) { continue; }
        if (e.map >= 240) { continue; }
        if (history.size() >= 64) { history.erase(history.begin()); }
        history.push_back({uint8_t(map_id),uint8_t(p->x),uint8_t(p->y)});
        next_map_id = e.map;
        next_x = e.to_x;
        next_y = e.to_y;
        entrance_animation = 30;
        render_effect(effect_enter_entrance);
        return true;
    }
    return false;
}

bool map_render::may_exit_to_last_map(character *, int x, int y) {
    int sx = movable_offset[map_id * 2], sy = movable_offset[map_id * 2 + 1];
    bool outside = x < sx || y < sy || x > sx + movable_size[map_id * 2] ||
                   y > sy + movable_size[map_id * 2 + 1]
                   || x < 0 || y < 0 || x >= map_size[map_id * 2 + 1] || y >= map_size[map_id * 2];
    if (!outside) { return false; }
    const auto &border_t = game_data::map_border(map_id);
    if (border_t.type == 0) {
        if (history.empty()) { return true; }
        const auto pos = history.back();
        history.pop_back();
        next_map_id = pos.map;
        next_x = pos.x;
        next_y = pos.y;
    } else {
        int direction = 0;
        if (border_t.type == 2) {
            direction = y < sy ? 0 : y > sy + movable_size[map_id * 2 + 1] ? 1 : x < sx ? 2 : 3;
        }
        const auto &target = border_t.targets[direction];
        if (target.map >= 240) { return true; }
        next_map_id = target.map;
        next_x = target.x;
        next_y = target.y;
    }
    entrance_animation = 30;
    return true;
}

bool map_render::enter_another_map(character *) {
    if (next_map_id < 0) { return false; }
    int map = next_map_id;
    next_map_id = -1;
    update_map(map, next_x, next_y);
    return true;
}

void map_render::trigger_monster() const {
    auto *p = get_default_player();
    if (p->god_mode || encounter_cooldown) {
        const_cast<map_render *>(this)->encounter_cooldown = encounter_cooldown ? encounter_cooldown - 1
                                                                             : 0;
        return;
    }
    if (rand() % 100 >= 11) { return; }
    int group = 0;
    if (!map_id) { group = monster_distribution_world[(p->y / 16) * 16 + p->x / 16]; }
    else if (map_id >= 128) { group = monster_distribution_4_samll_maps[map_id - 128]; }
    else { return; }
    if (group >= 93) { return; }
    int monster = monster_group[group * 10 + rand() % 10];
    if (monster > 0 && monster < monster_count) {
        const_cast<map_render *>(this)->encounter_cooldown = 12;
        push(new battle_render(monster));
    }
}
} // namespace metalmax
