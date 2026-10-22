// Generated module-local resource dispatch.
#include "../data/game_data.h"
#include "../data/resource_decode.h"
#include <cstring>
#include <algorithm>
namespace {
#include "item_data/value_table.h"
}
namespace metalmax::game_data {uint32_t value(unsigned i){return i<256?resource_decode::u24(k_item_values+i*3):0;}}
