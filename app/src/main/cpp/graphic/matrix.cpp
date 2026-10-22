//
// Created by youngpark on 2023/4/3.
//

#include "matrix.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

namespace metalmax {
void ortho_m(float m[], int m_offset,
            float left, float right, float bottom, float top,
            float near, float far) {

    float r_width  = 1.0f / (right - left);
    float r_height = 1.0f / (top - bottom);
    float r_depth  = 1.0f / (far - near);
    float x =  2.0f * (r_width);
    float y =  2.0f * (r_height);
    float z = -2.0f * (r_depth);
    float tx = -(right + left) * r_width;
    float ty = -(top + bottom) * r_height;
    float tz = -(far + near) * r_depth;
    m[m_offset + 0] = x;
    m[m_offset + 5] = y;
    m[m_offset +10] = z;
    m[m_offset +12] = tx;
    m[m_offset +13] = ty;
    m[m_offset +14] = tz;
    m[m_offset +15] = 1.0f;
    m[m_offset + 1] = 0.0f;
    m[m_offset + 2] = 0.0f;
    m[m_offset + 3] = 0.0f;
    m[m_offset + 4] = 0.0f;
    m[m_offset + 6] = 0.0f;
    m[m_offset + 7] = 0.0f;
    m[m_offset + 8] = 0.0f;
    m[m_offset + 9] = 0.0f;
    m[m_offset + 11] = 0.0f;
}
} // namespace metalmax
