//
// Created by youngpark on 2023/4/24.
//

#include "map.h"
#include "../opt/mem_opt.h"
#include "tile_bmp.h"
#include "map_data/map_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <__threading_support>

volatile int map_width = 256,map_height = 256;
pthread_mutex_t mapRefreshMutex;

void fill(int i, int j, int bmpIdx, unsigned char *result) {
    for(int x = 0;x<16;x++) {
        int holeX = x + i*16;
        for(int y = 0;y<16;y++) {
            int holeY = y + j*16;
            result[holeX*(map_width*16) + holeY] = bitmaps[bmpIdx][x * 16 + y];
        }
    }
}

unsigned char *fullMap = nullptr;
unsigned char *current_map;

void initFullMap() {
    if(fullMap == nullptr) {
        unsigned char *result = (unsigned char *)malloc(sizeof (char) * (map_width*map_height*256));
        __memset_aarch64(result, 0, sizeof (char) * (map_width*map_height*256));
        int bmpIdx = 0;
        for(int i = 0;i<map_height;i++) {
            for(int j = 0;j<map_width;j++) {
                bmpIdx = current_map[i*map_width + j];
                fill(i, j, bmpIdx, result);
            }
        }
        fullMap = result;
    }
}

void releaseMap() {
    if(fullMap != nullptr) {
        free(fullMap);
    }
}

int min(int a, int b) {
    return a < b ? a : b;
}

void refreshCurrentMap(int mapId) {
    pthread_mutex_lock(&mapRefreshMutex);
//    releaseMap(); fixme bug crash
    fullMap = nullptr;
    current_map = map_data[mapId];
    map_height = map_size[mapId * 2];
    map_width = map_size[mapId * 2 + 1];
    initFullMap();
    pthread_mutex_unlock(&mapRefreshMutex);
}

unsigned char *getImage(int x, int y, unsigned char *result) {
    if(result == nullptr) {
        result = (unsigned char *) malloc(sizeof(char) * (16 * 16 * 256));
    }
    pthread_mutex_lock(&mapRefreshMutex);
    if(fullMap == nullptr) {
        initFullMap();
    }
    __memset_aarch64(result, 0, 16 * 16 * 256);
    int maxX = map_height * 256, maxY = map_width * 256;
    int renderXEnd = x + 256, renderYEnd = y + 256;
    for (int i = 0; i < 256; i++) {
        if (i + x >= maxX
            || i + x >= renderXEnd
            || i + x < 0) {
            continue;
        }
        int resultStartIdx = i * 256;
        int fullMapStartIdx = (i + x) * (16 * map_width) + y;
        int maxj1 = (maxY - y - 1);
        int maxj2 = (renderYEnd - y - 1);
        int maxj = min(maxj1, maxj2);
        int length = min(maxj, 256);
        __memcpy_aarch64_simd(result + resultStartIdx, fullMap + fullMapStartIdx, length);
    }
    pthread_mutex_unlock(&mapRefreshMutex);
    return result;
}



