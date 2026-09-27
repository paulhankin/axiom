// sema.h - semantic analysis: module loading, name resolution, type checking.
#ifndef PH_SEMA_H
#define PH_SEMA_H

#include "types.h"

typedef struct Program {
    Vec(Module *) modules;
    Module *root;
    Vec(Sym) error_names;  // code = index + 1
    SymMap error_codes;    // Sym -> (void *)(intptr_t)code
    Vec(Decl *) fns;       // every fn decl (including methods), for codegen
    Vec(Decl *) globals;
    Vec(Decl *) host_fns;
} Program;

extern Program g_prog;

typedef struct SemaOptions {
    const char *std_dir;
} SemaOptions;

// Load, parse, resolve and type-check the program rooted at `path`.
// Returns false if any errors were reported.
bool sema_run(const char *path, SemaOptions *opts);

uint32_t error_code_of(Sym name);  // 0 if undeclared

#endif
