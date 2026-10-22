//
// Created by parkyu on 2023/5/12.
//

#ifndef metalmax_render_battle_h
#define metalmax_render_battle_h

#include "logic.h"
#include "render.h"

namespace metalmax {
class battle_render : public base_render {
private:
    volatile bool enter_battle = false;
    volatile int splash_clk = 0;
    volatile int splash_count = 0;
    int monster_idx = 0;
public:
    battle_render(int monster_id, bool preview = false, int event = -1);

    uint8_t *render(uint8_t *screen_buffer) override;

    void tik_logic() override;

    void process_key_click(uint8_t direct_key, uint8_t function_key) override;

    void on_focus() override;

private:
    bool preview = false, resolved = false, victory = false;
    int enemy_hp = 1, selected = 0, result_event = -1;
    char message[256] = {};

    void turn(int action);

    void finish(bool win);

    bool used_inventory = false;
    int inventory_hp = 0;
};

} // namespace metalmax
#endif //METALMAX_RENDER_BATTLE_H
