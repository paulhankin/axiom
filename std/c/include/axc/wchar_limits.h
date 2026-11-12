/* axc/wchar_limits.h - WCHAR_MIN and WCHAR_MAX, which <stdint.h> and
   <wchar.h> both define (the same definitions, so that a program may
   include both), and the widths of wchar_t and wint_t. Each limit has the
   type of a wchar_t after the integer promotions (C17 7.20.3p2). Private
   to the compiler's headers. */
#ifndef __AXIOM_AXC_WCHAR_LIMITS_H
#define __AXIOM_AXC_WCHAR_LIMITS_H

#if __AXIOM_WCHAR_WIDTH__ == 16
#if __AXIOM_WCHAR_SIGNED__
#define WCHAR_MIN (-32767 - 1)
#define WCHAR_MAX 32767
#elif __AXIOM_INT_WIDTH__ == 16
#define WCHAR_MIN 0U
#define WCHAR_MAX 65535U
#else
#define WCHAR_MIN 0
#define WCHAR_MAX 65535
#endif
#define __AXIOM_WCHAR_BITS 16
#elif __AXIOM_INT_WIDTH__ == 64
/* (wchar_t is int: there is no 32-bit type) */
#if __AXIOM_WCHAR_SIGNED__
#define WCHAR_MIN (-9223372036854775807 - 1)
#define WCHAR_MAX 9223372036854775807
#else
#define WCHAR_MIN 0U
#define WCHAR_MAX 18446744073709551615U
#endif
#define __AXIOM_WCHAR_BITS 64
#elif __AXIOM_LONG_WIDTH__ == 64 && __AXIOM_INT_WIDTH__ == 16
/* (wchar_t is long: there is no 32-bit type) */
#if __AXIOM_WCHAR_SIGNED__
#define WCHAR_MIN (-9223372036854775807L - 1)
#define WCHAR_MAX 9223372036854775807L
#else
#define WCHAR_MIN 0UL
#define WCHAR_MAX 18446744073709551615UL
#endif
#define __AXIOM_WCHAR_BITS 64
#elif __AXIOM_INT_WIDTH__ == 32
#if __AXIOM_WCHAR_SIGNED__
#define WCHAR_MIN (-2147483647 - 1)
#define WCHAR_MAX 2147483647
#else
#define WCHAR_MIN 0U
#define WCHAR_MAX 4294967295U
#endif
#define __AXIOM_WCHAR_BITS 32
#else
/* (wchar_t is long, of 32 bits) */
#if __AXIOM_WCHAR_SIGNED__
#define WCHAR_MIN (-2147483647L - 1)
#define WCHAR_MAX 2147483647L
#else
#define WCHAR_MIN 0UL
#define WCHAR_MAX 4294967295UL
#endif
#define __AXIOM_WCHAR_BITS 32
#endif

#endif
