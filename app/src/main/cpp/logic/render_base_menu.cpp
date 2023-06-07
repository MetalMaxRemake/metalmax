//
// Created by parkyu on 2023/6/7.
//

#include <cstdlib>
#include "render_base_menu.h"
#include "../opt/mem_opt.h"

BaseMenuRender::BaseMenuRender() {
    selectIcon = (byte *) malloc(8 * 8);
    __memset_aarch64(selectIcon, 3, 8 * 8);
    for (int i = 0; i < 4; i++) {
        __memset_aarch64(selectIcon + (i * 8), 8, i + 1);
    }
    for (int i = 4; i < 8; i++) {
        __memset_aarch64(selectIcon + (i * 8), 8, 8 - i);
    }
}

BaseMenuRender::~BaseMenuRender() {
    free(selectIcon);
}