#ifndef _LIBNSF_H_
#define _LIBNSF_H_
#include "../player.h"
#include "nsfconfig.h"
#include "nsf.h"

#include "../../devices/CPU/nes_cpu.h"
#include "../../devices/Memory/nes_bank.h"
#include "../../devices/Memory/nes_mem.h"
#include "../../devices/Memory/nsf2_vectors.h"
#include "../../devices/Sound/nes_apu.h"
#include "../../devices/Sound/nes_dmc.h"
#include "../../devices/Audio/mixer.h"
#include "../../devices/Audio/fader.h"
#include "../../devices/Audio/amplifier.h"
#include "../../devices/Audio/rconv.h"
#include "../../devices/Misc/nsf2_irq.h"

namespace xgm
{

  class NSFPlayer : public PlayerMSP
  {
  protected:
    NSFPlayerConfig *config;

    double rate;
    int nch; // number of channels
    int song;

    INT32 last_out;
    int silent_length;

    double cpu_clock_rest;
    double apu_clock_rest;

    int time_in_ms;             // ���t��������(ms)
    bool playtime_detected;     // ���t���Ԃ����o���ꂽ��true
    bool infinite;               // never fade out

    void Reload ();
    void DetectLoop ();
    void DetectSilent ();
    void CheckTerminal ();

  public:
    void UpdateInfo();

    Bus apu_bus;
    Layer stack;
    Layer layer;
    Mixer mixer;
    Fader fader;

    NES_CPU cpu;
    NES_MEM mem;
    NES_BANK bank;
    NSF2_Vectors nsf2_vectors;
    NSF2_IRQ nsf2_irq;

    ISoundChip *sc[NES_DEVICE_MAX];      // �T�E���h�`�b�v�̃C���X�^���X
    Amplifier amp[NES_DEVICE_MAX];       // �A���v
    RateConverter rconv;

    // �g���b�N�ԍ��̗�
    enum {
      APU1_TRK0=0, APU1_TRK1, 
      APU2_TRK0, APU2_TRK1, APU2_TRK2,
      NES_TRACK_MAX
    };
    InfoBuffer infobuf[NES_TRACK_MAX];   // �e�g���b�N�̏���ۑ�
    
    int total_render; // ����܂łɐ��������g�`�̃o�C�g��
    int frame_render; // �P�t���[�����̃o�C�g��
    int frame_in_ms;  // �P�t���[���̒���(ms)

    // �e�T�E���h�`�b�v�̃G�C���A�X�Q��
    NES_APU *apu;
    NES_DMC *dmc;

  public:
    NSF *nsf;
    NSFPlayer ();
    ~NSFPlayer ();

    /** �R���t�B�O���̃Z�b�g */
    virtual void SetConfig(PlayerConfig *pc) ;

    /** �f�[�^�����[�h���� */
    virtual bool Load (SoundData * sdat);

    /** �Đ����g����ݒ肷�� */
    virtual void SetPlayFreq (double);

    /**
     * Number of channels to output.
     */
    virtual void SetChannels(int);

    /** ���Z�b�g����D�O�̉��t�Ńf�[�^�̎��ȏ����������������Ă��Ă��C�����Ȃ��D */
    virtual void Reset ();

    /** ���݉��t���̋Ȕԍ���Ԃ� */
    virtual int GetSong ();

    /** �t�F�[�h�A�E�g���J�n���� */
    virtual void FadeOut (int fade_in_ms);

    /** ���t����Ȕԍ���ݒ肷�� */
    virtual bool SetSong (int s);
    virtual bool PrevSong (int s);
    virtual bool NextSong (int s);

    /** �����_�����O���s�� */
    virtual UINT32 Render (INT16 * b, UINT32 length);

    /** �����_�����O���X�L�b�v���� */
    virtual UINT32 Skip (UINT32 length);

    /** �Ȗ����擾���� */
    virtual const char *GetTitleString ();

    /** ���t���Ԃ��擾���� */
    virtual int GetLength ();

    /** ���t���Ԃ��������o���ꂽ���ǂ������`�F�b�N���� */
    virtual bool IsDetected ();

    /** ���t����~�������ǂ������`�F�b�N���� */
    virtual bool IsStopped ();

    /** ���݂�NES�������󋵂𕶎���Ƃ��Ċl������ */
    virtual void GetMemoryString (char *buf);   // Memory Dump as String

    /** ���݂�NES�������󋵂��l������ */
    virtual void GetMemory (UINT8 * buf);       // Memory Dump

    /** �R���t�B�O���[�V�����̍X�V���ʒm���󂯎��R�[���o�b�N */
    virtual void Notify (int id);

    /** Notify for panning */
    virtual void NotifyPan (int id);

    /** time_in_ms���_�ł̃f�o�C�X�����擾���� */
    virtual IDeviceInfo *GetInfo(int time_in_ms, int device_id);

    /** Whether to use PAL/NTSC/Dendy based on flags and REGION config */
    virtual int GetRegion(UINT8 flags, int pref);

    enum {
        REGION_NTSC = 0,
        REGION_PAL,
        REGION_DENDY
    };

    /** Refresh infinite playback setting from PLAY_ADVANCE config */
    virtual void UpdateInfinite();
  };

}// namespace

#endif
