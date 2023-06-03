//
// Created by parkyu on 2023/5/11.
//

#ifndef METALMAX_RENDER_MAP_H
#define METALMAX_RENDER_MAP_H
#include "logic.h"
#include "render.h"
#include "status/character.h"

class MapRender : public BaseRender {
private:
    bool showDebug = true;
    volatile int skipFrame = 0;
    void drawCopyRight(byte *screenBuffer) const;

    bool checkEntrance(Character *player, int targetX, int targetY);

    bool checkOutOfMap(Character *player, int targetX, int targetY);
public:
    MapRender();
    void updateMap(int newMapId, int x, int y);
    int getMapId();
    byte * render(byte *screenBuffer);
    void tikLogic();

    bool processKey(byte directKey, byte functionKey);

    void processKeyClick(byte directKey, byte functionKey);

    void onFocus();
    void onUnFocus();

    ~MapRender();
};

#endif //METALMAX_RENDER_MAP_H
