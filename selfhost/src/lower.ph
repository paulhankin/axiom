// =============================================================================
// lower.ph - makes the IR scalar: aggregate (struct and array) SSA values
// become memory, and the internal calling convention passes aggregates by
// address.
//
// Every aggregate value gets a *slot*, the address of memory holding it:
//   - a load copies into a fresh stack slot;
//   - insertvalue writes into its operand's slot when it is the operand's
//     only use (the usual chain that builds a struct literal), and into a
//     copy otherwise;
//   - extractvalue of an aggregate field is an address inside the slot, of
//     a scalar field a load from it;
//   - a constant aggregate is a read-only global;
//   - aggregate parameters arrive as addresses, and aggregate results are
//     written through a hidden first parameter (sret).
// Slots are never written after their value is complete, so aliasing a slot
// (a field address, a select of two slots) is safe.
// =============================================================================
import "util.ph" as u;
import "ir.ph" as ir;

let mut M: *mut ir.Module = @ptr_from_int(*mut ir.Module, 0);
let mut B: *mut ir.Builder = @ptr_from_int(*mut ir.Builder, 0);
let mut F: *mut ir.Func = @ptr_from_int(*mut ir.Func, 0);
let mut sret_calls: u.List.[*mut ir.Value] = u.List.[*mut ir.Value].{ data: @ptr_from_int(*mut *mut ir.Value, 0), len: 0, cap: 0 };
let mut const_globals: u.SymMap.[*mut ir.Value] = u.SymMap.[*mut ir.Value].{
    keys: @ptr_from_int(*mut usize, 0), vals: @ptr_from_int(*mut *mut ir.Value, 0), cap: 0, len: 0 };

fn vnil() *mut ir.Value {
    return u.nil.[ir.Value]();
}

fn agg(t: *ir.Ty) bool {
    return ir.is_agg(t);
}

fn isz(v: u64) *mut ir.Value {
    return ir.cint(ir.isz(), v);
}

// The signature with aggregates passed by address and returned through sret.
fn lower_fty(t: *mut ir.Ty) *mut ir.Ty {
    let mut any = agg(t.ret);
    for p in t.params {
        if agg(p) {
            any = true;
        }
    }
    if not any {
        return t;
    }
    let n = t.params.len;
    let ps = u.newn.[*mut ir.Ty](n + 2);
    let mut k: usize = 0;
    let mut rt = t.ret;
    if agg(t.ret) {
        ps[0] = ir.t_ptr();
        k = 1;
        rt = ir.t_void();
    }
    for p in t.params {
        if agg(p) {
            ps[k] = ir.t_ptr();
        } else {
            ps[k] = p;
        }
        k += 1;
    }
    return ir.t_fn(rt, ps[0..k], t.vararg);
}

fn lower_sig(f: *mut ir.Func) void {
    let nt = lower_fty(f.ty);
    if nt == f.ty {
        return;
    }
    let sret = agg(f.ty.ret);
    let n = f.params.len;
    let mut k: usize = 0;
    let ps = u.newn.[*mut ir.Value](n + 2);
    let ext = u.newn.[u8](n + 2);
    if sret {
        ps[0] = ir.new_param(f, ir.t_ptr(), 0);
        k = 1;
    }
    for i in 0..n {
        let p = f.params[i];
        if agg(p.ty) {
            let np = ir.new_param(f, ir.t_ptr(), k);
            p.auxp = @ptr_cast(*mut void, np);  // the slot of the aggregate parameter
            ps[k] = np;
        } else {
            p.cv = @as(u64, k);
            ps[k] = p;
        }
        ext[k] = f.param_ext[i];
        k += 1;
    }
    f.params = ps[0..k];
    f.param_ext = ext[0..k];
    f.ty = nt;
    f.sret = sret;
    if sret {
        f.ret_ext = 0;
    }
}

fn entry_alloca(t: *mut ir.Ty) *mut ir.Value {
    let saved_blk = B.blk;
    let saved_before = B.before;
    let e = F.first;
    if u.notnil(e.first) {
        ir.at_before(B, e.first);
    } else {
        ir.at_end(B, e);
    }
    let a = ir.alloca(B, t, "agg");
    B.blk = saved_blk;
    B.before = saved_before;
    return a;
}

fn slot(v: *mut ir.Value) *mut ir.Value {
    return @ptr_cast(*mut ir.Value, v.auxp);
}

fn set_slot(v: *mut ir.Value, s: *mut ir.Value) void {
    v.auxp = @ptr_cast(*mut void, s);
}

// A read-only global holding constant c.
fn const_global(c: *mut ir.Value) *mut ir.Value {
    let k = u.addr_key(@int_from_ptr(c));
    let found = const_globals.get(k);
    if u.notnil(found) {
        return found;
    }
    let g = ir.add_global(M, c.ty, ".const");
    g.init = c;
    g.is_const = true;
    g.linkage = ir.LINK_INTERNAL;
    const_globals.put(k, g.val);
    return g.val;
}

// The address of aggregate value x (at the builder's position).
fn slot_of(x: *mut ir.Value) *mut ir.Value {
    let k = x.kind;
    if (k == ir.V_INST) or (k == ir.V_PARAM) {
        let s = slot(x);
        if u.isnil(s) {
            u.die(u.f1("internal compiler error: aggregate without a slot: %s", ir.val_str(x)));
        }
        return s;
    }
    if k == ir.V_UNDEF {
        return entry_alloca(x.ty);
    }
    return const_global(x);
}

fn memcpy(d: *mut ir.Value, s: *mut ir.Value, n: u64) void {
    if n == 0 {
        return;
    }
    _ = ir.memop(B, ir.OP_MEMCPY, d, s, isz(n));
}

// Stores constant c at address d.
fn store_const(c: *mut ir.Value, d: *mut ir.Value) void {
    let t = c.ty;
    if c.kind == ir.V_UNDEF {
        return;
    }
    if not agg(t) {
        let st = ir.store(B, c, d);
        st.align = @int_cast(u32, t.align);
        return;
    }
    if ir.is_zero(c) {
        _ = ir.memop(B, ir.OP_MEMSET, d, ir.cint(ir.t_int(8), 0), isz(t.size));
        return;
    }
    let n = ir.field_count(t);
    if n > 16 {
        memcpy(d, const_global(c), t.size);
        return;
    }
    for i in 0..n {
        store_const(c.xs.get(i), ir.ptradd(B, d, isz(ir.field_off(t, i))));
    }
}

// Stores aggregate value x at address d.
fn store_agg(x: *mut ir.Value, d: *mut ir.Value) void {
    if ir.is_const(x) {
        store_const(x, d);
        return;
    }
    memcpy(d, slot_of(x), x.ty.size);
}

fn field_const(c: *mut ir.Value, i: usize) *mut ir.Value {
    let ft = ir.field_ty(c.ty, i);
    if c.kind == ir.V_CAGG {
        return c.xs.get(i);
    }
    if c.kind == ir.V_ZERO {
        return ir.czero(ft);
    }
    return ir.undef(ft);
}

fn count_uses(f: *mut ir.Func) void {
    let mut b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            i.uses = 0;
            i = i.next;
        }
        b = b.next;
    }
    b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            let ops: [3]*mut ir.Value = [i.a, i.b, i.c];
            for o in ops {
                if u.notnil(o) {
                    o.uses += 1;
                }
            }
            for k in 0..i.xs.len {
                i.xs.get(k).uses += 1;
            }
            i = i.next;
        }
        b = b.next;
    }
}

fn lower_call(v: *mut ir.Value) void {
    let old = v.fty;
    let nt = lower_fty(old);
    if nt == old {
        return;
    }
    let sret = agg(old.ret);
    let n = v.xs.len;
    let mut args = u.List.[*mut ir.Value].{ data: u.nil.[*mut ir.Value](), len: 0, cap: 0 };
    let ext = u.newn.[u8](n + 3);
    let mut k: usize = 1;
    if sret {
        let s = entry_alloca(old.ret);
        args.push(s);
        set_slot(v, s);
        v.aux2 = 1;
        k = 2;
    }
    for i in 0..n {
        let x = v.xs.get(i);
        if agg(x.ty) {
            args.push(slot_of(x));
        } else {
            args.push(x);
        }
        if (i + 1) < v.ext.len {
            ext[k] = v.ext[i + 1];
        }
        k += 1;
    }
    v.xs = args;
    v.ext = ext[0..k];
    v.fty = nt;
    if sret {
        sret_calls.push(v);  // typed void once every use is lowered
    }
}

fn lower_body(f: *mut ir.Func) void {
    F = f;
    count_uses(f);
    let mut phis = u.List.[*mut ir.Value].{ data: u.nil.[*mut ir.Value](), len: 0, cap: 0 };
    // aggregate phis get their slots first (their inputs may come later)
    let mut b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) and (i.op == ir.OP_PHI) {
            if agg(i.ty) {
                set_slot(i, entry_alloca(i.ty));
                i.aux2 = 1;
                phis.push(i);
            }
            i = i.next;
        }
        b = b.next;
    }
    let mut sret_p = vnil();
    if f.sret {
        sret_p = f.params[0];
    }
    b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            let next = i.next;
            ir.at_before(B, i);
            let op = i.op;
            if (op == ir.OP_LOAD) and agg(i.ty) {
                let s = entry_alloca(i.ty);
                memcpy(s, i.a, i.ty.size);
                set_slot(i, s);
                i.aux2 = 1;
                ir.remove_inst(i);
            } else if (op == ir.OP_STORE) and agg(i.a.ty) {
                store_agg(i.a, i.b);
                ir.remove_inst(i);
            } else if op == ir.OP_INSERTVALUE {
                let base = i.a;
                let mut s = vnil();
                if base.kind == ir.V_UNDEF {
                    s = entry_alloca(i.ty);
                } else if (base.kind == ir.V_INST) and (base.uses == 1) and (base.aux2 == 1) {
                    s = slot(base);  // the only use: build in place
                } else if ir.is_const(base) {
                    s = entry_alloca(i.ty);
                    store_const(base, s);
                } else {
                    s = entry_alloca(i.ty);
                    memcpy(s, slot_of(base), i.ty.size);
                }
                let fi = @truncate(usize, i.cv);
                let ft = ir.field_ty(i.ty, fi);
                let d = ir.ptradd(B, s, isz(ir.field_off(i.ty, fi)));
                if agg(ft) {
                    store_agg(i.b, d);
                } else {
                    let st = ir.store(B, i.b, d);
                    st.align = @int_cast(u32, ft.align);
                }
                set_slot(i, s);
                i.aux2 = 1;
                ir.remove_inst(i);
            } else if op == ir.OP_EXTRACTVALUE {
                let a = i.a;
                let fi = @truncate(usize, i.cv);
                if ir.is_const(a) {
                    let c = field_const(a, fi);
                    if agg(i.ty) {
                        set_slot(i, slot_of(c));
                    } else {
                        i.fwd = c;
                    }
                } else {
                    let p = ir.ptradd(B, slot_of(a), isz(ir.field_off(a.ty, fi)));
                    if agg(i.ty) {
                        set_slot(i, p);
                    } else {
                        let l = ir.load(B, i.ty, p);
                        i.fwd = l;
                    }
                }
                ir.remove_inst(i);
            } else if (op == ir.OP_SELECT) and agg(i.ty) {
                set_slot(i, ir.select(B, i.a, slot_of(i.b), slot_of(i.c)));
                ir.remove_inst(i);
            } else if op == ir.OP_CALL {
                lower_call(i);
            } else if (op == ir.OP_RET) and u.notnil(i.a) and agg(i.a.ty) {
                store_agg(i.a, sret_p);
                i.a = vnil();
            }
            i = next;
        }
        b = b.next;
    }
    // phi copies at the end of each predecessor
    for k in 0..phis.len {
        let p = phis.get(k);
        for j in 0..p.xs.len {
            let pred = p.bs.get(j);
            ir.at_before(B, ir.terminator(pred));
            store_agg(p.xs.get(j), slot(p));
        }
        ir.remove_inst(p);
    }
    u.free(@ptr_cast(*mut void, phis.data));
    for k in 0..sret_calls.len {
        sret_calls.get(k).ty = ir.t_void();
    }
    sret_calls.clear();
    ir.apply_forwarding(f);
}

pub fn lower_module(m: *mut ir.Module) void {
    M = m;
    B = ir.new_builder();
    let nf = m.funcs.len;
    for i in 0..nf {
        lower_sig(m.funcs.get(i));
    }
    for i in 0..nf {
        let f = m.funcs.get(i);
        if not ir.is_decl(f) {
            lower_body(f);
        }
    }
}

// -----------------------------------------------------------------------------
// Vector legalization: vector instructions the target cannot select become
// lane-by-lane scalar code (extract, scalar operation, insert).
// -----------------------------------------------------------------------------
fn lane_of(x: *mut ir.Value, i: u64) *mut ir.Value {
    if (x.kind == ir.V_INST) and (x.op == ir.OP_SPLAT) {
        return x.a;
    }
    if x.kind == ir.V_CAGG {
        return x.xs.get(@truncate(usize, i));
    }
    if x.kind == ir.V_ZERO {
        return ir.czero(x.ty.elem);
    }
    return ir.extract_elem(B, x, i);
}

fn scalarize_one(v: *mut ir.Value) void {
    ir.at_before(B, v);
    let o = v.op;
    let mut vt = v.ty;
    if (o == ir.OP_INTR) and (vt.kind != ir.T_VEC) {
        vt = v.xs.get(0).ty;
    }
    let n = @as(u64, @bit_cast(u32, vt.lanes));
    let id = @truncate(i32, @bit_cast(i64, v.cv));
    if (o == ir.OP_INTR) and (id == ir.IN_ALLTRUE) {
        let x = v.xs.get(0);
        let mut acc = ir.cint(ir.t_int(1), 1);
        for i in @as(u64, 0)..n {
            let l = lane_of(x, i);
            acc = ir.and_(B, acc, ir.icmp(B, ir.IntNE, l, ir.czero(l.ty)));
        }
        v.fwd = acc;
        ir.remove_inst(v);
        return;
    }
    let mut r = ir.undef(v.ty);
    for i in @as(u64, 0)..n {
        let mut s = vnil();
        if (o >= ir.OP_ADD) and (o <= ir.OP_FREM) {
            s = ir.binop(B, o, lane_of(v.a, i), lane_of(v.b, i));
        } else if o == ir.OP_FNEG {
            s = ir.fneg(B, lane_of(v.a, i));
        } else if (o == ir.OP_ICMP) or (o == ir.OP_FCMP) {
            let p = @truncate(i32, @bit_cast(i64, v.cv));
            let mut c = vnil();
            if o == ir.OP_ICMP {
                c = ir.icmp(B, p, lane_of(v.a, i), lane_of(v.b, i));
            } else {
                c = ir.fcmp(B, p, lane_of(v.a, i), lane_of(v.b, i));
            }
            s = ir.sext(B, c, v.ty.elem);
        } else if (o >= ir.OP_ZEXT) and (o <= ir.OP_FPTOUI_SAT) {
            s = ir.cast(B, o, lane_of(v.a, i), v.ty.elem);
        } else if o == ir.OP_INTR {
            let args = u.newn.[*mut ir.Value](v.xs.len + 1);
            for k in 0..v.xs.len {
                args[k] = lane_of(v.xs.get(k), i);
            }
            s = ir.intr(B, id, v.ty.elem, args[0..v.xs.len]);
        } else if o == ir.OP_SELECT {
            s = ir.select(B, v.a, lane_of(v.b, i), lane_of(v.c, i));
        } else {
            u.die(u.f1("internal compiler error: cannot split vector instruction %s", ir.inst_str(v)));
        }
        r = ir.insert_elem(B, r, s, i);
    }
    v.fwd = r;
    ir.remove_inst(v);
}

pub fn scalarize(f: *mut ir.Func, legal: fn(*mut ir.Value) bool) void {
    B = ir.new_builder();
    F = f;
    let mut any = false;
    let mut b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            let next = i.next;
            let vecish = (i.ty.kind == ir.T_VEC) or ((i.op == ir.OP_INTR) and (i.xs.len > 0) and
                                                     (i.xs.get(0).ty.kind == ir.T_VEC)) or
                         ((i.op == ir.OP_ICMP) and (i.a.ty.kind == ir.T_VEC));
            if vecish and not legal(i) {
                scalarize_one(i);
                any = true;
            }
            i = next;
        }
        b = b.next;
    }
    if any {
        ir.apply_forwarding(f);
    }
}
