//
// Created by youngpark on 2023/4/23.
//

#ifndef metalmax_native_graphic_h
#define metalmax_native_graphic_h

#include <android/native_window.h>
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "GLES2/gl2platform.h"
#include "../global.h"

namespace metalmax::native_graphic {

    void set_render_callback(uint8_t *(*get_buffer)(uint8_t *screen_buffer));

    bool init_graphic(ANativeWindow *window);

    void set_global_fade_percent(unsigned int percent);

    void apply_new_palette(int *current_palette);

    void release_graphic();

    int get_fps();


    //for internal usage
    bool is_render_running();

    uint8_t *get_screen_buffer();

    ANativeWindow *get_native_window();

    int *get_palette_buffer();

    size_t get_palette_size();

    void require_logic_render();

    int get_window_width();
    int get_window_height();


}

namespace native_graphic = metalmax::native_graphic;
#endif //METALMAX_NATIVE_GRAPHIC_H
