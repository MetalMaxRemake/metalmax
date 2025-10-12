//
// Created by parkyu on 2023/5/11.
//

#ifndef METALMAX_GLOBAL_H
#define METALMAX_GLOBAL_H

#include "stdint.h"

namespace global_config {
    //this should be 16:9
    constexpr uint16_t k_screen_width = 455;
    constexpr uint16_t k_screen_height = 256;

    enum RenderMode {
        kSoftware,
        kOpenGL,
        kVulkan
    };
    const RenderMode k_render_mode = RenderMode::kOpenGL;
}

void logd(char *tag, const char *fmt, ...);

void loge(char *tag, const char *fmt, ...);

#endif //METALMAX_GLOBAL_H
