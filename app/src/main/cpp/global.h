//
// Created by parkyu on 2023/5/11.
//

#ifndef METALMAX_GLOBAL_H
#define METALMAX_GLOBAL_H

#include <android/log.h>
#include <stdint.h>

namespace global_config {
    //this should be 16:9
    constexpr uint16_t k_screen_width = 455;
    constexpr uint16_t k_screen_height = 256;

    constexpr size_t k_screen_pixel_size = sizeof(uint8_t);
    constexpr size_t k_screen_buffer_size = k_screen_pixel_size * k_screen_width * k_screen_height;

    enum RenderMode {
        kSoftware,
        kOpenGL,
        kVulkan
    };
    const RenderMode k_render_mode = RenderMode::kOpenGL;
}

#define LOGI(LOG_TAG, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(LOG_TAG, ...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(LOG_TAG, ...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(LOG_TAG, ...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#endif //METALMAX_GLOBAL_H
