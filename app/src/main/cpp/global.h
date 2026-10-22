//
// Created by parkyu on 2023/5/11.
//

#ifndef metalmax_global_h
#define metalmax_global_h

#include <android/log.h>
#include <stdint.h>
#include <stddef.h>

namespace metalmax::global_config {
    //this should be 16:9
    constexpr uint16_t k_screen_width = 455;
    constexpr uint16_t k_screen_height = 256;

    constexpr uint16_t k_tile_size = 16;
    constexpr uint16_t k_tile_width = k_screen_width / k_tile_size;
    constexpr uint16_t k_tile_height = k_screen_height / k_tile_size;

    constexpr size_t k_screen_pixel_size = sizeof(uint8_t);
    constexpr size_t k_screen_buffer_size = k_screen_pixel_size * k_screen_width * k_screen_height;

    enum render_mode {
        k_software,
        k_open_gl,
        k_vulkan
    };
    const render_mode k_render_mode = render_mode::k_open_gl;
}
namespace global_config = metalmax::global_config;

#define log_i(log_tag, ...) __android_log_print(ANDROID_LOG_INFO, log_tag, __VA_ARGS__)
#define log_d(log_tag, ...) __android_log_print(ANDROID_LOG_DEBUG, log_tag, __VA_ARGS__)
#define log_w(log_tag, ...) __android_log_print(ANDROID_LOG_WARN, log_tag, __VA_ARGS__)
#define log_e(log_tag, ...) __android_log_print(ANDROID_LOG_ERROR, log_tag, __VA_ARGS__)

#endif //METALMAX_GLOBAL_H
