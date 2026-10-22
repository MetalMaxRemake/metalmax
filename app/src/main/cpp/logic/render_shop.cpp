#include "render_shop.h"
#include "render_dialogue.h"
#include "status/game_state.h"
#include "../charset/utf8_text.h"
#include "../graphic/palette_data.h"
#include <cstring>
#include <cstdio>
#include <algorithm>

namespace metalmax {
shop_render::shop_render(int type,int page):data(game_data::shop(type,page)) {}
uint8_t *shop_render::render(uint8_t *screen) {
    memset(screen+32*global_config::k_screen_width,palette::k_palette_black,224*global_config::k_screen_width);
    char line[160];snprintf(line,sizeof(line),"商店  金钱 %u  A 购买  B 返回",game_state::get().money);render_utf_8_text(screen,line,16,40,423,16);
    if (!data.count) { render_utf_8_text(screen,"没有商品。",16,72,423,16);return screen; }
    int first=selected/10*10;
    for (int row=0;row<10 && first+row<data.count;row++) {
        const auto &entry=game_data::shop_entry_at(data.offset+first+row);
        if (entry.item>=222) { continue; }const auto &item=game_data::item(entry.item);
        snprintf(line,sizeof(line),"%c %s x%u  %u",selected==first+row ? '>' : ' ',item.name,entry.count,item.price*entry.count);
        render_utf_8_text(screen,line,16,72+row*16,423,16);
    }
    return screen;
}
void shop_render::process_key_click(uint8_t direct,uint8_t function) {
    if (function&b) { pop();return; }
    if (!data.count) { return; }
    if (direct&up) { selected=std::max(0,selected-1); }
    if (direct&down) { selected=std::min(static_cast<int>(data.count)-1,selected+1); }
    if (function&a) {
        const auto &entry=game_data::shop_entry_at(data.offset+selected);
        if (!entry.item || entry.item>=222 || !game_data::item(entry.item).buyable) { push(new dialogue_render("这件商品不能购买。"));return; }
        unsigned free=0;for (auto item:game_state::get().inventory) { if (!item) { free++; } }
        uint32_t price=game_data::item(entry.item).price*entry.count;
        if (!entry.count || free<entry.count) { push(new dialogue_render("物品栏已满。"));return; }
        if (!game_state::spend(price)) { push(new dialogue_render("金钱不足。"));return; }
        for (unsigned i=0;i<entry.count;i++) { game_state::add_item(entry.item); }
        push(new dialogue_render("购买完成。"));
    }
}
} // namespace metalmax
