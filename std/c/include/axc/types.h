/* axc/types.h - the types that several of the C library's headers declare
   (size_t, wchar_t, wint_t, mbstate_t), each defined once whichever header
   comes first: a typedef may not be repeated before C11. A header defines
   __AXIOM_WANT_SIZE_T, __AXIOM_WANT_WCHAR_T, __AXIOM_WANT_WINT_T or
   __AXIOM_WANT_MBSTATE_T for each it declares, then includes this one,
   which undefines them. The types are the platform's, from the compiler's
   __AXIOM_*_TYPE__ macros where it has them. Also the library's
   declarations' macros (__AXC_NORETURN, __AXC_RESTRICT). Private to the
   compiler's headers. */

#ifndef __AXIOM_AXC_TYPES_MACROS
#define __AXIOM_AXC_TYPES_MACROS
/* _Noreturn where the standard has it, for the library's declarations */
#if __STDC_VERSION__ >= 201112L
#define __AXC_NORETURN _Noreturn
#else
#define __AXC_NORETURN
#endif
#if __STDC_VERSION__ >= 199901L
#define __AXC_RESTRICT restrict
#else
#define __AXC_RESTRICT
#endif
#endif

/* (wchar_t's type: as the literals L"..." are, 16 or 32 bits, the
   narrowest type of at least 32 when none has 32) */
#if __AXIOM_WCHAR_WIDTH__ == 16
#if __AXIOM_WCHAR_SIGNED__
#define __AXIOM_WCHAR_TYPE short
#else
#define __AXIOM_WCHAR_TYPE unsigned short
#endif
#elif __AXIOM_INT_WIDTH__ == 32 || (__AXIOM_LONG_WIDTH__ != 32 && __AXIOM_INT_WIDTH__ >= 32)
#if __AXIOM_WCHAR_SIGNED__
#define __AXIOM_WCHAR_TYPE int
#else
#define __AXIOM_WCHAR_TYPE unsigned int
#endif
#else
#if __AXIOM_WCHAR_SIGNED__
#define __AXIOM_WCHAR_TYPE long
#else
#define __AXIOM_WCHAR_TYPE unsigned long
#endif
#endif

/* size_t: as <stddef.h> has it */
#if defined(__AXIOM_WANT_SIZE_T) && !defined(__AXIOM_SIZE_T_DEFINED)
#define __AXIOM_SIZE_T_DEFINED
#ifdef __AXIOM_SIZE_TYPE__
typedef __AXIOM_SIZE_TYPE__ size_t;
#elif __AXIOM_INT_WIDTH__ >= __AXIOM_PTR_WIDTH__
typedef unsigned int size_t;
#elif __AXIOM_LONG_WIDTH__ >= __AXIOM_PTR_WIDTH__
typedef unsigned long size_t;
#elif __STDC_VERSION__ >= 199901L
typedef unsigned long long size_t;
#else
typedef unsigned __axiom_int64 size_t;
#endif
#endif

/* wchar_t: as <stddef.h> has it */
#if defined(__AXIOM_WANT_WCHAR_T) && !defined(__AXIOM_WCHAR_T_DEFINED)
#define __AXIOM_WCHAR_T_DEFINED
typedef __AXIOM_WCHAR_TYPE wchar_t;
#endif

/* wint_t (C95 7.24.1): the platform's type, which the compiler names; it
   holds every wchar_t value and WEOF */
#if defined(__AXIOM_WANT_WINT_T) && !defined(__AXIOM_WINT_T_DEFINED)
#define __AXIOM_WINT_T_DEFINED
#ifdef __AXIOM_WINT_TYPE__
typedef __AXIOM_WINT_TYPE__ wint_t;
#else
typedef __AXIOM_WCHAR_TYPE wint_t;
#endif
#endif

/* mbstate_t (C95 7.24.1): all zero bits is the initial conversion state.
   A conversion from multibyte characters keeps the character it has begun
   (its value so far, the bytes it still needs and the range the next one
   must be in, for UTF-8's well-formedness rules); one to UTF-16 or UTF-8
   code units keeps the units it has still to give; one from UTF-16 keeps
   a high surrogate until its low one comes. */
#if defined(__AXIOM_WANT_MBSTATE_T) && !defined(__AXIOM_MBSTATE_T_DEFINED)
#define __AXIOM_MBSTATE_T_DEFINED
typedef struct {
    unsigned long __axiom_value;
    unsigned long __axiom_out;
    unsigned char __axiom_need;
    unsigned char __axiom_lo;
    unsigned char __axiom_hi;
    unsigned char __axiom_npend;
    unsigned char __axiom_high;
} mbstate_t;
#endif

#undef __AXIOM_WCHAR_TYPE
#undef __AXIOM_WANT_SIZE_T
#undef __AXIOM_WANT_WCHAR_T
#undef __AXIOM_WANT_WINT_T
#undef __AXIOM_WANT_MBSTATE_T
