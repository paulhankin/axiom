// wasmops.h - the WebAssembly instructions accepted inside `wasm { }` blocks.
#ifndef PH_WASMOPS_H
#define PH_WASMOPS_H

#include "util.h"

// Value-type letters: i = i32, l = i64, f = f32, d = f64, v = v128,
// p = address (i32 on 32-bit targets, i64 on 64-bit), * = any (polymorphic).
typedef enum WasmImm { WIMM_NONE, WIMM_LOCAL, WIMM_I32, WIMM_I64, WIMM_F32, WIMM_F64, WIMM_MEM, WIMM_LANE } WasmImm;

typedef struct WasmOpInfo {
    Sym name;
    const char *prefix;  // "i32", "f32x4", "local", ... (text before the first '.')
    const char *op;      // text after the first '.', or the whole name
    char in[4];          // operand types (bottom of stack first)
    char out;            // result type or 0
    WasmImm imm;
    int mem_bytes;       // loads/stores: access width
    bool mem_signed;     // loads: sign-extend
} WasmOpInfo;

const WasmOpInfo *wasm_op_lookup(Sym name);

#endif
