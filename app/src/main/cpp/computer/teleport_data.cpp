#include "teleport_data.h"
#include "computer_data/teleport_locations.h"

namespace metalmax::teleport_data {
    bool location(unsigned index, int &map, int &x, int &y) {
        if (index >= 16) { return false; }
        const uint8_t *p = k_teleport_locations + index * 3;
        if (p[0] >= 240) { return false; }
        map = p[0];
        x = p[1];
        y = p[2];
        return true;
    }
}
