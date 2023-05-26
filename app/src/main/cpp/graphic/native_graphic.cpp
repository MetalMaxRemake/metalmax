//
// Created by youngpark on 2023/4/23.
// EGL & FrameBuffer & OpenGL
//

#include "native_graphic.h"
#include "../opt/matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>
#include <__threading_support>

#define WIDTH 256
#define HEIGHT_PAL 240

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
#include "../perf.h"

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

EGLint context_attrib_list[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

EGLint format;
EGLint major, minor;
EGLConfig *config;

EGLSurface surface;
EGLContext context;
EGLDisplay eglDisplay;


static ANativeWindow *mANativeWindow;
static ANativeWindow_Buffer nwBuffer;

volatile int window_height, window_width;
/**
 * OpenGL
 */
const int paletteSize = 256;

byte*(*renderBuffer)(byte* screenBuffer);

byte *currentScreenBuffer;

extern volatile bool graphicRunning;

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
    __memcpy_aarch64_simd(quadCoords, tempQuadCoords, 12 * sizeof(float));
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
    __memcpy_aarch64_simd(textureCoords, tempTextureCoords, 8 * sizeof(float));
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

void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    program = loadProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    initTextures();
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
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT_PAL, GL_ALPHA,
                    GL_UNSIGNED_BYTE, currentScreenBuffer);
    checkGlError("emu render");
    glDrawElements(GL_TRIANGLES, 6,
                   GL_UNSIGNED_SHORT, drawOrder);
    glDisableVertexAttribArray(positionHandle);
    glDisableVertexAttribArray(texCoordHandle);
    checkGlError("disable vertex arrays");
    eglSwapBuffers(eglDisplay, surface);
}

void onSoftDraw() {
    //direct draw to buffer
    if(currentScreenBuffer == nullptr) {
        return;
    }
    byte *screenBuffer = (byte *) currentScreenBuffer;
    ANativeWindow_Buffer mNativeWindowBuffer;
    ANativeWindow_lock(mANativeWindow, &mNativeWindowBuffer, nullptr);
    int *dstBuffer = static_cast<int *>(mNativeWindowBuffer.bits);
    int dstHeight = mNativeWindowBuffer.height;
    int dstWidth = mNativeWindowBuffer.height;
    int offset = (mNativeWindowBuffer.stride - mNativeWindowBuffer.height) / 2;
    if(!graphicRunning) {
        return;
    }
    __memset_aarch64(dstBuffer, 0,
                     mNativeWindowBuffer.stride * mNativeWindowBuffer.height * sizeof(int)); //对数组清零
    float scale = (float) mNativeWindowBuffer.height / 256.0f; //计算图像宽度缩放比例
    float scaleLeft = scale - (int) scale; //求出缩放比例的小数部分
    int addArg = (scaleLeft > 0.5) ? 1 : 0;
    int x, y;
    for (int hnum = 0; hnum < dstHeight; ++hnum) //按照从左到右，从上到下的顺序进行转换
    {
        y = (int) (hnum / scale) + addArg;   //计算当前临近坐标的y值
        for (int wnum = 0; wnum < dstWidth; ++wnum) {
            x = (int) (wnum / scale) + addArg; //计算当前临近坐标的x值
            dstBuffer[hnum * mNativeWindowBuffer.stride + wnum + offset] = palette[screenBuffer[
                    y * 256 + x]];
        }
    }
    ANativeWindow_unlockAndPost(mANativeWindow);
}

void setRenderCallback(byte*(*renderScreenBuffer)(byte* screenBuffer)) {
    renderBuffer = renderScreenBuffer;
}

void initEGL(ANativeWindow *window) {
    mANativeWindow = window;
    //初始化EGL
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
    surface = eglCreateWindowSurface(eglDisplay, config[0], mANativeWindow, nullptr);
    context = eglCreateContext(eglDisplay, config[0], nullptr, context_attrib_list);
    eglMakeCurrent(eglDisplay, surface, surface, context);
}

void releaseEGL() {
    eglReleaseThread();
    eglTerminate(eglDisplay);
}

bool paletteInited = false;

void initPalette() {
    if (paletteInited) {
        return;
    }
    paletteInited = true;
    for (int i = 0; i < paletteSize; i++) {
        int dd = palette[i];
        int b = (dd & 0x00FF0000) >> 16;
        int g = (dd & 0x0000FF00) >> 8;
        int r = (dd & 0x000000FF) >> 0;
        palette[i] = 0xff000000 | (r << 16) | (g << 8) | b;
    }
}

volatile bool enableFps = true;

volatile long duration = 0;

int getFps() {
    float fps = 1000.0f / (duration * 1.f);
    return (int) fps;
}

inline void calculateFps(bool &first, long &totalDuration, int &count) {
    if (enableFps) {
        if (first) {
            first = false;
        } else {
            totalDuration += getDuration();
            count++;
            if (totalDuration >= 200) {
                duration = totalDuration / count;
                count = 0;
                totalDuration = 0;
            }
        }
        startTimestamp();
    }
}

void software() {
    logd("native_graphic", "use software");
    graphicRunning = true;
    bool first = true;
    long totalDuration = 0;
    int count = 0;
    ANativeWindow_acquire(mANativeWindow);
    while (graphicRunning) {
        calculateFps(first, totalDuration, count);
        onSoftDraw();
        renderBuffer(currentScreenBuffer);
    }
    if (mANativeWindow) {
        ANativeWindow_release(mANativeWindow);
    }
}

void openGL() {
    logd("native_graphic", "use opengl");
    initEGL(mANativeWindow);
    initGL();
    onGLSurfaceChange();
    graphicRunning = true;
    bool first = true;
    long totalDuration = 0;
    int count = 0;
    while (graphicRunning) {
        calculateFps(first, totalDuration, count);
        onGLDraw();
        renderBuffer(currentScreenBuffer);
    }
    releaseEGL();
}

void vulkan() {
    logd("native_graphic", "use vulkan");
    loge("native_graphic", "current version not support vulkan");
    exit(-1);
}

/**
 * 控制gl线程运行
 * 每个v-sync判断一次
 */
volatile bool graphicRunning = true;

const static byte SOFTWARE = 0, OPEN_GL = 1, VULKAN = 2;

volatile byte renderMode = OPEN_GL;

//定义线程函数
void *gl_thread(void *arg) {
    window_height = ANativeWindow_getHeight(mANativeWindow);
    window_width = ANativeWindow_getWidth(mANativeWindow);
    ANativeWindow_setBuffersGeometry(mANativeWindow,
                                     window_width,
                                     window_height,
                                     WINDOW_FORMAT_RGBA_8888);
    if (renderMode == SOFTWARE) {
        software();
    } else if (renderMode == OPEN_GL) {
        openGL();
    } else if (renderMode == VULKAN) {
        vulkan();
    }
    return nullptr;
}

void initGraphic(ANativeWindow *window) {
    mANativeWindow = window;
    pthread_t id;
    initPalette();
    currentScreenBuffer = (byte *) malloc(sizeof(char) * (256 * 256));
    pthread_create(&id, nullptr, gl_thread, mANativeWindow);
}

void releaseGraphic() {
    logd("native_gl", "releaseGraphic");
    graphicRunning = false;
    free(config);
    free(currentScreenBuffer);
}
