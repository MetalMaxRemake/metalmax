//
// Created by youngpark on 2023/4/23.
//

#ifndef METALMAX_NATIVE_GL_H
#define METALMAX_NATIVE_GL_H

#include <android/native_window.h>
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "GLES2/gl2platform.h"

void updateScreenBuffer(unsigned char * buffer);
void initGraphic(ANativeWindow* window);
void releaseGraphic();

#endif //METALMAX_NATIVE_GL_H
