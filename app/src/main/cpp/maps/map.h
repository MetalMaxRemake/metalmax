//
// Created by youngpark on 2023/4/24.
//

#ifndef METALMAX_MAP_H
#define METALMAX_MAP_H

#include "../global.h"

unsigned char *renderMap(int render_y, int render_x, unsigned char *screen_buffer);
unsigned short getTileIdx(int x, int y);
void refreshCurrentMap(int mapId);
void fillOneTileInScreen(int posX, int posY, int bmpIdx, uint8_t *result);

#endif //METALMAX_MAP_H
