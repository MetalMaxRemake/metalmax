#include <assert.h>
#include <cstring>
#include "ram64k.h"
#include "../../../opt/mem_opt.h"

namespace xgm
{
/////////////////////////////////////////////////////////////////////////////

RAM64K::RAM64K()
{
}

RAM64K::~RAM64K()
{
}

void
RAM64K::Reset ()
{
  __memset_aarch64 (image, 0, 0x10000);
}

bool
RAM64K::SetImage (UINT8 * data, UINT32 offset, UINT32 size)
{
  if( offset + size < 0x10000 )
    __memcpy_aarch64_simd (image + offset, data, size );
  else 
    __memcpy_aarch64_simd (image + offset, data, 0x10000 - offset);
  return true;
}

bool
RAM64K::Write (UINT32 adr, UINT32 val, UINT32 id)
{
  image[adr & 0xFFFF] = val&0xFF;
  return true;
}

bool
RAM64K::Read (UINT32 adr, UINT32 & val, UINT32 id)
{
  val = image[adr & 0xFFFF];
  return true;
}

/////////////////////////////////////////////////////////////////////////////
}// namespace
