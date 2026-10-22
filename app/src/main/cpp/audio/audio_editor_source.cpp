// Generated module-local resource dispatch.
#include "../data/game_data.h"
#include "../data/resource_decode.h"
#include <cstring>
#include <algorithm>

namespace {

#include "source_data/audio_editor_01a010.h"
#include "source_data/audio_editor_03aef0.h"
#include "source_data/audio_editor_03aff6.h"
#include "source_data/audio_editor_03b720.h"
#include "source_data/audio_editor_07c010.h"
#include "source_data/audio_editor_07cc90.h"
#include "source_data/audio_editor_07cfd0.h"

}
namespace metalmax::game_data {
    const uint8_t *audio_editor_source(uint32_t file, unsigned length) {
        if (file >= 106512 && file - 106512 <= 24576 && length <= 24576 - (file - 106512))
            return k_source_audio_editor_01_a_010 + file - 106512;
        if (file >= 241392 && file - 241392 <= 246 && length <= 246 - (file - 241392))
            return k_source_audio_editor_03_aef_0 + file - 241392;
        if (file >= 241654 && file - 241654 <= 774 && length <= 774 - (file - 241654))
            return k_source_audio_editor_03_aff_6 + file - 241654;
        if (file >= 243488 && file - 243488 <= 2288 && length <= 2288 - (file - 243488))
            return k_source_audio_editor_03_b_720 + file - 243488;
        if (file >= 507920 && file - 507920 <= 3073 && length <= 3073 - (file - 507920))
            return k_source_audio_editor_07_c_010 + file - 507920;
        if (file >= 511120 && file - 511120 <= 641 && length <= 641 - (file - 511120))
            return k_source_audio_editor_07_cc_90 + file - 511120;
        if (file >= 511952 && file - 511952 <= 42 && length <= 42 - (file - 511952))
            return k_source_audio_editor_07_cfd_0 + file - 511952;
        return nullptr;
    }
}
