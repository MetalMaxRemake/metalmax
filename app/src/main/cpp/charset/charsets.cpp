//
// Created by youngpark on 2023/5/3.
//

#include "charsets.h"
#include "../logic/logic.h"
#include "../graphic/palette_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

namespace metalmax {
    extern const unsigned char asci[96][8];

    extern const unsigned char zh_cn[4][12][2];

    unsigned char *get_zh_string_img(const char *str, int len) {
    unsigned char *res = (unsigned char *) malloc(sizeof(char) * 12 * 12 * len);
    int img_width = 12 * len;
        for (int char_idx = 0; char_idx < len; char_idx++) {
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 8; j++) {
                res[i * img_width + j + char_idx * 12] = (zh_cn[str[char_idx]][i][0] &
                                                          (1 << (8 - j))) ? palette::k_palette_white
                                                                          : palette::k_palette_black;//black - white
            }
            for (int j = 8; j < 12; j++) {
                res[i * img_width + j + char_idx * 12] = (zh_cn[str[char_idx]][i][1] &
                                                          (1 << (16 - j)))
                                                         ? palette::k_palette_white
                                                         : palette::k_palette_black;
            }
        }
    }
    return res;
}

    void render_asci_text(uint8_t *screen_buffer, const char *str, int x, int y) {
    int len = strlen(str);
    int img_width = global_config::k_screen_width;
    int offset = y*img_width + x;
        for (int char_idx = 0; char_idx < len; char_idx++) {
            int ascii_code = str[char_idx] - 32;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 6; j++) {
                int current_pos = i * img_width + j + char_idx * 6;
                screen_buffer[current_pos + offset] = (asci[ascii_code][i] & (1 << (8 - j)))
                                                      ? palette::k_palette_white
                                                      : palette::k_palette_black;
            }
        }
    }
}

    void render_zh_text(uint8_t *screen_buffer, const char *str, int len, int x, int y) {
    int img_width = global_config::k_screen_width;
    int offset = y*img_width + x;
        for (int char_idx = 0; char_idx < len; char_idx++) {
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 8; j++) {
                int current_pos = i * img_width + j + char_idx * 12;
                screen_buffer[current_pos + offset] = (zh_cn[str[char_idx]][i][0] & (1 << (8 - j)))
                                                      ? palette::k_palette_white
                                                      : palette::k_palette_black;//white-black
            }
            for (int j = 8; j < 12; j++) {
                int current_pos = i * img_width + j + char_idx * 12;
                screen_buffer[current_pos + offset] = (zh_cn[str[char_idx]][i][1] & (1 << (16 - j)))
                                                      ? palette::k_palette_white
                                                      : palette::k_palette_black;
            }
        }
    }
}
} // namespace metalmax
