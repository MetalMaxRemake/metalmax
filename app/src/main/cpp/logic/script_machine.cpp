#include "script_machine.h"
#include "status/game_state.h"
#include "status/character.h"
#include "../global.h"
#include <algorithm>

namespace metalmax {
void script_machine::start(bool dynamic,unsigned id) {
    script={};stopped=true;pc=0;delay=0;move_remaining=0;move_pc=-1;loop_depth=0;yes=false;
    if ((dynamic && id>=169) || (!dynamic && (id==0 || id>108))) { return; }
    script=game_data::script(dynamic,id);stopped=false;
}
bool script_machine::awaiting_yes() const {
    return !stopped && script.count && pc<script.count
        && game_data::script_node_at(script.offset+pc).op==game_data::k_if_yes;
}
script_machine::result script_machine::tick(script_host &host) {
    using namespace game_data;
    if (stopped || !script.count) { return k_done; }
    if (delay>0) { delay--;return k_yield; }
    if (host.busy()) { return k_yield; }
    for (unsigned budget=0;budget<32;budget++) {
        if (pc>=script.count) { stopped=true;return k_done; }
        const auto &n=script_node_at(script.offset+pc);bool condition=true;
        switch (n.op) {
            case game_data::k_stop: stopped=true;return k_done;
            case game_data::k_text: page=n.a;row=n.b;pc++;return script_machine::k_text;
            case k_event_text: page=n.b;row=game_state::event(n.a) ? n.c : n.d;pc++;return script_machine::k_text;
            case k_set_event: game_state::set_event(n.a,true);break;
            case k_clear_event: game_state::set_event(n.a,false);break;
            case k_if_event: condition=game_state::event(n.a);break;
            case k_if_position: condition=get_default_player()->x==n.a && get_default_player()->y==n.b;break;
            case k_if_yes: condition=yes;break;
            case k_if_team: condition=n.a>=1 && n.a<=3 && game_state::get().players[n.a-1].active;break;
            case k_if_dead: condition=n.a>=1 && n.a<=3 && game_state::get().players[n.a-1].hp<=0;break;
            case k_if_level: condition=game_state::get().players[0].level>=n.a;break;
            case k_if_money: condition=n.a>=0 && n.a<256 && game_state::get().money>=value(n.a);break;
            case k_if_item: condition=game_state::has_item(n.a);break;
            case k_if_treasure: condition=n.a>=0 && n.a<game_data::treasure_count && (game_state::get().treasures[n.a/8]&(1<<(n.a%8)));break;
            case game_data::k_jump: if (n.target<0 || n.target>=script.count) { stopped=true;return k_unsupported; }pc=n.target;continue;
            case k_loop:
                if (!loop_depth || loops[loop_depth-1].pc!=pc) {
                    if (loop_depth>=8) { stopped=true;return k_unsupported; }
                    loops[loop_depth++]={pc,uint16_t(std::max<int16_t>(1,n.a))};
                }
                if (--loops[loop_depth-1].remaining>0) { pc=n.target;continue; }
                loop_depth--;break;
            case k_wait: delay=std::max<int16_t>(1,n.a);pc++;return k_yield;
            case k_wait_set: if (!game_state::event(n.a)) { return k_yield; }break;
            case k_wait_clear: if (game_state::event(n.a)) { host.hide();return k_yield; }host.show();break;
            case k_face: host.face(n.a);break;
            case k_face_player:host.face_player();break;
            case k_face_back:host.face_back();break;
            case k_move:
                if (move_pc!=static_cast<int>(pc)) { move_pc=pc;move_remaining=std::max<int16_t>(1,n.b); }
                if (move_remaining>0) { host.move(n.a);move_remaining--;return k_yield; }
                move_pc=-1;break;
            case k_move_to: if (!host.move_to(n.a,n.b)) { return k_yield; }break;
            case k_teleport:host.teleport(n.a,n.b);break;
            case k_wander:host.wander(n.a,n.b,n.c,n.d);pc++;delay=48;return k_yield;
            case k_chase:host.move_to(get_default_player()->x,get_default_player()->y);pc++;delay=24;return k_yield;
            case k_animate:host.animate();pc++;delay=16;return k_yield;
            case k_hide:host.hide();break;
            case k_change_model:host.change_model(n.a,n.b!=0);break;
            case k_set_frame:host.set_frame(n.a);break;
            case k_set_attrs:host.set_attrs(n.a);break;
            case k_sprite_bank:host.set_bank(n.a);break;
            case k_model_delta:host.model_delta(n.a);break;
            case k_offset:host.offset(n.a);break;
            case k_recover:
                for (auto &p:game_state::get().players) { if (p.active) { p.hp=p.max_hp; } }break;
            case k_spend:if (n.a>=0 && n.a<256) { game_state::spend(value(n.a)); }break;
            case k_recruit:
                if (n.a>=1 && n.a<=3) { game_state::get().players[n.a-1].active=1; }break;
            case game_data::k_battle: monster=n.a;battle_event=n.b;pc++;return script_machine::k_battle;
            default:log_w("ScriptMachine","Unsupported op %u at node %u; script stopped",n.a,pc);stopped=true;return k_unsupported;
        }
        if (n.op>=k_if_event && n.op<=k_if_treasure && !condition) {
            if (n.c) { return k_yield; }
            if (n.target<0 || n.target>=script.count) { stopped=true;return k_unsupported; }
            pc=n.target;continue;
        }
        pc++;
    }
    return k_yield;
}
} // namespace metalmax
