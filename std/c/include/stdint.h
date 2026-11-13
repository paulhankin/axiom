/* stdint.h - integer types (C99 7.18, C11 7.20, C23 7.22), for the Axiom
   compiler's C front end. short is 16 bits, int 16, 32 or 64, long 32 or
   64, long long 64; a pointer 32 or 64. */
/* (Annex K: whether its names are asked for, K.3.1.1) */
#include <axc/ext1.h>
#ifndef __AXIOM_STDINT_H
#define __AXIOM_STDINT_H

#if __STDC_VERSION__ < 199901L
#error "<stdint.h> is a header of C99 and later"
#endif

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_STDINT_H__ 202311L
#endif

/* exact widths (there is no 32-bit type when int is 64 bits) */
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
#if __AXIOM_INT_WIDTH__ == 32
#define __AXIOM_HAS_INT32 1
typedef int int32_t;
typedef unsigned int uint32_t;
#define __AXIOM_C32(c) c
#define __AXIOM_UC32(c) c##U
#elif __AXIOM_LONG_WIDTH__ == 32
#define __AXIOM_HAS_INT32 1
typedef long int32_t;
typedef unsigned long uint32_t;
#define __AXIOM_C32(c) c##L
#define __AXIOM_UC32(c) c##UL
#endif
/* (long or long long, as the platform has it: the compiler's macros) */
typedef __AXIOM_INT64_TYPE__ int64_t;
typedef __AXIOM_UINT64_TYPE__ uint64_t;

/* the least and the fastest of at least a width: the exact ones (int for
   32 bits when there is no 32-bit type) */
typedef int8_t int_least8_t;
typedef uint8_t uint_least8_t;
typedef int16_t int_least16_t;
typedef uint16_t uint_least16_t;
typedef int64_t int_least64_t;
typedef uint64_t uint_least64_t;
typedef int8_t int_fast8_t;
typedef uint8_t uint_fast8_t;
typedef int64_t int_fast64_t;
typedef uint64_t uint_fast64_t;
/* (the fast types of 16 and 32 bits as the platform's C library has them) */
typedef __AXIOM_INT_FAST16_TYPE__ int_fast16_t;
typedef __AXIOM_UINT_FAST16_TYPE__ uint_fast16_t;
typedef __AXIOM_INT_FAST32_TYPE__ int_fast32_t;
typedef __AXIOM_UINT_FAST32_TYPE__ uint_fast32_t;
#ifdef __AXIOM_HAS_INT32
typedef int32_t int_least32_t;
typedef uint32_t uint_least32_t;
#define __AXIOM_LEAST32_MIN INT32_MIN
#define __AXIOM_LEAST32_MAX INT32_MAX
#define __AXIOM_ULEAST32_MAX UINT32_MAX
#define __AXIOM_LEAST32_WIDTH 32
#define INT32_C(c) __AXIOM_C32(c)
#define UINT32_C(c) __AXIOM_UC32(c)
#elif __AXIOM_INT_WIDTH__ == 64
typedef int int_least32_t;
typedef unsigned int uint_least32_t;
#define __AXIOM_LEAST32_MIN (-9223372036854775807 - 1)
#define __AXIOM_LEAST32_MAX 9223372036854775807
#define __AXIOM_ULEAST32_MAX 18446744073709551615U
#define __AXIOM_LEAST32_WIDTH 64
#define INT32_C(c) c
#define UINT32_C(c) c##U
#else
/* (int is 16 bits and long 64) */
typedef long int_least32_t;
typedef unsigned long uint_least32_t;
#define __AXIOM_LEAST32_MIN (-9223372036854775807L - 1)
#define __AXIOM_LEAST32_MAX 9223372036854775807L
#define __AXIOM_ULEAST32_MAX 18446744073709551615UL
#define __AXIOM_LEAST32_WIDTH 64
#define INT32_C(c) c##L
#define UINT32_C(c) c##UL
#endif

/* integers that hold pointers: as size_t and ptrdiff_t */
typedef __AXIOM_PTRDIFF_TYPE__ intptr_t;
typedef __AXIOM_SIZE_TYPE__ uintptr_t;

typedef __AXIOM_INTMAX_TYPE__ intmax_t;
typedef __AXIOM_UINTMAX_TYPE__ uintmax_t;

/* limits */
#define INT8_MIN (-127 - 1)
#define INT8_MAX 127
#define UINT8_MAX 255
#define INT16_MIN (-32767 - 1)
#define INT16_MAX 32767
#if __AXIOM_INT_WIDTH__ == 16
#define UINT16_MAX 65535U
#else
#define UINT16_MAX 65535
#endif
#ifdef __AXIOM_HAS_INT32
#define INT32_MIN (-INT32_C(2147483647) - 1)
#define INT32_MAX INT32_C(2147483647)
#define UINT32_MAX UINT32_C(4294967295)
#endif
#define INT64_MIN __AXIOM_INT64_MIN__
#define INT64_MAX __AXIOM_INT64_MAX__
#define UINT64_MAX __AXIOM_UINT64_MAX__

#define INT_LEAST8_MIN INT8_MIN
#define INT_LEAST8_MAX INT8_MAX
#define UINT_LEAST8_MAX UINT8_MAX
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST16_MAX INT16_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define INT_LEAST32_MIN __AXIOM_LEAST32_MIN
#define INT_LEAST32_MAX __AXIOM_LEAST32_MAX
#define UINT_LEAST32_MAX __AXIOM_ULEAST32_MAX
#define INT_LEAST64_MIN INT64_MIN
#define INT_LEAST64_MAX INT64_MAX
#define UINT_LEAST64_MAX UINT64_MAX

#define INT_FAST8_MIN INT8_MIN
#define INT_FAST8_MAX INT8_MAX
#define UINT_FAST8_MAX UINT8_MAX
#define INT_FAST16_MIN __AXIOM_INT_FAST16_MIN__
#define INT_FAST16_MAX __AXIOM_INT_FAST16_MAX__
#define UINT_FAST16_MAX __AXIOM_UINT_FAST16_MAX__
#define INT_FAST32_MIN __AXIOM_INT_FAST32_MIN__
#define INT_FAST32_MAX __AXIOM_INT_FAST32_MAX__
#define UINT_FAST32_MAX __AXIOM_UINT_FAST32_MAX__
#define INT_FAST64_MIN INT64_MIN
#define INT_FAST64_MAX INT64_MAX
#define UINT_FAST64_MAX UINT64_MAX

#define INTPTR_MIN __AXIOM_PTRDIFF_MIN__
#define INTPTR_MAX __AXIOM_PTRDIFF_MAX__
#define UINTPTR_MAX __AXIOM_SIZE_MAX__
#define PTRDIFF_MIN __AXIOM_PTRDIFF_MIN__
#define PTRDIFF_MAX __AXIOM_PTRDIFF_MAX__
#define SIZE_MAX __AXIOM_SIZE_MAX__
#if __AXC_EXT1
/* the largest size the functions of Annex K take (K.3.4): half of SIZE_MAX
   (as K.3.4p4 recommends), so that a negative value converted to rsize_t
   is caught */
#define RSIZE_MAX (__AXIOM_SIZE_MAX__ >> 1)
#endif

#define INTMAX_MIN __AXIOM_INTMAX_MIN__
#define INTMAX_MAX __AXIOM_INTMAX_MAX__
#define UINTMAX_MAX __AXIOM_UINTMAX_MAX__

/* sig_atomic_t (<signal.h>) is int; wint_t (<wchar.h>) is as wchar_t */
#if __AXIOM_INT_WIDTH__ == 16
#define SIG_ATOMIC_MIN (-32767 - 1)
#define SIG_ATOMIC_MAX 32767
#elif __AXIOM_INT_WIDTH__ == 32
#define SIG_ATOMIC_MIN (-2147483647 - 1)
#define SIG_ATOMIC_MAX 2147483647
#else
#define SIG_ATOMIC_MIN (-9223372036854775807 - 1)
#define SIG_ATOMIC_MAX 9223372036854775807
#endif

#include <axc/wchar_limits.h>
/* wint_t (<wchar.h>): as the platform has it */
#define WINT_MIN __AXIOM_WINT_MIN__
#define WINT_MAX __AXIOM_WINT_MAX__

/* constants */
#define INT8_C(c) c
#define UINT8_C(c) c
#define INT16_C(c) c
#if __AXIOM_INT_WIDTH__ == 16
#define UINT16_C(c) c##U
#else
#define UINT16_C(c) c
#endif
#define INT64_C(c) __AXIOM_INT64_C__(c)
#define UINT64_C(c) __AXIOM_UINT64_C__(c)
#define INTMAX_C(c) __AXIOM_INTMAX_C__(c)
#define UINTMAX_C(c) __AXIOM_UINTMAX_C__(c)

#if __STDC_VERSION__ >= 202311L
#define INT8_WIDTH 8
#define UINT8_WIDTH 8
#define INT16_WIDTH 16
#define UINT16_WIDTH 16
#ifdef __AXIOM_HAS_INT32
#define INT32_WIDTH 32
#define UINT32_WIDTH 32
#endif
#define INT64_WIDTH 64
#define UINT64_WIDTH 64
#define INT_LEAST8_WIDTH 8
#define UINT_LEAST8_WIDTH 8
#define INT_LEAST16_WIDTH 16
#define UINT_LEAST16_WIDTH 16
#define INT_LEAST32_WIDTH __AXIOM_LEAST32_WIDTH
#define UINT_LEAST32_WIDTH __AXIOM_LEAST32_WIDTH
#define INT_LEAST64_WIDTH 64
#define UINT_LEAST64_WIDTH 64
#define INT_FAST8_WIDTH 8
#define UINT_FAST8_WIDTH 8
#define INT_FAST16_WIDTH __AXIOM_INT_FAST16_WIDTH__
#define UINT_FAST16_WIDTH __AXIOM_INT_FAST16_WIDTH__
#define INT_FAST32_WIDTH __AXIOM_INT_FAST32_WIDTH__
#define UINT_FAST32_WIDTH __AXIOM_INT_FAST32_WIDTH__
#define INT_FAST64_WIDTH 64
#define UINT_FAST64_WIDTH 64
#define INTPTR_WIDTH __AXIOM_PTRDIFF_WIDTH__
#define UINTPTR_WIDTH __AXIOM_PTRDIFF_WIDTH__
#define INTMAX_WIDTH 64
#define UINTMAX_WIDTH 64
#define PTRDIFF_WIDTH __AXIOM_PTRDIFF_WIDTH__
#define SIZE_WIDTH __AXIOM_PTRDIFF_WIDTH__
#define SIG_ATOMIC_WIDTH __AXIOM_INT_WIDTH__
#define WCHAR_WIDTH __AXIOM_WCHAR_BITS
#define WINT_WIDTH __AXIOM_WINT_WIDTH__
#endif

#endif
