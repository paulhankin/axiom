/* setjmp.h - nonlocal jumps (C89 4.6, C99 7.13, C11 7.13, C23 7.13), for
   the Axiom compiler's C library.

   setjmp is a macro (7.13.1.1p2 allows it) for __axc_setjmp, which the
   compiler knows returns twice: a function that calls it keeps its objects
   in memory, so that after longjmp those not modified since the setjmp
   call have their values (7.13.2.1p3), at every optimization level. Its
   address cannot be taken (C3301). The linkers provide both functions
   (cmd/axiom/src/jstub.ax), which save and restore the registers the
   platform's calling convention keeps across a call; longjmp unwinds
   nothing (on Windows, no handlers of the frames it leaves run). Not
   available on WebAssembly (C3300). Jumping out of Axiom functions that
   C called, or into a function whose variable length arrays have changed
   since setjmp, is undefined. */
#ifndef __AXIOM_SETJMP_H
#define __AXIOM_SETJMP_H

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_SETJMP_H__ 202311L
#endif

#include <axc/types.h>

/* 32 words: the most a platform saves is Windows x64's 256 bytes */
typedef __axiom_int64 jmp_buf[32];

int __axc_setjmp(jmp_buf env);
#define setjmp(env) __axc_setjmp(env)

__AXC_NORETURN void longjmp(jmp_buf env, int val);

#endif
