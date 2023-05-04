//
// Created by youngpark on 2023/4/3.
//

#ifndef SUPERCUBE_MATRIX_H
#define SUPERCUBE_MATRIX_H

#include <arm_neon.h>

float *getIdentityMatrix(int size, int offset);
void rotateM(float *m, int mOffset,
             float a, float x, float y, float z);
void setLookAtM(float *rm, int rmOffset,
                float eyeX, float eyeY, float eyeZ,
                float centerX, float centerY, float centerZ, float upX, float upY,
                float upZ);
void frustumM(float *m, int offset,
              float left, float right, float bottom, float top,
              float near, float far);
void scaleM(float *sm, int smOffset,
            float *m, int mOffset,
            float x, float y, float z);

void scaleM(float* m, int mOffset,
            float x, float y, float z);
void translateM(
        float *m, int mOffset,
        float x, float y, float z);
void translateM(float *tm, int tmOffset,
                float *m, int mOffset,
                float x, float y, float z);
void setRotateM(float *rm, int rmOffset,
                float a, float x, float y, float z);

void orthoM(float m[], int mOffset,
            float left, float right, float bottom, float top,
            float near, float far);

#endif //SUPERCUBE_MATRIX_H
