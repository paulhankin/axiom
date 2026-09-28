// =============================================================================
// a64.ph - AArch64 code generation (AAPCS64, Linux): instruction selection
// from the scalar IR into machine IR (mach.ph), register allocation, frame
// layout and encoding (a64enc.ph) into an ELF object.
//
// Like the x86-64 backend: integers narrower than 32 bits live in registers
// with unspecified upper bits (i1 is always 0 or 1); every function keeps a
// frame pointer (x29). Stack slots are addressed from sp, which does not
// move after the prologue. x16 and x17 are scratch registers of the
// encoder (large offsets, symbol addresses); x18 is reserved.
//
// Most instructions are "raw": the selector computes the instruction word
// and the encoder fills in the allocated registers.
// =============================================================================
import "util.ph" as u;
import "ir.ph" as ir;
import "opt.ph" as opt;
import "mach.ph" as mc;
import "a64enc.ph" as ae;
import "elf.ph" as elf;
import "bin.ph" as bn;
import "target.ph" as tg;
import "lower.ph" as lower;

const GPR: u8 = 0;
const VR: u8 = 1;

// physical registers: x0-x30 are 0-30, sp is 31, v0-v31 are 32-63
const X0: i32 = 0;
const X16: i32 = 16;
const X17: i32 = 17;
const FP: i32 = 29;
const LR: i32 = 30;
const SP: i32 = 31;
const V0: i32 = 32;

// machine ops
const MOV: i32 = 1;     // d, s (64-bit register copy)
const FMOV: i32 = 2;    // d, s (128-bit vector register copy)
const MOVI: i32 = 3;    // d, imm (sz 4 / 8)
const LDR: i32 = 4;     // d, mem (sz 1, 2, 4, 8: zero-extending)
const STR: i32 = 5;     // mem, s | imm 0
const FLDR: i32 = 6;    // d, mem (sz 4, 8, 16)
const FSTR: i32 = 7;    // mem, s
const LEA: i32 = 8;     // d, mem
const ADDR: i32 = 9;    // d, sym (adrp + add)
const GOT: i32 = 10;    // d, sym (adrp + ldr from the GOT)
const TLS: i32 = 11;    // d, sym (thread pointer + offset)
const B: i32 = 12;      // label
const BCC: i32 = 13;    // label, fake uses...; cc: condition
const CBZ: i32 = 14;    // r, label, fake uses...; cc: 0 cbz, 1 cbnz; sz
const CALL: i32 = 15;   // sym | reg, uses..., defs...
const RET: i32 = 16;    // uses
const LABEL: i32 = 17;  // aux: local label
const RAW: i32 = 18;    // up to 4 register operands: Rd, Rn, Rm, Ra fields; aux: the instruction word

const LOCAL_BASE: i32 = 1_000_000;

// condition codes
const EQ: i32 = 0;
const NE: i32 = 1;
const HS: i32 = 2;
const LO: i32 = 3;
const MI: i32 = 4;
const PL: i32 = 5;
const VS: i32 = 6;
const VC: i32 = 7;
const HI: i32 = 8;
const LS: i32 = 9;
const GE: i32 = 10;
const LT: i32 = 11;
const GT: i32 = 12;
const LE: i32 = 13;

let mut OBJ: *mut elf.Obj = @ptr_from_int(*mut elf.Obj, 0);
let mut TEXT: i32 = 0;
let mut RODATA: i32 = 0;
let mut fsym: u.List.[u32] = u.List.[u32].{ data: @ptr_from_int(*mut u32, 0), len: 0, cap: 0 };
let mut gsym: u.List.[u32] = u.List.[u32].{ data: @ptr_from_int(*mut u32, 0), len: 0, cap: 0 };
let mut galign: u.List.[u64] = u.List.[u64].{ data: @ptr_from_int(*mut u64, 0), len: 0, cap: 0 };
let mut consts: u.SymMap.[u32] = u.SymMap.[u32].{
    keys: @ptr_from_int(*mut usize, 0), vals: @ptr_from_int(*mut u32, 0), cap: 0, len: 0 };
let mut libsyms: u.SymMap.[u32] = u.SymMap.[u32].{
    keys: @ptr_from_int(*mut usize, 0), vals: @ptr_from_int(*mut u32, 0), cap: 0, len: 0 };
let mut errors: bool = false;
let mut LSE_: bool = false;

let mut F: *mut ir.Func = @ptr_from_int(*mut ir.Func, 0);
let mut MF: *mut mc.MFunc = @ptr_from_int(*mut mc.MFunc, 0);
let mut CB: *mut mc.MBlock = @ptr_from_int(*mut mc.MBlock, 0);
let mut nlocal: i32 = 0;
let mut out_area: u64 = 0;   // outgoing stack-argument bytes

fn fail(msg: []u8) void {
    u.eprint(u.f2("phalange: AArch64 code generation: %s (in %s)\n", msg, F.name));
    errors = true;
}

fn vnil() *mut ir.Value {
    return u.nil.[ir.Value]();
}

// -----------------------------------------------------------------------------
// Emission helpers
// -----------------------------------------------------------------------------
fn emit(op: i32, sz: u8, ops: []mc.MOp) *mut mc.MInst {
    let i = mc.new_inst(op, sz, ops);
    mc.append(CB, i);
    return i;
}

fn e0(op: i32, sz: u8) *mut mc.MInst {
    return emit(op, sz, @slice_from_ptr(u.nil.[mc.MOp](), 0));
}

fn e1(op: i32, sz: u8, a: mc.MOp) *mut mc.MInst {
    let ops: [1]mc.MOp = [a];
    return emit(op, sz, ops[..]);
}

fn e2(op: i32, sz: u8, a: mc.MOp, b: mc.MOp) *mut mc.MInst {
    let ops: [2]mc.MOp = [a, b];
    return emit(op, sz, ops[..]);
}

fn R(r: i32) mc.MOp {
    return mc.reg(r, mc.RU);
}

fn W(r: i32) mc.MOp {
    return mc.reg(r, mc.RD);
}

fn RW(r: i32) mc.MOp {
    return mc.reg(r, mc.RUD);
}

fn I(x: i64) mc.MOp {
    return mc.imm(x);
}

fn NONE() mc.MOp {
    return mc.none();
}

// A raw instruction word with register fields Rd, Rn, Rm, Ra.
fn raw(w: u64, d: mc.MOp, n: mc.MOp, m: mc.MOp, a: mc.MOp) *mut mc.MInst {
    let ops: [4]mc.MOp = [d, n, m, a];
    let i = emit(RAW, 0, ops[..]);
    i.aux = @bit_cast(i64, w);
    return i;
}

fn raw2(w: u64, d: i32, n: i32) void {
    _ = raw(w, W(d), R(n), NONE(), NONE());
}

fn raw3(w: u64, d: i32, n: i32, m: i32) void {
    _ = raw(w, W(d), R(n), R(m), NONE());
}

// Rd and Rm (Rn fixed in the word, usually the zero register)
fn raw_dm(w: u64, d: i32, m: i32) void {
    _ = raw(w, W(d), NONE(), R(m), NONE());
}

fn raw1(w: u64, d: i32) void {
    _ = raw(w, W(d), NONE(), NONE(), NONE());
}

fn vgpr() i32 {
    return mc.new_vreg(MF, GPR);
}

fn vvr() i32 {
    return mc.new_vreg(MF, VR);
}

fn class_of(t: *ir.Ty) u8 {
    if (t.kind == ir.T_FLOAT) or (t.kind == ir.T_VEC) {
        return VR;
    }
    return GPR;
}

fn is64(t: *ir.Ty) bool {
    return (t.kind == ir.T_PTR) or ((t.kind == ir.T_INT) and (t.bits > 32));
}

fn sfv(t: *ir.Ty) u64 {
    if is64(t) {
        return 1;
    }
    return 0;
}

fn hint(v: i32, r: i32) void {
    if mc.is_vreg(v) {
        let k = @int_cast(usize, v - mc.VREG0);
        if MF.hint.get(k) < 0 {
            MF.hint.set(k, r);
        }
    }
}

fn copy(d: i32, s: i32, class: u8) void {
    if d == s {
        return;
    }
    if class == GPR {
        let i = e2(MOV, 8, W(d), R(s));
        i.is_move = true;
    } else {
        let i = e2(FMOV, 16, W(d), R(s));
        i.is_move = true;
    }
    hint(d, s);
    hint(s, d);
}

fn movi(d: i32, x: i64, sf: u64) void {
    let mut sz: u8 = 4;
    if sf == 1 {
        sz = 8;
    }
    _ = e2(MOVI, sz, W(d), I(x));
}

fn new_label() i32 {
    nlocal += 1;
    return LOCAL_BASE + nlocal;
}

fn place(l: i32) void {
    let i = e0(LABEL, 0);
    i.aux = @as(i64, l);
}

fn bcc(cc: i32, l: i32) void {
    let i = e1(BCC, 0, mc.label(l));
    i.cc = cc;
}

fn jmp(l: i32) void {
    _ = e1(B, 0, mc.label(l));
}

fn invert(cc: i32) i32 {
    return cc ^ 1;
}

// -----------------------------------------------------------------------------
// Symbols and constants
// -----------------------------------------------------------------------------
fn func_sym(f: *ir.Func) i32 {
    return @bit_cast(i32, fsym.get(@int_cast(usize, f.id)));
}

fn extern_global(g: *ir.Global) bool {
    return u.isnil(g.init);
}

fn global_sym(g: *ir.Global) i32 {
    return @bit_cast(i32, gsym.get(@int_cast(usize, g.id)));
}

// A C library function (memcpy, ...).
fn lib_sym(name: []u8) i32 {
    let k = u.intern(name);
    let found = libsyms.get(k);
    if found != 0 {
        return @bit_cast(i32, found - 1);
    }
    let s = elf.ext_sym(OBJ, name, false);
    libsyms.put(k, s + 1);
    return @bit_cast(i32, s);
}

// A memory operand for a symbol: `label` carries the known alignment.
fn mem_sym(sym: i32, disp: i64, align: u64) mc.MOp {
    let mut m = mc.mem_sym(sym, disp);
    m.label = @truncate(i32, @bit_cast(i64, align));
    return m;
}

// A 16-byte constant in the constant pool (rodata).
fn const16(lo: u64, hi: u64) mc.MOp {
    let key = u.intern(u.f2("%s:%s", u.hex16(lo), u.hex16(hi)));
    let found = consts.get(key);
    if found != 0 {
        return mem_sym(@bit_cast(i32, found - 1), 0, 16);
    }
    let s = OBJ.secs.get(@int_cast(usize, RODATA));
    s.data.align(16);
    let off = @as(u64, s.data.len());
    s.data.u64_(lo);
    s.data.u64_(hi);
    let sym = elf.add_sym(OBJ, elf.Sym.{ name: ".LC", sec: RODATA, value: off, size: 16, bind: elf.B_LOCAL,
                                         ty: elf.T_OBJECT, hidden: false });
    consts.put(key, sym + 1);
    return mem_sym(@bit_cast(i32, sym), 0, 16);
}

fn vconst(lo: u64, hi: u64) i32 {
    let r = vvr();
    _ = e2(FLDR, 16, W(r), const16(lo, hi));
    return r;
}

// -----------------------------------------------------------------------------
// Values
// -----------------------------------------------------------------------------
fn is_inst(v: *ir.Value, op: i32) bool {
    return (v.kind == ir.V_INST) and (v.op == op);
}

fn cval(v: *ir.Value) i64 {
    if v.kind == ir.V_CINT {
        if v.ty.bits == 1 {
            return @bit_cast(i64, v.cv);
        }
        return ir.sval(v);
    }
    return 0;
}

fn is_c(v: *ir.Value) bool {
    return (v.kind == ir.V_CINT) or ((v.kind == ir.V_ZERO) and ((v.ty.kind == ir.T_INT) or (v.ty.kind == ir.T_PTR)));
}

fn vreg(v: *mut ir.Value) i32 {
    if v.loc < 0 {
        v.loc = mc.new_vreg(MF, class_of(v.ty));
    }
    return v.loc;
}

// A register holding v (materializing constants and addresses here).
fn reg(v: *mut ir.Value) i32 {
    let k = v.kind;
    if (k == ir.V_INST) and ((v.op == ir.OP_ALLOCA) or ((v.flag & 1) != 0)) {
        let r = vgpr();
        _ = e2(LEA, 8, W(r), addr(v));
        return r;
    }
    if (k == ir.V_INST) or (k == ir.V_PARAM) {
        return vreg(v);
    }
    let t = v.ty;
    if k == ir.V_GLOBAL {
        let r = vgpr();
        let g = v.gv;
        if g.tls {
            _ = e2(TLS, 8, W(r), mc.symop(global_sym(g), 0));
        } else if extern_global(g) {
            _ = e2(GOT, 8, W(r), mc.symop(global_sym(g), 0));
        } else {
            _ = e2(ADDR, 8, W(r), mc.symop(global_sym(g), 0));
        }
        return r;
    }
    if k == ir.V_FUNC {
        let r = vgpr();
        let f = v.fnp;
        if ir.is_decl(f) {
            _ = e2(GOT, 8, W(r), mc.symop(func_sym(f), 0));
        } else {
            _ = e2(ADDR, 8, W(r), mc.symop(func_sym(f), 0));
        }
        return r;
    }
    if class_of(t) == VR {
        let r = vvr();
        if ir.is_zero(v) or (k == ir.V_UNDEF) {
            raw1(ae.movi_zero(0), r);
            return r;
        }
        let mut b = bn.new_bytes();
        const_le(v, &mut b, 0);
        while b.len() < 16 {
            b.u8_(0);
        }
        let lo = b.get_u64(0);
        let hi = b.get_u64(8);
        b.free();
        if t.kind == ir.T_VEC {
            _ = e2(FLDR, 16, W(r), const16(lo, hi));
            return r;
        }
        // a scalar through a general register
        let g = vgpr();
        if t.bits == 32 {
            movi(g, @bit_cast(i64, lo & 0xFFFF_FFFF), 0);
            raw2(ae.fcvt_int(0, 0, 0, 7, 0, 0), r, g);
        } else {
            movi(g, @bit_cast(i64, lo), 1);
            raw2(ae.fcvt_int(1, 1, 0, 7, 0, 0), r, g);
        }
        return r;
    }
    let r = vgpr();
    let x = cval(v);
    if (k == ir.V_UNDEF) or (k == ir.V_ZERO) {
        movi(r, 0, 0);
    } else if (x >= 0) and (x <= 0xffff_ffff) {
        movi(r, x, 0);
    } else if not is64(t) {
        movi(r, x & 0xffff_ffff, 0);
    } else {
        movi(r, x, 1);
    }
    return r;
}

fn const_le(x: *mut ir.Value, out: *mut bn.Bytes, base: usize) void {
    let t = x.ty;
    while out.len() < (base + @truncate(usize, t.size)) {
        out.u8_(0);
    }
    let k = x.kind;
    if k == ir.V_CINT {
        for i in 0..@truncate(usize, t.size) {
            out.set_u8(base + i, @truncate(u8, x.cv >> @int_cast(u32, i * 8)));
        }
    } else if k == ir.V_CFP {
        if t.bits == 32 {
            out.set_u32(base, @as(u64, @bit_cast(u32, @float_cast(f32, x.fv))));
        } else {
            out.set_u64(base, @bit_cast(u64, x.fv));
        }
    } else if k == ir.V_CAGG {
        for i in 0..x.xs.len {
            const_le(x.xs.get(i), out, base + @truncate(usize, ir.field_off(t, i)));
        }
    }
}

fn fits32(x: i64) bool {
    return (x >= -2147483648) and (x <= 2147483647);
}

// A memory operand for address p.
fn addr(p: *mut ir.Value) mc.MOp {
    if is_inst(p, ir.OP_ALLOCA) {
        return mc.mem_slot(@truncate(i32, p.aux), 0);
    }
    if is_inst(p, ir.OP_PTRADD) {
        let off = p.b;
        if is_c(off) {
            let mut m = addr(p.a);
            let nd = m.disp + cval(off);
            if fits32(nd) and (m.index < 0) {
                m.disp = nd;
                return m;
            }
        }
        if (p.flag & 1) != 0 {
            let b = reg(p.a);
            if is_inst(off, ir.OP_MUL) and is_c(off.b) {
                let s = cval(off.b);
                if (s == 1) or (s == 2) or (s == 4) or (s == 8) or (s == 16) {
                    return mc.mem(b, reg(off.a), @truncate(u8, @bit_cast(u64, s)), 0);
                }
            }
            return mc.mem(b, reg(off), 1, 0);
        }
    }
    if (p.kind == ir.V_GLOBAL) and not p.gv.tls and not extern_global(p.gv) {
        return mem_sym(global_sym(p.gv), 0, galign.get(@int_cast(usize, p.gv.id)));
    }
    return mc.mem(reg(p), -1, 0, 0);
}

// -----------------------------------------------------------------------------
// Integer operations
// -----------------------------------------------------------------------------
// ADD/SUB immediate encoding for x (either sign), or false.
fn addsub_imm_ok(x: i64) bool {
    let mut a = x;
    if x < 0 {
        a = 0 - x;
    }
    return (a >= 0) and ((a <= 0xfff) or (((a & 0xfff) == 0) and (a <= 0xff_f000)));
}

// d = a +/- imm
fn add_imm(sf: u64, d: i32, a: i32, x: i64) void {
    let sub = x < 0;
    let mut v = x;
    if sub {
        v = 0 - x;
    }
    if v <= 0xfff {
        raw2(ae.addsub_imm(sf, sub, false, 0, 0, @bit_cast(u64, v), false), d, a);
    } else {
        raw2(ae.addsub_imm(sf, sub, false, 0, 0, @bit_cast(u64, v) >> 12, true), d, a);
    }
}

fn logic_opc(o: i32) u64 {
    if o == ir.OP_AND {
        return 0;
    }
    if o == ir.OP_OR {
        return 1;
    }
    return 2;
}

fn gen_alu(v: *mut ir.Value) void {
    let o = v.op;
    let t = v.ty;
    let sf = sfv(t);
    let mut a = v.a;
    let mut b = v.b;
    if is_c(a) and not is_c(b) and (o != ir.OP_SUB) {
        a = v.b;
        b = v.a;
    }
    let d = vreg(v);
    let ra = reg(a);
    if o == ir.OP_MUL {
        raw3(ae.madd(sf, false, 0, 0, 0, 31), d, ra, reg(b));
    } else if (o == ir.OP_ADD) or (o == ir.OP_SUB) {
        if is_c(b) and addsub_imm_ok(cval(b)) {
            let mut x = cval(b);
            if o == ir.OP_SUB {
                x = -x;
            }
            add_imm(sf, d, ra, x);
        } else {
            raw3(ae.addsub_reg(sf, o == ir.OP_SUB, false, 0, 0, 0, 0, 0), d, ra, reg(b));
        }
    } else {
        let opc = logic_opc(o);
        let mut done = false;
        if is_c(b) {
            let mut width: u32 = 32;
            if sf == 1 {
                width = 64;
            }
            let e = ae.logical_imm(@bit_cast(u64, cval(b)), width);
            if e >= 0 {
                raw2(ae.logic_imm(sf, opc, 0, 0, @bit_cast(u64, e)), d, ra);
                done = true;
            }
        }
        if not done {
            raw3(ae.logic_reg(sf, opc, false, 0, 0, 0, 0, 0), d, ra, reg(b));
        }
    }
    if (t.bits == 1) and (o != ir.OP_AND) and (o != ir.OP_OR) and (o != ir.OP_XOR) {
        raw2(ae.logic_imm(0, 0, 0, 0, @bit_cast(u64, ae.logical_imm(1, 32))), d, d);
    }
}

// Extends the narrow integer in r into a fresh register (32 or 64 bits).
fn ext_reg(r: i32, bits: i32, sign: bool, to64: bool) i32 {
    let d = vgpr();
    let mut sf: u64 = 0;
    if to64 {
        sf = 1;
    }
    if bits == 1 {
        if sign {
            raw_dm(ae.addsub_reg(sf, true, false, 0, 31, 0, 0, 0), d, r);  // neg
        } else {
            copy(d, r, GPR);
        }
        return d;
    }
    if bits == 32 {
        if sign {
            raw2(ae.bfm(1, 0, 0, 0, 0, 31), d, r);  // sxtw
        } else {
            raw_dm(ae.logic_reg(0, 1, false, 0, 31, 0, 0, 0), d, r);  // mov wd, wn
        }
        return d;
    }
    let hi = @as(u64, @bit_cast(u32, bits - 1));
    if sign {
        raw2(ae.bfm(sf, 0, 0, 0, 0, hi), d, r);
    } else {
        raw2(ae.bfm(0, 2, 0, 0, 0, hi), d, r);
    }
    return d;
}

// A register with v extended to 32 bits (narrow) or v itself.
fn reg_ext(v: *mut ir.Value, sign: bool) i32 {
    let r = reg(v);
    let b = v.ty.bits;
    if (v.ty.kind == ir.T_INT) and (b < 32) and (b > 1) {
        return ext_reg(r, b, sign, false);
    }
    return r;
}

fn gen_div(v: *mut ir.Value) void {
    let o = v.op;
    let sg = (o == ir.OP_SDIV) or (o == ir.OP_SREM);
    let sf = sfv(v.ty);
    let a = reg_ext(v.a, sg);
    let b = reg_ext(v.b, sg);
    let mut opc: u64 = 2;
    if sg {
        opc = 3;
    }
    let d = vreg(v);
    if (o == ir.OP_UDIV) or (o == ir.OP_SDIV) {
        raw3(ae.dp2(sf, opc, 0, 0, 0), d, a, b);
        return;
    }
    let q = vgpr();
    raw3(ae.dp2(sf, opc, 0, 0, 0), q, a, b);
    // d = a - q * b
    _ = raw(ae.madd(sf, true, 0, 0, 0, 0), W(d), R(q), R(b), R(a));
}

fn gen_shift(v: *mut ir.Value) void {
    let o = v.op;
    let t = v.ty;
    let sf = sfv(t);
    let bits = t.bits;
    let d = vreg(v);
    let mut a = reg(v.a);
    if (bits < 32) and (bits > 1) {
        if o == ir.OP_LSHR {
            a = ext_reg(a, bits, false, false);
        } else if o == ir.OP_ASHR {
            a = ext_reg(a, bits, true, false);
        }
    }
    let mut w: u64 = 32;
    if sf == 1 {
        w = 64;
    }
    if is_c(v.b) {
        let k = @bit_cast(u64, cval(v.b)) & (w - 1);
        if o == ir.OP_SHL {
            raw2(ae.lsl_imm(sf, 0, 0, k), d, a);
        } else if o == ir.OP_LSHR {
            raw2(ae.lsr_imm(sf, 0, 0, k), d, a);
        } else {
            raw2(ae.asr_imm(sf, 0, 0, k), d, a);
        }
        return;
    }
    let mut c = reg(v.b);
    if (v.b.ty.bits < 32) and (bits < 32) {
        let m = vgpr();
        raw2(ae.logic_imm(0, 0, 0, 0, @bit_cast(u64, ae.logical_imm(@as(u64, @bit_cast(u32, bits - 1)), 32))), m, c);
        c = m;
    }
    let mut opc: u64 = 8;
    if o == ir.OP_LSHR {
        opc = 9;
    } else if o == ir.OP_ASHR {
        opc = 10;
    }
    raw3(ae.dp2(sf, opc, 0, 0, 0), d, a, c);
}

fn icc(p: i32) i32 {
    if p == ir.IntEQ {
        return EQ;
    }
    if p == ir.IntNE {
        return NE;
    }
    if p == ir.IntULT {
        return LO;
    }
    if p == ir.IntULE {
        return LS;
    }
    if p == ir.IntUGT {
        return HI;
    }
    if p == ir.IntUGE {
        return HS;
    }
    if p == ir.IntSLT {
        return LT;
    }
    if p == ir.IntSLE {
        return LE;
    }
    if p == ir.IntSGT {
        return GT;
    }
    return GE;
}

fn is_signed_pred(p: i32) bool {
    return (p == ir.IntSLT) or (p == ir.IntSLE) or (p == ir.IntSGT) or (p == ir.IntSGE);
}

// cmp a, #x (or cmn) when x fits an immediate; false otherwise
fn cmp_imm(sf: u64, a: i32, x: i64) bool {
    if (x >= 0) and (x <= 0xfff) {
        _ = raw(ae.addsub_imm(sf, true, true, 31, 0, @bit_cast(u64, x), false), NONE(), R(a), NONE(), NONE());
        return true;
    }
    if (x < 0) and (x >= -0xfff) {
        _ = raw(ae.addsub_imm(sf, false, true, 31, 0, @bit_cast(u64, 0 - x), false), NONE(), R(a), NONE(), NONE());
        return true;
    }
    return false;
}

fn cmp_reg(sf: u64, a: i32, b: i32) void {
    _ = raw(ae.addsub_reg(sf, true, true, 31, 0, 0, 0, 0), NONE(), R(a), R(b), NONE());
}

// Emits the comparison for an icmp and returns the condition.
fn int_compare(v: *mut ir.Value) i32 {
    let p = @truncate(i32, @bit_cast(i64, v.cv));
    let t = v.a.ty;
    let sf = sfv(t);
    let sg = is_signed_pred(p);
    let a = reg_ext(v.a, sg);
    if is_c(v.b) {
        let mut x = cval(v.b);
        let bits = t.bits;
        if (t.kind == ir.T_INT) and (bits < 32) and (bits > 1) and not sg {
            x = x & ((@as(i64, 1) << @int_cast(u32, bits)) - 1);
        } else if (sf == 0) and not sg {
            x = x & 0xffff_ffff;
        }
        if cmp_imm(sf, a, x) {
            return icc(p);
        }
    }
    cmp_reg(sf, a, reg_ext(v.b, sg));
    return icc(p);
}

fn fty(t: *ir.Ty) u64 {
    if t.bits == 32 {
        return 0;
    }
    return 1;
}

fn float_compare(v: *mut ir.Value) i32 {
    let p = @truncate(i32, @bit_cast(i64, v.cv));
    let a = reg(v.a);
    let b = reg(v.b);
    _ = raw(ae.fcmp(fty(v.a.ty), 0, 0), NONE(), R(a), R(b), NONE());
    if p == ir.RealOEQ {
        return EQ;
    }
    if p == ir.RealUNE {
        return NE;
    }
    if p == ir.RealOLT {
        return MI;
    }
    if p == ir.RealOLE {
        return LS;
    }
    if p == ir.RealOGT {
        return GT;
    }
    return GE;
}

fn cset(d: i32, cc: i32) void {
    raw1(ae.csel(0, 1, 0, 31, 31, @as(u64, @bit_cast(u32, invert(cc)))), d);
}

fn fused(v: *mut ir.Value) bool {
    return (v.kind == ir.V_INST) and ((v.flag & 2) != 0);
}

fn cond_code(c: *mut ir.Value) i32 {
    if fused(c) {
        if c.op == ir.OP_ICMP {
            return int_compare(c);
        }
        return float_compare(c);
    }
    let r = reg(c);
    _ = cmp_imm(0, r, 0);
    return NE;
}

fn cc64(cc: i32) u64 {
    return @as(u64, @bit_cast(u32, cc));
}

fn gen_select(v: *mut ir.Value) void {
    let d = vreg(v);
    let t = v.ty;
    let rt = reg(v.b);
    let rf = reg(v.c);
    if class_of(t) == GPR {
        let cc = cond_code(v.a);
        raw3(ae.csel(sfv(t), 0, 0, 0, 0, cc64(cc)), d, rt, rf);
        return;
    }
    if t.kind == ir.T_FLOAT {
        let cc = cond_code(v.a);
        raw3(ae.fcsel(fty(t), 0, 0, 0, cc64(cc)), d, rt, rf);
        return;
    }
    copy(d, rf, VR);
    let cc = cond_code(v.a);
    let skip = new_label();
    bcc(invert(cc), skip);
    let m = e2(FMOV, 16, RW(d), R(rt));
    _ = m;
    place(skip);
}

fn gen_icmp(v: *mut ir.Value) void {
    if v.a.ty.kind == ir.T_VEC {
        vec_icmp(v);
        return;
    }
    let d = vreg(v);
    let cc = int_compare(v);
    cset(d, cc);
}

fn gen_fcmp(v: *mut ir.Value) void {
    if v.a.ty.kind == ir.T_VEC {
        vec_fcmp(v);
        return;
    }
    let d = vreg(v);
    let cc = float_compare(v);
    cset(d, cc);
}

// -----------------------------------------------------------------------------
// Conversions
// -----------------------------------------------------------------------------
fn gen_cast(v: *mut ir.Value) void {
    let o = v.op;
    let from = v.a.ty;
    let to = v.ty;
    let d = vreg(v);
    if (o == ir.OP_ZEXT) or (o == ir.OP_SEXT) {
        if to.kind == ir.T_VEC {
            copy(d, reg(v.a), VR);  // lane masks are already all ones / zeros
            return;
        }
        let x = ext_reg(reg(v.a), from.bits, o == ir.OP_SEXT, to.bits > 32);
        copy(d, x, GPR);
        return;
    }
    if o == ir.OP_TRUNC {
        let r = reg(v.a);
        if to.bits == 1 {
            raw2(ae.logic_imm(0, 0, 0, 0, @bit_cast(u64, ae.logical_imm(1, 32))), d, r);
        } else {
            copy(d, r, GPR);
        }
        return;
    }
    if (o == ir.OP_PTRTOINT) or (o == ir.OP_INTTOPTR) {
        copy(d, reg(v.a), GPR);
        return;
    }
    if o == ir.OP_BITCAST {
        let fc = class_of(from);
        let tc = class_of(to);
        let r = reg(v.a);
        if fc == tc {
            copy(d, r, fc);
        } else if tc == VR {
            if from.bits == 64 {
                raw2(ae.fcvt_int(1, 1, 0, 7, 0, 0), d, r);
            } else {
                raw2(ae.fcvt_int(0, 0, 0, 7, 0, 0), d, r);
            }
        } else {
            if to.bits == 64 {
                raw2(ae.fcvt_int(1, 1, 0, 6, 0, 0), d, r);
            } else {
                raw2(ae.fcvt_int(0, 0, 0, 6, 0, 0), d, r);
            }
        }
        return;
    }
    if o == ir.OP_FPEXT {
        raw2(ae.fp1(0, 5, 0, 0), d, reg(v.a));
        return;
    }
    if o == ir.OP_FPTRUNC {
        raw2(ae.fp1(1, 4, 0, 0), d, reg(v.a));
        return;
    }
    if (o == ir.OP_SITOFP) or (o == ir.OP_UITOFP) {
        let sg = o == ir.OP_SITOFP;
        let mut r = reg(v.a);
        let fb = from.bits;
        if fb < 32 {
            r = ext_reg(r, fb, sg, false);
        }
        let mut sf: u64 = 0;
        if fb > 32 {
            sf = 1;
        }
        let mut opc: u64 = 3;
        if sg {
            opc = 2;
        }
        raw2(ae.fcvt_int(sf, fty(to), 0, opc, 0, 0), d, r);
        return;
    }
    gen_float_to_int(v, d);
}

// Saturating float -> int: FCVTZS / FCVTZU saturate (NaN -> 0); narrower
// results are clamped from 32 bits.
fn gen_float_to_int(v: *mut ir.Value, d: i32) void {
    let sg = v.op == ir.OP_FPTOSI_SAT;
    let x = reg(v.a);
    let tb = v.ty.bits;
    let mut sf: u64 = 0;
    if tb > 32 {
        sf = 1;
    }
    let mut opc: u64 = 1;
    if sg {
        opc = 0;
    }
    if tb >= 32 {
        raw2(ae.fcvt_int(sf, fty(v.a.ty), 3, opc, 0, 0), d, x);
        return;
    }
    let r = vgpr();
    raw2(ae.fcvt_int(0, fty(v.a.ty), 3, opc, 0, 0), r, x);
    let k = @int_cast(u32, tb);
    if sg {
        let hi = (@as(i64, 1) << (k - 1)) - 1;
        let lo = -(@as(i64, 1) << (k - 1));
        let h = vgpr();
        let l = vgpr();
        let t = vgpr();
        movi(h, hi, 0);
        movi(l, lo & 0xffff_ffff, 0);
        cmp_reg(0, r, h);
        raw3(ae.csel(0, 0, 0, 0, 0, cc64(LT)), t, r, h);
        cmp_reg(0, t, l);
        raw3(ae.csel(0, 0, 0, 0, 0, cc64(GT)), d, t, l);
        return;
    }
    let hi = (@as(i64, 1) << k) - 1;
    let h = vgpr();
    movi(h, hi, 0);
    cmp_reg(0, r, h);
    raw3(ae.csel(0, 0, 0, 0, 0, cc64(LO)), d, r, h);
}

// -----------------------------------------------------------------------------
// Floating point
// -----------------------------------------------------------------------------
fn is_f32(t: *ir.Ty) bool {
    if t.kind == ir.T_VEC {
        return t.elem.bits == 32;
    }
    return t.bits == 32;
}

fn fsz(t: *ir.Ty) u64 {
    if is_f32(t) {
        return 0;
    }
    return 1;
}

fn gen_fbin(v: *mut ir.Value) void {
    let d = vreg(v);
    let a = reg(v.a);
    let b = reg(v.b);
    let o = v.op;
    let s = fsz(v.ty);
    if v.ty.kind == ir.T_VEC {
        let mut w: u64 = 0;
        if o == ir.OP_FADD {
            w = ae.v3(1, 0, s, 0x1a, 0, 0, 0);
        } else if o == ir.OP_FSUB {
            w = ae.v3(1, 0, 2 | s, 0x1a, 0, 0, 0);
        } else if o == ir.OP_FMUL {
            w = ae.v3(1, 1, s, 0x1b, 0, 0, 0);
        } else {
            w = ae.v3(1, 1, s, 0x1f, 0, 0, 0);
        }
        raw3(w, d, a, b);
        return;
    }
    let mut opc: u64 = 1;
    if o == ir.OP_FADD {
        opc = 2;
    } else if o == ir.OP_FSUB {
        opc = 3;
    } else if o == ir.OP_FMUL {
        opc = 0;
    }
    raw3(ae.fp2(s, opc, 0, 0, 0), d, a, b);
}

fn gen_fneg(v: *mut ir.Value) void {
    let d = vreg(v);
    let a = reg(v.a);
    let s = fsz(v.ty);
    if v.ty.kind == ir.T_VEC {
        raw2(ae.v2(1, 1, 2 | s, 0x0f, 0, 0), d, a);
    } else {
        raw2(ae.fp1(s, 2, 0, 0), d, a);
    }
}

fn sign_mask(single: bool) i32 {
    if single {
        return vconst(0x8000_0000_8000_0000, 0x8000_0000_8000_0000);
    }
    return vconst(0x8000_0000_0000_0000, 0x8000_0000_0000_0000);
}

fn gen_float_intr(v: *mut ir.Value, id: i32) void {
    let t = v.ty;
    let vec = t.kind == ir.T_VEC;
    let s = fsz(t);
    let d = vreg(v);
    let a = reg(v.xs.get(0));
    if (id == ir.IN_FABS) or (id == ir.IN_SQRT) or (id == ir.IN_FLOOR) or (id == ir.IN_CEIL) or (id == ir.IN_FTRUNC) or
       (id == ir.IN_NEAREST) {
        if vec {
            let mut w: u64 = 0;
            if id == ir.IN_FABS {
                w = ae.v2(1, 0, 2 | s, 0x0f, 0, 0);
            } else if id == ir.IN_SQRT {
                w = ae.v2(1, 1, 2 | s, 0x1f, 0, 0);
            } else if id == ir.IN_NEAREST {
                w = ae.v2(1, 0, s, 0x18, 0, 0);
            } else if id == ir.IN_FLOOR {
                w = ae.v2(1, 0, s, 0x19, 0, 0);
            } else if id == ir.IN_CEIL {
                w = ae.v2(1, 0, 2 | s, 0x18, 0, 0);
            } else {
                w = ae.v2(1, 0, 2 | s, 0x19, 0, 0);
            }
            raw2(w, d, a);
            return;
        }
        let mut opc: u64 = 11;
        if id == ir.IN_FABS {
            opc = 1;
        } else if id == ir.IN_SQRT {
            opc = 3;
        } else if id == ir.IN_NEAREST {
            opc = 8;
        } else if id == ir.IN_CEIL {
            opc = 9;
        } else if id == ir.IN_FLOOR {
            opc = 10;
        }
        raw2(ae.fp1(s, opc, 0, 0), d, a);
        return;
    }
    if (id == ir.IN_FMIN) or (id == ir.IN_FMAX) {
        let b = reg(v.xs.get(1));
        if vec {
            let mut size = s;
            if id == ir.IN_FMIN {
                size = 2 | s;
            }
            raw3(ae.v3(1, 0, size, 0x1e, 0, 0, 0), d, a, b);
        } else {
            let mut opc: u64 = 4;
            if id == ir.IN_FMIN {
                opc = 5;
            }
            raw3(ae.fp2(s, opc, 0, 0, 0), d, a, b);
        }
        return;
    }
    if id == ir.IN_COPYSIGN {
        let b = reg(v.xs.get(1));
        let m = sign_mask(s == 0);
        let x = vvr();
        let y = vvr();
        raw3(ae.v3(1, 0, 1, 3, 0, 0, 0), x, a, m);  // bic: a without the sign
        raw3(ae.v3(1, 0, 0, 3, 0, 0, 0), y, b, m);  // and: b's sign
        raw3(ae.v3(1, 0, 2, 3, 0, 0, 0), d, x, y);  // orr
        return;
    }
    if id == ir.IN_FMA {
        let b = reg(v.xs.get(1));
        let c = reg(v.xs.get(2));
        if vec {
            copy(d, c, VR);
            _ = raw(ae.v3(1, 0, s, 0x19, 0, 0, 0), RW(d), R(a), R(b), NONE());
        } else {
            _ = raw(ae.fmadd(s, 0, 0, 0, 0), W(d), R(a), R(b), R(c));
        }
        return;
    }
    fail(u.f1("float intrinsic %s", u.idec(id)));
}

// -----------------------------------------------------------------------------
// Memory
// -----------------------------------------------------------------------------
fn mem_size(t: *ir.Ty) u8 {
    if (t.kind == ir.T_INT) and (t.bits <= 8) {
        return 1;
    }
    return @truncate(u8, t.size);
}

fn size_code(sz: u8) u64 {
    if sz == 1 {
        return 0;
    }
    if sz == 2 {
        return 1;
    }
    if sz == 4 {
        return 2;
    }
    return 3;
}

// A register holding the address of memory operand m.
fn base_reg(m: mc.MOp) i32 {
    if (m.base >= 0) and (m.index < 0) and (m.disp == 0) and (m.slot < 0) and (m.sym < 0) {
        return m.base;
    }
    let r = vgpr();
    _ = e2(LEA, 8, W(r), m);
    return r;
}

fn gen_load(v: *mut ir.Value) void {
    let t = v.ty;
    let d = vreg(v);
    let m = addr(v.a);
    let k = t.kind;
    if ((v.flags & ir.F_ATOMIC) != 0) and (k != ir.T_VEC) and (k != ir.T_FLOAT) {
        let b = base_reg(m);
        _ = raw(ae.ldar(size_code(mem_size(t)), 0, 0), W(d), R(b), NONE(), NONE());
        return;
    }
    if k == ir.T_VEC {
        _ = e2(FLDR, 16, W(d), m);
        return;
    }
    if k == ir.T_FLOAT {
        _ = e2(FLDR, @truncate(u8, t.size), W(d), m);
        return;
    }
    _ = e2(LDR, mem_size(t), W(d), m);
}

fn gen_store(v: *mut ir.Value) void {
    let x = v.a;
    let t = x.ty;
    let m = addr(v.b);
    let k = t.kind;
    if ((v.flags & ir.F_ATOMIC) != 0) and (k != ir.T_VEC) and (k != ir.T_FLOAT) {
        let b = base_reg(m);
        let r = reg(x);
        _ = raw(ae.stlr(size_code(mem_size(t)), 0, 0), R(r), R(b), NONE(), NONE());
        raw_none(ae.DMB_ISH);
        return;
    }
    if k == ir.T_VEC {
        _ = e2(FSTR, 16, m, R(reg(x)));
        return;
    }
    if k == ir.T_FLOAT {
        _ = e2(FSTR, @truncate(u8, t.size), m, R(reg(x)));
        return;
    }
    if is_c(x) and (cval(x) == 0) {
        _ = e2(STR, mem_size(t), m, I(0));
        return;
    }
    _ = e2(STR, mem_size(t), m, R(reg(x)));
}

fn raw_none(w: u64) void {
    _ = raw(w, NONE(), NONE(), NONE(), NONE());
}

fn at(m: mc.MOp, k: i64) mc.MOp {
    let mut x = m;
    x.disp += k;
    return x;
}

// A call to a C library function with integer arguments in x0-x2.
fn call_lib(name: []u8, args: []i32) void {
    let mut ops = u.List.[mc.MOp].{ data: u.nil.[mc.MOp](), len: 0, cap: 0 };
    ops.push(mc.symop(lib_sym(name), 0));
    for k in 0..args.len {
        let m = e2(MOV, 8, W(@int_cast(i32, k)), R(args[k]));
        m.is_move = true;
        ops.push(R(@int_cast(i32, k)));
    }
    let ci = emit(CALL, 0, ops.items());
    ci.clobber = caller_saved();
    MF.has_calls = true;
}

// memcpy / memmove / memset
fn gen_memop(v: *mut ir.Value) void {
    let o = v.op;
    if (o == ir.OP_MEMCPY) and is_c(v.c) and (cval(v.c) <= 128) {
        let n = cval(v.c);
        let dm = addr(v.a);
        let sm = addr(v.b);
        let mut k: i64 = 0;
        while k < n {
            let rem = n - k;
            if rem >= 16 {
                let t = vvr();
                _ = e2(FLDR, 16, W(t), at(sm, k));
                _ = e2(FSTR, 16, at(dm, k), R(t));
                k += 16;
                continue;
            }
            let mut w: i64 = 8;
            while w > rem {
                w = w / 2;
            }
            let t = vgpr();
            let sz = @truncate(u8, @bit_cast(u64, w));
            _ = e2(LDR, sz, W(t), at(sm, k));
            _ = e2(STR, sz, at(dm, k), R(t));
            k += w;
        }
        return;
    }
    if (o == ir.OP_MEMSET) and is_c(v.c) and (cval(v.c) <= 64) and is_c(v.b) {
        let n = cval(v.c);
        let dm = addr(v.a);
        let byte = cval(v.b) & 0xff;
        let pat = byte *% 0x0101_0101_0101_0101;
        let t = vgpr();
        movi(t, pat, 1);
        let mut k: i64 = 0;
        while k < n {
            let mut w: i64 = 8;
            while w > (n - k) {
                w = w / 2;
            }
            _ = e2(STR, @truncate(u8, @bit_cast(u64, w)), at(dm, k), R(t));
            k += w;
        }
        return;
    }
    let args: [3]i32 = [reg(v.a), reg(v.b), reg(v.c)];
    if o == ir.OP_MEMSET {
        call_lib("memset", args[..]);
    } else if o == ir.OP_MEMCPY {
        call_lib("memcpy", args[..]);
    } else {
        call_lib("memmove", args[..]);
    }
}

// -----------------------------------------------------------------------------
// Atomics
// -----------------------------------------------------------------------------
// A backward (or forward) branch within a block, with fake reads of what
// the loop uses so their registers stay allocated across it.
fn loop_branch(op: i32, cc: i32, r: i32, l: i32, uses: []i32) void {
    let mut ops = u.List.[mc.MOp].{ data: u.nil.[mc.MOp](), len: 0, cap: 0 };
    if op == CBZ {
        ops.push(R(r));
    }
    ops.push(mc.label(l));
    for x in uses {
        ops.push(R(x));
    }
    let i = emit(op, 8, ops.items());
    i.cc = cc;
}

fn gen_rmw(v: *mut ir.Value) void {
    let o = @truncate(i32, @bit_cast(i64, v.cv));
    let t = v.ty;
    let msz = mem_size(t);
    let sc = size_code(msz);
    let sf = sfv(t);
    let d = vreg(v);
    let a = base_reg(addr(v.a));
    let mut x = reg(v.b);
    if LSE_ {
        if o == ir.RMW_XCHG {
            _ = raw(ae.swpal(sc, 0, 0, 0), W(d), R(a), R(x), NONE());
            return;
        }
        let mut op: u64 = 0;
        if o == ir.RMW_SUB {
            let n = vgpr();
            raw_dm(ae.addsub_reg(sf, true, false, 0, 31, 0, 0, 0), n, x);
            x = n;
        } else if o == ir.RMW_AND {
            let n = vgpr();
            raw_dm(ae.logic_reg(sf, 1, true, 0, 31, 0, 0, 0), n, x);  // mvn
            x = n;
            op = 1;
        } else if o == ir.RMW_OR {
            op = 3;
        } else if o == ir.RMW_XOR {
            op = 2;
        } else if o == ir.RMW_MAX {
            op = 4;
        } else if o == ir.RMW_MIN {
            op = 5;
        } else if o == ir.RMW_UMAX {
            op = 6;
        } else if o == ir.RMW_UMIN {
            op = 7;
        }
        _ = raw(ae.ldopal(sc, op, 0, 0, 0), W(d), R(a), R(x), NONE());
        return;
    }
    // load-exclusive / store-exclusive loop
    let sg = (o == ir.RMW_MIN) or (o == ir.RMW_MAX);
    if (msz < 4) and ((o == ir.RMW_MIN) or (o == ir.RMW_MAX) or (o == ir.RMW_UMIN) or (o == ir.RMW_UMAX)) {
        x = ext_reg(x, t.bits, sg, false);
    }
    let lp = new_label();
    place(lp);
    _ = raw(ae.ldxr(sc, true, 0, 0), W(d), R(a), NONE(), NONE());
    let mut nv = x;
    if o != ir.RMW_XCHG {
        nv = vgpr();
        if o == ir.RMW_ADD {
            raw3(ae.addsub_reg(sf, false, false, 0, 0, 0, 0, 0), nv, d, x);
        } else if o == ir.RMW_SUB {
            raw3(ae.addsub_reg(sf, true, false, 0, 0, 0, 0, 0), nv, d, x);
        } else if o == ir.RMW_AND {
            raw3(ae.logic_reg(sf, 0, false, 0, 0, 0, 0, 0), nv, d, x);
        } else if o == ir.RMW_OR {
            raw3(ae.logic_reg(sf, 1, false, 0, 0, 0, 0, 0), nv, d, x);
        } else if o == ir.RMW_XOR {
            raw3(ae.logic_reg(sf, 2, false, 0, 0, 0, 0, 0), nv, d, x);
        } else {
            let mut cur = d;
            if (msz < 4) and sg {
                cur = ext_reg(d, t.bits, true, false);
            }
            cmp_reg(sf, cur, x);
            let mut cc = LT;
            if o == ir.RMW_MAX {
                cc = GT;
            } else if o == ir.RMW_UMIN {
                cc = LO;
            } else if o == ir.RMW_UMAX {
                cc = HI;
            }
            raw3(ae.csel(sf, 0, 0, 0, 0, cc64(cc)), nv, cur, x);
        }
    }
    let st = vgpr();
    _ = raw(ae.stxr(sc, true, 0, 0, 0), R(nv), R(a), mc.reg(st, mc.RD | mc.RE), NONE());
    let uses: [2]i32 = [a, x];
    loop_branch(CBZ, 1, st, lp, uses[..]);
}

fn gen_cmpxchg(v: *mut ir.Value) void {
    let t = v.ty;
    let msz = mem_size(t);
    let sc = size_code(msz);
    let sf = sfv(t);
    let a = base_reg(addr(v.a));
    let e = reg(v.b);
    let n = reg(v.c);
    let d = vreg(v);
    if LSE_ {
        copy(d, e, GPR);
        _ = raw(ae.casal(sc, 0, 0, 0), R(n), R(a), RW(d), NONE());
        return;
    }
    let mut ex = e;
    if msz < 4 {
        ex = ext_reg(e, t.bits, false, false);
    }
    let lp = new_label();
    let done = new_label();
    place(lp);
    _ = raw(ae.ldxr(sc, true, 0, 0), W(d), R(a), NONE(), NONE());
    cmp_reg(sf, d, ex);
    let u1: [3]i32 = [a, ex, n];
    loop_branch(BCC, NE, 0, done, u1[..]);
    let st = vgpr();
    _ = raw(ae.stxr(sc, true, 0, 0, 0), R(n), R(a), mc.reg(st, mc.RD | mc.RE), NONE());
    loop_branch(CBZ, 1, st, lp, u1[..]);
    place(done);
}

// -----------------------------------------------------------------------------
// Calls
// -----------------------------------------------------------------------------
fn caller_saved() u64 {
    // x0-x18, x30 and (conservatively: only their low halves are preserved) all
    // vector registers
    return (0x7_ffff | (@as(u64, 1) << 30)) | (@as(u64, 0xFFFF_FFFF) << 32);
}

fn gen_call(v: *mut ir.Value) void {
    let fty_ = v.fty;
    let n = v.xs.len;
    let regs = u.newn.[i32](n + 1);
    for i in 0..n {
        let x = v.xs.get(i);
        let mut e: u8 = 0;
        if (i + 1) < v.ext.len {
            e = v.ext[i + 1];
        }
        let mut r = reg(x);
        if (e != 0) and (x.ty.kind == ir.T_INT) and (x.ty.bits < 32) {
            r = ext_reg(r, x.ty.bits, e == 2, false);
        }
        regs[i] = r;
    }
    let mut target: i32 = -1;
    if v.a.kind != ir.V_FUNC {
        target = reg(v.a);
    }
    let mut ng: i32 = 0;
    let mut nv: i32 = 0;
    let mut off: u64 = 0;
    let mut uses = u.List.[mc.MOp].{ data: u.nil.[mc.MOp](), len: 0, cap: 0 };
    let dest = u.newn.[i32](n + 1);
    for i in 0..n {
        let x = v.xs.get(i);
        let c = class_of(x.ty);
        dest[i] = -1;
        if (c == GPR) and (ng < 8) {
            dest[i] = ng;
            ng += 1;
        } else if (c == VR) and (nv < 8) {
            dest[i] = V0 + nv;
            nv += 1;
        } else {
            let mut sz: u64 = 8;
            if x.ty.kind == ir.T_VEC {
                sz = 16;
                off = (off + 15) & ~@as(u64, 15);
            }
            let m = mc.mem(SP, -1, 0, @bit_cast(i64, off));
            if c == GPR {
                _ = e2(STR, 8, m, R(regs[i]));
            } else {
                _ = e2(FSTR, @truncate(u8, x.ty.size), m, R(regs[i]));
            }
            off += sz;
        }
    }
    off = (off + 15) & ~@as(u64, 15);
    if off > out_area {
        out_area = off;
    }
    for i in 0..n {
        if dest[i] >= 0 {
            if dest[i] >= V0 {
                let m = e2(FMOV, 16, W(dest[i]), R(regs[i]));
                m.is_move = true;
            } else {
                let m = e2(MOV, 8, W(dest[i]), R(regs[i]));
                m.is_move = true;
            }
            hint(regs[i], dest[i]);
            uses.push(R(dest[i]));
        }
    }
    let mut ops = u.List.[mc.MOp].{ data: u.nil.[mc.MOp](), len: 0, cap: 0 };
    if target >= 0 {
        ops.push(R(target));
    } else {
        ops.push(mc.symop(func_sym(v.a.fnp), 0));
    }
    for k in 0..uses.len {
        ops.push(uses.get(k));
    }
    let rt = fty_.ret;
    let has_ret = rt.kind != ir.T_VOID;
    let mut rreg = X0;
    if has_ret and (class_of(rt) == VR) {
        rreg = V0;
    }
    if has_ret {
        ops.push(W(rreg));
    }
    let ci = emit(CALL, 0, ops.items());
    ci.clobber = caller_saved();
    if (v.a.kind == ir.V_FUNC) and v.a.fnp.noreturn {
        ci.clobber = 0;  // nothing is live after it
    }
    MF.has_calls = true;
    if has_ret {
        copy(vreg(v), rreg, class_of(rt));
    }
    u.free(@ptr_cast(*mut void, uses.data));
    u.free(@ptr_cast(*mut void, ops.data));
}

// -----------------------------------------------------------------------------
// Integer intrinsics
// -----------------------------------------------------------------------------
fn and_imm(sf: u64, d: i32, a: i32, x: u64) void {
    let mut w: u32 = 32;
    if sf == 1 {
        w = 64;
    }
    raw2(ae.logic_imm(sf, 0, 0, 0, @bit_cast(u64, ae.logical_imm(x, w))), d, a);
}

fn orr_imm(sf: u64, d: i32, a: i32, x: u64) void {
    let mut w: u32 = 32;
    if sf == 1 {
        w = 64;
    }
    raw2(ae.logic_imm(sf, 1, 0, 0, @bit_cast(u64, ae.logical_imm(x, w))), d, a);
}

fn gen_int_intr(v: *mut ir.Value, id: i32) void {
    let t = v.ty;
    let bits = t.bits;
    let sf = sfv(t);
    let d = vreg(v);
    let a0 = v.xs.get(0);
    if (id == ir.IN_UMUL_OV) or (id == ir.IN_SMUL_OV) {
        // unsigned: the high half is nonzero; signed: the high half is not
        // the sign extension of the low half
        let a = reg(a0);
        let b = reg(v.xs.get(1));
        let h = vgpr();
        raw3(ae.mulh(id == ir.IN_SMUL_OV, 0, 0, 0), h, a, b);
        if id == ir.IN_UMUL_OV {
            _ = cmp_imm(1, h, 0);
        } else {
            let l = vgpr();
            raw3(ae.madd(1, false, 0, 0, 0, 31), l, a, b);
            _ = raw(ae.addsub_reg(1, true, true, 31, 0, 0, 2, 63), NONE(), R(h), R(l), NONE());
        }
        cset(d, NE);
        return;
    }
    if id == ir.IN_CTLZ {
        let x = reg_ext(a0, false);
        if bits < 32 {
            let c = vgpr();
            raw2(ae.dp1(0, 4, 0, 0), c, x);
            add_imm(0, d, c, -@as(i64, 32 - bits));
        } else {
            raw2(ae.dp1(sf, 4, 0, 0), d, x);
        }
        return;
    }
    if id == ir.IN_CTTZ {
        let mut x = reg(a0);
        if bits < 32 {
            let y = vgpr();
            orr_imm(0, y, x, @as(u64, 1) << @int_cast(u32, bits));
            x = y;
        }
        let r = vgpr();
        raw2(ae.dp1(sf, 0, 0, 0), r, x);
        raw2(ae.dp1(sf, 4, 0, 0), d, r);
        return;
    }
    if id == ir.IN_CTPOP {
        let x = reg_ext(a0, false);
        let f = vvr();
        if bits > 32 {
            raw2(ae.fcvt_int(1, 1, 0, 7, 0, 0), f, x);
        } else {
            raw2(ae.fcvt_int(0, 0, 0, 7, 0, 0), f, x);
        }
        let c = vvr();
        raw2(ae.v2(0, 0, 0, 5, 0, 0), c, f);        // cnt v.8b
        let s = vvr();
        raw2(ae.vacross(0, 0, 0, 0x1b, 0, 0), s, c);  // addv b, v.8b
        raw2(ae.umov(0, 0, 0, 0), d, s);
        return;
    }
    if (id == ir.IN_ROTL) or (id == ir.IN_ROTR) {
        let mut x = reg(a0);
        let k = v.xs.get(1);
        let mut rsf = sf;
        let mut rb: i32 = bits;
        if bits < 32 {
            // replicate the value so a 32-bit rotation rotates it
            let z = ext_reg(x, bits, false, false);
            let y = vgpr();
            _ = raw(ae.logic_reg(0, 1, false, 0, 0, 0, 0, @as(u64, @bit_cast(u32, bits))), W(y), R(z), R(z), NONE());
            x = y;
            if bits == 8 {
                let y2 = vgpr();
                _ = raw(ae.logic_reg(0, 1, false, 0, 0, 0, 0, 16), W(y2), R(y), R(y), NONE());
                x = y2;
            }
            rsf = 0;
            rb = 32;
        }
        if is_c(k) {
            let m = @bit_cast(u64, cval(k)) & @as(u64, @bit_cast(u32, bits - 1));
            let mut n = m;
            if id == ir.IN_ROTL {
                n = (@as(u64, @bit_cast(u32, bits)) - m) & @as(u64, @bit_cast(u32, bits - 1));
            }
            _ = rb;
            _ = raw(ae.extr(rsf, 0, 0, 0, n), W(d), R(x), R(x), NONE());
            return;
        }
        let mut c = reg(k);
        if id == ir.IN_ROTL {
            let nc = vgpr();
            raw_dm(ae.addsub_reg(0, true, false, 0, 31, 0, 0, 0), nc, c);
            c = nc;
        }
        raw3(ae.dp2(rsf, 11, 0, 0, 0), d, x, c);
        return;
    }
    if (id == ir.IN_SMIN) or (id == ir.IN_SMAX) or (id == ir.IN_UMIN) or (id == ir.IN_UMAX) {
        let sg = (id == ir.IN_SMIN) or (id == ir.IN_SMAX);
        let a = reg_ext(a0, sg);
        let b = reg_ext(v.xs.get(1), sg);
        cmp_reg(sf, a, b);
        let mut cc = LT;
        if id == ir.IN_SMAX {
            cc = GT;
        } else if id == ir.IN_UMIN {
            cc = LO;
        } else if id == ir.IN_UMAX {
            cc = HI;
        }
        raw3(ae.csel(sf, 0, 0, 0, 0, cc64(cc)), d, a, b);
        return;
    }
    if id == ir.IN_ABS {
        let a = reg_ext(a0, true);
        _ = cmp_imm(sf, a, 0);
        raw3(ae.csel(sf, 3, 0, 0, 0, cc64(GE)), d, a, a);
        return;
    }
    if (id == ir.IN_SADD_SAT) or (id == ir.IN_UADD_SAT) or (id == ir.IN_SSUB_SAT) or (id == ir.IN_USUB_SAT) {
        let sg = (id == ir.IN_SADD_SAT) or (id == ir.IN_SSUB_SAT);
        let is_add = (id == ir.IN_SADD_SAT) or (id == ir.IN_UADD_SAT);
        if bits < 64 {
            // exact in 64 bits, then clamped
            let a = ext_reg(reg(a0), bits, sg, true);
            let b = ext_reg(reg(v.xs.get(1)), bits, sg, true);
            let r = vgpr();
            raw3(ae.addsub_reg(1, not is_add, false, 0, 0, 0, 0, 0), r, a, b);
            let mut lo: i64 = 0;
            let mut hi: i64 = 0;
            if sg {
                lo = -(@as(i64, 1) << @int_cast(u32, bits - 1));
                hi = (@as(i64, 1) << @int_cast(u32, bits - 1)) - 1;
            } else {
                hi = (@as(i64, 1) << @int_cast(u32, bits)) - 1;
            }
            let lr = vgpr();
            let hr = vgpr();
            movi(lr, lo, 1);
            movi(hr, hi, 1);
            let r2 = vgpr();
            cmp_reg(1, r, lr);
            raw3(ae.csel(1, 0, 0, 0, 0, cc64(GT)), r2, r, lr);
            cmp_reg(1, r2, hr);
            raw3(ae.csel(1, 0, 0, 0, 0, cc64(LT)), d, r2, hr);
            return;
        }
        let a = reg(a0);
        let b = reg(v.xs.get(1));
        let r = vgpr();
        raw3(ae.addsub_reg(1, not is_add, true, 0, 0, 0, 0, 0), r, a, b);
        if sg {
            // on overflow: a < 0 ? MIN : MAX
            let s = vgpr();
            raw2(ae.asr_imm(1, 0, 0, 63), s, a);
            let mx = vgpr();
            movi(mx, 9223372036854775807, 1);
            let sat = vgpr();
            raw3(ae.logic_reg(1, 2, false, 0, 0, 0, 0, 0), sat, s, mx);
            // flags are from the adds / subs above (the moves do not set them)
            raw3(ae.csel(1, 0, 0, 0, 0, cc64(VS)), d, sat, r);
        } else if is_add {
            raw2(ae.csel(1, 2, 0, 0, 31, cc64(LO)), d, r);  // carry: all ones
        } else {
            raw2(ae.csel(1, 0, 0, 0, 31, cc64(HS)), d, r);  // borrow: zero
        }
        return;
    }
    fail(u.f1("integer intrinsic %s", u.idec(id)));
}

// -----------------------------------------------------------------------------
// Vectors
// -----------------------------------------------------------------------------
fn lane_bits(t: *ir.Ty) i32 {
    return t.elem.bits;
}

fn lane_float(t: *ir.Ty) bool {
    return t.elem.kind == ir.T_FLOAT;
}

// lane size code 0..3 for 8/16/32/64-bit lanes
fn lk(t: *ir.Ty) u64 {
    let b = lane_bits(t);
    if b == 8 {
        return 0;
    }
    if b == 16 {
        return 1;
    }
    if b == 32 {
        return 2;
    }
    return 3;
}

// Can the instruction selector handle this vector instruction directly?
// (Everything else is split into lanes by lower.scalarize.)
pub fn legal(v: *mut ir.Value) bool {
    let t = v.ty;
    let o = v.op;
    if o == ir.OP_ICMP {
        return true;
    }
    if (o == ir.OP_INTR) and (v.xs.len > 0) and (v.xs.get(0).ty.kind == ir.T_VEC) {
        let id = @truncate(i32, @bit_cast(i64, v.cv));
        let at_ = v.xs.get(0).ty;
        if (id == ir.IN_ANYTRUE) or (id == ir.IN_BITMASK) or (id == ir.IN_DOT_I16) or (id == ir.IN_ALLTRUE) or
           (id == ir.IN_SWIZZLE) {
            return true;
        }
        if lane_float(at_) {
            return (id == ir.IN_FLOOR) or (id == ir.IN_CEIL) or (id == ir.IN_FTRUNC) or (id == ir.IN_NEAREST) or
                   (id == ir.IN_FMA) or (id == ir.IN_FABS) or (id == ir.IN_SQRT) or (id == ir.IN_FMIN) or
                   (id == ir.IN_FMAX) or (id == ir.IN_COPYSIGN);
        }
        let b = lane_bits(at_);
        if id == ir.IN_ABS {
            return true;
        }
        if (id == ir.IN_SMIN) or (id == ir.IN_SMAX) or (id == ir.IN_UMIN) or (id == ir.IN_UMAX) {
            return b < 64;
        }
        if (id == ir.IN_SADD_SAT) or (id == ir.IN_UADD_SAT) or (id == ir.IN_SSUB_SAT) or (id == ir.IN_USUB_SAT) {
            return true;
        }
        return false;
    }
    if t.kind != ir.T_VEC {
        return true;
    }
    if o == ir.OP_MUL {
        return lane_bits(t) < 64;
    }
    if (o == ir.OP_UDIV) or (o == ir.OP_SDIV) or (o == ir.OP_UREM) or (o == ir.OP_SREM) or (o == ir.OP_FREM) {
        return false;
    }
    if (o >= ir.OP_ZEXT) and (o <= ir.OP_FPTOUI_SAT) {
        return (o == ir.OP_BITCAST) or (o == ir.OP_SEXT) or (o == ir.OP_ZEXT);
    }
    return true;
}

fn vec_binop(v: *mut ir.Value) void {
    let t = v.ty;
    let o = v.op;
    let d = vreg(v);
    let a = reg(v.a);
    if (o == ir.OP_SHL) or (o == ir.OP_LSHR) or (o == ir.OP_ASHR) {
        vec_shift(v, d, a);
        return;
    }
    let b = reg(v.b);
    let k = lk(t);
    let mut w: u64 = 0;
    if o == ir.OP_ADD {
        w = ae.v3(1, 0, k, 0x10, 0, 0, 0);
    } else if o == ir.OP_SUB {
        w = ae.v3(1, 1, k, 0x10, 0, 0, 0);
    } else if o == ir.OP_MUL {
        w = ae.v3(1, 0, k, 0x13, 0, 0, 0);
    } else if o == ir.OP_AND {
        w = ae.v3(1, 0, 0, 3, 0, 0, 0);
    } else if o == ir.OP_OR {
        w = ae.v3(1, 0, 2, 3, 0, 0, 0);
    } else if o == ir.OP_XOR {
        w = ae.v3(1, 1, 0, 3, 0, 0, 0);
    } else {
        fail(u.f1("vector op %s", u.idec(o)));
        return;
    }
    raw3(w, d, a, b);
}

fn vec_shift(v: *mut ir.Value, d: i32, a: i32) void {
    let t = v.ty;
    let o = v.op;
    let k = lk(t);
    let esize = @as(u64, @bit_cast(u32, lane_bits(t)));
    let c = v.b;
    if (c.kind == ir.V_CAGG) or (c.kind == ir.V_ZERO) or (is_inst(c, ir.OP_SPLAT) and is_c(c.a)) {
        let mut n: u64 = 0;
        if c.kind == ir.V_CAGG {
            n = @bit_cast(u64, cval(c.xs.get(0)));
        } else if is_inst(c, ir.OP_SPLAT) {
            n = @bit_cast(u64, cval(c.a));
        }
        n = n & (esize - 1);
        if n == 0 {
            copy(d, a, VR);
            return;
        }
        if o == ir.OP_SHL {
            raw2(ae.vshift(1, 0, 0x0a, esize, true, n, 0, 0), d, a);
        } else if o == ir.OP_LSHR {
            raw2(ae.vshift(1, 1, 0, esize, false, n, 0, 0), d, a);
        } else {
            raw2(ae.vshift(1, 0, 0, esize, false, n, 0, 0), d, a);
        }
        return;
    }
    // by a register: SSHL / USHL shift left by a signed per-lane count
    let cs = reg(c.a);
    let m = vgpr();
    and_imm(0, m, cs, esize - 1);
    let mut cnt = m;
    if o != ir.OP_SHL {
        cnt = vgpr();
        raw_dm(ae.addsub_reg(0, true, false, 0, 31, 0, 0, 0), cnt, m);
    }
    let vc = vvr();
    raw2(ae.dup_gen(k, 0, 0), vc, cnt);
    let mut u_: u64 = 1;
    if o == ir.OP_ASHR {
        u_ = 0;
    }
    raw3(ae.v3(1, u_, k, 0x08, 0, 0, 0), d, a, vc);
}

fn vnot(d: i32, r: i32) void {
    raw2(ae.v2(1, 1, 0, 5, 0, 0), d, r);
}

fn vec_icmp(v: *mut ir.Value) void {
    let t = v.a.ty;
    let p = @truncate(i32, @bit_cast(i64, v.cv));
    let d = vreg(v);
    let a = reg(v.a);
    let b = reg(v.b);
    let k = lk(t);
    if (p == ir.IntEQ) or (p == ir.IntNE) {
        if p == ir.IntEQ {
            raw3(ae.v3(1, 1, k, 0x11, 0, 0, 0), d, a, b);
        } else {
            let x = vvr();
            raw3(ae.v3(1, 1, k, 0x11, 0, 0, 0), x, a, b);
            vnot(d, x);
        }
        return;
    }
    let mut u_: u64 = 0;
    if (p == ir.IntULT) or (p == ir.IntULE) or (p == ir.IntUGT) or (p == ir.IntUGE) {
        u_ = 1;
    }
    let mut opc: u64 = 0x06;  // greater than (CMGT / CMHI)
    if (p == ir.IntSGE) or (p == ir.IntUGE) or (p == ir.IntSLE) or (p == ir.IntULE) {
        opc = 0x07;  // or equal (CMGE / CMHS)
    }
    if (p == ir.IntSLT) or (p == ir.IntULT) or (p == ir.IntSLE) or (p == ir.IntULE) {
        raw3(ae.v3(1, u_, k, opc, 0, 0, 0), d, b, a);
    } else {
        raw3(ae.v3(1, u_, k, opc, 0, 0, 0), d, a, b);
    }
}

fn vec_fcmp(v: *mut ir.Value) void {
    let t = v.a.ty;
    let p = @truncate(i32, @bit_cast(i64, v.cv));
    let d = vreg(v);
    let a = reg(v.a);
    let b = reg(v.b);
    let s = fsz(t);
    let eq = ae.v3(1, 0, s, 0x1c, 0, 0, 0);
    let ge = ae.v3(1, 1, s, 0x1c, 0, 0, 0);
    let gt = ae.v3(1, 1, 2 | s, 0x1c, 0, 0, 0);
    if p == ir.RealOEQ {
        raw3(eq, d, a, b);
    } else if p == ir.RealUNE {
        let x = vvr();
        raw3(eq, x, a, b);
        vnot(d, x);
    } else if p == ir.RealOGT {
        raw3(gt, d, a, b);
    } else if p == ir.RealOGE {
        raw3(ge, d, a, b);
    } else if p == ir.RealOLT {
        raw3(gt, d, b, a);
    } else {
        raw3(ge, d, b, a);
    }
}

fn gen_extract(v: *mut ir.Value) void {
    let t = v.a.ty;
    let lane = @bit_cast(u64, v.cv);
    let d = vreg(v);
    let a = reg(v.a);
    let k = lk(t);
    if lane_float(t) {
        if lane == 0 {
            copy(d, a, VR);
        } else {
            raw2(ae.dup_scalar(k, lane, 0, 0), d, a);
        }
        return;
    }
    raw2(ae.umov(k, lane, 0, 0), d, a);
}

fn gen_insert(v: *mut ir.Value) void {
    let t = v.ty;
    let lane = @bit_cast(u64, v.cv);
    let d = vreg(v);
    let a = reg(v.a);
    let x = reg(v.b);
    let k = lk(t);
    copy(d, a, VR);
    if lane_float(t) {
        _ = raw(ae.ins_elem(k, lane, 0, 0, 0), RW(d), R(x), NONE(), NONE());
    } else {
        _ = raw(ae.ins_gen(k, lane, 0, 0), RW(d), R(x), NONE(), NONE());
    }
}

fn gen_splat(v: *mut ir.Value) void {
    let t = v.ty;
    let d = vreg(v);
    let x = reg(v.a);
    if lane_float(t) {
        raw2(ae.dup_elem(lk(t), 0, 0, 0), d, x);
    } else {
        raw2(ae.dup_gen(lk(t), 0, 0), d, x);
    }
}

fn le8(b: []u8) u64 {
    let mut x: u64 = 0;
    for i in @as(usize, 0)..8 {
        x = x | (@as(u64, b[i]) << @int_cast(u32, i * 8));
    }
    return x;
}

fn gen_shuffle(v: *mut ir.Value) void {
    let t = v.ty;
    let d = vreg(v);
    let a = reg(v.a);
    let b = reg(v.b);
    let lb = @int_cast(u64, lane_bits(t) / 8);
    let n = @as(u64, v.mask.len);
    // bytes from a and from b (out of range: zero), or'ed
    let mut ma: [16]u8 = [0xff; 16];
    let mut mb: [16]u8 = [0xff; 16];
    let mut any_b = false;
    for i in 0..v.mask.len {
        let m = @as(u64, @bit_cast(u32, v.mask[i]));
        for k in @as(u64, 0)..lb {
            let dst = @truncate(usize, (@as(u64, i) * lb) + k);
            if m < n {
                ma[dst] = @truncate(u8, (m * lb) + k);
            } else {
                mb[dst] = @truncate(u8, ((m - n) * lb) + k);
                any_b = true;
            }
        }
    }
    let ia = vconst(le8(ma[0..8]), le8(ma[8..16]));
    if not any_b {
        raw3(ae.tbl(0, 0, 0), d, a, ia);
        return;
    }
    let ib = vconst(le8(mb[0..8]), le8(mb[8..16]));
    let xa = vvr();
    let xb = vvr();
    raw3(ae.tbl(0, 0, 0), xa, a, ia);
    raw3(ae.tbl(0, 0, 0), xb, b, ib);
    raw3(ae.v3(1, 0, 2, 3, 0, 0, 0), d, xa, xb);
}

fn gen_vec_intr(v: *mut ir.Value, id: i32) void {
    let a0 = v.xs.get(0);
    let at_ = a0.ty;
    let d = vreg(v);
    let k = lk(at_);
    if (id == ir.IN_ANYTRUE) or (id == ir.IN_ALLTRUE) {
        let a = reg(a0);
        let s = vvr();
        let g = vgpr();
        if id == ir.IN_ANYTRUE {
            raw2(ae.vacross(1, 1, 0, 0x0a, 0, 0), s, a);  // umaxv b
            raw2(ae.umov(0, 0, 0, 0), g, s);
            _ = cmp_imm(0, g, 0);
            cset(d, NE);
            return;
        }
        if k == 3 {
            let z = vvr();
            raw2(ae.v2(1, 0, 3, 0x09, 0, 0), z, a);      // cmeq v.2d, #0
            raw2(ae.vacross(1, 1, 0, 0x0a, 0, 0), s, z);  // any zero lane?
            raw2(ae.umov(0, 0, 0, 0), g, s);
            _ = cmp_imm(0, g, 0);
            cset(d, EQ);
            return;
        }
        raw2(ae.vacross(1, 1, k, 0x1a, 0, 0), s, a);  // uminv
        raw2(ae.umov(k, 0, 0, 0), g, s);
        _ = cmp_imm(0, g, 0);
        cset(d, NE);
        return;
    }
    if id == ir.IN_BITMASK {
        let a = reg(a0);
        let esize = @as(u64, 8) << @truncate(u32, k);
        let bitv = vvr();
        if k == 3 {
            raw2(ae.vshift(1, 1, 0, esize, false, esize - 1, 0, 0), bitv, a);  // ushr: top bits as 0 / 1
            let l0 = vgpr();
            let l1 = vgpr();
            raw2(ae.umov(3, 0, 0, 0), l0, bitv);
            raw2(ae.umov(3, 1, 0, 0), l1, bitv);
            _ = raw(ae.logic_reg(0, 1, false, 0, 0, 0, 0, 1), W(d), R(l0), R(l1), NONE());
            return;
        }
        raw2(ae.vshift(1, 0, 0, esize, false, esize - 1, 0, 0), bitv, a);  // sshr: top bits as all ones
        let mut wl: u64 = 0;
        let mut wh: u64 = 0;
        if k == 0 {
            wl = 0x8040_2010_0804_0201;
            wh = wl;
        } else if k == 1 {
            wl = 0x0008_0004_0002_0001;
            wh = 0x0080_0040_0020_0010;
        } else {
            wl = 0x0000_0002_0000_0001;
            wh = 0x0000_0008_0000_0004;
        }
        let wv = vconst(wl, wh);
        let m = vvr();
        raw3(ae.v3(1, 0, 0, 3, 0, 0, 0), m, bitv, wv);  // and
        let s = vvr();
        if k == 0 {
            // two halves of 8 bytes
            raw2(ae.vacross(0, 0, 0, 0x1b, 0, 0), s, m);
            let hi = vvr();
            raw2(ae.dup_scalar(3, 1, 0, 0), hi, m);
            let s2 = vvr();
            raw2(ae.vacross(0, 0, 0, 0x1b, 0, 0), s2, hi);
            let lo = vgpr();
            let h = vgpr();
            raw2(ae.umov(0, 0, 0, 0), lo, s);
            raw2(ae.umov(0, 0, 0, 0), h, s2);
            _ = raw(ae.logic_reg(0, 1, false, 0, 0, 0, 0, 8), W(d), R(lo), R(h), NONE());
            return;
        }
        raw2(ae.vacross(1, 0, k, 0x1b, 0, 0), s, m);
        raw2(ae.umov(k, 0, 0, 0), d, s);
        return;
    }
    if id == ir.IN_SWIZZLE {
        raw3(ae.tbl(0, 0, 0), d, reg(a0), reg(v.xs.get(1)));
        return;
    }
    if id == ir.IN_DOT_I16 {
        let a = reg(a0);
        let b = reg(v.xs.get(1));
        let p0 = vvr();
        let p1 = vvr();
        raw3(ae.smull(0, 0, 0, 0), p0, a, b);
        raw3(ae.smull(1, 0, 0, 0), p1, a, b);
        raw3(ae.v3(1, 0, 2, 0x17, 0, 0, 0), d, p0, p1);  // addp .4s
        return;
    }
    if lane_float(at_) {
        gen_float_intr(v, id);
        return;
    }
    let a = reg(a0);
    if id == ir.IN_ABS {
        raw2(ae.v2(1, 0, k, 0x0b, 0, 0), d, a);
        return;
    }
    let b = reg(v.xs.get(1));
    let mut w: u64 = 0;
    if id == ir.IN_SMIN {
        w = ae.v3(1, 0, k, 0x0d, 0, 0, 0);
    } else if id == ir.IN_SMAX {
        w = ae.v3(1, 0, k, 0x0c, 0, 0, 0);
    } else if id == ir.IN_UMIN {
        w = ae.v3(1, 1, k, 0x0d, 0, 0, 0);
    } else if id == ir.IN_UMAX {
        w = ae.v3(1, 1, k, 0x0c, 0, 0, 0);
    } else if id == ir.IN_SADD_SAT {
        w = ae.v3(1, 0, k, 0x01, 0, 0, 0);
    } else if id == ir.IN_UADD_SAT {
        w = ae.v3(1, 1, k, 0x01, 0, 0, 0);
    } else if id == ir.IN_SSUB_SAT {
        w = ae.v3(1, 0, k, 0x05, 0, 0, 0);
    } else if id == ir.IN_USUB_SAT {
        w = ae.v3(1, 1, k, 0x05, 0, 0, 0);
    } else {
        fail(u.f1("vector intrinsic %s", u.idec(id)));
        return;
    }
    raw3(w, d, a, b);
}

// -----------------------------------------------------------------------------
// Instructions
// -----------------------------------------------------------------------------
fn gen_ptradd(v: *mut ir.Value) void {
    let d = vreg(v);
    if is_c(v.b) {
        let mut m = addr(v.a);
        let nd = m.disp + cval(v.b);
        if fits32(nd) and (m.index < 0) {
            m.disp = nd;
            _ = e2(LEA, 8, W(d), m);
            return;
        }
    }
    let a = reg(v.a);
    let off = v.b;
    if is_inst(off, ir.OP_MUL) and is_c(off.b) {
        let s = cval(off.b);
        if (s > 0) and ((s & (s - 1)) == 0) {
            _ = e2(LEA, 8, W(d), mc.mem(a, reg(off.a), @truncate(u8, @bit_cast(u64, s)), 0));
            return;
        }
    }
    raw3(ae.addsub_reg(1, false, false, 0, 0, 0, 0, 0), d, a, reg(off));
}

fn is_tls_global(v: *ir.Value) bool {
    return (v.kind == ir.V_GLOBAL) and v.gv.tls;
}

// Constants that take instructions to build (worth building once per loop).
fn const_needs_reg(v: *ir.Value) bool {
    let k = v.kind;
    if k == ir.V_CINT {
        let x = cval(v);
        return (v.ty.kind == ir.T_INT) and (v.ty.bits <= 64) and ((x > 0xffff) or (x < -0xffff));
    }
    if k == ir.V_CFP {
        return not ir.is_zero(v);
    }
    if (k == ir.V_CAGG) and (v.ty.kind == ir.T_VEC) {
        return not ir.is_zero(v);
    }
    return false;
}

fn gen_inst(v: *mut ir.Value) void {
    let o = v.op;
    let t = v.ty;
    if o == ir.OP_COPY {
        copy(vreg(v), reg(v.a), class_of(t));
        return;
    }
    if (o >= ir.OP_ADD) and (o <= ir.OP_ASHR) {
        if t.kind == ir.T_VEC {
            vec_binop(v);
        } else if (o == ir.OP_UDIV) or (o == ir.OP_SDIV) or (o == ir.OP_UREM) or (o == ir.OP_SREM) {
            gen_div(v);
        } else if (o == ir.OP_SHL) or (o == ir.OP_LSHR) or (o == ir.OP_ASHR) {
            gen_shift(v);
        } else {
            gen_alu(v);
        }
        return;
    }
    if (o >= ir.OP_FADD) and (o <= ir.OP_FDIV) {
        gen_fbin(v);
        return;
    }
    if o == ir.OP_FNEG {
        gen_fneg(v);
        return;
    }
    if o == ir.OP_ICMP {
        if not fused(v) {
            gen_icmp(v);
        }
        return;
    }
    if o == ir.OP_FCMP {
        if not fused(v) {
            gen_fcmp(v);
        }
        return;
    }
    if o == ir.OP_SELECT {
        gen_select(v);
        return;
    }
    if (o >= ir.OP_ZEXT) and (o <= ir.OP_FPTOUI_SAT) {
        gen_cast(v);
        return;
    }
    if (o == ir.OP_ALLOCA) or (o == ir.OP_PHI) {
        return;
    }
    if o == ir.OP_LOAD {
        gen_load(v);
        return;
    }
    if o == ir.OP_STORE {
        gen_store(v);
        return;
    }
    if o == ir.OP_PTRADD {
        if (v.flag & 1) == 0 {
            gen_ptradd(v);
        }
        return;
    }
    if (o == ir.OP_MEMCPY) or (o == ir.OP_MEMMOVE) or (o == ir.OP_MEMSET) {
        gen_memop(v);
        return;
    }
    if o == ir.OP_RMW {
        gen_rmw(v);
        return;
    }
    if o == ir.OP_CMPXCHG {
        gen_cmpxchg(v);
        return;
    }
    if o == ir.OP_FENCE {
        raw_none(ae.DMB_ISH);
        return;
    }
    if o == ir.OP_EXTRACTELEM {
        gen_extract(v);
        return;
    }
    if o == ir.OP_INSERTELEM {
        gen_insert(v);
        return;
    }
    if o == ir.OP_SPLAT {
        gen_splat(v);
        return;
    }
    if o == ir.OP_SHUFFLE {
        gen_shuffle(v);
        return;
    }
    if o == ir.OP_CALL {
        gen_call(v);
        return;
    }
    if o == ir.OP_INTR {
        let id = @truncate(i32, @bit_cast(i64, v.cv));
        if id == ir.IN_TRAP {
            raw_none(ae.UDF);
            return;
        }
        if (v.xs.len > 0) and (v.xs.get(0).ty.kind == ir.T_VEC) {
            gen_vec_intr(v, id);
            return;
        }
        if t.kind == ir.T_FLOAT {
            gen_float_intr(v, id);
            return;
        }
        gen_int_intr(v, id);
        return;
    }
    fail(u.f1("cannot select %s", ir.inst_str(v)));
}

// Copies into the successors' phis (as one parallel assignment).
fn phi_copies(b: *mut ir.Block) void {
    let t = ir.terminator(b);
    for k in 0..ir.succ_count(t) {
        let s = ir.succ(t, k);
        let mut dup = false;
        for j in 0..k {
            if ir.succ(t, j) == s {
                dup = true;
            }
        }
        if dup {
            continue;
        }
        let mut tmps = u.List.[i32].{ data: u.nil.[i32](), len: 0, cap: 0 };
        let mut p = s.first;
        while u.notnil(p) and (p.op == ir.OP_PHI) {
            let mut x = ir.undef(p.ty);
            for j in 0..p.bs.len {
                if p.bs.get(j) == b {
                    x = p.xs.get(j);
                }
            }
            let c = class_of(p.ty);
            let tr = mc.new_vreg(MF, c);
            if x.kind != ir.V_UNDEF {
                copy(tr, reg(x), c);
            }
            tmps.push(tr);
            p = p.next;
        }
        p = s.first;
        let mut k2: usize = 0;
        while u.notnil(p) and (p.op == ir.OP_PHI) {
            let c = class_of(p.ty);
            let tr = tmps.get(k2);
            let d = vreg(p);
            let mut defined = false;
            for j in 0..p.bs.len {
                if (p.bs.get(j) == b) and (p.xs.get(j).kind != ir.V_UNDEF) {
                    defined = true;
                }
            }
            if defined {
                copy(d, tr, c);
            }
            k2 += 1;
            p = p.next;
        }
        u.free(@ptr_cast(*mut void, tmps.data));
    }
}

fn next_block_is(b: *mut ir.Block, s: *mut ir.Block) bool {
    return s.aux == (b.aux + 1);
}

fn blk(s: *ir.Block) i32 {
    return @truncate(i32, s.aux);
}

fn gen_term(b: *mut ir.Block) void {
    let t = ir.terminator(b);
    let o = t.op;
    if o == ir.OP_BR {
        phi_copies(b);
        if not next_block_is(b, t.t1) {
            jmp(blk(t.t1));
        }
        return;
    }
    if o == ir.OP_CONDBR {
        let cc = cond_code(t.a);
        phi_copies(b);
        let tt = t.t1;
        let ff = t.t2;
        if next_block_is(b, tt) {
            bcc(invert(cc), blk(ff));
            return;
        }
        bcc(cc, blk(tt));
        if not next_block_is(b, ff) {
            jmp(blk(ff));
        }
        return;
    }
    if o == ir.OP_SWITCH {
        let x = t.a;
        let r = reg_ext(x, true);
        let sf = sfv(x.ty);
        let tr = vgpr();
        copy(tr, r, GPR);
        phi_copies(b);
        for k in 0..t.xs.len {
            let c = t.xs.get(k);
            let mut cv = ir.sval(c);
            if sf == 0 {
                cv = @as(i64, @truncate(i32, cv));
            }
            if not cmp_imm(sf, tr, cv) {
                let cr = vgpr();
                movi(cr, cv, sf);
                cmp_reg(sf, tr, cr);
            }
            bcc(EQ, blk(t.bs.get(k)));
        }
        if not next_block_is(b, t.t1) {
            jmp(blk(t.t1));
        }
        return;
    }
    if o == ir.OP_RET {
        if u.notnil(t.a) {
            let x = t.a;
            let mut r = reg(x);
            if (F.ret_ext != 0) and (x.ty.kind == ir.T_INT) and (x.ty.bits < 32) {
                r = ext_reg(r, x.ty.bits, F.ret_ext == 2, false);
            }
            if class_of(x.ty) == VR {
                let m = e2(FMOV, 16, W(V0), R(r));
                m.is_move = true;
                _ = e1(RET, 0, R(V0));
            } else {
                let m = e2(MOV, 8, W(X0), R(r));
                m.is_move = true;
                _ = e1(RET, 0, R(X0));
            }
            return;
        }
        _ = e0(RET, 0);
        return;
    }
    raw_none(ae.UDF);
}
fn prepare(f: *mut ir.Func) void {
    for p in f.params {
        p.loc = -1;
        p.flag = 0;
    }
    let mut b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            i.loc = -1;
            i.flag = 0;
            i.uses = 0;
            i.user = vnil();
            if i.op == ir.OP_PTRADD {
                i.flag = 1;
            }
            i = i.next;
        }
        b = b.next;
    }
    // uses; address-only pointer arithmetic
    b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            let ops: [3]*mut ir.Value = [i.a, i.b, i.c];
            for k in @as(usize, 0)..3 {
                let o = ops[k];
                if u.isnil(o) or (o.kind != ir.V_INST) {
                    continue;
                }
                o.uses += 1;
                o.user = i;
                if o.op == ir.OP_PTRADD {
                    let is_addr = ((i.op == ir.OP_LOAD) and (k == 0)) or ((i.op == ir.OP_STORE) and (k == 1)) or
                                  ((i.op == ir.OP_PTRADD) and (k == 0)) or
                                  (((i.op == ir.OP_RMW) or (i.op == ir.OP_CMPXCHG)) and (k == 0));
                    if not is_addr {
                        o.flag = o.flag & ~1;
                    }
                }
            }
            for k in 0..i.xs.len {
                let o = i.xs.get(k);
                if o.kind == ir.V_INST {
                    o.uses += 1;
                    o.user = i;
                    if o.op == ir.OP_PTRADD {
                        o.flag = o.flag & ~1;
                    }
                }
            }
            i = i.next;
        }
        b = b.next;
    }
    // comparisons fused into their branch / select
    b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            if ((i.op == ir.OP_ICMP) or (i.op == ir.OP_FCMP)) and (i.a.ty.kind != ir.T_VEC) and (i.uses == 1) {
                let us = i.user;
                if (us.blk == b) and (us.a == i) {
                    if (us.op == ir.OP_CONDBR) or ((us.op == ir.OP_SELECT) and (i.next == us)) {
                        i.flag = i.flag | 2;
                    }
                }
            }
            i = i.next;
        }
        b = b.next;
    }
}


// x9-x15 first (free temporaries), then the argument registers, then the
// callee-saved x19-x28; v16-v31, v0-v7, then v8-v15
let gpr_order: [25]i32 = [9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 19, 20, 21, 22, 23, 24, 25, 26, 27];
let vr_order: [32]i32 = [48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 32, 33, 34, 35, 36, 37, 38, 39,
                         40, 41, 42, 43, 44, 45, 46, 47];

fn mk2(a: mc.MOp, b: mc.MOp) [2]mc.MOp {
    return [a, b];
}

fn spill_load(f: *mut mc.MFunc, r: i32, class: i32, slot: i32) *mut mc.MInst {
    let ops = mk2(W(r), mc.mem_slot(slot, 0));
    if class == 0 {
        return mc.new_inst(LDR, 8, ops[..]);
    }
    return mc.new_inst(FLDR, 16, ops[..]);
}

fn spill_store(f: *mut mc.MFunc, r: i32, class: i32, slot: i32) *mut mc.MInst {
    let ops = mk2(mc.mem_slot(slot, 0), R(r));
    if class == 0 {
        return mc.new_inst(STR, 8, ops[..]);
    }
    return mc.new_inst(FSTR, 16, ops[..]);
}

fn is_label(i: *mc.MInst) bool {
    return i.op == LABEL;
}

fn compile_func(f: *mut ir.Func) void {
    F = f;
    nlocal = 0;
    out_area = 0;
    opt.split_critical_edges(f);
    lower.scalarize(f, legal);
    opt.compute_preds(f);
    let mut order = opt.dominators(f);
    opt.loop_depths(f, &order);
    opt.hoist_constants(f, &order, const_needs_reg);
    opt.hoist_to_entry(f, is_tls_global);
    opt.layout(&mut order);
    prepare(f);
    MF = mc.new_func();
    for i in 0..order.len {
        let b = order.get(i);
        b.aux = @int_cast(i64, i);
        let mb = mc.new_block(MF);
        mb.loop_depth = b.loop_depth;
    }
    let mut b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            if i.op == ir.OP_ALLOCA {
                let mut al = i.aty.align;
                if @as(u64, i.align) > al {
                    al = @as(u64, i.align);
                }
                if al == 0 {
                    al = 1;
                }
                i.aux = @int_cast(i64, mc.new_slot(MF, i.aty.size, al));
            }
            i = i.next;
        }
        b = b.next;
    }
    // parameters
    CB = MF.blocks.get(0);
    let mut ng: i32 = 0;
    let mut nv: i32 = 0;
    let mut soff: i64 = 16;
    for p in f.params {
        let c = class_of(p.ty);
        let d = vreg(p);
        if (c == GPR) and (ng < 8) {
            copy(d, ng, GPR);
            ng += 1;
        } else if (c == VR) and (nv < 8) {
            copy(d, V0 + nv, VR);
            nv += 1;
        } else {
            if p.ty.kind == ir.T_VEC {
                soff = (soff + 15) & ~@as(i64, 15);
            }
            let m = mc.mem(FP, -1, 0, soff);
            if c == GPR {
                _ = e2(LDR, 8, W(d), m);
            } else {
                _ = e2(FLDR, @truncate(u8, p.ty.size), W(d), m);
            }
            if p.ty.kind == ir.T_VEC {
                soff += 16;
            } else {
                soff += 8;
            }
        }
    }
    for bi in 0..order.len {
        let ib = order.get(bi);
        CB = MF.blocks.get(bi);
        let mut i = ib.first;
        while u.notnil(i) {
            if not ir.is_term(i.op) {
                gen_inst(i);
            }
            i = i.next;
        }
        gen_term(ib);
        let t = ir.terminator(ib);
        for k in 0..ir.succ_count(t) {
            CB.succs.push(blk(ir.succ(t, k)));
        }
    }
    u.free(@ptr_cast(*mut void, order.data));
    if errors {
        return;
    }
    let tgt = mc.Target.{ nclasses: 2, alloc_order: [gpr_order[..], vr_order[..]], caller_saved: caller_saved(),
                          spill_load: spill_load, spill_store: spill_store };
    mc.allocate(MF, &tgt);
    mc.drop_reloads(MF, is_label);
    encode_func(f);
}

// -----------------------------------------------------------------------------
// Encoding
// -----------------------------------------------------------------------------
struct Rl {
    off: u64,
    ty: u32,
    sym: i32,
    addend: i64,
}

struct Fx {
    off: u64,
    label: i32,
    kind: i32,   // 0: imm26, 1: imm19
}

let mut code: bn.Bytes = bn.Bytes.{ b: u.List.[u8].{ data: @ptr_from_int(*mut u8, 0), len: 0, cap: 0 } };
let mut rls: u.List.[Rl] = u.List.[Rl].{ data: @ptr_from_int(*mut Rl, 0), len: 0, cap: 0 };
let mut fxs: u.List.[Fx] = u.List.[Fx].{ data: @ptr_from_int(*mut Fx, 0), len: 0, cap: 0 };
let mut local_off: u.List.[u64] = u.List.[u64].{ data: @ptr_from_int(*mut u64, 0), len: 0, cap: 0 };
let mut saved: [18]i32 = [0; 18];
let mut nsaved: usize = 0;
let mut save_off: [18]i64 = [0; 18];
let mut frame: i64 = 0;

fn P(r: i32) i32 {
    return mc.phys(MF, r);
}

fn w32(x: u64) void {
    code.u32_(x & 0xFFFF_FFFF);
}

fn here() u64 {
    return @as(u64, code.len());
}

fn reloc(ty: u32, sym: i32, addend: i64) void {
    rls.push(Rl.{ off: here(), ty: ty, sym: sym, addend: addend });
}

// rd = x (movz / movn / movk, or orr with a logical immediate)
fn enc_movi(rd: i32, x0: i64, sf: u64) void {
    let mut x = @bit_cast(u64, x0);
    let mut nch: u64 = 4;
    let mut width: u32 = 64;
    if sf == 0 {
        x = x & 0xFFFF_FFFF;
        nch = 2;
        width = 32;
    }
    let mut nzero: u64 = 0;
    let mut nones: u64 = 0;
    for i in @as(u64, 0)..nch {
        let c = (x >> (16 * i)) & 0xffff;
        if c == 0 {
            nzero += 1;
        } else if c == 0xffff {
            nones += 1;
        }
    }
    let mut need = nch - nzero;
    let inverted = nones > nzero;
    if inverted {
        need = nch - nones;
    }
    if need > 1 {
        let e = ae.logical_imm(x, width);
        if e >= 0 {
            w32(ae.logic_imm(sf, 1, rd, 31, @bit_cast(u64, e)));
            return;
        }
    }
    if x == 0 {
        w32(ae.movw(sf, 2, rd, 0, 0));
        return;
    }
    let mut first = true;
    for i in @as(u64, 0)..nch {
        let c = (x >> (16 * i)) & 0xffff;
        if inverted {
            if (c == 0xffff) and not (first and (i == (nch - 1))) {
                continue;
            }
            if first {
                w32(ae.movw(sf, 0, rd, (~c) & 0xffff, i));
                first = false;
            } else {
                w32(ae.movw(sf, 3, rd, c, i));
            }
        } else {
            if (c == 0) and not (first and (i == (nch - 1))) {
                continue;
            }
            if first {
                w32(ae.movw(sf, 2, rd, c, i));
                first = false;
            } else {
                w32(ae.movw(sf, 3, rd, c, i));
            }
        }
    }
}

fn log2(x: u64) u64 {
    let mut k: u64 = 0;
    while (@as(u64, 1) << k) < x {
        k += 1;
    }
    return k;
}

// rd = base + index << sh
fn add_index(rd: i32, base: i32, index: i32, sh: u64) void {
    if base == SP {
        w32(ae.addsub_ext(1, false, false, rd, base, index, sh));
    } else {
        w32(ae.addsub_reg(1, false, false, rd, base, index, 0, sh));
    }
}

// rd = base + disp
fn add_disp(rd: i32, base: i32, disp: i64) void {
    if disp == 0 {
        if rd != base {
            w32(ae.addsub_imm(1, false, false, rd, base, 0, false));
        }
        return;
    }
    let sub = disp < 0;
    let mut a = disp;
    if sub {
        a = 0 - disp;
    }
    if a <= 0xfff {
        w32(ae.addsub_imm(1, sub, false, rd, base, @bit_cast(u64, a), false));
        return;
    }
    if a <= 0xff_ffff {
        w32(ae.addsub_imm(1, sub, false, rd, base, @bit_cast(u64, a) >> 12, true));
        if (a & 0xfff) != 0 {
            w32(ae.addsub_imm(1, sub, false, rd, rd, @bit_cast(u64, a) & 0xfff, false));
        }
        return;
    }
    enc_movi(X17, disp, 1);
    w32(ae.addsub_ext(1, false, false, rd, base, X17, 0));
}

struct Loc {
    base: i32,
    index: i32,
    scale: u64,
    disp: i64,
    sym: i32,
    align: u64,
}

fn resolve(o: mc.MOp) Loc {
    let mut l = Loc.{ base: -1, index: -1, scale: @as(u64, o.scale), disp: o.disp, sym: -1, align: 0 };
    if o.slot >= 0 {
        l.base = SP;
        l.disp += MF.slots.get(@int_cast(usize, o.slot)).offset;
        return l;
    }
    if o.base >= 0 {
        l.base = P(o.base);
    }
    if o.index >= 0 {
        l.index = P(o.index);
    }
    if (o.base < 0) and (o.sym >= 0) {
        l.sym = o.sym;
        if o.label > 0 {
            l.align = @as(u64, @bit_cast(u32, o.label));
        }
    }
    if l.scale == 0 {
        l.scale = 1;
    }
    return l;
}

fn lo12_reloc(bytes: u64) u32 {
    if bytes == 1 {
        return elf.R_AARCH64_LDST8_ABS_LO12_NC;
    }
    if bytes == 2 {
        return elf.R_AARCH64_LDST16_ABS_LO12_NC;
    }
    if bytes == 4 {
        return elf.R_AARCH64_LDST32_ABS_LO12_NC;
    }
    if bytes == 8 {
        return elf.R_AARCH64_LDST64_ABS_LO12_NC;
    }
    return elf.R_AARCH64_LDST128_ABS_LO12_NC;
}

// A load or store of `bytes` bytes of register rt at memory operand o.
fn ldst(size: u64, v: bool, opc: u64, bytes: u64, rt: i32, o: mc.MOp) void {
    let mut l = resolve(o);
    if l.sym >= 0 {
        if (l.index < 0) and (l.align >= bytes) and (l.disp >= 0) and ((l.disp % @bit_cast(i64, bytes)) == 0) {
            reloc(elf.R_AARCH64_ADR_PREL_PG_HI21, l.sym, l.disp);
            w32(ae.adrp(X16));
            reloc(lo12_reloc(bytes), l.sym, l.disp);
            w32(ae.ldst_uimm(size, v, opc, rt, X16, 0));
            return;
        }
        reloc(elf.R_AARCH64_ADR_PREL_PG_HI21, l.sym, l.disp);
        w32(ae.adrp(X16));
        reloc(elf.R_AARCH64_ADD_ABS_LO12_NC, l.sym, l.disp);
        w32(ae.addsub_imm(1, false, false, X16, X16, 0, false));
        l.base = X16;
        l.disp = 0;
    }
    if l.index >= 0 {
        if (l.disp == 0) and ((l.scale == 1) or (l.scale == bytes)) {
            w32(ae.ldst_reg(size, v, opc, rt, l.base, l.index, (l.scale == bytes) and (bytes > 1)));
            return;
        }
        add_index(X16, l.base, l.index, log2(l.scale));
        l.base = X16;
    }
    let d = l.disp;
    let bi = @bit_cast(i64, bytes);
    if (d >= 0) and ((d % bi) == 0) and ((d / bi) < 4096) {
        w32(ae.ldst_uimm(size, v, opc, rt, l.base, @bit_cast(u64, d / bi)));
        return;
    }
    if (d >= -256) and (d <= 255) {
        w32(ae.ldst_simm9(size, v, opc, rt, l.base, d));
        return;
    }
    enc_movi(X17, d, 1);
    w32(ae.ldst_reg(size, v, opc, rt, l.base, X17, false));
}

fn lea(rd: i32, o: mc.MOp) void {
    let l = resolve(o);
    if l.sym >= 0 {
        reloc(elf.R_AARCH64_ADR_PREL_PG_HI21, l.sym, l.disp);
        w32(ae.adrp(rd));
        reloc(elf.R_AARCH64_ADD_ABS_LO12_NC, l.sym, l.disp);
        w32(ae.addsub_imm(1, false, false, rd, rd, 0, false));
        if l.index >= 0 {
            add_index(rd, rd, l.index, log2(l.scale));
        }
        return;
    }
    if l.index >= 0 {
        if l.disp == 0 {
            add_index(rd, l.base, l.index, log2(l.scale));
            return;
        }
        add_index(X16, l.base, l.index, log2(l.scale));
        add_disp(rd, X16, l.disp);
        return;
    }
    add_disp(rd, l.base, l.disp);
}

fn fldst_size(sz: u8) u64 {
    if sz == 4 {
        return 2;
    }
    if sz == 8 {
        return 3;
    }
    return 0;
}

fn label_ref(l: i32, kind: i32) void {
    fxs.push(Fx.{ off: here(), label: l, kind: kind });
}

fn save_restore(load: bool) void {
    for k in 0..nsaved {
        let r = saved[k];
        let m = mc.mem(SP, -1, 0, save_off[k]);
        if r >= V0 {
            let mut opc: u64 = 2;
            if load {
                opc = 3;
            }
            ldst(0, true, opc, 16, r, m);
        } else {
            let mut opc: u64 = 0;
            if load {
                opc = 1;
            }
            ldst(3, false, opc, 8, r, m);
        }
    }
}

fn prologue() void {
    w32(0xA9BF_7BFD);  // stp x29, x30, [sp, #-16]!
    w32(0x9100_03FD);  // mov x29, sp
    let mut rem = frame;
    while rem > 4096 {
        w32(ae.addsub_imm(1, true, false, SP, SP, 1, true));  // sub sp, sp, #4096
        w32(ae.ldst_uimm(3, false, 0, 31, SP, 0));           // str xzr, [sp] (touch the page)
        rem -= 4096;
    }
    if rem == 4096 {
        w32(ae.addsub_imm(1, true, false, SP, SP, 1, true));
    } else if rem > 0 {
        w32(ae.addsub_imm(1, true, false, SP, SP, @bit_cast(u64, rem), false));
    }
    save_restore(false);
}

fn epilogue() void {
    save_restore(true);
    w32(0x9100_03BF);  // mov sp, x29
    w32(0xA8C1_7BFD);  // ldp x29, x30, [sp], #16
    w32(ae.RET);
}

fn enc_inst(i: *mut mc.MInst) void {
    let op = i.op;
    let sz = i.sz;
    if op == RAW {
        let mut w = @bit_cast(u64, i.aux);
        let shifts: [4]u64 = [0, 5, 16, 10];
        for k in 0..i.ops.len {
            let o = i.ops[k];
            if o.kind == mc.K_REG {
                w = w | (ae.r5(P(o.reg)) << shifts[k]);
            }
        }
        w32(w);
        return;
    }
    if op == MOV {
        let d = P(i.ops[0].reg);
        let s = P(i.ops[1].reg);
        if d != s {
            w32(ae.logic_reg(1, 1, false, d, 31, s, 0, 0));
        }
        return;
    }
    if op == FMOV {
        let d = P(i.ops[0].reg);
        let s = P(i.ops[1].reg);
        if d != s {
            w32(ae.vmov(d, s));
        }
        return;
    }
    if op == MOVI {
        let mut sf: u64 = 0;
        if sz == 8 {
            sf = 1;
        }
        enc_movi(P(i.ops[0].reg), i.ops[1].disp, sf);
        return;
    }
    if op == LDR {
        ldst(size_code(sz), false, 1, @as(u64, sz), P(i.ops[0].reg), i.ops[1]);
        return;
    }
    if op == STR {
        let mut rt: i32 = 31;
        if i.ops[1].kind == mc.K_REG {
            rt = P(i.ops[1].reg);
        }
        ldst(size_code(sz), false, 0, @as(u64, sz), rt, i.ops[0]);
        return;
    }
    if op == FLDR {
        let mut opc: u64 = 1;
        if sz == 16 {
            opc = 3;
        }
        ldst(fldst_size(sz), true, opc, @as(u64, sz), P(i.ops[0].reg), i.ops[1]);
        return;
    }
    if op == FSTR {
        let mut opc: u64 = 0;
        if sz == 16 {
            opc = 2;
        }
        ldst(fldst_size(sz), true, opc, @as(u64, sz), P(i.ops[1].reg), i.ops[0]);
        return;
    }
    if op == LEA {
        lea(P(i.ops[0].reg), i.ops[1]);
        return;
    }
    if op == ADDR {
        let d = P(i.ops[0].reg);
        let s = i.ops[1];
        reloc(elf.R_AARCH64_ADR_PREL_PG_HI21, s.sym, s.disp);
        w32(ae.adrp(d));
        reloc(elf.R_AARCH64_ADD_ABS_LO12_NC, s.sym, s.disp);
        w32(ae.addsub_imm(1, false, false, d, d, 0, false));
        return;
    }
    if op == GOT {
        let d = P(i.ops[0].reg);
        let s = i.ops[1];
        reloc(elf.R_AARCH64_ADR_GOT_PAGE, s.sym, 0);
        w32(ae.adrp(d));
        reloc(elf.R_AARCH64_LD64_GOT_LO12_NC, s.sym, 0);
        w32(ae.ldst_uimm(3, false, 1, d, d, 0));
        return;
    }
    if op == TLS {
        let d = P(i.ops[0].reg);
        let s = i.ops[1];
        w32(ae.mrs_tpidr(d));
        reloc(elf.R_AARCH64_TLSLE_ADD_TPREL_HI12, s.sym, s.disp);
        w32(ae.addsub_imm(1, false, false, d, d, 0, true));
        reloc(elf.R_AARCH64_TLSLE_ADD_TPREL_LO12_NC, s.sym, s.disp);
        w32(ae.addsub_imm(1, false, false, d, d, 0, false));
        return;
    }
    if op == B {
        label_ref(i.ops[0].label, 0);
        w32(ae.B);
        return;
    }
    if op == BCC {
        label_ref(i.ops[0].label, 1);
        w32(ae.bcond(cc64(i.cc)));
        return;
    }
    if op == CBZ {
        label_ref(i.ops[1].label, 1);
        let mut sf: u64 = 0;
        if sz == 8 {
            sf = 1;
        }
        w32(ae.cbz(sf, i.cc == 1, P(i.ops[0].reg)));
        return;
    }
    if op == LABEL {
        let l = @truncate(i32, i.aux) - LOCAL_BASE;
        while local_off.len <= @int_cast(usize, l) {
            local_off.push(0);
        }
        local_off.set(@int_cast(usize, l), here());
        return;
    }
    if op == CALL {
        let t = i.ops[0];
        if t.kind == mc.K_SYM {
            reloc(elf.R_AARCH64_CALL26, t.sym, 0);
            w32(ae.BL);
        } else {
            w32(ae.blr(P(t.reg)));
        }
        return;
    }
    if op == RET {
        epilogue();
        return;
    }
    fail(u.f1("cannot encode machine op %s", u.idec(op)));
}

fn encode_func(f: *mut ir.Func) void {
    // callee-saved registers in use
    nsaved = 0;
    for r in @as(usize, 19)..29 {
        if (MF.used_regs & (@as(u64, 1) << @int_cast(u32, r))) != 0 {
            saved[nsaved] = @int_cast(i32, r);
            nsaved += 1;
        }
    }
    for r in @as(usize, 40)..48 {
        if (MF.used_regs & (@as(u64, 1) << @int_cast(u32, r))) != 0 {
            saved[nsaved] = @int_cast(i32, r);
            nsaved += 1;
        }
    }
    // frame layout from sp: outgoing arguments, slots, saved registers
    let mut cur = @bit_cast(i64, (out_area + 15) & ~@as(u64, 15));
    for k in 0..MF.slots.len {
        let s = MF.slots.at(k);
        let a = @bit_cast(i64, s.align);
        cur = (cur + a - 1) & ~(a - 1);
        s.offset = cur;
        cur += @bit_cast(i64, s.size);
    }
    for k in 0..nsaved {
        if saved[k] >= V0 {
            cur = (cur + 15) & ~@as(i64, 15);
            save_off[k] = cur;
            cur += 16;
        } else {
            cur = (cur + 7) & ~@as(i64, 7);
            save_off[k] = cur;
            cur += 8;
        }
    }
    frame = (cur + 15) & ~@as(i64, 15);
    code = bn.new_bytes();
    rls.clear();
    fxs.clear();
    local_off.clear();
    prologue();
    let nb = MF.blocks.len;
    let boff = u.newn.[u64](nb + 1);
    for bi in 0..nb {
        let b = MF.blocks.get(bi);
        boff[bi] = here();
        let mut i = b.first;
        while u.notnil(i) {
            enc_inst(i);
            i = i.next;
        }
    }
    // branch targets
    let cv = code.view();
    for k in 0..fxs.len {
        let x = fxs.get(k);
        let mut target: u64 = 0;
        if x.label >= LOCAL_BASE {
            target = local_off.get(@int_cast(usize, x.label - LOCAL_BASE));
        } else {
            target = boff[@int_cast(usize, x.label)];
        }
        let rel = (@bit_cast(i64, target) - @bit_cast(i64, x.off)) / 4;
        let o = @truncate(usize, x.off);
        let mut w = @as(u64, cv[o]) | (@as(u64, cv[o + 1]) << 8) | (@as(u64, cv[o + 2]) << 16) | (@as(u64, cv[o + 3]) << 24);
        if x.kind == 0 {
            w = w | (@bit_cast(u64, rel) & 0x3ff_ffff);
        } else {
            if (rel < -0x4_0000) or (rel >= 0x4_0000) {
                fail("conditional branch out of range (function too large)");
            }
            w = w | ((@bit_cast(u64, rel) & 0x7_ffff) << 5);
        }
        code.set_u32(o, w);
    }
    let text = OBJ.secs.get(@int_cast(usize, TEXT));
    text.data.align(16);
    let base = @as(u64, text.data.len());
    text.data.put(code.view());
    for k in 0..rls.len {
        let r = rls.get(k);
        text.relocs.push(elf.Rel.{ off: base + r.off, ty: r.ty, sym: @bit_cast(u32, r.sym), addend: r.addend });
    }
    let s = OBJ.syms.at(@as(usize, fsym.get(@int_cast(usize, f.id))));
    s.value = base;
    s.size = @as(u64, code.len());
    code.free();
}

// -----------------------------------------------------------------------------
// Module
// -----------------------------------------------------------------------------
fn bind_of(l: i32) u8 {
    if l == ir.LINK_INTERNAL {
        return elf.B_LOCAL;
    }
    if (l == ir.LINK_LINKONCE) or (l == ir.LINK_WEAK) or (l == ir.LINK_EXTWEAK) {
        return elf.B_WEAK;
    }
    return elf.B_GLOBAL;
}

fn emit_data(s: *mut elf.Sec, base: u64, x: *mut ir.Value) void {
    let t = x.ty;
    let k = x.kind;
    if k == ir.V_GLOBAL {
        s.relocs.push(elf.Rel.{ off: base, ty: elf.R_AARCH64_ABS64, sym: gsym.get(@int_cast(usize, x.gv.id)), addend: 0 });
        return;
    }
    if k == ir.V_FUNC {
        s.relocs.push(elf.Rel.{ off: base, ty: elf.R_AARCH64_ABS64, sym: fsym.get(@int_cast(usize, x.fnp.id)), addend: 0 });
        return;
    }
    if (k == ir.V_CAGG) and (t.kind != ir.T_VEC) {
        for i in 0..x.xs.len {
            emit_data(s, base + ir.field_off(t, i), x.xs.get(i));
        }
        return;
    }
    let mut b = bn.new_bytes();
    const_le(x, &mut b, 0);
    let v = s.data.view();
    for i in 0..b.len() {
        v[@truncate(usize, base) + i] = b.view()[i];
    }
    b.free();
}

fn has_addresses(x: *ir.Value) bool {
    if (x.kind == ir.V_GLOBAL) or (x.kind == ir.V_FUNC) {
        return true;
    }
    if x.kind == ir.V_CAGG {
        for i in 0..x.xs.len {
            if has_addresses(x.xs.get(i)) {
                return true;
            }
        }
    }
    return false;
}

pub fn compile(m: *mut ir.Module) *mut elf.Obj {
    errors = false;
    LSE_ = tg.has(tg.A_LSE);
    OBJ = elf.new_obj(elf.EM_AARCH64);
    consts = u.SymMap.[u32].{ keys: u.nil.[usize](), vals: u.nil.[u32](), cap: 0, len: 0 };
    libsyms = u.SymMap.[u32].{ keys: u.nil.[usize](), vals: u.nil.[u32](), cap: 0, len: 0 };
    TEXT = elf.add_sec(OBJ, ".text", elf.S_TEXT, 16);
    RODATA = elf.add_sec(OBJ, ".rodata", elf.S_RODATA, 16);
    let data = elf.add_sec(OBJ, ".data", elf.S_DATA, 16);
    let bss = elf.add_sec(OBJ, ".bss", elf.S_BSS, 16);
    let tbss = elf.add_sec(OBJ, ".tbss", elf.S_TBSS, 16);
    let tdata = elf.add_sec(OBJ, ".tdata", elf.S_TDATA, 16);
    let relro = elf.add_sec(OBJ, ".data.rel.ro", elf.S_DATA, 16);
    fsym.clear();
    gsym.clear();
    galign.clear();
    for i in 0..m.funcs.len {
        let f = m.funcs.get(i);
        if ir.is_decl(f) {
            fsym.push(elf.ext_sym(OBJ, f.name, f.linkage == ir.LINK_EXTWEAK));
        } else {
            fsym.push(elf.add_sym(OBJ, elf.Sym.{ name: f.name, sec: TEXT, value: 0, size: 0, bind: bind_of(f.linkage),
                                                 ty: elf.T_FUNC, hidden: false }));
        }
    }
    for i in 0..m.globals.len {
        let g = m.globals.get(i);
        let mut al = g.align;
        if al == 0 {
            al = 1;
        }
        galign.push(al);
        if u.isnil(g.init) {
            let si = elf.ext_sym(OBJ, g.name, false);
            if g.tls {
                OBJ.syms.at(@as(usize, si)).ty = elf.T_TLS;
            }
            gsym.push(si);
            continue;
        }
        let mut sec = data;
        if g.tls {
            sec = tbss;
            if not ir.is_zero(g.init) {
                sec = tdata;
            }
        } else if g.is_const {
            sec = RODATA;
            if has_addresses(g.init) {
                sec = relro;
            }
        } else if ir.is_zero(g.init) {
            sec = bss;
        }
        let s = OBJ.secs.get(@int_cast(usize, sec));
        if al > s.align {
            s.align = al;  // the section is at least as aligned as its contents
        }
        let mut off: u64 = 0;
        if (sec == bss) or (sec == tbss) {
            s.size = (s.size + al - 1) & ~(al - 1);
            off = s.size;
            s.size += g.ty.size;
        } else {
            s.data.align(@truncate(usize, al));
            off = @as(u64, s.data.len());
            s.data.zeros(@truncate(usize, g.ty.size));
        }
        let mut ty = elf.T_OBJECT;
        if g.tls {
            ty = elf.T_TLS;
        }
        gsym.push(elf.add_sym(OBJ, elf.Sym.{ name: g.name, sec: sec, value: off, size: g.ty.size, bind: bind_of(g.linkage),
                                             ty: ty, hidden: false }));
    }
    for i in 0..m.globals.len {
        let g = m.globals.get(i);
        if u.isnil(g.init) {
            continue;
        }
        let s = OBJ.syms.at(@as(usize, gsym.get(i)));
        let sec = OBJ.secs.get(@int_cast(usize, s.sec));
        if (sec.kind == elf.S_BSS) or (sec.kind == elf.S_TBSS) {
            continue;
        }
        emit_data(sec, s.value, g.init);
    }
    for i in 0..m.funcs.len {
        let f = m.funcs.get(i);
        if not ir.is_decl(f) {
            compile_func(f);
            if errors {
                return u.nil.[elf.Obj]();
            }
        }
    }
    return OBJ;
}
