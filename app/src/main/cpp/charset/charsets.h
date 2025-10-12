//
// Created by youngpark on 2023/5/3.
//

#ifndef METALMAX_CHARSETS_H
#define METALMAX_CHARSETS_H

#include "../logic/logic.h"

unsigned char *getZhStringImg(const char *str, int len);

void renderAsciText(uint8_t* screenBuffer, const char *str, int x, int y);
void renderZhText(uint8_t* screenBuffer, const char *str, int len, int x, int y);

#endif //METALMAX_CHARSETS_H
