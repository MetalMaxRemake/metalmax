//
// Created by parkyu on 2023/5/9.
//

#ifndef METALMAX_LOGIC_H
#define METALMAX_LOGIC_H

#include "../global.h"

extern "C" const byte up;
extern "C" const byte down;
extern "C" const byte left;
extern "C" const byte right;

void initLogic();
void releaseLogicThread();
void updateDirectKey(byte key);
void updateFunctionKey(byte key);

#endif //METALMAX_LOGIC_H
