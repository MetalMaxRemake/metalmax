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
byte *renderBitmap(byte *originData,
                   int width, int height,
                   int x, int y,
                   byte *result) {
    int offset = y * 256 + x;
    int realWidth = width;
    int startX = 0;
    if(x < 0) {
        realWidth += x;
        startX = -x;
    } else if(x > 240) {
        realWidth -= (x - 240);
    }
    for (int i = 0; i < height; i++) {
        if (i + y < 0 || i + y > 240) {
            continue;
        }
        __memcpy_aarch64_simd(result + i * 256 + offset + startX,
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
byte *renderBitmapWithTrans(byte *originData,
                            int width, int height,
                            int x, int y,
                            byte *result) {
    int realWidth = width;//16
    int startX = 0;
    if(x < 0) {
        realWidth += x;//16 + -3 = 13
        startX = -x;//3
    } else if(x > 240) {
        realWidth -= (x - 240);
    }

    int offset = y * 256 + x;
    for (int i = 0; i < height; i++) {
        if (i + y < 0 || i + y > 240) {
            continue;
        }
        for (int j = startX; j < startX + realWidth; j++) {
            if (originData[i * width + j] == TRANSPARENT) {
                //skip trans color!
                continue;
            } else {
                result[i * 256 + offset + j] = originData[i * width + j];
            }
        }
    }
    return result;
}

byte *renderBitmapWithScroll(byte *originData, byte direct,
                             int width, int height,
                             int x, int y,
                             byte *result) {
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
        } else if(x > 240) {
            realWidth -= (x - 240);
        }

        int offset = y * 256 + x;
        for (int i = 0; i < height; i++) {
            if ((15 - i) + y < 0 || (15 - i) + y > 240) {
                continue;
            }
            for (int j = startX; j < startX + realWidth; j++) {
                if (originData[i * width + j] == TRANSPARENT) {
                    //skip trans color!
                    continue;
                } else {
                    result[(15 - i) * 256 + offset + j] = originData[i * width + j];
                }
            }
        }
    }
    if (direct & left) {
        int offset = y * 256 + x;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (j + y < 0 || j + y > 240) {
                    continue;
                }
                if (i + x < 0 || i + x > 255) {
                    continue;
                }
                if (originData[i * width + j] == TRANSPARENT) {
                    //skip trans color!
                    continue;
                } else {
                    result[j * 256 + offset + i] = originData[i * width + j];
                }
            }
        }
    }
    if (direct & right) {
        int offset = y * 256 + x;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (j + y < 0 || j + y > 240) {
                    continue;
                }
                if ((width - 1 - i) + x < 0 || (width - 1 - i) + x > 255) {
                    continue;
                }
                if (originData[i * width + j] == TRANSPARENT) {
                    //skip trans color!
                    continue;
                } else {
                    result[j * 256 + offset + (width - 1 - i)] = originData[i * width + j];
                }
            }
        }
    }
    return result;
}