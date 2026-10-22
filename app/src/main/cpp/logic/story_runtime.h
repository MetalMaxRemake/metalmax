#ifndef metalmax_story_runtime_h
#define metalmax_story_runtime_h

#include <stdint.h>

namespace metalmax::story_runtime {

void reset_runtime();
void reset_map_runtime(int map_id);

bool control_locked();
void toggle_control();

bool player_visible();
bool team_visible();
void player_show(int custom);
void player_hide();
void player_hide_all();
void player_become(int player);
void player_face(int direction);
void player_teleport(int map_id,int x,int y,bool quiet);
void player_pull(int object_id);
void player_unpull();

void play_music(int music_id);
void open_menu(int function,int line);
void scroll(int direction,int count);
void scene(int stage);
void scene_end();
void apply_pending_scene();
void quake(int ticks);
void flash(int ticks);
void sleep(int room);
void next_day();
void code(int code_id);

uint16_t tile(int map_id,int x,int y,uint16_t fallback);
void draw_tile(int map_id,int x,int y,uint16_t tile_id);
void open_door(int map_id,int x,int y,int direction);

bool detector(int x,int y,int range);
bool hp_at_least(int value);
bool drank();
bool ride_tank();
bool tank_riding(int tank_id);
bool tank_here(int tank_id);
bool has_ok_tank();

void give_item(int item_id,int text_row);
void swap_item(int item_a,int item_b);
void team_join(int player);
void team_leave(int player);
void team_hide();
void system(int command);
void conveyor(int direction);
void change_tank_map(int from_map,int to_map);
void medal();
void henshin();

} // namespace metalmax::story_runtime

#endif
