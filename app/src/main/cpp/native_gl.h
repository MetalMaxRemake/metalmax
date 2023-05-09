//
// Created by youngpark on 2023/4/23.
//

#ifndef METALMAX_NATIVE_GL_H
#define METALMAX_NATIVE_GL_H
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "GLES2/gl2platform.h"

void initGL();
void onGLSurfaceChange(int width, int height);
void onGLDraw();
void updateScreenBuffer(unsigned char * buffer);

#endif //METALMAX_NATIVE_GL_H
