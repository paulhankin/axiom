/* float.h - characteristics of floating types (C89 7.1.5, C99 7.7 and
   5.2.4.2.2, C11 7.7, C23 7.7), for the Axiom compiler's C front end:
   float is IEC 60559 binary32, double binary64, and long double the
   format __AXIOM_LDBL_WIDTH__ names (--c-long-double): 64, binary64; 80,
   the x87's 80-bit extended format (64-bit significand); 128, binary128.
   In C23 also the decimal floating types' DEC_ macros. */
#ifndef __AXIOM_FLOAT_H
#define __AXIOM_FLOAT_H

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_FLOAT_H__ 202311L
#endif

#define FLT_ROUNDS 1
#define FLT_RADIX 2

/* (decimal constants that round to the values: hexadecimal ones are C99's) */
#if __AXIOM_LDBL_WIDTH__ == 80
#define LDBL_MANT_DIG 64
#define LDBL_DIG 18
#define LDBL_MIN_EXP (-16381)
#define LDBL_MIN_10_EXP (-4931)
#define LDBL_MAX_EXP 16384
#define LDBL_MAX_10_EXP 4932
#define LDBL_MAX 1.18973149535723176502e+4932L
#define LDBL_EPSILON 1.08420217248550443401e-19L
#define LDBL_MIN 3.36210314311209350626e-4932L
#define __AXIOM_LDBL_TRUE_MIN 3.64519953188247460253e-4951L
#define __AXIOM_LDBL_DECIMAL_DIG 21
#elif __AXIOM_LDBL_WIDTH__ == 128
#define LDBL_MANT_DIG 113
#define LDBL_DIG 33
#define LDBL_MIN_EXP (-16381)
#define LDBL_MIN_10_EXP (-4931)
#define LDBL_MAX_EXP 16384
#define LDBL_MAX_10_EXP 4932
#define LDBL_MAX 1.18973149535723176508575932662800702e+4932L
#define LDBL_EPSILON 1.92592994438723585305597794258492732e-34L
#define LDBL_MIN 3.36210314311209350626267781732175260e-4932L
#define __AXIOM_LDBL_TRUE_MIN 6.47517511943802511092443895822764655e-4966L
#define __AXIOM_LDBL_DECIMAL_DIG 36
#else
#define LDBL_MANT_DIG 53
#define LDBL_DIG 15
#define LDBL_MIN_EXP (-1021)
#define LDBL_MIN_10_EXP (-307)
#define LDBL_MAX_EXP 1024
#define LDBL_MAX_10_EXP 308
#define LDBL_MAX 1.7976931348623157e+308L
#define LDBL_EPSILON 2.2204460492503131e-16L
#define LDBL_MIN 2.2250738585072014e-308L
#define __AXIOM_LDBL_TRUE_MIN 4.9406564584124654e-324L
#define __AXIOM_LDBL_DECIMAL_DIG 17
#endif

#define FLT_MANT_DIG 24
#define DBL_MANT_DIG 53

#define FLT_DIG 6
#define DBL_DIG 15

#define FLT_MIN_EXP (-125)
#define DBL_MIN_EXP (-1021)

#define FLT_MIN_10_EXP (-37)
#define DBL_MIN_10_EXP (-307)

#define FLT_MAX_EXP 128
#define DBL_MAX_EXP 1024

#define FLT_MAX_10_EXP 38
#define DBL_MAX_10_EXP 308

#define FLT_MAX 3.40282347e+38F
#define DBL_MAX 1.7976931348623157e+308

#define FLT_EPSILON 1.19209290e-7F
#define DBL_EPSILON 2.2204460492503131e-16

#define FLT_MIN 1.17549435e-38F
#define DBL_MIN 2.2250738585072014e-308

#if __STDC_VERSION__ >= 199901L
#define FLT_EVAL_METHOD 0
#define DECIMAL_DIG __AXIOM_LDBL_DECIMAL_DIG
#endif

#if __STDC_VERSION__ >= 201112L
#define FLT_DECIMAL_DIG 9
#define DBL_DECIMAL_DIG 17
#define LDBL_DECIMAL_DIG __AXIOM_LDBL_DECIMAL_DIG
#define FLT_HAS_SUBNORM 1
#define DBL_HAS_SUBNORM 1
#define LDBL_HAS_SUBNORM 1
#define FLT_TRUE_MIN 1.40129846e-45F
#define DBL_TRUE_MIN 4.9406564584124654e-324
#define LDBL_TRUE_MIN __AXIOM_LDBL_TRUE_MIN
#endif

#if __STDC_VERSION__ >= 202311L
#define FLT_NORM_MAX FLT_MAX
#define DBL_NORM_MAX DBL_MAX
#define LDBL_NORM_MAX LDBL_MAX
#define FLT_IS_IEC_60559 1
#define DBL_IS_IEC_60559 1
#define LDBL_IS_IEC_60559 1
#define INFINITY __builtin_inff()
#define NAN __builtin_nanf("")
#endif

#ifdef __STDC_IEC_60559_DFP__
/* the decimal floating types (C23 5.2.5.3.4): IEC 60559's decimal32,
   decimal64 and decimal128 formats, evaluated in their own range and
   precision; their signaling NaNs stay signaling (the arithmetic is the
   library's, in software) */
#define DEC_EVAL_METHOD 0
#define DEC32_MANT_DIG 7
#define DEC64_MANT_DIG 16
#define DEC128_MANT_DIG 34
#define DEC32_MIN_EXP (-94)
#define DEC64_MIN_EXP (-382)
#define DEC128_MIN_EXP (-6142)
#define DEC32_MAX_EXP 97
#define DEC64_MAX_EXP 385
#define DEC128_MAX_EXP 6145
#define DEC32_MAX 9.999999E96DF
#define DEC64_MAX 9.999999999999999E384DD
#define DEC128_MAX 9.999999999999999999999999999999999E6144DL
#define DEC32_EPSILON 1E-6DF
#define DEC64_EPSILON 1E-15DD
#define DEC128_EPSILON 1E-33DL
#define DEC32_MIN 1E-95DF
#define DEC64_MIN 1E-383DD
#define DEC128_MIN 1E-6143DL
#define DEC32_TRUE_MIN 0.000001E-95DF
#define DEC64_TRUE_MIN 0.000000000000001E-383DD
#define DEC128_TRUE_MIN 0.000000000000000000000000000000001E-6143DL
#define DEC32_SNAN __builtin_axiom_dec(32, 0, 0x7e000000)
#define DEC64_SNAN __builtin_axiom_dec(64, 0, 0x7e00000000000000)
#define DEC128_SNAN __builtin_axiom_dec(128, 0x7e00000000000000, 0)
#endif

#endif
