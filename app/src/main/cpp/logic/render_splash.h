//
// Created by parkyu on 2023/5/13.
//

#ifndef METALMAX_RENDER_SPLASH_H
#define METALMAX_RENDER_SPLASH_H

#include "../global.h"
#include "render.h"

class SplashRender : public BaseRender {
private:
public:

    uint8_t * render(uint8_t *screenBuffer);
    void tikLogic();

    void processKeyClick(uint8_t directKey, uint8_t functionKey);

    ~SplashRender();
};

#endif //METALMAX_RENDER_SPLASH_H
