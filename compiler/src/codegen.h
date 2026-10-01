// codegen.h - LLVM backend.
#ifndef AX_CODEGEN_H
#define AX_CODEGEN_H

#include "sema.h"

typedef enum EmitKind { EMIT_OBJ, EMIT_LLVM, EMIT_ASM } EmitKind;

// What one codegen run produces (spec §16.8):
//   CG_WHOLE   the whole program in one object (-O2 and above by default)
//   CG_MODULE  one module's non-generic functions, plus the generic instances
//              it uses (link-once); everything else is referenced by name
//   CG_LINK    the per-program pieces: globals, error codes and names,
//              runtime state, test and host exports
typedef enum CgUnit { CG_WHOLE, CG_MODULE, CG_LINK } CgUnit;

typedef struct CgOptions {
    int opt_level;           // 0..3
    bool overflow_saturate;  // --overflow=saturate (--release)
    const char *cpu;         // NULL = generic
    EmitKind emit;
    const char *out_path;    // file to write
    bool stack_traces;       // shadow stack for traces and profiles (--stack-traces=on, default)
    bool profile;            // axiom profile: the shadow stack is sampled concurrently
    bool sanitize_address;   // --sanitize=address: heap accesses are checked (spec §16.8)
    CgUnit unit;
    Module *module;          // CG_MODULE
} CgOptions;

// Stable identity of a source file in site records (hash of its display path).
uint32_t file_key(int file_id);

// Exported symbol names of the whole program (for the linker), and libraries
// named by extern "C" blocks: computed from the checked program.
void collect_link_names(const CgOptions *o);

// Generates code for the checked program. Returns false on failure
// (errors reported through diag or printed to stderr).
bool codegen_program(CgOptions *o);

// Names of exported functions (for the linker).
extern SymVec g_export_names;
// Libraries named by extern "C" "lib" blocks (for the linker).
extern SymVec g_link_libs;

// Set before codegen: the program spawns threads (wasm: shared memory and a
// thread entry export).
extern bool g_threads;
// Build a test binary (axiom test): tests become roots and are exported.
extern bool g_test_mode;
// Set by codegen: the output calls into runtime/native_host.c.
extern bool g_needs_native_runtime;

#endif
