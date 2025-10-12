//
// Created by parkyu on 2023/5/11.
//

#ifndef METALMAX_RENDER_DEBUG_H
#define METALMAX_RENDER_DEBUG_H

#include "logic.h"
#include "render.h"
#include "render_base_menu.h"

class DebugRender : public BaseMenuRender {
private:
    int selectPos = 0;
    void processSelection();
    void renderDebugMenu(uint8_t *screenBuffer);
    void renderDebugInfo(uint8_t *screenBuffer) const;
public:
    DebugRender();
    ~DebugRender();
    uint8_t * render(uint8_t *screenBuffer);
    void tikLogic();
    void processKeyClick(uint8_t directKey, uint8_t functionKey);
};

#endif //METALMAX_RENDER_DEBUG_H
