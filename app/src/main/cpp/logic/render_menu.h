//
// Created by parkyu on 2023/5/11.
//

#ifndef METALMAX_RENDER_MENU_H
#define METALMAX_RENDER_MENU_H

#include "render.h"
#include "render_base_menu.h"

#define MAP_MENU 0
#define START_MENU 1

class MenuRender : public BaseMenuRender {
private:
    int selectPos = 0;
    void processSelection();
public:
    MenuRender(int menuType);
    byte * render(byte *screenBuffer);
    void tikLogic();
    void processKeyClick(byte directKey, byte functionKey);
};

#endif //METALMAX_RENDER_MENU_H
