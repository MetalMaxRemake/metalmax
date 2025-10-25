#ifndef _SOUNDDATA_H_
#define _SOUNDDATA_H_

#include "../xtypes.h"
#include "../debugout.h"
#include <stddef.h>

namespace xgm
{
  /**
   * �V�[�P���X�^���t�f�[�^
   */
  class SoundData
  {
  public:
    SoundData(){};
    virtual ~SoundData(){};
    /**
     * �o�C�i���C���[�W����̓ǂݍ���
     *
     * @param image �o�C�i���C���[�W�ւ̃|�C���^
     * @param size  �C���[�W�̃T�C�Y
     * @return ������ true ���s�� false
     */
    virtual bool Load(UINT8 *data, UINT32 size){ return false; }

    /**
     * �t�@�C������̓ǂݍ���
     * @param fn �t�@�C�����i�܂��̓t�@�C����������������j�ւ̃|�C���^
     */
    virtual bool LoadFile(const char *fn){ return false; }

    /**
     * �^�C�g���̎擾
     * @return �^�C�g�����
     */
    virtual const char *GetTitleString(const char *format=nullptr, int song=-1){ return ""; }

    /**
     * �^�C�g���̐ݒ�
     * @param title �V�����^�C�g���ւ̃|�C���^(255�����܂�)
     */
    virtual void SetTitleString(char *title){}

    /**
     * ���t����(ms)�̎擾
     */
    virtual int GetLength(){ return 3*60*1000; }

    /**
     * ���t����(ms)�̐ݒ�
     */
    virtual void SetLength(int time_in_ms){}
  };

  /**
   * �����ȓ��艉�t�f�[�^
   */
  class SoundDataMSP : public SoundData
  {
  public:
    bool enable_multi_tracks;
    SoundDataMSP() : enable_multi_tracks(false){}
    virtual ~SoundDataMSP(){}
    virtual int GetSong()=0;
    virtual void SetSong(int)=0;
    virtual int GetSongNum()=0;
  };
}

#endif
