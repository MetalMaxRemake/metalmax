//
// Created by parkyu on 2023/6/7.
//

#include <cstdio>
#include "render_computer.h"
#include "../charset/charsets.h"

byte *ComputerRender::render(byte *screenBuffer) {
    char info[100];
    if(type == -1) {
        sprintf(info, "NOTHING WAS FOUND!");
    } else {
        sprintf(info, "COMPUTER TYPE = %d", type);
    }
    int start = 192;
    int textHeight = 8;
    renderAsciText(screenBuffer, info, 108, start + textHeight * 1);
    return screenBuffer;
}

void ComputerRender::processKeyClick(byte directKey, byte functionKey) {
    if (functionKey != 0) {
        pop();
    }
}

void ComputerRender::tikLogic() {

}

ComputerRender::ComputerRender(int type) {
    this->type = type;
}