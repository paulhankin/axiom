/* predef_checks.h - the conventional predefined macros of the types
   (__SIZEOF_INT__, __INT_MAX__, __SIZE_TYPE__, __INT64_C, __FLT_MAX__ ...)
   against sizeof, <limits.h>, <stdint.h>, <stddef.h>, <float.h> and, in a
   hosted build, <wchar.h> and <uchar.h>, for the c/run/predef_types_*.c
   tests: each a static assertion (C23), the types compared by _Generic, so
   that a macro has the value of the header's and its type too. */
#ifndef PREDEF_CHECKS_H
#define PREDEF_CHECKS_H
#include <float.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#if __STDC_HOSTED__
#include <signal.h>
#include <uchar.h>
#include <wchar.h>
#endif

/* the type of an expression, as a number (0: none of these) */
#define TK(x) _Generic((x), char: 1, signed char: 2, unsigned char: 3, short: 4, unsigned short: 5, int: 6, \
    unsigned: 7, long: 8, unsigned long: 9, long long: 10, unsigned long long: 11, float: 12, double: 13, \
    long double: 14, default: 0)
#define TYPE_IS(T, U) static_assert(TK((T)0) == TK((U)0) && TK((T)0) != 0, #T " is " #U)
#define VALUE_IS(a, b) static_assert((a) == (b) && TK(a) == TK(b), #a " is " #b)
/* (a floating constant: compared in a named constant's initializer, an
   arithmetic constant expression) */
#define FVALUE_IS(tag, a, b) constexpr bool f_same_##tag = (a) == (b); \
    static_assert(f_same_##tag && TK(a) == TK(b), #a " is " #b)

static_assert(__CHAR_BIT__ == CHAR_BIT);
/* sizes */
static_assert(__SIZEOF_SHORT__ == sizeof(short));
static_assert(__SIZEOF_INT__ == sizeof(int));
static_assert(__SIZEOF_LONG__ == sizeof(long));
static_assert(__SIZEOF_LONG_LONG__ == sizeof(long long));
static_assert(__SIZEOF_FLOAT__ == sizeof(float));
static_assert(__SIZEOF_DOUBLE__ == sizeof(double));
static_assert(__SIZEOF_LONG_DOUBLE__ == sizeof(long double));
static_assert(__SIZEOF_POINTER__ == sizeof(void *));
static_assert(__SIZEOF_SIZE_T__ == sizeof(size_t));
static_assert(__SIZEOF_PTRDIFF_T__ == sizeof(ptrdiff_t));
static_assert(__SIZEOF_WCHAR_T__ == sizeof(L'x'));
#ifdef __STDC_GNU__
#error "a GNU dialect's test defines it"
#endif
#if defined(__AXIOM_GNU__)
static_assert(__SIZEOF_INT128__ == sizeof(__int128));
static_assert(__SIZEOF_FLOAT128__ == sizeof(__float128));
#elif defined(__SIZEOF_INT128__) || defined(__SIZEOF_FLOAT128__)
#error "__int128 is a GNU dialect's"
#endif
static_assert(__BIGGEST_ALIGNMENT__ >= alignof(max_align_t) && __BIGGEST_ALIGNMENT__ >= alignof(long double));
/* the data model */
#if defined(__LP64__) != (__SIZEOF_INT__ == 4 && __SIZEOF_LONG__ == 8 && __SIZEOF_POINTER__ == 8)
#error "__LP64__"
#endif
#if defined(_LP64) != defined(__LP64__)
#error "_LP64"
#endif
#if defined(__ILP32__) != (__SIZEOF_INT__ == 4 && __SIZEOF_LONG__ == 4 && __SIZEOF_POINTER__ == 4)
#error "__ILP32__"
#endif
#if defined(__LLP64__) != (__SIZEOF_INT__ == 4 && __SIZEOF_LONG__ == 4 && __SIZEOF_POINTER__ == 8)
#error "__LLP64__"
#endif
#if defined(__CHAR_UNSIGNED__) != (CHAR_MIN == 0)
#error "__CHAR_UNSIGNED__"
#endif
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ || __FLOAT_WORD_ORDER__ != __ORDER_LITTLE_ENDIAN__ || \
    __ORDER_BIG_ENDIAN__ != 4321 || __ORDER_PDP_ENDIAN__ != 3412
#error "__BYTE_ORDER__"
#endif

/* <limits.h> */
VALUE_IS(__SCHAR_MAX__, SCHAR_MAX);
VALUE_IS(__SHRT_MAX__, SHRT_MAX);
VALUE_IS(__INT_MAX__, INT_MAX);
VALUE_IS(__LONG_MAX__, LONG_MAX);
VALUE_IS(__LONG_LONG_MAX__, LLONG_MAX);
VALUE_IS(__SCHAR_WIDTH__, SCHAR_WIDTH);
VALUE_IS(__SHRT_WIDTH__, SHRT_WIDTH);
VALUE_IS(__INT_WIDTH__, INT_WIDTH);
VALUE_IS(__LONG_WIDTH__, LONG_WIDTH);
VALUE_IS(__LONG_LONG_WIDTH__, LLONG_WIDTH);

/* <stddef.h> and <stdint.h> */
TYPE_IS(__SIZE_TYPE__, size_t);
TYPE_IS(__PTRDIFF_TYPE__, ptrdiff_t);
TYPE_IS(__WCHAR_TYPE__, wchar_t);
TYPE_IS(__WCHAR_TYPE__, typeof(L'x'));
VALUE_IS(__SIZE_MAX__, SIZE_MAX);
VALUE_IS(__PTRDIFF_MAX__, PTRDIFF_MAX);
VALUE_IS(__SIZE_WIDTH__, SIZE_WIDTH);
VALUE_IS(__PTRDIFF_WIDTH__, PTRDIFF_WIDTH);
VALUE_IS(__WCHAR_MAX__, WCHAR_MAX);
VALUE_IS(__WCHAR_MIN__, WCHAR_MIN);
VALUE_IS(__WCHAR_WIDTH__, WCHAR_WIDTH);
#if defined(__WCHAR_UNSIGNED__) != (WCHAR_MIN == 0)
#error "__WCHAR_UNSIGNED__"
#endif
VALUE_IS(__SIG_ATOMIC_MAX__, SIG_ATOMIC_MAX);
VALUE_IS(__SIG_ATOMIC_MIN__, SIG_ATOMIC_MIN);
VALUE_IS(__SIG_ATOMIC_WIDTH__, SIG_ATOMIC_WIDTH);
VALUE_IS(__WINT_MAX__, WINT_MAX);
VALUE_IS(__WINT_MIN__, WINT_MIN);
VALUE_IS(__WINT_WIDTH__, WINT_WIDTH);
#if __STDC_HOSTED__
TYPE_IS(__SIG_ATOMIC_TYPE__, sig_atomic_t);
TYPE_IS(__WINT_TYPE__, wint_t);
TYPE_IS(__CHAR16_TYPE__, char16_t);
TYPE_IS(__CHAR32_TYPE__, char32_t);
static_assert(__SIZEOF_WINT_T__ == sizeof(wint_t));
#endif
TYPE_IS(__CHAR16_TYPE__, typeof(u'x'));
TYPE_IS(__CHAR32_TYPE__, typeof(U'x'));

/* exact widths, their limits and constants */
#define EXACT(N) TYPE_IS(__INT##N##_TYPE__, int##N##_t); TYPE_IS(__UINT##N##_TYPE__, uint##N##_t); \
    VALUE_IS(__INT##N##_MAX__, INT##N##_MAX); VALUE_IS(__UINT##N##_MAX__, UINT##N##_MAX); \
    VALUE_IS(__INT##N##_C(7), INT##N##_C(7)); VALUE_IS(__UINT##N##_C(7), UINT##N##_C(7))
EXACT(8);
EXACT(16);
#ifdef INT32_MAX
EXACT(32);
#elif defined(__INT32_TYPE__) || defined(__INT32_MAX__) || defined(__INT32_C)
#error "there is no 32-bit type"
#endif
EXACT(64);
/* least and fast widths, pointers' integers, the widest */
#define ROLE(R, r) TYPE_IS(__##R##_TYPE__, r##_t); TYPE_IS(__U##R##_TYPE__, u##r##_t); \
    VALUE_IS(__##R##_MAX__, R##_MAX); VALUE_IS(__U##R##_MAX__, U##R##_MAX); VALUE_IS(__##R##_WIDTH__, R##_WIDTH)
ROLE(INT_LEAST8, int_least8);
ROLE(INT_LEAST16, int_least16);
ROLE(INT_LEAST32, int_least32);
ROLE(INT_LEAST64, int_least64);
ROLE(INT_FAST8, int_fast8);
ROLE(INT_FAST16, int_fast16);
ROLE(INT_FAST32, int_fast32);
ROLE(INT_FAST64, int_fast64);
ROLE(INTPTR, intptr);
ROLE(INTMAX, intmax);
VALUE_IS(__INTMAX_C(7), INTMAX_C(7));
VALUE_IS(__UINTMAX_C(7), UINTMAX_C(7));

/* <float.h> */
VALUE_IS(__FLT_RADIX__, FLT_RADIX);
VALUE_IS(__FLT_EVAL_METHOD__, FLT_EVAL_METHOD);
VALUE_IS(__DECIMAL_DIG__, DECIMAL_DIG);
#define FLOATS(T) VALUE_IS(__##T##_MANT_DIG__, T##_MANT_DIG); VALUE_IS(__##T##_DIG__, T##_DIG); \
    VALUE_IS(__##T##_DECIMAL_DIG__, T##_DECIMAL_DIG); VALUE_IS(__##T##_MIN_EXP__, T##_MIN_EXP); \
    VALUE_IS(__##T##_MIN_10_EXP__, T##_MIN_10_EXP); VALUE_IS(__##T##_MAX_EXP__, T##_MAX_EXP); \
    VALUE_IS(__##T##_MAX_10_EXP__, T##_MAX_10_EXP); VALUE_IS(__##T##_HAS_DENORM__, T##_HAS_SUBNORM); \
    VALUE_IS(__##T##_IS_IEC_60559__, T##_IS_IEC_60559); FVALUE_IS(T##_max, __##T##_MAX__, T##_MAX); \
    FVALUE_IS(T##_norm_max, __##T##_NORM_MAX__, T##_NORM_MAX); FVALUE_IS(T##_min, __##T##_MIN__, T##_MIN); \
    FVALUE_IS(T##_eps, __##T##_EPSILON__, T##_EPSILON); FVALUE_IS(T##_tmin, __##T##_DENORM_MIN__, T##_TRUE_MIN); \
    static_assert(__##T##_HAS_INFINITY__ == 1 && __##T##_HAS_QUIET_NAN__ == 1)
FLOATS(FLT);
FLOATS(DBL);
FLOATS(LDBL);
#endif
