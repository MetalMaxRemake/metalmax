//
// Created by parkyu on 2023/5/11.
//

#include "render_debug.h"
#include "logic.h"
#include "../charset/charsets.h"
#include "../monster/monster.h"

const char chinese_demo[4] = {0,1,2,3};
const char *version_string = "PARK_671 TEST VERSION 0.2";
const char *graphic_string = "OPENGL ES 2.0";

byte * DebugRender::render(byte *screenBuffer) {
//    renderMonster(0, 10,20, screenBuffer);
    renderAsciText(screenBuffer, version_string, 10, 10);
    renderAsciText(screenBuffer, graphic_string, 10, 20);
    renderZhText(screenBuffer, chinese_demo, 4, 10, 30);
    return screenBuffer;
}

bool DebugRender::processKey(byte directKey, byte functionKey) {
    if(functionKey & b) {
        pop();
    }
    return true;
}

DebugRender::DebugRender() {
    logd("DebugRender", "new");
}

DebugRender::~DebugRender() {
    logd("DebugRender", "delete");
}
