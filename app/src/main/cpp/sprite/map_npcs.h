#ifndef metalmax_map_npcs_h
#define metalmax_map_npcs_h
#include "../logic/script_machine.h"
namespace metalmax {
namespace map_npcs {
struct npc : script_host {
    game_data::npc_data_t data={};
    script_machine script;
    int16_t tile_x=0,tile_y=0,render_x=0,render_y=0;
    uint8_t map=0;
    uint8_t direction=0,frame=0,model=0,model_type=0,animation_ticks=0;
    uint8_t movement_delay=2,movement_ticks=0,movement_pixels=1;
    uint8_t attributes=0,bank=100;
    bool visible=true,moving=false,talking=false;
    bool move(int dir) override;
    bool move(int dir,uint8_t delay,uint8_t pixels);
    bool move_to(int x,int y) override;
    void teleport(int x,int y) override;
    void face(int dir) override;
    void face_player() override;
    void face_back() override;
    void animate() override;
    void hide() override { visible=false; }
    void show() override { visible=true; }
    void change_model(int id,bool whole_type) override;
    void set_frame(int value) override;
    void offset(int value) override;
    void set_attrs(int value) override { attributes=value; }
    void set_bank(int value) override { bank=value; }
    void model_delta(int value) override { model=(model+value)&63; }
    void wander(int x1,int y1,int x2,int y2) override;
    bool busy() const override { return moving; }
    int x() const override { return tile_x; }
    int y() const override { return tile_y; }
};
void load(int map);
void tick();
void render(uint8_t *buffer,int camera_x,int camera_y);
npc *at(int x,int y);
bool blocks(int x,int y,const npc *except=nullptr);
bool push_npc(int x,int y,int direction);
bool passable(int map,int x,int y);
int count();
npc *get(int index);
}
} // namespace metalmax
#endif
