#ifndef _NSF_H_
#define _NSF_H_
#include "../soundData.h"

#define NSF_MAX_PATH 512

namespace xgm
{
  struct NSFE_Entry
  {
    const char* tlbl;
    const char* taut;
    INT32 time;
    INT32 fade;
    bool psfx;
  };
  const unsigned int NSFE_ENTRIES = 256;
  const unsigned int NSFE_MIXES = 2;
  const INT16 NSFE_MIXE_DEFAULT = 32767;

  /**
   * NSF�t�@�C���^
   */
  class NSF : public SoundDataMSP
  {
  public:
    char magic[5];
    UINT8 version;
    UINT8 songs;
    UINT8 total_songs;
    UINT8 start;
    UINT16 load_address;
    UINT16 init_address;
    UINT16 play_address;
    char filename[NSF_MAX_PATH];
    char print_title[256+64]; // margin 64 chars.
    char title_nsf[32];
    char artist_nsf[32];
    char copyright_nsf[32];
    const char* title;
    const char* artist;
    const char* copyright;
    const char* ripper; // NSFe only
    const char* text; // NSFe only
    UINT32 text_len; // NSFe only
    UINT16 speed_ntsc;
    UINT8 bankswitch[8];
    UINT16 speed_pal;
    UINT8 pal_ntsc;
    UINT8 soundchip;
    UINT8 extra[4];
    UINT8 *body;
    int bodysize;
    UINT8* nsfe_image;
    UINT8* nsfe_plst;
    int nsfe_plst_size;
    NSFE_Entry nsfe_entry[NSFE_ENTRIES];
    INT16 nsfe_mixe[NSFE_MIXES];
    UINT16 speed_dendy;
    UINT8 regn;
    int regn_pref;
    UINT8 nsf2_bits;

    /** ���ݑI�𒆂̋Ȕԍ� */
    int song;
    /** �v���C���X�g����ǂ񂾃f�[�^�Ȃ�true */
    bool playlist_mode;
    /** ���t���� */
    int time_in_ms, default_playtime;
    /** ���[�v���� */
    int loop_in_ms;
    /** �t�F�[�h���� */
    int fade_in_ms, default_fadetime;
    /** ���[�v�� */
    int loop_num, default_loopnum;
    /** ���t���Ԃ��s���̎�true�i�f�t�H���g�̉��t���Ԃ��g�p�j*/
    bool playtime_unknown;
    bool title_unknown;

      NSF ();
     ~NSF ();
    /**
     * NSF�t�@�C���̓ǂݍ���
     *
     * @param image �o�C�i���C���[�W�ւ̃|�C���^
     * @param size  �C���[�W�̃T�C�Y
     * @return ������ true ���s�� false
     */

    // loads file (playlist or NSF or NSFe)
    bool LoadFile (const char *fn);

    // loads NSF (or NSFe via LoadNSFe)
    bool Load (UINT8 * image, UINT32 size);

    // loads NSFe, or NSFe suffix for NSF2
    bool LoadNSFe(UINT8* image, UINT32 size, bool nsf2);

    // returns descriptive error of last Load (English only)
    const char* LoadError();

    void DebugOut ();
    /**
     * �^�C�g��������̎擾
     *
     * @return �^�C�g�������� (��Ȏ� - �^�C�g��)
     */
    const char *GetTitleString (const char *format=nullptr, int song=-1);
    const char *GetPlaylistString (const char *format, bool b);
    int GetLength ();
    void SetTitleString (char *);
    void SetDefaults (int playtime, int fadetime, int loopnum);
    void SetLength (int time_in_ms);
    void SetSong (int);
    int GetSong ();
    int GetSongNum();

    int GetPlayTime ();
    int GetLoopTime ();
    int GetFadeTime ();
    int GetLoopNum ();

    bool UseNSFePlaytime();
  };

}                               // namespace 
#endif
