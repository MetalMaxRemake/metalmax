#ifndef metalmax_render_dialogue_h
#define metalmax_render_dialogue_h
#include "render.h"
#include "script_machine.h"
#include "../sprite/map_npcs.h"
#include <string>
namespace metalmax {
class dialogue_render : public base_render {
public:
    explicit dialogue_render(const char *text);
    dialogue_render(int page,int row);
    explicit dialogue_render(unsigned script,map_npcs::npc *npc=nullptr);
    uint8_t *render(uint8_t *screen) override;
    void process_key_click(uint8_t direct,uint8_t function) override;
    void on_close() override;
    void tik_logic() override;
private:
    void advance();
    std::string owned_message; // only messages formatted on a caller's stack
    const char *message="";  // ROM text directly borrows read-only bin2c pool
    unsigned offset=0,consumed=0;
    bool scripted=false,finished=false;
    bool waiting=false;
    script_machine machine;
    map_npcs::npc *npc=nullptr;
};
} // namespace metalmax
#endif
