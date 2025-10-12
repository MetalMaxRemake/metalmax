//
// Created by youngpark on 2023/5/20.
//

#include "bitmap_render.h"
#include "../opt/mem_opt.h"
#include "palette_data.h"
#include "../logic/logic.h"

/**
 *
 * @param originData
 * @param width
 * @param height
 * @param x - [0,width]
 * @param y - [0,height]
 * @param result
 * @return
 */
uint8_t *renderBitmap(uint8_t *originData,
                   int width, int height,
                   int x, int y,
                   uint8_t *result) {
    int offset = y * global_config::k_screen_width + x;
    int realWidth = width;
    int startX = 0;
    if(x < 0) {
        realWidth += x;
        startX = -x;
    } else if(x > global_config::k_screen_height) {
        realWidth -= (x - global_config::k_screen_height);
    }
    for (int i = 0; i < height; i++) {
        if (i + y < 0 || i + y > global_config::k_screen_height) {
            continue;
        }
        __memcpy_aarch64_simd(result + i * global_config::k_screen_width + offset + startX,
                              originData + i * width + startX,
                              realWidth);
    }
    return result;
}

/**
 * warning: this func is slower than upper!
 * warning: trans bitmap MUST use this func!
 * @param originData
 * @param colorOffset
 * @param width
 * @param height
 * @param x
 * @param y
 * @param result
 * @return
 */
uint8_t *renderBitmapWithTrans(uint8_t *originData,
                            int width, int height,
                            int x, int y,
                            uint8_t *result) {
    int realWidth = width;//16
    int startX = 0;
    if(x < 0) {
        realWidth += x;//16 + -3 = 13
        startX = -x;//3
    } else if(x > global_config::k_screen_width) {
        realWidth -= (x - global_config::k_screen_width);
    }

    int offset = y * global_config::k_screen_width + x;
    for (int i = 0; i < height; i++) {
        if (i + y < 0 || i + y > global_config::k_screen_height) {
            continue;
        }
        for (int j = startX; j < startX + realWidth; j++) {
            if (originData[i * width + j] == TRANSPARENT) {
                //skip trans color!
                continue;
            } else {
                result[i * global_config::k_screen_width + offset + j] = originData[i * width + j];
            }
        }
    }
    return result;
}

uint8_t *renderBitmapWithScroll(uint8_t *originData, uint8_t direct,
                             int width, int height,
                             int x, int y,
                             uint8_t *result) {
    if (direct & up) {
        renderBitmapWithTrans(originData,
                              width, height,
                              x, y,
                              result);
    }
    if (direct & down) {
        int realWidth = width;//16
        int startX = 0;
        if(x < 0) {
            realWidth += x;//16 + -3 = 13
            startX = -x;//3
        } else if(x > global_config::k_screen_height) {
            realWidth -= (x - global_config::k_screen_height);
        }

        int offset = y * global_config::k_screen_width + x;
        for (int i = 0; i < height; i++) {
            if ((15 - i) + y < 0 || (15 - i) + y > global_config::k_screen_height) {
                continue;
            }
            for (int j = startX; j < startX + realWidth; j++) {
                if (originData[i * width + j] == TRANSPARENT) {
                    //skip trans color!
                    continue;
                } else {
                    result[(15 - i) * global_config::k_screen_width + offset + j] = originData[i * width + j];
                }
            }
        }
    }
    if (direct & left) {
        int offset = y * global_config::k_screen_width + x;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (j + y < 0 || j + y > global_config::k_screen_height) {
                    continue;
                }
                if (i + x < 0 || i + x > (global_config::k_screen_width - 1)) {
                    continue;
                }
                if (originData[i * width + j] == TRANSPARENT) {
                    //skip trans color!
                    continue;
                } else {
                    result[j * global_config::k_screen_width + offset + i] = originData[i * width + j];
                }
            }
        }
    }
    if (direct & right) {
        int offset = y * global_config::k_screen_width + x;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (j + y < 0 || j + y > global_config::k_screen_height) {
                    continue;
                }
                if ((width - 1 - i) + x < 0 || (width - 1 - i) + x > (global_config::k_screen_width - 1)) {
                    continue;
                }
                if (originData[i * width + j] == TRANSPARENT) {
                    //skip trans color!
                    continue;
                } else {
                    result[j * global_config::k_screen_width + offset + (width - 1 - i)] = originData[i * width + j];
                }
            }
        }
    }
    return result;
}