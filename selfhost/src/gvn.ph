// =============================================================================
// gvn.ph - global value numbering over the dominator tree (in the style of
// LLVM's EarlyCSE, with memory):
//
//   - an instruction without effects that repeats one that dominates it is
//     replaced by it (commutative operands in either order);
//   - a load of a location whose value is known (an earlier load or store,
//     with no possibly aliasing write in between) is replaced by that value,
//     and a store of the value a location already holds is deleted;
//   - below a conditional branch, its condition is a constant, and so are
//     comparisons it implies (the same, inverted or swapped), the operands of
//     an `and` known true or an `or` known false, and x in `x == K`.
//
// Writes between a block's immediate dominator and the block (other paths
// into a join, or a loop's body) are collected so known values survive
// joins and loops that do not write them.
//
// Alias analysis: a pointer is a base plus a constant offset; the object
// under a chain of ptradds is identified when it is an alloca or a global.
// Distinct identified objects never alias, an alloca whose address never
// escapes is not written by calls, and no computed pointer reaches a noalias
// global (the shadow stack).
// =============================================================================
import "util.ph" as u;
import "ir.ph" as ir;
import "opt.ph" as opt;
import "combine.ph" as cb;

fn vnil() *mut ir.Value {
    return u.nil.[ir.Value]();
}

const K_EXPR: i32 = 0;   // key: an instruction (representative), val: its value
const K_MEM: i32 = 1;    // key: base pointer, off, ty; val: the value in memory
const K_KNOWN: i32 = 2;  // key: a value, val: the constant it equals

struct Ent {
    kind: i32,
    key: *mut ir.Value,
    off: i64,
    ty: *mut ir.Ty,
    val: *mut ir.Value,
    height: usize,
    next: i64,
    bucket: usize,
}

const W_LOC: i32 = 0;     // a store / memory operation on one location
const W_CALL: i32 = 1;    // everything but non-escaping allocas
const W_ALL: i32 = 2;     // everything

struct Write {
    kind: i32,
    base: *mut ir.Value,
    obj: *mut ir.Value,
    off: i64,
    size: i64,            // -1: unknown
}

struct Loc {
    base: *mut ir.Value,
    obj: *mut ir.Value,   // alloca or global under the ptradds, or nil
    off: i64,
    size: i64,
}

let mut ents: u.List.[Ent] = u.List.[Ent].{ data: @ptr_from_int(*mut Ent, 0), len: 0, cap: 0 };
let mut writes: u.List.[Write] = u.List.[Write].{ data: @ptr_from_int(*mut Write, 0), len: 0, cap: 0 };
let mut bk: u.List.[i64] = u.List.[i64].{ data: @ptr_from_int(*mut i64, 0), len: 0, cap: 0 };
let mut nb: usize = 0;
let mut B: *mut ir.Builder = @ptr_from_int(*mut ir.Builder, 0);
let mut changed: bool = false;
let mut g_fn: *mut ir.Func = @ptr_from_int(*mut ir.Func, 0);

// -----------------------------------------------------------------------------
// Hashing
// -----------------------------------------------------------------------------
fn hp(p: *ir.Value) u64 {
    let x = @as(u64, @int_from_ptr(p));
    return (x >> 4) *% 0x9E37_79B9_7F4A_7C15;
}

fn mix(h: u64, x: u64) u64 {
    return (h ^ x) *% 0x1000_0000_01B3 +% 0x7F4A_7C15;
}

fn commutative(op: i32) bool {
    return (op == ir.OP_ADD) or (op == ir.OP_MUL) or (op == ir.OP_AND) or (op == ir.OP_OR) or (op == ir.OP_XOR) or
           (op == ir.OP_FADD) or (op == ir.OP_FMUL);
}

// Operand identity: constants compare by value.
fn same_val(a: *ir.Value, b: *ir.Value) bool {
    if a == b {
        return true;
    }
    if u.isnil(a) or u.isnil(b) {
        return false;
    }
    if (a.kind == ir.V_CINT) and (b.kind == ir.V_CINT) {
        return (a.ty == b.ty) and (a.cv == b.cv);
    }
    if (a.kind == ir.V_ZERO) and (b.kind == ir.V_ZERO) {
        return a.ty == b.ty;
    }
    if (a.kind == ir.V_CFP) and (b.kind == ir.V_CFP) {
        return (a.ty == b.ty) and (@bit_cast(u64, a.fv) == @bit_cast(u64, b.fv));
    }
    return false;
}

fn hv(v: *ir.Value) u64 {
    if u.isnil(v) {
        return 7;
    }
    if v.kind == ir.V_CINT {
        return mix(@as(u64, @bit_cast(u32, v.ty.id)), v.cv);
    }
    if v.kind == ir.V_ZERO {
        return mix(@as(u64, @bit_cast(u32, v.ty.id)), 0x5a5a);
    }
    if v.kind == ir.V_CFP {
        return mix(@as(u64, @bit_cast(u32, v.ty.id)), @bit_cast(u64, v.fv));
    }
    return hp(v);
}

fn hash_expr(i: *ir.Value) u64 {
    let mut h = mix(@as(u64, @bit_cast(u32, i.op)), @as(u64, @bit_cast(u32, i.ty.id)));
    h = mix(h, i.cv);
    if commutative(i.op) {
        h = mix(h, hv(i.a) +% hv(i.b));
    } else {
        h = mix(h, hv(i.a));
        h = mix(h, hv(i.b));
    }
    h = mix(h, hv(i.c));
    for k in 0..i.xs.len {
        h = mix(h, hv(i.xs.get(k)));
    }
    for k in 0..i.mask.len {
        h = mix(h, @as(u64, @bit_cast(u32, i.mask[k])));
    }
    return h;
}

fn same_expr(x: *ir.Value, y: *ir.Value) bool {
    if (x.op != y.op) or (x.ty != y.ty) or (x.cv != y.cv) or (x.xs.len != y.xs.len) or (x.mask.len != y.mask.len) {
        return false;
    }
    if not same_val(x.c, y.c) {
        return false;
    }
    let direct = same_val(x.a, y.a) and same_val(x.b, y.b);
    if not direct {
        if not commutative(x.op) or not (same_val(x.a, y.b) and same_val(x.b, y.a)) {
            return false;
        }
    }
    for k in 0..x.xs.len {
        if not same_val(x.xs.get(k), y.xs.get(k)) {
            return false;
        }
    }
    for k in 0..x.mask.len {
        if x.mask[k] != y.mask[k] {
            return false;
        }
    }
    // operand types (e.g. casts of different source types)
    if u.notnil(x.a) and u.notnil(y.a) and (x.a.ty != y.a.ty) {
        return false;
    }
    return true;
}

fn push_ent(kind: i32, h: u64, key: *mut ir.Value, off: i64, ty: *mut ir.Ty, val: *mut ir.Value) void {
    let b = @truncate(usize, h) & (nb - 1);
    ents.push(Ent.{ kind: kind, key: key, off: off, ty: ty, val: val, height: writes.len, next: bk.get(b), bucket: b });
    bk.set(b, @int_cast(i64, ents.len) - 1);
}

fn pop_to(n: usize) void {
    while ents.len > n {
        let e = ents.pop();
        bk.set(e.bucket, e.next);
    }
}

fn find_expr(i: *ir.Value) *mut ir.Value {
    let h = hash_expr(i);
    let mut e = bk.get(@truncate(usize, h) & (nb - 1));
    while e >= 0 {
        let x = ents.at(@int_cast(usize, e));
        if (x.kind == K_EXPR) and same_expr(x.key, i) {
            return x.val;
        }
        e = x.next;
    }
    return vnil();
}

fn add_expr(key: *mut ir.Value, val: *mut ir.Value) void {
    push_ent(K_EXPR, hash_expr(key), key, 0, key.ty, val);
}

fn known_hash(v: *ir.Value) u64 {
    return mix(hp(v), 0x4b4e);
}

fn find_known(v: *mut ir.Value) *mut ir.Value {
    let h = known_hash(v);
    let mut e = bk.get(@truncate(usize, h) & (nb - 1));
    while e >= 0 {
        let x = ents.at(@int_cast(usize, e));
        if (x.kind == K_KNOWN) and (x.key == v) {
            return x.val;
        }
        e = x.next;
    }
    return vnil();
}

fn add_known(v: *mut ir.Value, c: *mut ir.Value) void {
    push_ent(K_KNOWN, known_hash(v), v, 0, v.ty, c);
}

fn mem_hash(base: *ir.Value, off: i64, ty: *ir.Ty) u64 {
    return mix(mix(hp(base), @bit_cast(u64, off)), @as(u64, @bit_cast(u32, ty.id)) +% 0x3131);
}

// -----------------------------------------------------------------------------
// Alias analysis
// -----------------------------------------------------------------------------
fn is_inst(v: *ir.Value, op: i32) bool {
    return (v.kind == ir.V_INST) and (v.op == op);
}

fn is_obj(v: *ir.Value) bool {
    return is_inst(v, ir.OP_ALLOCA) or (v.kind == ir.V_GLOBAL);
}

fn loc_of(p0: *mut ir.Value, size: i64) Loc {
    let mut p = p0;
    let mut off: i64 = 0;
    while is_inst(p, ir.OP_PTRADD) and ir.is_cint(p.b) {
        off = off +% ir.sval(p.b);
        p = p.a;
    }
    let base = p;
    while is_inst(p, ir.OP_PTRADD) {
        p = p.a;
    }
    let mut obj = vnil();
    if is_obj(p) {
        obj = p;
    } else if is_inst(p, ir.OP_SELECT) and noalias_global(p.b) {
        // wasm: the thread's shadow stack record, or the main thread's
        obj = p.b;
    }
    return Loc.{ base: base, obj: obj, off: off, size: size };
}

// Memory the program can only reach by the global's name (the shadow stack).
fn noalias_global(o: *ir.Value) bool {
    return u.notnil(o) and (o.kind == ir.V_GLOBAL) and o.gv.noalias;
}

// An alloca whose address is not visible to calls (aux2 == 1 after escapes()).
fn private_obj(o: *ir.Value) bool {
    return u.notnil(o) and is_inst(o, ir.OP_ALLOCA) and (o.aux2 == 1);
}

fn overlap(a: i64, an: i64, b: i64, bn: i64) bool {
    if (an < 0) or (bn < 0) {
        return true;
    }
    return (a < (b + bn)) and (b < (a + an));
}

fn may_alias(x: *Loc, w: *Write) bool {
    if w.kind == W_ALL {
        return true;
    }
    if w.kind == W_CALL {
        return not private_obj(x.obj);
    }
    if x.base == w.base {
        return overlap(x.off, x.size, w.off, w.size);
    }
    if u.notnil(x.obj) and u.notnil(w.obj) and (x.obj != w.obj) {
        return false;
    }
    if (private_obj(x.obj) and (w.obj != x.obj)) or (private_obj(w.obj) and (x.obj != w.obj)) {
        return false;
    }
    if (noalias_global(x.obj) and (w.obj != x.obj)) or (noalias_global(w.obj) and (x.obj != w.obj)) {
        return false;
    }
    return true;
}

fn push_write(kind: i32, l: Loc) void {
    writes.push(Write.{ kind: kind, base: l.base, obj: l.obj, off: l.off, size: l.size });
}

fn nowhere() Loc {
    return Loc.{ base: vnil(), obj: vnil(), off: 0, size: -1 };
}

// Marks allocas whose address stays private (aux2 = 1): used only as the
// address of loads, stores and memory operations, or offset by ptradd.
fn escapes(f: *mut ir.Func) void {
    let mut b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            i.aux2 = 0;
            if i.op == ir.OP_ALLOCA {
                i.aux2 = 1;
            }
            i = i.next;
        }
        b = b.next;
    }
    b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            let op = i.op;
            if op == ir.OP_LOAD {
                // address use only
            } else if op == ir.OP_STORE {
                leak(i.a);
            } else if (op == ir.OP_MEMCPY) or (op == ir.OP_MEMMOVE) or (op == ir.OP_MEMSET) {
                leak(i.c);
                if op == ir.OP_MEMSET {
                    leak(i.b);
                }
            } else if op == ir.OP_PTRADD {
                leak(i.b);
            } else if op == ir.OP_ICMP {
                // comparing addresses does not reveal the object
            } else {
                leak(i.a);
                leak(i.b);
                leak(i.c);
                for k in 0..i.xs.len {
                    leak(i.xs.get(k));
                }
            }
            i = i.next;
        }
        b = b.next;
    }
    // ptradds of an alloca whose results escape
    b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            if (i.op == ir.OP_PTRADD) and (i.aux2 == 2) {
                let mut p = i.a;
                while is_inst(p, ir.OP_PTRADD) {
                    p = p.a;
                }
                if is_inst(p, ir.OP_ALLOCA) {
                    p.aux2 = 0;
                }
            }
            i = i.next;
        }
        b = b.next;
    }
}

fn leak(v: *mut ir.Value) void {
    if u.isnil(v) or (v.kind != ir.V_INST) {
        return;
    }
    if v.op == ir.OP_ALLOCA {
        v.aux2 = 0;
    } else if v.op == ir.OP_PTRADD {
        v.aux2 = 2;  // resolved to its alloca afterwards
    }
}

// -----------------------------------------------------------------------------
// Effects
// -----------------------------------------------------------------------------
fn pure_intr(id: i32) bool {
    return (id != ir.IN_TRAP) and (id != ir.IN_MEMGROW) and (id != ir.IN_WAIT32) and (id != ir.IN_NOTIFY) and
           (id != ir.IN_SET_SP) and (id != ir.IN_MEMSIZE);
}

fn cse_able(i: *ir.Value) bool {
    let op = i.op;
    if ((op >= ir.OP_ADD) and (op <= ir.OP_FPTOUI_SAT)) or (op == ir.OP_PTRADD) {
        return true;
    }
    if (op == ir.OP_EXTRACTELEM) or (op == ir.OP_INSERTELEM) or (op == ir.OP_SHUFFLE) or (op == ir.OP_SPLAT) {
        return true;
    }
    if op == ir.OP_INTR {
        return pure_intr(@truncate(i32, @bit_cast(i64, i.cv)));
    }
    return false;
}

fn size_of(n: *ir.Value) i64 {
    if ir.is_cint(n) {
        let x = ir.cint_val(n);
        if x < 0x4000_0000_0000_0000 {
            return @bit_cast(i64, x);
        }
    }
    return -1;
}

// The writes of instruction i (for region summaries and the walk).
fn note_writes(i: *mut ir.Value) void {
    let op = i.op;
    if op == ir.OP_STORE {
        if i.flags != 0 {
            push_write(W_ALL, nowhere());
        } else {
            push_write(W_LOC, loc_of(i.b, @bit_cast(i64, i.a.ty.size)));
        }
    } else if (op == ir.OP_MEMCPY) or (op == ir.OP_MEMMOVE) or (op == ir.OP_MEMSET) {
        push_write(W_LOC, loc_of(i.a, size_of(i.c)));
    } else if op == ir.OP_CALL {
        push_write(W_CALL, nowhere());
    } else if (op == ir.OP_RMW) or (op == ir.OP_CMPXCHG) or (op == ir.OP_FENCE) {
        push_write(W_ALL, nowhere());
    } else if (op == ir.OP_LOAD) and (i.flags != 0) {
        push_write(W_ALL, nowhere());
    } else if (op == ir.OP_INTR) and not pure_intr(@truncate(i32, @bit_cast(i64, i.cv))) {
        push_write(W_ALL, nowhere());
    }
}

// Is the value at l (of type t) known? Returns it or nil.
fn find_mem(l: *Loc, t: *mut ir.Ty) *mut ir.Value {
    let h = mem_hash(l.base, l.off, t);
    let mut e = bk.get(@truncate(usize, h) & (nb - 1));
    while e >= 0 {
        let x = ents.at(@int_cast(usize, e));
        if (x.kind == K_MEM) and (x.key == l.base) and (x.off == l.off) and (x.ty == t) {
            // no possibly aliasing write since
            if (writes.len - x.height) > 256 {
                return vnil();
            }
            for w in x.height..writes.len {
                if may_alias(l, writes.at(w)) {
                    return vnil();
                }
            }
            return x.val;
        }
        e = x.next;
    }
    return vnil();
}

fn add_mem(l: *Loc, t: *mut ir.Ty, v: *mut ir.Value) void {
    push_ent(K_MEM, mem_hash(l.base, l.off, t), l.base, l.off, t, v);
}

// -----------------------------------------------------------------------------
// Conditions
// -----------------------------------------------------------------------------
fn kbool(b: bool) *mut ir.Value {
    if b {
        return ir.cint(ir.t_int(1), 1);
    }
    return ir.cint(ir.t_int(1), 0);
}

fn synth_icmp(p: i32, a: *mut ir.Value, b: *mut ir.Value) *mut ir.Value {
    let v = u.new.[ir.Value]();
    v.kind = ir.V_INST;
    v.op = ir.OP_ICMP;
    v.ty = ir.t_int(1);
    v.cv = @as(u64, @bit_cast(u32, p));
    v.a = a;
    v.b = b;
    return v;
}

// c is known to be `val` in the current scope.
fn assume(c: *mut ir.Value, val: bool, depth: i32) void {
    if (depth > 4) or ir.is_const(c) {
        return;
    }
    add_known(c, kbool(val));
    if c.kind != ir.V_INST {
        return;
    }
    if (c.op == ir.OP_ICMP) and (c.ty.kind == ir.T_INT) {
        let p = cb.pred_of(c);
        let mut tp = p;         // the predicate that holds
        if not val {
            tp = cb.inv_pred(p);
        }
        add_expr(synth_icmp(tp, c.a, c.b), kbool(true));
        add_expr(synth_icmp(cb.inv_pred(tp), c.a, c.b), kbool(false));
        add_expr(synth_icmp(cb.swap_pred(tp), c.b, c.a), kbool(true));
        add_expr(synth_icmp(cb.inv_pred(cb.swap_pred(tp)), c.b, c.a), kbool(false));
        // x == K: x is K
        if (tp == ir.IntEQ) and ir.is_const(c.b) and not ir.is_const(c.a) {
            add_known(c.a, c.b);
        }
        return;
    }
    if (c.op == ir.OP_AND) and val and (c.ty.bits == 1) {
        assume(c.a, true, depth + 1);
        assume(c.b, true, depth + 1);
    } else if (c.op == ir.OP_OR) and not val and (c.ty.bits == 1) {
        assume(c.a, false, depth + 1);
        assume(c.b, false, depth + 1);
    } else if (c.op == ir.OP_XOR) and (c.ty.bits == 1) and cb.is_k(c.b) {
        assume(c.a, val != (ir.cint_val(c.b) != 0), depth + 1);
    }
}

// Facts on entry to b from its only predecessor.
fn edge_facts(b: *mut ir.Block) void {
    if b.preds.len != 1 {
        return;
    }
    let p = b.preds.get(0);
    let t = ir.terminator(p);
    if u.isnil(t) {
        return;
    }
    if (t.op == ir.OP_CONDBR) and (t.t1 != t.t2) {
        assume(t.a, t.t1 == b, 0);
    } else if (t.op == ir.OP_SWITCH) and (t.t1 != b) and not ir.is_const(t.a) {
        let mut n: i32 = 0;
        let mut c = vnil();
        for k in 0..t.bs.len {
            if t.bs.get(k) == b {
                n += 1;
                c = t.xs.get(k);
            }
        }
        if n == 1 {
            add_known(t.a, c);
        }
    }
}

// Writes on paths from b's immediate dominator to b that avoid it.
fn region_writes(b: *mut ir.Block) void {
    if b.preds.len <= 1 {
        return;
    }
    let d = b.idom;
    let mut work = u.List.[*mut ir.Block].{ data: u.nil.[*mut ir.Block](), len: 0, cap: 0 };
    let mut seen = u.List.[*mut ir.Block].{ data: u.nil.[*mut ir.Block](), len: 0, cap: 0 };
    let tag = @int_cast(i64, b.id) + 1 + (@int_cast(i64, g_fn.nblocks) * @int_cast(i64, b.rpo + 1));
    for k in 0..b.preds.len {
        let p = b.preds.get(k);
        if (p != d) and (p.rpo >= 0) and (p.aux != tag) {
            p.aux = tag;
            work.push(p);
            seen.push(p);
        }
    }
    let mut ok = true;
    while work.len > 0 {
        if seen.len > 200 {
            ok = false;
            break;
        }
        let x = work.pop();
        for k in 0..x.preds.len {
            let p = x.preds.get(k);
            if (p != d) and (p.rpo >= 0) and (p.aux != tag) {
                p.aux = tag;
                work.push(p);
                seen.push(p);
            }
        }
    }
    if ok {
        let start = writes.len;
        for k in 0..seen.len {
            let mut i = seen.get(k).first;
            while u.notnil(i) {
                note_writes(i);
                i = i.next;
            }
            if (writes.len - start) > 128 {
                ok = false;
                break;
            }
        }
    }
    if not ok {
        push_write(W_ALL, nowhere());
    }
    u.free(@ptr_cast(*mut void, work.data));
    u.free(@ptr_cast(*mut void, seen.data));
}

// -----------------------------------------------------------------------------
// The walk
// -----------------------------------------------------------------------------
fn replace(i: *mut ir.Value, r: *mut ir.Value) void {
    i.fwd = r;
    ir.remove_inst(i);
    changed = true;
}

fn known_operand(o: *mut ir.Value) *mut ir.Value {
    if u.isnil(o) {
        return o;
    }
    let r = ir.resolve(o);
    if (r.kind == ir.V_INST) or (r.kind == ir.V_PARAM) {
        let k = find_known(r);
        if u.notnil(k) {
            changed = true;
            return k;
        }
    }
    return r;
}

fn visit_block(b: *mut ir.Block) void {
    let mut i = b.first;
    while u.notnil(i) {
        let next = i.next;
        let op = i.op;
        if op == ir.OP_PHI {
            for k in 0..i.xs.len {
                i.xs.set(k, ir.resolve(i.xs.get(k)));
            }
            i = next;
            continue;
        }
        i.a = known_operand(i.a);
        i.b = known_operand(i.b);
        i.c = known_operand(i.c);
        for k in 0..i.xs.len {
            i.xs.set(k, known_operand(i.xs.get(k)));
        }
        if cse_able(i) {
            let r = find_expr(i);
            if u.notnil(r) {
                replace(i, r);
            } else {
                add_expr(i, i);
            }
        } else if op == ir.OP_LOAD {
            if i.flags != 0 {
                note_writes(i);
            } else {
                let l = loc_of(i.a, @bit_cast(i64, i.ty.size));
                let r = find_mem(&l, i.ty);
                if u.notnil(r) {
                    replace(i, r);
                } else {
                    add_mem(&l, i.ty, i);
                }
            }
        } else if op == ir.OP_STORE {
            if i.flags != 0 {
                note_writes(i);
            } else {
                let l = loc_of(i.b, @bit_cast(i64, i.a.ty.size));
                let r = find_mem(&l, i.a.ty);
                if u.notnil(r) and (ir.resolve(r) == i.a) {
                    ir.remove_inst(i);  // memory already holds the value
                    changed = true;
                } else {
                    note_writes(i);
                    add_mem(&l, i.a.ty, i.a);
                }
            }
        } else {
            note_writes(i);
        }
        i = next;
    }
}

fn walk(entry: *mut ir.Block) void {
    let mut blocks = u.List.[*mut ir.Block].{ data: u.nil.[*mut ir.Block](), len: 0, cap: 0 };
    let mut kids = u.List.[usize].{ data: u.nil.[usize](), len: 0, cap: 0 };
    let mut ent_marks = u.List.[usize].{ data: u.nil.[usize](), len: 0, cap: 0 };
    let mut wr_marks = u.List.[usize].{ data: u.nil.[usize](), len: 0, cap: 0 };
    blocks.push(entry);
    kids.push(0);
    ent_marks.push(0);
    wr_marks.push(0);
    visit_block(entry);
    while blocks.len > 0 {
        let b = blocks.last();
        let k = kids.last();
        if k < b.dom_kids.len {
            kids.set(kids.len - 1, k + 1);
            let c = b.dom_kids.get(k);
            ent_marks.push(ents.len);
            wr_marks.push(writes.len);
            region_writes(c);
            edge_facts(c);
            visit_block(c);
            blocks.push(c);
            kids.push(0);
        } else {
            pop_to(ent_marks.pop());
            writes.len = wr_marks.pop();
            _ = blocks.pop();
            _ = kids.pop();
        }
    }
    u.free(@ptr_cast(*mut void, blocks.data));
    u.free(@ptr_cast(*mut void, kids.data));
    u.free(@ptr_cast(*mut void, ent_marks.data));
    u.free(@ptr_cast(*mut void, wr_marks.data));
}

// True if anything changed.
pub fn gvn(f: *mut ir.Func) bool {
    if u.isnil(f.first) {
        return false;
    }
    if u.isnil(B) {
        B = ir.new_builder();
    }
    g_fn = f;
    changed = false;
    opt.compute_preds(f);
    let order = opt.dominators(f);
    let mut n: usize = 0;
    let mut b = f.first;
    while u.notnil(b) {
        b.aux = 0;
        let mut i = b.first;
        while u.notnil(i) {
            n += 1;
            i = i.next;
        }
        b = b.next;
    }
    nb = 256;
    while nb < (n * 2) {
        nb = nb * 2;
    }
    bk.clear();
    for _k in 0..nb {
        bk.push(-1);
    }
    ents.clear();
    writes.clear();
    escapes(f);
    walk(order.get(0));
    u.free(@ptr_cast(*mut void, order.data));
    ir.apply_forwarding(f);
    return changed;
}

// -----------------------------------------------------------------------------
// Dead store elimination (within blocks): a store to memory that is written
// again later in the block, with nothing in between that may read it, is
// deleted; so is a store to a private alloca that nothing reads before the
// function returns.
// -----------------------------------------------------------------------------
fn loc_write(l: *Loc) Write {
    return Write.{ kind: W_LOC, base: l.base, obj: l.obj, off: l.off, size: l.size };
}

// Is a covered by b (both exact)?
fn covers(b: *Loc, a: *Loc) bool {
    return (a.base == b.base) and (a.size >= 0) and (b.size >= 0) and (a.off >= b.off) and ((a.off + a.size) <= (b.off + b.size));
}

// Removes the killed locations that the read of r may observe.
fn read_loc(killed: *mut u.List.[Loc], r: *Loc) void {
    let w = loc_write(r);
    let mut k: usize = 0;
    while k < killed.len {
        if may_alias(killed.at(k), &w) {
            killed.set(k, killed.get(killed.len - 1));
            killed.len -= 1;
        } else {
            k += 1;
        }
    }
}

// Keeps only the locations in private allocas (a call or return reads the rest).
fn read_nonprivate(killed: *mut u.List.[Loc]) void {
    let mut k: usize = 0;
    while k < killed.len {
        if not private_obj(killed.get(k).obj) {
            killed.set(k, killed.get(killed.len - 1));
            killed.len -= 1;
        } else {
            k += 1;
        }
    }
}

fn has_obj(xs: *u.List.[*mut ir.Value], o: *mut ir.Value) bool {
    for k in 0..xs.len {
        if xs.get(k) == o {
            return true;
        }
    }
    return false;
}

fn dse_block(b: *mut ir.Block, killed: *mut u.List.[Loc], read_objs: *mut u.List.[*mut ir.Value]) bool {
    killed.clear();
    read_objs.clear();
    let mut any = false;
    let t = ir.terminator(b);
    // at a return, private allocas are dead unless read later in the block
    let mut at_ret = u.notnil(t) and (t.op == ir.OP_RET);
    let mut i = b.last;
    while u.notnil(i) {
        let prev = i.prev;
        let op = i.op;
        if op == ir.OP_STORE {
            if i.flags != 0 {
                killed.clear();
                at_ret = false;
            } else {
                let l = loc_of(i.b, @bit_cast(i64, i.a.ty.size));
                let mut dead = at_ret and private_obj(l.obj) and not has_obj(read_objs, l.obj);
                for k in 0..killed.len {
                    if covers(killed.at(k), &l) {
                        dead = true;
                    }
                }
                if dead {
                    ir.remove_inst(i);
                    any = true;
                } else if (killed.len < 32) and (l.size >= 0) {
                    killed.push(l);
                }
            }
        } else if op == ir.OP_LOAD {
            if i.flags != 0 {
                killed.clear();
                at_ret = false;
            } else {
                let l = loc_of(i.a, @bit_cast(i64, i.ty.size));
                read_loc(killed, &l);
                if u.notnil(l.obj) {
                    read_objs.push(l.obj);
                }
            }
        } else if (op == ir.OP_MEMCPY) or (op == ir.OP_MEMMOVE) {
            let s = loc_of(i.b, size_of(i.c));
            read_loc(killed, &s);
            if u.notnil(s.obj) {
                read_objs.push(s.obj);
            }
            let d = loc_of(i.a, size_of(i.c));
            if (killed.len < 32) and (d.size >= 0) {
                killed.push(d);
            }
        } else if op == ir.OP_MEMSET {
            let d = loc_of(i.a, size_of(i.c));
            if (killed.len < 32) and (d.size >= 0) {
                killed.push(d);
            }
        } else if (op == ir.OP_CALL) or (op == ir.OP_RET) {
            read_nonprivate(killed);
        } else if (op == ir.OP_RMW) or (op == ir.OP_CMPXCHG) or (op == ir.OP_FENCE) {
            killed.clear();
            at_ret = false;
        } else if (op == ir.OP_INTR) and not pure_intr(@truncate(i32, @bit_cast(i64, i.cv))) {
            killed.clear();
            at_ret = false;
        }
        i = prev;
    }
    return any;
}

pub fn dse(f: *mut ir.Func) bool {
    if u.isnil(f.first) {
        return false;
    }
    escapes(f);
    let mut killed = u.List.[Loc].{ data: u.nil.[Loc](), len: 0, cap: 0 };
    let mut read_objs = u.List.[*mut ir.Value].{ data: u.nil.[*mut ir.Value](), len: 0, cap: 0 };
    let mut any = false;
    let mut b = f.first;
    while u.notnil(b) {
        if dse_block(b, &mut killed, &mut read_objs) {
            any = true;
        }
        b = b.next;
    }
    u.free(@ptr_cast(*mut void, killed.data));
    u.free(@ptr_cast(*mut void, read_objs.data));
    return any;
}
