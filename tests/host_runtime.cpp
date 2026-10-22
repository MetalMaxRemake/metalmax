#include "logic/render_map.h"
#include "maps/map_data/map_data.h"
#include "logic/render_menu.h"
#include "logic/render_battle.h"
#include "logic/render_dialogue.h"
#include "logic/render_inventory.h"
#include "logic/render_tank.h"
#include "logic/render_splash.h"
#include "logic/render_name.h"
#include "logic/map_interaction.h"
#include "logic/story_runtime.h"
#include "logic/render_shop.h"
#include "logic/status/game_state.h"
#include "sprite/map_npcs.h"
#include "data/game_data.h"
#include "graphic/palette_data.h"
#include "graphic/bitmap_render.h"
#include "graphic/native_graphic.h"
#include "charset/utf8_text.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <fstream>
using namespace metalmax;

static int fade=100;
static int audio=0;
namespace metalmax::native_graphic {
void set_render_callback(uint8_t*(*)(uint8_t*)) {}
int get_fps() { return 60; }
int palette_buffer[256];
int *get_palette_buffer() { return palette_buffer; }
void apply_new_palette(int*) {}
void set_global_fade_percent(unsigned value) { fade=value; }
}
extern "C" void change_audio(int value) { audio=value; }
extern "C" void render_effect(int) {}
extern "C" int get_audio_idx() { return audio; }
extern "C" int get_audio_count() { return 96; }

static void draw(const char *path,std::vector<uint8_t> &screen) {
    FILE *f=fopen(path,"wb");assert(f);fprintf(f,"P6\n455 256\n255\n");
    for (uint8_t index:screen) {
        uint32_t c=palette::palette_rgb[index];uint8_t rgb[3]={static_cast<uint8_t>(c>>16),static_cast<uint8_t>(c>>8),static_cast<uint8_t>(c)};
        fwrite(rgb,3,1,f);
    }
    fclose(f);
}
int main(int argc,char **argv) {
    assert(argc==3);game_data::apply_palette();game_state::new_game();game_state::set_save_directory(argv[1]);
    auto find_player_item = [](unsigned player, unsigned id) {
        for (unsigned i = 0; i < game_state::k_player_inventory_size; i++) {
            if (game_state::get().players[player].inventory[i] == id) { return i; }
        }
        assert(false);
        return 0u;
    };
    auto write_flat_inventory = [](uint8_t *out) {
        unsigned slot = 0;
        for (const auto &p: game_state::get().players) {
            for (uint8_t item: p.inventory) {
                if (item && slot < game_state::k_inventory_size) { out[slot++] = item; }
            }
        }
        for (uint8_t item: game_state::get().inventory) {
            if (item && slot < game_state::k_inventory_size) { out[slot++] = item; }
        }
    };
    std::vector<uint8_t> screen(global_config::k_screen_buffer_size);
    splash_render splash;
    for (int i = 0; i < 400; i++) { splash.tik_logic(); }
    splash.render(screen.data());
    name_render naming;
    naming.render(screen.data());
    naming.process_key_click(0, 1);
    assert(!strcmp(game_state::player_name(0), "HUNTER") && get_current_map() == 3);
    assert(map_npcs::count() == 3);
    story_runtime::scene(2);
    dynamic_cast<map_render *>(top())->tik_logic();
    assert(get_current_map() == 2 && map_npcs::count() == 2);
    release_logic_thread();
    auto *map=new map_render;map->update_map(1,14,14);push(map);
    auto pop_to_map = [&]() {
        while (top() && top() != map) { pop(); }
        assert(top() == map);
    };
    auto *npc=map_npcs::get(2);assert(npc && npc->data.action==2);
    int old_x=npc->tile_x,old_y=npc->tile_y;bool moved=false,animated=false;
    srand(123);
    for (int i=0;i<1200;i++) {
        map->tik_logic();if (npc->tile_x!=old_x || npc->tile_y!=old_y) { moved=true; }
        if (npc->frame==1) { animated=true; }
    }
    assert(moved && animated);map->render(screen.data());draw((std::string(argv[1])+"/npc-walking.ppm").c_str(),screen);
    npc=map_npcs::get(2);npc->set_attrs(npc->attributes|64);
    for (int i=0;i<100;i++) { map->tik_logic();assert(npc->frame==0); }
    npc=map_npcs::get(0);assert(npc);npc->set_attrs(npc->attributes|128);int original=npc->direction;npc->face(0);assert(npc->direction==original);
    // Movement speed is explicit: the player should feel responsive, scripted
    // NPCs should not race across the map, and pushed NPCs keep pace with the
    // player to avoid visual overlap.
    auto *player=get_default_player();bool found_step=false;int speed_x=0,speed_y=0;
    for (int y=1;y<map_size[2] && !found_step;y++) {
        for (int x=1;x+1<map_size[3] && !found_step;x++) {
            if (map_npcs::passable(1,x,y) && map_npcs::passable(1,x+1,y)) {
                map->update_map(1,x,y);assert(map->process_key(right,0));
                int start=player->render_x;for (int i=0;i<7;i++) { map->tik_logic(); }
                assert(player->steping && player->render_x==start+14);
                map->tik_logic();assert(!player->steping && player->render_x==(x+1)*16);
                speed_x=x;speed_y=y;found_step=true;
            }
        }
    }
    assert(found_step);
    for (int i=0;i<map_npcs::count();i++) { map_npcs::get(i)->script.start(true,200); }
    npc=map_npcs::get(2);assert(npc);npc->set_attrs(npc->attributes|0x10);
    player->set_pos(0,0);
    npc->teleport(speed_x,speed_y);assert(npc->move(3));int npc_start=npc->render_x;
    for (int i=0;i<31;i++) { map_npcs::tick(); }
    assert(npc->moving && npc->render_x==npc_start+15);
    map_npcs::tick();assert(!npc->moving && npc->render_x==(speed_x+1)*16);
    npc->teleport(speed_x,speed_y);assert(npc->move(3,1,2));npc_start=npc->render_x;
    for (int i=0;i<7;i++) { map_npcs::tick(); }
    assert(npc->moving && npc->render_x==npc_start+14);
    map_npcs::tick();assert(!npc->moving && npc->render_x==(speed_x+1)*16);
    // Reference executes the ROM's actual6502 sprite routines and rasterizes
    // OAM. It includes image50 and all three model types, independently of
    // the editor's guessed table boundaries and image composition helpers.
    unsigned poses=0;
    FILE *normal=fopen((std::string(argv[2])+"/normal.bin").c_str(),"rb");assert(normal);
    FILE *offsets=fopen((std::string(argv[2])+"/normal_y_offsets.bin").c_str(),"rb");assert(offsets);
    FILE *actual_poses=fopen((std::string(argv[1])+"/normal_sprites.bin").c_str(),"wb");assert(actual_poses);
    for (int bank:{0,100,102,106,107,116,148,150,184,186,188}) {
        for (int model=1;model<64;model++) {
            for (int direction=0;direction<4;direction++) {
                for (int frame=0;frame<2;frame++) {
                    uint8_t pixels[256],expected[256];game_data::decode_sprite(bank,model,direction,frame,pixels);
                    assert(fread(expected,256,1,normal)==1);
                    if (memcmp(pixels,expected,256)) {
                        fprintf(stderr,"POSE_ERROR bank=%d model=%d direction=%d frame=%d\n",bank,model,direction,frame);abort();
                    }
                    int dy=fgetc(offsets);assert(dy==0 || dy==1);
                    std::fill(screen.begin(),screen.end(),35);
                    game_data::draw_sprite(bank,model,direction,frame,0,64,64,screen.data());
                    for (int y=0;y<17;y++)for (int x=0;x<16;x++) {
                        uint8_t value=y>=dy && y<16+dy ? expected[(y-dy)*16+x] : 35;
                        assert(screen[(64+y)*455+64+x]==value);
                    }
                    fwrite(pixels,256,1,actual_poses);poses++;
                }
            }
        }
    }
    assert(fgetc(normal)==EOF && fgetc(offsets)==EOF);
    fclose(normal);fclose(offsets);fclose(actual_poses);
    unsigned model_cases=0;
    FILE *models=fopen((std::string(argv[2])+"/models.bin").c_str(),"rb");assert(models);
    FILE *actual_models=fopen((std::string(argv[1])+"/sprite_models.bin").c_str(),"wb");assert(actual_models);
    for (int bank:{0,100,102,106,107,116,148,150,184,186,188}) {
        for (int kind:{1,2}) {
            for (int model=1;model<=(kind==1?69:54);model++) {
                std::fill(screen.begin(),screen.end(),35);
                game_data::draw_sprite(bank,model,0,0,kind,64,64,screen.data());
                for (int y=0;y<96;y++)for (int x=0;x<96;x++) {
                    int expected=fgetc(models);assert(expected!=EOF);
                    if (screen[(32+y)*455+32+x]!=expected) {
                        fprintf(stderr,"MODEL_ERROR bank=%d type=%d model=%d x=%d y=%d expected=%d actual=%d\n",bank,kind,model,x,y,expected,screen[(32+y)*455+32+x]);abort();
                    }
                    fputc(screen[(32+y)*455+32+x],actual_models);
                }
                model_cases++;
            }
        }
    }
    assert(fgetc(models)==EOF);fclose(models);fclose(actual_models);
    // Exercise the actual Character route as well as the generic decoder.
    player=get_default_player();assert(player->bitmap_id==9);
    int saved_direction=player->direct,saved_step=player->half_step;
    FILE *player_frames=fopen((std::string(argv[1])+"/player_sprites.bin").c_str(),"wb");assert(player_frames);
    for (int direction=0;direction<4;direction++)for (int frame=0;frame<2;frame++) {
        player->direct=direction;player->half_step=frame;
        uint8_t expected[256];game_data::decode_sprite(0,9,direction,frame,expected);
        assert(!memcmp(player->current_bitmap(),expected,256));fwrite(player->current_bitmap(),256,1,player_frames);
    }
    fclose(player_frames);player->direct=saved_direction;player->half_step=saved_step;
    uint8_t pose_a[256],pose_b[256];game_data::decode_sprite(100,1,1,0,pose_a);game_data::decode_sprite(100,1,1,1,pose_b);assert(memcmp(pose_a,pose_b,256));
    // Conditional text responds to event08 and uses game static IDs1..108.
    script_machine machine;script_host host;machine.start(false,30);
    assert(machine.tick(host)==script_machine::k_text && machine.page==5 && machine.row==0x33);
    game_state::set_event(8,true);machine.start(false,30);
    assert(machine.tick(host)==script_machine::k_text && machine.row==0x9e);
    game_state::set_event(8,false);
    // Scene changes can close dialogue/menu layers without invalidating actor
    // callbacks before their onClose has run.
    push(new dialogue_render(30,map_npcs::get(1)));assert(map_npcs::get(1)->talking);
    change_map(2,3,6);assert(top()==map && get_current_map()==2);
    // Same category replacement keeps equipment slots compact and balances
    // attack/defense; full inventory operations are transactional.
    assert(!game_state::has_item(202));
    assert(game_state::add_item(42));
    unsigned slot = find_player_item(0, 42);
    int attack = game_state::get().players[0].attack;
    assert(game_state::equip(slot, 0));
    assert(game_state::get().players[0].attack==attack+7 && game_state::has_item(41));
    game_state::get().players[1].active = 1;
    assert(game_state::add_item_to_player(42, 1));
    slot = find_player_item(1, 42);
    int player0_attack = game_state::get().players[0].attack;
    int player1_attack = game_state::get().players[1].attack;
    assert(game_state::equip(slot, 1));
    assert(game_state::get().players[0].attack == player0_attack);
    assert(game_state::get().players[1].attack == player1_attack + game_data::item(42).value);
    assert(game_state::has_item(202));
    assert(game_state::add_item_to_player(170, 1));
    slot = find_player_item(1, 170);
    assert(game_state::discard_item(slot, 1));
    assert(game_state::add_item(169));
    slot = find_player_item(0, 169);
    game_state::get().players[0].hp = 20;
    assert(game_state::use_healing_item(slot, 0));
    assert(game_state::get().players[0].hp == 50);
    inventory_render inventory;
    inventory.render(screen.data());
    inventory.process_key_click(right, 0);
    inventory.render(screen.data());
    equipment_render equipment;
    equipment.render(screen.data());
    equipment.process_key_click(right, 0);
    equipment.render(screen.data());
    status_render status;
    status.render(screen.data());
    status.process_key_click(right, 0);
    status.render(screen.data());
    // Tanks are now first-class state: map position, riding, pulling,
    // equipment and UI all share the save-backed game_state.
    assert(game_state::get().tanks[0].owned && game_state::get().tanks[0].map == 139);
    map->update_map(139, 20, 21);
    assert(map->process_key(right, 0));
    assert(game_state::get().riding_tank == 0 && game_state::get().tanks[0].riding);
    for (int i = 0; i < 8; i++) { map->tik_logic(); }
    assert(game_state::get().tanks[0].x == 21 && game_state::get().tanks[0].y == 21);
    game_state::exit_tank();
    assert(game_state::get().riding_tank < 0 && !game_state::get().tanks[0].riding);
    game_state::move_tank(0, 139, 18, 21, 3);
    game_state::move_tank(1, 139, 20, 21, 3);
    game_state::get().tanks[1].owned = 1;
    game_state::pull_tank(1);
    map->update_map(139, 21, 21);
    assert(map->process_key(right, 0));
    for (int i = 0; i < 8; i++) { map->tik_logic(); }
    assert(game_state::get().tanks[1].x == 21 && game_state::get().tanks[1].y == 21);
    game_state::unpull_tank();
    assert(game_state::add_tank_item(65, 0));
    unsigned tank_slot = 0;
    while (game_state::get().tanks[0].inventory[tank_slot] != 65) { tank_slot++; }
    unsigned old_weapon = game_state::get().tanks[0].equipment[0];
    assert(game_state::equip_tank(tank_slot, 0, 0));
    assert(game_state::get().tanks[0].equipment[0] == 65 &&
           game_state::get().tanks[0].inventory[tank_slot] == old_weapon);
    tank_status_render tank_status;
    tank_status.render(screen.data());
    tank_status.process_key_click(right, 0);
    tank_status.render(screen.data());
    tank_equipment_render tank_equipment;
    tank_equipment.render(screen.data());
    tank_equipment.process_key_click(right, 0);
    tank_equipment.render(screen.data());
    // Save includes map/event/player/inventory. A corrupted save is rejected
    // without replacing the live state.
    assert(game_state::save());uint32_t money=game_state::get().money;game_state::get().money=1;
    assert(game_state::load() && game_state::get().money==money);
    // Real v1 file layout was244 bytes with32-bit player fields. Migration
    // must validate before narrowing; v2 writes156-byte state on the next save.
    uint8_t old[244]={};auto put32=[&](unsigned at,uint32_t v) { for (int i=0;i<4;i++) { old[at+i]=v>>(8*i); } };
    for (unsigned i=0;i<3;i++) {
        const auto &p=game_state::get().players[i];unsigned base=i*40;
        put32(base,p.hp);put32(base+4,p.max_hp);put32(base+8,p.attack);put32(base+12,p.defense);
        put32(base+16,p.level);put32(base+20,p.speed);put32(base+24,p.experience);
        old[base+28]=p.active;memcpy(old+base+29,p.equipment,8);old[base+37]=p.equipment_state;
    }
    put32(120,money);
    uint8_t flat_inventory[game_state::k_inventory_size] = {};
    write_flat_inventory(flat_inventory);
    for (unsigned i = 0; i < 32; i++) { old[124 + i * 2] = flat_inventory[i]; }
    old[236]=game_state::get().map;old[238]=game_state::get().x;old[240]=game_state::get().y;old[242]=game_state::get().direction;
    uint32_t crc=0xffffffff;
    for (uint8_t value:old) { crc^=value;for (int bit=0;bit<8;bit++) { crc=(crc>>1)^(0xedb88320u&-(crc&1)); } }
    uint32_t old_header[4]={0x4d4d5356,1,244,~crc};
    FILE *legacy=fopen((std::string(argv[1])+"/metalmax-v1.sav").c_str(),"wb");assert(legacy);
    fwrite(old_header,sizeof(old_header),1,legacy);fwrite(old,sizeof(old),1,legacy);fclose(legacy);
    game_state::get().money=1;assert(game_state::load() && game_state::get().money==money);assert(game_state::save());
    std::string save=std::string(argv[1])+"/metalmax-v1.sav";FILE *f=fopen(save.c_str(),"r+b");assert(f);fseek(f,25,SEEK_SET);fputc(0xff,f);fclose(f);
    game_state::get().money=7;assert(!game_state::load() && game_state::get().money==7);
    assert(game_state::save());
    // Buy data-backed human equipment and reject an unaffordable purchase
    // without changing inventory. The UI uses the same state path.
    auto shop_data_t=game_data::shop(2,0);assert(shop_data_t.count);
    const auto &entry=game_data::shop_entry_at(shop_data_t.offset);auto price=game_data::item(entry.item).price;
    game_state::get().money=1000;auto *shop=new shop_render(2,0);push(shop);
    shop->process_key_click(0,1);assert(game_state::get().money==1000-price && game_state::has_item(entry.item));
    pop();pop();assert(top()==map);
    auto tank_shop_data = game_data::shop(0, 0);
    assert(tank_shop_data.count);
    const auto &tank_entry = game_data::shop_entry_at(tank_shop_data.offset);
    game_state::get().money = 100000;
    auto *tank_shop = new shop_render(0, 0);
    push(tank_shop);
    tank_shop->process_key_click(0, 1);
    assert(game_state::has_item(tank_entry.item));
    pop_to_map();
    game_state::get().players[0].hp = 1;
    auto *inn = new shop_render(6, 0);
    push(inn);
    inn->process_key_click(0, 1);
    assert(game_state::get().players[0].hp == game_state::get().players[0].max_hp);
    pop_to_map();
    game_state::get().tanks[0].sp = 1;
    auto *sp_vendor = new shop_render(13, 0);
    push(sp_vendor);
    sp_vendor->process_key_click(0, 1);
    assert(game_state::get().tanks[0].sp > 1);
    pop_to_map();
    audio = 0;
    auto *jukebox = new shop_render(10, 0);
    push(jukebox);
    jukebox->process_key_click(0, 1);
    assert(audio != 0);
    pop_to_map();
    game_state::get().players[0].hp = 1;
    open_computer(0x88, 0);
    assert(game_state::get().players[0].hp == game_state::get().players[0].max_hp);
    pop_to_map();
    game_state::get().tanks[0].sp = 1;
    open_computer(0xa4, 0);
    assert(game_state::get().tanks[0].sp == game_state::get().tanks[0].max_sp);
    pop_to_map();
    unsigned old_max_defense = game_state::get().tanks[0].max_defense;
    game_state::get().money = 1000;
    open_computer(0x8c, 0);
    assert(game_state::get().tanks[0].max_defense > old_max_defense);
    pop_to_map();
    money = game_state::get().money;
    open_computer(0xc8, 0);
    assert(game_state::get().money != money || game_state::get().money == money - 10);
    pop_to_map();
    open_computer(0, -1);
    assert(get_current_map() == 9);
    pop_to_map();
    change_map(1, 14, 14);
    open_computer(0x40, 0);
    assert(dynamic_cast<shop_render *>(top()) != nullptr);
    pop_to_map();
    // New-game world entrance_t reaches town01 after the middle of the fade.
    change_map(0,91,232);get_default_player()->god_mode=true;
    map->process_key(up,0);
    for (int i=0;i<31;i++) { map->tik_logic();assert(fade>=0 && fade<=100); }
    assert(map->get_map_id()==1 && get_default_player()->x==13 && get_default_player()->y==14 && fade==100);
    // The next step into an ordinary wall is rejected using map-local attrs.
    bool wall_tested=false;
    for (int y=1;y<14 && !wall_tested;y++) {
        for (int x=1;x<23 && !wall_tested;x++) {
            if (map_npcs::passable(1,x,y) && !map_npcs::passable(1,x,y-1)) {
                map->update_map(1,x,y);map->process_key(up,0);
                assert(get_default_player()->x==x && get_default_player()->y==y);wall_tested=true;
            }
        }
    }
    assert(wall_tested);
    // Forced world edge with no history is blocked, never underflows a stack.
    change_map(0,0,0);get_default_player()->god_mode=true;
    assert(map->process_key(up,0));assert(get_default_player()->x==0 && get_default_player()->y==0);
    // All map caches, actors and generated special images fit clipped buffers.
    for (int id=0;id<240;id++) {
        map->update_map(id,0,0);map->tik_logic();map->render(screen.data());
    }
    change_map(1,14,14);map->render(screen.data());
    dialogue_render dialogue(game_data::text(11,129));dialogue.render(screen.data());draw((std::string(argv[1])+"/dialogue.ppm").c_str(),screen);
    // Battle-local item and equipment commands mutate the selected character or
    // ridden tank directly, without opening map inventory screens.
    memset(game_state::get().players[0].inventory, 0, game_state::k_player_inventory_size);
    game_state::get().players[0].inventory[0] = 169;
    game_state::get().players[0].hp = 20;
    game_state::get().players[0].defense = 5000;
    auto *item_battle = new battle_render(1);
    push(item_battle);
    for (int i = 0; i < 61; i++) { item_battle->tik_logic(); }
    item_battle->process_key_click(right, 0);
    item_battle->process_key_click(right, 0);
    item_battle->process_key_click(0, 1);
    item_battle->process_key_click(0, 1);
    assert(game_state::get().players[0].hp > 20 && game_state::get().players[0].inventory[0] == 0);
    pop();
    memset(game_state::get().players[0].inventory, 0, game_state::k_player_inventory_size);
    game_state::get().players[0].equipment[0] = 41;
    game_state::get().players[0].equipment_state |= 0x80;
    game_state::get().players[0].attack = 15;
    game_state::get().players[0].inventory[0] = 42;
    attack = game_state::get().players[0].attack;
    auto *equip_battle = new battle_render(1);
    push(equip_battle);
    for (int i = 0; i < 61; i++) { equip_battle->tik_logic(); }
    equip_battle->process_key_click(right, 0);
    equip_battle->process_key_click(right, 0);
    equip_battle->process_key_click(right, 0);
    equip_battle->process_key_click(0, 1);
    equip_battle->process_key_click(0, 1);
    assert(game_state::get().players[0].attack == attack + 7);
    pop();
    game_state::enter_tank(0);
    auto &battle_tank = game_state::get().tanks[0];
    battle_tank.sp = 10;
    battle_tank.max_sp = 300;
    memset(battle_tank.inventory, 0, game_state::k_tank_inventory_size);
    battle_tank.inventory[0] = 203;
    auto *tank_item_battle = new battle_render(1);
    push(tank_item_battle);
    for (int i = 0; i < 61; i++) { tank_item_battle->tik_logic(); }
    tank_item_battle->process_key_click(right, 0);
    tank_item_battle->process_key_click(right, 0);
    tank_item_battle->process_key_click(0, 1);
    tank_item_battle->process_key_click(0, 1);
    assert(battle_tank.inventory[0] == 0 && battle_tank.sp > 10);
    pop();
    battle_tank.inventory[0] = 65;
    old_weapon = battle_tank.equipment[0];
    auto *tank_equip_battle = new battle_render(1);
    push(tank_equip_battle);
    for (int i = 0; i < 61; i++) { tank_equip_battle->tik_logic(); }
    tank_equip_battle->process_key_click(right, 0);
    tank_equip_battle->process_key_click(right, 0);
    tank_equip_battle->process_key_click(right, 0);
    tank_equip_battle->process_key_click(0, 1);
    tank_equip_battle->process_key_click(0, 1);
    assert(battle_tank.equipment[0] == 65 && battle_tank.inventory[0] == old_weapon);
    pop();
    game_state::exit_tank();
    // A resolved battle awards exactly once, then returns to the same map.
    game_state::get().players[0].attack=5000;auto *battle=new battle_render(25);push(battle);
    for (int i=0;i<61;i++) { battle->tik_logic(); }
    auto before=game_state::get().money;battle->process_key_click(0,1);assert(game_state::get().money==before+game_data::monster(25).gold);
    battle->process_key_click(0,1);assert(top()==map);
    // Fully offscreen opaque blits used to pass negative lengths to memcpy.
    uint8_t pixels[256]={};bitmap_render::render_bitmap(pixels,16,16,999,0,screen.data());
    bitmap_render::render_bitmap(pixels,16,16,-999,-999,screen.data());
    release_logic_thread();assert(!top());
    // Decode every shipped legacy image and every map cell through the C++
    // runtime for independent source-byte comparison in verify_bin2c.py.
    f=fopen((std::string(argv[1])+"/decoded-images.bin").c_str(),"wb");assert(f);
    for (unsigned id=0;id<2041;id++) {
        unsigned w,h;game_data::image_size(id,w,h);std::vector<uint8_t> pixels(w*h);
        game_data::decode_image(id,pixels.data());uint16_t wh[2]={uint16_t(w),uint16_t(h)};
        fwrite(wh,sizeof(wh),1,f);fwrite(pixels.data(),pixels.size(),1,f);
    }
    fclose(f);
    f=fopen((std::string(argv[1])+"/decoded-maps.bin").c_str(),"wb");assert(f);
    for (unsigned mid=0;mid<240;mid++) {
        for (unsigned y=0;y<map_size[mid*2];y++) { for (unsigned x=0;x<map_size[mid*2+1];x++) {
            uint16_t tile=game_data::map_tile(mid,x,y);uint8_t attr=game_data::map_attribute(mid,x,y);
            fwrite(&tile,2,1,f);fwrite(&attr,1,1,f);
        } }
    }
    fclose(f);
    printf("MEMORY State=%zu Player=%zu Npc=%zu ScriptMachine=%zu Character=%zu packed=%u\n",sizeof(game_state::state),sizeof(game_state::player),sizeof(map_npcs::npc),sizeof(script_machine),sizeof(character),game_data::packed_bytes());
    printf("HOST_RUNTIME_OK poses=%u sprite_models=%u player_model=9 npc_walk=ok dialogue=ok saves=ok map_renders=240 battle=ok shop=ok splash=ok entrance_t=ok collision=ok\n",
           poses, model_cases);
}
