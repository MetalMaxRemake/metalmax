//
// Created by youngpark on 2023/4/23.
// EGL & FrameBuffer & OpenGL
//

#include "native_graphic.h"
#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>
#include "../global.h"

#include "../maps/map.h"
#include "../charset/charsets.h"
#include "../maps/map_data/map_data.h"
#include "palette_data.h"
#include <EGL/egl.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <unistd.h>
#include <atomic>
#include "../data/game_data.h"

using namespace metalmax;
namespace metalmax::software_render {

    const char *tag = "software_render";

    void on_soft_draw() {
        //direct draw to buffer
        uint8_t *screen_buffer = native_graphic::get_screen_buffer();
        if (screen_buffer == nullptr) {
            return;
        }
        ANativeWindow *native_window = native_graphic::get_native_window();
        ANativeWindow_Buffer m_native_window_buffer;
        ANativeWindow_lock(native_window, &m_native_window_buffer, nullptr);
        int *dst_buffer = static_cast<int *>(m_native_window_buffer.bits);
        int dst_height = m_native_window_buffer.height;
        int dst_width =
                dst_height * (global_config::k_screen_width * 1.f / global_config::k_screen_height);
        int offset = (m_native_window_buffer.stride - m_native_window_buffer.width) / 2;
        if (!native_graphic::is_render_running()) {
            return;
        }
        memset(dst_buffer, 0,
               m_native_window_buffer.stride * m_native_window_buffer.height * sizeof(int)); //对数组清零
        float scale = (float) m_native_window_buffer.height /
                      (global_config::k_screen_height * 1.f); //计算图像宽度缩放比例
        float scale_left = scale - (int) scale; //求出缩放比例的小数部分
        int add_arg = (scale_left > 0.5) ? 1 : 0;
        int x, y;

        int *palette_texture_pixels = native_graphic::get_palette_buffer();

        for (int hnum = 0; hnum < dst_height; ++hnum) //按照从左到右，从上到下的顺序进行转换
        {
            y = (int) (hnum / scale) + add_arg;   //计算当前临近坐标的y值
            for (int wnum = 0; wnum < dst_width; ++wnum) {
                x = (int) (wnum / scale) + add_arg; //计算当前临近坐标的x值
                dst_buffer[hnum * m_native_window_buffer.stride + wnum +
                          offset] = palette_texture_pixels[screen_buffer[
                        y * global_config::k_screen_width + x]];
            }
        }
        ANativeWindow_unlockAndPost(native_window);
    }

    void init() {
        log_d(tag, "init()");
        ANativeWindow *native_window = native_graphic::get_native_window();
        uint8_t *screen_buffer = native_graphic::get_screen_buffer();
        ANativeWindow_acquire(native_window);
        while (native_graphic::is_render_running()) {
            on_soft_draw();
            native_graphic::require_logic_render();
        }
        if (native_window) {
            ANativeWindow_release(native_window);
        }
    }
}

namespace metalmax::opengl_render {
    const char *tag = "opengl_render";

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint surface_attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };

    EGLint context_attrib_list[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

    EGLint format;
    EGLint major, minor;
    EGLConfig *config;

    EGLSurface surface;
    EGLContext context;
    EGLDisplay egl_display;

    const int coords_per_vertex = 3;
    const int coords_per_texture = 2;

    int vertex_stride = coords_per_vertex * 4;
    int texture_stride = coords_per_texture * 4;

    volatile float global_fade_rate = 1.1f;// won't apply any fade when rate>1.0

    //form 0-100
    void set_global_fade_rate(unsigned int fade_percent) {
        if (fade_percent < 100) {
            global_fade_rate = (fade_percent * 1.f / 100.f);
        } else {
            global_fade_rate = 1.1f;
        }
        log_d(tag, "globalFadeRate:%f", global_fade_rate);
    }

    float proj_matrix[16];
    GLint position_handle;
    GLint texture_handle;
    GLint palette_handle;
    GLint tex_coord_handle;
    GLint mvp_matrix_handle;
    GLint global_fade_rate_handle;

    unsigned int program;
    unsigned int palette_texture_id;
    unsigned int main_texture_id;

    const char *fragment_shader = "precision mediump float;"
                                  "varying vec2 v_texCoord;"
                                  "uniform sampler2D s_texture;"
                                  "uniform sampler2D s_palette; "
                                  "uniform float u_global_fade_rate; "//全局fade in/out效果的比例，从0.0-1.0，大于1.0不执行fade操作
                                  "void main()"
                                  "{           "
                                  "  float a = texture2D(s_texture, v_texCoord).a;"
                                  "  float x = (floor(a * 255.0 + 0.5) + 0.5) / 256.0;"
                                  "  vec2 curPt = vec2(x, 0);"
                                  "  vec3 current_rgb = texture2D(s_palette, curPt).rgb;"
                                  "  if (u_global_fade_rate <= 1.0) {"
                                  "    current_rgb = current_rgb * u_global_fade_rate;"
                                  "  }"
                                  "  gl_FragColor = vec4(current_rgb, 1.0);"
                                  "}";

    const char *vertex_shader = "attribute vec4 a_position;"
                                "attribute vec2 a_texCoord;"
                                "uniform mat4 uMVPMatrix;"
                                "varying lowp vec2 v_texCoord;"
                                "void main()"
                                "{"
                                "  gl_Position =  uMVPMatrix  * a_position;"
                                "  v_texCoord = a_texCoord;"
                                "}";

    float quad_coords[12];
    float texture_coords[8];
    short draw_order[] = {0, 1, 2, 0, 2, 3};

    void init_quad_coordinates(int width, int height) {
        float temp_quad_coords[] = {
                -width / 2.0f, -height / 2.0f, 0,
                -width / 2.0f, height / 2.0f, 0,
                width / 2.0f, height / 2.0f, 0,
                width / 2.0f, -height / 2.0f, 0
        };
        memcpy(quad_coords, temp_quad_coords, 12 * sizeof(float));
        float temp_texture_coords[] = {
                0,
                1,
                0,
                0,
                1,
                0,
                1,
                1,
        };
        memcpy(texture_coords, temp_texture_coords, 8 * sizeof(float));
    }

    void check_gl_error(const char *gl_operation) {
        GLenum error;
        while ((error = glGetError()) != GL_NO_ERROR) {
            __android_log_print(ANDROID_LOG_ERROR, "GLView", "%s: glError :%ud", gl_operation,
                                error);
        }
    }

    unsigned int load_shader(const char *shader_code, GLenum type) {
        int compiled;
        unsigned int shader = glCreateShader(type);
        glShaderSource(shader, 1, &shader_code, nullptr);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            const size_t info_log_length = 1024;
            char *info_log = (char *) malloc(sizeof(char) * info_log_length);
            glGetShaderInfoLog(shader, info_log_length, nullptr, info_log);
            log_e(tag, "compile shader fail: %s", info_log);
            free(info_log);
            return 0;
        }
        return shader;
    }

    unsigned int load_program(const char *v_shader_code, const char *f_shader_code) {
        unsigned int i_vshader, i_f_shader, i_program_id;
        int linked;
        i_vshader = load_shader(v_shader_code, GL_VERTEX_SHADER);
        i_f_shader = load_shader(f_shader_code, GL_FRAGMENT_SHADER);
        if (!(i_vshader && i_f_shader)) {
            log_e(tag, "shader fail");
            return 0;
        }
        i_program_id = glCreateProgram();
        glAttachShader(i_program_id, i_vshader);
        glAttachShader(i_program_id, i_f_shader);
        glLinkProgram(i_program_id);
        glGetProgramiv(i_program_id, GL_LINK_STATUS, &linked);
        if (!linked) {
            const size_t info_log_length = 1024;
            char *info_log = (char *) malloc(sizeof(char) * info_log_length);
            glGetProgramInfoLog(program, info_log_length, nullptr, info_log);
            log_e(tag, "program linked fail: %d", linked);
            free(info_log);
            return 0;
        }
        glDeleteShader(i_vshader);
        glDeleteShader(i_f_shader);
        return i_program_id;
    }

    void init_textures() {
        log_d(tag, "initTextures()");
        GLsizei num_textures = 2;
        GLuint texture_ids[2];
        int texture_width = global_config::k_screen_width;
        int texture_height = global_config::k_screen_height;
        glGenTextures(num_textures, texture_ids);
        glBindTexture(GL_TEXTURE_2D, texture_ids[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, texture_width,
                     texture_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


        glBindTexture(GL_TEXTURE_2D, texture_ids[1]);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);



        //palette!
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, native_graphic::get_palette_size(), 1, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     native_graphic::get_palette_buffer());
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        palette_texture_id = texture_ids[1];
        main_texture_id = texture_ids[0];
        check_gl_error("textures");
    }

    void init_gl_view_port() {
        log_d(tag, "initGlViewPort()");
        int window_width = native_graphic::get_window_width();
        int window_height = native_graphic::get_window_height();
        ortho_m(proj_matrix, 0, -window_width / 2.0f, +window_width / 2.0f, -window_height / 2.0f,
               +window_height / 2.0f, -2.0f, 2.0f);
        glViewport(0, 0, window_width, window_height);
        int gl_height = window_height;
        int gl_width =
                gl_height * (global_config::k_screen_width * 1.f / global_config::k_screen_height);
        log_d(tag, "gl_height: %d, gl_width: %d", gl_height, gl_width);
        init_quad_coordinates(gl_width, gl_height);
        glUseProgram(program);
        position_handle = glGetAttribLocation(program, "a_position");
        texture_handle = glGetUniformLocation(program, "s_texture");
        global_fade_rate_handle = glGetUniformLocation(program, "u_global_fade_rate");
        palette_handle = glGetUniformLocation(program, "s_palette");
        tex_coord_handle = glGetAttribLocation(program, "a_texCoord");
    }

    void init_open_gl() {
        log_d(tag, "initOpenGl()");
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        program = load_program(vertex_shader, fragment_shader);
        init_textures();
        init_gl_view_port();
    }

    void on_draw() {
        lock_logic();
        uint8_t *screen_buffer = native_graphic::get_screen_buffer();
        if (screen_buffer == nullptr) {
            unlock_logic();
            return;
        }
        glClear(GL_COLOR_BUFFER_BIT);
        glEnableVertexAttribArray(position_handle);
        glEnableVertexAttribArray(tex_coord_handle);
        check_gl_error("handles");
        glVertexAttribPointer(position_handle, coords_per_vertex, GL_FLOAT,
                              false, vertex_stride, quad_coords);
        glVertexAttribPointer(tex_coord_handle, coords_per_texture, GL_FLOAT,
                              false, texture_stride, texture_coords);
        mvp_matrix_handle = glGetUniformLocation(program, "uMVPMatrix");
        glUniformMatrix4fv(mvp_matrix_handle, 1, GL_FALSE, proj_matrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, main_texture_id);
        glUniform1i(texture_handle, 0);
        glUniform1f(global_fade_rate_handle, global_fade_rate);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, palette_texture_id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, native_graphic::get_palette_size(), 1, GL_RGBA,
                        GL_UNSIGNED_BYTE, native_graphic::get_palette_buffer());
        glUniform1i(palette_handle, 1);

        glActiveTexture(GL_TEXTURE0);
        check_gl_error("uniforms");
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, global_config::k_screen_width,
                        global_config::k_screen_height, GL_ALPHA,
                        GL_UNSIGNED_BYTE, screen_buffer);
        check_gl_error("emu render");
        glDrawElements(GL_TRIANGLES, 6,
                       GL_UNSIGNED_SHORT, draw_order);
        glDisableVertexAttribArray(position_handle);
        glDisableVertexAttribArray(tex_coord_handle);
        check_gl_error("disable vertex arrays");
        unlock_logic();
        eglSwapBuffers(egl_display, surface);
    }

    void init_egl() {
        log_d(tag, "initEgl()");
        ANativeWindow *native_window = native_graphic::get_native_window();
        EGLint config_count;
        egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        eglInitialize(egl_display, &major, &minor);
        eglChooseConfig(egl_display, surface_attribs, nullptr, 0, &config_count);
        if (config_count == 0) {
            return;
        }
        config = (EGLConfig *) malloc(config_count * sizeof(EGLConfig));
        eglChooseConfig(egl_display, surface_attribs, config, config_count, &config_count);
        eglGetConfigAttrib(egl_display, config[0], EGL_NATIVE_VISUAL_ID, &format);
        surface = eglCreateWindowSurface(egl_display, config[0], native_window, nullptr);
        context = eglCreateContext(egl_display, config[0], nullptr, context_attrib_list);
        eglMakeCurrent(egl_display, surface, surface, context);
    }

    void release_egl() {
        log_d(tag, "releaseEgl()");
        glDeleteTextures(1, &main_texture_id);
        glDeleteTextures(1, &palette_texture_id);
        glDeleteProgram(program);
        eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(egl_display, context);
        eglDestroySurface(egl_display, surface);
        eglReleaseThread();
        eglTerminate(egl_display);
        free(config);
    }

    void init() {
        log_d(tag, "init()");
        init_egl();
        init_open_gl();
        while (native_graphic::is_render_running()) {
            native_graphic::require_logic_render();
            on_draw();
        }
        release_egl();
    }
}


namespace metalmax::native_graphic {

    const char *tag = "native_graphic";

    const int palette_texture_size = 256;
    int *palette_texture_pixels;

    static ANativeWindow *m_a_native_window;

    volatile int32_t window_height;
    volatile int32_t window_width;

    uint8_t *(*render_buffer_callback)(uint8_t *screen_buffer);

    uint8_t *screen_buffer;

    std::atomic<bool> render_running(false);
    static pthread_t render_thread;
    static bool render_thread_started = false;

    //fps recorder
    volatile bool enable_fps = true;
    volatile long duration = 0;
    timespec time1;
    timespec time2;
    bool first_frame = true;
    long frame_duration_ms = 0;
    int frame_count = 0;

    bool is_render_running() {
        return render_running;
    }

    uint8_t *get_screen_buffer() {
        return screen_buffer;
    }

    ANativeWindow *get_native_window() {
        return m_a_native_window;
    }

    int *get_palette_buffer() {
        return palette_texture_pixels;
    }

    size_t get_palette_size() {
        return palette_texture_size;
    }

    void set_render_callback(uint8_t *(*render_screen_buffer)(uint8_t *screen_buffer)) {
        render_buffer_callback = render_screen_buffer;
    }

    void init_palette() {
        game_data::apply_palette();
        palette_texture_pixels = (int *) malloc(sizeof(int) * palette_texture_size);
        for (int i = 0; i < palette::palette_size; i++) {
            int dd = palette::palette_rgb[i];
            int b = (dd & 0x00ff0000) >> 16;
            int g = (dd & 0x0000ff00) >> 8;
            int r = (dd & 0x000000ff) >> 0;
            palette_texture_pixels[i] = 0xff000000 | (r << 16) | (g << 8) | b;
        }
    }

    int get_fps() {
        if (duration <= 0) { return 0; }
        float fps = 1000.0f / (duration * 1.f);
        return (int) fps;
    }

    inline void calculate_fps() {
        if (enable_fps) {
            if (first_frame) {
                first_frame = false;
            } else {
                clock_gettime(CLOCK_MONOTONIC, &time2);
                frame_duration_ms += ((time2.tv_sec - time1.tv_sec) * 1000 +
                                    (time2.tv_nsec - time1.tv_nsec) / 1000000);
                frame_count++;
                if (frame_duration_ms >= 200) {
                    duration = frame_duration_ms / frame_count;
                    frame_count = 0;
                    frame_duration_ms = 0;
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &time1);
        }
    }

    void require_logic_render() {
        calculate_fps();
        if (render_buffer_callback) { render_buffer_callback(screen_buffer); }
    }

    int get_window_width() {
        return window_width;
    }

    int get_window_height() {
        return window_height;
    }

    void vulkan() {
        log_d(tag, "use vulkan");
        log_e(tag, "current version not support vulkan");
        exit(-1);
    }

    void *render_thread_task(void *arg) {
        log_i(tag, "render thread start!");
        int32_t n_window_width = ANativeWindow_getWidth(m_a_native_window);
        int32_t n_window_height = ANativeWindow_getHeight(m_a_native_window);
        lock_logic();
        if (n_window_width > 0 && n_window_height > 0) {
            window_height = n_window_height;
            window_width = n_window_width;
        }
        log_d(tag, "window w&h: %d, %d", window_height, window_width);
        ANativeWindow_setBuffersGeometry(m_a_native_window,
                                         window_width,
                                         window_height,
                                         WINDOW_FORMAT_RGBA_8888);
        unlock_logic();
        if (global_config::k_render_mode == global_config::render_mode::k_software) {
            software_render::init();
        } else if (global_config::k_render_mode == global_config::render_mode::k_open_gl) {
            opengl_render::init();
        } else if (global_config::k_render_mode == global_config::render_mode::k_vulkan) {
            vulkan();
        }
        return nullptr;
    }

    bool init_graphic(ANativeWindow *window) {
        log_d(tag, "initGraphic");
        if (render_thread_started) { release_graphic(); }
        m_a_native_window = window;
        lock_logic();
        init_palette();
        screen_buffer = (uint8_t *) calloc(global_config::k_screen_buffer_size, 1);
        render_running = true;
        unlock_logic();
        render_thread_started =
                pthread_create(&render_thread, nullptr, render_thread_task, m_a_native_window) == 0;
        if (!render_thread_started) {
            render_running = false;
            free(screen_buffer);
            free(palette_texture_pixels);
            screen_buffer = nullptr;
            palette_texture_pixels = nullptr;
            m_a_native_window = nullptr;
        }
        return render_thread_started;
    }

    void set_global_fade_percent(unsigned int percent) {
        opengl_render::set_global_fade_rate(percent);
    }

    void apply_new_palette(int *new_palette) {
        log_d(tag, "applyNewPalette");
        if (new_palette && new_palette != palette_texture_pixels && palette_texture_pixels) {
            memcpy(palette_texture_pixels, new_palette, sizeof(int) * palette_texture_size);
        }
    }

    void release_graphic() {
        log_d(tag, "releaseGraphic");
        render_running = false;
        if (render_thread_started) {
            pthread_join(render_thread, nullptr);
            render_thread_started = false;
        }
        lock_logic();
        free(screen_buffer);
        free(palette_texture_pixels);
        screen_buffer = nullptr;
        palette_texture_pixels = nullptr;
        if (m_a_native_window) {
            ANativeWindow_release(m_a_native_window);
            m_a_native_window = nullptr;
        }
        unlock_logic();
    }

}
