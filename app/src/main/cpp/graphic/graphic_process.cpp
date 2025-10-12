//
// Created by parkyu on 2023/5/18.
//

#include <cstdlib>
#include "graphic_process.h"
#include "../opt/mem_opt.h"

uint8_t *horizontalFlip(uint8_t *source, int width, int height) {
    uint8_t tmp;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width / 2; j++) {
            tmp = source[i * width + j];
            source[i * width + j] = source[i * width + width - j - 1];
            source[i * width + width - j - 1] = tmp;
        }
    }
    return source;
}

uint8_t *verticalFlip(uint8_t *source, int width, int height) {
    uint8_t *line = (uint8_t *) malloc(width);
    for (int i = 0; i < height / 2; ++i) {
        __memcpy_aarch64_simd(line, source + i * width, width);
        __memcpy_aarch64_simd(source + i * width, source + (height - i - 1) * width, width);
        __memcpy_aarch64_simd(source + (height - i - 1) * width, line, width);
    }
    free(line);
    return source;
}