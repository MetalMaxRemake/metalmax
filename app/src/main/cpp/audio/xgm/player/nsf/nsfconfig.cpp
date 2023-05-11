#include <stdio.h>
#include <string.h>
#include "nsfconfig.h"
#include "nsfplay.h"

#include "../../devices/Sound/nes_apu.h"
#include "../../devices/Sound/nes_dmc.h"

using namespace xgm;

const char *NSFPlayerConfig::dname[NES_DEVICE_MAX] =
  { "APU1", "APU2", };

const char *NSFPlayerConfig::channel_name[NES_CHANNEL_MAX] =
  {
      "SQR0", "SQR1", "TRI", "NOISE", "DMC",
  };

int NSFPlayerConfig::channel_device[NES_CHANNEL_MAX] =
  {
      APU, APU, DMC, DMC, DMC,
  };

int NSFPlayerConfig::channel_device_index[NES_CHANNEL_MAX] =
  {
      0, 1, 0, 1, 2,
  };

int NSFPlayerConfig::channel_track[NES_CHANNEL_MAX] =
  {
    NSFPlayer::APU1_TRK0, NSFPlayer::APU1_TRK1, NSFPlayer::APU2_TRK0,
    NSFPlayer::APU2_TRK1, NSFPlayer::APU2_TRK2
  };

static const char* DEFAULT_CHANNEL_COL[NES_CHANNEL_MAX] = {
  "FF0000", "FF0000", //APU1
  "00FF00", "00FF00", "000000", //APU2
  };
static const char* DEFAULT_5B_ENVELOPE_COL = "0000FF";
static const char* DEFAULT_5B_NOISE_COL = "000000";

NSFPlayerConfig::NSFPlayerConfig () : PlayerConfig ()
{
  int i, j;

  CreateValue("RATE", 48000);
  CreateValue("NCH",  2);
  CreateValue("BPS",  16);
  CreateValue("MASK", 0);
  CreateValue("PLAY_TIME", 60*5*1000);
  CreateValue("FADE_TIME", 5*1000);
  CreateValue("STOP_SEC", 3);
  CreateValue("LOOP_NUM", 2);
  CreateValue("AUTO_STOP", 1);
  CreateValue("AUTO_DETECT", 0);
  CreateValue("DETECT_TIME", 30*1000);
  CreateValue("DETECT_INT", 5000);
  CreateValue("LPF", 112);
  CreateValue("HPF", 164);
  CreateValue("TITLE_FORMAT", "%L (%n/%e) %T - %A");
  CreateValue("DETECT_ALT", 0);
  CreateValue("VSYNC_ADJUST", 0);
  CreateValue("MULT_SPEED", 256); // clock speed multiplier
  CreateValue("NSFE_PLAYLIST", 1); // use NSFe playlist

  CreateValue("NTSC_BASECYCLES", 1789773);
  CreateValue("PAL_BASECYCLES", 1662607);
  CreateValue("DENDY_BASECYCLES", 1773448);
  CreateValue("IRQ_ENABLE", 0);
  CreateValue("REGION", 0);
  CreateValue("LOG_CPU", 0);
  CreateValue("LOG_CPU_FILE", "nsf_write.log");

  CreateValue("PLAY_ADVANCE", 0);
  CreateValue("FAST_SEEK", 1);
  CreateValue("QUALITY", 10);
  CreateValue("MASTER_VOLUME", 128);

  for (i = 0; i < NES_CHANNEL_MAX; ++i)
  {
      std::string str;
      char num[5];
      vcm_itoa(i, num, 10);
      str = "CHANNEL_";
      if (i < 10) str += "0";
      str += num;
      str += "_PAN";
      CreateValue(str, 128);
      str = "CHANNEL_";
      if (i < 10) str += "0";
      str += num;
      str += "_VOL";
      CreateValue(str, 128);
      str = "CHANNEL_";
      if (i < 10) str += "0";
      str += num;
      str += "_COL";
      CreateValue(str, DEFAULT_CHANNEL_COL[i]);
  }
  CreateValue("5B_ENVELOPE_COL", DEFAULT_5B_ENVELOPE_COL);
  CreateValue("5B_NOISE_COL", DEFAULT_5B_NOISE_COL);

  for (i = 0; i < NES_DEVICE_MAX; i++)
  {
    std::string str;
    str = dname[i];
    CreateValue((std::string)dname[i]+"_VOLUME", 128);
    CreateValue((std::string)dname[i]+"_MUTE", 0);

    static const int DEVICE_OPTION_MAX[NES_DEVICE_MAX] =
    {
        NES_APU::OPT_END,
        NES_DMC::OPT_END,
    };

    static const int DEFAULT_DEVICE_OPTION[NES_DEVICE_MAX][16] =
    {
        { 1, 1, 1, 0, 0 },
        { 1, 1, 1, 0, 1, 1, 1, 1, 0 },
    };

    for (j = 0; j < DEVICE_OPTION_MAX[i]; j++)
    {
      static char itoa[] = "0123456789ABCDEF";
      CreateValue((std::string)dname[i]+"_OPTION"+itoa[j], DEFAULT_DEVICE_OPTION[i][j]);
    }
  }
}

NSFPlayerConfig::~NSFPlayerConfig ()
{
}


