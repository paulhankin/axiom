/* __axc_exit_handlers: what exit does before the process ends (exit.c):
   the atexit functions, last registered first; the destructors (a GNU
   dialect's, __axc_at_exit_dtors); the streams flushed and closed (the
   standard I/O's hook, __axc_at_exit_flush). Each once. A program whose
   main is an Axiom module's calls it when that main returns, so that its
   C code's handlers run and its output is written as a C program's would
   be (codegen: cgen.ax, wrap_axiom_main). */
#include "axc/core_impl.h"

void __axc_exit_handlers(void) {
    void (*flush)(void);
    /* (with threads: one thread runs the rest; std/c/rt/core.ax) */
    __axc_exit_begin();
    __axc_fn_run(&__axc_atexit_tab);
    /* the destructors (a GNU dialect's), as after main's return */
    flush = __axc_at_exit_dtors;
    if (flush) {
        __axc_at_exit_dtors = (void (*)(void))0;
        flush();
    }
    flush = __axc_at_exit_flush;
    if (flush) {
        __axc_at_exit_flush = (void (*)(void))0; /* (once, if a handler of it exits) */
        flush();
    }
}
