/* hdr_checks.h - checks of <stdint.h>, <limits.h>, <stddef.h>, <stdarg.h>
   and <stdbool.h> (in the standards that have them) against the types, for
   the c/run/hdr_*.c tests, which include it under several --c-int,
   --c-long and --c-std choices: each relation is a constant expression
   that must be true, checked as an array size (a negative one is an error). */
#ifndef HDR_CHECKS_H
#define HDR_CHECKS_H
#include <limits.h>
#include <stddef.h>
#include <stdarg.h>
#define HC_CAT2(a, b) a##b
#define HC_CAT(a, b) HC_CAT2(a, b)
#define CHECK(c) typedef char HC_CAT(hc_check_, __LINE__)[(c) ? 1 : -1]
/* t is signed; x has the type of t after the integer promotions */
#define SIGNED(t) ((t)-1 < 0)
#define PROMOTED(x, t) (sizeof(x) == sizeof(+(t)0) && (((x) * 0 - 1 < 0) == ((+(t)0) - 1 < 0)))
#define UMAX(t) ((t)-1)
/* the largest value of signed type t of n bits (no padding bits) */
#define SMAX(t, n) ((t)((((unsigned long long)1 << ((n) - 2)) - 1) * 2 + 1))

CHECK(CHAR_BIT == 8);
CHECK(SCHAR_MAX == 127 && SCHAR_MIN == -128 && UCHAR_MAX == 255);
CHECK(PROMOTED(SCHAR_MAX, signed char) && PROMOTED(UCHAR_MAX, unsigned char) && PROMOTED(CHAR_MAX, char));
CHECK(SIGNED(char) ? (CHAR_MIN == SCHAR_MIN && CHAR_MAX == SCHAR_MAX) : (CHAR_MIN == 0 && CHAR_MAX == UCHAR_MAX));
CHECK(sizeof(short) * CHAR_BIT == 16 && SHRT_MAX == 32767 && SHRT_MIN == -32767 - 1 && USHRT_MAX == UMAX(unsigned short));
CHECK(PROMOTED(SHRT_MAX, short) && PROMOTED(USHRT_MAX, unsigned short));
CHECK(sizeof(int) * CHAR_BIT == __AXIOM_INT_WIDTH__ && UINT_MAX == UMAX(unsigned) && INT_MIN == -INT_MAX - 1);
CHECK(INT_MAX == (int)(UINT_MAX / 2) && PROMOTED(INT_MAX, int) && PROMOTED(UINT_MAX, unsigned));
CHECK(sizeof(long) * CHAR_BIT == __AXIOM_LONG_WIDTH__ && ULONG_MAX == UMAX(unsigned long) && LONG_MIN == -LONG_MAX - 1);
CHECK(LONG_MAX == (long)(ULONG_MAX / 2) && PROMOTED(LONG_MAX, long) && PROMOTED(ULONG_MAX, unsigned long));
CHECK(sizeof(LONG_MIN) == sizeof(long) && sizeof(INT_MIN) == sizeof(int));
CHECK(MB_LEN_MAX >= 1);

/* <stddef.h> */
CHECK(sizeof(size_t) == sizeof(sizeof(int)) && !SIGNED(size_t) && SIGNED(ptrdiff_t));
CHECK(sizeof(ptrdiff_t) == sizeof(size_t) && sizeof(size_t) >= sizeof(void *));
CHECK(sizeof(wchar_t) * CHAR_BIT == __AXIOM_WCHAR_WIDTH__ || (__AXIOM_WCHAR_WIDTH__ == 32 && sizeof(wchar_t) == 8));
CHECK(sizeof(L'a') == sizeof(wchar_t) || sizeof(wchar_t) < sizeof(int));
struct hc_s { char c; int i[3]; struct { short s; double d; } in[2]; };
CHECK(offsetof(struct hc_s, c) == 0 && offsetof(struct hc_s, i[2]) == offsetof(struct hc_s, i) + 2 * sizeof(int));
CHECK(offsetof(struct hc_s, in[1].d) == offsetof(struct hc_s, in) + sizeof(((struct hc_s *)0)->in[0]) + offsetof(struct hc_s, in[0].d) - offsetof(struct hc_s, in));
CHECK(sizeof(offsetof(struct hc_s, i)) == sizeof(size_t));
static int hc_null(void) { int *p = NULL; return p == 0 && sizeof(NULL) == sizeof(void *); }
#if __STDC_VERSION__ >= 201112L
CHECK(_Alignof(max_align_t) >= _Alignof(long long) && _Alignof(max_align_t) >= _Alignof(long double));
CHECK(_Alignof(max_align_t) >= _Alignof(double) && _Alignof(max_align_t) >= _Alignof(void *));
#endif
#if __STDC_VERSION__ >= 202311L
CHECK(sizeof(nullptr_t) == sizeof(void *));
CHECK(BOOL_MAX == 1 && BOOL_WIDTH == 1 && CHAR_WIDTH == 8 && SCHAR_WIDTH == 8 && UCHAR_WIDTH == 8);
CHECK(SHRT_WIDTH == 16 && USHRT_WIDTH == 16 && INT_WIDTH == sizeof(int) * CHAR_BIT && UINT_WIDTH == INT_WIDTH);
CHECK(LONG_WIDTH == sizeof(long) * CHAR_BIT && ULONG_WIDTH == LONG_WIDTH && LLONG_WIDTH == 64 && ULLONG_WIDTH == 64);
CHECK(BITINT_MAXWIDTH >= ULLONG_WIDTH);
#endif
#if __STDC_VERSION__ >= 199901L
CHECK(sizeof(long long) * CHAR_BIT == 64 && ULLONG_MAX == UMAX(unsigned long long) && LLONG_MIN == -LLONG_MAX - 1);
CHECK(PROMOTED(LLONG_MAX, long long) && PROMOTED(ULLONG_MAX, unsigned long long));
#endif

/* <stdarg.h> */
#ifndef va_start
#error "va_start"
#endif
#ifndef va_arg
#error "va_arg"
#endif
#ifndef va_end
#error "va_end"
#endif
#if __STDC_VERSION__ >= 199901L && !defined(va_copy)
#error "va_copy"
#endif
static int hc_va(int n, ...) {
    va_list ap;
    int r;
    va_start(ap, n);
    r = va_arg(ap, int);
    va_end(ap);
    return r;
}

#if __STDC_VERSION__ >= 199901L
#include <stdint.h>
#include <stdbool.h>
CHECK(__bool_true_false_are_defined == 1 && true == 1 && false == 0 && sizeof(bool) == 1);

CHECK(SIGNED(int8_t) && INT8_MAX == SMAX(int8_t, sizeof(int8_t) * CHAR_BIT) && INT8_MIN == -INT8_MAX - 1);
CHECK(PROMOTED(INT8_MAX, int8_t) && PROMOTED(INT8_MIN, int8_t));
CHECK(sizeof(int8_t) * CHAR_BIT == 8);
#if __STDC_VERSION__ >= 202311L
CHECK(INT8_WIDTH == sizeof(int8_t) * CHAR_BIT);
#endif
CHECK(!SIGNED(uint8_t) && UINT8_MAX == UMAX(uint8_t) && PROMOTED(UINT8_MAX, uint8_t));
CHECK(sizeof(uint8_t) * CHAR_BIT == 8);
#if __STDC_VERSION__ >= 202311L
CHECK(UINT8_WIDTH == sizeof(uint8_t) * CHAR_BIT);
#endif
CHECK(SIGNED(int_least8_t) && INT_LEAST8_MAX == SMAX(int_least8_t, sizeof(int_least8_t) * CHAR_BIT) && INT_LEAST8_MIN == -INT_LEAST8_MAX - 1);
CHECK(PROMOTED(INT_LEAST8_MAX, int_least8_t) && PROMOTED(INT_LEAST8_MIN, int_least8_t));
CHECK(sizeof(int_least8_t) * CHAR_BIT >= 8);
CHECK(PROMOTED(INT8_C(1), int_least8_t) && INT8_C(7) == 7);
#if __STDC_VERSION__ >= 202311L
CHECK(INT_LEAST8_WIDTH == sizeof(int_least8_t) * CHAR_BIT);
#endif
CHECK(!SIGNED(uint_least8_t) && UINT_LEAST8_MAX == UMAX(uint_least8_t) && PROMOTED(UINT_LEAST8_MAX, uint_least8_t));
CHECK(sizeof(uint_least8_t) * CHAR_BIT >= 8);
CHECK(PROMOTED(UINT8_C(1), uint_least8_t) && UINT8_C(8) == 8);
#if __STDC_VERSION__ >= 202311L
CHECK(UINT_LEAST8_WIDTH == sizeof(uint_least8_t) * CHAR_BIT);
#endif
CHECK(SIGNED(int_fast8_t) && INT_FAST8_MAX == SMAX(int_fast8_t, sizeof(int_fast8_t) * CHAR_BIT) && INT_FAST8_MIN == -INT_FAST8_MAX - 1);
CHECK(PROMOTED(INT_FAST8_MAX, int_fast8_t) && PROMOTED(INT_FAST8_MIN, int_fast8_t));
CHECK(sizeof(int_fast8_t) * CHAR_BIT >= 8);
#if __STDC_VERSION__ >= 202311L
CHECK(INT_FAST8_WIDTH == sizeof(int_fast8_t) * CHAR_BIT);
#endif
CHECK(!SIGNED(uint_fast8_t) && UINT_FAST8_MAX == UMAX(uint_fast8_t) && PROMOTED(UINT_FAST8_MAX, uint_fast8_t));
CHECK(sizeof(uint_fast8_t) * CHAR_BIT >= 8);
#if __STDC_VERSION__ >= 202311L
CHECK(UINT_FAST8_WIDTH == sizeof(uint_fast8_t) * CHAR_BIT);
#endif
CHECK(SIGNED(int16_t) && INT16_MAX == SMAX(int16_t, sizeof(int16_t) * CHAR_BIT) && INT16_MIN == -INT16_MAX - 1);
CHECK(PROMOTED(INT16_MAX, int16_t) && PROMOTED(INT16_MIN, int16_t));
CHECK(sizeof(int16_t) * CHAR_BIT == 16);
#if __STDC_VERSION__ >= 202311L
CHECK(INT16_WIDTH == sizeof(int16_t) * CHAR_BIT);
#endif
CHECK(!SIGNED(uint16_t) && UINT16_MAX == UMAX(uint16_t) && PROMOTED(UINT16_MAX, uint16_t));
CHECK(sizeof(uint16_t) * CHAR_BIT == 16);
#if __STDC_VERSION__ >= 202311L
CHECK(UINT16_WIDTH == sizeof(uint16_t) * CHAR_BIT);
#endif
CHECK(SIGNED(int_least16_t) && INT_LEAST16_MAX == SMAX(int_least16_t, sizeof(int_least16_t) * CHAR_BIT) && INT_LEAST16_MIN == -INT_LEAST16_MAX - 1);
CHECK(PROMOTED(INT_LEAST16_MAX, int_least16_t) && PROMOTED(INT_LEAST16_MIN, int_least16_t));
CHECK(sizeof(int_least16_t) * CHAR_BIT >= 16);
CHECK(PROMOTED(INT16_C(1), int_least16_t) && INT16_C(15) == 15);
#if __STDC_VERSION__ >= 202311L
CHECK(INT_LEAST16_WIDTH == sizeof(int_least16_t) * CHAR_BIT);
#endif
CHECK(!SIGNED(uint_least16_t) && UINT_LEAST16_MAX == UMAX(uint_least16_t) && PROMOTED(UINT_LEAST16_MAX, uint_least16_t));
CHECK(sizeof(uint_least16_t) * CHAR_BIT >= 16);
CHECK(PROMOTED(UINT16_C(1), uint_least16_t) && UINT16_C(16) == 16);
#if __STDC_VERSION__ >= 202311L
CHECK(UINT_LEAST16_WIDTH == sizeof(uint_least16_t) * CHAR_BIT);
#endif
CHECK(SIGNED(int_fast16_t) && INT_FAST16_MAX == SMAX(int_fast16_t, sizeof(int_fast16_t) * CHAR_BIT) && INT_FAST16_MIN == -INT_FAST16_MAX - 1);
CHECK(PROMOTED(INT_FAST16_MAX, int_fast16_t) && PROMOTED(INT_FAST16_MIN, int_fast16_t));
CHECK(sizeof(int_fast16_t) * CHAR_BIT >= 16);
#if __STDC_VERSION__ >= 202311L
CHECK(INT_FAST16_WIDTH == sizeof(int_fast16_t) * CHAR_BIT);
#endif
CHECK(!SIGNED(uint_fast16_t) && UINT_FAST16_MAX == UMAX(uint_fast16_t) && PROMOTED(UINT_FAST16_MAX, uint_fast16_t));
CHECK(sizeof(uint_fast16_t) * CHAR_BIT >= 16);
#if __STDC_VERSION__ >= 202311L
CHECK(UINT_FAST16_WIDTH == sizeof(uint_fast16_t) * CHAR_BIT);
#endif
#ifdef INT32_MAX
CHECK(SIGNED(int32_t) && INT32_MAX == SMAX(int32_t, sizeof(int32_t) * CHAR_BIT) && INT32_MIN == -INT32_MAX - 1);
CHECK(PROMOTED(INT32_MAX, int32_t) && PROMOTED(INT32_MIN, int32_t));
CHECK(sizeof(int32_t) * CHAR_BIT == 32);
#if __STDC_VERSION__ >= 202311L
CHECK(INT32_WIDTH == sizeof(int32_t) * CHAR_BIT);
#endif
CHECK(!SIGNED(uint32_t) && UINT32_MAX == UMAX(uint32_t) && PROMOTED(UINT32_MAX, uint32_t));
CHECK(sizeof(uint32_t) * CHAR_BIT == 32);
#if __STDC_VERSION__ >= 202311L
CHECK(UINT32_WIDTH == sizeof(uint32_t) * CHAR_BIT);
#endif
#endif
CHECK(SIGNED(int_least32_t) && INT_LEAST32_MAX == SMAX(int_least32_t, sizeof(int_least32_t) * CHAR_BIT) && INT_LEAST32_MIN == -INT_LEAST32_MAX - 1);
CHECK(PROMOTED(INT_LEAST32_MAX, int_least32_t) && PROMOTED(INT_LEAST32_MIN, int_least32_t));
CHECK(sizeof(int_least32_t) * CHAR_BIT >= 32);
CHECK(PROMOTED(INT32_C(1), int_least32_t) && INT32_C(31) == 31);
#if __STDC_VERSION__ >= 202311L
CHECK(INT_LEAST32_WIDTH == sizeof(int_least32_t) * CHAR_BIT);
#endif
CHECK(!SIGNED(uint_least32_t) && UINT_LEAST32_MAX == UMAX(uint_least32_t) && PROMOTED(UINT_LEAST32_MAX, uint_least32_t));
CHECK(sizeof(uint_least32_t) * CHAR_BIT >= 32);
CHECK(PROMOTED(UINT32_C(1), uint_least32_t) && UINT32_C(32) == 32);
#if __STDC_VERSION__ >= 202311L
CHECK(UINT_LEAST32_WIDTH == sizeof(uint_least32_t) * CHAR_BIT);
#endif
CHECK(SIGNED(int_fast32_t) && INT_FAST32_MAX == SMAX(int_fast32_t, sizeof(int_fast32_t) * CHAR_BIT) && INT_FAST32_MIN == -INT_FAST32_MAX - 1);
CHECK(PROMOTED(INT_FAST32_MAX, int_fast32_t) && PROMOTED(INT_FAST32_MIN, int_fast32_t));
CHECK(sizeof(int_fast32_t) * CHAR_BIT >= 32);
#if __STDC_VERSION__ >= 202311L
CHECK(INT_FAST32_WIDTH == sizeof(int_fast32_t) * CHAR_BIT);
#endif
CHECK(!SIGNED(uint_fast32_t) && UINT_FAST32_MAX == UMAX(uint_fast32_t) && PROMOTED(UINT_FAST32_MAX, uint_fast32_t));
CHECK(sizeof(uint_fast32_t) * CHAR_BIT >= 32);
#if __STDC_VERSION__ >= 202311L
CHECK(UINT_FAST32_WIDTH == sizeof(uint_fast32_t) * CHAR_BIT);
#endif
CHECK(SIGNED(int64_t) && INT64_MAX == SMAX(int64_t, sizeof(int64_t) * CHAR_BIT) && INT64_MIN == -INT64_MAX - 1);
CHECK(PROMOTED(INT64_MAX, int64_t) && PROMOTED(INT64_MIN, int64_t));
CHECK(sizeof(int64_t) * CHAR_BIT == 64);
#if __STDC_VERSION__ >= 202311L
CHECK(INT64_WIDTH == sizeof(int64_t) * CHAR_BIT);
#endif
CHECK(!SIGNED(uint64_t) && UINT64_MAX == UMAX(uint64_t) && PROMOTED(UINT64_MAX, uint64_t));
CHECK(sizeof(uint64_t) * CHAR_BIT == 64);
#if __STDC_VERSION__ >= 202311L
CHECK(UINT64_WIDTH == sizeof(uint64_t) * CHAR_BIT);
#endif
CHECK(SIGNED(int_least64_t) && INT_LEAST64_MAX == SMAX(int_least64_t, sizeof(int_least64_t) * CHAR_BIT) && INT_LEAST64_MIN == -INT_LEAST64_MAX - 1);
CHECK(PROMOTED(INT_LEAST64_MAX, int_least64_t) && PROMOTED(INT_LEAST64_MIN, int_least64_t));
CHECK(sizeof(int_least64_t) * CHAR_BIT >= 64);
CHECK(PROMOTED(INT64_C(1), int_least64_t) && INT64_C(63) == 63);
#if __STDC_VERSION__ >= 202311L
CHECK(INT_LEAST64_WIDTH == sizeof(int_least64_t) * CHAR_BIT);
#endif
CHECK(!SIGNED(uint_least64_t) && UINT_LEAST64_MAX == UMAX(uint_least64_t) && PROMOTED(UINT_LEAST64_MAX, uint_least64_t));
CHECK(sizeof(uint_least64_t) * CHAR_BIT >= 64);
CHECK(PROMOTED(UINT64_C(1), uint_least64_t) && UINT64_C(64) == 64);
#if __STDC_VERSION__ >= 202311L
CHECK(UINT_LEAST64_WIDTH == sizeof(uint_least64_t) * CHAR_BIT);
#endif
CHECK(SIGNED(int_fast64_t) && INT_FAST64_MAX == SMAX(int_fast64_t, sizeof(int_fast64_t) * CHAR_BIT) && INT_FAST64_MIN == -INT_FAST64_MAX - 1);
CHECK(PROMOTED(INT_FAST64_MAX, int_fast64_t) && PROMOTED(INT_FAST64_MIN, int_fast64_t));
CHECK(sizeof(int_fast64_t) * CHAR_BIT >= 64);
#if __STDC_VERSION__ >= 202311L
CHECK(INT_FAST64_WIDTH == sizeof(int_fast64_t) * CHAR_BIT);
#endif
CHECK(!SIGNED(uint_fast64_t) && UINT_FAST64_MAX == UMAX(uint_fast64_t) && PROMOTED(UINT_FAST64_MAX, uint_fast64_t));
CHECK(sizeof(uint_fast64_t) * CHAR_BIT >= 64);
#if __STDC_VERSION__ >= 202311L
CHECK(UINT_FAST64_WIDTH == sizeof(uint_fast64_t) * CHAR_BIT);
#endif
CHECK(SIGNED(intptr_t) && INTPTR_MAX == SMAX(intptr_t, sizeof(intptr_t) * CHAR_BIT) && INTPTR_MIN == -INTPTR_MAX - 1);
CHECK(PROMOTED(INTPTR_MAX, intptr_t) && PROMOTED(INTPTR_MIN, intptr_t));
CHECK(sizeof(intptr_t) * CHAR_BIT >= 16);
#if __STDC_VERSION__ >= 202311L
CHECK(INTPTR_WIDTH == sizeof(intptr_t) * CHAR_BIT);
#endif
CHECK(!SIGNED(uintptr_t) && UINTPTR_MAX == UMAX(uintptr_t) && PROMOTED(UINTPTR_MAX, uintptr_t));
CHECK(sizeof(uintptr_t) * CHAR_BIT >= 16);
#if __STDC_VERSION__ >= 202311L
CHECK(UINTPTR_WIDTH == sizeof(uintptr_t) * CHAR_BIT);
#endif
CHECK(SIGNED(intmax_t) && INTMAX_MAX == SMAX(intmax_t, sizeof(intmax_t) * CHAR_BIT) && INTMAX_MIN == -INTMAX_MAX - 1);
CHECK(PROMOTED(INTMAX_MAX, intmax_t) && PROMOTED(INTMAX_MIN, intmax_t));
CHECK(sizeof(intmax_t) * CHAR_BIT >= 64);
CHECK(PROMOTED(INTMAX_C(1), intmax_t) && INTMAX_C(63) == 63);
#if __STDC_VERSION__ >= 202311L
CHECK(INTMAX_WIDTH == sizeof(intmax_t) * CHAR_BIT);
#endif
CHECK(!SIGNED(uintmax_t) && UINTMAX_MAX == UMAX(uintmax_t) && PROMOTED(UINTMAX_MAX, uintmax_t));
CHECK(sizeof(uintmax_t) * CHAR_BIT >= 64);
CHECK(PROMOTED(UINTMAX_C(1), uintmax_t) && UINTMAX_C(64) == 64);
#if __STDC_VERSION__ >= 202311L
CHECK(UINTMAX_WIDTH == sizeof(uintmax_t) * CHAR_BIT);
#endif
CHECK(sizeof(intptr_t) >= sizeof(void *) && sizeof(uintptr_t) == sizeof(intptr_t));
CHECK(sizeof(intmax_t) >= sizeof(long long) && sizeof(uintmax_t) == sizeof(intmax_t));
CHECK(SIZE_MAX == UMAX(size_t) && PROMOTED(SIZE_MAX, size_t));
CHECK(PTRDIFF_MAX == SMAX(ptrdiff_t, sizeof(ptrdiff_t) * CHAR_BIT) && PTRDIFF_MIN == -PTRDIFF_MAX - 1);
CHECK(PROMOTED(PTRDIFF_MAX, ptrdiff_t) && PROMOTED(PTRDIFF_MIN, ptrdiff_t));
CHECK(SIGNED(wchar_t) ? (WCHAR_MAX == SMAX(wchar_t, sizeof(wchar_t) * CHAR_BIT) && WCHAR_MIN == -WCHAR_MAX - 1)
                      : (WCHAR_MAX == UMAX(wchar_t) && WCHAR_MIN == 0));
CHECK(PROMOTED(WCHAR_MAX, wchar_t) && PROMOTED(WCHAR_MIN, wchar_t));
CHECK(SIG_ATOMIC_MAX == INT_MAX && SIG_ATOMIC_MIN == INT_MIN);
CHECK(WINT_MAX >= WCHAR_MAX && WINT_MIN <= WCHAR_MIN);
#if __STDC_VERSION__ >= 202311L
CHECK(PTRDIFF_WIDTH == sizeof(ptrdiff_t) * CHAR_BIT && SIZE_WIDTH == sizeof(size_t) * CHAR_BIT);
CHECK(WCHAR_WIDTH == sizeof(wchar_t) * CHAR_BIT && SIG_ATOMIC_WIDTH == INT_WIDTH && WINT_WIDTH >= WCHAR_WIDTH);
#endif
#endif

static int hc_run(void) { return hc_null() + hc_va(1, 41); }
#endif

