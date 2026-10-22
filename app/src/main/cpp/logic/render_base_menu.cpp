//
// Created by parkyu on 2023/6/7.
//

#include <cstdlib>
#include <string.h>
#include "render_base_menu.h"

namespace metalmax {
base_menu_render::base_menu_render() {
    select_icon = (uint8_t *) malloc(8 * 8);
    memset(select_icon, 3, 8 * 8);
    for (int i = 0; i < 4; i++) {
        memset(select_icon + (i * 8), 8, i + 1);
    }
    for (int i = 4; i < 8; i++) {
        memset(select_icon + (i * 8), 8, 8 - i);
    }
}

base_menu_render::~base_menu_render() {
    free(select_icon);
}
} // namespace metalmax
