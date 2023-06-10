//
// Created by parkyu on 2023/5/9.
//

#include "sprite.h"
#include "../global.h"
#include "sprite_player.h"

byte *getSpriteImage(int spriteId, int status, byte *result) {
    return (byte *) sprite_player[status];
}