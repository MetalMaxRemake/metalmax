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

#define MAP_FILL_SIZE 16

volatile int map_width = 256,map_height = 256;
pthread_mutex_t mapRefreshMutex;

inline void fill(int i, int j, int bmpIdx, unsigned char *result) {
    int map_raw_width = ((map_width + 2 * MAP_FILL_SIZE) * 16);
    int map_column_offset = j * 16;
    int map_raw_offset = i * 16;
    for (int x = 0; x < 4; x++) {
        int block_start = x * 4;
        //循环展开 + simd_memcpy
        __memcpy_aarch64_simd(
                result + ((block_start + map_raw_offset) * map_raw_width + map_column_offset),
                bitmaps[bmpIdx] + (block_start * 16),
                16);
        __memcpy_aarch64_simd(
                result + ((block_start + 1 + map_raw_offset) * map_raw_width + map_column_offset),
                bitmaps[bmpIdx] + ((block_start + 1) * 16),
                16);
        __memcpy_aarch64_simd(
                result + ((block_start + 2 + map_raw_offset) * map_raw_width + map_column_offset),
                bitmaps[bmpIdx] + ((block_start + 2) * 16),
                16);
        __memcpy_aarch64_simd(
                result + ((block_start + 3 + map_raw_offset) * map_raw_width + map_column_offset),
                bitmaps[bmpIdx] + ((block_start + 3) * 16),
                16);
    }
}

unsigned char *fullMap = nullptr;
unsigned short *short_current_map;
unsigned short current_fill;

void initAllTilesMap() {
    fullMap = (unsigned char *) malloc(
            sizeof(char) * ((map_width + 2*MAP_FILL_SIZE) * (map_height + 2*MAP_FILL_SIZE) * 256));
    int bmpIdx = 0;
    for (int i = 0; i < map_height; i++) {
        for (int j = 0; j < map_width; j++) {
            fill(i + MAP_FILL_SIZE, j + MAP_FILL_SIZE, bmpIdx++ % 1904, fullMap);
        }
    }
}

void initFullMap() {
    if (fullMap == nullptr) {
        fullMap = (unsigned char *) malloc(
                sizeof(char) * ((map_width + 2*MAP_FILL_SIZE) * (map_height + 2*MAP_FILL_SIZE) * 256));
        int bmpIdx = 0;
        for (int i = 0; i < map_height; i++) {
            for (int j = 0; j < map_width; j++) {
                bmpIdx = short_current_map[i * (map_width) + j];
                fill(i + MAP_FILL_SIZE, j + MAP_FILL_SIZE, bmpIdx, fullMap);
            }
        }
        // fill
        for (int i = 0; i < MAP_FILL_SIZE; i++) {
            for (int j = 0; j < map_width + 2*MAP_FILL_SIZE; j++) {
                fill(i, j, current_fill, fullMap);
                fill(i + map_height + MAP_FILL_SIZE, j, current_fill, fullMap);
            }
        }
        for (int i = MAP_FILL_SIZE; i < map_height + MAP_FILL_SIZE; i++) {
            for (int j = 0; j < MAP_FILL_SIZE; j++) {
                fill(i, j, current_fill, fullMap);
                fill(i, j + map_width + MAP_FILL_SIZE, current_fill, fullMap);
            }
        }
    }
}

void releaseMap() {
    if(fullMap != nullptr) {
        free(fullMap);
        fullMap = nullptr;
    }
}

int min(int a, int b) {
    return a < b ? a : b;
}

void refreshCurrentMap(int mapId) {
    pthread_mutex_lock(&mapRefreshMutex);
    releaseMap();
    if(mapId == -1) {
        short_current_map = nullptr;
        current_fill = -1;
        map_height = 44;
        map_width = 44;
        initAllTilesMap();
    } else {
        short_current_map = short_map_data[mapId];
        current_fill = map_fill[mapId];
        map_height = map_size[mapId * 2];
        map_width = map_size[mapId * 2 + 1];
        initFullMap();
    }
    pthread_mutex_unlock(&mapRefreshMutex);
}

unsigned char *renderMap(int x, int y, unsigned char *result) {
    x += MAP_FILL_SIZE * 16;
    y += MAP_FILL_SIZE * 16;
    if(result == nullptr) {
        result = (unsigned char *) malloc(sizeof(char) * (16 * 16 * 256));
    }
    pthread_mutex_lock(&mapRefreshMutex);
    if(fullMap == nullptr) {
        initFullMap();
    }
    __memset_aarch64(result, 0, 16 * 16 * 256);
    //这里的逻辑是一大坨屎山
    int maxX = (map_height + 2 * MAP_FILL_SIZE) * 16, maxY = (map_width + 2 * MAP_FILL_SIZE) * 16;
    int renderXEnd = x + 255, renderYEnd = y + 255;
    int length = 0;
    int maxj1 = (maxY - y);
    int maxj2 = (renderYEnd - y) + 1;
    int maxj = min(maxj1, maxj2);
    int startIdx = x < 0 ? -x : 0;
    int endIdx = min(min(renderXEnd - x, maxX - x), 255);
    for (int i = startIdx; i <= endIdx; i++) {
        if(y >= maxY
            || y >= renderYEnd
            || 255 + y < 0) {
            continue;
        }
        int resultStartIdx = i * 256;
        int fullMapStartIdx = (i + x) * (16 * (map_width + 2 * MAP_FILL_SIZE)) + y;
        if (y < 0) {
            resultStartIdx -= y;
            fullMapStartIdx -= y;
            length = maxj + y;
        } else {
            length = min(maxj, 256);
        }
        __memcpy_aarch64_simd(result + resultStartIdx, fullMap + fullMapStartIdx, length);
    }
    pthread_mutex_unlock(&mapRefreshMutex);
    return result;
}



