#ifndef metalmax_resource_decode_h
#define metalmax_resource_decode_h
#include <stdint.h>
// Common read operations only. Resources remain in their module-local headers.
namespace metalmax::resource_decode {
inline uint16_t u16(const uint8_t *p) { return uint16_t(p[0])|(uint16_t(p[1])<<8); }
inline uint32_t u24(const uint8_t *p) { return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16); }
inline uint8_t attribute(const uint8_t *p,unsigned cell) {
    if(p[0]==255)return p[1+cell];
    unsigned bits=p[0],n=p[1]+1,bit=cell*bits;const uint8_t *indices=p+2+n;
    unsigned v=bits?indices[bit/8]:0;
    if(bit%8+bits>8)v|=unsigned(indices[bit/8+1])<<8;
    return p[2+((v>>(bit%8))&((1u<<bits)-1))];
}
inline uint8_t image_pixel(const uint8_t *image,unsigned x,unsigned y) {
    unsigned w=u16(image),h=u16(image+2),n=u16(image+4);
    unsigned columns=(w+7)/8,refs=columns*((h+7)/8);
    unsigned id=u16(image+6+(y/8*columns+x/8)*2);
    const uint8_t *p=image+6+refs*2+n*3+u24(image+6+refs*2+id*3);
    unsigned bits=p[0],count=p[1]+1,cell=(y%8)*8+x%8;
    unsigned index=bits?(p[2+count+cell*bits/8]>>((cell*bits)%8))&((1u<<bits)-1):0;
    return p[2+index];
}
}
namespace resource_decode = metalmax::resource_decode;
#endif
