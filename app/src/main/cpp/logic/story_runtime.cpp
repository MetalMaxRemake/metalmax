#include "story_runtime.h"
#include "render.h"
#include "render_dialogue.h"
#include "render_shop.h"
#include "render_inventory.h"
#include "render_computer.h"
#include "map_interaction.h"
#include "status/character.h"
#include "status/game_state.h"
#include "../sprite/map_npcs.h"
#include "../maps/map_data/map_data.h"
#include "../audio/mm_sound.h"
#include "../data/game_data.h"
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <vector>

extern "C" void change_audio(int audio_idx);
extern "C" void render_effect(int effect_audio_id);
extern "C" int get_audio_count();

namespace metalmax::story_runtime {
namespace {
struct tile_override_t {
    uint8_t map;
    uint8_t x;
    uint8_t y;
    uint16_t tile;
};

std::vector<tile_override_t> s_tiles;
bool s_control_locked = false;
bool s_player_visible = true;
bool s_team_visible = true;
bool s_drank = false;
int s_pulled_object = -1;
int s_camera_dx = 0;
int s_camera_dy = 0;
int s_effect_ticks = 0;
int s_pending_scene = -1;

int clamp_u8(int value) {
    return std::max(0,std::min(255,value));
}

void message(const char *text) {
    push(new dialogue_render(text));
}

void map_center(int &x,int &y) {
    int map_id = get_current_map();
    if (map_id >= 0 && map_id < MAP_COUNT) {
        x = map_size[map_id * 2 + 1] / 2;
        y = map_size[map_id * 2] / 2;
    } else {
        x = 0;
        y = 0;
    }
}

int normal_music_id(int music_id) {
    if (music_id >= 3 && music_id < 99) { return music_id - 3; }
    if (music_id >= 0 && music_id < 96) { return music_id; }
    return -1;
}
}

void reset_runtime() {
    s_tiles.clear();
    s_control_locked = false;
    s_player_visible = true;
    s_team_visible = true;
    s_drank = false;
    s_pulled_object = -1;
    s_camera_dx = 0;
    s_camera_dy = 0;
    s_effect_ticks = 0;
}

void reset_map_runtime(int) {
    s_camera_dx = 0;
    s_camera_dy = 0;
}

bool control_locked() { return s_control_locked; }

void toggle_control() { s_control_locked = !s_control_locked; }

bool player_visible() { return s_player_visible; }

bool team_visible() { return s_team_visible; }

void player_show(int custom) {
    s_player_visible = true;
    s_team_visible = true;
    if (custom > 0) { get_default_player()->bitmap_id = 9 + (custom & 3); }
}

void player_hide() { s_player_visible = false; }

void player_hide_all() {
    s_player_visible = false;
    s_team_visible = false;
}

void player_become(int player) {
    s_player_visible = true;
    get_default_player()->bitmap_id = 9 + (player & 3);
}

void player_face(int direction) { get_default_player()->direct = direction & 3; }

void player_teleport(int map_id,int x,int y,bool) {
    if (map_id < 0 || map_id >= MAP_COUNT) { map_id = get_current_map(); }
    if (map_id < 0 || map_id >= MAP_COUNT) { return; }
    if (x < 0 || x >= map_size[map_id * 2 + 1] || y < 0 || y >= map_size[map_id * 2]) {
        map_center(x,y);
    }
    change_map(map_id,x,y);
    s_player_visible = true;
}

void player_pull(int object_id) { s_pulled_object = object_id; }

void player_unpull() { s_pulled_object = -1; }

void play_music(int music_id) {
    int track = normal_music_id(music_id);
    if (track >= 0) { change_audio(track); }
    else { render_effect(music_id & 0x7f); }
}

void open_menu(int function,int line) {
    switch (function) {
        case 0x12:
        case 0x13:
        case 0x21:
        case 0x22:
        case 0x25:
        case 0x27:
        case 0x2a:
        case 0x2b:
        case 0x2c:
        case 0x34:
            open_computer(function,line);
            break;
        case 0x17:
        case 0x1e:
            push(new shop_render(function & 15,line));
            break;
        case 0x06:
            for (auto &player: game_state::get().players) {
                if (player.active) { player.hp = player.max_hp; }
            }
            message("生命恢复了。");
            break;
        default: {
            char text[96];
            snprintf(text,sizeof(text),"菜单功能 %02X:%02X 尚未完整开放。",function,line);
            message(text);
            break;
        }
    }
}

void scroll(int direction,int count) {
    static const int dx[4] = {0,0,-16,16};
    static const int dy[4] = {-16,16,0,0};
    if (direction >= 0 && direction < 4) {
        s_camera_dx += dx[direction] * std::max(1,count);
        s_camera_dy += dy[direction] * std::max(1,count);
    }
}

void scene(int stage) {
    s_control_locked = true;
    if (stage > 0 && stage < 169) {
        // SceneScript starts a dynamic scene in the original engine. The
        // current remake keeps the caller alive; the scene state is recorded
        // through event bits so generated dynamic actors can react to it.
        game_state::set_event(stage,true);
    }
    if (stage == 2) {
        s_pending_scene = 2;
    }
}

void scene_end() { s_control_locked = false; }

void apply_pending_scene() {
    if (s_pending_scene < 0) { return; }
    int scene_id=s_pending_scene;
    s_pending_scene=-1;
    if (scene_id==2) {
        player_hide_all();
        change_map(2,2,10);
        map_npcs::load(2,241);
    }
}

void quake(int ticks) {
    s_effect_ticks = std::max(s_effect_ticks,std::max(1,ticks));
    render_effect(effect_enter_entrance);
}

void flash(int ticks) {
    s_effect_ticks = std::max(s_effect_ticks,std::max(1,ticks));
    render_effect(effect_door);
}

void sleep(int) {
    s_drank = false;
    for (auto &player: game_state::get().players) {
        if (player.active) { player.hp = player.max_hp; }
    }
    message("休息后生命恢复了。");
}

void next_day() {
    s_drank = false;
    for (unsigned id = 0; id < 8; id++) { game_state::set_event(id,false); }
    player_show(0);
    s_control_locked=false;
    change_map(2,5,12);
}

void code(int code_id) {
    if (code_id == 0x57) {
        for (auto &player: game_state::get().players) {
            if (player.active) { player.hp = player.max_hp; }
        }
    }
}

uint16_t tile(int map_id,int x,int y,uint16_t fallback) {
    for (auto it = s_tiles.rbegin(); it != s_tiles.rend(); ++it) {
        if (it->map == map_id && it->x == x && it->y == y) { return it->tile; }
    }
    return fallback;
}

void draw_tile(int map_id,int x,int y,uint16_t tile_id) {
    if (map_id < 0 || map_id >= MAP_COUNT || x < 0 || y < 0 ||
        x >= map_size[map_id * 2 + 1] || y >= map_size[map_id * 2]) { return; }
    uint16_t tile_value = tile_id < 1904 ? tile_id : 0;
    for (auto &entry: s_tiles) {
        if (entry.map == map_id && entry.x == x && entry.y == y) {
            entry.tile = tile_value;
            return;
        }
    }
    if (s_tiles.size() >= 512) { s_tiles.erase(s_tiles.begin()); }
    s_tiles.push_back({uint8_t(map_id),uint8_t(x),uint8_t(y),tile_value});
}

void open_door(int map_id,int x,int y,int direction) {
    static const int dx[4] = {0,0,-1,1};
    static const int dy[4] = {-1,1,0,0};
    if (direction >= 0 && direction < 4) {
        x += dx[direction];
        y += dy[direction];
    }
    uint16_t current = tile(map_id,x,y,game_data::map_tile(map_id,x,y));
    draw_tile(map_id,x,y,current + 1);
    render_effect(effect_door);
}

bool detector(int x,int y,int range) {
    auto *player = get_default_player();
    return std::abs(player->x - x) + std::abs(player->y - y) <= std::max(0,range);
}

bool hp_at_least(int value) { return game_state::get().players[0].hp >= value; }

bool drank() { return s_drank; }

bool ride_tank() {
    return game_state::get().riding_tank>=0;
}

bool tank_riding(int tank_id) {
    return tank_id >= 0 && tank_id < static_cast<int>(game_state::k_tank_count) &&
           game_state::get().tanks[tank_id].riding;
}

bool tank_here(int tank_id) {
    int map_id = get_current_map();
    return tank_id >= 0 && tank_id < static_cast<int>(game_state::k_tank_count) &&
           game_state::get().tanks[tank_id].owned && game_state::get().tanks[tank_id].map == map_id;
}

bool has_ok_tank() {
    int map_id = get_current_map();
    for (const auto &tank: game_state::get().tanks) {
        if (tank.owned && tank.ok && !tank.riding && tank.map == map_id) { return true; }
    }
    return false;
}

void give_item(int item_id,int) {
    if (!game_state::add_item(item_id)) { message("物品栏已满。"); }
}

void swap_item(int item_a,int item_b) {
    auto &state = game_state::get();
    for (auto &item: state.inventory) {
        if (item == item_a) {
            item = item_b;
            return;
        }
    }
}

void team_join(int player) {
    if (player >= 1 && player <= 3) { game_state::get().players[player - 1].active = 1; }
}

void team_leave(int player) {
    if (player >= 1 && player <= 3) { game_state::get().players[player - 1].active = 0; }
}

void team_hide() { s_team_visible = false; }

void system(int command) {
    if (command == 0x00) { reset_runtime(); }
    else if (command == 0x03 || command == 0x04) { reset_map_runtime(get_current_map()); }
    else if (command == 0x05) { sleep(0); }
}

void conveyor(int) {}

void change_tank_map(int from_map,int to_map) {
    for (auto &tank: game_state::get().tanks) {
        if (tank.map == from_map) { tank.map = clamp_u8(to_map); }
    }
    if (get_current_map() == from_map) {
        auto *player = get_default_player();
        change_map(to_map,player->x,player->y);
    }
}

void medal() {
    if (game_state::get().riding_tank>=0) {
        game_state::get().tanks[game_state::get().riding_tank].ok=1;
    }
}

void henshin() {
    auto *player = get_default_player();
    player->bitmap_id = 12;
    s_player_visible = true;
}

} // namespace metalmax::story_runtime
