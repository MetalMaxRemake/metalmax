#include "script_machine.h"
#include "status/game_state.h"
#include "status/character.h"
#include "story_runtime.h"
#include "../global.h"
#include <algorithm>

namespace metalmax {
    void script_machine::start(bool dynamic, unsigned id) {
        script = {};
        stopped = true;
        pc = 0;
        delay = 0;
        move_remaining = 0;
        move_pc = -1;
        loop_depth = 0;
        yes = false;
        if ((dynamic && id >= 169) || (!dynamic && (id == 0 || id > 108))) { return; }
        script = game_data::script(dynamic, id);
        stopped = false;
    }

    bool script_machine::awaiting_yes() const {
        return !stopped && script.count && pc < script.count
               && game_data::script_node_at(script.offset + pc).op == game_data::k_if_yes;
    }

    script_machine::result script_machine::tick(script_host &host) {
        using namespace game_data;
        if (stopped || !script.count) { return k_done; }
        if (delay > 0) {
            delay--;
            return k_yield;
        }
        if (host.busy()) { return k_yield; }
        for (unsigned budget = 0; budget < 32; budget++) {
            if (pc >= script.count) {
                stopped = true;
                return k_done;
            }
            const auto &n = script_node_at(script.offset + pc);
            bool condition = true;
            bool conditional = false;
            bool loop_if = false;
            switch (n.op) {
                case game_data::k_stop:
                    stopped = true;
                    return k_done;
                case game_data::k_text:
                    page = n.a;
                    row = n.b;
                    pc++;
                    return script_machine::k_text;
                case k_event_text:
                    page = n.b;
                    row = game_state::event(n.a) ? n.c : n.d;
                    pc++;
                    return script_machine::k_text;
                case k_set_event:
                    game_state::set_event(n.a, true);
                    break;
                case k_clear_event:
                    game_state::set_event(n.a, false);
                    break;
                case k_if_event:
                    condition = game_state::event(n.a);
                    conditional = true;
                    break;
                case k_if_position:
                    condition = get_default_player()->x == n.a && get_default_player()->y == n.b;
                    conditional = true;
                    break;
                case k_if_yes:
                    condition = yes;
                    conditional = true;
                    break;
                case k_if_team:
                    condition = n.a >= 1 && n.a <= 3 && game_state::get().players[n.a - 1].active;
                    conditional = true;
                    break;
                case k_if_dead:
                    condition = n.a >= 1 && n.a <= 3 && game_state::get().players[n.a - 1].hp <= 0;
                    conditional = true;
                    break;
                case k_if_level:
                    condition = game_state::get().players[0].level >= n.a;
                    conditional = true;
                    break;
                case k_if_money:
                    condition = n.a >= 0 && n.a < 256 && game_state::get().money >= value(n.a);
                    conditional = true;
                    break;
                case k_if_item:
                    condition = game_state::has_item(n.a);
                    conditional = true;
                    break;
                case k_if_treasure:
                    condition = n.a >= 0 && n.a < game_data::treasure_count &&
                                (game_state::get().treasures[n.a / 8] & (1 << (n.a % 8)));
                    conditional = true;
                    break;
                case k_if_area: {
                    auto *player = get_default_player();
                    int y2 = n.d & 127;
                    condition = player->x >= n.a && player->x <= n.b && player->y >= n.c &&
                                player->y <= y2;
                    conditional = true;
                    loop_if = (n.d & 128) != 0;
                    break;
                }
                case k_if_face:
                    condition = get_default_player()->direct == (n.a & 3);
                    conditional = true;
                    break;
                case k_if_detector:
                    condition = story_runtime::detector(host.x(), host.y(), n.a);
                    conditional = true;
                    break;
                case k_if_hp:
                    condition = story_runtime::hp_at_least(n.a & 255);
                    conditional = true;
                    break;
                case k_if_drank:
                    condition = story_runtime::drank();
                    conditional = true;
                    break;
                case k_if_ride_tank:
                    condition = story_runtime::ride_tank();
                    conditional = true;
                    break;
                case k_if_tank_riding:
                    condition = story_runtime::tank_riding(n.a);
                    conditional = true;
                    break;
                case k_if_tank_here:
                    condition = story_runtime::tank_here(n.a);
                    conditional = true;
                    break;
                case k_if_has_ok_tank:
                    condition = story_runtime::has_ok_tank();
                    conditional = true;
                    break;
                case game_data::k_jump:
                    if (n.target < 0 || n.target >= script.count) {
                        stopped = true;
                        return k_unsupported;
                    }
                    pc = n.target;
                    continue;
                case k_loop:
                    if (!loop_depth || loops[loop_depth - 1].pc != pc) {
                        if (loop_depth >= 8) {
                            stopped = true;
                            return k_unsupported;
                        }
                        loops[loop_depth++] = {pc, uint16_t(std::max<int16_t>(1, n.a))};
                    }
                    if (--loops[loop_depth - 1].remaining > 0) {
                        pc = n.target;
                        continue;
                    }
                    loop_depth--;
                    break;
                case k_wait:
                    delay = std::max<int16_t>(1, n.a);
                    pc++;
                    return k_yield;
                case k_wait_set:
                    if (!game_state::event(n.a)) { return k_yield; }
                    break;
                case k_wait_clear:
                    if (game_state::event(n.a)) {
                        host.hide();
                        return k_yield;
                    }
                    host.show();
                    break;
                case k_face:
                    host.face(n.a);
                    break;
                case k_face_player:
                    host.face_player();
                    break;
                case k_face_back:
                    host.face_back();
                    break;
                case k_move:
                    if (move_pc != static_cast<int>(pc)) {
                        move_pc = pc;
                        move_remaining = std::max<int16_t>(1, n.b);
                    }
                    if (move_remaining > 0) {
                        host.move(n.a);
                        move_remaining--;
                        return k_yield;
                    }
                    move_pc = -1;
                    break;
                case k_move_to:
                    if (!host.move_to(n.a, n.b)) { return k_yield; }
                    break;
                case k_teleport:
                    host.teleport(n.a, n.b);
                    break;
                case k_wander:
                    host.wander(n.a, n.b, n.c, n.d);
                    pc++;
                    delay = 48;
                    return k_yield;
                case k_chase:
                    host.move_to(get_default_player()->x, get_default_player()->y);
                    pc++;
                    delay = 24;
                    return k_yield;
                case k_animate:
                    host.animate();
                    pc++;
                    delay = 16;
                    return k_yield;
                case k_hide:
                    host.hide();
                    break;
                case k_change_model:
                    host.change_model(n.a, n.b != 0);
                    break;
                case k_set_frame:
                    host.set_frame(n.a);
                    break;
                case k_set_attrs:
                    host.set_attrs(n.a);
                    break;
                case k_sprite_bank:
                    host.set_bank(n.a);
                    break;
                case k_model_delta:
                    host.model_delta(n.a);
                    break;
                case k_offset:
                    host.offset(n.a);
                    break;
                case k_recover:
                    for (auto &p: game_state::get().players) { if (p.active) { p.hp = p.max_hp; }}
                    break;
                case k_spend:
                    if (n.a >= 0 && n.a < 256) { game_state::spend(value(n.a)); }
                    break;
                case k_recruit:
                    if (n.a >= 1 && n.a <= 3) { game_state::get().players[n.a - 1].active = 1; }
                    break;
                case game_data::k_battle:
                    monster = n.a;
                    battle_event = n.b;
                    pc++;
                    return script_machine::k_battle;
                case k_control:
                    story_runtime::toggle_control();
                    break;
                case k_music:
                    story_runtime::play_music(n.a);
                    break;
                case k_menu:
                    story_runtime::open_menu(n.a, n.b);
                    pc++;
                    return k_yield;
                case k_player_show:
                    story_runtime::player_show(n.a);
                    break;
                case k_player_hide:
                    story_runtime::player_hide();
                    break;
                case k_player_hide_all:
                    story_runtime::player_hide_all();
                    break;
                case k_player_become:
                    story_runtime::player_become(n.a);
                    break;
                case k_player_teleport:
                    story_runtime::player_teleport(n.a, n.b, n.c, n.d != 0);
                    pc++;
                    return k_yield;
                case k_player_face:
                    story_runtime::player_face(n.a);
                    break;
                case k_player_pull:
                    story_runtime::player_pull(n.a);
                    break;
                case k_player_unpull:
                    story_runtime::player_unpull();
                    break;
                case k_scroll:
                    story_runtime::scroll(n.a, n.b);
                    pc++;
                    delay = std::max<int16_t>(1, n.b * 8);
                    return k_yield;
                case k_scene:
                    story_runtime::scene(n.a);
                    break;
                case k_scene_end:
                    story_runtime::scene_end();
                    break;
                case k_quake:
                    story_runtime::quake(n.a);
                    pc++;
                    delay = std::max<int16_t>(1, n.a);
                    return k_yield;
                case k_flash:
                    story_runtime::flash(n.a);
                    pc++;
                    delay = std::max<int16_t>(1, n.a);
                    return k_yield;
                case k_sleep:
                    story_runtime::sleep(n.a);
                    pc++;
                    return k_yield;
                case k_next_day:
                    story_runtime::next_day();
                    break;
                case k_code:
                    story_runtime::code(n.a);
                    break;
                case k_draw_tile:
                    story_runtime::draw_tile(host.current_map(), host.x(), host.y(), n.a);
                    break;
                case k_open_door:
                    story_runtime::open_door(host.current_map(), host.x(), host.y(), host.facing());
                    break;
                case k_npc_play:
                    host.play(n.a);
                    pc++;
                    delay = 16;
                    return k_yield;
                case k_npc_patrol:
                    host.patrol(n.a);
                    pc++;
                    delay = 16;
                    return k_yield;
                case k_npc_enter_tank:
                    host.enter_tank();
                    break;
                case k_npc_exit_tank:
                    host.exit_tank(n.a);
                    break;
                case k_npc_become:
                    host.become_player(n.a);
                    break;
                case k_npc_throw:
                    host.throw_anim(n.a, n.b);
                    pc++;
                    delay = 24;
                    return k_yield;
                case k_npc_explode:
                    host.explode();
                    pc++;
                    delay = 20;
                    return k_yield;
                case k_npc_hurt:
                    host.hurt();
                    break;
                case k_move_tank:
                    if (!host.move_tank()) { return k_yield; }
                    break;
                case k_give_item:
                    story_runtime::give_item(n.a, n.b);
                    break;
                case k_swap_item:
                    story_runtime::swap_item(n.a, n.b);
                    break;
                case k_team_join:
                    story_runtime::team_join(n.a);
                    break;
                case k_team_leave:
                    story_runtime::team_leave(n.a);
                    break;
                case k_team_hide:
                    story_runtime::team_hide();
                    break;
                case k_system:
                    story_runtime::system(n.a);
                    break;
                case k_conveyor:
                    story_runtime::conveyor(n.a);
                    break;
                case k_change_tank_map:
                    story_runtime::change_tank_map(n.a, n.b);
                    break;
                case k_medal:
                    story_runtime::medal();
                    break;
                case k_henshin:
                    story_runtime::henshin();
                    break;
                default:
                    log_w("ScriptMachine", "Unsupported op %u at node %u; script stopped", n.a, pc);
                    stopped = true;
                    return k_unsupported;
            }
            if (conditional && !condition) {
                bool should_loop = loop_if || (n.op != k_if_area && n.c);
                if (should_loop) { return k_yield; }
                if (n.target < 0 || n.target >= script.count) {
                    stopped = true;
                    return k_unsupported;
                }
                pc = n.target;
                continue;
            }
            pc++;
        }
        return k_yield;
    }
} // namespace metalmax
