#include <assert.h>
#include <string.h>
#include "nes_mem.h"
#include "../../../opt/mem_opt.h"

namespace xgm
{
  NES_MEM::NES_MEM ()
  {
    fds_enable = false;
  }

  NES_MEM::~NES_MEM ()
  {
  }

  void NES_MEM::Reset ()
  {
    __memset_aarch64 (image, 0, 0x800);
    //__memset_aarch64 (image + 0x6000, 0, 0x2000); // �������ĂĂ����ď��������Ă܂���B
  }

  bool NES_MEM::SetImage (UINT8 * data, UINT32 offset, UINT32 size)
  {
    __memset_aarch64 (image, 0, 0x10000);
    if( offset + size < 0x10000 )
      __memcpy_aarch64_simd (image + offset, data, size );
    else 
      __memcpy_aarch64_simd (image + offset, data, 0x10000 - offset);
    return true;
  }

  bool NES_MEM::Write (UINT32 adr, UINT32 val, UINT32 id)
  {
    if (0x0000 <= adr && adr < 0x2000)
    {
      image[adr & 0x7ff] = val;
      return true;
    }
    if (0x6000 <= adr && adr < 0x8000)
    {
      image[adr] = val;
      return true;
    }
    if (fds_enable && 0x8000 <= adr && adr < 0xe000)
    {
      image[adr] = val;
    }
    return false;
  }

  bool NES_MEM::Read (UINT32 adr, UINT32 & val, UINT32 id)
  {
    if (0x0000 <= adr && adr < 0x2000)
    {
      val = image[adr & 0x7ff];
      return true;
    }
    if (PLAYER_RESERVED <= adr && adr < (PLAYER_RESERVED + PLAYER_RESERVED_SIZE))
    {
      val = image[adr];
      return true;
    }
    if (0x6000 <= adr && adr < 0x10000)
    {
      val = image[adr];
      return true;
    }
    val = 0;
    return false;
  }

  void NES_MEM::SetFDSMode (bool t)
  {
    fds_enable = t;
  }

  void NES_MEM::SetReserved (const UINT8* data, UINT32 size)
  {
    assert(size <= PLAYER_RESERVED_SIZE);
    ::__memcpy_aarch64_simd(image + PLAYER_RESERVED, data, size);
  }

  bool NES_MEM::WriteReserved (UINT32 adr, UINT32 val)
  {
    if (PLAYER_RESERVED <= adr && adr < (PLAYER_RESERVED + PLAYER_RESERVED_SIZE))
    {
      image[adr] = val;
      return true;
    }
    return false;
  }

}                               // namespace
