// Generated module-local resource dispatch.
#include "../data/game_data.h"
#include "../data/resource_decode.h"
#include <cstring>
#include <algorithm>
namespace {
#include "source_data/tank_editor_022010.h"
#include "source_data/tank_editor_0220d4.h"
#include "source_data/tank_editor_028142.h"
#include "source_data/tank_editor_028158.h"
#include "source_data/tank_editor_038e5c.h"

}
namespace metalmax::game_data {
    const uint8_t *tank_editor_source(uint32_t file, unsigned length) {
        if (file >= 139280 && file - 139280 <= 108 && length <= 108 - (file - 139280))
            return k_source_tank_editor_022010 + file - 139280;
        if (file >= 139476 && file - 139476 <= 20 && length <= 20 - (file - 139476))
            return k_source_tank_editor_0220_d_4 + file - 139476;
        if (file >= 164162 && file - 164162 <= 16 && length <= 16 - (file - 164162))
            return k_source_tank_editor_028142 + file - 164162;
        if (file >= 164184 && file - 164184 <= 16 && length <= 16 - (file - 164184))
            return k_source_tank_editor_028158 + file - 164184;
        if (file >= 233052 && file - 233052 <= 27 && length <= 27 - (file - 233052))
            return k_source_tank_editor_038_e_5_c + file - 233052;
        return nullptr;
    }
}
