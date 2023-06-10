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
    volatile int entranceAnimation = 0;
    volatile bool isOriginPalette = true;
    bool checkEntrance(Character *player, int targetX, int targetY);
    bool checkOutOfMap(Character *player, int targetX, int targetY);
    void fadeOut() const;
    void fadeIn() const;
    void refreshMusic() const;
    void triggerMonster() const;
    void triggerDoor(Character *player) const;
    void renderDoor(byte *screenBuffer) const;
    void renderSprite(byte *screenBuffer) const;
    void renderWater(byte *screenBuffer) const;
    void resetPalette() const;
    void renderPlayers(byte *screenBuffer, Character *player) const;
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
