#include <assert.h>
#include <cstring>
#include "nes_cpu.h"
#include "../Memory/nes_mem.h"
#include "../Misc/nsf2_irq.h"
#include "../../../../opt/mem_opt.h"

namespace xgm
{

inline void exec(K6502_Context& context, xgm::IDevice* bus)
{
    K6502_Exec(&context);
}

// bits of fixed point for timing
// 16 causes overflow at low update rate values (~27 Hz)
// 15 should be sufficient for any NSF (~13.6 Hz), since the format only allows down to ~15.25 Hz
// 14 used here just for one extra bit of safety
const int FRAME_FIXED = 14;

NES_CPU::NES_CPU (double clock)
{
  nes_basecycles = clock;
  bus = NULL;
  nes_mem = NULL;
  irqs = 0;
  enable_irq = true;
  enable_nmi = false;
  nsf2_bits = 0;
  nsf2_irq = NULL;
}

NES_CPU::~NES_CPU ()
{
}

void writeByte (void *__THIS, UINT32 adr, UINT32 val)
{
  static_cast < NES_CPU * >(__THIS)->Write (adr, val);
}

UINT32 readByte (void *__THIS, UINT32 adr)
{
  UINT32 val = 0;
  static_cast < NES_CPU * >(__THIS)->Read (adr, val);
  return val;
}

void NES_CPU::run_from (UINT32 address)
{
	#if TRACE
		DEBUG_OUT("run_from($%04X)\n",address);
	#endif
	breaked = false;
	context.PC = PLAYER_RESERVED; // JSR, followed by infinite loop ("breaked")
	breakpoint = context.PC+3;
	assert (nes_mem);
	nes_mem->WriteReserved (PLAYER_RESERVED+1, address & 0xff);
	nes_mem->WriteReserved (PLAYER_RESERVED+2, address>>8);
	// see PLAYER_PROGRAM in nsfplay.cpp
}

int NES_CPU::Exec (int clocks)
{
	// DPCM cycle stealing
	context.clock = stolen_cycles;
	stolen_cycles = 0;

	while ( int(context.clock) < clocks )
	{
		if (breaked)
		{
			if (extra_init)
			{
				enable_nmi = true;
				extra_init = false;
				context.A = song;
				context.X = region;
				context.Y = 0x81;
				run_from(init_addr);
			}

			if (enable_irq && !(context.P & K6502_I_FLAG) && context.iRequest)
			{
				breaked = false;
			}
		}

		Uword clock_start = context.clock;
		if (!breaked)
		{
			//DEBUG_OUT("PC: 0x%04X\n", context.PC);
			exec(context,bus);
			if (context.PC == breakpoint)
			{
				breaked = true;
			}
		}
		else 
		{
			if ( (fclocks_left_in_frame >> FRAME_FIXED) < INT64(clocks) )
			{
				if (fclocks_left_in_frame < 0)
					context.clock = 0;
				else
					context.clock = (unsigned int)(fclocks_left_in_frame >> FRAME_FIXED)+1;
			}
			else
				context.clock = clocks;
		}
		if (nsf2_irq) nsf2_irq->Clock(context.clock-clock_start);

		if (breaked && play_ready)
		{
			if (play_addr >= 0)
			{
				run_from (play_addr);
			}
			play_ready = false;
		}

		// signal next play
		if ( (fclocks_left_in_frame >> FRAME_FIXED) < context.clock)
		{
			if (play_addr >= 0)
			{
				if (nmi_play) // trigger play by NMI
				{
					if (enable_nmi)
					{
						context.iRequest |= K6502_NMI;
						breaked = false;
					}
					play_ready = false;
				}
				else
				{
					play_ready = true;
				}
			}
			fclocks_left_in_frame += fclocks_per_frame;
			//DEBUG_OUT("NMI\n");
		}
	}

	fclocks_left_in_frame -= (context.clock << FRAME_FIXED);

	return context.clock; // return actual number of clocks executed
}

void NES_CPU::SetMemory (IDevice * b)
{
  bus = b;
}

void NES_CPU::SetNESMemory (NES_MEM * b)
{
  nes_mem = b;
}

bool NES_CPU::Write (UINT32 adr, UINT32 val, UINT32 id)
{
  if (bus)
    return bus->Write (adr, val, id);
  else
    return false;
}

bool NES_CPU::Read (UINT32 adr, UINT32 & val, UINT32 id)
{
  if (bus)
  {
    bool result = bus->Read (adr, val, id);
    return result;
  }
  else
    return false;
}

void NES_CPU::Reset ()
{
  __memset_aarch64 (&context, 0, sizeof (K6502_Context));
  context.ReadByte = readByte;
  context.WriteByte = writeByte;
  context.iRequest = K6502_INIT;
  context.clock = 0;
  context.user = this;
  context.A = 0;
  context.X = 0;
  context.Y = 0;
  context.S = 0xff;
  context.PC = breakpoint = 0xffff;
  context.illegal = 0;
  breaked = false;
  irqs = 0;
  stolen_cycles = 0;
  play_ready = false;
  exec(context, bus);
}

void NES_CPU::Start (
	int init_addr_,
	int play_addr_,
	double play_rate,
	int song_,
	int region_,
	UINT8 nsf2_bits_,
	bool enable_irq_,
	NSF2_IRQ* nsf2_irq_)
{
	// approximate frame timing as an integer number of CPU clocks
	init_addr = init_addr_;
	play_addr = play_addr_;
	song = song_;
	region = region_;
	fclocks_per_frame = (INT64)((double)((1 << FRAME_FIXED) * nes_basecycles) / play_rate );
	fclocks_left_in_frame = 0;
	stolen_cycles = 0;
	play_ready = false;
	irqs = 0;
	nsf2_bits = nsf2_bits_;
	nsf2_irq = nsf2_irq_;
	enable_irq = enable_irq_;
	enable_nmi = false;

	// enable NSF2 IRQ
	if (nsf2_bits & 0x10) enable_irq = true;
	else nsf2_irq = NULL;

	// NSF2 disable PLAY
	if (nsf2_bits & 0x40)
	{
		play_addr = -1;
	}

	extra_init = false;
	nmi_play = false;
	if (nsf2_bits & 0x20)
	{
		extra_init = true;
		nmi_play = true;
	}
	context.A = song;
	context.X = region;
	context.Y = extra_init ? 0x80 : 0;
	context.P = 0x26; // VIZ

	run_from (init_addr);

	// temporarily disable PLAY and IRQ for INIT
	bool irq_temp = enable_irq;
	bool extra_init_temp = extra_init;
	int play_addr_temp = play_addr;
	enable_irq = false;
	extra_init = false;
	play_addr = -1;

	// run up to 1 second of init before starting real playback (this allows INIT to modify $4011 etc. silently)
	// note: things like DMC, Frame Counter, NSF2 IRQ, MMC Frame Counter, etc. aren't receiving cycles here
	//       but this should be OK?
	//       - Use of IRQs should really be taking place in PLAY or non-returning second INIT.
	//         IRQs are intentionally disabled here during first INIT to avoid issue with IRQs being mistaken
	//         for the audio-free INIT segment, but a hardware implementation probably wouldn't prevent this.
	//       - Timing between end of INIT and first PLAY is not guaranteed by the player and should not be relied on.
	//       - For NSFs that do not reset $4017 this leaves the envelope starting in synch with the first PLAY.
	//       - Waiting on an IRQ during the first init should hit the timeout and eventually trigger.
	//         (Could be a problem if they're trying to count cycles there?)
	int timeout = int(nes_basecycles);
	while (timeout > 0)
	{
		timeout -= Exec(1);
		if (breaked)
		{
			if (nmi_play) enable_nmi = true;
		}
	}
	enable_irq = irq_temp; // restore enable_irq
	extra_init = extra_init_temp; // restore extra_init
	play_addr = play_addr_temp; // restore PLAY

	// start of first frame
	fclocks_left_in_frame = fclocks_per_frame;
	play_ready = breaked && !extra_init;
}

unsigned int NES_CPU::GetPC() const
{
	return context.PC;
}

void NES_CPU::StealCycles(unsigned int cycles)
{
	stolen_cycles += cycles;
}

void NES_CPU::EnableNMI(bool enable)
{
	enable_nmi = enable;
}

void NES_CPU::UpdateIRQ(int device, bool irq)
{
	if (!enable_irq) return;
	if (device < 0 || device >= IRQD_COUNT) return;
	UINT32 mask = 1 << device;
	irqs &= ~mask;
	if (irq) irqs |= mask;
	if (irqs)
	{
		context.iRequest |= K6502_INT;
	}
	else
	{
		context.iRequest &= ~Uword(K6502_INT);
	}
}

} // namespace xgm
