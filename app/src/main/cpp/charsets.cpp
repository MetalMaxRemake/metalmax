//
// Created by youngpark on 2023/5/3.
//

#include "charsets.h"
#include "logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

unsigned char data[96][8]={
        {0,0,0,0,0,0,0,0,},
        {0,0,32,32,32,0,32,0,},
        {0,0,48,48,0,0,0,0,},
        {0,0,24,60,40,124,48,0,},
        {0,32,112,160,112,40,240,32,},
        {0,0,112,120,16,44,12,0,},
        {0,0,48,64,40,80,104,0,},
        {0,0,16,16,0,0,0,0,},
        {0,8,8,16,16,16,16,8,},
        {0,0,32,16,16,16,16,32,},
        {0,0,16,56,40,0,0,0,},
        {0,0,16,16,124,16,0,0,},
        {0,0,0,0,0,0,16,32,},
        {0,0,0,0,48,0,0,0,},
        {0,0,0,0,0,0,32,0,},
        {0,0,8,16,16,16,32,32,},
        {0,0,48,80,80,80,96,0,},
        {0,0,16,48,16,16,16,0,},
        {0,0,96,16,32,64,112,0,},
        {0,0,112,16,32,16,96,0,},
        {0,0,32,64,80,112,16,0,},
        {0,0,112,64,112,16,96,0,},
        {0,0,48,64,112,80,96,0,},
        {0,0,112,16,32,32,32,0,},
        {0,0,48,80,32,80,96,0,},
        {0,0,48,80,112,16,96,0,},
        {0,0,0,0,0,0,0,0,},
        {0,0,0,16,0,0,16,16,},
        {0,0,0,8,48,24,0,0,},
        {0,0,0,56,0,56,0,0,},
        {0,0,0,32,28,48,0,0,},
        {0,0,112,16,32,0,32,0,},
        {0,0,112,168,184,128,120,0,},
        {0,0,16,40,72,120,72,0,},
        {0,0,112,72,112,72,112,0,},
        {0,0,48,72,64,72,48,0,},
        {0,0,112,72,72,80,96,0,},
        {0,0,112,64,112,64,112,0,},
        {0,0,112,64,112,64,64,0,},
        {0,0,56,64,88,72,56,0,},
        {0,0,72,72,120,72,72,0,},
        {0,0,32,32,32,32,32,0,},
        {0,0,16,16,16,80,32,0,},
        {0,0,72,80,96,80,72,0,},
        {0,0,64,64,64,64,112,0,},
        {0,0,136,216,168,168,136,0,},
        {0,0,72,104,88,72,72,0,},
        {0,0,48,72,72,72,48,0,},
        {0,0,112,72,112,64,64,0,},
        {0,0,48,72,104,80,40,0,},
        {0,0,112,72,112,80,72,0,},
        {0,0,48,64,48,8,112,0,},
        {0,0,112,32,32,32,32,0,},
        {0,0,72,72,72,72,48,0,},
        {0,0,72,72,72,80,32,0,},
        {0,0,136,168,168,80,80,0,},
        {0,0,72,72,48,72,72,0,},
        {0,0,72,72,48,16,32,0,},
        {0,0,112,16,32,64,112,0,},
        {24,16,16,16,16,16,16,24,},
        {0,0,32,32,16,16,8,0,},
        {48,16,16,16,16,16,16,48,},
        {0,0,16,40,40,0,0,0,},
        {0,0,0,0,0,0,0,120,},
        {0,0,32,0,0,0,0,0,},
        {0,0,0,96,48,80,112,0,},
        {0,64,64,96,80,80,96,0,},
        {0,0,0,32,80,64,48,0,},
        {0,16,16,48,80,80,48,0,},
        {0,0,0,48,80,96,48,0,},
        {0,16,32,48,32,32,32,0,},
        {0,0,0,48,80,80,48,16,},
        {0,64,64,96,80,80,80,0,},
        {0,32,0,32,32,32,32,0,},
        {0,16,0,16,16,16,16,16,},
        {0,64,64,80,96,80,80,0,},
        {0,32,32,32,32,32,32,0,},
        {0,0,0,208,168,168,168,0,},
        {0,0,0,96,80,80,80,0,},
        {0,0,0,32,80,80,32,0,},
        {0,0,0,96,80,80,96,64,},
        {0,0,0,48,80,80,48,16,},
        {0,0,0,16,32,32,32,0,},
        {0,0,0,48,32,16,48,0,},
        {0,0,32,48,32,32,16,0,},
        {0,0,0,80,80,80,48,0,},
        {0,0,0,80,80,32,32,0,},
        {0,0,0,136,168,168,80,0,},
        {0,0,0,80,32,80,80,0,},
        {0,0,0,80,80,80,32,32,},
        {0,0,0,112,32,64,112,0,},
        {0,24,16,16,32,16,16,24,},
        {0,0,16,16,16,16,16,16,},
        {0,32,16,16,8,16,16,48,},
        {0,0,0,0,52,8,0,0,},
        {0,148,88,148,0,156,220,212,},
};

unsigned char zh_cn[4][12][2]={
        {{0,0},{0,0},{8,0},{8,0},{254,128},{136,128},{136,128},{254,128},{8,0},{8,0},{8,0},{0,0},},
        {{0,0},{0,0},{16,0},{8,0},{254,128},{34,0},{34,0},{20,0},{8,0},{20,0},{226,128},{0,0},},
        {{0,0},{0,0},{188,128},{100,128},{166,128},{110,128},{46,128},{170,128},{136,128},{148,128},{164,128},{0,0},},
        {{0,0},{0,0},{66,128},{34,0},{14,128},{194,0},{94,0},{74,0},{72,0},{108,0},{88,128},{0,0},},
};

unsigned char *getZhStringImg(const char *str, int len) {
    unsigned char *res = (unsigned char *) malloc(sizeof(char) * 12 * 12 * len);
    int img_width = 12 * len;
    for (int charIdx = 0; charIdx < len; charIdx++) {
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 8; j++) {
                res[i * img_width + j + charIdx * 12] = (zh_cn[str[charIdx]][i][0] & (1 << (8 - j))) ? 8 : 3;//black - white
            }
            for (int j = 8; j < 12; j++) {
                res[i * img_width + j + charIdx * 12] = (zh_cn[str[charIdx]][i][1] & (1 << (16 - j))) ? 8 : 3;
            }
        }
    }
    return res;
}

unsigned char *getStringImg(const char *str) {
    int len = strlen(str);
    unsigned char *res = (unsigned char *) malloc(sizeof(char) * 8 * 8 * len);
    int img_width = 8 * len;
    for (int charIdx = 0; charIdx < len; charIdx++) {
        int ascii = str[charIdx] - 32;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                res[i * img_width + j + charIdx * 8] = (data[ascii][i] & (1 << (8 - j))) ? 8 : 3;
            }
        }
    }
    return res;
}

void drawText(byte* screenBuffer, const char *str, int x, int y) {
    int len = strlen(str);
    int img_width = 256;
    int offset = y*img_width + x;
    for (int charIdx = 0; charIdx < len; charIdx++) {
        int ascii = str[charIdx] - 32;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                int currentPos = i * img_width + j + charIdx * 8;
                screenBuffer[currentPos + offset] = (data[ascii][i] & (1 << (8 - j))) ? 8 : 3;
            }
        }
    }
}

void drawZhText(byte* screenBuffer, const char *str, int len, int x, int y) {
    int img_width = 256;
    int offset = y*img_width + x;
    for (int charIdx = 0; charIdx < len; charIdx++) {
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 8; j++) {
                int currentPos = i * img_width + j + charIdx * 12;
                screenBuffer[currentPos + offset] = (zh_cn[str[charIdx]][i][0] & (1 << (8 - j))) ? 8 : 3;//black - white
            }
            for (int j = 8; j < 12; j++) {
                int currentPos = i * img_width + j + charIdx * 12;
                screenBuffer[currentPos + offset] = (zh_cn[str[charIdx]][i][1] & (1 << (16 - j))) ? 8 : 3;
            }
        }
    }
}