// =============================================================================
// thread.ph - jump threading.
//
// A block B that ends in `condbr c` where, coming from predecessor P, c is a
// constant (B's phis take P's incoming values, and c is computed from them
// in B), is bypassed on that edge: P branches to a copy of B's instructions
// (a new block N) that goes straight to the successor S that c selects.
//
// Short-circuit conditions (`a and b` is a phi of false and b) and the
// error-union results of inlined calls (a phi of the error code, tested
// right away) are the usual cases.
//
// Values of B used outside it: uses under S get a phi at S (S's only
// predecessor was B), uses under B's other successor are unaffected (when B
// is its only predecessor); any other use, and the edge stays.
// =============================================================================
import "util.ph" as u;
import "ir.ph" as ir;
import "opt.ph" as opt;

fn vnil() *mut ir.Value {
    return u.nil.[ir.Value]();
}

const MAX_COPY: i32 = 6;

fn is_k(v: *ir.Value) bool {
    return ir.is_cint(v) and (v.ty.kind == ir.T_INT) and (v.ty.bits <= 64);
}

fn incoming(p: *mut ir.Value, from: *mut ir.Block) *mut ir.Value {
    for k in 0..p.bs.len {
        if p.bs.get(k) == from {
            return p.xs.get(k);
        }
    }
    return vnil();
}

// The constant v is when B is entered from P, or nil.
fn eval(v: *mut ir.Value, b: *mut ir.Block, p: *mut ir.Block, depth: i32) *mut ir.Value {
    if is_k(v) {
        return v;
    }
    if (v.kind != ir.V_INST) or (v.blk != b) or (depth > 6) {
        return vnil();
    }
    let op = v.op;
    if op == ir.OP_PHI {
        let x = incoming(v, p);
        if u.notnil(x) and is_k(x) {
            return x;
        }
        return vnil();
    }
    if (op == ir.OP_AND) or (op == ir.OP_OR) {
        // one operand may decide the result
        let x = eval(v.a, b, p, depth + 1);
        let y = eval(v.b, b, p, depth + 1);
        let zero_decides = op == ir.OP_AND;
        if u.notnil(x) and u.notnil(y) {
            return ir.fold_int_bin(op, v.ty, x, y);
        }
        if v.ty.bits == 1 {
            if u.notnil(x) and ((ir.cint_val(x) == 0) == zero_decides) {
                return x;
            }
            if u.notnil(y) and ((ir.cint_val(y) == 0) == zero_decides) {
                return y;
            }
        }
        return vnil();
    }
    if (op >= ir.OP_ADD) and (op <= ir.OP_ASHR) {
        let x = eval(v.a, b, p, depth + 1);
        let y = eval(v.b, b, p, depth + 1);
        if u.isnil(x) or u.isnil(y) or (v.ty.kind != ir.T_INT) or (v.ty.bits > 64) {
            return vnil();
        }
        return ir.fold_int_bin(op, v.ty, x, y);
    }
    if op == ir.OP_ICMP {
        if (v.ty.kind != ir.T_INT) or (v.a.ty.kind != ir.T_INT) or (v.a.ty.bits > 64) {
            return vnil();
        }
        let x = eval(v.a, b, p, depth + 1);
        let y = eval(v.b, b, p, depth + 1);
        if u.isnil(x) or u.isnil(y) {
            return vnil();
        }
        return ir.icmp(ir.new_builder(), @truncate(i32, @bit_cast(i64, v.cv)), x, y);
    }
    if op == ir.OP_SELECT {
        let c = eval(v.a, b, p, depth + 1);
        if u.isnil(c) {
            return vnil();
        }
        if ir.cint_val(c) != 0 {
            return eval(v.b, b, p, depth + 1);
        }
        return eval(v.c, b, p, depth + 1);
    }
    if ((op == ir.OP_ZEXT) or (op == ir.OP_SEXT) or (op == ir.OP_TRUNC)) and (v.ty.kind == ir.T_INT) and (v.ty.bits <= 64) {
        let x = eval(v.a, b, p, depth + 1);
        if u.isnil(x) {
            return vnil();
        }
        return ir.cast(ir.new_builder(), op, x, v.ty);
    }
    return vnil();
}

// -----------------------------------------------------------------------------
// Uses of B's values
// -----------------------------------------------------------------------------
struct Use {
    user: *mut ir.Value,
    at: *mut ir.Block,    // where the value must be available
    phi_from_b: bool,     // a phi input coming from B
}

let mut uses: u.List.[Use] = u.List.[Use].{ data: @ptr_from_int(*mut Use, 0), len: 0, cap: 0 };

// (a phi's input is used at the end of the block it comes from: a phi of B
// itself uses B's values from the blocks of a loop through B)
fn note(o: *mut ir.Value, i: *mut ir.Value, at: *mut ir.Block, b: *mut ir.Block, from_b: bool) void {
    if u.notnil(o) and (o.kind == ir.V_INST) and (o.blk == b) and (at != b) {
        uses.push(Use.{ user: i, at: at, phi_from_b: from_b });
    }
}

// Every use outside B of a value defined in B.
fn outside_uses(f: *mut ir.Func, b: *mut ir.Block) void {
    uses.clear();
    let mut x = f.first;
    while u.notnil(x) {
        let mut i = x.first;
        while u.notnil(i) {
            if i.op == ir.OP_PHI {
                for k in 0..i.xs.len {
                    let from = i.bs.get(k);
                    note(i.xs.get(k), i, from, b, from == b);
                }
            } else {
                note(i.a, i, x, b, false);
                note(i.b, i, x, b, false);
                note(i.c, i, x, b, false);
                for k in 0..i.xs.len {
                    note(i.xs.get(k), i, x, b, false);
                }
            }
            i = i.next;
        }
        x = x.next;
    }
}

// Can B's values still be used where they are after P is sent to S?
fn uses_ok(b: *mut ir.Block, s: *mut ir.Block, other: *mut ir.Block) bool {
    let s_owned = (s.preds.len == 1) and (s.rpo >= 0);
    let o_owned = u.notnil(other) and (other.preds.len == 1) and (other.rpo >= 0);
    for k in 0..uses.len {
        let us = uses.get(k);
        if us.phi_from_b {
            continue;  // B -> Y edges stay (Y == S gets an input from N)
        }
        let x = us.at;
        if x == b {
            continue;
        }
        if x.rpo < 0 {
            continue;  // unreachable
        }
        if s_owned and opt.dominates(s, x) {
            continue;
        }
        if o_owned and opt.dominates(other, x) {
            continue;
        }
        return false;
    }
    return true;
}

fn remove_phi_input(s: *mut ir.Block, from: *mut ir.Block) void {
    let mut p = s.first;
    while u.notnil(p) and (p.op == ir.OP_PHI) {
        let mut k: usize = 0;
        while k < p.bs.len {
            if p.bs.get(k) == from {
                p.bs.set(k, p.bs.get(p.bs.len - 1));
                p.xs.set(k, p.xs.get(p.xs.len - 1));
                p.bs.len -= 1;
                p.xs.len -= 1;
            } else {
                k += 1;
            }
        }
        p = p.next;
    }
}

// The value v of B as seen on the path through N (clones in auxp).
fn mapped(v: *mut ir.Value, b: *mut ir.Block, p: *mut ir.Block) *mut ir.Value {
    if u.notnil(v) and (v.kind == ir.V_INST) and (v.blk == b) {
        if v.op == ir.OP_PHI {
            return incoming(v, p);
        }
        return @ptr_cast(*mut ir.Value, v.auxp);
    }
    return v;
}

// Sends P to S through a copy of B's instructions.
fn thread(f: *mut ir.Func, b: *mut ir.Block, p: *mut ir.Block, s: *mut ir.Block) void {
    let n = ir.append_block(f, "thread");
    ir.move_block_after(n, p);
    let mut i = b.first;
    while u.notnil(i) {
        if (i.op != ir.OP_PHI) and not ir.is_term(i.op) {
            let y = ir.new_inst(f, i.op, i.ty);
            y.cv = i.cv;
            y.fv = i.fv;
            y.flags = i.flags;
            y.align = i.align;
            y.aty = i.aty;
            y.fty = i.fty;
            y.mask = i.mask;
            y.ext = i.ext;
            y.name = i.name;
            y.a = mapped(i.a, b, p);
            y.b = mapped(i.b, b, p);
            y.c = mapped(i.c, b, p);
            for k in 0..i.xs.len {
                y.xs.push(mapped(i.xs.get(k), b, p));
            }
            i.auxp = @ptr_cast(*mut void, y);
            ir.append_inst(n, y);
        }
        i = i.next;
    }
    let br = ir.new_inst(f, ir.OP_BR, ir.t_void());
    br.t1 = s;
    ir.append_inst(n, br);
    // S's phis: the input from B, as seen through N
    let mut q = s.first;
    while u.notnil(q) and (q.op == ir.OP_PHI) {
        let x = incoming(q, b);
        if u.notnil(x) {
            ir.add_incoming(q, mapped(x, b, p), n);
        }
        q = q.next;
    }
    // uses under S: a phi at S of B's value and N's
    let bld = ir.new_builder();
    for k in 0..uses.len {
        let us = uses.get(k);
        if us.phi_from_b or (us.at == b) or (us.at.rpo < 0) {
            continue;
        }
        if not ((s.preds.len == 1) and opt.dominates(s, us.at)) {
            continue;
        }
        let user = us.user;
        let ops: [3]*mut ir.Value = [user.a, user.b, user.c];
        for j in @as(usize, 0)..3 {
            let o = ops[j];
            if u.notnil(o) and (o.kind == ir.V_INST) and (o.blk == b) {
                let r = repair_phi(bld, o, b, p, n, s);
                if j == 0 {
                    user.a = r;
                } else if j == 1 {
                    user.b = r;
                } else {
                    user.c = r;
                }
            }
        }
        for j in 0..user.xs.len {
            let o = user.xs.get(j);
            if (o.kind == ir.V_INST) and (o.blk == b) {
                let skip = (user.op == ir.OP_PHI) and (user.bs.get(j) != us.at);
                if not skip {
                    user.xs.set(j, repair_phi(bld, o, b, p, n, s));
                }
            }
        }
    }
    // P goes to N instead of B
    let t = ir.terminator(p);
    for k in 0..ir.succ_count(t) {
        if ir.succ(t, k) == b {
            ir.set_succ(t, k, n);
        }
    }
    remove_phi_input(b, p);
}

let mut repaired: u.List.[*mut ir.Value] = u.List.[*mut ir.Value].{ data: @ptr_from_int(*mut *mut ir.Value, 0), len: 0, cap: 0 };

// The phi at S merging v (from B) and its copy (from N); made once per value.
fn repair_phi(bld: *mut ir.Builder, v: *mut ir.Value, b: *mut ir.Block, p: *mut ir.Block, n: *mut ir.Block, s: *mut ir.Block) *mut ir.Value {
    for k in 0..repaired.len {
        let ph = repaired.get(k);
        if ph.xs.get(0) == v {
            return ph;
        }
    }
    if u.notnil(s.first) {
        ir.at_before(bld, s.first);
    } else {
        ir.at_end(bld, s);
    }
    let ph = ir.phi(bld, v.ty);
    ir.add_incoming(ph, v, b);
    ir.add_incoming(ph, mapped(v, b, p), n);
    repaired.push(ph);
    return ph;
}

fn copy_cost(b: *ir.Block) i32 {
    let mut n: i32 = 0;
    let mut i = b.first;
    while u.notnil(i) {
        if (i.op != ir.OP_PHI) and not ir.is_term(i.op) {
            n += 1;
            if (i.op == ir.OP_CALL) or (i.op == ir.OP_ALLOCA) {
                n += 100;
            }
        }
        i = i.next;
    }
    return n;
}

// Threads one edge in f; true if it did.
fn thread_one(f: *mut ir.Func) bool {
    opt.compute_preds(f);
    let order = opt.dominators(f);
    let mut done = false;
    for bi in 1..order.len {
        let b = order.get(bi);
        let t = ir.terminator(b);
        if u.isnil(t) or (t.op != ir.OP_CONDBR) or (t.t1 == t.t2) or (b.preds.len < 2) {
            continue;
        }
        if (t.a.kind != ir.V_INST) or (t.a.blk != b) or (copy_cost(b) > MAX_COPY) {
            continue;
        }
        // not through a loop header: that would peel iterations, one per round
        let mut header = false;
        for k in 0..b.preds.len {
            let p = b.preds.get(k);
            if (p.rpo >= 0) and opt.dominates(b, p) {
                header = true;
            }
        }
        if header {
            continue;
        }
        for k in 0..b.preds.len {
            let p = b.preds.get(k);
            if (p == b) or (p.rpo < 0) {
                continue;
            }
            let c = eval(t.a, b, p, 0);
            if u.isnil(c) {
                continue;
            }
            let mut s = t.t2;
            let mut other = t.t1;
            if ir.cint_val(c) != 0 {
                s = t.t1;
                other = t.t2;
            }
            if s == b {
                continue;
            }
            if b.mark == gen {
                continue;  // its values cannot be repaired (found earlier)
            }
            outside_uses(f, b);
            if not uses_ok(b, s, other) {
                b.mark = gen;
                continue;
            }
            repaired.clear();
            thread(f, b, p, s);
            done = true;
            break;
        }
        if done {
            break;
        }
    }
    u.free(@ptr_cast(*mut void, order.data));
    return done;
}

let mut gen: i64 = 0;

// Threads edges until none is left (or a limit); true if any.
pub fn jump_thread(f: *mut ir.Func) bool {
    if u.isnil(f.first) {
        return false;
    }
    gen += 1;
    let mut n: i32 = 0;
    while (n < 256) and thread_one(f) {
        n += 1;
    }
    if n > 0 {
        ir.remove_unreachable(f);
    }
    return n > 0;
}

// -----------------------------------------------------------------------------
// If-conversion: a branch whose arms only compute a few cheap values that
// cannot trap, merged by phis, becomes straight-line code with selects
// (conditional moves). Sorting and searching branch unpredictably on data.
// -----------------------------------------------------------------------------
const MAX_SPEC: i32 = 4;
const MAX_SELECTS: i32 = 4;

fn speculatable(i: *ir.Value) bool {
    let op = i.op;
    if (op == ir.OP_UDIV) or (op == ir.OP_SDIV) or (op == ir.OP_UREM) or (op == ir.OP_SREM) {
        return false;  // a zero divisor faults
    }
    if (op >= ir.OP_ADD) and (op <= ir.OP_FNEG) {
        return (i.ty.kind == ir.T_INT) or (i.ty.kind == ir.T_FLOAT) or (i.ty.kind == ir.T_PTR);
    }
    if (op == ir.OP_ICMP) or (op == ir.OP_FCMP) or (op == ir.OP_SELECT) or (op == ir.OP_PTRADD) {
        return i.ty.kind != ir.T_VEC;
    }
    if (op >= ir.OP_ZEXT) and (op <= ir.OP_INTTOPTR) {
        return true;
    }
    return false;
}

// Does arm block a (a single-predecessor block going to m) hold only a few
// speculatable instructions? -1: no; else how many.
fn arm_cost(a: *mut ir.Block, d: *mut ir.Block, m: *mut ir.Block) i32 {
    if (a.preds.len != 1) or (a.preds.get(0) != d) {
        return -1;
    }
    let t = ir.terminator(a);
    if u.isnil(t) or (t.op != ir.OP_BR) or (t.t1 != m) {
        return -1;
    }
    let mut n: i32 = 0;
    let mut i = a.first;
    while u.notnil(i) and (i != t) {
        if (i.op == ir.OP_PHI) or not speculatable(i) {
            return -1;
        }
        n += 1;
        i = i.next;
    }
    return n;
}

fn phi_count(m: *ir.Block) i32 {
    let mut n: i32 = 0;
    let mut p = m.first;
    while u.notnil(p) and (p.op == ir.OP_PHI) {
        if p.ty.kind == ir.T_VEC {
            return 1000;
        }
        n += 1;
        p = p.next;
    }
    return n;
}

fn hoist(a: *mut ir.Block, before: *mut ir.Value) void {
    let t = ir.terminator(a);
    let mut i = a.first;
    while u.notnil(i) and (i != t) {
        let nx = i.next;
        ir.remove_inst(i);
        ir.insert_before(before, i);
        i = nx;
    }
}

// The value phi p gets over the edge from block `from` (nil: none).
fn input(p: *mut ir.Value, from: *mut ir.Block) *mut ir.Value {
    return incoming(p, from);
}

fn drop_input(p: *mut ir.Value, from: *mut ir.Block) void {
    let mut k: usize = 0;
    while k < p.bs.len {
        if p.bs.get(k) == from {
            p.bs.set(k, p.bs.get(p.bs.len - 1));
            p.xs.set(k, p.xs.get(p.xs.len - 1));
            p.bs.len -= 1;
            p.xs.len -= 1;
        } else {
            k += 1;
        }
    }
}

// D ends in condbr c -> t, e. Converts the triangles d -> t -> e, d -> e
// and d -> e -> t, d -> t, and the diamond d -> t -> m, d -> e -> m.
fn convert(f: *mut ir.Func, d: *mut ir.Block) bool {
    let br = ir.terminator(d);
    let c = br.a;
    let t = br.t1;
    let e = br.t2;
    if (t == e) or (t == d) or (e == d) {
        return false;
    }
    // the join, and which arms are real blocks
    let mut m = ir.bnil();
    let mut via_t = false;
    let mut via_e = false;
    let tt = ir.terminator(t);
    let et = ir.terminator(e);
    if u.notnil(tt) and (tt.op == ir.OP_BR) and (tt.t1 == e) {
        m = e;
        via_t = true;          // triangle: d -> t -> e, d -> e
    } else if u.notnil(et) and (et.op == ir.OP_BR) and (et.t1 == t) {
        m = t;
        via_e = true;          // triangle: d -> e -> t, d -> t
    } else if u.notnil(tt) and u.notnil(et) and (tt.op == ir.OP_BR) and (et.op == ir.OP_BR) and (tt.t1 == et.t1) {
        m = tt.t1;
        via_t = true;
        via_e = true;          // diamond
    } else {
        return false;
    }
    if (m == d) or not u.notnil(m.first) or (m.first.op != ir.OP_PHI) {
        return false;          // nothing to select (a plain skip is left to simplify_cfg)
    }
    let mut cost: i32 = 0;
    if via_t {
        let x = arm_cost(t, d, m);
        if x < 0 {
            return false;
        }
        cost += x;
    }
    if via_e {
        let x = arm_cost(e, d, m);
        if x < 0 {
            return false;
        }
        cost += x;
    }
    if (cost > MAX_SPEC) or (phi_count(m) > MAX_SELECTS) {
        return false;
    }
    // the values arriving at m on the true and the false path
    let mut tb = d;
    if via_t {
        tb = t;
    }
    let mut eb = d;
    if via_e {
        eb = e;
    }
    if via_t {
        hoist(t, br);
    }
    if via_e {
        hoist(e, br);
    }
    let bld = ir.new_builder();
    ir.at_before(bld, br);
    let mut p = m.first;
    while u.notnil(p) and (p.op == ir.OP_PHI) {
        let vt = input(p, tb);
        let ve = input(p, eb);
        let s = ir.select(bld, c, vt, ve);
        if via_t {
            drop_input(p, t);
        }
        if via_e {
            drop_input(p, e);
        }
        drop_input(p, d);
        ir.add_incoming(p, s, d);
        p = p.next;
    }
    br.op = ir.OP_BR;
    br.a = vnil();
    br.t1 = m;
    br.t2 = ir.bnil();
    return true;
}

pub fn if_convert(f: *mut ir.Func) bool {
    if u.isnil(f.first) {
        return false;
    }
    opt.compute_preds(f);
    let mut any = false;
    let mut b = f.first;
    while u.notnil(b) {
        let t = ir.terminator(b);
        if u.notnil(t) and (t.op == ir.OP_CONDBR) and (t.a.ty.kind == ir.T_INT) {
            if convert(f, b) {
                any = true;
                opt.compute_preds(f);
            }
        }
        b = b.next;
    }
    if any {
        ir.remove_unreachable(f);
    }
    return any;
}
