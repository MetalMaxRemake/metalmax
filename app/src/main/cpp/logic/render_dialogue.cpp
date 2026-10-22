#include "render_dialogue.h"
#include "render_battle.h"
#include "../charset/utf8_text.h"
#include "../graphic/palette_data.h"
#include <cstring>

namespace metalmax {
dialogue_render::dialogue_render(const char *text):owned_message(text ? text : "") { message=owned_message.c_str(); }
dialogue_render::dialogue_render(int page,int row):message(game_data::text(page,row)) {}
dialogue_render::dialogue_render(unsigned id,map_npcs::npc *actor):scripted(true),npc(actor) {
    if (npc) { npc->talking=true;npc->face_player(); }
    machine.start(false,id);waiting=true;
}
void dialogue_render::advance() {
    script_host dummy;auto result=machine.tick(npc ? static_cast<script_host&>(*npc) : dummy);
    offset=0;
    waiting=result==script_machine::k_yield;
    if (result==script_machine::k_text) { message=game_data::text(machine.page,machine.row); }
    else if (result==script_machine::k_battle) {
        message="准备战斗。";push(new battle_render(machine.monster,false,machine.battle_event));
    } else if (result==script_machine::k_unsupported) { message="这项功能尚未开放。";finished=true; }
    else if (result==script_machine::k_done) { message="";finished=true; }
    else { message="请稍候。"; }
}
uint8_t *dialogue_render::render(uint8_t *screen) {
    memset(screen+184*global_config::k_screen_width,palette::k_palette_black,72*global_config::k_screen_width);
    consumed=render_utf_8_text(screen,message+offset,16,192,423,40);
    render_utf_8_text(screen,machine.awaiting_yes() ? "A 是  B 否" : "A 继续  B 结束",16,240,423,14);return screen;
}
void dialogue_render::process_key_click(uint8_t,uint8_t function) {
    if (!(function&(a|b))) { return; }
    if (finished) { pop();return; }
    if (function&a && offset+consumed<strlen(message) && consumed) { offset+=consumed;return; }
    if (!scripted) { pop();return; }
    if ((function&b) && !machine.awaiting_yes()) { pop();return; }
    machine.answer(function&a);advance();if (finished && !*message) { pop(); }
}
void dialogue_render::tik_logic() {
    if (waiting && !finished) { advance(); }
}
void dialogue_render::on_close() { if (npc) { npc->talking=false; } }
} // namespace metalmax
