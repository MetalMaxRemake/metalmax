#ifndef metalmax_game_data_h
#define metalmax_game_data_h
#include <stdint.h>
// Shared API declarations only. Each resource is defined in its owning module
// header. Existing maps arrays and texture files remain unchanged.
namespace metalmax::game_data {
struct range_t { uint16_t offset; uint8_t count; };
struct npc_data_t { uint8_t type,x,y,talk1,talk2,action; };
struct entrance_t { uint8_t x,y,to_x,to_y,map; };
struct point_t { uint8_t map,x,y; };
struct border_t { uint8_t type; point_t targets[4]; };
struct item_data_t { const char *name; uint32_t price; uint16_t value; uint8_t kind,can_equip; bool buyable,discardable; };
struct monster_data_t { const char *name; uint16_t hp,attack,defense,experience,gold; uint8_t speed,hit,evasion; };
struct shop_entry_t { uint8_t item,count; };
struct shop_data_t { uint16_t offset; uint8_t type,page,count; };
struct treasure_data_t { uint8_t map,x,y,item; };
struct player_data_t { uint32_t experience; uint16_t hp,max_hp,attack,defense; uint8_t level,speed,team,equipment[8],inventory[8],equipment_state; };
enum script_op : uint8_t { k_stop,k_text,k_event_text,k_set_event,k_clear_event,k_if_event,k_if_position,k_if_yes,k_if_team,k_if_dead,k_if_level,k_if_money,k_if_item,k_if_treasure,k_jump,k_loop,k_wait,k_wait_set,k_wait_clear,k_face,k_move,k_move_to,k_teleport,k_wander,k_chase,k_animate,k_hide,k_recover,k_spend,k_recruit,k_battle,k_change_model,k_set_frame,k_face_player,k_face_back,k_offset,k_if_area,k_if_face,k_set_attrs,k_sprite_bank,k_model_delta,k_unsupported };
struct script_node_t { int16_t a,b,c,d,target; uint8_t op; };
struct script_data_t { uint16_t offset,count; };
constexpr unsigned treasure_count=91;
void apply_palette();
void decode_sprite(int bank,int model,int direction,int step,uint8_t result[256]);
void draw_sprite(int bank,int model,int direction,int step,int type,int x,int y,uint8_t *screen);
void draw_image(unsigned id,int x,int y,uint8_t *screen,int rows=-1);
void image_size(unsigned id,unsigned &width,unsigned &height);
void decode_image(unsigned id,uint8_t *destination); // tools/tests, caller owns space
uint16_t map_tile(unsigned map,int x,int y);
uint8_t map_attribute(unsigned map,int x,int y);
uint8_t map_sprite_bank(unsigned map);
range_t map_npcs(unsigned map);
npc_data_t npc(unsigned index);
range_t map_entrances(unsigned map);
entrance_t entrance_at(unsigned index);
border_t map_border(unsigned map);
const char *text(int page,int row);
const uint8_t *glyph(uint32_t unicode);
item_data_t item(unsigned id);
monster_data_t monster(unsigned id);
shop_data_t shop(int type,int page);
shop_entry_t shop_entry_at(unsigned index);
treasure_data_t treasure(unsigned index);
player_data_t initial_player(unsigned index);
uint32_t value(unsigned index);
uint32_t experience(unsigned level);
uint32_t initial_money();
script_data_t script(bool dynamic,unsigned game_id);
script_node_t script_node_at(unsigned index);
// Stable read-only entry for not-yet-ported ROM tables. This intentionally
// retains the cold source blob even when gameplay does not read it yet.
__attribute__((visibility("default"))) const uint8_t *source_bytes(uint32_t file_offset,unsigned length);
unsigned packed_bytes();
constexpr unsigned k_monster_image=1904,k_splash_image=2035,k_logo_image=2036,k_logo_2_image=2037,k_water_image=2038;
}
namespace game_data = metalmax::game_data;
#endif
