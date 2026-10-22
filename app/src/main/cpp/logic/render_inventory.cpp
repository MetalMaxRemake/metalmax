#include "render_inventory.h"
#include "render_dialogue.h"
#include "status/game_state.h"
#include "../charset/utf8_text.h"
#include "../graphic/palette_data.h"
#include <cstring>
#include <cstdio>
#include <algorithm>

namespace metalmax {
uint8_t *inventory_render::render(uint8_t *screen) {
    memset(screen+48*global_config::k_screen_width,palette::k_palette_black,208*global_config::k_screen_width);
    render_utf_8_text(screen,equipment ? "选择装备" : "物品：A 使用/装备  B 返回",16,56,423,16);
    int first=selected/10*10;
    for (int row=0;row<10 && first+row<game_state::k_inventory_size;row++) {
        unsigned id=game_state::get().inventory[first+row];char line[128];
        snprintf(line,sizeof(line),"%c %02d  %s",first+row==selected ? '>' : ' ',first+row+1,id ? game_data::item(id).name : "空");
        render_utf_8_text(screen,line,16,80+row*16,423,16);
    }
    return screen;
}
void inventory_render::process_key_click(uint8_t direct,uint8_t function) {
    if (direct&up) { selected=std::max(0,selected-1); }
    if (direct&down) { selected=std::min(static_cast<int>(game_state::k_inventory_size)-1,selected+1); }
    if (function&b) { pop();return; }
    if (function&a) {
        if (game_state::equip(selected)) { push(new dialogue_render("装备完成。")); }
        else if (!equipment && game_state::use_healing_item(selected)) { push(new dialogue_render("生命恢复了。")); }
        else { push(new dialogue_render("现在不能使用这件物品。")); }
    }
}
uint8_t *status_render::render(uint8_t *screen) {
    memset(screen+32*global_config::k_screen_width,palette::k_palette_black,224*global_config::k_screen_width);
    char line[180];snprintf(line,sizeof(line),"金钱 %u",game_state::get().money);render_utf_8_text(screen,line,16,40,423,16);
    int y=64;
    for (unsigned i=0;i<3;i++) {
        const auto &p=game_state::get().players[i];if (!p.active) { continue; }
        snprintf(line,sizeof(line),"队员 %u  等级 %d  HP %d/%d",i+1,p.level,p.hp,p.max_hp);render_utf_8_text(screen,line,16,y,423,16);
        snprintf(line,sizeof(line),"攻击 %d  防御 %d  经验 %u",p.attack,p.defense,p.experience);render_utf_8_text(screen,line,16,y+16,423,16);y+=48;
    }
    render_utf_8_text(screen,"任意功能键返回",16,232,423,16);return screen;
}
} // namespace metalmax
