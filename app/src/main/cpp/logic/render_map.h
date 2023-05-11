//
// Created by parkyu on 2023/5/11.
//

#ifndef METALMAX_RENDER_MAP_H
#define METALMAX_RENDER_MAP_H
#include "logic.h"
#include "render.h"

class MapRender : public BaseRender {
private:
    int posX, posY;
    int mapId = 0;
public:
    void updateMap(int newMapId, int x, int y);

    byte * render(byte *screenBuffer);

    bool processKey(byte directKey, byte functionKey);

    ~MapRender();
};

#endif //METALMAX_RENDER_MAP_H
