//
// Created by youngpark on 2023/5/3.
//

#ifndef metalmax_charsets_h
#define metalmax_charsets_h

#include "../logic/logic.h"

namespace metalmax {
    unsigned char *get_zh_string_img(const char *str, int len);

    void render_asci_text(uint8_t *screen_buffer, const char *str, int x, int y);

    void render_zh_text(uint8_t *screen_buffer, const char *str, int len, int x, int y);

} // namespace metalmax
#endif //METALMAX_CHARSETS_H
