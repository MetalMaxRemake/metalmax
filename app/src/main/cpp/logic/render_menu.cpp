//
// Created by parkyu on 2023/5/11.
//

#include "render_menu.h"
#include "../charset/charsets.h"
#include "../audio/mm_sound.h"
#include "../audio/native_sound.h"
#include "status/character.h"
#include "../computer/computer_data.h"
#include "render_computer.h"
#include "../opt/mem_opt.h"
#include "../graphic/palette_data.h"

MenuRender::MenuRender(int menuType) {

}

void MenuRender::tikLogic() {

}

inline void MenuRender::processSelection() {
    if (selectPos == 0) {

    } else if (selectPos == 1) {

    } else if (selectPos == 2) {

    } else if (selectPos == 3) {
        int x = getDefaultPlayer()->x;
        int y = getDefaultPlayer()->y;
        int direct = getDefaultPlayer()->direct;
        if (direct == direct_down) {
            y++;
        } else if (direct == direct_up) {
            y--;
        } else if (direct == direct_left) {
            x--;
        } else if (direct == direct_right) {
            x++;
        }
        bool found = false;
        int mapId = getCurrentMap();
        for (int i = 0; i < COMPUTER_COUNT; i++) {
            if (computers[i * 4 + 2] == mapId) {
                if (x == computers[i * 4 + 0]
                    && y == computers[i * 4 + 1]) {
                    found = true;
                    ComputerRender *computerRender = new ComputerRender(computers[i * 4 + 3]);
                    push(computerRender);
                    break;
                }
            }
        }
        if(!found) {
            ComputerRender *computerRender = new ComputerRender(-1);
            push(computerRender);
        }
    }
}

void MenuRender::processKeyClick(uint8_t directKey, uint8_t functionKey) {
    renderEffect(EFFECT_PUSH_BUTTON);
    if (directKey & up) {
        selectPos--;
        if (selectPos < 0) {
            selectPos = 0;
        }
    }
    if (directKey & down) {
        selectPos++;
        if (selectPos > 3) {
            selectPos = 3;
        }
    }
    if (functionKey & a) {
        processSelection();
    }
    if (functionKey & b) {
        pop();
    }
}


uint8_t *MenuRender::render(uint8_t *screenBuffer) {
    int start = 192;
    int textHeight = 8;
    int total = global_config::k_screen_width * global_config::k_screen_height;
    __memset_aarch64(screenBuffer + start * global_config::k_screen_width, palette::k_palette_black, total - start * global_config::k_screen_width);
    renderAsciText(screenBuffer, "  TALK", 10, start + textHeight * 1);
    renderAsciText(screenBuffer, "  EQUP", 10, start + textHeight * 2);
    renderAsciText(screenBuffer, "  STRE", 10, start + textHeight * 3);
    renderAsciText(screenBuffer, "  INVE", 10, start + textHeight * 4);
    renderSelectPos(screenBuffer, 13, start + (textHeight * (selectPos + 1)));
    return screenBuffer;
}