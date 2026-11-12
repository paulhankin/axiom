/* stddef.h - common definitions (C89 7.1.6, C99 7.17, C11 7.19, C23 7.21),
   for the Axiom compiler's C front end. */
#ifndef __AXIOM_STDDEF_H
#define __AXIOM_STDDEF_H

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_STDDEF_H__ 202311L
#endif

/* size_t and wchar_t (defined once with the other headers: axc/types.h),
   ptrdiff_t: the platform's types (the compiler's macros) */
#define __AXIOM_WANT_SIZE_T
#define __AXIOM_WANT_WCHAR_T
#include <axc/types.h>
typedef __AXIOM_PTRDIFF_TYPE__ ptrdiff_t;

#if __STDC_VERSION__ >= 201112L
/* the platforms' choices: aligned to 16 bytes, or to 8 (Windows, macOS) */
#if __AXIOM_MAX_ALIGN__ == 16
typedef struct {
    long long __axiom_ll;
    _Alignas(16) long double __axiom_ld;
} max_align_t;
#else
typedef long double max_align_t;
#endif
#endif

#if __STDC_VERSION__ >= 202311L
typedef typeof(nullptr) nullptr_t;
/* (reaching it is undefined: the code after it is not generated) */
#define unreachable() __builtin_unreachable()
#endif

#define NULL ((void *)0)
#define offsetof(type, member) __builtin_offsetof(type, member)

#endif
