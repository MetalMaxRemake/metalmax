//
// Created by parkyu on 2023/6/7.
//

#ifndef METALMAX_RENDER_BASE_MENU_H
#define METALMAX_RENDER_BASE_MENU_H

#include "render.h"

class BaseMenuRender : public BaseRender {
private:
    uint8_t *selectIcon;
protected:
    virtual void renderSelectPos(uint8_t *screenBuffer, int x, int y) {
        int img_width = global_config::k_screen_width;
        int offset = y * img_width + x;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                int currentPos = i * img_width + j;
                screenBuffer[currentPos + offset] = selectIcon[i * 8 + j];
            }
        }
    }
public:
    BaseMenuRender();
    ~BaseMenuRender();
};

#endif //METALMAX_RENDER_BASE_MENU_H
