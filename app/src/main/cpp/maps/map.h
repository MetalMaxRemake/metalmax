//
// Created by youngpark on 2023/4/24.
//

#ifndef METALMAX_MAP_H
#define METALMAX_MAP_H

unsigned char *renderMap(int x, int y, unsigned char *result);
void refreshCurrentMap(int mapId);
int getCurrentMapId();

#endif //METALMAX_MAP_H
