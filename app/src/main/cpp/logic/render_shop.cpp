#include "render_shop.h"
#include "render_dialogue.h"
#include "status/game_state.h"
#include "../audio/native_sound.h"
#include "../charset/utf8_text.h"
#include "../graphic/palette_data.h"
#include "../tank/tank_data.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace metalmax {
    namespace {
        const char *shop_name(int type) {
            static const char *names[] = {
                    "战车装备", "战车工具", "人类装备", "人类工具", "出租战车", "特殊炮弹", "旅馆",
                    "酒吧",
                    "酒吧", "装饰店", "点唱机", "道具售货机", "炮弹售货机", "装甲片售货机", "药店",
                    "电梯"
            };
            return type >= 0 && type < 16 ? names[type] : "商店";
        }

        bool is_item_shop(int type) {
            return type == 0 || type == 1 || type == 2 || type == 3 || type == 11 || type == 14;
        }

        uint32_t entry_price(const game_data::shop_entry_t &entry, int type) {
            if (is_item_shop(type) && entry.item < 222) {
                return game_data::item(entry.item).price * entry.count;
            }
            return game_data::value(entry.item) * std::max<unsigned>(1, entry.count);
        }

        int first_owned_tank() {
            if (game_state::get().riding_tank >= 0) { return game_state::get().riding_tank; }
            for (unsigned i = 0; i < game_state::k_tank_count; i++) {
                if (game_state::get().tanks[i].owned) { return i; }
            }
            return -1;
        }
    }

    shop_render::shop_render(int type, int page_id) : data(game_data::shop(type, page_id)),
                                                      shop_type(type), page(page_id) {}

uint8_t *shop_render::render(uint8_t *screen) {
    memset(screen+32*global_config::k_screen_width,palette::k_palette_black,224*global_config::k_screen_width);
    char line[180];
    snprintf(line, sizeof(line), "%s  金钱 %u  A 确认  B 返回", shop_name(shop_type),
             game_state::get().money);
    render_utf_8_text(screen, line, 16, 40, 423, 16);
    if (!data.count) {
        render_utf_8_text(screen, "这里暂时没有项目。", 16, 72, 423, 16);
        return screen;
    }
    int first=selected/10*10;
    for (int row=0;row<10 && first+row<data.count;row++) {
        const auto &entry=game_data::shop_entry_at(data.offset+first+row);
        uint32_t price = entry_price(entry, shop_type);
        const char *name = "";
        char fallback[64];
        if (is_item_shop(shop_type) && entry.item < 222) {
            name = game_data::item(entry.item).name;
        }
        else {
            unsigned value = entry.count ? entry.count : entry.item;
            if (shop_type == 15) { snprintf(fallback, sizeof(fallback), "目标%u", value); }
            else if (shop_type == 10) { snprintf(fallback, sizeof(fallback), "曲目%u", value); }
            else if (shop_type == 13) { snprintf(fallback, sizeof(fallback), "装甲片+%u", value); }
            else if (shop_type == 4) { snprintf(fallback, sizeof(fallback), "出租战车%u", value); }
            else if (shop_type == 6) { snprintf(fallback, sizeof(fallback), "休息套餐%u", value); }
            else { snprintf(fallback, sizeof(fallback), "服务%u", value); }
            name = fallback;
        }
        snprintf(line, sizeof(line), "%c %s x%u  %u", selected == first + row ? '>' : ' ', name,
                 std::max<unsigned>(1, entry.count), price);
        render_utf_8_text(screen,line,16,72+row*16,423,16);
    }
    return screen;
}

void shop_render::process_key_click(uint8_t direct,uint8_t function) {
    if (function&b) { pop();return; }
    if (!data.count) { return; }
    if (direct&up) { selected=std::max(0,selected-1); }
    if (direct&down) { selected=std::min(static_cast<int>(data.count)-1,selected+1); }
    if (!(function & a)) { return; }
    const auto &entry = game_data::shop_entry_at(data.offset + selected);
    uint32_t price = entry_price(entry, shop_type);
    if (price && !game_state::spend(price)) {
        push(new dialogue_render("金钱不足。"));
        return;
    }
    if (is_item_shop(shop_type)) {
        if (!entry.item || entry.item >= 222 || !game_data::item(entry.item).buyable) {
            push(new dialogue_render("这件商品不能购买。"));
            return;
        }
        if (!entry.count || game_state::free_item_slots() < entry.count) {
            push(new dialogue_render("物品栏已满。"));
            return;
        }
        for (unsigned i=0;i<entry.count;i++) { game_state::add_item(entry.item); }
        push(new dialogue_render("购买完成。"));
        return;
    }
    int tank_id = first_owned_tank();
    if (shop_type == 5 || shop_type == 12) {
        if (tank_id < 0) {
            push(new dialogue_render("没有可补给的战车。"));
            return;
        }
        auto &tank = game_state::get().tanks[tank_id];
        tank.shells = std::min<unsigned>(tank.shells + std::max<unsigned>(1, entry.count),
                                         tank_data::max_shells());
        push(new dialogue_render("炮弹补给完成。"));
        return;
    }
    if (shop_type == 13) {
        if (tank_id < 0) {
            push(new dialogue_render("没有可补给的战车。"));
            return;
        }
        auto &tank = game_state::get().tanks[tank_id];
        tank.sp = std::min<unsigned>(tank.max_sp, tank.sp + std::max<unsigned>(1, entry.count));
        push(new dialogue_render("装甲片补给完成。"));
        return;
    }
    if (shop_type == 6) {
        for (auto &player: game_state::get().players) { if (player.active) { player.hp = player.max_hp; }}
        push(new dialogue_render("休息后生命恢复了。"));
        return;
    }
    if (shop_type == 10) {
        change_audio(entry.item % 96);
        push(new dialogue_render("点唱机开始播放。"));
        return;
    }
    if (shop_type == 4) {
        int slot = std::min<int>(game_state::k_tank_count - 1, std::max<int>(0, entry.item - 8));
        auto &tank = game_state::get().tanks[slot];
        tank.owned = 1;
        tank.map = game_state::get().map;
        tank.x = game_state::get().x;
        tank.y = game_state::get().y;
        tank.ok = 1;
        game_state::enter_tank(slot);
        push(new dialogue_render("出租战车已经准备好。"));
        return;
    }
    if (shop_type == 15) {
        push(new dialogue_render("电梯启动了。"));
        return;
    }
    push(new dialogue_render("服务完成。"));
}
} // namespace metalmax
