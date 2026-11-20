/* stddef.h - common definitions (C89 7.1.6, C99 7.17, C11 7.19, C23 7.21),
   for the Axiom compiler's C front end. */
#if defined(__need_ptrdiff_t) || defined(__need_size_t) || defined(__need_wchar_t) || defined(__need_NULL) || defined(__need_wint_t)
#ifdef __need_ptrdiff_t
#undef __need_ptrdiff_t
#ifndef __AXIOM_PTRDIFF_T_DEFINED
#define __AXIOM_PTRDIFF_T_DEFINED
typedef __AXIOM_PTRDIFF_TYPE__ ptrdiff_t;
#endif
#endif
#ifdef __need_size_t
#undef __need_size_t
#define __AXIOM_WANT_SIZE_T
#include <axc/types.h>
#endif
#ifdef __need_wchar_t
#undef __need_wchar_t
#define __AXIOM_WANT_WCHAR_T
#include <axc/types.h>
#endif
#ifdef __need_wint_t
#undef __need_wint_t
#define __AXIOM_WANT_WINT_T
#include <axc/types.h>
#endif
#ifdef __need_NULL
#undef __need_NULL
#ifdef __AXIOM_GNU__
#undef NULL
#endif
#define NULL ((void *)0)
#endif
#else
/* (Annex K: whether its names are asked for, K.3.1.1) */
#include <axc/ext1.h>
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
#ifndef __AXIOM_PTRDIFF_T_DEFINED
#define __AXIOM_PTRDIFF_T_DEFINED
typedef __AXIOM_PTRDIFF_TYPE__ ptrdiff_t;
#endif
#if __AXC_EXT1
/* rsize_t (K.3.3) */
#define __AXIOM_WANT_SIZE_T
#define __AXIOM_WANT_RSIZE_T
#include <axc/types.h>
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

#ifdef __AXIOM_GNU__
/* (a GNU dialect: a definition the program made before is replaced) */
#undef NULL
#undef offsetof
#endif
#define NULL ((void *)0)
#define offsetof(type, member) __builtin_offsetof(type, member)

#endif
#endif
