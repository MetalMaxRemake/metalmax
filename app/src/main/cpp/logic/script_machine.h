#ifndef metalmax_script_machine_h
#define metalmax_script_machine_h
#include "../data/game_data.h"
namespace metalmax {
struct script_host {
    virtual bool move(int direction) { return false; }
    virtual bool move_to(int x,int y) { return false; }
    virtual void teleport(int x,int y) {}
    virtual void face(int direction) {}
    virtual void face_player() {}
    virtual void face_back() {}
    virtual void animate() {}
    virtual void hide() {}
    virtual void show() {}
    virtual void change_model(int model,bool whole_type) {}
    virtual void set_frame(int frame) {}
    virtual void offset(int xy) {}
    virtual void set_attrs(int attrs) {}
    virtual void set_bank(int bank) {}
    virtual void model_delta(int delta) {}
    virtual void wander(int x1,int y1,int x2,int y2) {}
    virtual bool busy() const { return false; }
    virtual int x() const { return 0; }
    virtual int y() const { return 0; }
    virtual ~script_host() {}
};
class script_machine {
public:
    enum result { k_yield,k_text,k_battle,k_done,k_unsupported };
    void start(bool dynamic,unsigned id);
    result tick(script_host &host);
    void answer(bool value) { yes=value; }
    uint8_t page=0,row=0,monster=0,battle_event=0;
    bool done() const { return stopped; }
    bool awaiting_yes() const;
private:
    game_data::script_data_t script={};
    uint16_t pc=0;
    uint16_t delay=0;
    int16_t move_remaining=0,move_pc=-1;
    struct loop_state { uint16_t pc,remaining; } loops[8];
    uint8_t loop_depth=0;
    bool stopped=true,yes=false;
};
} // namespace metalmax
#endif
