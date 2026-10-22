//
// Created by parkyu on 2023/5/9.
//

#ifndef metalmax_logic_h
#define metalmax_logic_h

#include "../global.h"

namespace metalmax {
extern "C" const uint8_t up;
extern "C" const uint8_t down;
extern "C" const uint8_t left;
extern "C" const uint8_t right;

void init_logic();
void release_logic_thread();
void update_direct_key(uint8_t key);
void update_function_key(uint8_t key);
void set_logic_paused(bool paused);
void lock_logic();
void unlock_logic();

} // namespace metalmax
#endif //METALMAX_LOGIC_H
