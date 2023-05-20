//
// Created by parkyu on 2023/5/13.
//

#include "render_splash.h"
#include "../opt/mem_opt.h"
#include "../graphic/bitmap_render.h"
#include "../splash/splash.h"
#include "render_map.h"
#include "../splash/logo.h"
#include "../splash/logo_2.h"
#include "../splash/set_name.h"
#include "../audio/native_sound.h"
#include "../charset/charsets.h"

int count = 0;
int splash_scene = 0;

int logo_y = 0;

void SplashRender::processKeyClick(byte directKey, byte functionKey) {
    if (splash_scene >= 4 && (functionKey | directKey)) {
        pop();
        MapRender *mapRender = new MapRender;
        mapRender->updateMap(0, 224, 83);
        push(mapRender);
    } else {
        splash_scene++;
        if(splash_scene == 4) {
            changeAudio(1);
        }
    }
}

void sceneTik() {
    if (count > 60 * 2) {
        splash_scene++;
        count = 0;
    } else {
        count++;
    }
}

byte *SplashRender::render(byte *screenBuffer) {
    if (splash_scene <= 1) {
        sceneTik();
    }
    __memset_aarch64(screenBuffer, 3, 256 * 256);
    if (splash_scene == 0) {
        int width = 143;
        int height = 79;
        return renderBitmap((byte *) splash, width, height,
                            128 - width / 2, 128 - height / 2, screenBuffer);
    } else if (splash_scene == 1) {
        int width = 208;
        int height = 32;
        return renderBitmap((byte *) logo, width, height,
                            128 - width / 2 + 4, 128 - height / 2, screenBuffer);
    } else if (splash_scene == 2) {
        int width = 208;
        int height = 32;
        if (logo_y == 0) {
            logo_y = 128 - height / 2;
        } else if (logo_y > 52) {
            logo_y--;
        } else {
            splash_scene++;
        }
        return renderBitmap((byte *) logo, width, height,
                            128 - width / 2 + 4, logo_y, screenBuffer);
    } else if (splash_scene == 3) {
        int width = 248;
        int height = 106;
        renderAsciText(screenBuffer, "PUSH START", 90, 150);
        return renderBitmap((byte *) logo_2, width, height,
                            128 - width / 2, 10, screenBuffer);
    } else {
        return renderBitmap((byte *) set_name, 236, 221,
                            0, 0, screenBuffer);
    }
}

SplashRender::~SplashRender() noexcept {

}