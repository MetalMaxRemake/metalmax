//
// Created by parkyu on 2023/5/13.
//

#include "render_splash.h"
#include "../opt/mem_opt.h"
#include "../graphic/bitmap_render.h"
#include "../graphic/palette_data.h"
#include "../splash/splash.h"
#include "render_map.h"
#include "../splash/logo.h"
#include "../splash/logo_2.h"
#include "../splash/set_name.h"
#include "../audio/native_sound.h"
#include "../charset/charsets.h"

static volatile int count = 0;
static volatile int splash_scene = 0;

static volatile int logo_y = 0;
static volatile int percent = 0;

void SplashRender::processKeyClick(uint8_t directKey, uint8_t functionKey) {
    if (splash_scene >= 5 && (functionKey | directKey)) {
        pop();
        MapRender *mapRender = new MapRender;
        mapRender->updateMap(0, 91 - 8, 232 - 8);
        push(mapRender);
    } else {
        splash_scene++;
        if (splash_scene == 5) {
            changeAudio(1);
        }
    }
}

/**
 * @param clk time unit:second
 */
void sceneTik(float clk) {
    if (count > 60 * clk) {
        splash_scene++;
        count = 0;
    } else {
        count++;
    }
}

void SplashRender::tikLogic() {
    if (splash_scene == 0) {
        sceneTik(2);
    } else if (splash_scene == 1) {
        percent++;
        int height = 32;
        if (percent >= height) {
            percent = 0;
            splash_scene++;
        }
    } else if (splash_scene == 2) {
        sceneTik(0.5);
    } else if (splash_scene == 3) {
        int height = 32;
        if (logo_y == 0) {
            logo_y = (global_config::k_screen_height / 2) - height / 2;
        } else if (logo_y > 52) {
            logo_y--;
        } else {
            splash_scene++;
        }
    } else if (splash_scene == 4) {
        int height = 106;
        if (percent < height) {
            percent++;
        }
    }
}

uint8_t *SplashRender::render(uint8_t *screenBuffer) {
    __memset_aarch64(screenBuffer,
                     palette::k_palette_black,
                     global_config::k_screen_width * global_config::k_screen_height);
    if (splash_scene == 0) {
        int width = 143;
        int height = 79;
        return bitmap_render::render_bitmap((uint8_t *) splash, width, height,
                                            (global_config::k_screen_width / 2) - width / 2,
                                            (global_config::k_screen_height / 2) - height / 2,
                                            screenBuffer);
    } else if (splash_scene == 1) {
        int width = 208;
        int height = 32;
        return bitmap_render::render_bitmap((uint8_t *) logo, width, percent,
                                            (global_config::k_screen_width / 2) - width / 2 + 4,
                                            (global_config::k_screen_height / 2) - height / 2,
                                            screenBuffer);
    } else if (splash_scene == 2) {
        int width = 208;
        int height = 32;
        return bitmap_render::render_bitmap((uint8_t *) logo, width, height,
                                            (global_config::k_screen_width / 2) - width / 2 + 4,
                                            (global_config::k_screen_height / 2) - height / 2,
                                            screenBuffer);
    } else if (splash_scene == 3) {
        int width = 208;
        int height = 32;
        return bitmap_render::render_bitmap((uint8_t *) logo, width, height,
                                            (global_config::k_screen_width / 2) - width / 2 + 4,
                                            logo_y, screenBuffer);
    } else if (splash_scene == 4) {
        int width = 248;
        int height = 106;
        if (percent < height) {
            screenBuffer = bitmap_render::render_bitmap((uint8_t *) logo, 208, 32,
                                                        (global_config::k_screen_width / 2) -
                                                        208 / 2 + 4, 52, screenBuffer);
        }
        renderAsciText(screenBuffer, "PUSH START", (global_config::k_screen_width / 2) - 32, 150);
        renderAsciText(screenBuffer, "1996 DATA EAST", (global_config::k_screen_width / 2) - 48, 180);
        renderAsciText(screenBuffer, "1996 CREA-TECH", (global_config::k_screen_width / 2) - 48, 190);
        renderAsciText(screenBuffer, "2023 PARK REMAKE", (global_config::k_screen_width / 2) - 56, 200);
        return bitmap_render::render_bitmap((uint8_t *) logo_2, width, percent,
                                            (global_config::k_screen_width / 2) - width / 2, 10,
                                            screenBuffer);
    } else {
        //todo
        return bitmap_render::render_bitmap((uint8_t *) set_name,
                                            236,
                                            221,
                                            0,
                                            0,
                                            screenBuffer);
    }
}

SplashRender::~SplashRender() noexcept {

}