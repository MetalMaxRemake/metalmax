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
    uint8_t * render(uint8_t *screenBuffer);
    void tikLogic();
    void processKeyClick(uint8_t directKey, uint8_t functionKey);
};

#endif //METALMAX_RENDER_MENU_H
