// Generated module-local resource dispatch.
#include "../data/game_data.h"
#include "../data/resource_decode.h"
#include <cstring>
#include <algorithm>
#include "font_data/asci.h"
#include "font_data/zh_cn.h"
namespace {
#include "font_data/bank_20.h"
#include "font_data/bank_21.h"
#include "font_data/bank_22.h"
#include "font_data/bank_23.h"
}
namespace {
const uint8_t *find(const uint8_t *p,unsigned n,uint32_t unicode){unsigned lo=0,hi=n;while(lo<hi){unsigned mid=(lo+hi)/2;if(resource_decode::u16(p+mid*20)<unicode)lo=mid+1;else hi=mid;}return lo<n&&resource_decode::u16(p+lo*20)==unicode?p+lo*20+2:nullptr;}
}
namespace metalmax::game_data {const uint8_t *glyph(uint32_t unicode){if(const uint8_t *p=find(k_font_bank_32,448,unicode))return p;if(const uint8_t *p=find(k_font_bank_33,448,unicode))return p;if(const uint8_t *p=find(k_font_bank_34,448,unicode))return p;if(const uint8_t *p=find(k_font_bank_35,327,unicode))return p; return nullptr;}}
