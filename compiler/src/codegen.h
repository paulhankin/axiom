// codegen.h - LLVM backend.
#ifndef PH_CODEGEN_H
#define PH_CODEGEN_H

#include "sema.h"

typedef enum EmitKind { EMIT_OBJ, EMIT_LLVM, EMIT_ASM } EmitKind;

typedef struct CgOptions {
    int opt_level;           // 0..3
    bool overflow_saturate;  // --overflow=saturate (--release)
    const char *cpu;         // NULL = generic
    EmitKind emit;
    const char *out_path;    // file to write
} CgOptions;

// Generates code for the checked program. Returns false on failure
// (errors reported through diag or printed to stderr).
bool codegen_program(CgOptions *o);

// Names of exported functions (for the linker).
extern SymVec g_export_names;

#endif
