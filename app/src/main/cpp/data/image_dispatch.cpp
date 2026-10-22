#include "game_data.h"
#include "resource_decode.h"
#include "../maps/tile_bmp.h"
#include "../maps/active_tile_bmp.h"
#include "../graphic/bitmap_render.h"
#include <algorithm>
#include <cstring>
using namespace metalmax;
namespace metalmax::game_data {
const uint8_t *monster_image(unsigned id);
const uint8_t *splash_image(unsigned id);
static const uint8_t *packed_image(unsigned id) {
    if(id>=k_monster_image && id<k_splash_image)return monster_image(id-k_monster_image);
    if(id>=k_splash_image && id<k_water_image)return splash_image(id-k_splash_image);
    return nullptr;
}
void image_size(unsigned id,unsigned &w,unsigned &h) {
    w=h=0;
    if(id<1904 || (id>=k_water_image && id<k_water_image+3)){w=h=16;return;}
    if(const uint8_t *p=packed_image(id)){w=resource_decode::u16(p);h=resource_decode::u16(p+2);}
}
void decode_image(unsigned id,uint8_t *out) {
    if(id<1904){memcpy(out,bitmaps[id],256);return;}
    if(id>=k_water_image && id<k_water_image+3){memcpy(out,water[id-k_water_image],256);return;}
    const uint8_t *p=packed_image(id);if(!p)return;unsigned w=resource_decode::u16(p),h=resource_decode::u16(p+2);
    for(unsigned y=0;y<h;y++)for(unsigned x=0;x<w;x++)out[y*w+x]=resource_decode::image_pixel(p,x,y);
}
void draw_image(unsigned id,int x,int y,uint8_t *screen,int rows) {
    if(id<1904){bitmap_render::render_bitmap(bitmaps[id],16,rows>=0?std::min(16,rows):16,x,y,screen);return;}
    if(id>=k_water_image && id<k_water_image+3){bitmap_render::render_bitmap(water[id-k_water_image],16,rows>=0?std::min(16,rows):16,x,y,screen);return;}
    const uint8_t *p=packed_image(id);if(!p)return;unsigned w=resource_decode::u16(p),h=resource_decode::u16(p+2);
    if(rows>=0)h=std::min(h,unsigned(rows));
    int x0=std::max(0,-x),x1=std::min(int(w),455-x),y0=std::max(0,-y),y1=std::min(int(h),256-y);
    for(int yy=y0;yy<y1;yy++)for(int xx=x0;xx<x1;xx++)screen[(y+yy)*455+x+xx]=resource_decode::image_pixel(p,xx,yy);
}
}
