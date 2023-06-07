//
// Created by parkyu on 2023/6/7.
//

#ifndef METALMAX_RENDER_COMPUTER_H
#define METALMAX_RENDER_COMPUTER_H

#include "render_base_menu.h"

class ComputerRender : public BaseMenuRender {
private:
    int type;
public:
    ComputerRender(int type);
    byte * render(byte *screenBuffer);
    void tikLogic();
    void processKeyClick(byte directKey, byte functionKey);
};

#endif //METALMAX_RENDER_COMPUTER_H
