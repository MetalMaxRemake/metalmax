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

void setRenderCallback(byte*(*getBuffer)(byte* screenBuffer));
void initGraphic(ANativeWindow* window);
int* getCurrentPalette();
void refreshPalette(int *currentPalette);
void releaseGraphic();
int getFps();

#endif //METALMAX_NATIVE_GRAPHIC_H
