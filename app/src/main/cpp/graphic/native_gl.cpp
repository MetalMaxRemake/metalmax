//
// Created by youngpark on 2023/4/23.
//

#include "native_gl.h"
#include "../opt/matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>
#include <__threading_support>

#define WIDTH 256
#define HEIGHT_PAL 240
#define HEIGHT_NTSC 224

#include "../maps/map.h"
#include "../charset/charsets.h"
#include "../opt/mem_opt.h"
#include "../maps/map_data/map_data.h"
#include "palette_data.h"
#include "../perf.h"
#include "../sprite/sprite.h"
#include <EGL/egl.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <unistd.h>

/**
 * EGL
 */
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

EGLint context_attrib_list[] = {EGL_CONTEXT_CLIENT_VERSION, 2,EGL_NONE };

EGLint format;
EGLint major, minor;

EGLSurface surface;
EGLContext context;
EGLDisplay eglDisplay;


static ANativeWindow* mANativeWindow;
static ANativeWindow_Buffer nwBuffer;

volatile int window_height, window_width;
/**
 * EGL
 */

void * currentScreenBuffer;

const int COORDS_PER_VERTEX = 3;
const int COORDS_PER_TEXTURE = 2;

int VERTEX_STRIDE = COORDS_PER_VERTEX * 4;
int TEXTURE_STRIDE = COORDS_PER_TEXTURE * 4;

float projMatrix[16];
int positionHandle, textureHandle, paletteHandle, texCoordHandle, mvpMatrixHandle;

unsigned int program;
unsigned int paletteTextureId;
unsigned int mainTextureId;

int getTextureSize() {
    return 256;
}

const char *FRAGMENT_SHADER = "precision mediump float;"
                              "varying vec2 v_texCoord;"
                              "uniform sampler2D s_texture;"
                              "uniform sampler2D s_palette; "
                              "void main()"
                              "{           "
                              "		 float a = texture2D(s_texture, v_texCoord).a;"
                              "	     float c = floor((a * 256.0) / 127.5);"
                              "      float x = a - c * 0.001953;"
                              "      vec2 curPt = vec2(x, 0);"
                              "      gl_FragColor.rgb = texture2D(s_palette, curPt).rgb;"
                              "}";

const char *VERTEX_SHADER = "attribute vec4 a_position;"
                            "attribute vec2 a_texCoord;"
                            "uniform mat4 uMVPMatrix;"
                            "varying lowp vec2 v_texCoord;"
                            "void main()"
                            "{"
                            "   gl_Position =  uMVPMatrix  * a_position;"
                            "   v_texCoord = a_texCoord;"
                            "}";

float quadCoords[12];
float textureCoords[8];
short drawOrder[] = {0, 1, 2, 0, 2, 3};

void initQuadCoordinates(int width, int height) {
    int maxTexX = WIDTH;
    int maxTexY = HEIGHT_PAL;
    int textureSize = getTextureSize();
    float tempQuadCoords[] = {
            -width / 2.0f, -height / 2.0f, 0,
            -width / 2.0f, height / 2.0f, 0,
            width / 2.0f, height / 2.0f, 0,
            width / 2.0f, -height / 2.0f, 0
    };
    __memcpy_aarch64_simd(quadCoords, tempQuadCoords, 12 * sizeof (float));
    float tempTextureCoords[] = {
            0,
            maxTexY / (float) textureSize,
            0,
            0,
            maxTexX / (float) textureSize,
            0,
            maxTexX / (float) textureSize,
            maxTexY / (float) textureSize,
    };
    __memcpy_aarch64_simd(textureCoords, tempTextureCoords, 8 * sizeof (float));
}

void checkGlError(const char *glOperation) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, "GLView", "%s: glError :%ud", glOperation, error);
    }
}

unsigned int loadShader(const char *shaderCode, GLenum type) {
    int compiled;
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderCode, nullptr);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        __android_log_print(ANDROID_LOG_ERROR, "native_GL", "compile err");
        return 0;
    }
    return shader;
}

unsigned int loadProgram(const char *VShaderCode, const char *FShaderCode) {
    unsigned int iVshader, iFShader, iProgramId;
    int link;
    iVshader = loadShader(VShaderCode, GL_VERTEX_SHADER);
    iFShader = loadShader(FShaderCode, GL_FRAGMENT_SHADER);
    if (!(iVshader && iFShader)) {
        __android_log_print(ANDROID_LOG_ERROR, "native_GL", "shader err");
        return 0;
    }
    iProgramId = glCreateProgram();
    glAttachShader(iProgramId, iVshader);
    glAttachShader(iProgramId, iFShader);
    glLinkProgram(iProgramId);
    glGetProgramiv(iProgramId, GL_LINK_STATUS, &link);
    if (!link) {
        __android_log_print(ANDROID_LOG_ERROR, "native_GL", "link err");
        return 0;
    }
    glDeleteShader(iVshader);
    glDeleteShader(iFShader);
    return iProgramId;
}

void initTextures() {
    GLsizei numTextures = 2;
    GLuint textureIds[2];
    int textureWidth = 256;
    int textureHeight = 256;
    int paletteSize = 256;
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

    for (int i = 0; i < paletteSize; i++) {
        int dd = palette[i];
        int b = (dd & 0x00FF0000) >> 16;
        int g = (dd & 0x0000FF00) >> 8;
        int r = (dd & 0x000000FF) >> 0;
        palette[i] = 0xff000000 | (r << 16) | (g << 8) | b;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, paletteSize, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                palette);
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

unsigned char colorBlockBuffer[256][256];

void initRenderBuffer() {
    unsigned char color = 0;
    for (int j = 0; j < 16; j++) {
        for (int i = 0; i < 16; i++) {
            int startX = i*16;
            int startY = j*16;
            for(int x = startX; x<startX+16;x++) {
                for(int y = startY;y<startY+16;y++) {
                    colorBlockBuffer[x][y] = color;
                }
            }
            color++;
        }
    }
}

void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    program = loadProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    initTextures();
    initRenderBuffer();
}

void onGLSurfaceChange() {
    orthoM(projMatrix, 0, -window_width / 2.0f, +window_width / 2.0f, -window_height / 2.0f,
           +window_height / 2.0f, -2.0f, 2.0f);
    glViewport(0, 0, window_width, window_height);
    initQuadCoordinates(window_height, window_height);
    glUseProgram(program);
    positionHandle = glGetAttribLocation(program, "a_position");
    textureHandle = glGetUniformLocation(program, "s_texture");
    paletteHandle = glGetUniformLocation(program, "s_palette");
    texCoordHandle = glGetAttribLocation(program, "a_texCoord");
}

pthread_mutex_t onMutex;

void onGLDraw() {
    if(currentScreenBuffer == nullptr) {
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

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, paletteTextureId);
    glUniform1i(paletteHandle, 1);

    glActiveTexture(GL_TEXTURE0);
    checkGlError("uniforms");
    pthread_mutex_lock(&onMutex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT_PAL, GL_ALPHA,
                    GL_UNSIGNED_BYTE, currentScreenBuffer);
    pthread_mutex_unlock(&onMutex);
    checkGlError("emu render");
    glDrawElements(GL_TRIANGLES, 6,
                   GL_UNSIGNED_SHORT, drawOrder);
    glDisableVertexAttribArray(positionHandle);
    glDisableVertexAttribArray(texCoordHandle);
    checkGlError("disable vertex arrays");
    eglSwapBuffers(eglDisplay, surface);
}

bool debugColorModeEnable = false;

bool isColorDebugMode() {
    return debugColorModeEnable;
}

void updateScreenBuffer(unsigned char * buffer) {
    pthread_mutex_lock(&onMutex);
    currentScreenBuffer = buffer;
    pthread_mutex_unlock(&onMutex);
}

void initEGL(ANativeWindow* window) {
    mANativeWindow = window;
    //初始化EGL
    EGLint configCount;
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(eglDisplay, &major, &minor);
    eglChooseConfig(eglDisplay, surface_attribs, nullptr, 0, &configCount);
    if(configCount == 0) {
        return;
    }
    EGLConfig *config = (EGLConfig *)malloc(configCount * sizeof (EGLConfig));
    eglChooseConfig(eglDisplay, surface_attribs, config, configCount, &configCount);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(eglDisplay, config[0], EGL_NATIVE_VISUAL_ID, &format);
    surface = eglCreateWindowSurface(eglDisplay, config[0], mANativeWindow, nullptr);
    context = eglCreateContext(eglDisplay, config[0], nullptr, context_attrib_list);
    eglMakeCurrent(eglDisplay, surface, surface, context);
}

//定义线程函数
[[noreturn]] void *gl_thread(void *arg) {
    window_height = ANativeWindow_getHeight(mANativeWindow);
    window_width = ANativeWindow_getWidth(mANativeWindow);
    ANativeWindow_setBuffersGeometry(mANativeWindow,
                                     window_width,
                                     window_height,
                                     WINDOW_FORMAT_RGBA_8888);
    initEGL(mANativeWindow);
    initGL();
    onGLSurfaceChange();
    while (true) {
        onGLDraw();
    }
}

void initGraphic(ANativeWindow* window) {
    mANativeWindow = window;
    pthread_t id;
    pthread_create(&id, NULL, gl_thread, mANativeWindow);
}
