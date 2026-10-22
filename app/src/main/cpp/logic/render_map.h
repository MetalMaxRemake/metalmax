//
// Created by parkyu on 2023/5/11.
//

#ifndef metalmax_render_map_h
#define metalmax_render_map_h

#include "logic.h"
#include "render.h"
#include "status/character.h"

namespace metalmax {
class map_render : public base_render {
private:
    int entrance_animation = 0;
    int next_map_id = -1;

    bool may_enter_another_map(character *player, int target_x, int target_y);

    bool enter_another_map(character *player);

    bool may_exit_to_last_map(character *player, int target_x, int target_y);

    void refresh_music() const;

    void trigger_monster() const;

    void trigger_door(character *player) const;

    void render_sprite(uint8_t *screen_buffer) const;

    void render_tanks(uint8_t *screen_buffer) const;

    void render_water(uint8_t *screen_buffer) const;

    void reset_palette() const;

    int map_id = 0;
    int camera_x = 0, camera_y = 0;
    int next_x = 0, next_y = 0;
    unsigned encounter_cooldown = 30;

    void render_players(uint8_t *screen_buffer, character *player) const;

public:
    map_render();

    void update_map(int new_map_id, int x, int y);

    int get_map_id();

    uint8_t *render(uint8_t *screen_buffer) override;

    void tik_logic() override;

    bool process_key(uint8_t direct_key, uint8_t function_key) override;

    void process_key_click(uint8_t direct_key, uint8_t function_key) override;

    void on_focus() override;

    void on_un_focus() override;

    ~map_render();
};

} // namespace metalmax
#endif //METALMAX_RENDER_MAP_H
