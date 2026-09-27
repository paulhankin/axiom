// =============================================================================
// wasmops.ph - the WebAssembly instructions accepted inside `wasm { }` blocks,
// built on first use.
//
// Value-type letters: i = i32, l = i64, f = f32, d = f64, v = v128,
// p = address (i32 on 32-bit targets, i64 on 64-bit), * = any (polymorphic).
// =============================================================================
import "util.ph" as u;

pub const WIMM_NONE: i32 = 0;
pub const WIMM_LOCAL: i32 = 1;
pub const WIMM_I32: i32 = 2;
pub const WIMM_I64: i32 = 3;
pub const WIMM_F32: i32 = 4;
pub const WIMM_F64: i32 = 5;
pub const WIMM_MEM: i32 = 6;
pub const WIMM_LANE: i32 = 7;

pub struct WasmOpInfo {
    name: []u8,
    prefix: []u8,     // "i32", "f32x4", "local", ... (text before the first '.')
    op: []u8,         // text after the first '.', or the whole name
    ins: []u8,        // operand types (bottom of stack first)
    out: u8,          // result type or 0
    imm: i32,
    mem_bytes: i32,   // loads/stores: access width
    mem_signed: bool, // loads: sign-extend
}

let mut ops: u.SymMap.[*mut WasmOpInfo] = u.SymMap.[*mut WasmOpInfo].{
    keys: @ptr_from_int(*mut usize, 0), vals: @ptr_from_int(*mut *mut WasmOpInfo, 0), cap: 0, len: 0 };
let mut built: bool = false;

fn add(prefix: []u8, op: []u8, ins: []u8, out: u8, imm: i32, mem_bytes: i32, msigned: bool) void {
    let w = u.new.[WasmOpInfo]();
    if prefix.len > 0 {
        w.name = u.intern(u.f2("%s.%s", prefix, op));
    } else {
        w.name = u.intern(op);
    }
    w.prefix = prefix;
    w.op = op;
    w.ins = u.dup(ins);
    w.out = out;
    w.imm = imm;
    w.mem_bytes = mem_bytes;
    w.mem_signed = msigned;
    ops.put(w.name, w);
}

fn build() void {
    built = true;
    add("", "nop", "", 0, WIMM_NONE, 0, false);
    add("", "unreachable", "", 0, WIMM_NONE, 0, false);
    add("", "drop", "*", 0, WIMM_NONE, 0, false);
    add("", "select", "**i", '*', WIMM_NONE, 0, false);
    add("local", "get", "", '*', WIMM_LOCAL, 0, false);
    add("local", "set", "*", 0, WIMM_LOCAL, 0, false);
    add("local", "tee", "*", '*', WIMM_LOCAL, 0, false);

    let ibin = ["add", "sub", "mul", "div_s", "div_u", "rem_s", "rem_u", "and", "or", "xor", "shl", "shr_s", "shr_u",
                "rotl", "rotr"];
    let icmp = ["eq", "ne", "lt_s", "lt_u", "gt_s", "gt_u", "le_s", "le_u", "ge_s", "ge_u"];
    let fbin = ["add", "sub", "mul", "div", "min", "max", "copysign"];
    let fun = ["abs", "neg", "sqrt", "ceil", "floor", "trunc", "nearest"];
    let fcmp = ["eq", "ne", "lt", "gt", "le", "ge"];
    for w in @as(i32, 0)..2 {
        let wide = w != 0;
        let p = u.pick(wide, "i64", "i32");
        let mut t: u8 = 'i';
        if wide {
            t = 'l';
        }
        let tt = u.dup(u.pick(wide, "ll", "ii"));
        let t1 = u.dup(u.pick(wide, "l", "i"));
        let st = u.pick(wide, "pl", "pi");
        for x in ibin {
            add(p, x, tt, t, WIMM_NONE, 0, false);
        }
        for x in icmp {
            add(p, x, tt, 'i', WIMM_NONE, 0, false);
        }
        add(p, "clz", t1, t, WIMM_NONE, 0, false);
        add(p, "ctz", t1, t, WIMM_NONE, 0, false);
        add(p, "popcnt", t1, t, WIMM_NONE, 0, false);
        add(p, "eqz", t1, 'i', WIMM_NONE, 0, false);
        let mut ci = WIMM_I32;
        let mut wb: i32 = 4;
        if wide {
            ci = WIMM_I64;
            wb = 8;
        }
        add(p, "const", "", t, ci, 0, false);
        add(p, "load", "p", t, WIMM_MEM, wb, false);
        add(p, "store", st, 0, WIMM_MEM, wb, false);
        add(p, "load8_s", "p", t, WIMM_MEM, 1, true);
        add(p, "load8_u", "p", t, WIMM_MEM, 1, false);
        add(p, "load16_s", "p", t, WIMM_MEM, 2, true);
        add(p, "load16_u", "p", t, WIMM_MEM, 2, false);
        add(p, "store8", st, 0, WIMM_MEM, 1, false);
        add(p, "store16", st, 0, WIMM_MEM, 2, false);
        add(p, "extend8_s", t1, t, WIMM_NONE, 0, false);
        add(p, "extend16_s", t1, t, WIMM_NONE, 0, false);
        add(p, "trunc_sat_f32_s", "f", t, WIMM_NONE, 0, false);
        add(p, "trunc_sat_f32_u", "f", t, WIMM_NONE, 0, false);
        add(p, "trunc_sat_f64_s", "d", t, WIMM_NONE, 0, false);
        add(p, "trunc_sat_f64_u", "d", t, WIMM_NONE, 0, false);
    }
    add("i64", "load32_s", "p", 'l', WIMM_MEM, 4, true);
    add("i64", "load32_u", "p", 'l', WIMM_MEM, 4, false);
    add("i64", "store32", "pl", 0, WIMM_MEM, 4, false);
    add("i64", "extend32_s", "l", 'l', WIMM_NONE, 0, false);
    add("i32", "wrap_i64", "l", 'i', WIMM_NONE, 0, false);
    add("i64", "extend_i32_s", "i", 'l', WIMM_NONE, 0, false);
    add("i64", "extend_i32_u", "i", 'l', WIMM_NONE, 0, false);
    add("i32", "reinterpret_f32", "f", 'i', WIMM_NONE, 0, false);
    add("i64", "reinterpret_f64", "d", 'l', WIMM_NONE, 0, false);
    add("f32", "reinterpret_i32", "i", 'f', WIMM_NONE, 0, false);
    add("f64", "reinterpret_i64", "l", 'd', WIMM_NONE, 0, false);
    add("f32", "demote_f64", "d", 'f', WIMM_NONE, 0, false);
    add("f64", "promote_f32", "f", 'd', WIMM_NONE, 0, false);
    for w in @as(i32, 0)..2 {
        let wide = w != 0;
        let p = u.pick(wide, "f64", "f32");
        let mut t: u8 = 'f';
        if wide {
            t = 'd';
        }
        let tt = u.pick(wide, "dd", "ff");
        let t1 = u.pick(wide, "d", "f");
        for x in fbin {
            add(p, x, tt, t, WIMM_NONE, 0, false);
        }
        for x in fun {
            add(p, x, t1, t, WIMM_NONE, 0, false);
        }
        for x in fcmp {
            add(p, x, tt, 'i', WIMM_NONE, 0, false);
        }
        let mut ci = WIMM_F32;
        let mut wb: i32 = 4;
        if wide {
            ci = WIMM_F64;
            wb = 8;
        }
        add(p, "const", "", t, ci, 0, false);
        add(p, "load", "p", t, WIMM_MEM, wb, false);
        add(p, "store", u.pick(wide, "pd", "pf"), 0, WIMM_MEM, wb, false);
        add(p, "convert_i32_s", "i", t, WIMM_NONE, 0, false);
        add(p, "convert_i32_u", "i", t, WIMM_NONE, 0, false);
        add(p, "convert_i64_s", "l", t, WIMM_NONE, 0, false);
        add(p, "convert_i64_u", "l", t, WIMM_NONE, 0, false);
    }
    // SIMD (a practical subset)
    add("v128", "load", "p", 'v', WIMM_MEM, 16, false);
    add("v128", "store", "pv", 0, WIMM_MEM, 16, false);
    add("v128", "and", "vv", 'v', WIMM_NONE, 0, false);
    add("v128", "or", "vv", 'v', WIMM_NONE, 0, false);
    add("v128", "xor", "vv", 'v', WIMM_NONE, 0, false);
    add("v128", "andnot", "vv", 'v', WIMM_NONE, 0, false);
    add("v128", "not", "v", 'v', WIMM_NONE, 0, false);
    let shapes = ["i8x16", "i16x8", "i32x4", "i64x2", "f32x4", "f64x2"];
    let scalars = "iiilfd";
    for s in @as(usize, 0)..6 {
        let p = shapes[s];
        let sc = scalars[s];
        let is_float = s >= 4;
        let has_mul = s != 0;
        let scs = [sc];
        let vs: [2]u8 = ['v', sc];
        let scstr = u.dup(scs[..]);
        let vsstr = u.dup(vs[..]);
        add(p, "add", "vv", 'v', WIMM_NONE, 0, false);
        add(p, "sub", "vv", 'v', WIMM_NONE, 0, false);
        if has_mul {
            add(p, "mul", "vv", 'v', WIMM_NONE, 0, false);
        }
        add(p, "neg", "v", 'v', WIMM_NONE, 0, false);
        add(p, "splat", scstr, 'v', WIMM_NONE, 0, false);
        add(p, "replace_lane", vsstr, 'v', WIMM_LANE, 0, false);
        if is_float {
            add(p, "div", "vv", 'v', WIMM_NONE, 0, false);
            add(p, "sqrt", "v", 'v', WIMM_NONE, 0, false);
            add(p, "min", "vv", 'v', WIMM_NONE, 0, false);
            add(p, "max", "vv", 'v', WIMM_NONE, 0, false);
            add(p, "abs", "v", 'v', WIMM_NONE, 0, false);
            add(p, "extract_lane", "v", sc, WIMM_LANE, 0, false);
        } else if (sc == 'l') or u.str_eq(p, "i32x4") {
            add(p, "extract_lane", "v", sc, WIMM_LANE, 0, false);
        } else {
            add(p, "extract_lane_s", "v", 'i', WIMM_LANE, 0, false);
            add(p, "extract_lane_u", "v", 'i', WIMM_LANE, 0, false);
        }
    }
}

// The instruction's description, or nil.
pub fn lookup(name: []u8) *mut WasmOpInfo {
    if not built {
        build();
    }
    return ops.get(name);
}
