// =============================================================================
// mach.ph - machine IR shared by the native backends (x64.ph, a64.ph), and
// register allocation.
//
// A machine instruction has target-defined opcode and operands; each
// register operand says whether it is read, written or both. Registers
// below VREG0 are physical (target numbering), the rest virtual.
//
// Register allocation is linear scan over live intervals (one interval per
// virtual register: the positions where it is live, with holes; positions
// are 2i for instruction i's reads and 2i+1 for its writes). Physical registers
// used explicitly (argument and result registers, division, shifts) and
// registers clobbered by calls are fixed ranges an interval must avoid.
// When registers run out, the interval with the lowest spill weight (uses,
// weighted by loop depth, per position covered) is spilled to a stack slot
// and its uses and definitions are rewritten through short-lived registers;
// allocation then runs again (those never spill).
// =============================================================================
import "util.ph" as u;

pub const VREG0: i32 = 64;

// operand kinds
pub const K_NONE: u8 = 0;
pub const K_REG: u8 = 1;
pub const K_IMM: u8 = 2;
pub const K_MEM: u8 = 3;    // [base + index*scale + disp], or [slot + disp], or [sym + disp] (pc-relative)
pub const K_SYM: u8 = 4;    // a symbol (call target, address)
pub const K_LABEL: u8 = 5;  // block / local label

// roles of register operands
pub const RU: u8 = 1;       // read
pub const RD: u8 = 2;       // written
pub const RUD: u8 = 3;
pub const RE: u8 = 4;       // written before all reads are done (early clobber)

pub struct MOp {
    kind: u8,
    role: u8,
    reg: i32,
    base: i32,
    index: i32,
    scale: u8,
    disp: i64,
    sym: i32,
    slot: i32,
    label: i32,
}

pub struct MInst {
    op: i32,
    sz: u8,
    cc: i32,
    ops: []mut MOp,
    clobber: u64,     // physical registers clobbered (calls)
    is_move: bool,    // register copy (for coalescing hints)
    next: *mut MInst,
    prev: *mut MInst,
    pos: i32,
    aux: i64,
}

pub struct MBlock {
    id: i32,
    first: *mut MInst,
    last: *mut MInst,
    succs: u.List.[i32],
    preds: u.List.[i32],
    loop_depth: i32,
    live_in: []mut u64,
    live_out: []mut u64,
    start: i32,
    end: i32,
    offset: u64,      // encoder: code offset
}

pub struct Slot {
    size: u64,
    align: u64,
    offset: i64,      // assigned by the target's frame layout
}

pub struct Target {
    nclasses: i32,
    alloc_order: [2][]i32,   // allocatable registers per class, in preference order
    caller_saved: u64,
    // spill code: load/store of register r (class c) from/to a slot
    spill_load: fn(*mut MFunc, i32, i32, i32) *mut MInst,
    spill_store: fn(*mut MFunc, i32, i32, i32) *mut MInst,
}

pub struct MFunc {
    blocks: u.List.[*mut MBlock],
    vclass: u.List.[u8],      // class of each virtual register (index: vreg - VREG0)
    nospill: u.List.[bool],
    hint: u.List.[i32],
    slots: u.List.[Slot],
    assign: []mut i32,        // after allocation: vreg - VREG0 -> physical register
    used_regs: u64,           // physical registers used (for callee-saved saves)
    ninst: i32,
    has_calls: bool,
}

pub fn new_func() *mut MFunc {
    return u.new.[MFunc]();
}

pub fn new_vreg(f: *mut MFunc, class: u8) i32 {
    f.vclass.push(class);
    f.nospill.push(false);
    f.hint.push(-1);
    return VREG0 + @int_cast(i32, f.vclass.len - 1);
}

pub fn nvregs(f: *MFunc) usize {
    return f.vclass.len;
}

pub fn new_slot(f: *mut MFunc, size: u64, align: u64) i32 {
    f.slots.push(Slot.{ size: size, align: align, offset: 0 });
    return @int_cast(i32, f.slots.len - 1);
}

pub fn new_block(f: *mut MFunc) *mut MBlock {
    let b = u.new.[MBlock]();
    b.id = @int_cast(i32, f.blocks.len);
    f.blocks.push(b);
    return b;
}

pub fn is_vreg(r: i32) bool {
    return r >= VREG0;
}

// Operand constructors
pub fn reg(r: i32, role: u8) MOp {
    return MOp.{ kind: K_REG, role: role, reg: r, base: -1, index: -1, scale: 0, disp: 0, sym: -1, slot: -1, label: -1 };
}

pub fn imm(x: i64) MOp {
    return MOp.{ kind: K_IMM, role: 0, reg: -1, base: -1, index: -1, scale: 0, disp: x, sym: -1, slot: -1, label: -1 };
}

pub fn mem(base: i32, index: i32, scale: u8, disp: i64) MOp {
    return MOp.{ kind: K_MEM, role: 0, reg: -1, base: base, index: index, scale: scale, disp: disp, sym: -1, slot: -1,
                 label: -1 };
}

pub fn mem_slot(slot: i32, disp: i64) MOp {
    return MOp.{ kind: K_MEM, role: 0, reg: -1, base: -1, index: -1, scale: 0, disp: disp, sym: -1, slot: slot,
                 label: -1 };
}

pub fn mem_sym(sym: i32, disp: i64) MOp {
    return MOp.{ kind: K_MEM, role: 0, reg: -1, base: -1, index: -1, scale: 0, disp: disp, sym: sym, slot: -1,
                 label: -1 };
}

pub fn symop(sym: i32, disp: i64) MOp {
    return MOp.{ kind: K_SYM, role: 0, reg: -1, base: -1, index: -1, scale: 0, disp: disp, sym: sym, slot: -1, label: -1 };
}

pub fn label(l: i32) MOp {
    return MOp.{ kind: K_LABEL, role: 0, reg: -1, base: -1, index: -1, scale: 0, disp: 0, sym: -1, slot: -1, label: l };
}

pub fn none() MOp {
    return MOp.{ kind: K_NONE, role: 0, reg: -1, base: -1, index: -1, scale: 0, disp: 0, sym: -1, slot: -1, label: -1 };
}

pub fn new_inst(op: i32, sz: u8, ops: []MOp) *mut MInst {
    let i = u.new.[MInst]();
    i.op = op;
    i.sz = sz;
    let o = u.newn.[MOp](ops.len + 1);
    for k in 0..ops.len {
        o[k] = ops[k];
    }
    i.ops = o[0..ops.len];
    return i;
}

pub fn append(b: *mut MBlock, i: *mut MInst) void {
    i.prev = b.last;
    i.next = u.nil.[MInst]();
    if u.notnil(b.last) {
        b.last.next = i;
    } else {
        b.first = i;
    }
    b.last = i;
}

pub fn insert_before(b: *mut MBlock, pos: *mut MInst, i: *mut MInst) void {
    i.next = pos;
    i.prev = pos.prev;
    if u.notnil(pos.prev) {
        pos.prev.next = i;
    } else {
        b.first = i;
    }
    pos.prev = i;
}

pub fn insert_after(b: *mut MBlock, pos: *mut MInst, i: *mut MInst) void {
    i.prev = pos;
    i.next = pos.next;
    if u.notnil(pos.next) {
        pos.next.prev = i;
    } else {
        b.last = i;
    }
    pos.next = i;
}

pub fn remove(b: *mut MBlock, i: *mut MInst) void {
    if u.notnil(i.prev) {
        i.prev.next = i.next;
    } else {
        b.first = i.next;
    }
    if u.notnil(i.next) {
        i.next.prev = i.prev;
    } else {
        b.last = i.prev;
    }
}

// -----------------------------------------------------------------------------
// Liveness
// -----------------------------------------------------------------------------
fn words(n: usize) usize {
    return (n + 63) / 64;
}

fn bset(s: []mut u64, i: usize) void {
    s[i / 64] = s[i / 64] | (@as(u64, 1) << @int_cast(u32, i % 64));
}

fn bclr(s: []mut u64, i: usize) void {
    s[i / 64] = s[i / 64] & ~(@as(u64, 1) << @int_cast(u32, i % 64));
}

fn bget(s: []u64, i: usize) bool {
    return (s[i / 64] & (@as(u64, 1) << @int_cast(u32, i % 64))) != 0;
}

// Calls fn(reg, role) for every register operand of i (memory bases and
// indexes are reads).
fn each_reg_use(i: *MInst, v: i32) bool {
    for k in 0..i.ops.len {
        let o = i.ops[k];
        if (o.kind == K_REG) and (o.reg == v) and ((o.role & RU) != 0) {
            return true;
        }
        if (o.kind == K_MEM) and ((o.base == v) or (o.index == v)) {
            return true;
        }
    }
    return false;
}

fn number(f: *mut MFunc) void {
    let mut n: i32 = 0;
    for bi in 0..f.blocks.len {
        let b = f.blocks.get(bi);
        b.start = 2 * n;
        let mut i = b.first;
        while u.notnil(i) {
            i.pos = n;
            n += 1;
            i = i.next;
        }
        b.end = (2 * n) - 1;
        if b.end < b.start {
            b.end = b.start;
        }
    }
    f.ninst = n;
}

fn liveness(f: *mut MFunc) void {
    let nv = nvregs(f);
    let w = words(nv);
    let nb = f.blocks.len;
    let use_s = u.newn.[[]mut u64](nb + 1);
    let def_s = u.newn.[[]mut u64](nb + 1);
    for bi in 0..nb {
        let b = f.blocks.get(bi);
        use_s[bi] = u.newn.[u64](w + 1);
        def_s[bi] = u.newn.[u64](w + 1);
        b.live_in = u.newn.[u64](w + 1);
        b.live_out = u.newn.[u64](w + 1);
        let mut i = b.first;
        while u.notnil(i) {
            for k in 0..i.ops.len {
                let o = i.ops[k];
                if o.kind == K_REG {
                    if is_vreg(o.reg) and ((o.role & RU) != 0) {
                        let x = @int_cast(usize, o.reg - VREG0);
                        if not bget(def_s[bi], x) {
                            bset(use_s[bi], x);
                        }
                    }
                } else if o.kind == K_MEM {
                    if is_vreg(o.base) {
                        let x = @int_cast(usize, o.base - VREG0);
                        if not bget(def_s[bi], x) {
                            bset(use_s[bi], x);
                        }
                    }
                    if is_vreg(o.index) {
                        let x = @int_cast(usize, o.index - VREG0);
                        if not bget(def_s[bi], x) {
                            bset(use_s[bi], x);
                        }
                    }
                }
            }
            for k in 0..i.ops.len {
                let o = i.ops[k];
                if (o.kind == K_REG) and is_vreg(o.reg) and ((o.role & RD) != 0) {
                    bset(def_s[bi], @int_cast(usize, o.reg - VREG0));
                }
            }
            i = i.next;
        }
    }
    let mut changed = true;
    while changed {
        changed = false;
        let mut bi = nb;
        while bi > 0 {
            bi -= 1;
            let b = f.blocks.get(bi);
            // out = union of successors' in
            for k in 0..b.succs.len {
                let s = f.blocks.get(@int_cast(usize, b.succs.get(k)));
                for j in 0..w {
                    b.live_out[j] = b.live_out[j] | s.live_in[j];
                }
            }
            for j in 0..w {
                let nin = use_s[bi][j] | (b.live_out[j] & ~def_s[bi][j]);
                if nin != b.live_in[j] {
                    b.live_in[j] = nin;
                    changed = true;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Intervals and fixed ranges
// -----------------------------------------------------------------------------
struct Range {
    s: i32,
    e: i32,
}

let mut fixed: [64]u.List.[Range] = [u.List.[Range].{ data: @ptr_from_int(*mut Range, 0), len: 0, cap: 0 }; 64];
// after build_fixed: each register's ranges sorted by start, and the
// largest end among the first k+1 of them
let mut fixed_maxe: [64]u.List.[i32] = [u.List.[i32].{ data: @ptr_from_int(*mut i32, 0), len: 0, cap: 0 }; 64];

fn add_fixed(r: i32, s: i32, e: i32) void {
    fixed[@int_cast(usize, r)].push(Range.{ s: s, e: e });
}

fn range_before(a: Range, b: Range) i32 {
    if a.s < b.s {
        return -1;
    }
    if a.s > b.s {
        return 1;
    }
    return 0;
}

fn sort_fixed() void {
    for r in @as(usize, 0)..64 {
        let l = &mut fixed[r];
        u.sort.[Range](l.items(), range_before);
        let m = &mut fixed_maxe[r];
        m.clear();
        let mut mx: i32 = -1;
        for k in 0..l.len {
            let x = l.get(k);
            if x.e > mx {
                mx = x.e;
            }
            m.push(mx);
        }
    }
}

// Does a fixed range of r overlap [s, e]?
fn fixed_conflict(r: i32, s: i32, e: i32) bool {
    let l = &fixed[@int_cast(usize, r)];
    // the ranges starting at or before e: a prefix
    let mut lo: usize = 0;
    let mut hi = l.len;
    while lo < hi {
        let mid = (lo + hi) / 2;
        if l.get(mid).s <= e {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    if lo == 0 {
        return false;
    }
    return fixed_maxe[@int_cast(usize, r)].get(lo - 1) >= s;
}

fn build_fixed(f: *mut MFunc) void {
    for r in @as(usize, 0)..64 {
        fixed[r].clear();
    }
    for bi in 0..f.blocks.len {
        let b = f.blocks.get(bi);
        // per physical register: the position of the next read (scanning backwards)
        let mut next_use: [64]i32 = [-1; 64];
        let mut i = b.last;
        while u.notnil(i) {
            let p = i.pos;
            if i.clobber != 0 {
                for r in @as(u32, 0)..64 {
                    if (i.clobber & (@as(u64, 1) << r)) != 0 {
                        add_fixed(@as(i32, @bit_cast(i32, r)), (2 * p) + 1, (2 * p) + 1);
                    }
                }
            }
            for k in 0..i.ops.len {
                let o = i.ops[k];
                if (o.kind == K_REG) and not is_vreg(o.reg) and (o.reg >= 0) and ((o.role & RD) != 0) {
                    let ri = @int_cast(usize, o.reg);
                    let mut s = (2 * p) + 1;
                    if (o.role & RE) != 0 {
                        s = 2 * p;
                    }
                    let mut e = next_use[ri];
                    if e < 0 {
                        e = s;
                    }
                    add_fixed(o.reg, s, e);
                    next_use[ri] = -1;
                }
            }
            for k in 0..i.ops.len {
                let o = i.ops[k];
                if (o.kind == K_REG) and not is_vreg(o.reg) and (o.reg >= 0) and ((o.role & RU) != 0) {
                    let ri = @int_cast(usize, o.reg);
                    if next_use[ri] < 0 {
                        next_use[ri] = 2 * p;
                    }
                }
                if o.kind == K_MEM {
                    if (o.base >= 0) and not is_vreg(o.base) {
                        let ri = @int_cast(usize, o.base);
                        if next_use[ri] < 0 {
                            next_use[ri] = 2 * p;
                        }
                    }
                    if (o.index >= 0) and not is_vreg(o.index) {
                        let ri = @int_cast(usize, o.index);
                        if next_use[ri] < 0 {
                            next_use[ri] = 2 * p;
                        }
                    }
                }
            }
            i = i.prev;
        }
        // registers read before any write in the block: live from its start
        for r in @as(usize, 0)..64 {
            if next_use[r] >= 0 {
                add_fixed(@int_cast(i32, r), b.start, next_use[r]);
            }
        }
    }
    sort_fixed();
}

struct Interval {
    v: i32,       // vreg index (vreg - VREG0)
    s: i32,       // first position
    e: i32,       // last position
    reg: i32,
    spilled: bool,
    uses: f64,    // uses and definitions, weighted by 10^loop depth
    ranges: u.List.[Range],  // the positions it is live at: sorted, disjoint, inclusive
    cur: usize,   // scan cursor into ranges
    size: i32,    // positions covered
}

// The cost of spilling an interval: its (weighted) uses per position covered.
fn weight(f: *MFunc, x: *Interval) f64 {
    if f.nospill.get(@int_cast(usize, x.v)) {
        return 1.0e300;
    }
    return x.uses / @float_from_int(f64, x.size + 1);
}

fn depth_weight(d: i32) f64 {
    let mut w: f64 = 1.0;
    let mut k: i32 = 0;
    while (k < d) and (k < 6) {
        w = w * 10.0;
        k += 1;
    }
    return w;
}

// Ranges are built backwards (blocks in reverse order, instructions last to
// first), so they are appended in decreasing order and reversed at the end;
// the last one is the current block's.
fn add_range(x: *mut Interval, s: i32, e: i32) void {
    if x.ranges.len > 0 {
        let l = x.ranges.at(x.ranges.len - 1);
        if l.s <= (e + 1) {
            if s < l.s {
                l.s = s;
            }
            if e > l.e {
                l.e = e;
            }
            return;
        }
    }
    x.ranges.push(Range.{ s: s, e: e });
}

// A definition at p: the value is live from p (not from the block's start).
fn def_at(x: *mut Interval, p: i32, live: bool) void {
    if live and (x.ranges.len > 0) {
        let l = x.ranges.at(x.ranges.len - 1);
        if p <= l.e {
            l.s = p;
            return;
        }
    }
    x.ranges.push(Range.{ s: p, e: p });  // dead definition
}

fn build_intervals(f: *mut MFunc) []mut Interval {
    let nv = nvregs(f);
    let iv = u.newn.[Interval](nv + 1);
    for k in 0..nv {
        iv[k] = Interval.{ v: @int_cast(i32, k), s: 0x7fff_ffff, e: -1, reg: -1, spilled: false, uses: 0.0,
                           ranges: u.List.[Range].{ data: u.nil.[Range](), len: 0, cap: 0 }, cur: 0, size: 0 };
    }
    let w = words(nv);
    let live = u.newn.[u64](w + 1);
    let mut bi = f.blocks.len;
    while bi > 0 {
        bi -= 1;
        let b = f.blocks.get(bi);
        let dw = depth_weight(b.loop_depth);
        for j in 0..w {
            live[j] = b.live_out[j];
            let lo = b.live_out[j];
            if lo == 0 {
                continue;
            }
            for bit in @as(usize, 0)..64 {
                if (lo & (@as(u64, 1) << @int_cast(u32, bit))) != 0 {
                    add_range(&mut iv[(j * 64) + bit], b.start, b.end);
                }
            }
        }
        let mut i = b.last;
        while u.notnil(i) {
            let p = i.pos;
            // definitions end the value's life above them
            for k in 0..i.ops.len {
                let o = i.ops[k];
                if (o.kind == K_REG) and is_vreg(o.reg) and ((o.role & RD) != 0) {
                    let x = @int_cast(usize, o.reg - VREG0);
                    let mut dp = (2 * p) + 1;
                    if (o.role & RE) != 0 {
                        dp = 2 * p;
                    }
                    def_at(&mut iv[x], dp, bget(live, x));
                    bclr(live, x);
                    iv[x].uses += dw;
                }
            }
            // uses make it live from the block's start (until a definition)
            for k in 0..i.ops.len {
                let o = i.ops[k];
                if (o.kind == K_REG) and is_vreg(o.reg) and ((o.role & RU) != 0) {
                    let x = @int_cast(usize, o.reg - VREG0);
                    add_range(&mut iv[x], b.start, 2 * p);
                    bset(live, x);
                    iv[x].uses += dw;
                } else if o.kind == K_MEM {
                    let regs: [2]i32 = [o.base, o.index];
                    for r in regs {
                        if is_vreg(r) {
                            let x = @int_cast(usize, r - VREG0);
                            add_range(&mut iv[x], b.start, 2 * p);
                            bset(live, x);
                            iv[x].uses += dw;
                        }
                    }
                }
            }
            i = i.prev;
        }
    }
    for k in 0..nv {
        let x = &mut iv[k];
        let n = x.ranges.len;
        if n == 0 {
            continue;
        }
        // increasing order
        let rs = x.ranges.items();
        for j in 0..(n / 2) {
            let t = rs[j];
            rs[j] = rs[(n - 1) - j];
            rs[(n - 1) - j] = t;
        }
        x.s = rs[0].s;
        x.e = rs[n - 1].e;
        for j in 0..n {
            x.size += (rs[j].e - rs[j].s) + 1;
        }
    }
    return iv[0..nv];
}

// Does x cover position p? (p never decreases between calls: a cursor.)
fn covers(x: *mut Interval, p: i32) bool {
    while (x.cur < x.ranges.len) and (x.ranges.get(x.cur).e < p) {
        x.cur += 1;
    }
    return (x.cur < x.ranges.len) and (x.ranges.get(x.cur).s <= p);
}

// Do a and b overlap anywhere?
fn intersects(a: *Interval, b: *Interval) bool {
    let mut i = a.cur;  // the ranges before the cursors end before the current position
    let mut j = b.cur;
    while (i < a.ranges.len) and (j < b.ranges.len) {
        let x = a.ranges.get(i);
        let y = b.ranges.get(j);
        if x.e < y.s {
            i += 1;
        } else if y.e < x.s {
            j += 1;
        } else {
            return true;
        }
    }
    return false;
}

// Does a fixed range of r overlap x?
fn fixed_hits(r: i32, x: *Interval) bool {
    for k in x.cur..x.ranges.len {
        let y = x.ranges.get(k);
        if fixed_conflict(r, y.s, y.e) {
            return true;
        }
    }
    return false;
}

fn sort_by_start(xs: []mut *mut Interval) void {
    // merge sort by start (stable)
    let n = xs.len;
    if n < 2 {
        return;
    }
    let tmp = u.newn.[*mut Interval](n);
    let mut width: usize = 1;
    while width < n {
        let mut i: usize = 0;
        while i < n {
            let mid = min(i + width, n);
            let hi = min(i + (2 * width), n);
            let mut a = i;
            let mut b = mid;
            let mut k = i;
            while k < hi {
                if (a < mid) and ((b >= hi) or (xs[a].s <= xs[b].s)) {
                    tmp[k] = xs[a];
                    a += 1;
                } else {
                    tmp[k] = xs[b];
                    b += 1;
                }
                k += 1;
            }
            i += 2 * width;
        }
        for k in 0..n {
            xs[k] = tmp[k];
        }
        width *= 2;
    }
}

fn min(a: usize, b: usize) usize {
    if a < b {
        return a;
    }
    return b;
}

fn drop_at(l: *mut u.List.[*mut Interval], k: usize) void {
    l.set(k, l.get(l.len - 1));
    l.len -= 1;
}

// One allocation round (linear scan with lifetime holes: an interval in a
// hole is inactive and leaves its register to intervals that fit in the
// hole). Returns the number of intervals spilled.
fn scan(f: *mut MFunc, t: *Target, iv: []mut Interval) usize {
    let mut list = u.List.[*mut Interval].{ data: u.nil.[*mut Interval](), len: 0, cap: 0 };
    for k in 0..iv.len {
        if iv[k].e >= 0 {
            list.push(&mut iv[k]);
        }
    }
    let xs = list.items();
    sort_by_start(xs);
    let mut active = u.List.[*mut Interval].{ data: u.nil.[*mut Interval](), len: 0, cap: 0 };
    let mut inactive = u.List.[*mut Interval].{ data: u.nil.[*mut Interval](), len: 0, cap: 0 };
    let mut nspill: usize = 0;
    for ci in 0..xs.len {
        let cur = xs[ci];
        let pos = cur.s;
        let mut k: usize = 0;
        while k < active.len {
            let a = active.get(k);
            if a.e < pos {
                drop_at(&mut active, k);
            } else if not covers(a, pos) {
                inactive.push(a);
                drop_at(&mut active, k);
            } else {
                k += 1;
            }
        }
        k = 0;
        while k < inactive.len {
            let a = inactive.get(k);
            if a.e < pos {
                drop_at(&mut inactive, k);
            } else if covers(a, pos) {
                active.push(a);
                drop_at(&mut inactive, k);
            } else {
                k += 1;
            }
        }
        let class = f.vclass.get(@int_cast(usize, cur.v));
        let order = t.alloc_order[@as(usize, class)];
        // registers taken for (part of) cur's life
        let mut taken: [64]bool = [false; 64];
        for j in 0..active.len {
            taken[@int_cast(usize, active.get(j).reg)] = true;
        }
        for j in 0..inactive.len {
            let a = inactive.get(j);
            if not taken[@int_cast(usize, a.reg)] and intersects(a, cur) {
                taken[@int_cast(usize, a.reg)] = true;
            }
        }
        let mut pick: i32 = -1;
        // the hint first
        let h = f.hint.get(@int_cast(usize, cur.v));
        let mut hr: i32 = -1;
        if (h >= 0) and not is_vreg(h) {
            hr = h;
        } else if is_vreg(h) and (iv[@int_cast(usize, h - VREG0)].reg >= 0) {
            hr = iv[@int_cast(usize, h - VREG0)].reg;
        }
        if (hr >= 0) and in_order(order, hr) and not taken[@int_cast(usize, hr)] and not fixed_hits(hr, cur) {
            pick = hr;
        }
        if pick < 0 {
            for r in order {
                if not taken[@int_cast(usize, r)] and not fixed_hits(r, cur) {
                    pick = r;
                    break;
                }
            }
        }
        if pick < 0 {
            // free a register: the one whose intervals overlapping cur cost
            // least to spill (if less than spilling cur, or cur must not spill)
            let mut best: i32 = -1;
            let mut bw: f64 = 0.0;
            for r in order {
                if fixed_hits(r, cur) {
                    continue;
                }
                let mut w: f64 = 0.0;
                for j in 0..active.len {
                    let a = active.get(j);
                    if a.reg == r {
                        w += weight(f, a);
                    }
                }
                for j in 0..inactive.len {
                    let a = inactive.get(j);
                    if (a.reg == r) and intersects(a, cur) {
                        w += weight(f, a);
                    }
                }
                if (best < 0) or (w < bw) {
                    best = r;
                    bw = w;
                }
            }
            let must = f.nospill.get(@int_cast(usize, cur.v));
            if (best >= 0) and ((bw < weight(f, cur)) or must) and (bw < 1.0e299) {
                pick = best;
                k = 0;
                while k < active.len {
                    let a = active.get(k);
                    if a.reg == pick {
                        a.spilled = true;
                        a.reg = -1;
                        nspill += 1;
                        drop_at(&mut active, k);
                    } else {
                        k += 1;
                    }
                }
                k = 0;
                while k < inactive.len {
                    let a = inactive.get(k);
                    if (a.reg == pick) and intersects(a, cur) {
                        a.spilled = true;
                        a.reg = -1;
                        nspill += 1;
                        drop_at(&mut inactive, k);
                    } else {
                        k += 1;
                    }
                }
            } else {
                if must {
                    u.die("internal compiler error: register allocation failed (no register for a reload)");
                }
                cur.spilled = true;
                nspill += 1;
                continue;
            }
        }
        cur.reg = pick;
        active.push(cur);
    }
    u.free(@ptr_cast(*mut void, list.data));
    u.free(@ptr_cast(*mut void, active.data));
    u.free(@ptr_cast(*mut void, inactive.data));
    for k in 0..iv.len {
        u.free(@ptr_cast(*mut void, iv[k].ranges.data));
    }
    return nspill;
}

fn in_order(order: []i32, r: i32) bool {
    for x in order {
        if x == r {
            return true;
        }
    }
    return false;
}

// Rewrites spilled vregs: each instruction reading one gets a load into a
// fresh (unspillable) vreg before it; each writing one, a store after it.
fn rewrite_spills(f: *mut MFunc, t: *Target, iv: []mut Interval) void {
    let nv = iv.len;
    let slot = u.newn.[i32](nv + 1);
    for k in 0..nv {
        slot[k] = -1;
        if iv[k].spilled {
            let c = f.vclass.get(k);
            let mut sz: u64 = 8;
            if c == 1 {
                sz = 16;
            }
            slot[k] = new_slot(f, sz, sz);
        }
    }
    for bi in 0..f.blocks.len {
        let b = f.blocks.get(bi);
        let mut i = b.first;
        while u.notnil(i) {
            let next = i.next;
            // distinct spilled vregs of this instruction
            let mut done: [8]i32 = [-1; 8];
            let mut nd: usize = 0;
            for k in 0..i.ops.len {
                let o = i.ops[k];
                let cands: [3]i32 = [o.reg, o.base, o.index];
                for ci in @as(usize, 0)..3 {
                    let r = cands[ci];
                    if (ci == 0) and (o.kind != K_REG) {
                        continue;
                    }
                    if (ci > 0) and (o.kind != K_MEM) {
                        continue;
                    }
                    if not is_vreg(r) or (slot[@int_cast(usize, r - VREG0)] < 0) {
                        continue;
                    }
                    let mut seen = false;
                    for j in 0..nd {
                        if done[j] == r {
                            seen = true;
                        }
                    }
                    if not seen and (nd < 8) {
                        done[nd] = r;
                        nd += 1;
                    }
                }
            }
            for j in 0..nd {
                let r = done[j];
                let x = @int_cast(usize, r - VREG0);
                let c = f.vclass.get(x);
                let nr = new_vreg(f, c);
                f.nospill.set(@int_cast(usize, nr - VREG0), true);
                let mut reads = false;
                let mut writes = false;
                for k in 0..i.ops.len {
                    let o = &mut i.ops[k];
                    if (o.kind == K_REG) and (o.reg == r) {
                        o.reg = nr;
                        if (o.role & RU) != 0 {
                            reads = true;
                        }
                        if (o.role & RD) != 0 {
                            writes = true;
                        }
                    }
                    if o.kind == K_MEM {
                        if o.base == r {
                            o.base = nr;
                            reads = true;
                        }
                        if o.index == r {
                            o.index = nr;
                            reads = true;
                        }
                    }
                }
                if reads {
                    insert_before(b, i, t.spill_load(f, nr, @as(i32, c), slot[x]));
                }
                if writes {
                    insert_after(b, i, t.spill_store(f, nr, @as(i32, c), slot[x]));
                }
            }
            i = next;
        }
    }
}

// Allocates registers; afterwards f.assign maps every vreg to a physical
// register.
pub fn allocate(f: *mut MFunc, t: *Target) void {
    let mut rounds: i32 = 0;
    loop {
        number(f);
        liveness(f);
        build_fixed(f);
        let iv = build_intervals(f);
        let ns = scan(f, t, iv);
        rounds += 1;
        if ns == 0 {
            let nv = iv.len;
            f.assign = u.newn.[i32](nv + 1);
            f.used_regs = 0;
            for k in 0..nv {
                f.assign[k] = iv[k].reg;
                if iv[k].reg >= 0 {
                    f.used_regs = f.used_regs | (@as(u64, 1) << @int_cast(u32, iv[k].reg));
                }
            }
            return;
        }
        if rounds > 40 {
            u.die("internal compiler error: register allocation did not converge");
        }
        rewrite_spills(f, t, iv);
    }
}

// The physical register for r (a vreg after allocation, or physical).
pub fn phys(f: *MFunc, r: i32) i32 {
    if is_vreg(r) {
        return f.assign[@int_cast(usize, r - VREG0)];
    }
    return r;
}

// -----------------------------------------------------------------------------
// Dead instructions: `pure` instructions (no effect but their register
// results, no flags anything reads) whose virtual register results are never
// read are deleted, repeatedly (address arithmetic folded into memory
// operands leaves such chains behind).
// -----------------------------------------------------------------------------
fn reads_of(i: *MInst, v: i32) i32 {
    let mut n: i32 = 0;
    for k in 0..i.ops.len {
        let o = i.ops[k];
        if (o.kind == K_REG) and (o.reg == v) and ((o.role & RU) != 0) {
            n += 1;
        }
        if o.kind == K_MEM {
            if o.base == v {
                n += 1;
            }
            if o.index == v {
                n += 1;
            }
        }
    }
    return n;
}

fn count_reads(i: *MInst, cnt: []mut i32, delta: i32) void {
    for k in 0..i.ops.len {
        let o = i.ops[k];
        if (o.kind == K_REG) and is_vreg(o.reg) and ((o.role & RU) != 0) {
            cnt[@int_cast(usize, o.reg - VREG0)] += delta;
        }
        if o.kind == K_MEM {
            if is_vreg(o.base) {
                cnt[@int_cast(usize, o.base - VREG0)] += delta;
            }
            if is_vreg(o.index) {
                cnt[@int_cast(usize, o.index - VREG0)] += delta;
            }
        }
    }
}

fn dead(i: *MInst, cnt: []mut i32) bool {
    let mut defs: i32 = 0;
    for k in 0..i.ops.len {
        let o = i.ops[k];
        if (o.kind == K_REG) and ((o.role & RD) != 0) {
            if not is_vreg(o.reg) {
                return false;  // a physical register (an argument, say)
            }
            if (cnt[@int_cast(usize, o.reg - VREG0)] - reads_of(i, o.reg)) != 0 {
                return false;
            }
            defs += 1;
        }
    }
    return defs > 0;
}

pub fn dce(f: *mut MFunc, pure: fn(*MInst) bool) void {
    let cnt = u.newn.[i32](nvregs(f) + 1);
    for bi in 0..f.blocks.len {
        let mut i = f.blocks.get(bi).first;
        while u.notnil(i) {
            count_reads(i, cnt, 1);
            i = i.next;
        }
    }
    let mut changed = true;
    while changed {
        changed = false;
        for bi in 0..f.blocks.len {
            let b = f.blocks.get(bi);
            let mut i = b.last;
            while u.notnil(i) {
                let prev = i.prev;
                if pure(i) and dead(i, cnt) {
                    count_reads(i, cnt, -1);
                    remove(b, i);
                    changed = true;
                }
                i = prev;
            }
        }
    }
}
