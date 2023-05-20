//
// Created by parkyu on 2023/5/18.
//

#include <cstdlib>
#include "graphic_process.h"
#include "../opt/mem_opt.h"

byte *horizontalFlip(byte *source, int width, int height) {
    byte tmp;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width / 2; j++) {
            tmp = source[i * width + j];
            source[i * width + j] = source[i * width + width - j - 1];
            source[i * width + width - j - 1] = tmp;
        }
    }
    return source;
}

byte *verticalFlip(byte *source, int width, int height) {
    byte *line = (byte *) malloc(width);
    for (int i = 0; i < height / 2; ++i) {
        __memcpy_aarch64_simd(line, source + i * width, width);
        __memcpy_aarch64_simd(source + i * width, source + (height - i - 1) * width, width);
        __memcpy_aarch64_simd(source + (height - i - 1) * width, line, width);
    }
    free(line);
    return source;
}