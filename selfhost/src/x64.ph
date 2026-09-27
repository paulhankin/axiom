// =============================================================================
// x64.ph - x86-64 code generation (System V ABI, Linux): instruction
// selection from the scalar IR into machine IR (mach.ph), register
// allocation, frame layout, and encoding (x64enc.ph) into an ELF object.
//
// Integers narrower than 32 bits live in 32-bit registers with unspecified
// upper bits (i1 is always 0 or 1); instructions that depend on the upper
// bits extend first. Every function keeps a frame pointer (rbp); stack
// slots are addressed from it.
// =============================================================================
import "util.ph" as u;
import "ir.ph" as ir;
import "opt.ph" as opt;
import "mach.ph" as mc;
import "x64enc.ph" as xe;
import "elf.ph" as elf;
import "bin.ph" as bn;
import "target.ph" as tg;
import "lower.ph" as lower;

const GPR: u8 = 0;
const XMM: u8 = 1;

let mut OBJ: *mut elf.Obj = @ptr_from_int(*mut elf.Obj, 0);
let mut TEXT: i32 = 0;
let mut RODATA: i32 = 0;
let mut fsym: u.List.[u32] = u.List.[u32].{ data: @ptr_from_int(*mut u32, 0), len: 0, cap: 0 };
let mut gsym: u.List.[u32] = u.List.[u32].{ data: @ptr_from_int(*mut u32, 0), len: 0, cap: 0 };
let mut consts: u.SymMap.[u32] = u.SymMap.[u32].{
    keys: @ptr_from_int(*mut usize, 0), vals: @ptr_from_int(*mut u32, 0), cap: 0, len: 0 };
let mut errors: bool = false;
let mut AVX: bool = false;

let mut F: *mut ir.Func = @ptr_from_int(*mut ir.Func, 0);
let mut MF: *mut mc.MFunc = @ptr_from_int(*mut mc.MFunc, 0);
let mut CB: *mut mc.MBlock = @ptr_from_int(*mut mc.MBlock, 0);
let mut nlocal: i32 = 0;
let mut out_area: u64 = 0;   // outgoing stack-argument bytes

fn fail(msg: []u8) void {
    u.eprint(u.f2("phalange: x86-64 code generation: %s (in %s)\n", msg, F.name));
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

fn e3(op: i32, sz: u8, a: mc.MOp, b: mc.MOp, c: mc.MOp) *mut mc.MInst {
    let ops: [3]mc.MOp = [a, b, c];
    return emit(op, sz, ops[..]);
}

fn e4(op: i32, sz: u8, a: mc.MOp, b: mc.MOp, c: mc.MOp, d: mc.MOp) *mut mc.MInst {
    let ops: [4]mc.MOp = [a, b, c, d];
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

fn vgpr() i32 {
    return mc.new_vreg(MF, GPR);
}

fn vxmm() i32 {
    return mc.new_vreg(MF, XMM);
}

fn class_of(t: *ir.Ty) u8 {
    if (t.kind == ir.T_FLOAT) or (t.kind == ir.T_VEC) {
        return XMM;
    }
    return GPR;
}

fn isz(t: *ir.Ty) u8 {
    if (t.kind == ir.T_PTR) or ((t.kind == ir.T_INT) and (t.bits > 32)) {
        return 8;
    }
    return 4;
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
        let i = e2(xe.MOV_RR, 8, W(d), R(s));
        i.is_move = true;
    } else {
        let i = sse2(xe.MOVAPS, W(d), R(s));
        i.is_move = true;
    }
    hint(d, s);
    hint(s, d);
}

fn sse2(id: i32, a: mc.MOp, b: mc.MOp) *mut mc.MInst {
    let i = e4(xe.SSE, 0, a, b, NONE(), NONE());
    i.cc = id;
    return i;
}

fn sse_i(id: i32, a: mc.MOp, b: mc.MOp, x: i64) *mut mc.MInst {
    let i = e4(xe.SSE, 0, a, b, NONE(), I(x));
    i.cc = id;
    return i;
}

// d = a op b for a two-operand SSE instruction: VEX 3-operand with AVX,
// otherwise a copy of a into d first.
fn sse3(id: i32, d: i32, a: i32, b: mc.MOp) void {
    if AVX {
        let i = e4(xe.SSE, 0, W(d), b, R(a), NONE());
        i.cc = id;
        return;
    }
    // d = a; d op= b  (b must not be d: use a temporary if it is)
    if (b.kind == mc.K_REG) and (b.reg == d) and (a != d) {
        let t = vxmm();
        copy(t, a, XMM);
        let i = e4(xe.SSE, 0, RW(t), b, NONE(), NONE());
        i.cc = id;
        copy(d, t, XMM);
        return;
    }
    copy(d, a, XMM);
    let i = e4(xe.SSE, 0, RW(d), b, NONE(), NONE());
    i.cc = id;
}

fn sse3_i(id: i32, d: i32, a: i32, b: mc.MOp, x: i64) void {
    if AVX {
        let i = e4(xe.SSE, 0, W(d), b, R(a), I(x));
        i.cc = id;
        return;
    }
    copy(d, a, XMM);
    let i = e4(xe.SSE, 0, RW(d), b, NONE(), I(x));
    i.cc = id;
}

// A local label within the current block.
fn new_label() i32 {
    nlocal += 1;
    return xe.LOCAL_BASE + nlocal;
}

fn place(l: i32) void {
    let i = e0(xe.LABEL, 0);
    i.aux = @as(i64, l);
}

fn jcc(cc: i32, l: i32) void {
    let i = e1(xe.JCC, 0, mc.label(l));
    i.cc = cc;
}

fn jmp(l: i32) void {
    _ = e1(xe.JMP, 0, mc.label(l));
}

// -----------------------------------------------------------------------------
// Symbols and constants
// -----------------------------------------------------------------------------
fn func_sym(f: *ir.Func) i32 {
    return @as(i32, @bit_cast(i32, fsym.get(@int_cast(usize, f.id))));
}

fn global_sym(g: *ir.Global) i32 {
    return @as(i32, @bit_cast(i32, gsym.get(@int_cast(usize, g.id))));
}

// A 16-byte constant in the constant pool (rodata), as a memory operand.
fn const16(lo: u64, hi: u64) mc.MOp {
    let key = u.intern(u.f2("%s:%s", u.hex16(lo), u.hex16(hi)));
    let found = consts.get(key);
    if found != 0 {
        return mc.mem_sym(@as(i32, @bit_cast(i32, found - 1)), 0);
    }
    let s = OBJ.secs.get(@int_cast(usize, RODATA));
    s.data.align(16);
    let off = @as(u64, s.data.len());
    s.data.u64_(lo);
    s.data.u64_(hi);
    let sym = elf.add_sym(OBJ, elf.Sym.{ name: ".LC", sec: RODATA, value: off, size: 16, bind: elf.B_LOCAL,
                                         ty: elf.T_OBJECT, hidden: false });
    consts.put(key, sym + 1);
    return mc.mem_sym(@as(i32, @bit_cast(i32, sym)), 0);
}

fn fits32(x: i64) bool {
    return (x >= -2147483648) and (x <= 2147483647);
}

// -----------------------------------------------------------------------------
// Values
// -----------------------------------------------------------------------------
fn is_inst(v: *ir.Value, op: i32) bool {
    return (v.kind == ir.V_INST) and (v.op == op);
}

// Constant integer value (sign-extended), if v is one.
fn cval(v: *ir.Value) i64 {
    if v.kind == ir.V_CINT {
        return ir.sval(v);
    }
    return 0;
}

fn is_c(v: *ir.Value) bool {
    return (v.kind == ir.V_CINT) or ((v.kind == ir.V_ZERO) and ((v.ty.kind == ir.T_INT) or (v.ty.kind == ir.T_PTR)));
}

// The value as a 32-bit immediate, when it is an integer constant that fits.
fn imm_ok(v: *ir.Value) bool {
    return is_c(v) and fits32(cval(v));
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
        // folded into memory operands where it is used; recomputed here
        let r = vgpr();
        _ = e2(xe.LEA, 8, W(r), addr(v));
        return r;
    }
    if (k == ir.V_INST) or (k == ir.V_PARAM) {
        return vreg(v);
    }
    let t = v.ty;
    if (k == ir.V_GLOBAL) {
        let r = vgpr();
        let g = v.gv;
        if g.tls {
            _ = e1(xe.TLSBASE, 8, W(r));
            let mut m = mc.mem(r, -1, 0, 0);
            m.sym = global_sym(g);
            m.slot = -2;  // marks a TLS offset
            _ = e2(xe.LEA, 8, W(r), m);
            return r;
        }
        _ = e2(xe.LEA, 8, W(r), mc.mem_sym(global_sym(g), 0));
        return r;
    }
    if k == ir.V_FUNC {
        let r = vgpr();
        let f = v.fnp;
        if ir.is_decl(f) {
            // external: its address comes from the GOT
            let mut m = mc.mem_sym(func_sym(f), 0);
            m.scale = 99;  // GOT marker
            _ = e2(xe.LOAD, 8, W(r), m);
            return r;
        }
        _ = e2(xe.LEA, 8, W(r), mc.mem_sym(func_sym(f), 0));
        return r;
    }
    if class_of(t) == XMM {
        let r = vxmm();
        if ir.is_zero(v) or (k == ir.V_UNDEF) {
            sse3(xe.PXOR, r, r, R(r));
            return r;
        }
        let mut b = bn.new_bytes();
        const_le(v, &mut b, 0);
        while b.len() < 16 {
            b.u8_(0);
        }
        let lo = b.get_u64(0);
        let mut hi: u64 = 0;
        if b.len() >= 16 {
            hi = b.get_u64(8);
        }
        b.free();
        let m = const16(lo, hi);
        if t.kind == ir.T_VEC {
            _ = sse2(xe.MOVDQU_LD, W(r), m);
        } else if t.bits == 32 {
            _ = sse2(xe.MOVSS_LD, W(r), m);
        } else {
            _ = sse2(xe.MOVSD_LD, W(r), m);
        }
        return r;
    }
    // integer / pointer constant
    let r = vgpr();
    let x = cval(v);
    if (k == ir.V_UNDEF) or (k == ir.V_ZERO) {
        _ = e2(xe.MOV_RI, 4, W(r), I(0));
    } else if (x >= 0) and (x <= 0xffff_ffff) {
        _ = e2(xe.MOV_RI, 4, W(r), I(x));
    } else {
        _ = e2(xe.MOV_RI, 8, W(r), I(x));
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

// A memory operand for address p.
fn addr(p: *mut ir.Value) mc.MOp {
    if is_inst(p, ir.OP_ALLOCA) {
        return mc.mem_slot(@truncate(i32, p.aux), 0);
    }
    if is_inst(p, ir.OP_PTRADD) {
        let off = p.b;
        if imm_ok(off) {
            let mut m = addr(p.a);
            let nd = m.disp + cval(off);
            if fits32(nd) and (m.scale != 99) and (m.slot != -2) {
                m.disp = nd;
                return m;
            }
        }
        // base + index * scale
        if (p.flag & 1) != 0 {
            let b = reg(p.a);
            if is_inst(off, ir.OP_MUL) and is_c(off.b) {
                let s = cval(off.b);
                if (s == 1) or (s == 2) or (s == 4) or (s == 8) {
                    return mc.mem(b, reg(off.a), @truncate(u8, @bit_cast(u64, s)), 0);
                }
            }
            return mc.mem(b, reg(off), 1, 0);
        }
    }
    if (p.kind == ir.V_GLOBAL) and not p.gv.tls {
        return mc.mem_sym(global_sym(p.gv), 0);
    }
    return mc.mem(reg(p), -1, 0, 0);
}

// -----------------------------------------------------------------------------
// Integer operations
// -----------------------------------------------------------------------------
fn alu_digit(op: i32) i64 {
    if op == ir.OP_ADD {
        return 0;
    }
    if op == ir.OP_OR {
        return 1;
    }
    if op == ir.OP_AND {
        return 4;
    }
    if op == ir.OP_SUB {
        return 5;
    }
    return 6;  // xor
}

// d = a op b (add, sub, and, or, xor, mul)
fn gen_alu(v: *mut ir.Value) void {
    let o = v.op;
    let sz = isz(v.ty);
    let mut a = v.a;
    let mut b = v.b;
    if is_c(a) and not is_c(b) and (o != ir.OP_SUB) {
        a = v.b;
        b = v.a;
    }
    let d = vreg(v);
    let ra = reg(a);
    copy(d, ra, GPR);
    hint(d, ra);
    if o == ir.OP_MUL {
        if imm_ok(b) {
            let i = e3(xe.IMUL_I, sz, W(d), R(d), I(cval(b)));
            _ = i;
        } else {
            _ = e2(xe.IMUL, sz, RW(d), R(reg(b)));
        }
    } else {
        let i = if_imm_alu(sz, d, b);
        i.cc = @truncate(i32, alu_digit(o));
    }
    if (v.ty.bits == 1) and (o != ir.OP_AND) and (o != ir.OP_OR) and (o != ir.OP_XOR) {
        let i = e2(xe.ALU, 4, RW(d), I(1));
        i.cc = 4;
    }
}

fn if_imm_alu(sz: u8, d: i32, b: *mut ir.Value) *mut mc.MInst {
    if imm_ok(b) {
        return e2(xe.ALU, sz, RW(d), I(cval(b)));
    }
    return e2(xe.ALU, sz, RW(d), R(reg(b)));
}

// Extends the narrow integer in r (bits wide) into a fresh 32/64-bit register.
fn ext_reg(r: i32, bits: i32, sign: bool, to64: bool) i32 {
    let d = vgpr();
    let mut sz: u8 = 4;
    if to64 and sign {
        sz = 8;
    }
    if bits == 1 {
        // i1 is 0 or 1: sign extension is negation
        copy(d, r, GPR);
        if sign {
            let i = e1(xe.UNARY, sz, RW(d));
            i.cc = 3;
        }
        return d;
    }
    let i = e2(xe.EXT, sz, W(d), R(r));
    i.cc = ext_kind(bits, sign);
    return d;
}

fn ext_kind(bits: i32, sign: bool) i32 {
    if bits == 8 {
        if sign {
            return 3;
        }
        return 1;
    }
    if bits == 16 {
        if sign {
            return 4;
        }
        return 2;
    }
    if bits == 32 {
        if sign {
            return 5;
        }
        return 6;
    }
    return 0;
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
    let sz = isz(v.ty);
    let a = reg_ext(v.a, sg);
    let b = reg_ext(v.b, sg);
    _ = e2(xe.MOV_RR, 8, W(xe.RAX), R(a));
    if sg {
        _ = e2(xe.CQO, sz, W(xe.RDX), R(xe.RAX));
    } else {
        _ = e2(xe.MOV_RI, 4, W(xe.RDX), I(0));
    }
    let i = e4(xe.DIV, sz, RW(xe.RAX), RW(xe.RDX), R(b), NONE());
    i.cc = 6;
    if sg {
        i.cc = 7;
    }
    let d = vreg(v);
    if (o == ir.OP_UDIV) or (o == ir.OP_SDIV) {
        copy(d, xe.RAX, GPR);
    } else {
        copy(d, xe.RDX, GPR);
    }
}

fn gen_shift(v: *mut ir.Value) void {
    let o = v.op;
    let t = v.ty;
    let sz = isz(t);
    let bits = t.bits;
    let d = vreg(v);
    // the value, extended as the shift needs
    let mut a = reg(v.a);
    if (bits < 32) and (bits > 1) {
        if o == ir.OP_LSHR {
            a = ext_reg(a, bits, false, false);
        } else if o == ir.OP_ASHR {
            a = ext_reg(a, bits, true, false);
        }
    }
    let mut digit: i32 = 4;
    if o == ir.OP_LSHR {
        digit = 5;
    } else if o == ir.OP_ASHR {
        digit = 7;
    }
    if is_c(v.b) {
        copy(d, a, GPR);
        let k = cval(v.b) & @as(i64, (sz * 8) - 1);
        let i = e3(xe.SHIFT, sz, RW(d), NONE(), I(k));
        i.cc = digit;
        return;
    }
    let mut c = reg(v.b);
    if (v.b.ty.bits < 32) and (bits < 32) {
        let m = vgpr();
        copy(m, c, GPR);
        let i = e2(xe.ALU, 4, RW(m), I(@as(i64, bits - 1)));
        i.cc = 4;
        c = m;
    }
    if tg.has(tg.X_BMI2) {
        let i = e3(xe.SHIFTX, sz, W(d), R(a), R(c));
        if o == ir.OP_SHL {
            i.cc = 0;
        } else if o == ir.OP_LSHR {
            i.cc = 1;
        } else {
            i.cc = 2;
        }
        return;
    }
    copy(d, a, GPR);
    _ = e2(xe.MOV_RR, 8, W(xe.RCX), R(c));
    let i = e3(xe.SHIFT, sz, RW(d), R(xe.RCX), NONE());
    i.cc = digit;
}

fn icc(p: i32) i32 {
    if p == ir.IntEQ {
        return xe.CC_E;
    }
    if p == ir.IntNE {
        return xe.CC_NE;
    }
    if p == ir.IntULT {
        return xe.CC_B;
    }
    if p == ir.IntULE {
        return xe.CC_BE;
    }
    if p == ir.IntUGT {
        return xe.CC_A;
    }
    if p == ir.IntUGE {
        return xe.CC_AE;
    }
    if p == ir.IntSLT {
        return xe.CC_L;
    }
    if p == ir.IntSLE {
        return xe.CC_LE;
    }
    if p == ir.IntSGT {
        return xe.CC_G;
    }
    return xe.CC_GE;
}

// Emits the comparison for an icmp and returns the condition code.
fn int_compare(v: *mut ir.Value) i32 {
    let p = @truncate(i32, @bit_cast(i64, v.cv));
    let t = v.a.ty;
    let mut sz = isz(t);
    if t.kind == ir.T_INT {
        if t.bits == 8 {
            sz = 1;
        } else if t.bits == 16 {
            sz = 2;
        }
    }
    let a = reg(v.a);
    if imm_ok(v.b) {
        let mut x = cval(v.b);
        if sz == 1 {
            x = @as(i64, @truncate(i8, x));
        } else if sz == 2 {
            x = @as(i64, @truncate(i16, x));
        }
        let i = e2(xe.ALU, sz, R(a), I(x));
        i.cc = 7;
    } else {
        let i = e2(xe.ALU, sz, R(a), R(reg(v.b)));
        i.cc = 7;
    }
    return icc(p);
}

// ucomis* for an fcmp; returns the condition code, or -1 for OEQ (E and NP)
// and -2 for UNE (NE or P).
fn float_compare(v: *mut ir.Value) i32 {
    let p = @truncate(i32, @bit_cast(i64, v.cv));
    let mut op = xe.UCOMISD;
    if v.a.ty.bits == 32 {
        op = xe.UCOMISS;
    }
    let a = reg(v.a);
    let b = reg(v.b);
    if (p == ir.RealOLT) or (p == ir.RealOLE) {
        _ = sse2(op, R(b), R(a));
        if p == ir.RealOLT {
            return xe.CC_A;
        }
        return xe.CC_AE;
    }
    _ = sse2(op, R(a), R(b));
    if p == ir.RealOGT {
        return xe.CC_A;
    }
    if p == ir.RealOGE {
        return xe.CC_AE;
    }
    if p == ir.RealOEQ {
        return -1;
    }
    return -2;
}

fn setcc(d: i32, cc: i32) void {
    let i = e1(xe.SETCC, 1, W(d));
    i.cc = cc;
}

// d (32-bit, 0/1) = condition
fn materialize_cc(d: i32, cc: i32) void {
    if cc >= 0 {
        setcc(d, cc);
    } else if cc == -1 {
        let t = vgpr();
        setcc(d, xe.CC_E);
        setcc(t, xe.CC_NP);
        let i = e2(xe.ALU, 1, RW(d), R(t));
        i.cc = 4;
    } else {
        let t = vgpr();
        setcc(d, xe.CC_NE);
        setcc(t, xe.CC_P);
        let i = e2(xe.ALU, 1, RW(d), R(t));
        i.cc = 1;
    }
    let z = e2(xe.EXT, 4, W(d), R(d));
    z.cc = 1;
}

fn invert(cc: i32) i32 {
    return cc ^ 1;
}

// Is v a comparison fused into its only user (branch or select)?
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
    _ = e2(xe.TEST, 4, R(r), R(r));
    return xe.CC_NE;
}

fn gen_select(v: *mut ir.Value) void {
    let d = vreg(v);
    let cls = class_of(v.ty);
    if cls == GPR {
        let rt = reg(v.b);
        let rf = reg(v.c);
        copy(d, rf, GPR);
        let cc = cond_code(v.a);
        if cc >= 0 {
            let i = e2(xe.CMOV, isz(v.ty), RW(d), R(rt));
            i.cc = cc;
        } else {
            // OEQ: cmov if E and NP; UNE: if NE or P
            let skip = new_label();
            if cc == -1 {
                jcc(xe.CC_P, skip);
                let i = e2(xe.CMOV, isz(v.ty), RW(d), R(rt));
                i.cc = xe.CC_E;
            } else {
                let i = e2(xe.CMOV, isz(v.ty), RW(d), R(rt));
                i.cc = xe.CC_NE;
                let j = e2(xe.CMOV, isz(v.ty), RW(d), R(rt));
                j.cc = xe.CC_P;
            }
            place(skip);
        }
        return;
    }
    // floats and vectors: a branch around the move
    let rt = reg(v.b);
    let rf = reg(v.c);
    copy(d, rf, XMM);
    let cc = cond_code(v.a);
    let skip = new_label();
    if cc >= 0 {
        jcc(invert(cc), skip);
    } else if cc == -1 {
        jcc(xe.CC_NE, skip);
        jcc(xe.CC_P, skip);
    } else {
        let take = new_label();
        jcc(xe.CC_NE, take);
        jcc(xe.CC_NP, skip);
        place(take);
    }
    let m = sse2(xe.MOVAPS, RW(d), R(rt));
    m.ops[0].role = mc.RUD;
    place(skip);
}

fn gen_icmp(v: *mut ir.Value) void {
    if v.a.ty.kind == ir.T_VEC {
        vec_icmp(v);
        return;
    }
    let d = vreg(v);
    let cc = int_compare(v);
    materialize_cc(d, cc);
}

fn gen_fcmp(v: *mut ir.Value) void {
    if v.a.ty.kind == ir.T_VEC {
        vec_fcmp(v);
        return;
    }
    let d = vreg(v);
    let cc = float_compare(v);
    materialize_cc(d, cc);
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
            copy(d, reg(v.a), XMM);  // lane masks are already all ones / zeros
            return;
        }
        let sg = o == ir.OP_SEXT;
        let r = reg(v.a);
        let fb = from.bits;
        let to64 = to.bits > 32;
        if fb == 32 {
            let i = e2(xe.EXT, u.pick(sg, "\x08", "\x04")[0], W(d), R(r));
            i.cc = ext_kind(32, sg);
            return;
        }
        let x = ext_reg(r, fb, sg, to64);
        copy(d, x, GPR);
        return;
    }
    if o == ir.OP_TRUNC {
        let r = reg(v.a);
        copy(d, r, GPR);
        if to.bits == 1 {
            let i = e2(xe.ALU, 4, RW(d), I(1));
            i.cc = 4;
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
        } else if tc == XMM {
            if from.bits == 64 {
                _ = sse2(xe.MOVQ_XR, W(d), R(r));
            } else {
                _ = sse2(xe.MOVD_XR, W(d), R(r));
            }
        } else {
            if to.bits == 64 {
                _ = sse2(xe.MOVQ_RX, R(r), W(d));
            } else {
                _ = sse2(xe.MOVD_RX, R(r), W(d));
            }
        }
        return;
    }
    if o == ir.OP_FPEXT {
        let r = reg(v.a);
        sse_unary(xe.CVTSS2SD, d, R(r));
        return;
    }
    if o == ir.OP_FPTRUNC {
        let r = reg(v.a);
        sse_unary(xe.CVTSD2SS, d, R(r));
        return;
    }
    if (o == ir.OP_SITOFP) or (o == ir.OP_UITOFP) {
        gen_int_to_float(v, d);
        return;
    }
    gen_float_to_int(v, d);
}

// Unary SSE op d = op(s) (scalar forms merge into d's upper lanes, which we
// do not care about).
fn sse_unary(id: i32, d: i32, s: mc.MOp) void {
    if AVX {
        let mut vv = d;
        if s.kind == mc.K_REG {
            vv = s.reg;
        }
        let i = e4(xe.SSE, 0, W(d), s, R(vv), NONE());
        i.cc = id;
        return;
    }
    _ = sse2(id, W(d), s);
}

fn cvt_si2f(d: i32, r: i32, is64int: bool, single: bool) void {
    let mut id = xe.CVTSI2SD;
    if single {
        id = xe.CVTSI2SS;
    }
    let i = sse2(id, W(d), R(r));
    if is64int {
        i.sz = 8;
    }
    if AVX {
        i.ops[2] = R(d);
        i.ops[0].role = mc.RUD;
    }
}

fn gen_int_to_float(v: *mut ir.Value, d: i32) void {
    let sg = v.op == ir.OP_SITOFP;
    let from = v.a.ty;
    let single = v.ty.bits == 32;
    let mut r = reg(v.a);
    let fb = from.bits;
    if fb < 32 {
        r = ext_reg(r, fb, sg, false);
    }
    if sg {
        cvt_si2f(d, r, fb == 64, single);
        return;
    }
    if fb <= 32 {
        // zero-extended to 64 bits, then a signed 64-bit conversion
        let z = vgpr();
        let i = e2(xe.EXT, 4, W(z), R(r));
        i.cc = 6;
        cvt_si2f(d, z, true, single);
        return;
    }
    // u64: if the top bit is set, halve (keeping the low bit), convert, double
    let big = new_label();
    let done = new_label();
    _ = e2(xe.TEST, 8, R(r), R(r));
    jcc(xe.CC_S, big);
    cvt_si2f(d, r, true, single);
    jmp(done);
    place(big);
    let h = vgpr();
    let l = vgpr();
    copy(h, r, GPR);
    let s1 = e3(xe.SHIFT, 8, RW(h), NONE(), I(1));
    s1.cc = 5;
    copy(l, r, GPR);
    let a1 = e2(xe.ALU, 8, RW(l), I(1));
    a1.cc = 4;
    let o1 = e2(xe.ALU, 8, RW(h), R(l));
    o1.cc = 1;
    cvt_si2f(d, h, true, single);
    let mut add = xe.ADDSD;
    if single {
        add = xe.ADDSS;
    }
    sse3(add, d, d, R(d));
    place(done);
}

// Saturating float -> int (NaN -> 0, out of range -> the nearest bound).
fn gen_float_to_int(v: *mut ir.Value, d: i32) void {
    let sg = v.op == ir.OP_FPTOSI_SAT;
    let x = reg(v.a);
    let single = v.a.ty.bits == 32;
    let tb = v.ty.bits;
    let mut cvt = xe.CVTTSD2SI;
    let mut cmp = xe.UCOMISD;
    if single {
        cvt = xe.CVTTSS2SI;
        cmp = xe.UCOMISS;
    }
    // bounds as floats: lo (inclusive, representable) and hi (exclusive: 2^k)
    let mut lo: f64 = 0.0;
    let mut hi: f64 = 0.0;
    let mut lo_i: i64 = 0;
    let mut hi_i: i64 = 0;
    if sg {
        let k = @int_cast(u32, tb - 1);
        hi = pow2f(k);
        lo = -hi;
        if tb == 64 {
            lo_i = -9223372036854775807 - 1;
        } else {
            lo_i = -(@as(i64, 1) << k);
        }
        if tb == 64 {
            hi_i = 9223372036854775807;
        } else {
            hi_i = (@as(i64, 1) << k) - 1;
        }
    } else {
        let k = @int_cast(u32, tb);
        hi = pow2f(k);
        lo = 0.0;
        lo_i = 0;
        if tb == 64 {
            hi_i = -1;
        } else {
            hi_i = (@as(i64, 1) << k) - 1;
        }
    }
    let done = new_label();
    let not_nan = new_label();
    let not_low = new_label();
    let in_range = new_label();
    let out64 = tb > 32;
    let osz: u8 = u.pick(out64, "\x08", "\x04")[0];
    // NaN -> 0
    _ = sse2(cmp, R(x), R(x));
    jcc(xe.CC_NP, not_nan);
    _ = e2(xe.MOV_RI, 4, W(d), I(0));
    jmp(done);
    place(not_nan);
    // x < lo -> lo_i  (for unsigned: x <= -1 ... use x < 0 via compare with 0)
    let rlo = fconst_reg(lo, single);
    _ = sse2(cmp, R(rlo), R(x));
    jcc(xe.CC_BE, not_low);   // lo <= x
    _ = e2(xe.MOV_RI, osz, W(d), I(lo_i));
    jmp(done);
    place(not_low);
    let rhi = fconst_reg(hi, single);
    _ = sse2(cmp, R(x), R(rhi));
    jcc(xe.CC_B, in_range);   // x < hi
    _ = e2(xe.MOV_RI, osz, W(d), I(hi_i));
    jmp(done);
    place(in_range);
    if sg or (tb < 32) {
        let i = sse2(cvt, W(d), R(x));
        if out64 {
            i.sz = 8;
        }
    } else if tb == 32 {
        // u32: through a 64-bit signed conversion
        let i = sse2(cvt, W(d), R(x));
        i.sz = 8;
    } else {
        // u64: values >= 2^63 are converted after subtracting 2^63
        let small = new_label();
        let r63 = fconst_reg(pow2f(63), single);
        _ = sse2(cmp, R(x), R(r63));
        jcc(xe.CC_B, small);
        let t = vxmm();
        let mut sub = xe.SUBSD;
        if single {
            sub = xe.SUBSS;
        }
        sse3(sub, t, x, R(r63));
        let i = sse2(cvt, W(d), R(t));
        i.sz = 8;
        let top = vgpr();
        _ = e2(xe.MOV_RI, 8, W(top), I(-9223372036854775807 - 1));
        let o = e2(xe.ALU, 8, RW(d), R(top));
        o.cc = 6;
        jmp(done);
        place(small);
        let j = sse2(cvt, W(d), R(x));
        j.sz = 8;
    }
    place(done);
}

fn pow2f(k: u32) f64 {
    return @bit_cast(f64, @as(u64, 1023 + k) << 52);
}

fn fconst_reg(x: f64, single: bool) i32 {
    let r = vxmm();
    if single {
        _ = sse2(xe.MOVSS_LD, W(r), const16(@as(u64, @bit_cast(u32, @float_cast(f32, x))), 0));
    } else {
        _ = sse2(xe.MOVSD_LD, W(r), const16(@bit_cast(u64, x), 0));
    }
    return r;
}

// -----------------------------------------------------------------------------
// Floating point
// -----------------------------------------------------------------------------
fn fop(o: i32, single: bool, vec: bool) i32 {
    // ADDSS, ADDSD, ADDPS, ADDPD families: +0 ss, +1 sd, +2 ps, +3 pd
    let mut k: i32 = 1;
    if vec and single {
        k = 2;
    } else if vec {
        k = 3;
    } else if single {
        k = 0;
    }
    let mut base = xe.ADDSS;
    if o == ir.OP_FMUL {
        base = xe.MULSS;
    } else if o == ir.OP_FSUB {
        base = xe.SUBSS;
    } else if o == ir.OP_FDIV {
        base = xe.DIVSS;
    }
    return base + k;
}

fn is_f32(t: *ir.Ty) bool {
    if t.kind == ir.T_VEC {
        return t.elem.bits == 32;
    }
    return t.bits == 32;
}

fn gen_fbin(v: *mut ir.Value) void {
    let d = vreg(v);
    let a = reg(v.a);
    let b = reg(v.b);
    sse3(fop(v.op, is_f32(v.ty), v.ty.kind == ir.T_VEC), d, a, R(b));
}

// sign-bit masks
fn sign_mask(single: bool) mc.MOp {
    if single {
        return const16(0x8000_0000_8000_0000, 0x8000_0000_8000_0000);
    }
    return const16(0x8000_0000_0000_0000, 0x8000_0000_0000_0000);
}

fn abs_mask(single: bool) mc.MOp {
    if single {
        return const16(0x7fff_ffff_7fff_ffff, 0x7fff_ffff_7fff_ffff);
    }
    return const16(0x7fff_ffff_ffff_ffff, 0x7fff_ffff_ffff_ffff);
}

fn gen_fneg(v: *mut ir.Value) void {
    let d = vreg(v);
    let a = reg(v.a);
    let single = is_f32(v.ty);
    let mut op = xe.XORPD;
    if single {
        op = xe.XORPS;
    }
    sse3(op, d, a, sign_mask(single));
}

// IEEE minimum / maximum: or / and of both operand orders gives the right
// signed zero; NaN inputs are forced through with an unordered mask.
fn gen_fminmax(d: i32, a: i32, b: i32, is_min: bool, single: bool, vec: bool) void {
    let k = u.pick(single, "\x00", "\x01")[0];
    let mut kk: i32 = @as(i32, k);
    if vec {
        kk += 2;
    }
    let mut mn = xe.MINSS + kk;
    if not is_min {
        mn = xe.MAXSS + kk;
    }
    let m1 = vxmm();
    let m2 = vxmm();
    sse3(mn, m1, a, R(b));
    sse3(mn, m2, b, R(a));
    let mut orp = xe.ORPD;
    let mut andp = xe.ANDPD;
    let mut cmp = xe.CMPSD;
    let mut add = xe.ADDSD;
    if single {
        orp = xe.ORPS;
        andp = xe.ANDPS;
    }
    if vec and single {
        cmp = xe.CMPPS;
        add = xe.ADDPS;
    } else if vec {
        cmp = xe.CMPPD;
        add = xe.ADDPD;
    } else if single {
        cmp = xe.CMPSS;
        add = xe.ADDSS;
    }
    if is_min {
        sse3(orp, d, m1, R(m2));
        return;
    }
    let r = vxmm();
    sse3(andp, r, m1, R(m2));
    let un = vxmm();
    sse3_i(cmp, un, a, R(b), 3);  // unordered
    let s = vxmm();
    sse3(add, s, a, R(b));
    let n = vxmm();
    sse3(andp, n, un, R(s));
    sse3(orp, d, r, R(n));
}

fn round_imm(id: i32) i64 {
    if id == ir.IN_FLOOR {
        return 9;
    }
    if id == ir.IN_CEIL {
        return 10;
    }
    if id == ir.IN_FTRUNC {
        return 11;
    }
    return 8;
}

fn gen_float_intr(v: *mut ir.Value, id: i32) void {
    let t = v.ty;
    let vec = t.kind == ir.T_VEC;
    let single = is_f32(t);
    let d = vreg(v);
    let a = reg(v.xs.get(0));
    let mut kk: i32 = 1;
    if vec and single {
        kk = 2;
    } else if vec {
        kk = 3;
    } else if single {
        kk = 0;
    }
    if id == ir.IN_FABS {
        let mut op = xe.ANDPD;
        if single {
            op = xe.ANDPS;
        }
        sse3(op, d, a, abs_mask(single));
        return;
    }
    if id == ir.IN_SQRT {
        sse_unary(xe.SQRTSS + kk, d, R(a));
        return;
    }
    if (id == ir.IN_FMIN) or (id == ir.IN_FMAX) {
        gen_fminmax(d, a, reg(v.xs.get(1)), id == ir.IN_FMIN, single, vec);
        return;
    }
    if id == ir.IN_COPYSIGN {
        let b = reg(v.xs.get(1));
        let mut andp = xe.ANDPD;
        let mut orp = xe.ORPD;
        if single {
            andp = xe.ANDPS;
            orp = xe.ORPS;
        }
        let x = vxmm();
        let y = vxmm();
        sse3(andp, x, a, abs_mask(single));
        sse3(andp, y, b, sign_mask(single));
        sse3(orp, d, x, R(y));
        return;
    }
    if (id == ir.IN_FLOOR) or (id == ir.IN_CEIL) or (id == ir.IN_FTRUNC) or (id == ir.IN_NEAREST) {
        let mut op = xe.ROUNDSD;
        if vec and single {
            op = xe.ROUNDPS;
        } else if vec {
            op = xe.ROUNDPD;
        } else if single {
            op = xe.ROUNDSS;
        }
        if AVX {
            let i = e4(xe.SSE, 0, W(d), R(a), R(a), I(round_imm(id)));
            i.cc = op;
        } else {
            _ = sse_i(op, W(d), R(a), round_imm(id));
        }
        return;
    }
    if id == ir.IN_FMA {
        // d = a * b + c: vfmadd231 d(=c) += a * b
        let b = reg(v.xs.get(1));
        let c = reg(v.xs.get(2));
        copy(d, c, XMM);
        let mut op = xe.VFMADD231SD;
        if vec and single {
            op = xe.VFMADD231PS;
        } else if vec {
            op = xe.VFMADD231PD;
        } else if single {
            op = xe.VFMADD231SS;
        }
        let i = e4(xe.SSE, 0, RW(d), R(b), R(a), NONE());
        i.cc = op;
        return;
    }
    fail(u.f1("float intrinsic %s", u.idec(id)));
}

// -----------------------------------------------------------------------------
// Memory
// -----------------------------------------------------------------------------
fn gen_load(v: *mut ir.Value) void {
    let t = v.ty;
    let d = vreg(v);
    let m = addr(v.a);
    let k = t.kind;
    if k == ir.T_VEC {
        _ = sse2(xe.MOVDQU_LD, W(d), m);
        return;
    }
    if k == ir.T_FLOAT {
        if t.bits == 32 {
            _ = sse2(xe.MOVSS_LD, W(d), m);
        } else {
            _ = sse2(xe.MOVSD_LD, W(d), m);
        }
        return;
    }
    let b = t.bits;
    if b <= 16 {
        let mut sz: u8 = 1;
        if b == 16 {
            sz = 2;
        }
        let i = e2(xe.LOAD, sz, W(d), m);
        i.cc = 1;
        return;
    }
    _ = e2(xe.LOAD, isz(t), W(d), m);
}

fn mem_size(t: *ir.Ty) u8 {
    if (t.kind == ir.T_INT) and (t.bits <= 8) {
        return 1;
    }
    return @truncate(u8, t.size);
}

fn gen_store(v: *mut ir.Value) void {
    let x = v.a;
    let t = x.ty;
    let m = addr(v.b);
    let k = t.kind;
    if (v.flags & ir.F_ATOMIC) != 0 {
        // sequentially consistent store: xchg (a full barrier)
        let r = vgpr();
        copy(r, reg(x), GPR);
        _ = e2(xe.XCHG, mem_size(t), m, RW(r));
        return;
    }
    if k == ir.T_VEC {
        _ = sse2(xe.MOVDQU_ST, R(reg(x)), m);
        return;
    }
    if k == ir.T_FLOAT {
        if t.bits == 32 {
            _ = sse2(xe.MOVSS_ST, R(reg(x)), m);
        } else {
            _ = sse2(xe.MOVSD_ST, R(reg(x)), m);
        }
        return;
    }
    let sz = mem_size(t);
    if imm_ok(x) {
        let mut c = cval(x);
        if sz == 1 {
            c = @as(i64, @truncate(i8, c));
        } else if sz == 2 {
            c = @as(i64, @truncate(i16, c));
        }
        _ = e2(xe.STORE_I, sz, m, I(c));
        return;
    }
    _ = e2(xe.STORE, sz, m, R(reg(x)));
}

// memcpy / memmove / memset
fn gen_memop(v: *mut ir.Value) void {
    let o = v.op;
    if (o != ir.OP_MEMSET) and is_c(v.c) and (cval(v.c) <= 128) and (o == ir.OP_MEMCPY) {
        let n = cval(v.c);
        let dm = addr(v.a);
        let sm = addr(v.b);
        let mut k: i64 = 0;
        while k < n {
            let rem = n - k;
            if rem >= 16 {
                let t = vxmm();
                let mut s2 = sm;
                s2.disp += k;
                let mut d2 = dm;
                d2.disp += k;
                _ = sse2(xe.MOVDQU_LD, W(t), s2);
                _ = sse2(xe.MOVDQU_ST, R(t), d2);
                k += 16;
                continue;
            }
            let mut w: i64 = 8;
            while w > rem {
                w = w / 2;
            }
            let t = vgpr();
            let mut s2 = sm;
            s2.disp += k;
            let mut d2 = dm;
            d2.disp += k;
            let sz = @truncate(u8, @bit_cast(u64, w));
            let li = e2(xe.LOAD, sz, W(t), s2);
            if sz < 4 {
                li.cc = 1;
            }
            _ = e2(xe.STORE, sz, d2, R(t));
            k += w;
        }
        return;
    }
    if (o == ir.OP_MEMSET) and is_c(v.c) and (cval(v.c) <= 64) and is_c(v.b) {
        let n = cval(v.c);
        let dm = addr(v.a);
        let byte = cval(v.b) & 0xff;
        let pat = byte * 0x0101_0101_0101_0101;
        let t = vgpr();
        _ = e2(xe.MOV_RI, 8, W(t), I(pat));
        let mut k: i64 = 0;
        while k < n {
            let mut w: i64 = 8;
            while w > (n - k) {
                w = w / 2;
            }
            let mut d2 = dm;
            d2.disp += k;
            _ = e2(xe.STORE, @truncate(u8, @bit_cast(u64, w)), d2, R(t));
            k += w;
        }
        return;
    }
    let dst = reg(v.a);
    let src = reg(v.b);
    let n = reg(v.c);
    _ = e2(xe.MOV_RR, 8, W(xe.RDI), R(dst));
    if o == ir.OP_MEMSET {
        _ = e2(xe.MOV_RR, 8, W(xe.RAX), R(src));
    } else {
        _ = e2(xe.MOV_RR, 8, W(xe.RSI), R(src));
    }
    _ = e2(xe.MOV_RR, 8, W(xe.RCX), R(n));
    if o == ir.OP_MEMSET {
        let i = e3(xe.REP, 0, RW(xe.RDI), R(xe.RAX), RW(xe.RCX));
        i.cc = 1;
    } else if o == ir.OP_MEMCPY {
        let i = e3(xe.REP, 0, RW(xe.RDI), RW(xe.RSI), RW(xe.RCX));
        i.cc = 0;
    } else {
        let ops: [4]mc.MOp = [RW(xe.RDI), RW(xe.RSI), RW(xe.RCX), W(xe.RAX)];
        let i = emit(xe.REP, 0, ops[..]);
        i.cc = 2;
    }
}

// -----------------------------------------------------------------------------
// Atomics
// -----------------------------------------------------------------------------
fn gen_rmw(v: *mut ir.Value) void {
    let o = @truncate(i32, @bit_cast(i64, v.cv));
    let t = v.ty;
    let sz = mem_size(t);
    let d = vreg(v);
    let m = addr(v.a);
    let x = reg(v.b);
    if (o == ir.RMW_ADD) or (o == ir.RMW_SUB) or (o == ir.RMW_XCHG) {
        copy(d, x, GPR);
        if o == ir.RMW_SUB {
            let n = e1(xe.UNARY, isz(t), RW(d));
            n.cc = 3;
        }
        if o == ir.RMW_XCHG {
            _ = e2(xe.XCHG, sz, m, RW(d));
        } else {
            _ = e2(xe.XADD, sz, m, RW(d));
        }
        return;
    }
    // compare-exchange loop; rax holds the old value
    let lp = new_label();
    let ld = e2(xe.LOAD, sz, W(xe.RAX), m);
    if sz < 4 {
        ld.cc = 1;
    }
    place(lp);
    let nv = vgpr();
    copy(nv, xe.RAX, GPR);
    let sg = (o == ir.RMW_MIN) or (o == ir.RMW_MAX);
    if (o == ir.RMW_AND) or (o == ir.RMW_OR) or (o == ir.RMW_XOR) {
        let i = e2(xe.ALU, 8, RW(nv), R(x));
        if o == ir.RMW_AND {
            i.cc = 4;
        } else if o == ir.RMW_OR {
            i.cc = 1;
        } else {
            i.cc = 6;
        }
    } else {
        // min / max: compare extended values, cmov x
        let bits = t.bits;
        let mut a = nv;
        let mut b = x;
        if bits < 32 {
            a = ext_reg(nv, bits, sg, false);
            b = ext_reg(x, bits, sg, false);
        }
        let c = e2(xe.ALU, isz(t), R(a), R(b));
        c.cc = 7;
        let mv = e2(xe.CMOV, isz(t), RW(nv), R(x));
        if o == ir.RMW_MIN {
            mv.cc = xe.CC_G;
        } else if o == ir.RMW_MAX {
            mv.cc = xe.CC_L;
        } else if o == ir.RMW_UMIN {
            mv.cc = xe.CC_A;
        } else {
            mv.cc = xe.CC_B;
        }
    }
    let ops: [3]mc.MOp = [m, R(nv), RW(xe.RAX)];
    let cx = emit(xe.CMPXCHG, sz, ops[..]);
    _ = cx;
    // loop back; the operands stay live around the loop
    let back = loop_jcc(xe.CC_NE, lp, m, x);
    _ = back;
    copy(d, xe.RAX, GPR);
}

// A backward jcc within a block, with fake reads of what the loop uses so
// their registers stay allocated across it.
fn loop_jcc(cc: i32, l: i32, m: mc.MOp, x: i32) *mut mc.MInst {
    let mut ops = u.List.[mc.MOp].{ data: u.nil.[mc.MOp](), len: 0, cap: 0 };
    ops.push(mc.label(l));
    ops.push(R(x));
    if m.base >= 0 {
        ops.push(R(m.base));
    }
    if m.index >= 0 {
        ops.push(R(m.index));
    }
    let i = emit(xe.JCC, 0, ops.items());
    i.cc = cc;
    return i;
}

fn gen_cmpxchg(v: *mut ir.Value) void {
    let t = v.ty;
    let sz = mem_size(t);
    let m = addr(v.a);
    let e = reg(v.b);
    let n = reg(v.c);
    _ = e2(xe.MOV_RR, 8, W(xe.RAX), R(e));
    let ops: [3]mc.MOp = [m, R(n), RW(xe.RAX)];
    _ = emit(xe.CMPXCHG, sz, ops[..]);
    copy(vreg(v), xe.RAX, GPR);
}

// -----------------------------------------------------------------------------
// Calls
// -----------------------------------------------------------------------------
let arg_gprs: [6]i32 = [7, 6, 2, 1, 8, 9];   // rdi rsi rdx rcx r8 r9

fn caller_saved() u64 {
    // rax rcx rdx rsi rdi r8-r11, all xmm
    return (0x0fc7 | (@as(u64, 0xffff) << 16));
}

fn gen_call(v: *mut ir.Value) void {
    let fty = v.fty;
    let n = v.xs.len;
    // arguments into registers (extended as the callee expects)
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
    // stack arguments
    let mut ng: usize = 0;
    let mut nx: usize = 0;
    let mut off: u64 = 0;
    let mut uses = u.List.[mc.MOp].{ data: u.nil.[mc.MOp](), len: 0, cap: 0 };
    let dest = u.newn.[i32](n + 1);
    for i in 0..n {
        let x = v.xs.get(i);
        let c = class_of(x.ty);
        dest[i] = -1;
        if (c == GPR) and (ng < 6) {
            dest[i] = arg_gprs[ng];
            ng += 1;
        } else if (c == XMM) and (nx < 8) {
            dest[i] = xe.XMM0 + @int_cast(i32, nx);
            nx += 1;
        } else {
            let mut sz: u64 = 8;
            if x.ty.kind == ir.T_VEC {
                sz = 16;
                off = (off + 15) & ~@as(u64, 15);
            }
            let m = mc.mem(xe.RSP, -1, 0, @bit_cast(i64, off));
            if c == GPR {
                _ = e2(xe.STORE, 8, m, R(regs[i]));
            } else if x.ty.kind == ir.T_VEC {
                _ = sse2(xe.MOVDQU_ST, R(regs[i]), m);
            } else if x.ty.bits == 32 {
                _ = sse2(xe.MOVSS_ST, R(regs[i]), m);
            } else {
                _ = sse2(xe.MOVSD_ST, R(regs[i]), m);
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
            if dest[i] >= xe.XMM0 {
                let m = sse2(xe.MOVAPS, W(dest[i]), R(regs[i]));
                m.is_move = true;
            } else {
                let m = e2(xe.MOV_RR, 8, W(dest[i]), R(regs[i]));
                m.is_move = true;
            }
            hint(regs[i], dest[i]);
            uses.push(R(dest[i]));
        }
    }
    if fty.vararg {
        _ = e2(xe.MOV_RI, 4, W(xe.RAX), I(@int_cast(i64, nx)));
        uses.push(R(xe.RAX));
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
    let rt = fty.ret;
    let has_ret = rt.kind != ir.T_VOID;
    let mut rreg = xe.RAX;
    if has_ret and (class_of(rt) == XMM) {
        rreg = xe.XMM0;
    }
    if has_ret {
        ops.push(W(rreg));
    }
    let ci = emit(xe.CALL, 0, ops.items());
    ci.clobber = caller_saved();
    MF.has_calls = true;
    if has_ret {
        let d = vreg(v);
        if class_of(rt) == XMM {
            copy(d, xe.XMM0, XMM);
        } else {
            copy(d, xe.RAX, GPR);
        }
    }
    u.free(@ptr_cast(*mut void, uses.data));
    u.free(@ptr_cast(*mut void, ops.data));
}

// -----------------------------------------------------------------------------
// Integer intrinsics
// -----------------------------------------------------------------------------
fn gen_int_intr(v: *mut ir.Value, id: i32) void {
    let t = v.ty;
    let bits = t.bits;
    let sz = isz(t);
    let d = vreg(v);
    let a0 = v.xs.get(0);
    if id == ir.IN_CTLZ {
        let x = reg_ext(a0, false);
        if tg.has(tg.X_LZCNT) {
            let i = e2(xe.BITSCAN, sz, W(d), R(x));
            i.cc = 3;
            if bits < 32 {
                let s = e2(xe.ALU, 4, RW(d), I(@as(i64, 32 - bits)));
                s.cc = 5;
            }
            return;
        }
        // bits - 1 - bsr(x), with bsr(0) taken as -1
        let wbits = @as(i64, sz) * 8;
        let m1 = vgpr();
        _ = e2(xe.MOV_RI, sz, W(m1), I(-1));
        let b = vgpr();
        let i = e2(xe.BITSCAN, sz, W(b), R(x));
        i.cc = 1;
        let c = e2(xe.CMOV, sz, RW(b), R(m1));
        c.cc = xe.CC_E;
        _ = e2(xe.MOV_RI, sz, W(d), I(wbits - 1));
        let s = e2(xe.ALU, sz, RW(d), R(b));
        s.cc = 5;
        if bits < 32 {
            let s2 = e2(xe.ALU, 4, RW(d), I(@as(i64, 32 - bits)));
            s2.cc = 5;
        }
        return;
    }
    if id == ir.IN_CTTZ {
        let mut x = reg(a0);
        if bits < 32 {
            let y = vgpr();
            copy(y, x, GPR);
            let o = e2(xe.ALU, 4, RW(y), I(@as(i64, 1) << @int_cast(u32, bits)));
            o.cc = 1;
            x = y;
        }
        if tg.has(tg.X_BMI1) {
            let i = e2(xe.BITSCAN, sz, W(d), R(x));
            i.cc = 2;
            return;
        }
        let w = vgpr();
        _ = e2(xe.MOV_RI, 4, W(w), I(@as(i64, sz) * 8));
        let i = e2(xe.BITSCAN, sz, W(d), R(x));
        i.cc = 0;
        let c = e2(xe.CMOV, sz, RW(d), R(w));
        c.cc = xe.CC_E;
        return;
    }
    if id == ir.IN_CTPOP {
        let x = reg_ext(a0, false);
        if tg.has(tg.X_POPCNT) {
            let i = e2(xe.BITSCAN, sz, W(d), R(x));
            i.cc = 4;
            return;
        }
        popcount_swar(d, x, sz);
        return;
    }
    if (id == ir.IN_ROTL) or (id == ir.IN_ROTR) {
        let x = reg(a0);
        let k = v.xs.get(1);
        let mut rsz = sz;
        if bits == 8 {
            rsz = 1;
        } else if bits == 16 {
            rsz = 2;
        }
        let mut digit: i32 = 0;
        if id == ir.IN_ROTR {
            digit = 1;
        }
        copy(d, x, GPR);
        if is_c(k) {
            let i = e3(xe.SHIFT, rsz, RW(d), NONE(), I(cval(k) & @as(i64, bits - 1)));
            i.cc = digit;
        } else {
            _ = e2(xe.MOV_RR, 8, W(xe.RCX), R(reg(k)));
            let i = e3(xe.SHIFT, rsz, RW(d), R(xe.RCX), NONE());
            i.cc = digit;
        }
        return;
    }
    if (id == ir.IN_SMIN) or (id == ir.IN_SMAX) or (id == ir.IN_UMIN) or (id == ir.IN_UMAX) {
        let sg = (id == ir.IN_SMIN) or (id == ir.IN_SMAX);
        let a = reg_ext(a0, sg);
        let b = reg_ext(v.xs.get(1), sg);
        copy(d, a, GPR);
        let c = e2(xe.ALU, sz, R(a), R(b));
        c.cc = 7;
        let m = e2(xe.CMOV, sz, RW(d), R(b));
        if id == ir.IN_SMIN {
            m.cc = xe.CC_G;
        } else if id == ir.IN_SMAX {
            m.cc = xe.CC_L;
        } else if id == ir.IN_UMIN {
            m.cc = xe.CC_A;
        } else {
            m.cc = xe.CC_B;
        }
        return;
    }
    if id == ir.IN_ABS {
        let a = reg_ext(a0, true);
        copy(d, a, GPR);
        let n = e1(xe.UNARY, sz, RW(d));
        n.cc = 3;
        let m = e2(xe.CMOV, sz, RW(d), R(a));
        m.cc = xe.CC_S;
        return;
    }
    if (id == ir.IN_SADD_SAT) or (id == ir.IN_UADD_SAT) or (id == ir.IN_SSUB_SAT) or (id == ir.IN_USUB_SAT) {
        let sg = (id == ir.IN_SADD_SAT) or (id == ir.IN_SSUB_SAT);
        let is_add = (id == ir.IN_SADD_SAT) or (id == ir.IN_UADD_SAT);
        if bits < 64 {
            // exact in 64 bits, then clamped
            let a = ext_reg(reg(a0), bits, sg, true);
            let b = ext_reg(reg(v.xs.get(1)), bits, sg, true);
            let mut a64 = a;
            let mut b64 = b;
            if not sg and (bits == 32) {
                a64 = a;
                b64 = b;
            }
            let r = vgpr();
            copy(r, a64, GPR);
            let op = e2(xe.ALU, 8, RW(r), R(b64));
            if is_add {
                op.cc = 0;
            } else {
                op.cc = 5;
            }
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
            _ = e2(xe.MOV_RI, 8, W(lr), I(lo));
            _ = e2(xe.MOV_RI, 8, W(hr), I(hi));
            let c1 = e2(xe.ALU, 8, R(r), R(lr));
            c1.cc = 7;
            let m1 = e2(xe.CMOV, 8, RW(r), R(lr));
            m1.cc = xe.CC_L;
            let c2 = e2(xe.ALU, 8, R(r), R(hr));
            c2.cc = 7;
            let m2 = e2(xe.CMOV, 8, RW(r), R(hr));
            m2.cc = xe.CC_G;
            copy(d, r, GPR);
            return;
        }
        let a = reg(a0);
        let b = reg(v.xs.get(1));
        copy(d, a, GPR);
        let sat = vgpr();
        if sg {
            // saturation value from a's sign: a < 0 ? MIN : MAX
            copy(sat, a, GPR);
            let s = e3(xe.SHIFT, 8, RW(sat), NONE(), I(63));
            s.cc = 7;
            let mx = vgpr();
            _ = e2(xe.MOV_RI, 8, W(mx), I(9223372036854775807));
            let x = e2(xe.ALU, 8, RW(sat), R(mx));
            x.cc = 6;
        } else if is_add {
            _ = e2(xe.MOV_RI, 8, W(sat), I(-1));
        } else {
            _ = e2(xe.MOV_RI, 4, W(sat), I(0));
        }
        let op = e2(xe.ALU, 8, RW(d), R(b));
        if is_add {
            op.cc = 0;
        } else {
            op.cc = 5;
        }
        let m = e2(xe.CMOV, 8, RW(d), R(sat));
        if sg {
            m.cc = xe.CC_O;
        } else {
            m.cc = xe.CC_B;
        }
        return;
    }
    fail(u.f1("integer intrinsic %s", u.idec(id)));
}

// popcount without POPCNT: the classic SWAR reduction.
fn popcount_swar(d: i32, x: i32, sz: u8) void {
    let m1: i64 = 0x5555_5555_5555_5555;
    let m2: i64 = 0x3333_3333_3333_3333;
    let m4: i64 = 0x0f0f_0f0f_0f0f_0f0f;
    let h01: i64 = 0x0101_0101_0101_0101;
    let t = vgpr();
    let k = vgpr();
    // x - ((x >> 1) & m1)
    copy(t, x, GPR);
    let s1 = e3(xe.SHIFT, sz, RW(t), NONE(), I(1));
    s1.cc = 5;
    _ = e2(xe.MOV_RI, 8, W(k), I(m1));
    let a1 = e2(xe.ALU, sz, RW(t), R(k));
    a1.cc = 4;
    copy(d, x, GPR);
    let sb = e2(xe.ALU, sz, RW(d), R(t));
    sb.cc = 5;
    // (x & m2) + ((x >> 2) & m2)
    _ = e2(xe.MOV_RI, 8, W(k), I(m2));
    copy(t, d, GPR);
    let s2 = e3(xe.SHIFT, sz, RW(t), NONE(), I(2));
    s2.cc = 5;
    let a2 = e2(xe.ALU, sz, RW(t), R(k));
    a2.cc = 4;
    let a3 = e2(xe.ALU, sz, RW(d), R(k));
    a3.cc = 4;
    let ad = e2(xe.ALU, sz, RW(d), R(t));
    ad.cc = 0;
    // (x + (x >> 4)) & m4
    copy(t, d, GPR);
    let s3 = e3(xe.SHIFT, sz, RW(t), NONE(), I(4));
    s3.cc = 5;
    let ad2 = e2(xe.ALU, sz, RW(d), R(t));
    ad2.cc = 0;
    _ = e2(xe.MOV_RI, 8, W(k), I(m4));
    let a4 = e2(xe.ALU, sz, RW(d), R(k));
    a4.cc = 4;
    // (x * h01) >> (bits - 8)
    _ = e2(xe.MOV_RI, 8, W(k), I(h01));
    _ = e2(xe.IMUL, sz, RW(d), R(k));
    let s4 = e3(xe.SHIFT, sz, RW(d), NONE(), I((@as(i64, sz) * 8) - 8));
    s4.cc = 5;
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

// index 0..3 for 8/16/32/64-bit lanes
fn lk(t: *ir.Ty) i32 {
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

fn has41() bool {
    return tg.has(tg.X_SSE41);
}

// Can the instruction selector handle this vector instruction directly?
// (Everything else is split into lanes by lower.scalarize.)
pub fn legal(v: *mut ir.Value) bool {
    let t = v.ty;
    let o = v.op;
    if o == ir.OP_ICMP {
        let at = v.a.ty;
        if at.kind != ir.T_VEC {
            return true;
        }
        let p = @truncate(i32, @bit_cast(i64, v.cv));
        if lane_bits(at) == 64 {
            if (p == ir.IntEQ) or (p == ir.IntNE) {
                return has41();
            }
            return tg.has(tg.X_SSE42);
        }
        return true;
    }
    if (o == ir.OP_INTR) and (v.xs.len > 0) and (v.xs.get(0).ty.kind == ir.T_VEC) {
        let id = @truncate(i32, @bit_cast(i64, v.cv));
        let at = v.xs.get(0).ty;
        if (id == ir.IN_ANYTRUE) or (id == ir.IN_BITMASK) or (id == ir.IN_DOT_I16) {
            return true;
        }
        if id == ir.IN_ALLTRUE {
            return true;
        }
        if id == ir.IN_SWIZZLE {
            return true;
        }
        if lane_float(at) {
            if (id == ir.IN_FLOOR) or (id == ir.IN_CEIL) or (id == ir.IN_FTRUNC) or (id == ir.IN_NEAREST) {
                return has41();
            }
            if id == ir.IN_FMA {
                return tg.has(tg.X_FMA);
            }
            return (id == ir.IN_FABS) or (id == ir.IN_SQRT) or (id == ir.IN_FMIN) or (id == ir.IN_FMAX) or
                   (id == ir.IN_COPYSIGN);
        }
        let b = lane_bits(at);
        if id == ir.IN_ABS {
            return (b <= 32) and tg.has(tg.X_SSSE3);
        }
        if (id == ir.IN_SMIN) or (id == ir.IN_SMAX) {
            if b == 16 {
                return true;
            }
            return ((b == 8) or (b == 32)) and has41();
        }
        if (id == ir.IN_UMIN) or (id == ir.IN_UMAX) {
            if b == 8 {
                return true;
            }
            return ((b == 16) or (b == 32)) and has41();
        }
        if (id == ir.IN_SADD_SAT) or (id == ir.IN_UADD_SAT) or (id == ir.IN_SSUB_SAT) or (id == ir.IN_USUB_SAT) {
            return b <= 16;
        }
        return false;
    }
    if t.kind != ir.T_VEC {
        return true;
    }
    if o == ir.OP_MUL {
        let b = lane_bits(t);
        if b == 16 {
            return true;
        }
        return (b == 32) and has41();
    }
    if (o == ir.OP_SHL) or (o == ir.OP_LSHR) or (o == ir.OP_ASHR) {
        let b = lane_bits(t);
        if b == 8 {
            return false;
        }
        if (o == ir.OP_ASHR) and (b == 64) {
            return false;
        }
        return is_inst(v.b, ir.OP_SPLAT) or (v.b.kind == ir.V_CAGG) or (v.b.kind == ir.V_ZERO);
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
    let mut id: i32 = 0;
    if o == ir.OP_ADD {
        id = xe.PADDB + k;
    } else if o == ir.OP_SUB {
        id = xe.PSUBB + k;
    } else if o == ir.OP_AND {
        id = xe.PAND;
    } else if o == ir.OP_OR {
        id = xe.POR;
    } else if o == ir.OP_XOR {
        id = xe.PXOR;
    } else if o == ir.OP_MUL {
        id = xe.PMULLW;
        if k == 2 {
            id = xe.PMULLD;
        }
    } else {
        fail(u.f1("vector op %s", u.idec(o)));
        return;
    }
    sse3(id, d, a, R(b));
}

fn vec_shift(v: *mut ir.Value, d: i32, a: i32) void {
    let t = v.ty;
    let o = v.op;
    let k = lk(t);  // 1, 2, 3
    let c = v.b;
    // by an immediate?
    if (c.kind == ir.V_CAGG) or (c.kind == ir.V_ZERO) or (is_inst(c, ir.OP_SPLAT) and is_c(c.a)) {
        let mut n: i64 = 0;
        if c.kind == ir.V_CAGG {
            n = cval(c.xs.get(0));
        } else if is_inst(c, ir.OP_SPLAT) {
            n = cval(c.a);
        }
        let mut id = xe.PSLLW_I + (k - 1) * 3;
        if o == ir.OP_LSHR {
            id = xe.PSRLW_I + (k - 1) * 3;
        } else if o == ir.OP_ASHR {
            id = xe.PSRAW_I + (k - 1) * 3;
        }
        // immediate forms: legacy encodes the destination in r/m
        if AVX {
            let i = e4(xe.SSE, 0, NONE(), R(a), W(d), I(n));
            i.cc = id;
        } else {
            copy(d, a, XMM);
            let i = e4(xe.SSE, 0, NONE(), RW(d), NONE(), I(n));
            i.cc = id;
        }
        return;
    }
    let cnt = vxmm();
    let cs = reg(c.a);
    let z = vgpr();
    let e = e2(xe.EXT, 4, W(z), R(cs));
    e.cc = ext_kind(c.a.ty.bits, false);
    if c.a.ty.bits >= 32 {
        e.cc = 6;
        if c.a.ty.bits == 64 {
            e.op = xe.MOV_RR;
            e.sz = 8;
        }
    }
    _ = sse2(xe.MOVQ_XR, W(cnt), R(z));
    let mut id = xe.PSLLW + (k - 1);
    if o == ir.OP_LSHR {
        id = xe.PSRLW + (k - 1);
    } else if o == ir.OP_ASHR {
        id = xe.PSRAW + (k - 1);
    }
    sse3(id, d, a, R(cnt));
}

// Flips the sign bit of each lane (unsigned compares through signed ones).
fn flip_signs(r: i32, t: *ir.Ty) i32 {
    let b = lane_bits(t);
    let mut m: u64 = 0x8080_8080_8080_8080;
    if b == 16 {
        m = 0x8000_8000_8000_8000;
    } else if b == 32 {
        m = 0x8000_0000_8000_0000;
    } else if b == 64 {
        m = 0x8000_0000_0000_0000;
    }
    let d = vxmm();
    sse3(xe.PXOR, d, r, const16(m, m));
    return d;
}

fn vnot(d: i32, r: i32) void {
    sse3(xe.PXOR, d, r, const16(0xffff_ffff_ffff_ffff, 0xffff_ffff_ffff_ffff));
}

fn vec_icmp(v: *mut ir.Value) void {
    let t = v.a.ty;
    let p = @truncate(i32, @bit_cast(i64, v.cv));
    let d = vreg(v);
    let mut a = reg(v.a);
    let mut b = reg(v.b);
    let k = lk(t);
    let mut eq = xe.PCMPEQB + k;
    if k == 3 {
        eq = xe.PCMPEQQ;
    }
    let mut gt = xe.PCMPGTB + k;
    if k == 3 {
        gt = xe.PCMPGTQ;
    }
    if (p == ir.IntEQ) or (p == ir.IntNE) {
        if p == ir.IntEQ {
            sse3(eq, d, a, R(b));
        } else {
            let x = vxmm();
            sse3(eq, x, a, R(b));
            vnot(d, x);
        }
        return;
    }
    let uns = (p == ir.IntULT) or (p == ir.IntULE) or (p == ir.IntUGT) or (p == ir.IntUGE);
    if uns {
        a = flip_signs(a, t);
        b = flip_signs(b, t);
    }
    if (p == ir.IntSGT) or (p == ir.IntUGT) {
        sse3(gt, d, a, R(b));
    } else if (p == ir.IntSLT) or (p == ir.IntULT) {
        sse3(gt, d, b, R(a));
    } else if (p == ir.IntSGE) or (p == ir.IntUGE) {
        let x = vxmm();
        sse3(gt, x, b, R(a));
        vnot(d, x);
    } else {
        let x = vxmm();
        sse3(gt, x, a, R(b));
        vnot(d, x);
    }
}

fn vec_fcmp(v: *mut ir.Value) void {
    let t = v.a.ty;
    let p = @truncate(i32, @bit_cast(i64, v.cv));
    let d = vreg(v);
    let a = reg(v.a);
    let b = reg(v.b);
    let mut op = xe.CMPPD;
    if lane_float(t) and (lane_bits(t) == 32) {
        op = xe.CMPPS;
    }
    if p == ir.RealOEQ {
        sse3_i(op, d, a, R(b), 0);
    } else if p == ir.RealUNE {
        sse3_i(op, d, a, R(b), 4);
    } else if p == ir.RealOLT {
        sse3_i(op, d, a, R(b), 1);
    } else if p == ir.RealOLE {
        sse3_i(op, d, a, R(b), 2);
    } else if p == ir.RealOGT {
        sse3_i(op, d, b, R(a), 1);
    } else {
        sse3_i(op, d, b, R(a), 2);
    }
}

// Lanes through a 16-byte stack slot.
fn vec_slot() i32 {
    return mc.new_slot(MF, 16, 16);
}

fn gen_extract(v: *mut ir.Value) void {
    let t = v.a.ty;
    let lane = @bit_cast(i64, v.cv);
    let d = vreg(v);
    let a = reg(v.a);
    let b = lane_bits(t);
    if lane_float(t) {
        if lane == 0 {
            copy(d, a, XMM);
            return;
        }
        if b == 32 {
            _ = sse_i(xe.PSHUFD, W(d), R(a), lane);
        } else {
            _ = sse_i(xe.PSHUFD, W(d), R(a), 0xee);
        }
        return;
    }
    if b == 16 {
        _ = sse_i(xe.PEXTRW, W(d), R(a), lane);
        return;
    }
    if has41() {
        let mut id = xe.PEXTRB;
        if b == 32 {
            id = xe.PEXTRD;
        } else if b == 64 {
            id = xe.PEXTRQ;
        }
        _ = sse_i(id, R(a), W(d), lane);
        return;
    }
    if b == 32 {
        let x = vxmm();
        _ = sse_i(xe.PSHUFD, W(x), R(a), lane);
        _ = sse2(xe.MOVD_RX, R(x), W(d));
        return;
    }
    if b == 64 {
        if lane == 0 {
            _ = sse2(xe.MOVQ_RX, R(a), W(d));
        } else {
            let x = vxmm();
            _ = sse_i(xe.PSHUFD, W(x), R(a), 0xee);
            _ = sse2(xe.MOVQ_RX, R(x), W(d));
        }
        return;
    }
    // byte: through a 16-bit lane
    _ = sse_i(xe.PEXTRW, W(d), R(a), lane / 2);
    if (lane & 1) != 0 {
        let s = e3(xe.SHIFT, 4, RW(d), NONE(), I(8));
        s.cc = 5;
    }
}

fn gen_insert(v: *mut ir.Value) void {
    let t = v.ty;
    let lane = @bit_cast(i64, v.cv);
    let d = vreg(v);
    let a = reg(v.a);
    let x = reg(v.b);
    let b = lane_bits(t);
    if (b == 16) or (has41() and not lane_float(t)) {
        let mut id = xe.PINSRW;
        if b == 8 {
            id = xe.PINSRB;
        } else if b == 32 {
            id = xe.PINSRD;
        } else if b == 64 {
            id = xe.PINSRQ;
        }
        sse3_i(id, d, a, R(x), lane);
        return;
    }
    // through memory
    let s = vec_slot();
    _ = sse2(xe.MOVDQU_ST, R(a), mc.mem_slot(s, 0));
    let off = lane * @as(i64, b / 8);
    if lane_float(t) {
        if b == 32 {
            _ = sse2(xe.MOVSS_ST, R(x), mc.mem_slot(s, off));
        } else {
            _ = sse2(xe.MOVSD_ST, R(x), mc.mem_slot(s, off));
        }
    } else {
        _ = e2(xe.STORE, @truncate(u8, @bit_cast(u32, b / 8)), mc.mem_slot(s, off), R(x));
    }
    _ = sse2(xe.MOVDQU_LD, W(d), mc.mem_slot(s, 0));
}

fn gen_splat(v: *mut ir.Value) void {
    let t = v.ty;
    let d = vreg(v);
    let x = reg(v.a);
    let b = lane_bits(t);
    if lane_float(t) {
        if b == 32 {
            _ = sse_i(xe.PSHUFD, W(d), R(x), 0);
        } else {
            sse3(xe.PUNPCKLQDQ, d, x, R(x));
        }
        return;
    }
    let m = vxmm();
    if b == 64 {
        _ = sse2(xe.MOVQ_XR, W(m), R(x));
        sse3(xe.PUNPCKLQDQ, d, m, R(m));
        return;
    }
    _ = sse2(xe.MOVD_XR, W(m), R(x));
    if b == 32 {
        _ = sse_i(xe.PSHUFD, W(d), R(m), 0);
        return;
    }
    let mut w = m;
    if b == 8 {
        w = vxmm();
        sse3(xe.PUNPCKLBW, w, m, R(m));
    }
    let l = vxmm();
    _ = sse_i(xe.PSHUFLW, W(l), R(w), 0);
    _ = sse_i(xe.PSHUFD, W(d), R(l), 0);
}

fn gen_shuffle(v: *mut ir.Value) void {
    let t = v.ty;
    let d = vreg(v);
    let a = reg(v.a);
    let b = reg(v.b);
    let lb = @int_cast(u64, lane_bits(t) / 8);
    let n = @as(u64, v.mask.len);
    if tg.has(tg.X_SSSE3) {
        // bytes from a and from b (0x80: zero), or'ed
        let mut ma: [16]u8 = [0x80; 16];
        let mut mb: [16]u8 = [0x80; 16];
        for i in 0..v.mask.len {
            let m = @as(u64, @bit_cast(u32, v.mask[i]));
            for k in @as(u64, 0)..lb {
                let dst = (@as(u64, i) * lb) + k;
                if m < n {
                    ma[@truncate(usize, dst)] = @truncate(u8, (m * lb) + k);
                } else {
                    mb[@truncate(usize, dst)] = @truncate(u8, ((m - n) * lb) + k);
                }
            }
        }
        let xa = vxmm();
        let xb = vxmm();
        sse3(xe.PSHUFB, xa, a, const16(le8(ma[0..8]), le8(ma[8..16])));
        sse3(xe.PSHUFB, xb, b, const16(le8(mb[0..8]), le8(mb[8..16])));
        sse3(xe.POR, d, xa, R(xb));
        return;
    }
    // through memory: a and b in a 32-byte area, lanes copied one by one
    let s = mc.new_slot(MF, 32, 16);
    let r = vec_slot();
    _ = sse2(xe.MOVDQU_ST, R(a), mc.mem_slot(s, 0));
    _ = sse2(xe.MOVDQU_ST, R(b), mc.mem_slot(s, 16));
    let tmp = vgpr();
    let sz = @truncate(u8, lb);
    for i in 0..v.mask.len {
        let m = @as(i64, v.mask[i]);
        let li = e2(xe.LOAD, sz, W(tmp), mc.mem_slot(s, m * @bit_cast(i64, lb)));
        if sz < 4 {
            li.cc = 1;
        }
        _ = e2(xe.STORE, sz, mc.mem_slot(r, @int_cast(i64, i) * @bit_cast(i64, lb)), R(tmp));
    }
    _ = sse2(xe.MOVDQU_LD, W(d), mc.mem_slot(r, 0));
}

fn le8(b: []u8) u64 {
    let mut x: u64 = 0;
    for i in @as(usize, 0)..8 {
        x = x | (@as(u64, b[i]) << @int_cast(u32, i * 8));
    }
    return x;
}

fn gen_vec_intr(v: *mut ir.Value, id: i32) void {
    let a0 = v.xs.get(0);
    let at = a0.ty;
    if (id == ir.IN_ANYTRUE) or (id == ir.IN_ALLTRUE) or (id == ir.IN_BITMASK) {
        let d = vreg(v);
        let a = reg(a0);
        let k = lk(at);
        if id == ir.IN_BITMASK {
            if lane_float(at) or (k >= 2) {
                if k == 2 {
                    _ = sse2(xe.MOVMSKPS, W(d), R(a));
                } else {
                    _ = sse2(xe.MOVMSKPD, W(d), R(a));
                }
                return;
            }
            let mut x = a;
            if k == 1 {
                x = vxmm();
                sse3(xe.PACKSSWB, x, a, R(a));
            }
            _ = sse2(xe.PMOVMSKB, W(d), R(x));
            if k == 1 {
                let m = e2(xe.ALU, 4, RW(d), I(0xff));
                m.cc = 4;
            }
            return;
        }
        let z = vxmm();
        sse3(xe.PXOR, z, z, R(z));
        if id == ir.IN_ANYTRUE {
            // some byte nonzero: not all bytes equal to zero
            let e = vxmm();
            sse3(xe.PCMPEQB, e, a, R(z));
            let m = vgpr();
            _ = sse2(xe.PMOVMSKB, W(m), R(e));
            let c = e2(xe.ALU, 4, R(m), I(0xffff));
            c.cc = 7;
            materialize_cc(d, xe.CC_NE);
            return;
        }
        // all lanes nonzero: no lane equal to zero
        let e = vxmm();
        if k == 3 {
            if has41() {
                sse3(xe.PCMPEQQ, e, a, R(z));
            } else {
                let e1_ = vxmm();
                sse3(xe.PCMPEQD, e1_, a, R(z));
                let sw = vxmm();
                _ = sse_i(xe.PSHUFD, W(sw), R(e1_), 0xb1);
                sse3(xe.PAND, e, e1_, R(sw));
            }
        } else {
            sse3(xe.PCMPEQB + k, e, a, R(z));
        }
        let m = vgpr();
        _ = sse2(xe.PMOVMSKB, W(m), R(e));
        _ = e2(xe.TEST, 4, R(m), R(m));
        materialize_cc(d, xe.CC_E);
        return;
    }
    let d = vreg(v);
    if (id == ir.IN_SWIZZLE) and not tg.has(tg.X_SSSE3) {
        // byte by byte through memory
        let a = reg(a0);
        let idx = reg(v.xs.get(1));
        let sx = vec_slot();
        let si = vec_slot();
        let sr = vec_slot();
        _ = sse2(xe.MOVDQU_ST, R(a), mc.mem_slot(sx, 0));
        _ = sse2(xe.MOVDQU_ST, R(idx), mc.mem_slot(si, 0));
        for lane in @as(i64, 0)..16 {
            let t = vgpr();
            let b = vgpr();
            let li = e2(xe.LOAD, 1, W(t), mc.mem_slot(si, lane));
            li.cc = 1;
            _ = e2(xe.MOV_RI, 4, W(b), I(0));
            let c = e2(xe.ALU, 4, R(t), I(16));
            c.cc = 7;
            let skip = new_label();
            jcc(xe.CC_AE, skip);
            let mut m = mc.mem_slot(sx, 0);
            m.index = t;
            m.scale = 1;
            let lb = e2(xe.LOAD, 1, W(b), m);
            lb.cc = 1;
            place(skip);
            _ = e2(xe.STORE, 1, mc.mem_slot(sr, lane), R(b));
        }
        _ = sse2(xe.MOVDQU_LD, W(d), mc.mem_slot(sr, 0));
        return;
    }
    if id == ir.IN_SWIZZLE {
        let a = reg(a0);
        let idx = reg(v.xs.get(1));
        let adj = vxmm();
        sse3(xe.PADDUSB, adj, idx, const16(0x7070_7070_7070_7070, 0x7070_7070_7070_7070));
        sse3(xe.PSHUFB, d, a, R(adj));
        return;
    }
    if id == ir.IN_DOT_I16 {
        sse3(xe.PMADDWD, d, reg(a0), R(reg(v.xs.get(1))));
        return;
    }
    if lane_float(at) {
        gen_float_intr(v, id);
        return;
    }
    let a = reg(a0);
    let k = lk(at);
    if id == ir.IN_ABS {
        sse_unary(xe.PABSB + k, d, R(a));
        return;
    }
    let b = reg(v.xs.get(1));
    let mut op: i32 = 0;
    if id == ir.IN_SMIN {
        if k == 0 {
            op = xe.PMINSB;
        } else if k == 1 {
            op = xe.PMINSW;
        } else {
            op = xe.PMINSD;
        }
    } else if id == ir.IN_SMAX {
        if k == 0 {
            op = xe.PMAXSB;
        } else if k == 1 {
            op = xe.PMAXSW;
        } else {
            op = xe.PMAXSD;
        }
    } else if id == ir.IN_UMIN {
        if k == 0 {
            op = xe.PMINUB;
        } else if k == 1 {
            op = xe.PMINUW;
        } else {
            op = xe.PMINUD;
        }
    } else if id == ir.IN_UMAX {
        if k == 0 {
            op = xe.PMAXUB;
        } else if k == 1 {
            op = xe.PMAXUW;
        } else {
            op = xe.PMAXUD;
        }
    } else if id == ir.IN_SADD_SAT {
        op = xe.PADDSB + k;
    } else if id == ir.IN_UADD_SAT {
        op = xe.PADDUSB + k;
    } else if id == ir.IN_SSUB_SAT {
        op = xe.PSUBSB + k;
    } else if id == ir.IN_USUB_SAT {
        op = xe.PSUBUSB + k;
    } else {
        fail(u.f1("vector intrinsic %s", u.idec(id)));
        return;
    }
    sse3(op, d, a, R(b));
}

// -----------------------------------------------------------------------------
// Instructions
// -----------------------------------------------------------------------------
fn gen_ptradd(v: *mut ir.Value) void {
    let d = vreg(v);
    if imm_ok(v.b) {
        let mut m = addr(v.a);
        let nd = m.disp + cval(v.b);
        if fits32(nd) and (m.scale != 99) and (m.slot != -2) {
            m.disp = nd;
            _ = e2(xe.LEA, 8, W(d), m);
            return;
        }
    }
    let a = reg(v.a);
    let off = v.b;
    if is_inst(off, ir.OP_MUL) and is_c(off.b) {
        let s = cval(off.b);
        if (s == 1) or (s == 2) or (s == 4) or (s == 8) {
            _ = e2(xe.LEA, 8, W(d), mc.mem(a, reg(off.a), @truncate(u8, @bit_cast(u64, s)), 0));
            return;
        }
    }
    _ = e2(xe.LEA, 8, W(d), mc.mem(a, reg(off), 1, 0));
}

fn gen_inst(v: *mut ir.Value) void {
    let o = v.op;
    let t = v.ty;
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
        _ = e0(xe.MFENCE, 0);
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
            _ = e0(xe.UD2, 0);
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
            // an undefined input leaves the phi as it is
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
    return (s.aux == (b.aux + 1));
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
        if cc == -1 {
            // equal and ordered
            jcc(xe.CC_NE, blk(ff));
            jcc(xe.CC_P, blk(ff));
            if not next_block_is(b, tt) {
                jmp(blk(tt));
            }
            return;
        }
        if cc == -2 {
            jcc(xe.CC_NE, blk(tt));
            jcc(xe.CC_P, blk(tt));
            if not next_block_is(b, ff) {
                jmp(blk(ff));
            }
            return;
        }
        if next_block_is(b, tt) {
            jcc(invert(cc), blk(ff));
            return;
        }
        jcc(cc, blk(tt));
        if not next_block_is(b, ff) {
            jmp(blk(ff));
        }
        return;
    }
    if o == ir.OP_SWITCH {
        let x = t.a;
        let r = reg_ext(x, true);
        let wide = isz(x.ty) == 8;
        let tr = vgpr();
        copy(tr, r, GPR);
        phi_copies(b);
        let mut sz: u8 = 4;
        if wide {
            sz = 8;
        }
        for k in 0..t.xs.len {
            let c = t.xs.get(k);
            let mut cv = ir.sval(c);
            if (x.ty.bits < 32) and (x.ty.kind == ir.T_INT) {
                cv = ir.sval(c);
            }
            if fits32(cv) {
                let i = e2(xe.ALU, sz, R(tr), I(cv));
                i.cc = 7;
            } else {
                let cr = vgpr();
                _ = e2(xe.MOV_RI, 8, W(cr), I(cv));
                let i = e2(xe.ALU, sz, R(tr), R(cr));
                i.cc = 7;
            }
            jcc(xe.CC_E, blk(t.bs.get(k)));
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
            if class_of(x.ty) == XMM {
                let m = sse2(xe.MOVAPS, W(xe.XMM0), R(r));
                m.is_move = true;
                _ = e1(xe.RET, 0, R(xe.XMM0));
            } else {
                let m = e2(xe.MOV_RR, 8, W(xe.RAX), R(r));
                m.is_move = true;
                _ = e1(xe.RET, 0, R(xe.RAX));
            }
            return;
        }
        _ = e0(xe.RET, 0);
        return;
    }
    _ = e0(xe.UD2, 0);
}

// -----------------------------------------------------------------------------
// Per-function driver
// -----------------------------------------------------------------------------
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

let gpr_order: [14]i32 = [0, 1, 2, 6, 7, 8, 9, 10, 11, 3, 12, 13, 14, 15];
let xmm_order: [16]i32 = [16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31];

fn spill_load(f: *mut mc.MFunc, r: i32, class: i32, slot: i32) *mut mc.MInst {
    if class == 0 {
        let ops = mk2(W(r), mc.mem_slot(slot, 0));
        return mc.new_inst(xe.LOAD, 8, ops[..]);
    }
    let ops = mk4(W(r), mc.mem_slot(slot, 0), NONE(), NONE());
    let i = mc.new_inst(xe.SSE, 0, ops[..]);
    i.cc = xe.MOVDQU_LD;
    return i;
}

fn spill_store(f: *mut mc.MFunc, r: i32, class: i32, slot: i32) *mut mc.MInst {
    if class == 0 {
        let ops = mk2(mc.mem_slot(slot, 0), R(r));
        return mc.new_inst(xe.STORE, 8, ops[..]);
    }
    let ops = mk4(R(r), mc.mem_slot(slot, 0), NONE(), NONE());
    let i = mc.new_inst(xe.SSE, 0, ops[..]);
    i.cc = xe.MOVDQU_ST;
    return i;
}

fn mk2(a: mc.MOp, b: mc.MOp) [2]mc.MOp {
    return [a, b];
}

fn mk4(a: mc.MOp, b: mc.MOp, c: mc.MOp, d: mc.MOp) [4]mc.MOp {
    return [a, b, c, d];
}

fn compile_func(f: *mut ir.Func) void {
    F = f;
    nlocal = 0;
    out_area = 0;
    opt.split_critical_edges(f);
    lower.scalarize(f, legal);
    opt.compute_preds(f);
    let order = opt.rpo(f);
    prepare(f);
    MF = mc.new_func();
    for i in 0..order.len {
        let b = order.get(i);
        b.aux = @int_cast(i64, i);
        _ = mc.new_block(MF);
    }
    // stack slots for allocas
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
    let mut ng: usize = 0;
    let mut nx: usize = 0;
    let mut soff: i64 = 16;
    for p in f.params {
        let c = class_of(p.ty);
        let d = vreg(p);
        if (c == GPR) and (ng < 6) {
            copy(d, arg_gprs[ng], GPR);
            ng += 1;
        } else if (c == XMM) and (nx < 8) {
            copy(d, xe.XMM0 + @int_cast(i32, nx), XMM);
            nx += 1;
        } else {
            let m = mc.mem(xe.RBP, -1, 0, soff);
            if c == GPR {
                _ = e2(xe.LOAD, 8, W(d), m);
                soff += 8;
            } else if p.ty.kind == ir.T_VEC {
                soff = (soff + 15) & ~@as(i64, 15);
                _ = sse2(xe.MOVDQU_LD, W(d), m);
                soff += 16;
            } else {
                if p.ty.bits == 32 {
                    _ = sse2(xe.MOVSS_LD, W(d), m);
                } else {
                    _ = sse2(xe.MOVSD_LD, W(d), m);
                }
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
    let tgt = mc.Target.{ nclasses: 2, alloc_order: [gpr_order[..], xmm_order[..]], caller_saved: caller_saved(),
                          spill_load: spill_load, spill_store: spill_store };
    mc.allocate(MF, &tgt);
    encode_func(f);
}

// -----------------------------------------------------------------------------
// Encoding
// -----------------------------------------------------------------------------
let mut E: xe.Enc = xe.Enc.{ b: bn.Bytes.{ b: u.List.[u8].{ data: @ptr_from_int(*mut u8, 0), len: 0, cap: 0 } },
                             relocs: u.List.[xe.Reloc].{ data: @ptr_from_int(*mut xe.Reloc, 0), len: 0, cap: 0 },
                             fixups: u.List.[xe.Fixup].{ data: @ptr_from_int(*mut xe.Fixup, 0), len: 0, cap: 0 },
                             avx: false };
let mut nsaved: i64 = 0;
let mut saved: [5]i32 = [0; 5];
let mut local_off: u.List.[u64] = u.List.[u64].{ data: @ptr_from_int(*mut u64, 0), len: 0, cap: 0 };

fn P(r: i32) i32 {
    return mc.phys(MF, r);
}

fn b8(x: u64) void {
    E.b.u8_(@truncate(u8, x));
}

fn to_mem(o: mc.MOp) xe.Mem {
    let mut m = xe.Mem.{ base: -1, index: -1, scale: o.scale, disp: o.disp, sym: o.sym, tpoff: false };
    if o.base >= 0 {
        m.base = P(o.base);
    }
    if o.index >= 0 {
        m.index = P(o.index);
    }
    if o.slot >= 0 {
        m.base = xe.RBP;
        m.disp += MF.slots.get(@int_cast(usize, o.slot)).offset;
        m.sym = -1;
    } else if o.slot == -2 {
        m.tpoff = true;
    }
    return m;
}

// ModRM for a memory operand, with TLS offsets relative to a base register.
fn enc_mem(rf: i32, o: mc.MOp, imm_after: i64) void {
    let m = to_mem(o);
    if m.tpoff {
        // [base + sym@tpoff] : mod=10, disp32 relocated
        let rfb = @as(u64, @bit_cast(u32, rf & 7)) << 3;
        let bb = @as(u64, @bit_cast(u32, m.base & 7));
        if bb == 4 {
            b8((0x80 | rfb) | 4);
            b8(0x24);
        } else {
            b8((0x80 | rfb) | bb);
        }
        E.relocs.push(xe.Reloc.{ off: @as(u64, E.b.len()), ty: elf.R_X86_64_TPOFF32, sym: m.sym, addend: m.disp });
        E.b.u32_(0);
        return;
    }
    xe.modrm_mem(&mut E, @as(u64, @bit_cast(u32, rf)), &m, imm_after);
}

fn rex_for_mem(w: u64, rf: i32, o: mc.MOp, force: bool) void {
    let m = to_mem(o);
    let r = @as(u64, @bit_cast(u32, (rf & 15) >> 3));
    let x = xe.rm_x(&m);
    let b = xe.rm_b(&m);
    let v = ((w << 3) | (r << 2)) | ((x << 1) | b);
    if (v != 0) or force {
        b8(0x40 | v);
    }
}

fn byte_reg_force(r: i32) bool {
    let x = r & 15;
    return (x >= 4) and (x <= 7);
}

// opcode (1-2 bytes) with reg field `reg` and r/m operand `rm` (register or memory)
fn enc_op(sz: u8, nop: i32, op0: u8, op1: u8, rf: i32, rm: mc.MOp, imm_after: i64, byte_op: bool) void {
    if sz == 2 {
        b8(0x66);
    }
    let mut w: u64 = 0;
    if sz == 8 {
        w = 1;
    }
    if rm.kind == mc.K_REG {
        let rr = P(rm.reg);
        let force = byte_op and (byte_reg_force(rf) or byte_reg_force(rr));
        let v = ((w << 3) | ((@as(u64, @bit_cast(u32, rf & 15)) >> 3) << 2)) | (@as(u64, @bit_cast(u32, rr & 15)) >> 3);
        if (v != 0) or force {
            b8(0x40 | v);
        }
        opcode(nop, op0, op1);
        xe.modrm_rr(&mut E, @as(u64, @bit_cast(u32, rf)), @as(u64, @bit_cast(u32, rr)));
        return;
    }
    rex_for_mem(w, rf, rm, byte_op and byte_reg_force(rf));
    opcode(nop, op0, op1);
    enc_mem(rf, rm, imm_after);
}

fn opcode(n: i32, a: u8, b: u8) void {
    E.b.u8_(a);
    if n == 2 {
        E.b.u8_(b);
    }
}

fn imm_len(sz: u8) i64 {
    if sz == 1 {
        return 1;
    }
    if sz == 2 {
        return 2;
    }
    return 4;
}

fn enc_imm(x: i64, n: i64) void {
    xe.imm_n(&mut E, x, @truncate(u8, @bit_cast(u64, n)));
}


fn label_ref(l: i32) void {
    xe.rel32_fixup(&mut E, l);
}

fn epilogue() void {
    if nsaved > 0 {
        // lea rsp, [rbp - 8*nsaved]
        b8(0x48);
        b8(0x8d);
        let d = -8 * nsaved;
        if d >= -128 {
            b8(0x65);
            b8(@bit_cast(u64, d));
        } else {
            b8(0xa5);
            E.b.u32_(@bit_cast(u64, d));
        }
        let mut k = nsaved;
        while k > 0 {
            k -= 1;
            let r = saved[@int_cast(usize, k)];
            if r >= 8 {
                b8(0x41);
            }
            b8(0x58 + @as(u64, @bit_cast(u32, r & 7)));
        }
    } else {
        b8(0xc9);  // leave
        b8(0xc3);
        return;
    }
    b8(0x5d);
    b8(0xc3);
}

fn prologue(sub: i64) void {
    b8(0x55);            // push rbp
    b8(0x48);
    b8(0x89);
    b8(0xe5);            // mov rbp, rsp
    for k in 0..@int_cast(usize, nsaved) {
        let r = saved[k];
        if r >= 8 {
            b8(0x41);
        }
        b8(0x50 + @as(u64, @bit_cast(u32, r & 7)));
    }
    if sub > 0 {
        // touch each page of a large frame in order (stack guard)
        let mut rem = sub;
        while rem > 4096 {
            b8(0x48);
            b8(0x81);
            b8(0xec);
            E.b.u32_(4096);
            // or qword [rsp], 0
            b8(0x48);
            b8(0x83);
            b8(0x0c);
            b8(0x24);
            b8(0x00);
            rem -= 4096;
        }
        b8(0x48);
        b8(0x81);
        b8(0xec);
        E.b.u32_(@bit_cast(u64, rem));
    }
}

fn enc_sse(i: *mut mc.MInst) void {
    let id = i.cc;
    let d = xe.desc(id);
    let o0 = i.ops[0];
    let o1 = i.ops[1];
    let o2 = i.ops[2];
    let o3 = i.ops[3];
    let mut rf: i32 = 0;
    if o0.kind == mc.K_REG {
        rf = P(o0.reg);
    }
    let mut vv: i32 = -1;
    if o2.kind == mc.K_REG {
        vv = P(o2.reg);
    }
    let has_imm = o3.kind == mc.K_IMM;
    let wide = i.sz == 8;
    if o1.kind == mc.K_REG {
        let m = xe.Mem.{ base: -1, index: -1, scale: 0, disp: 0, sym: -1, tpoff: false };
        xe.sse(&mut E, id, rf, P(o1.reg), &m, vv, o3.disp, has_imm, wide);
    } else {
        let mut m = to_mem(o1);
        if m.tpoff {
            fail("TLS operand in an SSE instruction");
        }
        xe.sse(&mut E, id, rf, -1, &m, vv, o3.disp, has_imm, wide);
    }
    _ = d;
}

fn enc_inst(i: *mut mc.MInst) void {
    let op = i.op;
    let sz = i.sz;
    if op == xe.MOV_RR {
        let d = P(i.ops[0].reg);
        let s = P(i.ops[1].reg);
        if d == s {
            return;
        }
        enc_op(sz, 1, 0x8b, 0, d, R(i.ops[1].reg), 0, false);
        return;
    }
    if op == xe.MOV_RI {
        let d = P(i.ops[0].reg);
        let x = i.ops[1].disp;
        if sz == 8 and not ((x >= 0) and (x <= 0xffff_ffff)) {
            if fits32(x) {
                b8(0x48 | @as(u64, @bit_cast(u32, (d >> 3) & 1)));
                b8(0xc7);
                b8(0xc0 | @as(u64, @bit_cast(u32, d & 7)));
                E.b.u32_(@bit_cast(u64, x));
                return;
            }
            b8(0x48 | @as(u64, @bit_cast(u32, (d >> 3) & 1)));
            b8(0xb8 + @as(u64, @bit_cast(u32, d & 7)));
            E.b.u64_(@bit_cast(u64, x));
            return;
        }
        if d >= 8 {
            b8(0x41);
        }
        b8(0xb8 + @as(u64, @bit_cast(u32, d & 7)));
        E.b.u32_(@bit_cast(u64, x));
        return;
    }
    if op == xe.LOAD {
        let d = P(i.ops[0].reg);
        let m = i.ops[1];
        if m.scale == 99 {
            // mov r64, [rip + sym@GOTPCREL]
            b8(0x48 | (@as(u64, @bit_cast(u32, (d >> 3) & 1)) << 2));
            b8(0x8b);
            b8(0x05 | (@as(u64, @bit_cast(u32, d & 7)) << 3));
            E.relocs.push(xe.Reloc.{ off: @as(u64, E.b.len()), ty: elf.R_X86_64_REX_GOTPCRELX, sym: m.sym, addend: -4 });
            E.b.u32_(0);
            return;
        }
        let k = i.cc;
        if (k == 1) and (sz <= 2) {
            enc_op(4, 2, 0x0f, u.pick(sz == 1, "\xb6", "\xb7")[0], d, m, 0, false);
            return;
        }
        if (k == 2) and (sz <= 2) {
            enc_op(4, 2, 0x0f, u.pick(sz == 1, "\xbe", "\xbf")[0], d, m, 0, false);
            return;
        }
        if sz == 1 {
            enc_op(4, 2, 0x0f, 0xb6, d, m, 0, false);
            return;
        }
        enc_op(sz, 1, 0x8b, 0, d, m, 0, false);
        return;
    }
    if op == xe.STORE {
        let s = P(i.ops[1].reg);
        if sz == 1 {
            enc_op(1, 1, 0x88, 0, s, i.ops[0], 0, true);
        } else {
            enc_op(sz, 1, 0x89, 0, s, i.ops[0], 0, false);
        }
        return;
    }
    if op == xe.STORE_I {
        let x = i.ops[1].disp;
        if sz == 1 {
            enc_op(1, 1, 0xc6, 0, 0, i.ops[0], 1, false);
            enc_imm(x, 1);
        } else {
            enc_op(sz, 1, 0xc7, 0, 0, i.ops[0], imm_len(sz), false);
            enc_imm(x, imm_len(sz));
        }
        return;
    }
    if op == xe.LEA {
        enc_op(8, 1, 0x8d, 0, P(i.ops[0].reg), i.ops[1], 0, false);
        return;
    }
    if op == xe.ALU {
        let digit = i.cc;
        let d = P(i.ops[0].reg);
        let s = i.ops[1];
        if s.kind == mc.K_IMM {
            let x = s.disp;
            if sz == 1 {
                enc_op(1, 1, 0x80, 0, digit, R(i.ops[0].reg), 1, true);
                enc_imm(x, 1);
            } else if (x >= -128) and (x <= 127) {
                enc_op(sz, 1, 0x83, 0, digit, R(i.ops[0].reg), 1, false);
                enc_imm(x, 1);
            } else {
                enc_op(sz, 1, 0x81, 0, digit, R(i.ops[0].reg), imm_len(sz), false);
                enc_imm(x, imm_len(sz));
            }
            return;
        }
        let base = @truncate(u8, @bit_cast(u32, digit)) * 8;
        if sz == 1 {
            enc_op(1, 1, base + 2, 0, d, s, 0, true);
        } else {
            enc_op(sz, 1, base + 3, 0, d, s, 0, false);
        }
        return;
    }
    if op == xe.TEST {
        let a = i.ops[0];
        let s = i.ops[1];
        if s.kind == mc.K_IMM {
            if sz == 1 {
                enc_op(1, 1, 0xf6, 0, 0, a, 1, true);
                enc_imm(s.disp, 1);
            } else {
                enc_op(sz, 1, 0xf7, 0, 0, a, imm_len(sz), false);
                enc_imm(s.disp, imm_len(sz));
            }
            return;
        }
        if sz == 1 {
            enc_op(1, 1, 0x84, 0, P(s.reg), a, 0, true);
        } else {
            enc_op(sz, 1, 0x85, 0, P(s.reg), a, 0, false);
        }
        return;
    }
    if op == xe.IMUL {
        enc_op(sz, 2, 0x0f, 0xaf, P(i.ops[0].reg), i.ops[1], 0, false);
        return;
    }
    if op == xe.IMUL_I {
        let x = i.ops[2].disp;
        if (x >= -128) and (x <= 127) {
            enc_op(sz, 1, 0x6b, 0, P(i.ops[0].reg), i.ops[1], 1, false);
            enc_imm(x, 1);
        } else {
            enc_op(sz, 1, 0x69, 0, P(i.ops[0].reg), i.ops[1], 4, false);
            enc_imm(x, 4);
        }
        return;
    }
    if op == xe.SHIFT {
        let digit = i.cc;
        let dst = i.ops[0];
        if i.ops[2].kind == mc.K_IMM {
            if sz == 1 {
                enc_op(1, 1, 0xc0, 0, digit, dst, 1, true);
            } else {
                enc_op(sz, 1, 0xc1, 0, digit, dst, 1, false);
            }
            enc_imm(i.ops[2].disp, 1);
        } else if sz == 1 {
            enc_op(1, 1, 0xd2, 0, digit, dst, 0, true);
        } else {
            enc_op(sz, 1, 0xd3, 0, digit, dst, 0, false);
        }
        return;
    }
    if op == xe.SHIFTX {
        // VEX.LZ.{66,F2,F3}.0F38.W F7 /r: reg = dst, rm = src, vvvv = count
        let d = P(i.ops[0].reg);
        let s = P(i.ops[1].reg);
        let c = P(i.ops[2].reg);
        let mut pp: u64 = 1;
        if i.cc == 1 {
            pp = 3;
        } else if i.cc == 2 {
            pp = 2;
        }
        let mut w: u64 = 0;
        if sz == 8 {
            w = 1;
        }
        b8(0xc4);
        b8(((((~@as(u64, @bit_cast(u32, d >> 3))) & 1) << 7) | (1 << 6)) | ((((~@as(u64, @bit_cast(u32, s >> 3))) & 1) << 5) | 2));
        b8(((w << 7) | (((~@as(u64, @bit_cast(u32, c))) & 15) << 3)) | pp);
        b8(0xf7);
        xe.modrm_rr(&mut E, @as(u64, @bit_cast(u32, d)), @as(u64, @bit_cast(u32, s)));
        return;
    }
    if op == xe.UNARY {
        if sz == 1 {
            enc_op(1, 1, 0xf6, 0, i.cc, i.ops[0], 0, true);
        } else {
            enc_op(sz, 1, 0xf7, 0, i.cc, i.ops[0], 0, false);
        }
        return;
    }
    if op == xe.EXT {
        let d = P(i.ops[0].reg);
        let s = i.ops[1];
        let k = i.cc;
        if k == 1 {
            enc_op(4, 2, 0x0f, 0xb6, d, s, 0, true);
        } else if k == 2 {
            enc_op(4, 2, 0x0f, 0xb7, d, s, 0, false);
        } else if k == 3 {
            enc_op(sz, 2, 0x0f, 0xbe, d, s, 0, true);
        } else if k == 4 {
            enc_op(sz, 2, 0x0f, 0xbf, d, s, 0, false);
        } else if k == 5 {
            enc_op(8, 1, 0x63, 0, d, s, 0, false);
        } else {
            enc_op(4, 1, 0x8b, 0, d, s, 0, false);
        }
        return;
    }
    if op == xe.CQO {
        if sz == 8 {
            b8(0x48);
        }
        b8(0x99);
        return;
    }
    if op == xe.DIV {
        enc_op(sz, 1, 0xf7, 0, i.cc, i.ops[2], 0, false);
        return;
    }
    if op == xe.SETCC {
        enc_op(1, 2, 0x0f, 0x90 + @truncate(u8, @bit_cast(u32, i.cc)), 0, i.ops[0], 0, true);
        return;
    }
    if op == xe.CMOV {
        enc_op(sz, 2, 0x0f, 0x40 + @truncate(u8, @bit_cast(u32, i.cc)), P(i.ops[0].reg), i.ops[1], 0, false);
        return;
    }
    if op == xe.BITSCAN {
        let k = i.cc;
        if k >= 2 {
            b8(0xf3);
        }
        let mut opc: u8 = 0xbc;
        if (k == 1) or (k == 3) {
            opc = 0xbd;
        } else if k == 4 {
            opc = 0xb8;
        }
        enc_op(sz, 2, 0x0f, opc, P(i.ops[0].reg), i.ops[1], 0, false);
        return;
    }
    if op == xe.JMP {
        b8(0xe9);
        label_ref(i.ops[0].label);
        return;
    }
    if op == xe.JCC {
        b8(0x0f);
        b8(0x80 + @as(u64, @bit_cast(u32, i.cc)));
        label_ref(i.ops[0].label);
        return;
    }
    if op == xe.LABEL {
        let l = @truncate(i32, i.aux) - xe.LOCAL_BASE;
        while local_off.len <= @int_cast(usize, l) {
            local_off.push(0);
        }
        local_off.set(@int_cast(usize, l), @as(u64, E.b.len()));
        return;
    }
    if op == xe.CALL {
        let t = i.ops[0];
        if t.kind == mc.K_SYM {
            b8(0xe8);
            E.relocs.push(xe.Reloc.{ off: @as(u64, E.b.len()), ty: elf.R_X86_64_PLT32, sym: t.sym, addend: -4 });
            E.b.u32_(0);
        } else {
            enc_op(4, 1, 0xff, 0, 2, t, 0, false);
        }
        return;
    }
    if op == xe.RET {
        epilogue();
        return;
    }
    if op == xe.UD2 {
        b8(0x0f);
        b8(0x0b);
        return;
    }
    if op == xe.XCHG {
        let r = P(i.ops[1].reg);
        if sz == 1 {
            enc_op(1, 1, 0x86, 0, r, i.ops[0], 0, true);
        } else {
            enc_op(sz, 1, 0x87, 0, r, i.ops[0], 0, false);
        }
        return;
    }
    if op == xe.XADD {
        b8(0xf0);
        let r = P(i.ops[1].reg);
        if sz == 1 {
            enc_op(1, 2, 0x0f, 0xc0, r, i.ops[0], 0, true);
        } else {
            enc_op(sz, 2, 0x0f, 0xc1, r, i.ops[0], 0, false);
        }
        return;
    }
    if op == xe.CMPXCHG {
        b8(0xf0);
        let r = P(i.ops[1].reg);
        if sz == 1 {
            enc_op(1, 2, 0x0f, 0xb0, r, i.ops[0], 0, true);
        } else {
            enc_op(sz, 2, 0x0f, 0xb1, r, i.ops[0], 0, false);
        }
        return;
    }
    if op == xe.MFENCE {
        b8(0x0f);
        b8(0xae);
        b8(0xf0);
        return;
    }
    if op == xe.TLSBASE {
        let d = P(i.ops[0].reg);
        b8(0x64);
        b8(0x48 | (@as(u64, @bit_cast(u32, (d >> 3) & 1)) << 2));
        b8(0x8b);
        b8(0x04 | (@as(u64, @bit_cast(u32, d & 7)) << 3));
        b8(0x25);
        E.b.u32_(0);
        return;
    }
    if op == xe.SSE {
        enc_sse(i);
        return;
    }
    if op == xe.REP {
        if i.cc == 0 {
            b8(0xf3);
            b8(0xa4);
        } else if i.cc == 1 {
            b8(0xf3);
            b8(0xaa);
        } else {
            // memmove: backwards when the destination overlaps the source's end
            // mov rax, rdi; sub rax, rsi; cmp rax, rcx; jae fwd
            let seq: [9]u8 = [0x48, 0x89, 0xf8, 0x48, 0x29, 0xf0, 0x48, 0x39, 0xc8];
            E.b.put(seq[..]);
            b8(0x73);
            b8(0x10);  // jae +16 (over the backward copy)
            // lea rsi, [rsi+rcx-1]; lea rdi, [rdi+rcx-1]; std; rep movsb; cld; jmp done
            let back: [17]u8 = [0x48, 0x8d, 0x74, 0x0e, 0xff, 0x48, 0x8d, 0x7c, 0x0f, 0xff, 0xfd, 0xf3, 0xa4, 0xfc, 0xeb,
                                0x02, 0x90];
            E.b.put(back[0..16]);
            // fwd: rep movsb
            b8(0xf3);
            b8(0xa4);
        }
        return;
    }
    if op == xe.NOP {
        return;
    }
    fail(u.f1("cannot encode machine op %s", u.idec(op)));
}

fn encode_func(f: *mut ir.Func) void {
    // callee-saved registers in use
    nsaved = 0;
    let cs: [5]i32 = [xe.RBX, xe.R12, xe.R13, xe.R14, xe.R15];
    for r in cs {
        if (MF.used_regs & (@as(u64, 1) << @int_cast(u32, r))) != 0 {
            saved[@int_cast(usize, nsaved)] = r;
            nsaved += 1;
        }
    }
    // frame layout (slots below the saved registers)
    let mut cur: i64 = 8 * nsaved;
    for k in 0..MF.slots.len {
        let s = MF.slots.at(k);
        let a = @bit_cast(i64, s.align);
        cur = (cur + @bit_cast(i64, s.size) + a - 1) & ~(a - 1);
        s.offset = -cur;
    }
    let total = (cur + @bit_cast(i64, out_area) + 15) & ~@as(i64, 15);
    let sub = total - (8 * nsaved);
    E = xe.Enc.{ b: bn.new_bytes(), relocs: u.List.[xe.Reloc].{ data: u.nil.[xe.Reloc](), len: 0, cap: 0 },
                 fixups: u.List.[xe.Fixup].{ data: u.nil.[xe.Fixup](), len: 0, cap: 0 }, avx: AVX };
    local_off.clear();
    prologue(sub);
    let nb = MF.blocks.len;
    let boff = u.newn.[u64](nb + 1);
    for bi in 0..nb {
        let b = MF.blocks.get(bi);
        boff[bi] = @as(u64, E.b.len());
        let mut i = b.first;
        while u.notnil(i) {
            enc_inst(i);
            i = i.next;
        }
    }
    // branch targets
    for k in 0..E.fixups.len {
        let x = E.fixups.get(k);
        let mut target: u64 = 0;
        if x.label >= xe.LOCAL_BASE {
            target = local_off.get(@int_cast(usize, x.label - xe.LOCAL_BASE));
        } else {
            target = boff[@int_cast(usize, x.label)];
        }
        let rel = @bit_cast(i64, target) - @bit_cast(i64, x.off + 4);
        E.b.set_u32(@truncate(usize, x.off), @bit_cast(u64, rel));
    }
    // into .text
    let text = OBJ.secs.get(@int_cast(usize, TEXT));
    text.data.align(16);
    let base = @as(u64, text.data.len());
    text.data.put(E.b.view());
    for k in 0..E.relocs.len {
        let r = E.relocs.get(k);
        text.relocs.push(elf.Rel.{ off: base + r.off, ty: r.ty, sym: @bit_cast(u32, r.sym), addend: r.addend });
    }
    let s = OBJ.syms.at(@as(usize, fsym.get(@int_cast(usize, f.id))));
    s.value = base;
    s.size = @as(u64, E.b.len());
    E.b.free();
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
        s.relocs.push(elf.Rel.{ off: base, ty: elf.R_X86_64_64, sym: gsym.get(@int_cast(usize, x.gv.id)), addend: 0 });
        return;
    }
    if k == ir.V_FUNC {
        s.relocs.push(elf.Rel.{ off: base, ty: elf.R_X86_64_64, sym: fsym.get(@int_cast(usize, x.fnp.id)), addend: 0 });
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
    AVX = tg.has(tg.X_AVX);
    OBJ = elf.new_obj(elf.EM_X86_64);
    consts = u.SymMap.[u32].{ keys: u.nil.[usize](), vals: u.nil.[u32](), cap: 0, len: 0 };
    TEXT = elf.add_sec(OBJ, ".text", elf.S_TEXT, 16);
    RODATA = elf.add_sec(OBJ, ".rodata", elf.S_RODATA, 16);
    let data = elf.add_sec(OBJ, ".data", elf.S_DATA, 16);
    let bss = elf.add_sec(OBJ, ".bss", elf.S_BSS, 16);
    let tbss = elf.add_sec(OBJ, ".tbss", elf.S_TBSS, 16);
    let tdata = elf.add_sec(OBJ, ".tdata", elf.S_TDATA, 16);
    let relro = elf.add_sec(OBJ, ".data.rel.ro", elf.S_DATA, 16);
    fsym.clear();
    gsym.clear();
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
                sec = relro;  // relocated at load time
            }
        } else if ir.is_zero(g.init) {
            sec = bss;
        }
        let s = OBJ.secs.get(@int_cast(usize, sec));
        let mut al = g.align;
        if al == 0 {
            al = 1;
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
