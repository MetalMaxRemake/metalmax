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

    byte * render(byte *screenBuffer);

    void processKeyClick(byte directKey, byte functionKey);

    ~SplashRender();
};

#endif //METALMAX_RENDER_SPLASH_H
