//
// Created by youngpark on 2023/4/27.
//

#ifndef METALMAX_MEM_OPT_H
#define METALMAX_MEM_OPT_H
#include <stddef.h>

extern "C" void* __memmove_aarch64(void* const dst, const void* src, size_t copy_amount);

#endif //METALMAX_MEM_OPT_H
