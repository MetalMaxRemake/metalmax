//
// Created by parkyu on 2023/5/11.
//

#ifndef METALMAX_RENDER_MAP_H
#define METALMAX_RENDER_MAP_H
#include "logic.h"
#include "render.h"

class MapRender : public BaseRender {
private:
    bool showDebug = true;
    void drawCopyRight(byte *screenBuffer) const;
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
