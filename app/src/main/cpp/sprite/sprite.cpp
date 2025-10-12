//
// Created by parkyu on 2023/5/9.
//

#include "sprite.h"
#include "../global.h"
#include "sprite_player.h"

uint8_t *getSpriteImage(int spriteId, int status, uint8_t *result) {
    return (uint8_t *) sprite_player[status];
}