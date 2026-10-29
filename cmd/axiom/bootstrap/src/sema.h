// sema.h - semantic analysis: module loading, name resolution, type checking.
#ifndef AX_SEMA_H
#define AX_SEMA_H

#include "types.h"

typedef struct Program {
    Vec(Module *) modules;
    Module *root;
    Vec(Sym) error_names;  // code = index + 1
    SymMap error_codes;    // Sym -> (void *)(intptr_t)code
    Vec(Decl *) fns;       // every fn decl (including methods), for codegen
    Vec(Decl *) globals;
    Vec(Decl *) host_fns;
    Vec(Decl *) tests;     // test blocks of non-std modules, in source order
} Program;

extern Program g_prog;

typedef struct SemaOptions {
    const char *std_dir;
    // Extra modules to load after the root (axiom test <package> loads
    // every source file of the package).
    const char *const *extra_files;
    int n_extra_files;
} SemaOptions;

// Load, parse, resolve and type-check the program rooted at `path`.
// Returns false if any errors were reported.
bool sema_run(const char *path, SemaOptions *opts);

uint32_t error_code_of(Sym name);  // 0 if undeclared

// The value of a constant float expression of type f32 (or untyped,
// becoming f32), rounded as f32 literals and operations.
float const_f32(Expr *e);

// Processor extensions (sema_target.c, spec §13.4)
#define SEMA_NO_VERSION (~(uint64_t)0)  // a version naming another architecture's extension
// The value of a target condition in code compiled with extensions `feats`:
// 1 true, 0 false, -1 not a target condition.
int target_cond(Expr *e, uint64_t feats);
bool baseline_fn(Decl *d);           // std/os/cpu.ax: compiled for the bare architecture
uint64_t fn_base_features(Decl *d);  // the extensions its code may use, without an attribute

#endif
