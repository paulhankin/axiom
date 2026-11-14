/* alloca.h - alloca, a GNU dialect's: alloca(size) is __builtin_alloca(size),
   size bytes of the calling function's frame, until it returns (docs/c.md,
   "Builtins"). Not a header of the standard: under an ISO standard, which
   has no alloca, it declares nothing. The compiler's in every program, with
   the system's C library too (which has no alloca function). */
#ifndef __AXIOM_ALLOCA_H
#define __AXIOM_ALLOCA_H

#ifdef __AXIOM_GNU__
#define __AXIOM_WANT_SIZE_T
#include <axc/types.h>

#undef alloca
#define alloca(size) __builtin_alloca(size)
#endif

#endif
