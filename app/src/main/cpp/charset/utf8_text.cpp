#include "utf8_text.h"
#include "../global.h"
#include "../data/game_data.h"
#include "../graphic/palette_data.h"
namespace metalmax {
extern const unsigned char asci[96][8];

static void put(uint8_t *buffer,int x,int y,uint8_t color) {
    if (x>=0 && x<global_config::k_screen_width && y>=0 && y<global_config::k_screen_height) {
        buffer[y*global_config::k_screen_width+x]=color;
    }
}
unsigned render_utf_8_text(uint8_t *buffer,const char *text,int x,int y,int width,int height) {
    if (!text || width<12 || height<12) { return 0; }
    unsigned offset=0;int cx=0,cy=0;
    while (text[offset]) {
        unsigned char lead=static_cast<unsigned char>(text[offset]);
        if (lead=='\f') { return offset+1; }
        if (lead=='\n') { cx=0;cy+=16;offset++;continue; }
        uint32_t cp=lead;unsigned size=1;
        if ((lead&0xe0)==0xc0) { cp=lead&31;size=2; }
        else if ((lead&0xf0)==0xe0) { cp=lead&15;size=3; }
        else if ((lead&0xf8)==0xf0) { cp=lead&7;size=4; }
        bool valid=true;
        for (unsigned i=1;i<size;i++) {
            unsigned char next=static_cast<unsigned char>(text[offset+i]);
            if ((next&0xc0)!=0x80) { valid=false;break; }
            cp=(cp<<6)|(next&63);
        }
        if (!valid) { cp='?';size=1; }
        const int advance=cp<128 ? 6 : 12;
        if (cx+advance>width) { cx=0;cy+=16; }
        if (cy+12>height) { return offset; }
        const uint8_t *glyph=cp>=128 ? game_data::glyph(cp) : nullptr;
        if (glyph) {
            for (int dy=0;dy<12;dy++) {
                for (int dx=0;dx<12;dx++) {
                    uint8_t packed=glyph[(dx/4)*6+dy/2];
                    int bit=(packed>>(dy%2 ? 0 : 4))>>(3-dx%4);
                    if (bit&1) { put(buffer,x+cx+dx,y+cy+dy,palette::k_palette_white); }
                }
            }
        } else {
            unsigned index=cp>=32 && cp<128 ? cp-32 : '?' -32;
            for (int dy=0;dy<8;dy++) {
                for (int dx=0;dx<6;dx++) {
                    if (asci[index][dy]&(1<<(7-dx))) { put(buffer,x+cx+dx,y+cy+dy,palette::k_palette_white); }
                }
            }
        }
        cx+=advance;offset+=size;
    }
    return offset;
}
} // namespace metalmax
