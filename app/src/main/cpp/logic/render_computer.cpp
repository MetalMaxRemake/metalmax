//
// Created by parkyu on 2023/6/7.
//

#include <cstdio>
#include "render_computer.h"
#include "../charset/charsets.h"

namespace metalmax {
uint8_t *computer_render::render(uint8_t *screen_buffer) {
    char info[100];
    if (type == -1) {
        snprintf(info, sizeof(info), "NOTHING WAS FOUND!");
    } else {
        snprintf(info, sizeof(info), "COMPUTER TYPE = %d", type);
    }
    int start = 192;
    int text_height = 8;
    render_asci_text(screen_buffer, info, 108, start + text_height * 1);
    return screen_buffer;
}

void computer_render::process_key_click(uint8_t direct_key, uint8_t function_key) {
    if (function_key != 0) {
        pop();
    }
}

void computer_render::tik_logic() {

}

computer_render::computer_render(int type) {
    this->type = type;
}
} // namespace metalmax
