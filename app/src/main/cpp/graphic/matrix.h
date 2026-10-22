//
// Created by youngpark on 2023/4/3.
//

#ifndef supercube_matrix_h
#define supercube_matrix_h

#include "../global.h"

namespace metalmax {
void ortho_m(float m[], int m_offset,
            float left, float right, float bottom, float top,
            float near, float far);

} // namespace metalmax
#endif //SUPERCUBE_MATRIX_H
