#ifndef metalmax_utf_8_text_h
#define metalmax_utf_8_text_h
#include <stdint.h>
// Returns byte count consumed; stops before an explicit page break or a full box.
namespace metalmax {
unsigned render_utf_8_text(uint8_t *buffer,const char *text,int x,int y,int width,int height);
} // namespace metalmax
#endif
