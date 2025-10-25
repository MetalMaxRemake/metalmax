//
// Created by youngpark on 2023/4/23.
//

#ifndef METALMAX_NATIVE_GRAPHIC_H
#define METALMAX_NATIVE_GRAPHIC_H

#include <android/native_window.h>
#include <__threading_support>
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "GLES2/gl2platform.h"
#include "../global.h"

namespace native_graphic {

    void setRenderCallback(uint8_t *(*getBuffer)(uint8_t *screenBuffer));

    bool initGraphic(ANativeWindow *window);

    void applyNewPalette(int *currentPalette);

    void releaseGraphic();

    int getFps();


    //for internal usage
    bool isRenderRunning();

    uint8_t *getScreenBuffer();

    ANativeWindow *getNativeWindow();

    int *getPaletteBuffer();

    size_t getPaletteSize();

    void requireLogicRender();

    int getWindowWidth();
    int getWindowHeight();


}

#endif //METALMAX_NATIVE_GRAPHIC_H
