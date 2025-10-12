//
// Created by parkyu on 2023/5/9.
//

#ifndef METALMAX_LOGIC_H
#define METALMAX_LOGIC_H

#include "../global.h"

extern "C" const uint8_t up;
extern "C" const uint8_t down;
extern "C" const uint8_t left;
extern "C" const uint8_t right;

void initLogic();
void releaseLogicThread();
void updateDirectKey(uint8_t key);
void updateFunctionKey(uint8_t key);

#endif //METALMAX_LOGIC_H
