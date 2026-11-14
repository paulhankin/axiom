/* limits.h - sizes of integer types (C89 7.1.5, C99 7.10 and 5.2.4.2.1,
   C11 7.10, C23 7.10), for the Axiom compiler's C front end. */
#ifndef __AXIOM_LIMITS_H
#define __AXIOM_LIMITS_H

#ifdef __AXIOM_SYSTEM_C_LIB__
/* With the system's C library (--c-lib=system) its <limits.h>, with its POSIX
   limits, and what C23 adds to it that it leaves to the compiler. */
#include_next <limits.h>
#if __STDC_VERSION__ >= 202311L
#ifndef __STDC_VERSION_LIMITS_H__
#define __STDC_VERSION_LIMITS_H__ 202311L
#endif
#ifndef BOOL_MAX
#define BOOL_MAX 1
#endif
#ifndef BOOL_WIDTH
#define BOOL_WIDTH 1
#endif
#ifndef BITINT_MAXWIDTH
#define BITINT_MAXWIDTH 65535
#endif
#endif
#else

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_LIMITS_H__ 202311L
#endif

#define CHAR_BIT 8
#define SCHAR_MIN (-127 - 1)
#define SCHAR_MAX 127
#define UCHAR_MAX 255
#ifdef __CHAR_UNSIGNED__
#define CHAR_MIN 0
#define CHAR_MAX UCHAR_MAX
#else
#define CHAR_MIN SCHAR_MIN
#define CHAR_MAX SCHAR_MAX
#endif
/* (the characters are UTF-8) */
#define MB_LEN_MAX 4

#define SHRT_MIN (-32767 - 1)
#define SHRT_MAX 32767
#if __AXIOM_INT_WIDTH__ == 16
/* (unsigned short promotes to unsigned int) */
#define USHRT_MAX 65535U
#else
#define USHRT_MAX 65535
#endif

#if __AXIOM_INT_WIDTH__ == 16
#define INT_MIN (-32767 - 1)
#define INT_MAX 32767
#define UINT_MAX 65535U
#elif __AXIOM_INT_WIDTH__ == 32
#define INT_MIN (-2147483647 - 1)
#define INT_MAX 2147483647
#define UINT_MAX 4294967295U
#else
#define INT_MIN (-9223372036854775807 - 1)
#define INT_MAX 9223372036854775807
#define UINT_MAX 18446744073709551615U
#endif

#if __AXIOM_LONG_WIDTH__ == 32
#define LONG_MIN (-2147483647L - 1)
#define LONG_MAX 2147483647L
#define ULONG_MAX 4294967295UL
#else
#define LONG_MIN (-9223372036854775807L - 1)
#define LONG_MAX 9223372036854775807L
#define ULONG_MAX 18446744073709551615UL
#endif

#if __STDC_VERSION__ >= 199901L
#define LLONG_MIN (-9223372036854775807LL - 1)
#define LLONG_MAX 9223372036854775807LL
#define ULLONG_MAX 18446744073709551615ULL
#endif

#if __STDC_VERSION__ >= 202311L
#define BOOL_MAX 1
#define BOOL_WIDTH 1
#define CHAR_WIDTH 8
#define SCHAR_WIDTH 8
#define UCHAR_WIDTH 8
#define SHRT_WIDTH 16
#define USHRT_WIDTH 16
#define INT_WIDTH __AXIOM_INT_WIDTH__
#define UINT_WIDTH __AXIOM_INT_WIDTH__
#define LONG_WIDTH __AXIOM_LONG_WIDTH__
#define ULONG_WIDTH __AXIOM_LONG_WIDTH__
#define LLONG_WIDTH 64
#define ULLONG_WIDTH 64
#define BITINT_MAXWIDTH 65535
#endif

#endif /* __AXIOM_SYSTEM_C_LIB__ */

#endif
