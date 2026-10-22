// Generated module-local resource dispatch.
#include "../data/game_data.h"
#include "../data/resource_decode.h"
#include <cstring>
#include <algorithm>
namespace {
#include "source_data/elevator_editor_020150.h"
#include "source_data/elevator_editor_029226.h"
}
namespace metalmax::game_data {const uint8_t *elevator_editor_source(uint32_t file,unsigned length){if(file>=131408 && file-131408<=5 && length<=5-(file-131408))return k_source_elevator_editor_020150+file-131408;if(file>=168486 && file-168486<=10 && length<=10-(file-168486))return k_source_elevator_editor_029226+file-168486;return nullptr;}}
