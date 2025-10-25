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
#include <__threading_support>
#include "../global.h"

#include "../maps/map.h"
#include "../charset/charsets.h"
#include "../maps/map_data/map_data.h"
#include "palette_data.h"
#include "../sprite/sprite.h"
#include <EGL/egl.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <unistd.h>

namespace software_render {

    const char *TAG = "software_render";

    void onSoftDraw() {
        //direct draw to buffer
        uint8_t *screenBuffer = native_graphic::getScreenBuffer();
        if (screenBuffer == nullptr) {
            return;
        }
        ANativeWindow *nativeWindow = native_graphic::getNativeWindow();
        ANativeWindow_Buffer mNativeWindowBuffer;
        ANativeWindow_lock(nativeWindow, &mNativeWindowBuffer, nullptr);
        int *dstBuffer = static_cast<int *>(mNativeWindowBuffer.bits);
        int dstHeight = mNativeWindowBuffer.height;
        int dstWidth =
                dstHeight * (global_config::k_screen_width * 1.f / global_config::k_screen_height);
        int offset = (mNativeWindowBuffer.stride - mNativeWindowBuffer.width) / 2;
        if (!native_graphic::isRenderRunning()) {
            return;
        }
        memset(dstBuffer, 0,
               mNativeWindowBuffer.stride * mNativeWindowBuffer.height * sizeof(int)); //对数组清零
        float scale = (float) mNativeWindowBuffer.height /
                      (global_config::k_screen_height * 1.f); //计算图像宽度缩放比例
        float scaleLeft = scale - (int) scale; //求出缩放比例的小数部分
        int addArg = (scaleLeft > 0.5) ? 1 : 0;
        int x, y;

        int *palette_texture_pixels = native_graphic::getPaletteBuffer();

        for (int hnum = 0; hnum < dstHeight; ++hnum) //按照从左到右，从上到下的顺序进行转换
        {
            y = (int) (hnum / scale) + addArg;   //计算当前临近坐标的y值
            for (int wnum = 0; wnum < dstWidth; ++wnum) {
                x = (int) (wnum / scale) + addArg; //计算当前临近坐标的x值
                dstBuffer[hnum * mNativeWindowBuffer.stride + wnum +
                          offset] = palette_texture_pixels[screenBuffer[
                        y * global_config::k_screen_width + x]];
            }
        }
        ANativeWindow_unlockAndPost(nativeWindow);
    }

    void init() {
        LOGD(TAG, "init()");
        ANativeWindow *nativeWindow = native_graphic::getNativeWindow();
        uint8_t *screenBuffer = native_graphic::getScreenBuffer();
        ANativeWindow_acquire(nativeWindow);
        while (native_graphic::isRenderRunning()) {
            onSoftDraw();
            native_graphic::requireLogicRender();
        }
        if (nativeWindow) {
            ANativeWindow_release(nativeWindow);
        }
    }
}

namespace opengl_render {
    const char *TAG = "opengl_render";

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
    EGLDisplay eglDisplay;

    const int COORDS_PER_VERTEX = 3;
    const int COORDS_PER_TEXTURE = 2;

    int VERTEX_STRIDE = COORDS_PER_VERTEX * 4;
    int TEXTURE_STRIDE = COORDS_PER_TEXTURE * 4;

    volatile float globalFadeRate = 1.1f;// won't apply any fade when rate>1.0

    //form 0-100
    void setGlobalFadeRate(unsigned int fadePercent) {
        if (fadePercent < 100) {
            globalFadeRate = (fadePercent * 1.f / 100.f);
        } else {
            globalFadeRate = 1.1f;
        }
        LOGD(TAG, "globalFadeRate:%f", globalFadeRate);
    }

    float projMatrix[16];
    GLint positionHandle;
    GLint textureHandle;
    GLint paletteHandle;
    GLint texCoordHandle;
    GLint mvpMatrixHandle;
    GLint globalFadeRateHandle;

    unsigned int program;
    unsigned int paletteTextureId;
    unsigned int mainTextureId;

    const char *FRAGMENT_SHADER = "precision mediump float;"
                                  "varying vec2 v_texCoord;"
                                  "uniform sampler2D s_texture;"
                                  "uniform sampler2D s_palette; "
                                  "uniform float u_global_fade_rate; "//全局fade in/out效果的比例，从0.0-1.0，大于1.0不执行fade操作
                                  "void main()"
                                  "{           "
                                  "  float a = texture2D(s_texture, v_texCoord).a;"
                                  "  float c = floor((a * 256.0) / 127.5);"
                                  "  float x = a - c * 0.001953;"// = 1 / (screen_width * 2) //0.001953
                                  "  vec2 curPt = vec2(x, 0);"
                                  "  vec3 current_rgb = texture2D(s_palette, curPt).rgb;"
                                  "  if (u_global_fade_rate <= 1.0) {"
                                  "    current_rgb = current_rgb * u_global_fade_rate;"
                                  "  }"
                                  "  gl_FragColor.rgb = current_rgb;"//vec3(0.0, 0.0, 1.0);
                                  "}";

    const char *VERTEX_SHADER = "attribute vec4 a_position;"
                                "attribute vec2 a_texCoord;"
                                "uniform mat4 uMVPMatrix;"
                                "varying lowp vec2 v_texCoord;"
                                "void main()"
                                "{"
                                "  gl_Position =  uMVPMatrix  * a_position;"
                                "  v_texCoord = a_texCoord;"
                                "}";

    float quadCoords[12];
    float textureCoords[8];
    short drawOrder[] = {0, 1, 2, 0, 2, 3};

    void initQuadCoordinates(int width, int height) {
        float tempQuadCoords[] = {
                -width / 2.0f, -height / 2.0f, 0,
                -width / 2.0f, height / 2.0f, 0,
                width / 2.0f, height / 2.0f, 0,
                width / 2.0f, -height / 2.0f, 0
        };
        memcpy(quadCoords, tempQuadCoords, 12 * sizeof(float));
        float tempTextureCoords[] = {
                0,
                1,
                0,
                0,
                1,
                0,
                1,
                1,
        };
        memcpy(textureCoords, tempTextureCoords, 8 * sizeof(float));
    }

    void checkGlError(const char *glOperation) {
        GLenum error;
        while ((error = glGetError()) != GL_NO_ERROR) {
            __android_log_print(ANDROID_LOG_ERROR, "GLView", "%s: glError :%ud", glOperation,
                                error);
        }
    }

    unsigned int loadShader(const char *shaderCode, GLenum type) {
        int compiled;
        unsigned int shader = glCreateShader(type);
        glShaderSource(shader, 1, &shaderCode, nullptr);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            const size_t infoLogLength = 1024;
            char *infoLog = (char *) malloc(sizeof(char) * infoLogLength);
            glGetShaderInfoLog(shader, infoLogLength, nullptr, infoLog);
            LOGE(TAG, "compile shader fail: %s", infoLog);
            free(infoLog);
            return 0;
        }
        return shader;
    }

    unsigned int loadProgram(const char *VShaderCode, const char *FShaderCode) {
        unsigned int iVshader, iFShader, iProgramId;
        int linked;
        iVshader = loadShader(VShaderCode, GL_VERTEX_SHADER);
        iFShader = loadShader(FShaderCode, GL_FRAGMENT_SHADER);
        if (!(iVshader && iFShader)) {
            LOGE(TAG, "shader fail");
            return 0;
        }
        iProgramId = glCreateProgram();
        glAttachShader(iProgramId, iVshader);
        glAttachShader(iProgramId, iFShader);
        glLinkProgram(iProgramId);
        glGetProgramiv(iProgramId, GL_LINK_STATUS, &linked);
        if (!linked) {
            const size_t infoLogLength = 1024;
            char *infoLog = (char *) malloc(sizeof(char) * infoLogLength);
            glGetProgramInfoLog(program, infoLogLength, nullptr, infoLog);
            LOGE(TAG, "program linked fail: %d", linked);
            free(infoLog);
            return 0;
        }
        glDeleteShader(iVshader);
        glDeleteShader(iFShader);
        return iProgramId;
    }

    void initTextures() {
        LOGD(TAG, "initTextures()");
        GLsizei numTextures = 2;
        GLuint textureIds[2];
        int textureWidth = global_config::k_screen_width;
        int textureHeight = global_config::k_screen_height;
        glGenTextures(numTextures, textureIds);
        glBindTexture(GL_TEXTURE_2D, textureIds[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, textureWidth,
                     textureHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, nullptr);
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


        glBindTexture(GL_TEXTURE_2D, textureIds[1]);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);



        //palette!
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, native_graphic::getPaletteSize(), 1, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     native_graphic::getPaletteBuffer());
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        paletteTextureId = textureIds[1];
        mainTextureId = textureIds[0];
        checkGlError("textures");
    }

    void initGlViewPort() {
        LOGD(TAG, "initGlViewPort()");
        int window_width = native_graphic::getWindowWidth();
        int window_height = native_graphic::getWindowHeight();
        orthoM(projMatrix, 0, -window_width / 2.0f, +window_width / 2.0f, -window_height / 2.0f,
               +window_height / 2.0f, -2.0f, 2.0f);
        glViewport(0, 0, window_width, window_height);
        int gl_height = window_height;
        int gl_width =
                gl_height * (global_config::k_screen_width * 1.f / global_config::k_screen_height);
        LOGD(TAG, "gl_height: %d, gl_width: %d", gl_height, gl_width);
        initQuadCoordinates(gl_width, gl_height);
        glUseProgram(program);
        positionHandle = glGetAttribLocation(program, "a_position");
        textureHandle = glGetUniformLocation(program, "s_texture");
        globalFadeRateHandle = glGetUniformLocation(program, "u_global_fade_rate");
        paletteHandle = glGetUniformLocation(program, "s_palette");
        texCoordHandle = glGetAttribLocation(program, "a_texCoord");
    }

    void initOpenGl() {
        LOGD(TAG, "initOpenGl()");
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        program = loadProgram(VERTEX_SHADER, FRAGMENT_SHADER);
        initTextures();
        initGlViewPort();
    }

    void onDraw() {
        uint8_t *screenBuffer = native_graphic::getScreenBuffer();
        if (screenBuffer == nullptr) {
            return;
        }
        glClear(GL_COLOR_BUFFER_BIT);
        glEnableVertexAttribArray(positionHandle);
        glEnableVertexAttribArray(texCoordHandle);
        checkGlError("handles");
        glVertexAttribPointer(positionHandle, COORDS_PER_VERTEX, GL_FLOAT,
                              false, VERTEX_STRIDE, quadCoords);
        glVertexAttribPointer(texCoordHandle, COORDS_PER_TEXTURE, GL_FLOAT,
                              false, TEXTURE_STRIDE, textureCoords);
        mvpMatrixHandle = glGetUniformLocation(program, "uMVPMatrix");
        glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE, projMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mainTextureId);
        glUniform1i(textureHandle, 0);
        glUniform1f(globalFadeRateHandle, globalFadeRate);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, paletteTextureId);
        glUniform1i(paletteHandle, 1);

        glActiveTexture(GL_TEXTURE0);
        checkGlError("uniforms");
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, global_config::k_screen_width,
                        global_config::k_screen_height, GL_ALPHA,
                        GL_UNSIGNED_BYTE, screenBuffer);
        checkGlError("emu render");
        glDrawElements(GL_TRIANGLES, 6,
                       GL_UNSIGNED_SHORT, drawOrder);
        glDisableVertexAttribArray(positionHandle);
        glDisableVertexAttribArray(texCoordHandle);
        checkGlError("disable vertex arrays");
        eglSwapBuffers(eglDisplay, surface);
    }

    void initEgl() {
        LOGD(TAG, "initEgl()");
        ANativeWindow *nativeWindow = native_graphic::getNativeWindow();
        EGLint configCount;
        eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        eglInitialize(eglDisplay, &major, &minor);
        eglChooseConfig(eglDisplay, surface_attribs, nullptr, 0, &configCount);
        if (configCount == 0) {
            return;
        }
        config = (EGLConfig *) malloc(configCount * sizeof(EGLConfig));
        eglChooseConfig(eglDisplay, surface_attribs, config, configCount, &configCount);
        eglGetConfigAttrib(eglDisplay, config[0], EGL_NATIVE_VISUAL_ID, &format);
        surface = eglCreateWindowSurface(eglDisplay, config[0], nativeWindow, nullptr);
        context = eglCreateContext(eglDisplay, config[0], nullptr, context_attrib_list);
        eglMakeCurrent(eglDisplay, surface, surface, context);
    }

    void releaseEgl() {
        LOGD(TAG, "releaseEgl()");
        eglReleaseThread();
        eglTerminate(eglDisplay);
        free(config);
    }

    void init() {
        LOGD(TAG, "init()");
        initEgl();
        initOpenGl();
        while (native_graphic::isRenderRunning()) {
            onDraw();
            native_graphic::requireLogicRender();
        }
        releaseEgl();
    }
}


namespace native_graphic {

    const char *TAG = "native_graphic";

    const int palette_texture_size = 256;
    int *palette_texture_pixels;

    static ANativeWindow *mANativeWindow;

    volatile int32_t window_height;
    volatile int32_t window_width;

    uint8_t *(*renderBufferCallback)(uint8_t *screenBuffer);

    uint8_t *screenBuffer;

    volatile bool renderRunning = true;

    //fps recorder
    volatile bool enableFps = true;
    volatile long duration = 0;
    timespec time1;
    timespec time2;
    bool firstFrame = true;
    long frameDurationMs = 0;
    int frameCount = 0;

    bool isRenderRunning() {
        return renderRunning;
    }

    uint8_t *getScreenBuffer() {
        return screenBuffer;
    }

    ANativeWindow *getNativeWindow() {
        return mANativeWindow;
    }

    int *getPaletteBuffer() {
        return palette_texture_pixels;
    }

    size_t getPaletteSize() {
        return palette_texture_size;
    }

    void setRenderCallback(uint8_t *(*renderScreenBuffer)(uint8_t *screenBuffer)) {
        renderBufferCallback = renderScreenBuffer;
    }

    void initPalette() {
        palette_texture_pixels = (int *) malloc(sizeof(int) * palette_texture_size);
        for (int i = 0; i < palette::palette_size; i++) {
            int dd = palette::palette_rgb[i];
            int b = (dd & 0x00FF0000) >> 16;
            int g = (dd & 0x0000FF00) >> 8;
            int r = (dd & 0x000000FF) >> 0;
            palette_texture_pixels[i] = 0xff000000 | (r << 16) | (g << 8) | b;
        }
    }

    int getFps() {
        float fps = 1000.0f / (duration * 1.f);
        return (int) fps;
    }

    inline void calculateFps() {
        if (enableFps) {
            if (firstFrame) {
                firstFrame = false;
            } else {
                clock_gettime(CLOCK_MONOTONIC, &time2);
                frameDurationMs += ((time2.tv_sec - time1.tv_sec) * 1000 +
                                    (time2.tv_nsec - time1.tv_nsec) / 1000000);
                frameCount++;
                if (frameDurationMs >= 200) {
                    duration = frameDurationMs / frameCount;
                    frameCount = 0;
                    frameDurationMs = 0;
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &time1);
        }
    }

    void requireLogicRender() {
        calculateFps();
        renderBufferCallback(screenBuffer);
    }

    int getWindowWidth() {
        return window_width;
    }

    int getWindowHeight() {
        return window_height;
    }

    volatile bool needRefreshPalette = false;

    void vulkan() {
        LOGD(TAG, "use vulkan");
        LOGE(TAG, "current version not support vulkan");
        exit(-1);
    }

    void *render_thread_task(void *arg) {
        LOGI(TAG, "render thread start!");
        int32_t n_window_width = ANativeWindow_getWidth(mANativeWindow);
        int32_t n_window_height = ANativeWindow_getHeight(mANativeWindow);
        if (n_window_width > 0 && n_window_height > 0) {
            window_height = n_window_height;
            window_width = n_window_width;
        }
        LOGD(TAG, "window w&h: %d, %d", window_height, window_width);
        ANativeWindow_setBuffersGeometry(mANativeWindow,
                                         window_width,
                                         window_height,
                                         WINDOW_FORMAT_RGBA_8888);
        renderRunning = true;
        if (global_config::k_render_mode == global_config::RenderMode::kSoftware) {
            software_render::init();
        } else if (global_config::k_render_mode == global_config::RenderMode::kOpenGL) {
            opengl_render::init();
        } else if (global_config::k_render_mode == global_config::RenderMode::kVulkan) {
            vulkan();
        }
        return nullptr;
    }

    bool initGraphic(ANativeWindow *window) {
        LOGD(TAG, "initGraphic");
        mANativeWindow = window;
        pthread_t id;
        initPalette();
        screenBuffer = (uint8_t *) malloc(global_config::k_screen_buffer_size);
        pthread_create(&id, nullptr, render_thread_task, mANativeWindow);
        return true;
    }

    void setGlobalFadePercent(unsigned int percent) {
        opengl_render::setGlobalFadeRate(percent);
    }

    void applyNewPalette(int *newPalette) {
        LOGD(TAG, "applyNewPalette");
        needRefreshPalette = true;
        palette_texture_pixels = newPalette;
    }

    void releaseGraphic() {
        LOGD(TAG, "releaseGraphic");
        renderRunning = false;
        free(screenBuffer);
        free(palette_texture_pixels);
    }

}
