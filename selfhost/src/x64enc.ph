// =============================================================================
// x64enc.ph - x86-64 machine instructions: opcodes of the machine IR used by
// x64.ph, and their binary encoding (legacy SSE or VEX when AVX is enabled).
// =============================================================================
import "util.ph" as u;
import "bin.ph" as bn;
import "mach.ph" as mc;

// Physical registers: general purpose 0-15, xmm 16-31.
pub const RAX: i32 = 0;
pub const RCX: i32 = 1;
pub const RDX: i32 = 2;
pub const RBX: i32 = 3;
pub const RSP: i32 = 4;
pub const RBP: i32 = 5;
pub const RSI: i32 = 6;
pub const RDI: i32 = 7;
pub const R8: i32 = 8;
pub const R9: i32 = 9;
pub const R10: i32 = 10;
pub const R11: i32 = 11;
pub const R12: i32 = 12;
pub const R13: i32 = 13;
pub const R14: i32 = 14;
pub const R15: i32 = 15;
pub const XMM0: i32 = 16;

// Machine opcodes. Operand conventions in comments: r register, m memory
// (MOp K_MEM), i immediate; sz is the operand size in bytes.
pub const MOV_RR: i32 = 1;      // r(D) <- r(U)          sz 4/8
pub const MOV_RI: i32 = 2;      // r(D) <- i             sz 4/8 (8: any 64-bit constant)
pub const LOAD: i32 = 3;        // r(D) <- m             sz 1/2/4/8, cc: 0 plain / 1 zero-extend / 2 sign-extend (to sz 4 or 8 in aux)
pub const STORE: i32 = 4;       // m <- r(U)             sz 1/2/4/8
pub const STORE_I: i32 = 5;     // m <- i                sz 1/2/4/8 (i sign-extended imm32)
pub const LEA: i32 = 6;         // r(D) <- &m
pub const ALU: i32 = 7;         // r(UD) op= r(U)|m|i    cc: /digit (0 add 1 or 4 and 5 sub 6 xor 7 cmp); sz 1/2/4/8
pub const TEST: i32 = 8;        // r(U) & r(U)|i         sz 1/2/4/8
pub const IMUL: i32 = 9;        // r(UD) *= r(U)|m
pub const IMUL_I: i32 = 10;     // r(D) <- r(U) * i
pub const SHIFT: i32 = 11;      // r(UD) shift by cl (ops[1] = rcx U) or imm; cc: /digit (0 rol 1 ror 4 shl 5 shr 7 sar)
pub const UNARY: i32 = 12;      // r(UD); cc: /digit of F7 (2 not, 3 neg)
pub const EXT: i32 = 13;        // r(D) <- r(U): cc 1 movzx8, 2 movzx16, 3 movsx8, 4 movsx16, 5 movsxd, 6 mov32 (zero-extend)
pub const CQO: i32 = 14;        // rdx(D) <- sign of rax(U)   sz 4/8
pub const DIV: i32 = 15;        // rax,rdx(UD) / r(U)   cc: 6 div, 7 idiv
pub const SETCC: i32 = 16;      // r(D) (8-bit) <- cc
pub const CMOV: i32 = 17;       // r(UD) <- r(U) if cc
pub const BITSCAN: i32 = 18;    // r(D) <- op r(U): cc 0 bsf, 1 bsr, 2 tzcnt, 3 lzcnt, 4 popcnt
pub const JMP: i32 = 19;        // label
pub const JCC: i32 = 20;        // label, cc
pub const LABEL: i32 = 21;      // local label (aux)
pub const CALL: i32 = 22;       // sym or r(U); implicit register operands follow
pub const RET: i32 = 23;        // epilogue and return; implicit uses
pub const UD2: i32 = 24;
pub const XCHG: i32 = 25;       // m <-> r(UD)
pub const XADD: i32 = 26;       // lock xadd m, r(UD)
pub const CMPXCHG: i32 = 27;    // lock cmpxchg m, r(U); rax(UD)
pub const MFENCE: i32 = 28;
pub const TLSBASE: i32 = 29;    // r(D) <- fs:0
pub const SSE: i32 = 30;        // descriptor cc; ops: [reg field, rm, vvvv (VEX only) | none, imm | none]
pub const REP: i32 = 31;        // cc 0: rep movsb (rdi, rsi, rcx), 1: rep stosb (rdi, al, rcx); cld/std in aux
pub const SHIFTX: i32 = 32;     // BMI2: r(D) <- r(U) shift r(U): cc 0 shlx 1 shrx 2 sarx
pub const NOP: i32 = 33;
pub const PARAM: i32 = 34;      // pseudo: defines incoming argument registers at entry
pub const CMP_M: i32 = 35;      // cmp m, r(U) / i
pub const LEAVE_TRAP: i32 = 36; // unused

// Condition codes
pub const CC_O: i32 = 0;
pub const CC_NO: i32 = 1;
pub const CC_B: i32 = 2;
pub const CC_AE: i32 = 3;
pub const CC_E: i32 = 4;
pub const CC_NE: i32 = 5;
pub const CC_BE: i32 = 6;
pub const CC_A: i32 = 7;
pub const CC_S: i32 = 8;
pub const CC_NS: i32 = 9;
pub const CC_P: i32 = 10;
pub const CC_NP: i32 = 11;
pub const CC_L: i32 = 12;
pub const CC_GE: i32 = 13;
pub const CC_LE: i32 = 14;
pub const CC_G: i32 = 15;

// -----------------------------------------------------------------------------
// SSE / AVX descriptors
// -----------------------------------------------------------------------------
pub struct SseDesc {
    pfx: u8,      // 0, 0x66, 0xF2, 0xF3
    map: u8,      // 1: 0F, 2: 0F38, 3: 0F3A
    opc: u8,
    w: u8,        // REX.W / VEX.W
    digit: i8,    // /digit in the reg field (shift-by-immediate forms), -1 none
    gpr_reg: bool,  // the reg field is a general register (pextrw, movmsk, cvtt*2si)
    gpr_rm: bool,   // the r/m operand is a general register (movd/q, cvtsi2*, pinsr*)
    vex_only: bool, // no legacy encoding (FMA, blends with is4, BMI)
}

fn d(pfx: u8, map: u8, opc: u8) SseDesc {
    return SseDesc.{ pfx: pfx, map: map, opc: opc, w: 0, digit: -1, gpr_reg: false, gpr_rm: false, vex_only: false };
}

// Descriptor ids
pub const MOVSS_LD: i32 = 1;
pub const MOVSS_ST: i32 = 2;
pub const MOVSD_LD: i32 = 3;
pub const MOVSD_ST: i32 = 4;
pub const MOVAPS: i32 = 5;
pub const MOVDQU_LD: i32 = 6;
pub const MOVDQU_ST: i32 = 7;
pub const MOVD_XR: i32 = 8;
pub const MOVQ_XR: i32 = 9;
pub const MOVD_RX: i32 = 10;
pub const MOVQ_RX: i32 = 11;
pub const ADDSS: i32 = 12;
pub const ADDSD: i32 = 13;
pub const ADDPS: i32 = 14;
pub const ADDPD: i32 = 15;
pub const MULSS: i32 = 16;
pub const MULSD: i32 = 17;
pub const MULPS: i32 = 18;
pub const MULPD: i32 = 19;
pub const SUBSS: i32 = 20;
pub const SUBSD: i32 = 21;
pub const SUBPS: i32 = 22;
pub const SUBPD: i32 = 23;
pub const MINSS: i32 = 24;
pub const MINSD: i32 = 25;
pub const MINPS: i32 = 26;
pub const MINPD: i32 = 27;
pub const DIVSS: i32 = 28;
pub const DIVSD: i32 = 29;
pub const DIVPS: i32 = 30;
pub const DIVPD: i32 = 31;
pub const MAXSS: i32 = 32;
pub const MAXSD: i32 = 33;
pub const MAXPS: i32 = 34;
pub const MAXPD: i32 = 35;
pub const SQRTSS: i32 = 36;
pub const SQRTSD: i32 = 37;
pub const SQRTPS: i32 = 38;
pub const SQRTPD: i32 = 39;
pub const CVTSS2SD: i32 = 40;
pub const CVTSD2SS: i32 = 41;
pub const CVTSI2SS: i32 = 42;   // w set by the encoder from sz
pub const CVTSI2SD: i32 = 43;
pub const CVTTSS2SI: i32 = 44;
pub const CVTTSD2SI: i32 = 45;
pub const UCOMISS: i32 = 46;
pub const UCOMISD: i32 = 47;
pub const ANDPS: i32 = 48;
pub const ANDPD: i32 = 49;
pub const ANDNPS: i32 = 50;
pub const ANDNPD: i32 = 51;
pub const ORPS: i32 = 52;
pub const ORPD: i32 = 53;
pub const XORPS: i32 = 54;
pub const XORPD: i32 = 55;
pub const CMPPS: i32 = 56;
pub const CMPPD: i32 = 57;
pub const CMPSS: i32 = 58;
pub const CMPSD: i32 = 59;
pub const PADDB: i32 = 60;
pub const PADDW: i32 = 61;
pub const PADDD: i32 = 62;
pub const PADDQ: i32 = 63;
pub const PSUBB: i32 = 64;
pub const PSUBW: i32 = 65;
pub const PSUBD: i32 = 66;
pub const PSUBQ: i32 = 67;
pub const PMULLW: i32 = 68;
pub const PMULLD: i32 = 69;
pub const PMULUDQ: i32 = 70;
pub const PADDSB: i32 = 71;
pub const PADDSW: i32 = 72;
pub const PADDUSB: i32 = 73;
pub const PADDUSW: i32 = 74;
pub const PSUBSB: i32 = 75;
pub const PSUBSW: i32 = 76;
pub const PSUBUSB: i32 = 77;
pub const PSUBUSW: i32 = 78;
pub const PAND: i32 = 79;
pub const PANDN: i32 = 80;
pub const POR: i32 = 81;
pub const PXOR: i32 = 82;
pub const PCMPEQB: i32 = 83;
pub const PCMPEQW: i32 = 84;
pub const PCMPEQD: i32 = 85;
pub const PCMPEQQ: i32 = 86;
pub const PCMPGTB: i32 = 87;
pub const PCMPGTW: i32 = 88;
pub const PCMPGTD: i32 = 89;
pub const PCMPGTQ: i32 = 90;
pub const PMINUB: i32 = 91;
pub const PMAXUB: i32 = 92;
pub const PMINSW: i32 = 93;
pub const PMAXSW: i32 = 94;
pub const PMINSB: i32 = 95;
pub const PMINSD: i32 = 96;
pub const PMINUW: i32 = 97;
pub const PMINUD: i32 = 98;
pub const PMAXSB: i32 = 99;
pub const PMAXSD: i32 = 100;
pub const PMAXUW: i32 = 101;
pub const PMAXUD: i32 = 102;
pub const PABSB: i32 = 103;
pub const PABSW: i32 = 104;
pub const PABSD: i32 = 105;
pub const PSHUFB: i32 = 106;
pub const PSHUFD: i32 = 107;
pub const PSHUFLW: i32 = 108;
pub const PSHUFHW: i32 = 109;
pub const SHUFPS: i32 = 110;
pub const PUNPCKLBW: i32 = 111;
pub const PUNPCKLWD: i32 = 112;
pub const PUNPCKLDQ: i32 = 113;
pub const PUNPCKLQDQ: i32 = 114;
pub const PUNPCKHQDQ: i32 = 115;
pub const PACKSSWB: i32 = 116;
pub const PACKSSDW: i32 = 117;
pub const PMADDWD: i32 = 118;
pub const PSLLW: i32 = 119;
pub const PSLLD: i32 = 120;
pub const PSLLQ: i32 = 121;
pub const PSRLW: i32 = 122;
pub const PSRLD: i32 = 123;
pub const PSRLQ: i32 = 124;
pub const PSRAW: i32 = 125;
pub const PSRAD: i32 = 126;
pub const PSRLW_I: i32 = 127;
pub const PSRAW_I: i32 = 128;
pub const PSLLW_I: i32 = 129;
pub const PSRLD_I: i32 = 130;
pub const PSRAD_I: i32 = 131;
pub const PSLLD_I: i32 = 132;
pub const PSRLQ_I: i32 = 133;
pub const PSRLDQ_I: i32 = 134;
pub const PSLLQ_I: i32 = 135;
pub const PSLLDQ_I: i32 = 136;
pub const PEXTRB: i32 = 137;
pub const PEXTRW: i32 = 138;
pub const PEXTRD: i32 = 139;
pub const PEXTRQ: i32 = 140;
pub const PINSRB: i32 = 141;
pub const PINSRW: i32 = 142;
pub const PINSRD: i32 = 143;
pub const PINSRQ: i32 = 144;
pub const PMOVMSKB: i32 = 145;
pub const MOVMSKPS: i32 = 146;
pub const MOVMSKPD: i32 = 147;
pub const PTEST: i32 = 148;
pub const ROUNDSS: i32 = 149;
pub const ROUNDSD: i32 = 150;
pub const ROUNDPS: i32 = 151;
pub const ROUNDPD: i32 = 152;
pub const VFMADD231SS: i32 = 153;
pub const VFMADD231SD: i32 = 154;
pub const VFMADD231PS: i32 = 155;
pub const VFMADD231PD: i32 = 156;
pub const MOVQ_XM: i32 = 157;   // movq xmm <- m64 (zero upper)
pub const MOVQ_MX: i32 = 158;   // movq m64 <- xmm
pub const MOVD_XM: i32 = 159;   // movd xmm <- m32
pub const MOVD_MX: i32 = 160;   // movd m32 <- xmm
pub const PUNPCKHBW: i32 = 161;
pub const PSRAD_I2: i32 = 162;
pub const NSSE: i32 = 163;

let mut table: [163]SseDesc = [SseDesc.{ pfx: 0, map: 0, opc: 0, w: 0, digit: -1, gpr_reg: false, gpr_rm: false,
                                         vex_only: false }; 163];
let mut built: bool = false;

fn set(id: i32, x: SseDesc) void {
    table[@int_cast(usize, id)] = x;
}

fn build() void {
    built = true;
    let N: u8 = 0;
    let P66: u8 = 0x66;
    let PF2: u8 = 0xf2;
    let PF3: u8 = 0xf3;
    set(MOVSS_LD, d(PF3, 1, 0x10));
    set(MOVSS_ST, d(PF3, 1, 0x11));
    set(MOVSD_LD, d(PF2, 1, 0x10));
    set(MOVSD_ST, d(PF2, 1, 0x11));
    set(MOVAPS, d(N, 1, 0x28));
    set(MOVDQU_LD, d(PF3, 1, 0x6f));
    set(MOVDQU_ST, d(PF3, 1, 0x7f));
    let mut x = d(P66, 1, 0x6e);
    x.gpr_rm = true;
    set(MOVD_XR, x);
    x.w = 1;
    set(MOVQ_XR, x);
    x = d(P66, 1, 0x7e);
    x.gpr_rm = true;
    set(MOVD_RX, x);
    x.w = 1;
    set(MOVQ_RX, x);
    set(MOVQ_XM, d(PF3, 1, 0x7e));
    set(MOVQ_MX, d(P66, 1, 0xd6));
    set(MOVD_XM, d(P66, 1, 0x6e));
    set(MOVD_MX, d(P66, 1, 0x7e));
    let pf: [4]u8 = [PF3, PF2, N, P66];
    let arith: [6]u8 = [0x58, 0x59, 0x5c, 0x5d, 0x5e, 0x5f];
    for k in @as(usize, 0)..6 {
        for j in @as(usize, 0)..4 {
            set(ADDSS + @int_cast(i32, (k * 4) + j), d(pf[j], 1, arith[k]));
        }
    }
    for j in @as(usize, 0)..4 {
        set(SQRTSS + @int_cast(i32, j), d(pf[j], 1, 0x51));
    }
    set(CVTSS2SD, d(PF3, 1, 0x5a));
    set(CVTSD2SS, d(PF2, 1, 0x5a));
    x = d(PF3, 1, 0x2a);
    x.gpr_rm = true;
    set(CVTSI2SS, x);
    x.pfx = PF2;
    set(CVTSI2SD, x);
    x = d(PF3, 1, 0x2c);
    x.gpr_reg = true;
    set(CVTTSS2SI, x);
    x.pfx = PF2;
    set(CVTTSD2SI, x);
    set(UCOMISS, d(N, 1, 0x2e));
    set(UCOMISD, d(P66, 1, 0x2e));
    set(ANDPS, d(N, 1, 0x54));
    set(ANDPD, d(P66, 1, 0x54));
    set(ANDNPS, d(N, 1, 0x55));
    set(ANDNPD, d(P66, 1, 0x55));
    set(ORPS, d(N, 1, 0x56));
    set(ORPD, d(P66, 1, 0x56));
    set(XORPS, d(N, 1, 0x57));
    set(XORPD, d(P66, 1, 0x57));
    set(CMPPS, d(N, 1, 0xc2));
    set(CMPPD, d(P66, 1, 0xc2));
    set(CMPSS, d(PF3, 1, 0xc2));
    set(CMPSD, d(PF2, 1, 0xc2));
    let pops: [19]u8 = [0xfc, 0xfd, 0xfe, 0xd4, 0xf8, 0xf9, 0xfa, 0xfb, 0xd5, 0, 0xf4, 0xec, 0xed, 0xdc, 0xdd, 0xe8,
                        0xe9, 0xd8, 0xd9];
    for k in @as(usize, 0)..19 {
        set(PADDB + @int_cast(i32, k), d(P66, 1, pops[k]));
    }
    set(PMULLD, d(P66, 2, 0x40));
    set(PAND, d(P66, 1, 0xdb));
    set(PANDN, d(P66, 1, 0xdf));
    set(POR, d(P66, 1, 0xeb));
    set(PXOR, d(P66, 1, 0xef));
    set(PCMPEQB, d(P66, 1, 0x74));
    set(PCMPEQW, d(P66, 1, 0x75));
    set(PCMPEQD, d(P66, 1, 0x76));
    set(PCMPEQQ, d(P66, 2, 0x29));
    set(PCMPGTB, d(P66, 1, 0x64));
    set(PCMPGTW, d(P66, 1, 0x65));
    set(PCMPGTD, d(P66, 1, 0x66));
    set(PCMPGTQ, d(P66, 2, 0x37));
    set(PMINUB, d(P66, 1, 0xda));
    set(PMAXUB, d(P66, 1, 0xde));
    set(PMINSW, d(P66, 1, 0xea));
    set(PMAXSW, d(P66, 1, 0xee));
    let mm: [8]u8 = [0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f];
    for k in @as(usize, 0)..8 {
        set(PMINSB + @int_cast(i32, k), d(P66, 2, mm[k]));
    }
    set(PABSB, d(P66, 2, 0x1c));
    set(PABSW, d(P66, 2, 0x1d));
    set(PABSD, d(P66, 2, 0x1e));
    set(PSHUFB, d(P66, 2, 0x00));
    set(PSHUFD, d(P66, 1, 0x70));
    set(PSHUFLW, d(PF2, 1, 0x70));
    set(PSHUFHW, d(PF3, 1, 0x70));
    set(SHUFPS, d(N, 1, 0xc6));
    set(PUNPCKLBW, d(P66, 1, 0x60));
    set(PUNPCKLWD, d(P66, 1, 0x61));
    set(PUNPCKLDQ, d(P66, 1, 0x62));
    set(PUNPCKLQDQ, d(P66, 1, 0x6c));
    set(PUNPCKHQDQ, d(P66, 1, 0x6d));
    set(PUNPCKHBW, d(P66, 1, 0x68));
    set(PACKSSWB, d(P66, 1, 0x63));
    set(PACKSSDW, d(P66, 1, 0x6b));
    set(PMADDWD, d(P66, 1, 0xf5));
    set(PSLLW, d(P66, 1, 0xf1));
    set(PSLLD, d(P66, 1, 0xf2));
    set(PSLLQ, d(P66, 1, 0xf3));
    set(PSRLW, d(P66, 1, 0xd1));
    set(PSRLD, d(P66, 1, 0xd2));
    set(PSRLQ, d(P66, 1, 0xd3));
    set(PSRAW, d(P66, 1, 0xe1));
    set(PSRAD, d(P66, 1, 0xe2));
    let sh: [10]u8 = [0x71, 0x71, 0x71, 0x72, 0x72, 0x72, 0x73, 0x73, 0x73, 0x73];
    let dg: [10]i8 = [2, 4, 6, 2, 4, 6, 2, 3, 6, 7];
    for k in @as(usize, 0)..10 {
        x = d(P66, 1, sh[k]);
        x.digit = dg[k];
        set(PSRLW_I + @int_cast(i32, k), x);
    }
    x = d(P66, 3, 0x14);
    x.gpr_rm = true;
    set(PEXTRB, x);
    x = d(P66, 1, 0xc5);
    x.gpr_reg = true;
    set(PEXTRW, x);
    x = d(P66, 3, 0x16);
    x.gpr_rm = true;
    set(PEXTRD, x);
    x.w = 1;
    set(PEXTRQ, x);
    x = d(P66, 3, 0x20);
    x.gpr_rm = true;
    set(PINSRB, x);
    x = d(P66, 1, 0xc4);
    x.gpr_rm = true;
    set(PINSRW, x);
    x = d(P66, 3, 0x22);
    x.gpr_rm = true;
    set(PINSRD, x);
    x.w = 1;
    set(PINSRQ, x);
    x = d(P66, 1, 0xd7);
    x.gpr_reg = true;
    set(PMOVMSKB, x);
    x = d(N, 1, 0x50);
    x.gpr_reg = true;
    set(MOVMSKPS, x);
    x.pfx = P66;
    set(MOVMSKPD, x);
    set(PTEST, d(P66, 2, 0x17));
    set(ROUNDSS, d(P66, 3, 0x0a));
    set(ROUNDSD, d(P66, 3, 0x0b));
    set(ROUNDPS, d(P66, 3, 0x08));
    set(ROUNDPD, d(P66, 3, 0x09));
    x = d(P66, 2, 0xb9);
    x.vex_only = true;
    set(VFMADD231SS, x);
    x.w = 1;
    set(VFMADD231SD, x);
    x = d(P66, 2, 0xb8);
    x.vex_only = true;
    set(VFMADD231PS, x);
    x.w = 1;
    set(VFMADD231PD, x);
}

pub fn desc(id: i32) SseDesc {
    if not built {
        build();
    }
    return table[@int_cast(usize, id)];
}

// -----------------------------------------------------------------------------
// Encoding
// -----------------------------------------------------------------------------
pub struct Reloc {
    off: u64,
    ty: u32,
    sym: i32,
    addend: i64,
}

pub struct Fixup {
    off: u64,     // position of the rel32
    label: i32,   // block id, or local label (>= LOCAL_BASE)
}

pub const LOCAL_BASE: i32 = 1_000_000;

pub struct Enc {
    b: bn.Bytes,
    relocs: u.List.[Reloc],
    fixups: u.List.[Fixup],
    avx: bool,
}

fn byte(e: *mut Enc, x: u64) void {
    e.b.u8_(@truncate(u8, x));
}

fn gpr(r: i32) u64 {
    return @as(u64, @bit_cast(u32, r & 15));
}

fn hi(r: i32) u64 {
    return @as(u64, @bit_cast(u32, (r & 15) >> 3));
}

// REX prefix: w, r (reg field), x (index), b (base / rm)
fn rex(e: *mut Enc, w: u64, r: u64, x: u64, b: u64, force: bool) void {
    let v = ((w << 3) | (r << 2)) | ((x << 1) | b);
    if (v != 0) or force {
        byte(e, 0x40 | v);
    }
}

// Memory operand components after register assignment.
pub struct Mem {
    base: i32,     // -1: none (rip-relative when sym >= 0)
    index: i32,
    scale: u8,
    disp: i64,
    sym: i32,
    tpoff: bool,   // disp32 is a TLS offset of sym (absolute, no base)
}

pub fn rm_x(m: *Mem) u64 {
    if m.index >= 0 {
        return hi(m.index);
    }
    return 0;
}

pub fn rm_b(m: *Mem) u64 {
    if m.base >= 0 {
        return hi(m.base);
    }
    return 0;
}

// ModRM (+SIB, disp) for reg field `reg` and memory m. `imm_after`: bytes
// of immediate following (rip-relative displacement adjustment).
pub fn modrm_mem(e: *mut Enc, reg: u64, m: *Mem, imm_after: i64) void {
    let rf = (reg & 7) << 3;
    if m.tpoff {
        // [disp32] absolute via SIB (no base, no index)
        byte(e, 0x04 | rf);
        byte(e, 0x25);
        e.relocs.push(Reloc.{ off: @as(u64, e.b.len()), ty: 23, sym: m.sym, addend: m.disp });
        e.b.u32_(0);
        return;
    }
    if m.base < 0 {
        if m.sym >= 0 {
            // rip-relative
            byte(e, 0x05 | rf);
            e.relocs.push(Reloc.{ off: @as(u64, e.b.len()), ty: 2, sym: m.sym, addend: (m.disp - 4) - imm_after });
            e.b.u32_(0);
            return;
        }
        // absolute disp32 (index only)
        byte(e, 0x04 | rf);
        let mut ss: u64 = 0;
        let mut xi: u64 = 4;
        if m.index >= 0 {
            ss = scale_bits(m.scale);
            xi = gpr(m.index) & 7;
        }
        byte(e, ((ss << 6) | (xi << 3)) | 5);
        e.b.u32_(@bit_cast(u64, m.disp));
        return;
    }
    let bb = gpr(m.base) & 7;
    let mut md: u64 = 2;
    if (m.disp == 0) and (bb != 5) {
        md = 0;
    } else if (m.disp >= -128) and (m.disp <= 127) {
        md = 1;
    }
    if (m.index >= 0) or (bb == 4) {
        byte(e, ((md << 6) | rf) | 4);
        let mut ss: u64 = 0;
        let mut xi: u64 = 4;
        if m.index >= 0 {
            ss = scale_bits(m.scale);
            xi = gpr(m.index) & 7;
        }
        byte(e, ((ss << 6) | (xi << 3)) | bb);
    } else {
        byte(e, ((md << 6) | rf) | bb);
    }
    if md == 1 {
        byte(e, @bit_cast(u64, m.disp));
    } else if md == 2 {
        e.b.u32_(@bit_cast(u64, m.disp));
    }
}

fn scale_bits(s: u8) u64 {
    if s == 2 {
        return 1;
    }
    if s == 4 {
        return 2;
    }
    if s == 8 {
        return 3;
    }
    return 0;
}

pub fn modrm_rr(e: *mut Enc, reg: u64, rm: u64) void {
    byte(e, (0xc0 | ((reg & 7) << 3)) | (rm & 7));
}

// An integer instruction: [66] [rex] opcode bytes, ModRM reg/r|m.
// op: opcode bytes (1-3); rm: register (rm_reg >= 0) or memory.
pub fn int_rr(e: *mut Enc, sz: u8, op: []u8, reg: i32, rm: i32) void {
    if sz == 2 {
        byte(e, 0x66);
    }
    let w = u.pick(sz == 8, "\x01", "\x00")[0];
    // byte registers spl/bpl/sil/dil need a REX prefix
    let force = (sz == 1) and ((((reg & 15) >= 4) and ((reg & 15) <= 7)) or (((rm & 15) >= 4) and ((rm & 15) <= 7)));
    rex(e, @as(u64, w), hi(reg), 0, hi(rm), force);
    e.b.put(op);
    modrm_rr(e, gpr(reg), gpr(rm));
}

pub fn int_rm(e: *mut Enc, sz: u8, op: []u8, reg: i32, m: *Mem, imm_after: i64) void {
    if sz == 2 {
        byte(e, 0x66);
    }
    let w = u.pick(sz == 8, "\x01", "\x00")[0];
    let force = (sz == 1) and ((reg & 15) >= 4) and ((reg & 15) <= 7);
    rex(e, @as(u64, w), hi(reg), rm_x(m), rm_b(m), force);
    e.b.put(op);
    modrm_mem(e, gpr(reg), m, imm_after);
}

pub fn imm_n(e: *mut Enc, x: i64, n: u8) void {
    let v = @bit_cast(u64, x);
    if n == 1 {
        byte(e, v);
    } else if n == 2 {
        e.b.u16_(v);
    } else if n == 4 {
        e.b.u32_(v);
    } else {
        e.b.u64_(v);
    }
}

// SSE/AVX instruction. reg: the reg field register (xmm or gpr); rm: register
// (rm_is_mem false) or memory; vvvv: VEX first source (-1: none / 1111b).
pub fn sse(e: *mut Enc, id: i32, reg: i32, rm_reg: i32, m: *Mem, vvvv: i32, imm: i64, has_imm: bool, wide: bool) void {
    let x = desc(id);
    let mut w = @as(u64, x.w);
    if wide {
        w = 1;
    }
    let rmem = rm_reg < 0;
    let mut regf = gpr(reg);
    if x.digit >= 0 {
        regf = @as(u64, @bit_cast(u8, x.digit));
    }
    let r_hi = hi(reg);
    let mut xb: u64 = 0;
    let mut bb: u64 = 0;
    if rmem {
        xb = rm_x(m);
        bb = rm_b(m);
    } else {
        bb = hi(rm_reg);
    }
    let mut imm_len: i64 = 0;
    if has_imm {
        imm_len = 1;
    }
    if e.avx or x.vex_only {
        let mut pp: u64 = 0;
        if x.pfx == 0x66 {
            pp = 1;
        } else if x.pfx == 0xf3 {
            pp = 2;
        } else if x.pfx == 0xf2 {
            pp = 3;
        }
        let mut v: u64 = 15;
        if vvvv >= 0 {
            v = (~gpr(vvvv)) & 15;
        }
        let mut rr = r_hi;
        if x.digit >= 0 {
            rr = 0;
            // shift-by-immediate: the destination goes in vvvv, the source in rm
        }
        if (xb == 0) and (bb == 0) and (w == 0) and (x.map == 1) {
            byte(e, 0xc5);
            byte(e, ((((~rr) & 1) << 7) | (v << 3)) | pp);
        } else {
            byte(e, 0xc4);
            byte(e, ((((~rr) & 1) << 7) | (((~xb) & 1) << 6)) | ((((~bb) & 1) << 5) | @as(u64, x.map)));
            byte(e, ((w << 7) | (v << 3)) | pp);
        }
        byte(e, @as(u64, x.opc));
        if rmem {
            modrm_mem(e, regf, m, imm_len);
        } else {
            modrm_rr(e, regf, gpr(rm_reg));
        }
    } else {
        if x.pfx != 0 {
            byte(e, @as(u64, x.pfx));
        }
        let mut rh = r_hi;
        if x.digit >= 0 {
            rh = 0;
        }
        rex(e, w, rh, xb, bb, false);
        byte(e, 0x0f);
        if x.map == 2 {
            byte(e, 0x38);
        } else if x.map == 3 {
            byte(e, 0x3a);
        }
        byte(e, @as(u64, x.opc));
        if rmem {
            modrm_mem(e, regf, m, imm_len);
        } else {
            modrm_rr(e, regf, gpr(rm_reg));
        }
    }
    if has_imm {
        byte(e, @bit_cast(u64, imm));
    }
}

pub fn rel32_fixup(e: *mut Enc, label: i32) void {
    e.fixups.push(Fixup.{ off: @as(u64, e.b.len()), label: label });
    e.b.u32_(0);
}
