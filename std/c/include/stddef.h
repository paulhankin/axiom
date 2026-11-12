/* stddef.h - common definitions (C89 7.1.6, C99 7.17, C11 7.19, C23 7.21),
   for the Axiom compiler's C front end. */
#ifndef __AXIOM_STDDEF_H
#define __AXIOM_STDDEF_H

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_STDDEF_H__ 202311L
#endif

/* size_t and ptrdiff_t: the platform's types as wide as a pointer (the
   compiler's macros: unsigned long and long, or on Windows unsigned long
   long and long long) */
#ifndef __AXIOM_SIZE_T_DEFINED
#define __AXIOM_SIZE_T_DEFINED
typedef __AXIOM_SIZE_TYPE__ size_t;
#endif
typedef __AXIOM_PTRDIFF_TYPE__ ptrdiff_t;

/* wchar_t: as the literals L"..." are (16 or 32 bits: the narrowest type of
   at least 32 when none has 32) */
#if __AXIOM_WCHAR_WIDTH__ == 16
#if __AXIOM_WCHAR_SIGNED__
typedef short wchar_t;
#else
typedef unsigned short wchar_t;
#endif
#elif __AXIOM_INT_WIDTH__ == 32 || (__AXIOM_LONG_WIDTH__ != 32 && __AXIOM_INT_WIDTH__ >= 32)
#if __AXIOM_WCHAR_SIGNED__
typedef int wchar_t;
#else
typedef unsigned int wchar_t;
#endif
#else
#if __AXIOM_WCHAR_SIGNED__
typedef long wchar_t;
#else
typedef unsigned long wchar_t;
#endif
#endif

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
