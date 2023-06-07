//
// Created by parkyu on 2023/6/7.
//

#ifndef METALMAX_RENDER_BASE_MENU_H
#define METALMAX_RENDER_BASE_MENU_H

#include "render.h"

class BaseMenuRender : public BaseRender {
private:
    byte *selectIcon;
protected:
    virtual void renderSelectPos(byte *screenBuffer, int x, int y) {
        int img_width = 256;
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
