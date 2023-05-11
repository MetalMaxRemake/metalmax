//
// Created by parkyu on 2023/5/11.
//

#include "base_menu.h"
#include "../charset/charsets.h"

const char chinese_demo[4] = {0,1,2,3};
const char *version_string = "PARK_671 TEST VERSION 0.2";
const char *graphic_string = "OPENGL ES 2.0";

void render(byte *screenBuffer) {
    renderAsciText(screenBuffer, version_string, 10, 10);
    renderAsciText(screenBuffer, graphic_string, 10, 20);
    renderZhText(screenBuffer, chinese_demo, 4, 10, 30);
}