/* float.h - characteristics of floating types (C89 7.1.5, C99 7.7 and
   5.2.4.2.2, C11 7.7, C23 7.7), for the Axiom compiler's C front end:
   float is IEC 60559 binary32, double and long double binary64. */
#ifndef __AXIOM_FLOAT_H
#define __AXIOM_FLOAT_H

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_FLOAT_H__ 202311L
#endif

#define FLT_ROUNDS 1
#define FLT_RADIX 2

#define FLT_MANT_DIG 24
#define DBL_MANT_DIG 53
#define LDBL_MANT_DIG 53

#define FLT_DIG 6
#define DBL_DIG 15
#define LDBL_DIG 15

#define FLT_MIN_EXP (-125)
#define DBL_MIN_EXP (-1021)
#define LDBL_MIN_EXP (-1021)

#define FLT_MIN_10_EXP (-37)
#define DBL_MIN_10_EXP (-307)
#define LDBL_MIN_10_EXP (-307)

#define FLT_MAX_EXP 128
#define DBL_MAX_EXP 1024
#define LDBL_MAX_EXP 1024

#define FLT_MAX_10_EXP 38
#define DBL_MAX_10_EXP 308
#define LDBL_MAX_10_EXP 308

/* (decimal constants that round to the values: hexadecimal ones are C99's) */
#define FLT_MAX 3.40282347e+38F
#define DBL_MAX 1.7976931348623157e+308
#define LDBL_MAX 1.7976931348623157e+308L

#define FLT_EPSILON 1.19209290e-7F
#define DBL_EPSILON 2.2204460492503131e-16
#define LDBL_EPSILON 2.2204460492503131e-16L

#define FLT_MIN 1.17549435e-38F
#define DBL_MIN 2.2250738585072014e-308
#define LDBL_MIN 2.2250738585072014e-308L

#if __STDC_VERSION__ >= 199901L
#define FLT_EVAL_METHOD 0
#define DECIMAL_DIG 17
#endif

#if __STDC_VERSION__ >= 201112L
#define FLT_DECIMAL_DIG 9
#define DBL_DECIMAL_DIG 17
#define LDBL_DECIMAL_DIG 17
#define FLT_HAS_SUBNORM 1
#define DBL_HAS_SUBNORM 1
#define LDBL_HAS_SUBNORM 1
#define FLT_TRUE_MIN 1.40129846e-45F
#define DBL_TRUE_MIN 4.9406564584124654e-324
#define LDBL_TRUE_MIN 4.9406564584124654e-324L
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

#endif
