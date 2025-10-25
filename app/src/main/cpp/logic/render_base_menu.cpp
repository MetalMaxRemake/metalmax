//
// Created by parkyu on 2023/6/7.
//

#include <cstdlib>
#include <string.h>
#include "render_base_menu.h"

BaseMenuRender::BaseMenuRender() {
    selectIcon = (uint8_t *) malloc(8 * 8);
    memset(selectIcon, 3, 8 * 8);
    for (int i = 0; i < 4; i++) {
        memset(selectIcon + (i * 8), 8, i + 1);
    }
    for (int i = 4; i < 8; i++) {
        memset(selectIcon + (i * 8), 8, 8 - i);
    }
}

BaseMenuRender::~BaseMenuRender() {
    free(selectIcon);
}