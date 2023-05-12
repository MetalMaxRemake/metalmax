//
// Created by parkyu on 2023/5/9.
//

#ifndef METALMAX_LOGIC_H
#define METALMAX_LOGIC_H

#include "../global.h"

void initLogic();
void releaseLogicThread();
void updateDirectKey(byte key);
void updateFunctionKey(byte key);

#endif //METALMAX_LOGIC_H
