/* math.h - mathematics (C89 4.5, C99 7.12, C11 7.12, C17 7.12, C23 7.12),
   for the Axiom compiler's C library (std/c/rt/math*.ax and
   std/c/lib/math/; docs/c.md, "<math.h>").

   - FLT_EVAL_METHOD is 0: float_t is float, double_t is double.
   - math_errhandling is MATH_ERRNO | MATH_ERREXCEPT where the target has
     floating-point exception flags (x86-64, AArch64), MATH_ERRNO alone on
     WebAssembly, which has none (__builtin_axiom_fenv_kind is the target's:
     an integer constant expression either way).
   - The classification and comparison macros are the compiler's builtins
     (__builtin_axiom_isnan, ...): each operand is evaluated once, the
     classification is done on the bits and the comparisons are quiet, so
     none raises a floating-point exception for a quiet NaN.
   - long double has the format of the target's C ABI (<float.h>); where it
     is double, the l functions are the double ones'. */
#ifndef __AXIOM_MATH_H
#define __AXIOM_MATH_H

#define HUGE_VAL ((double)__builtin_inff())

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_MATH_H__ 202311L
#endif

#if __STDC_VERSION__ >= 199901L
typedef float float_t;
typedef double double_t;

#define HUGE_VALF __builtin_inff()
#define HUGE_VALL ((long double)__builtin_inff())
#define INFINITY __builtin_inff()
#define NAN __builtin_nanf("")

#define FP_NAN 0
#define FP_INFINITE 1
#define FP_ZERO 2
#define FP_SUBNORMAL 3
#define FP_NORMAL 4

/* ilogb's results for 0 and NaN: INT_MIN and INT_MAX of the build's int */
#if __AXIOM_INT_WIDTH__ == 16
#define FP_ILOGB0 (-32767 - 1)
#define FP_ILOGBNAN 32767
#elif __AXIOM_INT_WIDTH__ == 32
#define FP_ILOGB0 (-2147483647 - 1)
#define FP_ILOGBNAN 2147483647
#else
#define FP_ILOGB0 (-9223372036854775807 - 1)
#define FP_ILOGBNAN 9223372036854775807
#endif

#define MATH_ERRNO 1
#define MATH_ERREXCEPT 2
#define math_errhandling (__builtin_axiom_fenv_kind() != 0 ? MATH_ERRNO | MATH_ERREXCEPT : MATH_ERRNO)

#define fpclassify(x) __builtin_axiom_fpclassify(x)
#define isfinite(x) __builtin_axiom_isfinite(x)
#define isinf(x) __builtin_axiom_isinf(x)
#define isnan(x) __builtin_axiom_isnan(x)
#define isnormal(x) __builtin_axiom_isnormal(x)
#define signbit(x) __builtin_axiom_signbit(x)

#define isgreater(x, y) __builtin_axiom_isgreater(x, y)
#define isgreaterequal(x, y) __builtin_axiom_isgreaterequal(x, y)
#define isless(x, y) __builtin_axiom_isless(x, y)
#define islessequal(x, y) __builtin_axiom_islessequal(x, y)
#define islessgreater(x, y) __builtin_axiom_islessgreater(x, y)
#define isunordered(x, y) __builtin_axiom_isunordered(x, y)
#endif

#if __STDC_VERSION__ >= 202311L
#define issubnormal(x) __builtin_axiom_issubnormal(x)
#define iszero(x) __builtin_axiom_iszero(x)
#define issignaling(x) __builtin_axiom_issignaling(x)
#define iscanonical(x) __builtin_axiom_iscanonical(x)
#endif

/* the double functions of C89 */
double acos(double __x);
double asin(double __x);
double atan(double __x);
double cos(double __x);
double sin(double __x);
double tan(double __x);
double cosh(double __x);
double sinh(double __x);
double tanh(double __x);
double exp(double __x);
double log(double __x);
double log10(double __x);
double sqrt(double __x);
double ceil(double __x);
double fabs(double __x);
double floor(double __x);
double atan2(double __x, double __y);
double pow(double __x, double __y);
double fmod(double __x, double __y);
double frexp(double __x, int *__e);
double ldexp(double __x, int __e);
double modf(double __x, double *__i);

#if __STDC_VERSION__ >= 199901L
/* double */
double acosh(double __x);
double asinh(double __x);
double atanh(double __x);
double exp2(double __x);
double expm1(double __x);
double log1p(double __x);
double log2(double __x);
double logb(double __x);
double cbrt(double __x);
double erf(double __x);
double erfc(double __x);
double lgamma(double __x);
double tgamma(double __x);
double nearbyint(double __x);
double rint(double __x);
double round(double __x);
double trunc(double __x);
double hypot(double __x, double __y);
double remainder(double __x, double __y);
double copysign(double __x, double __y);
double nextafter(double __x, double __y);
double fdim(double __x, double __y);
double fmax(double __x, double __y);
double fmin(double __x, double __y);
int ilogb(double __x);
double scalbn(double __x, int __n);
double scalbln(double __x, long __n);
long lrint(double __x);
long long llrint(double __x);
long lround(double __x);
long long llround(double __x);
double remquo(double __x, double __y, int *__q);
double nan(const char *__tag);
double nexttoward(double __x, long double __y);
double fma(double __x, double __y, double __z);
/* float */
float acosf(float __x);
float asinf(float __x);
float atanf(float __x);
float cosf(float __x);
float sinf(float __x);
float tanf(float __x);
float coshf(float __x);
float sinhf(float __x);
float tanhf(float __x);
float expf(float __x);
float logf(float __x);
float log10f(float __x);
float sqrtf(float __x);
float ceilf(float __x);
float fabsf(float __x);
float floorf(float __x);
float atan2f(float __x, float __y);
float powf(float __x, float __y);
float fmodf(float __x, float __y);
float frexpf(float __x, int *__e);
float ldexpf(float __x, int __e);
float modff(float __x, float *__i);
float acoshf(float __x);
float asinhf(float __x);
float atanhf(float __x);
float exp2f(float __x);
float expm1f(float __x);
float log1pf(float __x);
float log2f(float __x);
float logbf(float __x);
float cbrtf(float __x);
float erff(float __x);
float erfcf(float __x);
float lgammaf(float __x);
float tgammaf(float __x);
float nearbyintf(float __x);
float rintf(float __x);
float roundf(float __x);
float truncf(float __x);
float hypotf(float __x, float __y);
float remainderf(float __x, float __y);
float copysignf(float __x, float __y);
float nextafterf(float __x, float __y);
float fdimf(float __x, float __y);
float fmaxf(float __x, float __y);
float fminf(float __x, float __y);
int ilogbf(float __x);
float scalbnf(float __x, int __n);
float scalblnf(float __x, long __n);
long lrintf(float __x);
long long llrintf(float __x);
long lroundf(float __x);
long long llroundf(float __x);
float remquof(float __x, float __y, int *__q);
float nanf(const char *__tag);
float nexttowardf(float __x, long double __y);
float fmaf(float __x, float __y, float __z);
/* long double */
long double acosl(long double __x);
long double asinl(long double __x);
long double atanl(long double __x);
long double cosl(long double __x);
long double sinl(long double __x);
long double tanl(long double __x);
long double coshl(long double __x);
long double sinhl(long double __x);
long double tanhl(long double __x);
long double expl(long double __x);
long double logl(long double __x);
long double log10l(long double __x);
long double sqrtl(long double __x);
long double ceill(long double __x);
long double fabsl(long double __x);
long double floorl(long double __x);
long double atan2l(long double __x, long double __y);
long double powl(long double __x, long double __y);
long double fmodl(long double __x, long double __y);
long double frexpl(long double __x, int *__e);
long double ldexpl(long double __x, int __e);
long double modfl(long double __x, long double *__i);
long double acoshl(long double __x);
long double asinhl(long double __x);
long double atanhl(long double __x);
long double exp2l(long double __x);
long double expm1l(long double __x);
long double log1pl(long double __x);
long double log2l(long double __x);
long double logbl(long double __x);
long double cbrtl(long double __x);
long double erfl(long double __x);
long double erfcl(long double __x);
long double lgammal(long double __x);
long double tgammal(long double __x);
long double nearbyintl(long double __x);
long double rintl(long double __x);
long double roundl(long double __x);
long double truncl(long double __x);
long double hypotl(long double __x, long double __y);
long double remainderl(long double __x, long double __y);
long double copysignl(long double __x, long double __y);
long double nextafterl(long double __x, long double __y);
long double fdiml(long double __x, long double __y);
long double fmaxl(long double __x, long double __y);
long double fminl(long double __x, long double __y);
int ilogbl(long double __x);
long double scalbnl(long double __x, int __n);
long double scalblnl(long double __x, long __n);
long lrintl(long double __x);
long long llrintl(long double __x);
long lroundl(long double __x);
long long llroundl(long double __x);
long double remquol(long double __x, long double __y, int *__q);
long double nanl(const char *__tag);
long double nexttowardl(long double __x, long double __y);
long double fmal(long double __x, long double __y, long double __z);
#endif

#if __STDC_VERSION__ >= 202311L
/* the functions of C23 that the library has */
/* double */
double exp10(double __x);
double roundeven(double __x);
double nextup(double __x);
double nextdown(double __x);
double sinpi(double __x);
double cospi(double __x);
double fmaximum(double __x, double __y);
double fminimum(double __x, double __y);
double fmaximum_mag(double __x, double __y);
double fminimum_mag(double __x, double __y);
double fmaximum_num(double __x, double __y);
double fminimum_num(double __x, double __y);
double fmaximum_mag_num(double __x, double __y);
double fminimum_mag_num(double __x, double __y);
/* float */
float exp10f(float __x);
float roundevenf(float __x);
float nextupf(float __x);
float nextdownf(float __x);
float sinpif(float __x);
float cospif(float __x);
float fmaximumf(float __x, float __y);
float fminimumf(float __x, float __y);
float fmaximum_magf(float __x, float __y);
float fminimum_magf(float __x, float __y);
float fmaximum_numf(float __x, float __y);
float fminimum_numf(float __x, float __y);
float fmaximum_mag_numf(float __x, float __y);
float fminimum_mag_numf(float __x, float __y);
/* long double */
long double exp10l(long double __x);
long double roundevenl(long double __x);
long double nextupl(long double __x);
long double nextdownl(long double __x);
long double sinpil(long double __x);
long double cospil(long double __x);
long double fmaximuml(long double __x, long double __y);
long double fminimuml(long double __x, long double __y);
long double fmaximum_magl(long double __x, long double __y);
long double fminimum_magl(long double __x, long double __y);
long double fmaximum_numl(long double __x, long double __y);
long double fminimum_numl(long double __x, long double __y);
long double fmaximum_mag_numl(long double __x, long double __y);
long double fminimum_mag_numl(long double __x, long double __y);
#endif

#endif
