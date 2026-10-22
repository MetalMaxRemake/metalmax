#include "render_battle.h"
#include "render_inventory.h"
#include "status/game_state.h"
#include "../data/game_data.h"
#include "../monster/monster.h"
#include "../charset/utf8_text.h"
#include "../audio/native_sound.h"
#include "../graphic/palette_data.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace metalmax {
battle_render::battle_render(int id, bool browse, int event) : preview(browse), result_event(event) {
    monster_idx = (id % 131 + 131) % 131;
    enemy_hp = game_data::monster(monster_idx).hp;
    snprintf(message, sizeof(message), "遇到了 %s！", game_data::monster(monster_idx).name);
}

void battle_render::tik_logic() {
    if (!enter_battle && ++splash_clk >= 60) { enter_battle = true; }
    int track = enter_battle ? 4 : 23;
    if (get_audio_idx() != track) { change_audio(track); }
}

void battle_render::on_focus() {
    if (used_inventory && game_state::get().players[0].hp > inventory_hp) { turn(1); }
    used_inventory = false;
}

void battle_render::finish(bool win) {
    if (resolved) { return; }
    resolved = true;
    victory = win;
    if (win) {
        const auto &m = game_data::monster(monster_idx);
        game_state::award(m.experience, m.gold);
        if (result_event >= 0) { game_state::set_event(result_event, true); }
        snprintf(message, sizeof(message), "胜利！经验 %u  金钱 %u", m.experience, m.gold);
    } else {
        game_state::get().money /= 2;
        for (auto &p: game_state::get().players) { if (p.active) { p.hp = p.max_hp; }}
        snprintf(message, sizeof(message), "战斗失败，失去一半金钱。");
    }
}

void battle_render::turn(int action) {
    auto &p = game_state::get().players[0];
    const auto &enemy = game_data::monster(monster_idx);
    if (p.hp <= 0) {
        finish(false);
        return;
    }
    bool defend = action == 1;
    if (action == 0) {
        // Baseline remake rule, pending full original RNG/hit/damage recovery.
        int damage = std::max(1, p.attack - enemy.defense / 2) + rand() % 4;
        enemy_hp = std::max(0, enemy_hp - damage);
        snprintf(message, sizeof(message), "造成 %d 伤害。", damage);
    }
    if (enemy_hp <= 0) {
        finish(true);
        return;
    }
    int damage = std::max(1, enemy.attack - p.defense / 2) + rand() % 4;
    if (defend) { damage = std::max(1, damage / 2); }
    p.hp = std::max(0, p.hp - damage);
    snprintf(message, sizeof(message), defend ? "防御！受到 %d 伤害。" : "敌人反击，受到 %d 伤害。",
             damage);
    if (p.hp <= 0) { finish(false); }
}

uint8_t *battle_render::render(uint8_t *screen) {
    memset(screen, palette::k_palette_black, global_config::k_screen_buffer_size);
    if (!enter_battle) {
        if ((splash_clk / 4) % 2) {
            memset(screen, 8, global_config::k_screen_buffer_size);
        }
        return screen;
    }
    render_monster(monster_idx, 20, 24, screen);
    const auto &m = game_data::monster(monster_idx);
    char line[180];
    render_utf_8_text(screen, m.name, 180, 32, 255, 16);
    snprintf(line, sizeof(line), "敌人 HP %d/%d", enemy_hp, m.hp);
    render_utf_8_text(screen, line, 180, 56, 255, 16);
    auto &p = game_state::get().players[0];
    snprintf(line, sizeof(line), "勇士 HP %d/%d", p.hp, p.max_hp);
    render_utf_8_text(screen, line, 180, 80, 255, 16);
    if (preview) {
        snprintf(line, sizeof(line), "攻击 %d  防御 %d", m.attack, m.defense);
        render_utf_8_text(screen, line, 180, 104, 255, 16);
        render_utf_8_text(screen, "上/下 浏览怪物  B 返回", 16, 224, 423, 16);
        return screen;
    }
    memset(screen + 160 * global_config::k_screen_width, palette::k_palette_black,
           96 * global_config::k_screen_width);
    render_utf_8_text(screen, message, 16, 168, 423, 32);
    if (resolved) {
        render_utf_8_text(screen, "A/B 返回地图", 16, 224, 423, 16);
        return screen;
    }
    static const char *actions[] = {"攻击", "防御", "物品", "逃跑"};
    for (int i = 0; i < 4; i++) {
        snprintf(line, sizeof(line), "%c %s", i == selected ? '>' : ' ', actions[i]);
        render_utf_8_text(screen, line, 16 + i * 108, 216, 102, 16);
    }
    return screen;
}

void battle_render::process_key_click(uint8_t direct, uint8_t function) {
    if (!enter_battle) { return; }
    if (preview) {
        if (direct & up) { monster_idx = (monster_idx + 130) % 131; }
        if (direct & down) { monster_idx = (monster_idx + 1) % 131; }
        enemy_hp = game_data::monster(monster_idx).hp;
        if (function & b) { pop(); }
        return;
    }
    if (resolved) {
        if (function & (a | b)) { pop(); }
        return;
    }
    if (direct & (left | up)) { selected = (selected + 3) % 4; }
    if (direct & (right | down)) { selected = (selected + 1) % 4; }
    if (function & b) {
        selected = 3;
        return;
    }
    if (!(function & a)) { return; }
    if (selected == 2) {
        inventory_hp = game_state::get().players[0].hp;
        used_inventory = true;
        push(new inventory_render);
        return;
    }
    if (selected == 3) {
        if (result_event < 0 && rand() % 100 < 70) {
            pop();
            return;
        }
        turn(1);
        return;
    }
    turn(selected);
}
} // namespace metalmax
