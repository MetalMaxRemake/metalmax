//
// Created by parkyu on 2023/5/18.
//

#ifndef metalmax_graphic_process_h
#define metalmax_graphic_process_h
#include "../global.h"

namespace metalmax {
uint8_t *horizontal_flip(uint8_t *source, int width, int height);
uint8_t *vertical_flip(uint8_t *source, int width, int height);

} // namespace metalmax
#endif //METALMAX_GRAPHIC_PROCESS_H
