/* stddef.h - common definitions (C89 7.1.6, C99 7.17, C11 7.19, C23 7.21),
   for the Axiom compiler's C front end. */
#ifndef __AXIOM_STDDEF_H
#define __AXIOM_STDDEF_H

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_STDDEF_H__ 202311L
#endif

/* size_t and ptrdiff_t: the first of int, long and long long as wide as a
   pointer (the type of sizeof and of a pointer difference) */
#if __AXIOM_INT_WIDTH__ >= __AXIOM_PTR_WIDTH__
typedef unsigned int size_t;
typedef int ptrdiff_t;
#elif __AXIOM_LONG_WIDTH__ >= __AXIOM_PTR_WIDTH__
typedef unsigned long size_t;
typedef long ptrdiff_t;
#elif __STDC_VERSION__ >= 199901L
typedef unsigned long long size_t;
typedef long long ptrdiff_t;
#else
typedef unsigned __axiom_int64 size_t;
typedef __axiom_int64 ptrdiff_t;
#endif

/* wchar_t: as the literals L"..." are (16 or 32 bits) */
#if __AXIOM_WCHAR_WIDTH__ == 16
#if __AXIOM_WCHAR_SIGNED__
typedef short wchar_t;
#else
typedef unsigned short wchar_t;
#endif
#elif __AXIOM_INT_WIDTH__ == 32 || __AXIOM_LONG_WIDTH__ != 32
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
typedef struct {
    long long __axiom_ll;
    long double __axiom_ld;
} max_align_t;
#endif

#if __STDC_VERSION__ >= 202311L
typedef typeof(nullptr) nullptr_t;
/* (reaching it is undefined: nothing need be done) */
#define unreachable() ((void)0)
#endif

#define NULL ((void *)0)
#define offsetof(type, member) __builtin_offsetof(type, member)

#endif
