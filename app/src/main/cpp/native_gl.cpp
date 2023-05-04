//
// Created by youngpark on 2023/4/23.
//

#include "native_gl.h"
#include "opt/matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>
#include <__threading_support>

#define WIDTH 256
#define HEIGHT_PAL 240
#define HEIGHT_NTSC 224

#include "maps/map.h"
#include "charsets.h"
#include "sound/mm_sound.h"
#include "opt/mem_opt.h"

void * currentBuffer;

int palette1[256]={
        -8795632,-16777216,-93432,-77426,-1,-10581506,-15385346,-13798656,-10987432,-6184543,-5618688,-14703106,-16755730,-3003392, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -3289651,
        -3289651, -3289651, -9145228,
        -7595996, -5767168, -6553532,
        -9174900, -15728472, -16777052,
        -16775044, -16765888, -16759808,
        -16756736, -15451136, -10732520,
        -16777216, -16777216, -16777216,
        -4408132, -1282048, -1296352,
        -1048448, -4456260, -11009820,
        -16766760, -15971128, -16748408,
        -16739328, -16734208, -13070336,
        -7831552, -16777216, -16777216,
        -16777216, -197380, -213956,
        -224164, -227124, -231180,
        -4950788, -10455812, -13068036,
        -12796688, -15675264, -12002228,
        -6752168, -2562048, -8882056,
        -16777216, -16777216, -197380,
        -203608, -207676, -210732,
        -211716, -2571012, -5194500,
        -5711620, -6232836, -6226720,
        -4394840, -3343184, -983908,
        -3881788, -16777216, -16777216,
        -11053225, -9891301, -8519680,
        -9109453, -11075479, -15990658,
        -16777093, -16775587, -16768720,
        -16764160, -16761856, -15782656,
        -12243694, -16777216, -16777216,
        -16777216, -7500403, -5155840,
        -5166568, -4980640, -7536499,
        -12451669, -16769374, -16172650,
        -16755610, -16748800, -16744960,
        -13997056, -10067968, -16777216,
        -16777216, -16777216, -4342339,
        -4354771, -4362427, -4364647,
        -4367689, -7907395, -12036163,
        -13995331, -13791820, -15950752,
        -13195975, -9258430, -6115840,
        -10855846, -16777216, -16777216,
        -4342339, -4347010, -4350061,
        -4352353, -4353091, -6122563,
        -8090179, -8478019, -8868931,
        -8864344, -7490434, -6701692,
        -4932235, -7105645, -16777216,
        -16777216};

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
    __memmove_aarch64(quadCoords, tempQuadCoords, 12 * sizeof (float));
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
    __memmove_aarch64(textureCoords, tempTextureCoords, 8 * sizeof (float));
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

void checkGlError(const char *glOperation) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, "GLView", "%s: glError :%ud", glOperation, error);
    }
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
        int dd = palette1[i];
        int b = (dd & 0x00FF0000) >> 16;
        int g = (dd & 0x0000FF00) >> 8;
        int r = (dd & 0x000000FF) >> 0;
        palette1[i] = 0xff000000 | (r << 16) | (g << 8) | b;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, paletteSize, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                palette1);
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
unsigned char *convertedMap;

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
    refreshCurrentMap(0);
}

void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    program = loadProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    initTextures();
    initRenderBuffer();
    currentBuffer = colorBlockBuffer;
}

void onGLSurfaceChange(int width, int height) {
    orthoM(projMatrix, 0, -width / 2.0f, +width / 2.0f, -height / 2.0f,
           +height / 2.0f, -2.0f, 2.0f);
    glViewport(0, 0, width, height);
    initQuadCoordinates(height, height);
    glUseProgram(program);
    positionHandle = glGetAttribLocation(program, "a_position");
    textureHandle = glGetUniformLocation(program, "s_texture");
    paletteHandle = glGetUniformLocation(program, "s_palette");
    texCoordHandle = glGetAttribLocation(program, "a_texCoord");
}

void changeBuffer() {
    currentBuffer = getImage(0, 0, nullptr);
}

pthread_mutex_t onMutex;

void drawText(const char *string, int x, int y) {
    int len = strlen(string);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x*16, y*16, 8 *len, 8, GL_ALPHA,
                    GL_UNSIGNED_BYTE, getStringImg(string));
}

void onGLDraw() {
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
                    GL_UNSIGNED_BYTE, currentBuffer);
    drawText("PARK_671 TEST VERSION", 2,2);
    drawText("GRAPHIC=OPENGL ES 2.0",2,3);
    drawText("SOUND=OPENSL ES 1.0",2,4);
    pthread_mutex_unlock(&onMutex);
    checkGlError("emu render");
    glDrawElements(GL_TRIANGLES, 6,
                   GL_UNSIGNED_SHORT, drawOrder);
    glDisableVertexAttribArray(positionHandle);
    glDisableVertexAttribArray(texCoordHandle);
    checkGlError("disable vertex arrays");
}

int posX = 0, posY = 0;
bool debugColorModeEnable = false;

bool isColorDebugMode() {
    return debugColorModeEnable;
}

void debugColorMode() {
    memset(currentBuffer, posX, 256*256);
    __android_log_print(ANDROID_LOG_ERROR, "GLView", "color:%d", posX);
}

void refreshImg() {
    if (debugColorModeEnable) {
        debugColorMode();
        return;
    }
    pthread_mutex_lock(&onMutex);
    currentBuffer = getImage(posX, posY, (unsigned char *)currentBuffer);
    pthread_mutex_unlock(&onMutex);
}

void onRight() {
    posY++;
    refreshImg();
}

void onLeft() {
    posY--;
    refreshImg();
}

void onUp() {
    posX--;
    refreshImg();
}

void onDown() {
    posX++;
    refreshImg();
}

int mapId = 0;

void changeMap() {
    mapId++;
    if (mapId >= 3) {
        mapId = 0;
    }
    posX = 0;
    posY = 0;
    refreshCurrentMap(mapId);
    refreshImg();
}