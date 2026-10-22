#include "map_npcs.h"
#include "../maps/map_data/map_data.h"
#include "../graphic/bitmap_render.h"
#include "../logic/status/character.h"
#include "../logic/render_dialogue.h"
#include "../logic/render_battle.h"
#include <vector>
#include <cstdlib>
#include <algorithm>

namespace metalmax::map_npcs {
static std::vector<npc> actors;
bool passable(int map,int x,int y) {
    if (map<0 || map>=240 || x<0 || y<0 || x>=map_size[map*2+1] || y>=map_size[map*2]) { return false; }
    unsigned a=game_data::map_attribute(map,x,y);
    // Doors are terrain triggers and can be walked through. Other bit7 tiles
    // are solid, retaining the original map's logical attributes after dedup.
    return !(a&128) || (a&252)==0x3c;
}
bool blocks(int x,int y,const npc *except) {
    for (const auto &npc:actors) {
        if (&npc!=except && npc.visible && npc.model && npc.tile_x==x && npc.tile_y==y) { return true; }
    }
    return false;
}
npc *at(int x,int y) {
    for (auto &npc:actors) {
        if (npc.visible && npc.model && npc.tile_x==x && npc.tile_y==y) { return &npc; }
    }
    return nullptr;
}
bool npc::move(int dir) {
    return move(dir,2,1);
}
bool npc::move(int dir,uint8_t delay,uint8_t pixels) {
    if (moving || dir<0 || dir>3) { return false; }
    static const int dx[4]={0,0,-1,1},dy[4]={-1,1,0,0};
    int xx=tile_x+dx[dir],yy=tile_y+dy[dir];face(dir);
    if (xx<0 || yy<0 || xx>=map_size[map*2+1] || yy>=map_size[map*2]) { return false; }
    if (!(attributes&0x10) && !passable(map,xx,yy)) { return false; }
    auto *p=get_default_player();
    if ((p->x==xx && p->y==yy) || blocks(xx,yy,this)) { return false; }
    const auto &range_t=game_data::map_entrances(map);
    for (unsigned i=0;i<range_t.count;i++) {
        const auto &e=game_data::entrance_at(range_t.offset+i);if (e.x==xx && e.y==yy) { return false; }
    }
    tile_x=xx;tile_y=yy;moving=true;visible=true;
    movement_delay=std::max<uint8_t>(1,delay);
    movement_pixels=std::max<uint8_t>(1,pixels);
    movement_ticks=0;
    return true;
}
bool npc::move_to(int xx,int yy) {
    if (tile_x==xx && tile_y==yy) { return true; }
    if (xx<0 || yy<0 || xx>=map_size[map*2+1] || yy>=map_size[map*2]) { return true; }
    if (tile_x!=xx && move(tile_x<xx ? 3 : 2)) { return false; }
    if (tile_y!=yy) { move(tile_y<yy ? 1 : 0); }
    return false;
}
void npc::teleport(int xx,int yy) {
    tile_x=xx;tile_y=yy;render_x=xx*16;render_y=yy*16;moving=false;
    movement_ticks=0;
    visible=xx>=0 && yy>=0 && xx<map_size[map*2+1] && yy<map_size[map*2];
}
void npc::face(int dir) { if (!(attributes&128)) { direction=dir&3; } }
void npc::face_player() {
    auto *p=get_default_player();int dx=p->x-tile_x,dy=p->y-tile_y;
    face(abs(dx)>abs(dy) ? (dx>0 ? 3 : 2) : (dy>0 ? 1 : 0));
}
void npc::face_back() { static const int opposite[4]={1,0,3,2};face(opposite[direction]); }
void npc::animate() { animation_ticks=16; }
void npc::change_model(int id,bool whole_type) {
    model=whole_type ? id>>2 : id;model&=63;
    if (whole_type) { face(id&3); }
}
void npc::set_frame(int value) { frame=value&1;animation_ticks=16; }
void npc::offset(int value) {
    int dx=(value>>4)&15,dy=value&15;teleport(tile_x+(dx>=8 ? dx-16 : dx),tile_y+(dy>=8 ? dy-16 : dy));
}
void npc::wander(int x1,int y1,int x2,int y2) {
    int dir=rand()%4;static const int dx[4]={0,0,-1,1},dy[4]={-1,1,0,0};
    if (x1>=0 && (tile_x+dx[dir]<x1 || tile_x+dx[dir]>=x2 || tile_y+dy[dir]<y1 || tile_y+dy[dir]>=y2)) { return; }
    move(dir);
}
void load(int map) {
    actors.clear();if (map<0 || map>=240) { return; }
    const auto &range_t=game_data::map_npcs(map);actors.reserve(range_t.count);
    for (unsigned i=0;i<range_t.count;i++) {
        npc n;n.data=game_data::npc(range_t.offset+i);n.map=map;
        n.direction=n.data.type&3;n.model=n.data.type>>2;n.model_type=n.data.talk1&3;
        n.attributes=(n.data.x&192)|((n.data.y&192)>>2);n.bank=game_data::map_sprite_bank(map);
        n.teleport(n.data.x&63,n.data.y&63);n.script.start(true,n.data.action);
        actors.push_back(n);
    }
}
void tick() {
    for (auto &npc:actors) {
        if (npc.talking) { continue; }
        if (npc.moving) {
            int dx=npc.tile_x*16-npc.render_x,dy=npc.tile_y*16-npc.render_y;
            if (++npc.movement_ticks>=npc.movement_delay) {
                npc.movement_ticks=0;
                if (dx) {
                    int step=std::min<int>(npc.movement_pixels,abs(dx));
                    npc.render_x+=dx>0 ? step : -step;
                } else if (dy) {
                    int step=std::min<int>(npc.movement_pixels,abs(dy));
                    npc.render_y+=dy>0 ? step : -step;
                }
            }
            int remaining=abs(npc.tile_x*16-npc.render_x)+abs(npc.tile_y*16-npc.render_y);
            npc.frame=remaining>8 ? 1 : 0;
            npc.moving=remaining!=0;
        }
        if (npc.animation_ticks>0) { npc.animation_ticks--;npc.frame=(npc.animation_ticks/8)&1; }
        if (!npc.moving && !npc.animation_ticks) { npc.frame=0; }
        if (npc.attributes&64) { npc.frame=0; }
        if (!npc.script.done()) {
            auto result=npc.script.tick(npc);
            if (result==script_machine::k_text) {
                push(new dialogue_render(npc.script.page,npc.script.row));return;
            }
            if (result==script_machine::k_battle) {
                push(new battle_render(npc.script.monster,false,npc.script.battle_event));return;
            }
        }
    }
}
void render(uint8_t *buffer,int cx,int cy) {
    for (const auto &npc:actors) {
        if (!npc.visible || !npc.model) { continue; }
        int x=npc.render_x-cx,y=npc.render_y-cy;
        game_data::draw_sprite(npc.bank,npc.model,npc.direction,npc.frame,npc.model_type,x,y,buffer);
    }
}
bool push_npc(int x,int y,int dir) {
    npc *npc=at(x,y);return npc && (npc->attributes&32) && npc->move(dir,1,2);
}
int count() { return actors.size(); }
npc *get(int index) { return index>=0 && index<static_cast<int>(actors.size()) ? &actors[index] : nullptr; }
}
