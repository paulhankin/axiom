// =============================================================================
// a64enc.ph - AArch64 instruction words. Registers are numbered 0-30 (x0-x30,
// 31 is sp or the zero register depending on the instruction) and vector
// registers 0-31. `sf` selects 64-bit (1) or 32-bit (0) forms.
// =============================================================================

pub fn r5(r: i32) u64 {
    return @as(u64, @bit_cast(u32, r)) & 31;
}

fn sfb(sf: u64) u64 {
    return sf << 31;
}

// -----------------------------------------------------------------------------
// Integer data processing
// -----------------------------------------------------------------------------
// ADD/SUB/ADDS/SUBS (shifted register): op 0 add, 1 sub; s: set flags
pub fn addsub_reg(sf: u64, sub: bool, s: bool, d: i32, n: i32, m: i32, shift: u64, amt: u64) u64 {
    let mut w: u64 = 0x0B00_0000;
    if sub {
        w = w | 0x4000_0000;
    }
    if s {
        w = w | 0x2000_0000;
    }
    return ((w | sfb(sf)) | (shift << 22)) | ((r5(m) << 16) | ((amt & 63) << 10)) | ((r5(n) << 5) | r5(d));
}

// ADD/SUB (extended register, UXTX / LSL #amt): allows sp as d and n
pub fn addsub_ext(sf: u64, sub: bool, s: bool, d: i32, n: i32, m: i32, amt: u64) u64 {
    let mut w: u64 = 0x0B20_0000;
    if sub {
        w = w | 0x4000_0000;
    }
    if s {
        w = w | 0x2000_0000;
    }
    let option: u64 = 3;  // UXTX (64-bit) / UXTW for 32-bit forms
    return ((w | sfb(sf)) | (r5(m) << 16)) | ((option << 13) | ((amt & 7) << 10)) | ((r5(n) << 5) | r5(d));
}

// ADD/SUB (immediate): imm12, optionally shifted left by 12
pub fn addsub_imm(sf: u64, sub: bool, s: bool, d: i32, n: i32, imm12: u64, sh12: bool) u64 {
    let mut w: u64 = 0x1100_0000;
    if sub {
        w = w | 0x4000_0000;
    }
    if s {
        w = w | 0x2000_0000;
    }
    if sh12 {
        w = w | (1 << 22);
    }
    return ((w | sfb(sf)) | ((imm12 & 0xfff) << 10)) | ((r5(n) << 5) | r5(d));
}

// Logical (shifted register): opc 0 and, 1 orr, 2 eor, 3 ands; neg: BIC/ORN/EON/BICS
pub fn logic_reg(sf: u64, opc: u64, neg: bool, d: i32, n: i32, m: i32, shift: u64, amt: u64) u64 {
    let mut w: u64 = 0x0A00_0000 | (opc << 29);
    if neg {
        w = w | (1 << 21);
    }
    return ((w | sfb(sf)) | (shift << 22)) | ((r5(m) << 16) | ((amt & 63) << 10)) | ((r5(n) << 5) | r5(d));
}

// Logical (immediate): `bits` is the (N, immr, imms) encoding from logical_imm
pub fn logic_imm(sf: u64, opc: u64, d: i32, n: i32, bits: u64) u64 {
    return (((0x1200_0000 | (opc << 29)) | sfb(sf)) | (bits << 10)) | ((r5(n) << 5) | r5(d));
}

// The N:immr:imms encoding (13 bits, as N<<12 | immr<<6 | imms) of x as a
// logical immediate of the given width, or -1.
pub fn logical_imm(x0: u64, width: u32) i64 {
    let mut x = x0;
    if width == 32 {
        x = (x & 0xFFFF_FFFF) | ((x & 0xFFFF_FFFF) << 32);
    }
    if (x == 0) or (x == 0xFFFF_FFFF_FFFF_FFFF) {
        return -1;
    }
    // the smallest repeating element size
    let mut size: u32 = 64;
    while size > 2 {
        let h = size / 2;
        let mask = (@as(u64, 1) << h) - 1;
        if (x & mask) != ((x >> h) & mask) {
            break;
        }
        size = h;
    }
    let mut emask: u64 = 0xFFFF_FFFF_FFFF_FFFF;
    if size < 64 {
        emask = (@as(u64, 1) << size) - 1;
    }
    let e = x & emask;
    // e must be a rotated run of ones: find the rotation making it 0..01..1
    let mut rot: u32 = 0;
    let mut v = e;
    while rot < size {
        // is v of the form 2^k - 1?
        if (v != 0) and ((v & (v +% 1)) == 0) {
            break;
        }
        // rotate right by 1 within size
        v = ((v >> 1) | ((v & 1) << (size - 1))) & emask;
        rot += 1;
    }
    if rot == size {
        return -1;
    }
    let mut ones: u32 = 0;
    while ((v >> ones) & 1) != 0 {
        ones += 1;
    }
    // x = ROR(ones-run, immr) where immr = (size - rot) % size
    let immr = @as(u64, (size - rot) % size);
    let mut nbit: u64 = 0;
    let mut imms: u64 = 0;
    if size == 64 {
        nbit = 1;
        imms = @as(u64, ones - 1);
    } else {
        // high bits of imms mark the element size: ~(size*2 - 1) & 0x3f
        imms = ((~@as(u64, (size * 2) - 1)) & 0x3f) | @as(u64, ones - 1);
    }
    return @bit_cast(i64, ((nbit << 12) | (immr << 6)) | imms);
}

// MOVZ (opc 2) / MOVN (0) / MOVK (3)
pub fn movw(sf: u64, opc: u64, d: i32, imm16: u64, hw: u64) u64 {
    return (((0x1280_0000 | (opc << 29)) | sfb(sf)) | (hw << 21)) | (((imm16 & 0xffff) << 5) | r5(d));
}

// SBFM (opc 0) / UBFM (2)
pub fn bfm(sf: u64, opc: u64, d: i32, n: i32, immr: u64, imms: u64) u64 {
    return ((((0x1300_0000 | (opc << 29)) | sfb(sf)) | (sf << 22)) | ((immr & 63) << 16)) | (((imms & 63) << 10) | ((r5(n) << 5) | r5(d)));
}

pub fn lsl_imm(sf: u64, d: i32, n: i32, s: u64) u64 {
    let w: u64 = u64_pick(sf == 1, 64, 32);
    return bfm(sf, 2, d, n, (w - s) % w, (w - 1) - s);
}

pub fn lsr_imm(sf: u64, d: i32, n: i32, s: u64) u64 {
    let w: u64 = u64_pick(sf == 1, 64, 32);
    return bfm(sf, 2, d, n, s, w - 1);
}

pub fn asr_imm(sf: u64, d: i32, n: i32, s: u64) u64 {
    let w: u64 = u64_pick(sf == 1, 64, 32);
    return bfm(sf, 0, d, n, s, w - 1);
}

// EXTR (ror by immediate when n == m)
pub fn extr(sf: u64, d: i32, n: i32, m: i32, lsb: u64) u64 {
    return (((0x1380_0000 | sfb(sf)) | (sf << 22)) | (r5(m) << 16)) | (((lsb & 63) << 10) | ((r5(n) << 5) | r5(d)));
}

// Data-processing (2 source): UDIV 2, SDIV 3, LSLV 8, LSRV 9, ASRV 10, RORV 11
pub fn dp2(sf: u64, opc: u64, d: i32, n: i32, m: i32) u64 {
    return ((0x1AC0_0000 | sfb(sf)) | (r5(m) << 16)) | ((opc << 10) | ((r5(n) << 5) | r5(d)));
}

// MADD (sub false) / MSUB: d = a +/- n*m
pub fn madd(sf: u64, sub: bool, d: i32, n: i32, m: i32, a: i32) u64 {
    let mut w: u64 = 0x1B00_0000 | sfb(sf);
    if sub {
        w = w | (1 << 15);
    }
    return (w | (r5(m) << 16)) | ((r5(a) << 10) | ((r5(n) << 5) | r5(d)));
}

// Data-processing (1 source): RBIT 0, CLZ 4
pub fn dp1(sf: u64, opc: u64, d: i32, n: i32) u64 {
    return ((0x5AC0_0000 | sfb(sf)) | (opc << 10)) | ((r5(n) << 5) | r5(d));
}

// CSEL (op 0), CSINC (1), CSINV (2), CSNEG (3)
pub fn csel(sf: u64, op: u64, d: i32, n: i32, m: i32, cond: u64) u64 {
    let mut w: u64 = 0x1A80_0000 | sfb(sf);
    if (op & 2) != 0 {
        w = w | 0x4000_0000;
    }
    if (op & 1) != 0 {
        w = w | 0x400;
    }
    return (w | (r5(m) << 16)) | (((cond & 15) << 12) | ((r5(n) << 5) | r5(d)));
}

// -----------------------------------------------------------------------------
// Loads and stores. size: 0 byte, 1 half, 2 word, 3 double; v: SIMD&FP;
// opc: 0 store, 1 load, 2 load signed to 64, 3 load signed to 32
// (for q registers: size 0, opc 2 store / 3 load).
// -----------------------------------------------------------------------------
pub fn ldst_uimm(size: u64, v: bool, opc: u64, t: i32, n: i32, imm12: u64) u64 {
    let mut w: u64 = 0x3900_0000 | (size << 30);
    if v {
        w = w | (1 << 26);
    }
    return ((w | (opc << 22)) | ((imm12 & 0xfff) << 10)) | ((r5(n) << 5) | r5(t));
}

pub fn ldst_simm9(size: u64, v: bool, opc: u64, t: i32, n: i32, imm9: i64) u64 {
    let mut w: u64 = 0x3800_0000 | (size << 30);
    if v {
        w = w | (1 << 26);
    }
    return ((w | (opc << 22)) | ((@bit_cast(u64, imm9) & 0x1ff) << 12)) | ((r5(n) << 5) | r5(t));
}

// [n, m] or [n, m, lsl #size] (scaled)
pub fn ldst_reg(size: u64, v: bool, opc: u64, t: i32, n: i32, m: i32, scaled: bool) u64 {
    let mut w: u64 = 0x3820_0800 | (size << 30);
    if v {
        w = w | (1 << 26);
    }
    if scaled {
        w = w | (1 << 12);
    }
    return (((w | (opc << 22)) | (r5(m) << 16)) | (3 << 13)) | ((r5(n) << 5) | r5(t));
}

// LDAXR / LDXR (acquire false)
pub fn ldxr(size: u64, acquire: bool, t: i32, n: i32) u64 {
    let mut w: u64 = 0x085F_7C00 | (size << 30);
    if acquire {
        w = w | (1 << 15);
    }
    return w | ((r5(n) << 5) | r5(t));
}

// STLXR / STXR: s receives the status
pub fn stxr(size: u64, release: bool, s: i32, t: i32, n: i32) u64 {
    let mut w: u64 = 0x0800_7C00 | (size << 30);
    if release {
        w = w | (1 << 15);
    }
    return (w | (r5(s) << 16)) | ((r5(n) << 5) | r5(t));
}

pub fn ldar(size: u64, t: i32, n: i32) u64 {
    return (0x08DF_FC00 | (size << 30)) | ((r5(n) << 5) | r5(t));
}

pub fn stlr(size: u64, t: i32, n: i32) u64 {
    return (0x089F_FC00 | (size << 30)) | ((r5(n) << 5) | r5(t));
}

// LSE: LD<op>AL (op: 0 add, 1 clr, 2 eor, 3 set, 4 smax, 5 smin, 6 umax, 7 umin)
pub fn ldopal(size: u64, op: u64, s: i32, t: i32, n: i32) u64 {
    return (((0x38E0_0000 | (size << 30)) | (r5(s) << 16)) | (op << 12)) | ((r5(n) << 5) | r5(t));
}

pub fn swpal(size: u64, s: i32, t: i32, n: i32) u64 {
    return ((0x38E0_8000 | (size << 30)) | (r5(s) << 16)) | ((r5(n) << 5) | r5(t));
}

// CASAL: s (compare value in, old value out), t (new value)
pub fn casal(size: u64, s: i32, t: i32, n: i32) u64 {
    return ((0x08E0_FC00 | (size << 30)) | (r5(s) << 16)) | ((r5(n) << 5) | r5(t));
}

// -----------------------------------------------------------------------------
// Branches and system
// -----------------------------------------------------------------------------
pub const B: u64 = 0x1400_0000;
pub const BL: u64 = 0x9400_0000;
pub const RET: u64 = 0xD65F_03C0;
pub const UDF: u64 = 0x0000_0000;
pub const DMB_ISH: u64 = 0xD503_3BBF;
pub const NOP: u64 = 0xD503_201F;

pub fn bcond(cond: u64) u64 {
    return 0x5400_0000 | (cond & 15);
}

pub fn cbz(sf: u64, nz: bool, t: i32) u64 {
    let mut w: u64 = 0x3400_0000 | sfb(sf);
    if nz {
        w = w | (1 << 24);
    }
    return w | r5(t);
}

pub fn blr(n: i32) u64 {
    return 0xD63F_0000 | (r5(n) << 5);
}

pub fn mrs_tpidr(t: i32) u64 {
    return 0xD53B_D040 | r5(t);
}

pub fn adrp(d: i32) u64 {
    return 0x9000_0000 | r5(d);
}

// -----------------------------------------------------------------------------
// Floating point (scalar). ty: 0 single, 1 double
// -----------------------------------------------------------------------------
// FADD 2, FSUB 3, FMUL 0, FDIV 1, FMAX 4, FMIN 5
pub fn fp2(ty: u64, opc: u64, d: i32, n: i32, m: i32) u64 {
    return (((0x1E20_0800 | (ty << 22)) | (r5(m) << 16)) | (opc << 12)) | ((r5(n) << 5) | r5(d));
}

// FMOV 0, FABS 1, FNEG 2, FSQRT 3, FCVT to single 4, to double 5,
// FRINTN 8, FRINTP 9, FRINTM 10, FRINTZ 11
pub fn fp1(ty: u64, opc: u64, d: i32, n: i32) u64 {
    return ((0x1E20_4000 | (ty << 22)) | (opc << 15)) | ((r5(n) << 5) | r5(d));
}

pub fn fcmp(ty: u64, n: i32, m: i32) u64 {
    return ((0x1E20_2000 | (ty << 22)) | (r5(m) << 16)) | (r5(n) << 5);
}

pub fn fcsel(ty: u64, d: i32, n: i32, m: i32, cond: u64) u64 {
    return (((0x1E20_0C00 | (ty << 22)) | (r5(m) << 16)) | ((cond & 15) << 12)) | ((r5(n) << 5) | r5(d));
}

// FMADD: d = n * m + a
pub fn fmadd(ty: u64, d: i32, n: i32, m: i32, a: i32) u64 {
    return (((0x1F00_0000 | (ty << 22)) | (r5(m) << 16)) | (r5(a) << 10)) | ((r5(n) << 5) | r5(d));
}

// Conversions between integer and floating point registers:
// rmode/opcode: SCVTF 0/2, UCVTF 0/3, FCVTZS 3/0, FCVTZU 3/1,
// FMOV to gpr 0/6, FMOV from gpr 0/7
pub fn fcvt_int(sf: u64, ty: u64, rmode: u64, opcode: u64, d: i32, n: i32) u64 {
    return ((((0x1E20_0000 | sfb(sf)) | (ty << 22)) | (rmode << 19)) | (opcode << 16)) | ((r5(n) << 5) | r5(d));
}

// -----------------------------------------------------------------------------
// Advanced SIMD (128-bit, q = 1 unless noted). size: lane size 0..3
// -----------------------------------------------------------------------------
// Three registers of the same type
pub fn v3(q: u64, u: u64, size: u64, opcode: u64, d: i32, n: i32, m: i32) u64 {
    return ((((0x0E20_0400 | (q << 30)) | (u << 29)) | (size << 22)) | ((r5(m) << 16) | (opcode << 11))) | ((r5(n) << 5) | r5(d));
}

// Two-register miscellaneous
pub fn v2(q: u64, u: u64, size: u64, opcode: u64, d: i32, n: i32) u64 {
    return ((((0x0E20_0800 | (q << 30)) | (u << 29)) | (size << 22)) | (opcode << 12)) | ((r5(n) << 5) | r5(d));
}

// Across lanes: ADDV (u 0, 27), UMAXV (1, 10), UMINV (1, 26)
pub fn vacross(q: u64, u: u64, size: u64, opcode: u64, d: i32, n: i32) u64 {
    return ((((0x0E30_0800 | (q << 30)) | (u << 29)) | (size << 22)) | (opcode << 12)) | ((r5(n) << 5) | r5(d));
}

// Shift by immediate: SHL (u 0, opcode 10), SSHR (0, 0), USHR (1, 0);
// esize: lane bits
pub fn vshift(q: u64, u: u64, opcode: u64, esize: u64, left: bool, sh: u64, d: i32, n: i32) u64 {
    let mut immhb: u64 = (2 * esize) - sh;
    if left {
        immhb = esize + sh;
    }
    return ((((0x0F00_0400 | (q << 30)) | (u << 29)) | ((immhb & 127) << 16)) | (opcode << 11)) | ((r5(n) << 5) | r5(d));
}

pub fn imm5(size: u64, index: u64) u64 {
    return ((index << 1) | 1) << size;
}

// DUP Vd.T, Rn
pub fn dup_gen(size: u64, d: i32, n: i32) u64 {
    return ((0x4E00_0C00 | (imm5(size, 0) << 16)) | (r5(n) << 5)) | r5(d);
}

// DUP Vd.T, Vn.T[index]
pub fn dup_elem(size: u64, index: u64, d: i32, n: i32) u64 {
    return ((0x4E00_0400 | (imm5(size, index) << 16)) | (r5(n) << 5)) | r5(d);
}

// DUP (scalar) Vd, Vn.T[index]: the lane into the low element
pub fn dup_scalar(size: u64, index: u64, d: i32, n: i32) u64 {
    return ((0x5E00_0400 | (imm5(size, index) << 16)) | (r5(n) << 5)) | r5(d);
}

// INS Vd.T[index], Rn
pub fn ins_gen(size: u64, index: u64, d: i32, n: i32) u64 {
    return ((0x4E00_1C00 | (imm5(size, index) << 16)) | (r5(n) << 5)) | r5(d);
}

// INS Vd.T[index], Vn.T[src]
pub fn ins_elem(size: u64, index: u64, src: u64, d: i32, n: i32) u64 {
    return (((0x6E00_0400 | (imm5(size, index) << 16)) | ((src << size) << 11)) | (r5(n) << 5)) | r5(d);
}

// UMOV Rd, Vn.T[index] (64-bit lanes: X register)
pub fn umov(size: u64, index: u64, d: i32, n: i32) u64 {
    let mut q: u64 = 0;
    if size == 3 {
        q = 1;
    }
    return (((0x0E00_3C00 | (q << 30)) | (imm5(size, index) << 16)) | (r5(n) << 5)) | r5(d);
}

pub fn movi_zero(d: i32) u64 {
    return 0x6F00_E400 | r5(d);
}

// TBL Vd.16B, {Vn.16B}, Vm.16B
pub fn tbl(d: i32, n: i32, m: i32) u64 {
    return ((0x4E00_0000 | (r5(m) << 16)) | (r5(n) << 5)) | r5(d);
}

// SMULL (q 0: low halves) / SMULL2 (q 1), 16-bit lanes -> 32-bit
pub fn smull(q: u64, d: i32, n: i32, m: i32) u64 {
    return (((0x0E60_C000 | (q << 30)) | (r5(m) << 16)) | (r5(n) << 5)) | r5(d);
}

// ORR Vd.16B, Vn.16B, Vn.16B (vector register copy)
pub fn vmov(d: i32, n: i32) u64 {
    return v3(1, 0, 2, 3, d, n, n);
}

fn u64_pick(c: bool, a: u64, b: u64) u64 {
    if c {
        return a;
    }
    return b;
}
