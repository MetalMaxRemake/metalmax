// Generated module-local resource dispatch.
#include "../data/game_data.h"
#include "../data/resource_decode.h"
#include <cstring>
#include <algorithm>
namespace {
#include "text_data/page_00.h"
#include "text_data/page_01.h"
#include "text_data/page_02.h"
#include "text_data/page_03.h"
#include "text_data/page_04.h"
#include "text_data/page_05.h"
#include "text_data/page_06.h"
#include "text_data/page_07.h"
#include "text_data/page_08.h"
#include "text_data/page_09.h"
#include "text_data/page_0a.h"
#include "text_data/page_0b.h"
#include "text_data/page_0c.h"
#include "text_data/page_0d.h"
#include "text_data/page_0e.h"
#include "text_data/page_0f.h"
#include "text_data/page_10.h"
#include "text_data/page_11.h"
#include "text_data/page_12.h"
#include "text_data/page_13.h"
#include "text_data/page_14.h"
#include "text_data/page_15.h"
}
namespace metalmax::game_data {
const char *text(int page,int row){const uint8_t *p=nullptr;switch(page){case 0:p=k_text_page_0;break;case 1:p=k_text_page_1;break;case 2:p=k_text_page_2;break;case 3:p=k_text_page_3;break;case 4:p=k_text_page_4;break;case 5:p=k_text_page_5;break;case 6:p=k_text_page_6;break;case 7:p=k_text_page_7;break;case 8:p=k_text_page_8;break;case 9:p=k_text_page_9;break;case 10:p=k_text_page_10;break;case 11:p=k_text_page_11;break;case 12:p=k_text_page_12;break;case 13:p=k_text_page_13;break;case 14:p=k_text_page_14;break;case 15:p=k_text_page_15;break;case 16:p=k_text_page_16;break;case 17:p=k_text_page_17;break;case 18:p=k_text_page_18;break;case 19:p=k_text_page_19;break;case 20:p=k_text_page_20;break;case 21:p=k_text_page_21;break; default:return "";}unsigned count=resource_decode::u16(p);if(row<0||unsigned(row)>=count)return "";return reinterpret_cast<const char*>(p+2+count*2+resource_decode::u16(p+2+row*2));}
}
