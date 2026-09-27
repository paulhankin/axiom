// wasmops.c - table of supported inline-wasm instructions, built on first use.
#include "wasmops.h"

static SymMap ops;
static bool built;

static void add(const char *prefix, const char *op, const char *in, char out, WasmImm imm, int mem_bytes, bool msigned) {
    char buf[64];
    if (prefix[0]) snprintf(buf, sizeof buf, "%s.%s", prefix, op);
    else snprintf(buf, sizeof buf, "%s", op);
    WasmOpInfo *w = NEW(WasmOpInfo);
    w->name = intern_cstr(buf);
    w->prefix = prefix;
    w->op = op;
    snprintf(w->in, sizeof w->in, "%s", in);
    w->out = out;
    w->imm = imm;
    w->mem_bytes = mem_bytes;
    w->mem_signed = msigned;
    symmap_put(&ops, w->name, w);
}

static void build(void) {
    built = true;
    add("", "nop", "", 0, WIMM_NONE, 0, false);
    add("", "unreachable", "", 0, WIMM_NONE, 0, false);
    add("", "drop", "*", 0, WIMM_NONE, 0, false);
    add("", "select", "**i", '*', WIMM_NONE, 0, false);
    add("local", "get", "", '*', WIMM_LOCAL, 0, false);
    add("local", "set", "*", 0, WIMM_LOCAL, 0, false);
    add("local", "tee", "*", '*', WIMM_LOCAL, 0, false);

    static const char *ibin[] = {"add", "sub", "mul", "div_s", "div_u", "rem_s", "rem_u", "and",
                                 "or",  "xor", "shl", "shr_s", "shr_u", "rotl",  "rotr"};
    static const char *icmp[] = {"eq", "ne", "lt_s", "lt_u", "gt_s", "gt_u", "le_s", "le_u", "ge_s", "ge_u"};
    static const char *fbin[] = {"add", "sub", "mul", "div", "min", "max", "copysign"};
    static const char *fun[] = {"abs", "neg", "sqrt", "ceil", "floor", "trunc", "nearest"};
    static const char *fcmp[] = {"eq", "ne", "lt", "gt", "le", "ge"};
    for (int w = 0; w < 2; w++) {
        const char *p = w ? "i64" : "i32";
        char t = w ? 'l' : 'i';
        char tt[3] = {t, t, 0}, t1[2] = {t, 0};
        for (size_t i = 0; i < sizeof ibin / sizeof *ibin; i++) add(p, ibin[i], tt, t, WIMM_NONE, 0, false);
        for (size_t i = 0; i < sizeof icmp / sizeof *icmp; i++) add(p, icmp[i], tt, 'i', WIMM_NONE, 0, false);
        add(p, "clz", t1, t, WIMM_NONE, 0, false);
        add(p, "ctz", t1, t, WIMM_NONE, 0, false);
        add(p, "popcnt", t1, t, WIMM_NONE, 0, false);
        add(p, "eqz", t1, 'i', WIMM_NONE, 0, false);
        add(p, "const", "", t, w ? WIMM_I64 : WIMM_I32, 0, false);
        add(p, "load", "p", t, WIMM_MEM, w ? 8 : 4, false);
        add(p, "store", w ? "pl" : "pi", 0, WIMM_MEM, w ? 8 : 4, false);
        add(p, "load8_s", "p", t, WIMM_MEM, 1, true);
        add(p, "load8_u", "p", t, WIMM_MEM, 1, false);
        add(p, "load16_s", "p", t, WIMM_MEM, 2, true);
        add(p, "load16_u", "p", t, WIMM_MEM, 2, false);
        add(p, "store8", w ? "pl" : "pi", 0, WIMM_MEM, 1, false);
        add(p, "store16", w ? "pl" : "pi", 0, WIMM_MEM, 2, false);
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
    for (int w = 0; w < 2; w++) {
        const char *p = w ? "f64" : "f32";
        char t = w ? 'd' : 'f';
        char tt[3] = {t, t, 0}, t1[2] = {t, 0};
        for (size_t i = 0; i < sizeof fbin / sizeof *fbin; i++) add(p, fbin[i], tt, t, WIMM_NONE, 0, false);
        for (size_t i = 0; i < sizeof fun / sizeof *fun; i++) add(p, fun[i], t1, t, WIMM_NONE, 0, false);
        for (size_t i = 0; i < sizeof fcmp / sizeof *fcmp; i++) add(p, fcmp[i], tt, 'i', WIMM_NONE, 0, false);
        add(p, "const", "", t, w ? WIMM_F64 : WIMM_F32, 0, false);
        add(p, "load", "p", t, WIMM_MEM, w ? 8 : 4, false);
        add(p, "store", w ? "pd" : "pf", 0, WIMM_MEM, w ? 8 : 4, false);
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
    static const struct {
        const char *shape;
        char scalar;
        bool is_float;
        bool has_mul;
    } shapes[] = {{"i8x16", 'i', false, false}, {"i16x8", 'i', false, true}, {"i32x4", 'i', false, true},
                  {"i64x2", 'l', false, true},  {"f32x4", 'f', true, true},   {"f64x2", 'd', true, true}};
    for (size_t s = 0; s < sizeof shapes / sizeof *shapes; s++) {
        const char *p = shapes[s].shape;
        char sc[2] = {shapes[s].scalar, 0};
        char vs[3] = {'v', shapes[s].scalar, 0};
        add(p, "add", "vv", 'v', WIMM_NONE, 0, false);
        add(p, "sub", "vv", 'v', WIMM_NONE, 0, false);
        if (shapes[s].has_mul) add(p, "mul", "vv", 'v', WIMM_NONE, 0, false);
        add(p, "neg", "v", 'v', WIMM_NONE, 0, false);
        add(p, "splat", sc, 'v', WIMM_NONE, 0, false);
        add(p, "replace_lane", vs, 'v', WIMM_LANE, 0, false);
        if (shapes[s].is_float) {
            add(p, "div", "vv", 'v', WIMM_NONE, 0, false);
            add(p, "sqrt", "v", 'v', WIMM_NONE, 0, false);
            add(p, "min", "vv", 'v', WIMM_NONE, 0, false);
            add(p, "max", "vv", 'v', WIMM_NONE, 0, false);
            add(p, "abs", "v", 'v', WIMM_NONE, 0, false);
            add(p, "extract_lane", "v", shapes[s].scalar, WIMM_LANE, 0, false);
        } else if (shapes[s].scalar == 'l' || !strcmp(p, "i32x4")) {
            add(p, "extract_lane", "v", shapes[s].scalar, WIMM_LANE, 0, false);
        } else {
            add(p, "extract_lane_s", "v", 'i', WIMM_LANE, 0, false);
            add(p, "extract_lane_u", "v", 'i', WIMM_LANE, 0, false);
        }
    }
}

const WasmOpInfo *wasm_op_lookup(Sym name) {
    if (!built) build();
    return symmap_get(&ops, name);
}
