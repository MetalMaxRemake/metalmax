typedef unsigned int Uword;				/* (0-0xFFFF) */
typedef unsigned char Ubyte;			/* unsigned 8bit integer for table */
#define RTO16(w) ((Uword)((w) & 0xFFFF))	/* Round to 16bit integer */
#define RTO8(w) ((Uword)((w) & 0xFF))		/* Round to  8bit integer */

#ifdef __cplusplus
extern "C" {
#endif

typedef Uword (*ReadHandler)(void *user, Uword adr);
typedef void (*WriteHandler)(void *user, Uword adr, Uword value);

struct K6502_Context {
    Uword A;            /* Accumulator */
    Uword P;            /* Status register */
    Uword X;            /* X register */
    Uword Y;            /* Y register */
    Uword S;            /* Stack pointer */
    Uword PC;            /* Program Counter */

    Uword iRequest;        /* interrupt request */
    Uword iMask;        /* interrupt mask */
    Uword clock;        /* (incremental)cycle counter */
    Uword lastcode;
    void *user;            /* pointer to user area */

    Uword illegal;

    ReadHandler ReadByte;
    WriteHandler WriteByte;
};

enum K6502_FLAGS {
    K6502_C_FLAG = 0x01,
    K6502_Z_FLAG = 0x02,
    K6502_I_FLAG = 0x04,
    K6502_D_FLAG = 0x08,
    K6502_B_FLAG = 0x10,
    K6502_R_FLAG = 0x20,
    K6502_V_FLAG = 0x40,
    K6502_N_FLAG = 0x80
};

enum K6502_IRQ {
    K6502_INIT = 1,
    K6502_RESET = 2,
    K6502_NMI = 4,
    K6502_BRK = 8,
    K6502_INT = 16
};

External void K6502_Exec(struct K6502_Context *pc);

#ifdef __cplusplus
}
#endif

#define BASE_OF_ZERO 0x0000

#define VEC_RESET	0xFFFC
#define VEC_NMI		0xFFFA
#define VEC_INT		0xFFFE

#define VEC_BRK		VEC_INT

static Uword __inline__ K_READ(struct K6502_Context *pc, Uword adr)
{
	return (*pc).ReadByte((*pc).user, adr);
}
static void __inline__ K_WRITE(struct K6502_Context *pc, Uword adr, Uword value)
{
	(*pc).WriteByte((*pc).user, adr, value);
}

const static Ubyte fl_table[0x200] = {
        0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,

        0x03,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,

        0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
        0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
        0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
        0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
        0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
        0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
        0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
        0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
};
#define FLAG_NZ(w)	(fl_table[RTO8(w)])
#define FLAG_NZC(w)	(fl_table[w & 0x01ff])

static void __inline__ KI_ADDCLOCK(struct K6502_Context *pc, Uword cycle)
{
    (*pc).clock += cycle;
}
static Uword __inline__ KI_READWORD(struct K6502_Context *pc, Uword adr)
{
    Uword ret = K_READ(pc, adr);
    return ret + (K_READ(pc, (Uword)(RTO16(adr + 1))) << 8);
}
static Uword __inline__ KI_READWORDZP(struct K6502_Context *pc, Uword adr)
{
    Uword ret = K_READ(pc, (Uword)(BASE_OF_ZERO + adr));
    return ret + (K_READ(pc, (Uword)(BASE_OF_ZERO + RTO8(adr + 1))) << 8);
}

static Uword __inline__ KAI_IMM(struct K6502_Context *pc)
{
    Uword ret = (*pc).PC;
    (*pc).PC = RTO16((*pc).PC + 1);
    return ret;
}
static Uword __inline__ KAI_IMM16(struct K6502_Context *pc)
{
    Uword ret = (*pc).PC;
    (*pc).PC = RTO16((*pc).PC + 2);
    return ret;
}
static Uword __inline__ KAI_ABS(struct K6502_Context *pc)
{
    return KI_READWORD(pc, KAI_IMM16(pc));
}
static Uword __inline__ KAI_ABSX(struct K6502_Context *pc)
{
    return RTO16(KAI_ABS(pc) + (*pc).X);
}
static Uword __inline__ KAI_ABSY(struct K6502_Context *pc)
{
    return RTO16(KAI_ABS(pc) + (*pc).Y);
}
static Uword __inline__ KAI_ZP(struct K6502_Context *pc)
{
    return K_READ(pc, KAI_IMM(pc));
}
static Uword __inline__ KAI_ZPX(struct K6502_Context *pc)
{
    return RTO8(KAI_ZP(pc) + (*pc).X);
}
static Uword __inline__ KAI_INDY(struct K6502_Context *pc)
{
    return RTO16(KI_READWORDZP(pc, KAI_ZP(pc)) + (*pc).Y);
}

static Uword KA_IMM(struct K6502_Context *pc)
{
    Uword ret = (*pc).PC;
    (*pc).PC = RTO16((*pc).PC + 1);
    return ret;
}
static Uword KA_IMM16(struct K6502_Context *pc)
{
    Uword ret = (*pc).PC;
    (*pc).PC = RTO16((*pc).PC + 2);
    return ret;
}
static Uword KA_ABS(struct K6502_Context *pc)
{
    return KI_READWORD(pc, KAI_IMM16(pc));
}
static Uword KA_ABSX(struct K6502_Context *pc)
{
    return RTO16(KAI_ABS(pc) + (*pc).X);
}
static Uword KA_ABSY(struct K6502_Context *pc)
{
    return RTO16(KAI_ABS(pc) + (*pc).Y);
}
static Uword KA_ZP(struct K6502_Context *pc)
{
    return BASE_OF_ZERO + K_READ(pc, KAI_IMM(pc));
}
static Uword KA_ZPX(struct K6502_Context *pc)
{
    return BASE_OF_ZERO + RTO8(KAI_ZP(pc) + (*pc).X);
}
static Uword KA_ZPY(struct K6502_Context *pc)
{
    return BASE_OF_ZERO + RTO8(KAI_ZP(pc) + (*pc).Y);
}
static Uword KA_INDX(struct K6502_Context *pc)
{
    return KI_READWORDZP(pc, KAI_ZPX(pc));
}
static Uword KA_INDY(struct K6502_Context *pc)
{
    return RTO16(KI_READWORDZP(pc, KAI_ZP(pc)) + (*pc).Y);
}

static Uword __inline__ KI_READWORDBUG(struct K6502_Context *pc, Uword adr)
{
    Uword ret = K_READ(pc, adr);
    return ret + (K_READ(pc, (Uword)((adr & 0xFF00) + RTO8(adr + 1))) << 8);
}

static Uword KA_ABSX_(struct K6502_Context *pc)
{
    if (RTO8((*pc).PC) == 0xFF) KI_ADDCLOCK(pc, 1);	/* page break */
    return KAI_ABSX(pc);
}
static Uword KA_ABSY_(struct K6502_Context *pc)
{
    if (RTO8((*pc).PC) == 0xFF) KI_ADDCLOCK(pc, 1);	/* page break */
    return KAI_ABSY(pc);
}
static Uword KA_INDY_(struct K6502_Context *pc)
{
    Uword adr = KAI_INDY(pc);
    if (RTO8(adr) == 0xFF) KI_ADDCLOCK(pc, 1);	/* page break */
    return adr;
}

static void KM_ALUADDER(struct K6502_Context *pc, Uword src)
{
    Uword w = (*pc).A + src + ((*pc).P & K6502_C_FLAG);
    (*pc).P &= ~(K6502_N_FLAG | K6502_V_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P += FLAG_NZC(w)
                  + ((((~(*pc).A ^ src) & ((*pc).A ^ w)) >> 1) & K6502_V_FLAG);
    (*pc).A = RTO8(w);
}

#if 0
static void KM_ALUADDER_S(struct K6502_Context *pc, Uword src)
{
	Uword w = (*pc).A + src + ((*pc).P & K6502_C_FLAG);
	(*pc).P &= ~(K6502_N_FLAG | K6502_V_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
	(*pc).P += FLAG_NZC(w)
		+ ((((*pc).A ^ ~src) & ((*pc).A ^ w)) & K6502_V_FLAG);
	(*pc).A = RTO8(w);
}
#endif

static void KM_ALUADDER_D(struct K6502_Context *pc, Uword src)
{
    Uword wl = ((*pc).A & 0x0F) + (src & 0x0F) + ((*pc).P & K6502_C_FLAG);
    Uword w = (*pc).A + src + ((*pc).P & K6502_C_FLAG);

    (*pc).P &= ~K6502_C_FLAG;
    if (wl > 0x9) w += 0x6;
    if (w > 0x9F)
    {
        (*pc).P += K6502_C_FLAG;
        w += 0x60;
    }
    (*pc).A = RTO8(w);
    KI_ADDCLOCK(pc, 1);
}

static void __inline__ KMI_ADC(struct K6502_Context *pc, Uword src)
{
    KM_ALUADDER(pc, src);
}
static void __inline__ KMI_ADC_D(struct K6502_Context *pc, Uword src)
{
    KM_ALUADDER_D(pc, src);
}
static void __inline__ KMI_SBC(struct K6502_Context *pc, Uword src)
{
    KM_ALUADDER(pc, src ^ 0xFF);
}
static void __inline__ KMI_SBC_D(struct K6502_Context *pc, Uword src)
{
    KM_ALUADDER_D(pc, RTO8((src ^ 0xFF) + (0x100 - 0x66)));
}

static void KM_CMP(struct K6502_Context *pc, Uword src)
{
    Uword w = (*pc).A + (src ^ 0xFF) + 1;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P += FLAG_NZC(w);
}
static void KM_CPX(struct K6502_Context *pc, Uword src)
{
    Uword w = (*pc).X + (src ^ 0xFF) + 1;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P += FLAG_NZC(w);
}
static void KM_CPY(struct K6502_Context *pc, Uword src)
{
    Uword w = (*pc).Y + (src ^ 0xFF) + 1;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P += FLAG_NZC(w);
}

static void KM_BIT(struct K6502_Context *pc, Uword src)
{
    Uword w = (*pc).A & src;
    (*pc).P &= ~(K6502_N_FLAG | K6502_V_FLAG | K6502_Z_FLAG);
    (*pc).P += (src & (K6502_N_FLAG | K6502_V_FLAG)) + (w ? 0 : K6502_Z_FLAG);
}

static void KM_AND(struct K6502_Context *pc, Uword src)
{
    (*pc).A &= src;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG);
    (*pc).P += FLAG_NZ((*pc).A);
}

static void KM_ORA(struct K6502_Context *pc, Uword src)
{
    (*pc).A |= src;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG);
    (*pc).P += FLAG_NZ((*pc).A);
}

static void KM_EOR(struct K6502_Context *pc, Uword src)
{
    (*pc).A ^= src;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG);
    (*pc).P += FLAG_NZ((*pc).A);
}

static Uword KM_DEC(struct K6502_Context *pc, Uword des)
{
    Uword w = des - 1;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG);
    (*pc).P += FLAG_NZ(w);
    return RTO8(w);
}

static Uword KM_INC(struct K6502_Context *pc, Uword des)
{
    Uword w = des + 1;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG);
    (*pc).P += FLAG_NZ(w);
    return RTO8(w);
}

static Uword KM_ASL(struct K6502_Context *pc, Uword des)
{
    Uword w = des << 1;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P += FLAG_NZ(w) + ((des >> 7)/* & K6502_C_FLAG*/);
    return RTO8(w);
}

static Uword KM_LSR(struct K6502_Context *pc, Uword des)
{
    Uword w = des >> 1;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P += FLAG_NZ(w) + (des & K6502_C_FLAG);
    return w;
}

static Uword KM_LD(struct K6502_Context *pc, Uword src)
{
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG);
    (*pc).P += FLAG_NZ(src);
    return src;
}


static Uword KM_ROL(struct K6502_Context *pc, Uword des)
{
    Uword w = (des << 1) + ((*pc).P & K6502_C_FLAG);
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P += FLAG_NZ(w) + ((des >> 7)/* & K6502_C_FLAG*/);
    return RTO8(w);
}

static Uword KM_ROR(struct K6502_Context *pc, Uword des)
{
    Uword w = (des >> 1) + (((*pc).P & K6502_C_FLAG) << 7);
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P += FLAG_NZ(w) + (des & K6502_C_FLAG);
    return RTO8(w);
}

static void KM_BRA(struct K6502_Context *pc, Uword rel)
{
    Uword oldPage = (*pc).PC & 0xFF00;
    (*pc).PC = RTO16((*pc).PC + (rel ^ 0x80) - 0x80);
    KI_ADDCLOCK(pc, (Uword)(1 + (oldPage != ((*pc).PC & 0xFF00))));
}

static void KM_PUSH(struct K6502_Context *pc, Uword src)
{
    K_WRITE(pc, (Uword)(BASE_OF_ZERO + 0x100 + (*pc).S), src);
    (*pc).S = RTO8((*pc).S - 1);
}
static Uword KM_POP(struct K6502_Context *pc)
{
    (*pc).S = RTO8((*pc).S + 1);
    return K_READ(pc, (Uword)(BASE_OF_ZERO + 0x100  + (*pc).S));
}
#define DEF_ADC(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ KMI_ADC(pc, K_READ(pc, a(pc))); } \
static void __inline__ D_Opco##i(struct K6502_Context *pc) \
{ KMI_ADC_D(pc, K_READ(pc, a(pc))); }
DEF_ADC(61,NP,KA_INDX)	/* 61 - ADC - (Indirect,X) */
DEF_ADC(65,ZP,KA_ZP)	/* 65 - ADC - Zero Page */
DEF_ADC(69,NP,KA_IMM)	/* 69 - ADC - Immediate */
DEF_ADC(6D,NP,KA_ABS)	/* 6D - ADC - Absolute */
DEF_ADC(71,NP,KA_INDY_)	/* 71 - ADC - (Indirect),Y */
DEF_ADC(75,ZP,KA_ZPX)	/* 75 - ADC - Zero Page,X */
DEF_ADC(79,NP,KA_ABSY_)	/* 79 - ADC - Absolute,Y */
DEF_ADC(7D,NP,KA_ABSX_)	/* 7D - ADC - Absolute,X */

#define DEF_AND(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ KM_AND(pc, K_READ(pc, a(pc))); }

DEF_AND(21,NP,KA_INDX)	/* 21 - AND - (Indirect,X) */
DEF_AND(25,ZP,KA_ZP)	/* 25 - AND - Zero Page */
DEF_AND(29,NP,KA_IMM)	/* 29 - AND - Immediate */
DEF_AND(2D,NP,KA_ABS)	/* 2D - AND - Absolute */
DEF_AND(31,NP,KA_INDY_)	/* 31 - AND - (Indirect),Y */
DEF_AND(35,ZP,KA_ZPX)	/* 35 - AND - Zero Page,X */
DEF_AND(39,NP,KA_ABSY_)	/* 39 - AND - Absolute,Y */
DEF_AND(3D,NP,KA_ABSX_)	/* 3D - AND - Absolute,X */

/* --- ASL ---  */
#define DEF_ASL(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
	Uword adr = a(pc); \
	K_WRITE(pc, adr, KM_ASL(pc, K_READ(pc, adr))); \
}
DEF_ASL(06,ZP,KA_ZP)	/* 06 - ASL - Zero Page */
DEF_ASL(0E,NP,KA_ABS)	/* 0E - ASL - Absolute */
DEF_ASL(16,ZP,KA_ZPX)	/* 16 - ASL - Zero Page,X */
DEF_ASL(1E,NP,KA_ABSX)	/* 1E - ASL - Absolute,X */
static void __inline__ Opcode0A(struct K6502_Context *pc)	/* 0A - ASL - Accumulator */
{ (*pc).A = KM_ASL(pc, (*pc).A); }

/* --- BIT ---  */
#define DEF_BIT(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ KM_BIT(pc, K_READ(pc, a(pc))); }
DEF_BIT(24,ZP,KA_ZP)	/* 24 - BIT - Zero Page */
DEF_BIT(2C,NP,KA_ABS)	/* 2C - BIT - Absolute */

/* --- Bcc ---  */
#define DEF_BRA(i,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
	Uword rel = K_READ(pc, KA_IMM(pc)); \
	if (a) KM_BRA(pc, rel); \
}
DEF_BRA(10,(((*pc).P & K6502_N_FLAG) == 0))	/* 10 - BPL */
DEF_BRA(30,(((*pc).P & K6502_N_FLAG) != 0))	/* 30 - BMI */
DEF_BRA(50,(((*pc).P & K6502_V_FLAG) == 0))	/* 50 - BVC */
DEF_BRA(70,(((*pc).P & K6502_V_FLAG) != 0))	/* 70 - BVS */
DEF_BRA(90,(((*pc).P & K6502_C_FLAG) == 0))	/* 90 - BCC */
DEF_BRA(B0,(((*pc).P & K6502_C_FLAG) != 0))	/* B0 - BCS */
DEF_BRA(D0,(((*pc).P & K6502_Z_FLAG) == 0))	/* D0 - BNE */
DEF_BRA(F0,(((*pc).P & K6502_Z_FLAG) != 0))	/* F0 - BEQ */

/* --- BRK --- */
static void __inline__ Opcode00(struct K6502_Context *pc)	/* 00 - BRK */
{
    (*pc).PC = RTO16((*pc).PC + 1);
    (*pc).iRequest |= K6502_BRK;
}

/* --- CLC --- */
static void __inline__ Opcode18(struct K6502_Context *pc)	/* 18 - CLC */
{ (*pc).P &= ~K6502_C_FLAG; }
/* --- CLD --- */
static void __inline__ OpcodeD8(struct K6502_Context *pc)	/* D8 - CLD */
{
    (*pc).P &= ~K6502_D_FLAG;
}
/* --- CLI --- */
static void __inline__ Opcode58(struct K6502_Context *pc)	/* 58 - CLI */
{ (*pc).P &= ~K6502_I_FLAG; }
/* --- CLV --- */
static void __inline__ OpcodeB8(struct K6502_Context *pc)	/* B8 - CLV */
{ (*pc).P &= ~K6502_V_FLAG; }

/* --- CMP --- */
#define DEF_CMP(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ KM_CMP(pc, K_READ(pc, a(pc))); }
DEF_CMP(C1,NP,KA_INDX)	/* C1 - CMP - (Indirect,X) */
DEF_CMP(C5,ZP,KA_ZP)	/* C5 - CMP - Zero Page */
DEF_CMP(C9,NP,KA_IMM)	/* C9 - CMP - Immediate */
DEF_CMP(CD,NP,KA_ABS)	/* CD - CMP - Absolute */
DEF_CMP(D1,NP,KA_INDY_)	/* D1 - CMP - (Indirect),Y */
DEF_CMP(D5,ZP,KA_ZPX)	/* D5 - CMP - Zero Page,X */
DEF_CMP(D9,NP,KA_ABSY_)	/* D9 - CMP - Absolute,Y */
DEF_CMP(DD,NP,KA_ABSX_)	/* DD - CMP - Absolute,X */

/* --- CPX --- */
#define DEF_CPX(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ KM_CPX(pc, K_READ(pc, a(pc))); }
DEF_CPX(E0,NP,KA_IMM)	/* E0 - CPX - Immediate */
DEF_CPX(E4,ZP,KA_ZP)	/* E4 - CPX - Zero Page */
DEF_CPX(EC,NP,KA_ABS)	/* EC - CPX - Absolute */

/* --- CPY --- */
#define DEF_CPY(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ KM_CPY(pc, K_READ(pc, a(pc))); }
DEF_CPY(C0,NP,KA_IMM)	/* C0 - CPY - Immediate */
DEF_CPY(C4,ZP,KA_ZP)	/* C4 - CPY - Zero Page */
DEF_CPY(CC,NP,KA_ABS)	/* CC - CPY - Absolute */

/* --- DEC ---  */
#define DEF_DEC(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
	Uword adr = a(pc); \
	K_WRITE(pc, adr, KM_DEC(pc, K_READ(pc, adr))); \
}
DEF_DEC(C6,ZP,KA_ZP)	/* C6 - DEC - Zero Page */
DEF_DEC(CE,NP,KA_ABS)	/* CE - DEC - Absolute */
DEF_DEC(D6,ZP,KA_ZPX)	/* D6 - DEC - Zero Page,X */
DEF_DEC(DE,NP,KA_ABSX)	/* DE - DEC - Absolute,X */

static void __inline__ OpcodeCA(struct K6502_Context *pc)	/* CA - DEX */
{ (*pc).X = KM_DEC(pc, (*pc).X); }
static void __inline__ Opcode88(struct K6502_Context *pc)	/* 88 - DEY */
{ (*pc).Y = KM_DEC(pc, (*pc).Y); }

#define DEF_EOR(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ KM_EOR(pc, K_READ(pc, a(pc))); }

DEF_EOR(41,NP,KA_INDX)	/* 41 - EOR - (Indirect,X) */
DEF_EOR(45,ZP,KA_ZP)	/* 45 - EOR - Zero Page */
DEF_EOR(49,NP,KA_IMM)	/* 49 - EOR - Immediate */
DEF_EOR(4D,NP,KA_ABS)	/* 4D - EOR - Absolute */
DEF_EOR(51,NP,KA_INDY_)	/* 51 - EOR - (Indirect),Y */
DEF_EOR(55,ZP,KA_ZPX)	/* 55 - EOR - Zero Page,X */
DEF_EOR(59,NP,KA_ABSY_)	/* 59 - EOR - Absolute,Y */
DEF_EOR(5D,NP,KA_ABSX_)	/* 5D - EOR - Absolute,X */

/* --- INC ---  */
#define DEF_INC(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
	Uword adr = a(pc); \
	K_WRITE(pc, adr, KM_INC(pc, K_READ(pc, adr))); \
}
DEF_INC(E6,ZP,KA_ZP)	/* E6 - INC - Zero Page */
DEF_INC(EE,NP,KA_ABS)	/* EE - INC - Absolute */
DEF_INC(F6,ZP,KA_ZPX)	/* F6 - INC - Zero Page,X */
DEF_INC(FE,NP,KA_ABSX)	/* FE - INC - Absolute,X */

static void __inline__ OpcodeE8(struct K6502_Context *pc)	/* E8 - INX */
{ (*pc).X = KM_INC(pc, (*pc).X); }
static void __inline__ OpcodeC8(struct K6502_Context *pc)	/* C8 - INY */
{ (*pc).Y = KM_INC(pc, (*pc).Y); }

/* --- JMP ---  */
#define DEF_JMP(i,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ (*pc).PC = KI_READWORD(pc, a(pc)); }

#define DEF_JMPBUG(i,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ (*pc).PC = KI_READWORDBUG(pc, a(pc)); }

DEF_JMP(4C,KA_IMM16)	/* 4C - JMP - Immediate */
DEF_JMPBUG(6C,KA_ABS)	/* 6C - JMP - Absolute */


/* --- JSR --- */
static void __inline__ Opcode20(struct K6502_Context *pc)	/* 20 - JSR */
{
    Uword adr = KA_IMM(pc);
    KM_PUSH(pc, RTO8((*pc).PC >> 8));	/* !!! PC = NEXT - 1; !!! */
    KM_PUSH(pc, RTO8((*pc).PC));
    (*pc).PC = KI_READWORD(pc, adr);
}

/* --- LDA --- */
#define DEF_LDA(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ (*pc).A = KM_LD(pc, K_READ(pc, a(pc))); }
DEF_LDA(A1,NP,KA_INDX)	/* A1 - LDA - (Indirect,X) */
DEF_LDA(A5,ZP,KA_ZP)	/* A5 - LDA - Zero Page */
DEF_LDA(A9,NP,KA_IMM)	/* A9 - LDA - Immediate */
DEF_LDA(AD,NP,KA_ABS)	/* AD - LDA - Absolute */
DEF_LDA(B1,NP,KA_INDY_)	/* B1 - LDA - (Indirect),Y */
DEF_LDA(B5,ZP,KA_ZPX)	/* B5 - LDA - Zero Page,X */
DEF_LDA(B9,NP,KA_ABSY_)	/* B9 - LDA - Absolute,Y */
DEF_LDA(BD,NP,KA_ABSX_)	/* BD - LDA - Absolute,X */

/* --- LDX ---  */
#define DEF_LDX(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ (*pc).X = KM_LD(pc, K_READ(pc, a(pc))); }
DEF_LDX(A2,NP,KA_IMM)	/* A2 - LDX - Immediate */
DEF_LDX(A6,ZP,KA_ZP)	/* A6 - LDX - Zero Page */
DEF_LDX(AE,NP,KA_ABS)	/* AE - LDX - Absolute */
DEF_LDX(B6,ZP,KA_ZPY)	/* B6 - LDX - Zero Page,Y */
DEF_LDX(BE,NP,KA_ABSY_)	/* BE - LDX - Absolute,Y */

/* --- LDY ---  */
#define DEF_LDY(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ (*pc).Y = KM_LD(pc, K_READ(pc, a(pc))); }
DEF_LDY(A0,NP,KA_IMM)	/* A0 - LDY - Immediate */
DEF_LDY(A4,ZP,KA_ZP)	/* A4 - LDY - Zero Page */
DEF_LDY(AC,NP,KA_ABS)	/* AC - LDY - Absolute */
DEF_LDY(B4,ZP,KA_ZPX)	/* B4 - LDY - Zero Page,X */
DEF_LDY(BC,NP,KA_ABSX_)	/* BC - LDY - Absolute,X */

/* --- LSR ---  */
#define DEF_LSR(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
	Uword adr = a(pc); \
	K_WRITE(pc, adr, KM_LSR(pc, K_READ(pc, adr))); \
}
DEF_LSR(46,ZP,KA_ZP)	/* 46 - LSR - Zero Page */
DEF_LSR(4E,NP,KA_ABS)	/* 4E - LSR - Absolute */
DEF_LSR(56,ZP,KA_ZPX)	/* 56 - LSR - Zero Page,X */
DEF_LSR(5E,NP,KA_ABSX)	/* 5E - LSR - Absolute,X */
static void __inline__ Opcode4A(struct K6502_Context *pc)	/* 4A - LSR - Accumulator */
{ (*pc).A = KM_LSR(pc, (*pc).A); }

/* --- NOP ---  */
static void __inline__ OpcodeEA(struct K6502_Context *pc)	/* EA - NOP */
{
}

#define DEF_ORA(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ KM_ORA(pc, K_READ(pc, a(pc))); }

DEF_ORA(01,NP,KA_INDX)	/* 01 - ORA - (Indirect,X) */
DEF_ORA(05,ZP,KA_ZP)	/* 05 - ORA - Zero Page */
DEF_ORA(09,NP,KA_IMM)	/* 09 - ORA - Immediate */
DEF_ORA(0D,NP,KA_ABS)	/* 0D - ORA - Absolute */
DEF_ORA(11,NP,KA_INDY_)	/* 11 - ORA - (Indirect),Y */
DEF_ORA(15,ZP,KA_ZPX)	/* 15 - ORA - Zero Page,X */
DEF_ORA(19,NP,KA_ABSY_)	/* 19 - ORA - Absolute,Y */
DEF_ORA(1D,NP,KA_ABSX_)	/* 1D - ORA - Absolute,X */

/* --- PHr PLr  --- */
static void __inline__ Opcode48(struct K6502_Context *pc)	/* 48 - PHA */
{ KM_PUSH(pc, (*pc).A); }
static void __inline__ Opcode08(struct K6502_Context *pc)	/* 08 - PHP */
{ KM_PUSH(pc, (Uword)(((*pc).P | K6502_B_FLAG | K6502_R_FLAG) & ~0)); }
static void __inline__ Opcode68(struct K6502_Context *pc)	/* 68 - PLA */
{ (*pc).A = KM_LD(pc, KM_POP(pc)); }
static void __inline__ Opcode28(struct K6502_Context *pc)	/* 28 - PLP */
{ (*pc).P = KM_POP(pc) & ~0; }

/* --- ROL ---  */
#define DEF_ROL(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
	Uword adr = a(pc); \
	K_WRITE(pc, adr, KM_ROL(pc, K_READ(pc, adr))); \
}
DEF_ROL(26,ZP,KA_ZP)	/* 26 - ROL - Zero Page */
DEF_ROL(2E,NP,KA_ABS)	/* 2E - ROL - Absolute */
DEF_ROL(36,ZP,KA_ZPX)	/* 36 - ROL - Zero Page,X */
DEF_ROL(3E,NP,KA_ABSX)	/* 3E - ROL - Absolute,X */
static void __inline__ Opcode2A(struct K6502_Context *pc)	/* 2A - ROL - Accumulator */
{ (*pc).A = KM_ROL(pc, (*pc).A); }

/* --- ROR ---  */
#define DEF_ROR(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
	Uword adr = a(pc); \
	K_WRITE(pc, adr, KM_ROR(pc, K_READ(pc, adr))); \
}
DEF_ROR(66,ZP,KA_ZP)	/* 66 - ROR - Zero Page */
DEF_ROR(6E,NP,KA_ABS)	/* 6E - ROR - Absolute */
DEF_ROR(76,ZP,KA_ZPX)	/* 76 - ROR - Zero Page,X */
DEF_ROR(7E,NP,KA_ABSX)	/* 7E - ROR - Absolute,X */
static void __inline__ Opcode6A(struct K6502_Context *pc)	/* 6A - ROR - Accumulator */
{ (*pc).A = KM_ROR(pc, (*pc).A); }

static void __inline__ Opcode40(struct K6502_Context *pc)	/* 40 - RTI */
{
    (*pc).P = KM_POP(pc);
    (*pc).PC  = KM_POP(pc);
    (*pc).PC += KM_POP(pc) << 8;
}
static void __inline__ Opcode60(struct K6502_Context *pc)	/* 60 - RTS */
{
    (*pc).PC  = KM_POP(pc);
    (*pc).PC += KM_POP(pc) << 8;
    (*pc).PC  = RTO16((*pc).PC + 1);
}

/* --- SBC ---  */
#define DEF_SBC(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ KMI_SBC(pc, K_READ(pc, a(pc))); } \
static void __inline__ D_Opco##i(struct K6502_Context *pc) \
{ KMI_SBC_D(pc, K_READ(pc, a(pc))); }
DEF_SBC(E1,NP,KA_INDX)	/* E1 - SBC - (Indirect,X) */
DEF_SBC(E5,ZP,KA_ZP)	/* E5 - SBC - Zero Page */
DEF_SBC(E9,NP,KA_IMM)	/* E9 - SBC - Immediate */
DEF_SBC(ED,NP,KA_ABS)	/* ED - SBC - Absolute */
DEF_SBC(F1,NP,KA_INDY_)	/* F1 - SBC - (Indirect),Y */
DEF_SBC(F5,ZP,KA_ZPX)	/* F5 - SBC - Zero Page,X */
DEF_SBC(F9,NP,KA_ABSY_)	/* F9 - SBC - Absolute,Y */
DEF_SBC(FD,NP,KA_ABSX_)	/* FD - SBC - Absolute,X */

/* --- SEC --- */
static void __inline__ Opcode38(struct K6502_Context *pc)	/* 38 - SEC */
{ (*pc).P |= K6502_C_FLAG; }
/* --- SED --- */
static void __inline__ OpcodeF8(struct K6502_Context *pc)	/* F8 - SED */
{ (*pc).P |= K6502_D_FLAG; }
/* --- SEI --- */
static void __inline__ Opcode78(struct K6502_Context *pc)	/* 78 - SEI */
{ (*pc).P |= K6502_I_FLAG; }

/* --- STA --- */
#define DEF_STA(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ K_WRITE(pc, a(pc), (*pc).A); }
DEF_STA(81,NP,KA_INDX)	/* 81 - STA - (Indirect,X) */
DEF_STA(85,ZP,KA_ZP)	/* 85 - STA - Zero Page */
DEF_STA(8D,NP,KA_ABS)	/* 8D - STA - Absolute */
DEF_STA(91,NP,KA_INDY)	/* 91 - STA - (Indirect),Y */
DEF_STA(95,ZP,KA_ZPX)	/* 95 - STA - Zero Page,X */
DEF_STA(99,NP,KA_ABSY)	/* 99 - STA - Absolute,Y */
DEF_STA(9D,NP,KA_ABSX)	/* 9D - STA - Absolute,X */

/* --- STX ---  */
#define DEF_STX(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ K_WRITE(pc, a(pc), (*pc).X); }
DEF_STX(86,ZP,KA_ZP)	/* 86 - STX - Zero Page */
DEF_STX(8E,NP,KA_ABS)	/* 8E - STX - Absolute */
DEF_STX(96,ZP,KA_ZPY)	/* 96 - STX - Zero Page,Y */

/* --- STY ---  */
#define DEF_STY(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ K_WRITE(pc, a(pc), (*pc).Y); }
DEF_STY(84,ZP,KA_ZP)	/* 84 - STY - Zero Page */
DEF_STY(8C,NP,KA_ABS)	/* 8C - STY - Absolute */
DEF_STY(94,ZP,KA_ZPX)	/* 94 - STY - Zero Page,X */

/* --- TAX ---  */
static void __inline__ OpcodeAA(struct K6502_Context *pc)	/* AA - TAX */
{ (*pc).X = KM_LD(pc, (*pc).A); }
/* --- TAY ---  */
static void __inline__ OpcodeA8(struct K6502_Context *pc)	/* A8 - TAY */
{ (*pc).Y = KM_LD(pc, (*pc).A); }
/* --- TSX ---  */
static void __inline__ OpcodeBA(struct K6502_Context *pc)	/* BA - TSX */
{ (*pc).X = KM_LD(pc, (*pc).S); }
/* --- TXA ---  */
static void __inline__ Opcode8A(struct K6502_Context *pc)	/* 8A - TXA */
{ (*pc).A = KM_LD(pc, (*pc).X); }
/* --- TXS ---  */
static void __inline__ Opcode9A(struct K6502_Context *pc)	/* 9A - TXS */
{ (*pc).S = (*pc).X; }
/* --- TYA ---  */
static void __inline__ Opcode98(struct K6502_Context *pc)	/* 98 - TYA */
{ (*pc).A = KM_LD(pc, (*pc).Y); }

/* BS - implementing all illegal opcodes */
#if ILLEGAL_OPCODES

/* --- KIL ---  */
/* halts CPU */
#define DEF_KIL(i) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
    (*pc).PC = RTO16((*pc).PC - 1); \
    (*pc).P |= K6502_I_FLAG; /* disable interrupt */ \
}
/* opcodes */
DEF_KIL(02)
DEF_KIL(12)
DEF_KIL(22)
DEF_KIL(32)
DEF_KIL(42)
DEF_KIL(52)
DEF_KIL(62)
DEF_KIL(72)
DEF_KIL(92)
DEF_KIL(B2)
DEF_KIL(D2)
DEF_KIL(F2)

/* --- NOP ---  */
/* does nothing */
#define DEF_NOP(i) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{}
/* fetches operands but does not use them, issues dummy reads (may have page boundary cycle penalty) */
#define DEF_NOP_A(i,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ a(pc); }
/* opcodes */
DEF_NOP_A(80,KAI_IMM);
DEF_NOP_A(82,KAI_IMM);
DEF_NOP_A(C2,KAI_IMM);
DEF_NOP_A(E2,KAI_IMM);
DEF_NOP_A(04,KAI_ZP);
DEF_NOP_A(14,KAI_ZPX);
DEF_NOP_A(34,KAI_ZPX);
DEF_NOP_A(44,KAI_ZP);
DEF_NOP_A(54,KAI_ZPX);
DEF_NOP_A(64,KAI_ZP);
DEF_NOP_A(74,KAI_ZPX);
DEF_NOP_A(D4,KAI_ZPX);
DEF_NOP_A(F4,KAI_ZPX);
DEF_NOP_A(89,KAI_IMM);
DEF_NOP(1A);
DEF_NOP(3A);
DEF_NOP(5A);
DEF_NOP(7A);
DEF_NOP(DA);
DEF_NOP(FA);
DEF_NOP_A(0C,KAI_ABS);
DEF_NOP_A(1C,KA_ABSX_);
DEF_NOP_A(3C,KA_ABSX_);
DEF_NOP_A(5C,KA_ABSX_);
DEF_NOP_A(7C,KA_ABSX_);
DEF_NOP_A(DC,KA_ABSX_);
DEF_NOP_A(FC,KA_ABSX_);

/* --- SLO ---  */
/* shift left, OR result */
static Uword KM_SLO(struct K6502_Context *pc, Uword src)
{
    Uword w = RTO8(src << 1);
    (*pc).A |= w;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P |= FLAG_NZ((*pc).A);
    (*pc).P |= (src >> 7) & K6502_C_FLAG;
    return w;
}
/* macro */
#define DEF_SLO(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
    Uword adr = a(pc); \
    Uword src = K_READ(pc, adr); \
    K_WRITE(pc, adr, KM_SLO(pc, src)); \
}
/* opcodes */
DEF_SLO(03,NP,KA_INDX);
DEF_SLO(13,NP,KA_INDY);
DEF_SLO(07,ZP,KA_ZP);
DEF_SLO(17,ZP,KA_ZPX);
DEF_SLO(1B,NP,KA_ABSY);
DEF_SLO(0F,NP,KA_ABS);
DEF_SLO(1F,NP,KA_ABSX);

/* --- RLA ---  */
/* rotate left, AND result */
static Uword KM_RLA(struct K6502_Context *pc, Uword src)
{
    Uword w = RTO8((src << 1) | ((*pc).P & K6502_C_FLAG));
    (*pc).A &= w;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P |= FLAG_NZ((*pc).A);
    (*pc).P |= (src >> 7) & K6502_C_FLAG;
    return w;
}
/* macro */
#define DEF_RLA(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
    Uword adr = a(pc); \
    Uword src = K_READ(pc, adr); \
    K_WRITE(pc, adr, KM_RLA(pc, src)); \
}
/* opcodes */
DEF_RLA(23,NP,KA_INDX);
DEF_RLA(33,NP,KA_INDY);
DEF_RLA(27,ZP,KA_ZP);
DEF_RLA(37,ZP,KA_ZPX);
DEF_RLA(3B,NP,KA_ABSY);
DEF_RLA(2F,NP,KA_ABS);
DEF_RLA(3F,NP,KA_ABSX);

/* --- SRE ---  */
/* shift right, EOR result */
static Uword KM_SRE(struct K6502_Context *pc, Uword src)
{
    Uword w = RTO8(src >> 1);
    (*pc).A ^= w;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P |= FLAG_NZ((*pc).A);
    (*pc).P |= src & K6502_C_FLAG;
    return w;
}
/* macro */
#define DEF_SRE(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
    Uword adr = a(pc); \
    Uword src = K_READ(pc, adr); \
    K_WRITE(pc, adr, KM_SRE(pc, src)); \
}
/* opcodes */
DEF_SRE(43,NP,KA_INDX);
DEF_SRE(53,NP,KA_INDY);
DEF_SRE(47,ZP,KA_ZP);
DEF_SRE(57,ZP,KA_ZPX);
DEF_SRE(5B,NP,KA_ABSY);
DEF_SRE(4F,NP,KA_ABS);
DEF_SRE(5F,NP,KA_ABSX);

/* --- RRA ---  */
/* rotate right, ADC result */
static Uword KM_RRA(struct K6502_Context *pc, Uword src)
{
    Uword w = RTO8((src >> 1) | (((*pc).P & K6502_C_FLAG) << 7));
    (*pc).P &= ~(K6502_C_FLAG);
    (*pc).P |= src & K6502_C_FLAG;
    KMI_ADC(pc, w);
    return w;
}
/* macro */
#define DEF_RRA(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
    Uword adr = a(pc); \
    Uword src = K_READ(pc, adr); \
    K_WRITE(pc, adr, KM_RRA(pc, src)); \
}
/* opcodes */
DEF_RRA(63,NP,KA_INDX);
DEF_RRA(73,NP,KA_INDY);
DEF_RRA(67,ZP,KA_ZP);
DEF_RRA(77,ZP,KA_ZPX);
DEF_RRA(7B,NP,KA_ABSY);
DEF_RRA(6F,NP,KA_ABS);
DEF_RRA(7F,NP,KA_ABSX);

/* --- DCP ---  */
/* decrement, CMP */
static Uword KM_DCP(struct K6502_Context *pc, Uword src)
{
    Uword w = RTO8(src - 1);
    KM_CMP(pc, w);
    return w;
}
/* macro */
#define DEF_DCP(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
    Uword adr = a(pc); \
    Uword src = K_READ(pc, adr); \
    K_WRITE(pc, adr, KM_DCP(pc, src)); \
}
/* opcodes */
DEF_DCP(C3,NP,KA_INDX);
DEF_DCP(D3,NP,KA_INDY);
DEF_DCP(C7,ZP,KA_ZP);
DEF_DCP(D7,ZP,KA_ZPX);
DEF_DCP(DB,NP,KA_ABSY);
DEF_DCP(CF,NP,KA_ABS);
DEF_DCP(DF,NP,KA_ABSX);

/* --- ISC ---  */
/* increment, SBC */
static Uword KM_ISC(struct K6502_Context *pc, Uword src)
{
    Uword w = RTO8(src + 1);
    KMI_SBC(pc, w);
    return w;
}
/* macro */
#define DEF_ISC(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
    Uword adr = a(pc); \
    Uword src = K_READ(pc, adr); \
    K_WRITE(pc, adr, KM_ISC(pc, src)); \
}
/* opcodes */
DEF_ISC(E3,NP,KA_INDX);
DEF_ISC(F3,NP,KA_INDY);
DEF_ISC(E7,ZP,KA_ZP);
DEF_ISC(F7,ZP,KA_ZPX);
DEF_ISC(FB,NP,KA_ABSY);
DEF_ISC(EF,NP,KA_ABS);
DEF_ISC(FF,NP,KA_ABSX);

/* --- LAX ---  */
/* load A and X */
static void KM_LAX(struct K6502_Context *pc, Uword src)
{
    (*pc).A = src;
    (*pc).X = src;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG);
    (*pc).P |= FLAG_NZ(src);
}
/* macro */
#define DEF_LAX(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
    Uword adr = a(pc); \
    Uword src = K_READ(pc, adr); \
    KM_LAX(pc, src); \
}
/* opcodes */
DEF_LAX(A3,NP,KA_INDX);
DEF_LAX(B3,NP,KA_INDY_);
DEF_LAX(A7,ZP,KA_ZP);
DEF_LAX(B7,ZP,KA_ZPY);
DEF_LAX(AB,NP,KA_IMM); /* this one is unstable on hardware */
DEF_LAX(AF,NP,KA_ABS);
DEF_LAX(BF,NP,KA_ABSY_);

/* --- SAX ---  */
/* store A AND X */
#define DEF_SAX(i,p,a) static void __inline__ Opcode##i(struct K6502_Context *pc) \
{ \
    K_WRITE(pc, a(pc), ((*pc).A & (*pc).X) ); \
}
/* opcodes */
DEF_SAX(83,NP,KA_INDX);
DEF_SAX(87,ZP,KA_ZP);
DEF_SAX(97,ZP,KA_ZPY);
DEF_SAX(8F,NP,KA_ABS);

/* --- AHX ---  */
/* store A AND X AND high address (somewhat unstable) */
static void __inline__ Opcode93(struct K6502_Context *pc)
{
    Uword adr = KA_ZPY(pc);
    K_WRITE(pc, adr, RTO8((*pc).A & (*pc).X & ((adr >> 8) + 1)) );
}
static void __inline__ Opcode9F(struct K6502_Context *pc)
{
    Uword adr = KA_ABSY(pc);
    K_WRITE(pc, adr, RTO8((*pc).A & (*pc).X & ((adr >> 8) + 1)) );
}

/* --- TAS --- */
/* transfer A AND X to S, store A AND X AND high address */
static void __inline__ Opcode9B(struct K6502_Context *pc)
{
    Uword adr = KA_ABSY(pc);
    (*pc).S = (*pc).A & (*pc).X;
    K_WRITE(pc, adr, RTO8((*pc).S & ((adr >> 8) + 1)) );
}

/* --- SHY --- */
/* store Y AND high address (somewhat unstable) */
static void __inline__ Opcode9C(struct K6502_Context *pc)
{
    Uword adr = KA_ABSX(pc);
    K_WRITE(pc, adr, RTO8((*pc).Y & ((adr >> 8) + 1)) );
}

/* --- SHX --- */
/* store X AND high address (somewhat unstable) */
static void __inline__ Opcode9E(struct K6502_Context *pc)
{
    Uword adr = KA_ABSY(pc);
    K_WRITE(pc, adr, RTO8((*pc).X & ((adr >> 8) + 1)) );
}

/* --- ANC --- */
/* a = A AND immediate */
static void __inline__ Opcode0B(struct K6502_Context *pc)
{
    Uword adr = KA_IMM(pc);
    (*pc).A = RTO8((*pc).A & K_READ(pc, adr));
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P |= FLAG_NZ((*pc).A);
    (*pc).P |= ((*pc).A >> 7); /* K6502_C_FLAG */
}
static void __inline__ Opcode2B(struct K6502_Context *pc)
{
    Uword adr = KA_IMM(pc);
    (*pc).A = RTO8((*pc).A & K_READ(pc, adr));
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P |= FLAG_NZ((*pc).A);
    (*pc).P |= ((*pc).A >> 7) & K6502_C_FLAG;
}

/* --- XAA --- */
/* a = X AND immediate (unstable) */
static void __inline__ Opcode8B(struct K6502_Context *pc)
{
    Uword adr = KA_IMM(pc);
    (*pc).A = RTO8((*pc).X & K_READ(pc, adr));
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG);
    (*pc).P |= FLAG_NZ((*pc).A);
}

/* --- ALR --- */
/* A AND immediate (unstable), shift right */
static void __inline__ Opcode4B(struct K6502_Context *pc)
{
    Uword adr = KA_IMM(pc);
    Uword res = RTO8((*pc).A & K_READ(pc, adr));
    (*pc).A = res >> 1;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P |= FLAG_NZ((*pc).A);
    (*pc).P |= (res & K6502_C_FLAG);
}

/* --- ARR --- */
/* A AND immediate (unstable), rotate right, weird carry */
static void __inline__ Opcode6B(struct K6502_Context *pc)
{
    Uword adr = KA_IMM(pc);
    Uword res = RTO8((*pc).A & K_READ(pc, adr));
    (*pc).A = (res >> 1) + (((*pc).P & K6502_C_FLAG) << 7);
    (*pc).P &= ~(K6502_N_FLAG | K6502_V_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P |= FLAG_NZ((*pc).A);
    (*pc).P |= (res ^ (res >> 1)) & K6502_V_FLAG;
    (*pc).P |= (res >> 7) & K6502_C_FLAG;
}

/* --- LAS --- */
/* stack AND immediate, copy to A and X */
static void __inline__ OpcodeBB(struct K6502_Context *pc)
{
    Uword adr = KA_ABSY_(pc);
    (*pc).S &= RTO8(K_READ(pc, adr));
    (*pc).A = (*pc).S;
    (*pc).X = (*pc).S;
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG);
    (*pc).P |= FLAG_NZ((*pc).A);
}

/* --- AXS --- */
/* (A & X) - immediate, result in X */
static void __inline__ OpcodeCB(struct K6502_Context *pc)
{
    Uword adr = KA_IMM(pc);
    Uword res = ((*pc).A & (*pc).X) - RTO8(K_READ(pc, adr));
    (*pc).X = RTO8(res);
    (*pc).P &= ~(K6502_N_FLAG | K6502_Z_FLAG | K6502_C_FLAG);
    (*pc).P |= FLAG_NZ((*pc).X);
    (*pc).P |= (res <= 0xFF) ? K6502_C_FLAG : 0;
}

/* --- SBC --- */
/* EB is alternate opcode for SBC E9 */
static void __inline__ OpcodeEB(struct K6502_Context *pc)
{
    OpcodeE9(pc);
}

#endif

/*

 m6502 clock cycle table

 (n)        undefined OP-code
 +n         +1 by page boundary case
 BRK(#$00)  +7 by interrupt

 BS - corrected NOP timings for undefined opcodes

*/
const static Ubyte cl_table[256] = {
/* L 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F      H */
        0 , 6 ,(2),(8),(3), 3 , 5 ,(5), 3 , 2 , 2 ,(2),(4), 4 , 6 ,(6), /* 0 */
        2 ,+5 ,(2),(8),(4), 4 , 6 ,(6), 2 ,+4 ,(2),(7),(4),+4 , 7 ,(7), /* 1 */
        6 , 6 ,(2),(8), 3 , 3 , 5 ,(5), 4 , 2 , 2 ,(2), 4 , 4 , 6 ,(6), /* 2 */
        2 ,+5 ,(2),(8),(4), 4 , 6 ,(6), 2 ,+4 ,(2),(7),(4),+4 , 7 ,(7), /* 3 */
        6 , 6 ,(2),(8),(3), 3 , 5 ,(5), 3 , 2 , 2 ,(2), 3 , 4 , 6 ,(6), /* 4 */
        2 ,+5 ,(2),(8),(4), 4 , 6 ,(6), 2 ,+4 ,(2),(7),(4),+4 , 7 ,(7), /* 5 */
        6 , 6 ,(2),(8),(3), 3 , 5 ,(5), 4 , 2 , 2 ,(2), 5 , 4 , 6 ,(6), /* 6 */
        2 ,+5 ,(2),(8),(4), 4 , 6 ,(6), 2 ,+4 ,(2),(7),(4),+4 , 7 ,(7), /* 7 */
        (2), 6 ,(2),(6), 3 , 3 , 3 ,(3), 2 ,(2), 2 ,(2), 4 , 4 , 4 ,(4), /* 8 */
        2 , 6 ,(2),(6), 4 , 4 , 4 ,(4), 2 , 5 , 2 ,(5),(5), 5 ,(5),(5), /* 9 */
        2 , 6 , 2 ,(6), 3 , 3 , 3 ,(3), 2 , 2 , 2 ,(2), 4 , 4 , 4 ,(4), /* A */
        2 ,+5 ,(2),(5), 4 , 4 , 4 ,(4), 2 ,+4 , 2 ,(4),+4 ,+4 ,+4 ,(4), /* B */
        2 , 6 ,(2),(8), 3 , 3 , 5 ,(5), 2 , 2 , 2 ,(2), 4 , 4 , 6 ,(6), /* C */
        2 ,+5 ,(2),(8),(4), 4 , 6 ,(6), 2 ,+4 ,(2),(7),(4),+4 , 7 ,(7), /* D */
        2 , 6 ,(2),(8), 3 , 3 , 5 ,(5), 2 , 2 , 2 ,(2), 4 , 4 , 6 ,(6), /* E */
        2 ,+5 ,(2),(8),(4), 4 , 6 ,(6), 2 ,+4 ,(2),(7),(4),+4 , 7 ,(7), /* F */
};

#define OP__(i) \
	case 0x##i: \
		Opcode##i(pc); \
		break;

/* BS - option to disable decimal execution for NES */
#define OP_d OP__

#define OPtd OP_d
#define OPt_ OP__

/* BS - allowing illegal opcode implementation */
#define OPxx(i) \
	case 0x##i: \
		Opcode##i(pc); \
		(*pc).illegal = 1; \
		break;

static void __inline__ K_OPEXEC(struct K6502_Context *pc)
{
    Uword opcode = (*pc).lastcode = K_READ(pc, KAI_IMM(pc));
    KI_ADDCLOCK(pc, cl_table[opcode]);
    switch(opcode)
    {
        OP__(00)	OPt_(01)	OPxx(02)	OPxx(03)	OPxx(04)	OPt_(05)	OP__(06)	OPxx(07)
        OP__(08)	OPt_(09)	OP__(0A)	OPxx(0B)	OPxx(0C)	OPt_(0D)	OP__(0E)	OPxx(0F)
        OP__(10)	OPt_(11)	OPxx(12)	OPxx(13)	OPxx(14)	OPt_(15)	OP__(16)	OPxx(17)
        OP__(18)	OPt_(19)	OPxx(1A)	OPxx(1B)	OPxx(1C)	OPt_(1D)	OP__(1E)	OPxx(1F)
        OP__(20)	OP__(21)	OPxx(22)	OPxx(23)	OP__(24)	OP__(25)	OP__(26)	OPxx(27)
        OP__(28)	OP__(29)	OP__(2A)	OPxx(2B)	OP__(2C)	OP__(2D)	OP__(2E)	OPxx(2F)
        OP__(30)	OPt_(31)	OPxx(32)	OPxx(33)	OPxx(34)	OPt_(35)	OP__(36)	OPxx(37)
        OP__(38)	OPt_(39)	OPxx(3A)	OPxx(3B)	OPxx(3C)	OPt_(3D)	OP__(3E)	OPxx(3F)
        OP__(40)	OPt_(41)	OPxx(42)	OPxx(43)	OPxx(44)	OPt_(45)	OP__(46)	OPxx(47)
        OP__(48)	OPt_(49)	OP__(4A)	OPxx(4B)	OP__(4C)	OPt_(4D)	OP__(4E)	OPxx(4F)
        OP__(50)	OPt_(51)	OPxx(52)	OPxx(53)	OPxx(54)	OPt_(55)	OP__(56)	OPxx(57)
        OP__(58)	OPt_(59)	OPxx(5A)	OPxx(5B)	OPxx(5C)	OPt_(5D)	OP__(5E)	OPxx(5F)
        OP__(60)	OPtd(61)	OPxx(62)	OPxx(63)	OPxx(64)	OPtd(65)	OP__(66)	OPxx(67)
        OP__(68)	OPtd(69)	OP__(6A)	OPxx(6B)	OP__(6C)	OPtd(6D)	OP__(6E)	OPxx(6F)
        OP__(70)	OPtd(71)	OPxx(72)	OPxx(73)	OPxx(74)	OPtd(75)	OP__(76)	OPxx(77)
        OP__(78)	OPtd(79)	OPxx(7A)	OPxx(7B)	OPxx(7C)	OPtd(7D)	OP__(7E)	OPxx(7F)

        OPxx(80)	OP__(81)	OPxx(82)	OPxx(83)	OP__(84)	OP__(85)	OP__(86)	OPxx(87)
        OP__(88)	OPxx(89)	OP__(8A)	OPxx(8B)	OP__(8C)	OP__(8D)	OP__(8E)	OPxx(8F)
        OP__(90)	OP__(91)	OPxx(92)	OPxx(93)	OP__(94)	OP__(95)	OP__(96)	OPxx(97)
        OP__(98)	OP__(99)	OP__(9A)	OPxx(9B)	OPxx(9C)	OP__(9D)	OPxx(9E)	OPxx(9F)
        OP__(A0)	OP__(A1)	OP__(A2)	OPxx(A3)	OP__(A4)	OP__(A5)	OP__(A6)	OPxx(A7)
        OP__(A8)	OP__(A9)	OP__(AA)	OPxx(AB)	OP__(AC)	OP__(AD)	OP__(AE)	OPxx(AF)
        OP__(B0)	OP__(B1)	OPxx(B2)	OPxx(B3)	OP__(B4)	OP__(B5)	OP__(B6)	OPxx(B7)
        OP__(B8)	OP__(B9)	OP__(BA)	OPxx(BB)	OP__(BC)	OP__(BD)	OP__(BE)	OPxx(BF)
        OP__(C0)	OP__(C1)	OPxx(C2)	OPxx(C3)	OP__(C4)	OP__(C5)	OP__(C6)	OPxx(C7)
        OP__(C8)	OP__(C9)	OP__(CA)	OPxx(CB)	OP__(CC)	OP__(CD)	OP__(CE)	OPxx(CF)
        OP__(D0)	OP__(D1)	OPxx(D2)	OPxx(D3)	OPxx(D4)	OP__(D5)	OP__(D6)	OPxx(D7)
        OP__(D8)	OP__(D9)	OPxx(DA)	OPxx(DB)	OPxx(DC)	OP__(DD)	OP__(DE)	OPxx(DF)
        OP__(E0)	OP_d(E1)	OPxx(E2)	OPxx(E3)	OP__(E4)	OP_d(E5)	OP__(E6)	OPxx(E7)
        OP__(E8)	OP_d(E9)	OP__(EA)	OPxx(EB)	OP__(EC)	OP_d(ED)	OP__(EE)	OPxx(EF)
        OP__(F0)	OP_d(F1)	OPxx(F2)	OPxx(F3)	OPxx(F4)	OP_d(F5)	OP__(F6)	OPxx(F7)
        OP__(F8)	OP_d(F9)	OPxx(FA)	OPxx(FB)	OPxx(FC)	OP_d(FD)	OP__(FE)	OPxx(FF)
    }
}

__inline void K6502_Exec(struct K6502_Context *pc)
{
    if ((*pc).iRequest)
    {
        if ((*pc).iRequest & K6502_INIT)
        {
            (*pc).A = 0;
            (*pc).X = 0;
            (*pc).Y = 0;
            (*pc).S = 0xFF;
            (*pc).P = K6502_Z_FLAG | K6502_R_FLAG | K6502_I_FLAG;
            (*pc).iRequest = 0;
            (*pc).iMask = ~0;
            KI_ADDCLOCK(pc, 7);
            return;
        }
        else if ((*pc).iRequest & K6502_RESET)
        {
            (*pc).A = 0;
            (*pc).X = 0;
            (*pc).Y = 0;
            (*pc).S = 0xFF;
            (*pc).P = K6502_Z_FLAG | K6502_R_FLAG | K6502_I_FLAG;
            (*pc).PC = KI_READWORD(pc, VEC_RESET);
            (*pc).iRequest = 0;
            (*pc).iMask = ~0;
        }
        else if ((*pc).iRequest & K6502_NMI)
        {
            KM_PUSH(pc, RTO8((*pc).PC >> 8));
            KM_PUSH(pc, RTO8((*pc).PC));
            KM_PUSH(pc, (Uword)((*pc).P | K6502_R_FLAG | K6502_B_FLAG));
            (*pc).P = ((*pc).P & ~ 0) | K6502_I_FLAG;	/* 6502 bug */
            (*pc).iRequest &= ~(K6502_NMI | K6502_BRK);
            (*pc).PC  =  KI_READWORD(pc, VEC_NMI);
            KI_ADDCLOCK(pc, 7);
        }
        else if ((*pc).iRequest & K6502_BRK)
        {
            KM_PUSH(pc, RTO8((*pc).PC >> 8));
            KM_PUSH(pc, RTO8((*pc).PC));
            KM_PUSH(pc, (Uword)((*pc).P | K6502_R_FLAG | K6502_B_FLAG));
            (*pc).P = ((*pc).P & ~0) | K6502_I_FLAG;	/* 6502 bug */
            (*pc).iRequest &= ~K6502_BRK;
            (*pc).PC = KI_READWORD(pc, VEC_BRK);
            KI_ADDCLOCK(pc, 7);
        }
        else if ((*pc).P & K6502_I_FLAG)
        {
            /* interrupt disabled */
        }
        else if ((*pc).iMask & (*pc).iRequest & K6502_INT)
        {
            KM_PUSH(pc, RTO8((*pc).PC >> 8));
            KM_PUSH(pc, RTO8((*pc).PC));
            KM_PUSH(pc, (Uword)(((*pc).P | K6502_R_FLAG) & ~K6502_B_FLAG));
            (*pc).P = ((*pc).P & ~0) | K6502_I_FLAG;	/* 6502 bug */
            (*pc).PC = KI_READWORD(pc, VEC_INT);
            KI_ADDCLOCK(pc, 7);
        }
    }
    K_OPEXEC(pc);
}
