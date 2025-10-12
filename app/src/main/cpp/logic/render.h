//
// Created by parkyu on 2023/5/11.
//

#ifndef METALMAX_RENDER_H
#define METALMAX_RENDER_H

#include "logic.h"

class BaseRender {
protected:
    const uint8_t a = 0b0001;
    const uint8_t b = 0b0010;
    const uint8_t ta = 0b0100;
    const uint8_t tb = 0b1000;
    const uint8_t keyX = 0b00010000;
    const uint8_t keyY = 0b00100000;
public:
    virtual uint8_t * render(uint8_t *screenBuffer) {};
    virtual void tikLogic() {};
    virtual bool processKey(uint8_t directKey, uint8_t functionKey) {return false;};
    virtual void processKeyClick(uint8_t directKey, uint8_t functionKey) {}
    virtual void onFocus() {};
    virtual void onUnFocus() {};
    virtual ~BaseRender() {};
};

void push(BaseRender *baseRender);

BaseRender *top();

void pop();

void changeMap(int mapId, int x, int y);//only support std pos

int getCurrentMap();

#endif //METALMAX_RENDER_H
