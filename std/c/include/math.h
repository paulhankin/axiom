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
/* the rounding directions of fromfp and the like (7.12p17) */
#define FP_INT_UPWARD 0
#define FP_INT_DOWNWARD 1
#define FP_INT_TOWARDZERO 2
#define FP_INT_TONEARESTFROMZERO 3
#define FP_INT_TONEAREST 4

/* llogb's results for 0 and NaN: LONG_MIN and LONG_MAX of the build's long */
#if __AXIOM_LONG_WIDTH__ == 32
#define FP_LLOGB0 (-2147483647L - 1)
#define FP_LLOGBNAN 2147483647L
#else
#define FP_LLOGB0 (-9223372036854775807L - 1)
#define FP_LLOGBNAN 9223372036854775807L
#endif

/* FP_FAST_FMA, FP_FAST_FMAF, FP_FAST_FMAL and the FP_FAST_ macros of the
   narrowing functions are not defined: each is a call of the library's
   function, slower than a multiplication and an addition */

/* the signaling comparison: "invalid" and a domain error for a NaN */
#define iseqsig(x, y) __builtin_axiom_tg(2, __axc_iseqsigf, __axc_iseqsig, __axc_iseqsigl, 0, 0, 0, x, y)
int __axc_iseqsigf(float __x, float __y);
int __axc_iseqsig(double __x, double __y);
int __axc_iseqsigl(long double __x, long double __y);

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
/* the functions of C23 */
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
/* C23's functions of double */
double exp2m1(double __x);
double exp10m1(double __x);
double log2p1(double __x);
double log10p1(double __x);
double logp1(double __x);
double rsqrt(double __x);
double tanpi(double __x);
double asinpi(double __x);
double acospi(double __x);
double atanpi(double __x);
double atan2pi(double __y, double __x);
double rootn(double __x, long long __n);
double pown(double __x, long long __n);
double powr(double __x, double __y);
double compoundn(double __x, long long __n);
double fromfp(double __x, int __r, unsigned int __w);
double ufromfp(double __x, int __r, unsigned int __w);
double fromfpx(double __x, int __r, unsigned int __w);
double ufromfpx(double __x, int __r, unsigned int __w);
double getpayload(const double *__x);
int setpayload(double *__r, double __pl);
int setpayloadsig(double *__r, double __pl);
int totalorder(const double *__x, const double *__y);
int totalordermag(const double *__x, const double *__y);
int canonicalize(double *__c, const double *__x);
long llogb(double __x);
/* C23's functions of float */
float exp2m1f(float __x);
float exp10m1f(float __x);
float log2p1f(float __x);
float log10p1f(float __x);
float logp1f(float __x);
float rsqrtf(float __x);
float tanpif(float __x);
float asinpif(float __x);
float acospif(float __x);
float atanpif(float __x);
float atan2pif(float __y, float __x);
float rootnf(float __x, long long __n);
float pownf(float __x, long long __n);
float powrf(float __x, float __y);
float compoundnf(float __x, long long __n);
float fromfpf(float __x, int __r, unsigned int __w);
float ufromfpf(float __x, int __r, unsigned int __w);
float fromfpxf(float __x, int __r, unsigned int __w);
float ufromfpxf(float __x, int __r, unsigned int __w);
float getpayloadf(const float *__x);
int setpayloadf(float *__r, float __pl);
int setpayloadsigf(float *__r, float __pl);
int totalorderf(const float *__x, const float *__y);
int totalordermagf(const float *__x, const float *__y);
int canonicalizef(float *__c, const float *__x);
long llogbf(float __x);
/* C23's functions of long double */
long double exp2m1l(long double __x);
long double exp10m1l(long double __x);
long double log2p1l(long double __x);
long double log10p1l(long double __x);
long double logp1l(long double __x);
long double rsqrtl(long double __x);
long double tanpil(long double __x);
long double asinpil(long double __x);
long double acospil(long double __x);
long double atanpil(long double __x);
long double atan2pil(long double __y, long double __x);
long double rootnl(long double __x, long long __n);
long double pownl(long double __x, long long __n);
long double powrl(long double __x, long double __y);
long double compoundnl(long double __x, long long __n);
long double fromfpl(long double __x, int __r, unsigned int __w);
long double ufromfpl(long double __x, int __r, unsigned int __w);
long double fromfpxl(long double __x, int __r, unsigned int __w);
long double ufromfpxl(long double __x, int __r, unsigned int __w);
long double getpayloadl(const long double *__x);
int setpayloadl(long double *__r, long double __pl);
int setpayloadsigl(long double *__r, long double __pl);
int totalorderl(const long double *__x, const long double *__y);
int totalordermagl(const long double *__x, const long double *__y);
int canonicalizel(long double *__c, const long double *__x);
long llogbl(long double __x);
/* the narrowing operations (7.12.14) */
float fadd(double __x, double __y);
float faddl(long double __x, long double __y);
double daddl(long double __x, long double __y);
float fsub(double __x, double __y);
float fsubl(long double __x, long double __y);
double dsubl(long double __x, long double __y);
float fmul(double __x, double __y);
float fmull(long double __x, long double __y);
double dmull(long double __x, long double __y);
float fdiv(double __x, double __y);
float fdivl(long double __x, long double __y);
double ddivl(long double __x, long double __y);
float ffma(double __x, double __y, double __z);
float ffmal(long double __x, long double __y, long double __z);
double dfmal(long double __x, long double __y, long double __z);
float fsqrt(double __x);
float fsqrtl(long double __x);
double dsqrtl(long double __x);
#endif

#ifdef __STDC_IEC_60559_DFP__
/* The decimal floating types (C23 7.12, its functions of the d32, d64 and d128 suffixes):
   the library's functions in software (std/c/rt/dfp.ax, dfp_math.ax; the
   C wrappers std/c/lib/math/dfp_*.c). Each rounds in the decimal rounding
   direction (fe_dec_setround) and raises its exceptions; the exact ones
   (the rounding functions, fmod, remainder, fma, sqrt, quantize, scalbn,
   frexp, nextafter, ...) are correctly rounded, the elementary ones within
   1 ulp (computed in arbitrary precision, then rounded once: almost always
   correctly rounded). An exact result has the quantum exponent C23 prefers
   for the function (the operands' for most, max(q, 0) for the rounding
   functions, floor(q / 2) for sqrt, floor(q / 3) for cbrt, 0 for the
   exponential and logarithmic ones); an inexact one has the format's
   precision. */
typedef _Decimal32 _Decimal32_t;
typedef _Decimal64 _Decimal64_t;

#define HUGE_VAL_D32 __builtin_axiom_dec(32, 0, 0x78000000)
#define HUGE_VAL_D64 __builtin_axiom_dec(64, 0, 0x7800000000000000)
#define HUGE_VAL_D128 __builtin_axiom_dec(128, 0x7800000000000000, 0)
#define DEC_INFINITY HUGE_VAL_D32
#define DEC_NAN __builtin_axiom_dec(32, 0, 0x7c000000)

/* the rounding directions of fromfp and its kin (7.12.9.10) */
#ifndef FP_INT_UPWARD
#define FP_INT_UPWARD 0
#define FP_INT_DOWNWARD 1
#define FP_INT_TOWARDZERO 2
#define FP_INT_TONEARESTFROMZERO 3
#define FP_INT_TONEAREST 4
#endif

/* iseqsig's (the type-generic macro's) */
int __axc_iseqsigd32(_Decimal32 __x, _Decimal32 __y);
int __axc_iseqsigd64(_Decimal64 __x, _Decimal64 __y);
int __axc_iseqsigd128(_Decimal128 __x, _Decimal128 __y);

/* _Decimal32 */
_Decimal32 cosd32(_Decimal32 __x);
_Decimal32 sind32(_Decimal32 __x);
_Decimal32 tand32(_Decimal32 __x);
_Decimal32 cospid32(_Decimal32 __x);
_Decimal32 sinpid32(_Decimal32 __x);
_Decimal32 tanpid32(_Decimal32 __x);
_Decimal32 acosd32(_Decimal32 __x);
_Decimal32 asind32(_Decimal32 __x);
_Decimal32 atand32(_Decimal32 __x);
_Decimal32 acospid32(_Decimal32 __x);
_Decimal32 asinpid32(_Decimal32 __x);
_Decimal32 atanpid32(_Decimal32 __x);
_Decimal32 atan2d32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 atan2pid32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 coshd32(_Decimal32 __x);
_Decimal32 sinhd32(_Decimal32 __x);
_Decimal32 tanhd32(_Decimal32 __x);
_Decimal32 acoshd32(_Decimal32 __x);
_Decimal32 asinhd32(_Decimal32 __x);
_Decimal32 atanhd32(_Decimal32 __x);
_Decimal32 expd32(_Decimal32 __x);
_Decimal32 exp10d32(_Decimal32 __x);
_Decimal32 exp10m1d32(_Decimal32 __x);
_Decimal32 exp2d32(_Decimal32 __x);
_Decimal32 exp2m1d32(_Decimal32 __x);
_Decimal32 expm1d32(_Decimal32 __x);
_Decimal32 logd32(_Decimal32 __x);
_Decimal32 log10d32(_Decimal32 __x);
_Decimal32 log10p1d32(_Decimal32 __x);
_Decimal32 log1pd32(_Decimal32 __x);
_Decimal32 logp1d32(_Decimal32 __x);
_Decimal32 log2d32(_Decimal32 __x);
_Decimal32 log2p1d32(_Decimal32 __x);
_Decimal32 cbrtd32(_Decimal32 __x);
_Decimal32 rsqrtd32(_Decimal32 __x);
_Decimal32 powd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 powrd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 hypotd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 pownd32(_Decimal32 __x, long long __n);
_Decimal32 rootnd32(_Decimal32 __x, long long __n);
_Decimal32 compoundnd32(_Decimal32 __x, long long __n);
_Decimal32 erfd32(_Decimal32 __x);
_Decimal32 erfcd32(_Decimal32 __x);
_Decimal32 lgammad32(_Decimal32 __x);
_Decimal32 tgammad32(_Decimal32 __x);
_Decimal32 sqrtd32(_Decimal32 __x);
_Decimal32 ceild32(_Decimal32 __x);
_Decimal32 floord32(_Decimal32 __x);
_Decimal32 nearbyintd32(_Decimal32 __x);
_Decimal32 rintd32(_Decimal32 __x);
_Decimal32 roundd32(_Decimal32 __x);
_Decimal32 roundevend32(_Decimal32 __x);
_Decimal32 truncd32(_Decimal32 __x);
_Decimal32 fabsd32(_Decimal32 __x);
_Decimal32 nextupd32(_Decimal32 __x);
_Decimal32 nextdownd32(_Decimal32 __x);
_Decimal32 quantumd32(_Decimal32 __x);
_Decimal32 logbd32(_Decimal32 __x);
_Decimal32 fmodd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 remainderd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 copysignd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 nextafterd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 fdimd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 fmaxd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 fmind32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 fmaximumd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 fminimumd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 fmaximum_magd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 fminimum_magd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 fmaximum_numd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 fminimum_numd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 fmaximum_mag_numd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 fminimum_mag_numd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 quantized32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 fmad32(_Decimal32 __x, _Decimal32 __y, _Decimal32 __z);
_Decimal32 ldexpd32(_Decimal32 __x, int __n);
_Decimal32 scalbnd32(_Decimal32 __x, int __n);
_Decimal32 scalblnd32(_Decimal32 __x, long __n);
int ilogbd32(_Decimal32 __x);
long llogbd32(_Decimal32 __x);
long lrintd32(_Decimal32 __x);
long lroundd32(_Decimal32 __x);
long long llrintd32(_Decimal32 __x);
long long llroundd32(_Decimal32 __x);
long long llquantexpd32(_Decimal32 __x);
_Decimal32 frexpd32(_Decimal32 __x, int *__e);
_Decimal32 modfd32(_Decimal32 __x, _Decimal32 *__i);
_Decimal32 nexttowardd32(_Decimal32 __x, _Decimal128 __y);
bool samequantumd32(_Decimal32 __x, _Decimal32 __y);
_Decimal32 nand32(const char *__tag);
_Decimal32 fromfpd32(_Decimal32 __x, int __rnd, unsigned int __width);
_Decimal32 ufromfpd32(_Decimal32 __x, int __rnd, unsigned int __width);
_Decimal32 fromfpxd32(_Decimal32 __x, int __rnd, unsigned int __width);
_Decimal32 ufromfpxd32(_Decimal32 __x, int __rnd, unsigned int __width);
int canonicalized32(_Decimal32 *__cx, const _Decimal32 *__x);
int totalorderd32(const _Decimal32 *__x, const _Decimal32 *__y);
int totalordermagd32(const _Decimal32 *__x, const _Decimal32 *__y);
_Decimal32 getpayloadd32(const _Decimal32 *__x);
int setpayloadd32(_Decimal32 *__res, _Decimal32 __pl);
int setpayloadsigd32(_Decimal32 *__res, _Decimal32 __pl);
void encodedecd32(_Decimal32 *restrict __enc, const _Decimal32 *restrict __x);
void decodedecd32(_Decimal32 *restrict __x, const _Decimal32 *restrict __enc);
void encodebind32(_Decimal32 *restrict __enc, const _Decimal32 *restrict __x);
void decodebind32(_Decimal32 *restrict __x, const _Decimal32 *restrict __enc);
/* _Decimal64 */
_Decimal64 cosd64(_Decimal64 __x);
_Decimal64 sind64(_Decimal64 __x);
_Decimal64 tand64(_Decimal64 __x);
_Decimal64 cospid64(_Decimal64 __x);
_Decimal64 sinpid64(_Decimal64 __x);
_Decimal64 tanpid64(_Decimal64 __x);
_Decimal64 acosd64(_Decimal64 __x);
_Decimal64 asind64(_Decimal64 __x);
_Decimal64 atand64(_Decimal64 __x);
_Decimal64 acospid64(_Decimal64 __x);
_Decimal64 asinpid64(_Decimal64 __x);
_Decimal64 atanpid64(_Decimal64 __x);
_Decimal64 atan2d64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 atan2pid64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 coshd64(_Decimal64 __x);
_Decimal64 sinhd64(_Decimal64 __x);
_Decimal64 tanhd64(_Decimal64 __x);
_Decimal64 acoshd64(_Decimal64 __x);
_Decimal64 asinhd64(_Decimal64 __x);
_Decimal64 atanhd64(_Decimal64 __x);
_Decimal64 expd64(_Decimal64 __x);
_Decimal64 exp10d64(_Decimal64 __x);
_Decimal64 exp10m1d64(_Decimal64 __x);
_Decimal64 exp2d64(_Decimal64 __x);
_Decimal64 exp2m1d64(_Decimal64 __x);
_Decimal64 expm1d64(_Decimal64 __x);
_Decimal64 logd64(_Decimal64 __x);
_Decimal64 log10d64(_Decimal64 __x);
_Decimal64 log10p1d64(_Decimal64 __x);
_Decimal64 log1pd64(_Decimal64 __x);
_Decimal64 logp1d64(_Decimal64 __x);
_Decimal64 log2d64(_Decimal64 __x);
_Decimal64 log2p1d64(_Decimal64 __x);
_Decimal64 cbrtd64(_Decimal64 __x);
_Decimal64 rsqrtd64(_Decimal64 __x);
_Decimal64 powd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 powrd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 hypotd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 pownd64(_Decimal64 __x, long long __n);
_Decimal64 rootnd64(_Decimal64 __x, long long __n);
_Decimal64 compoundnd64(_Decimal64 __x, long long __n);
_Decimal64 erfd64(_Decimal64 __x);
_Decimal64 erfcd64(_Decimal64 __x);
_Decimal64 lgammad64(_Decimal64 __x);
_Decimal64 tgammad64(_Decimal64 __x);
_Decimal64 sqrtd64(_Decimal64 __x);
_Decimal64 ceild64(_Decimal64 __x);
_Decimal64 floord64(_Decimal64 __x);
_Decimal64 nearbyintd64(_Decimal64 __x);
_Decimal64 rintd64(_Decimal64 __x);
_Decimal64 roundd64(_Decimal64 __x);
_Decimal64 roundevend64(_Decimal64 __x);
_Decimal64 truncd64(_Decimal64 __x);
_Decimal64 fabsd64(_Decimal64 __x);
_Decimal64 nextupd64(_Decimal64 __x);
_Decimal64 nextdownd64(_Decimal64 __x);
_Decimal64 quantumd64(_Decimal64 __x);
_Decimal64 logbd64(_Decimal64 __x);
_Decimal64 fmodd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 remainderd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 copysignd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 nextafterd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 fdimd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 fmaxd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 fmind64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 fmaximumd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 fminimumd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 fmaximum_magd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 fminimum_magd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 fmaximum_numd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 fminimum_numd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 fmaximum_mag_numd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 fminimum_mag_numd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 quantized64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 fmad64(_Decimal64 __x, _Decimal64 __y, _Decimal64 __z);
_Decimal64 ldexpd64(_Decimal64 __x, int __n);
_Decimal64 scalbnd64(_Decimal64 __x, int __n);
_Decimal64 scalblnd64(_Decimal64 __x, long __n);
int ilogbd64(_Decimal64 __x);
long llogbd64(_Decimal64 __x);
long lrintd64(_Decimal64 __x);
long lroundd64(_Decimal64 __x);
long long llrintd64(_Decimal64 __x);
long long llroundd64(_Decimal64 __x);
long long llquantexpd64(_Decimal64 __x);
_Decimal64 frexpd64(_Decimal64 __x, int *__e);
_Decimal64 modfd64(_Decimal64 __x, _Decimal64 *__i);
_Decimal64 nexttowardd64(_Decimal64 __x, _Decimal128 __y);
bool samequantumd64(_Decimal64 __x, _Decimal64 __y);
_Decimal64 nand64(const char *__tag);
_Decimal64 fromfpd64(_Decimal64 __x, int __rnd, unsigned int __width);
_Decimal64 ufromfpd64(_Decimal64 __x, int __rnd, unsigned int __width);
_Decimal64 fromfpxd64(_Decimal64 __x, int __rnd, unsigned int __width);
_Decimal64 ufromfpxd64(_Decimal64 __x, int __rnd, unsigned int __width);
int canonicalized64(_Decimal64 *__cx, const _Decimal64 *__x);
int totalorderd64(const _Decimal64 *__x, const _Decimal64 *__y);
int totalordermagd64(const _Decimal64 *__x, const _Decimal64 *__y);
_Decimal64 getpayloadd64(const _Decimal64 *__x);
int setpayloadd64(_Decimal64 *__res, _Decimal64 __pl);
int setpayloadsigd64(_Decimal64 *__res, _Decimal64 __pl);
void encodedecd64(_Decimal64 *restrict __enc, const _Decimal64 *restrict __x);
void decodedecd64(_Decimal64 *restrict __x, const _Decimal64 *restrict __enc);
void encodebind64(_Decimal64 *restrict __enc, const _Decimal64 *restrict __x);
void decodebind64(_Decimal64 *restrict __x, const _Decimal64 *restrict __enc);
/* _Decimal128 */
_Decimal128 cosd128(_Decimal128 __x);
_Decimal128 sind128(_Decimal128 __x);
_Decimal128 tand128(_Decimal128 __x);
_Decimal128 cospid128(_Decimal128 __x);
_Decimal128 sinpid128(_Decimal128 __x);
_Decimal128 tanpid128(_Decimal128 __x);
_Decimal128 acosd128(_Decimal128 __x);
_Decimal128 asind128(_Decimal128 __x);
_Decimal128 atand128(_Decimal128 __x);
_Decimal128 acospid128(_Decimal128 __x);
_Decimal128 asinpid128(_Decimal128 __x);
_Decimal128 atanpid128(_Decimal128 __x);
_Decimal128 atan2d128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 atan2pid128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 coshd128(_Decimal128 __x);
_Decimal128 sinhd128(_Decimal128 __x);
_Decimal128 tanhd128(_Decimal128 __x);
_Decimal128 acoshd128(_Decimal128 __x);
_Decimal128 asinhd128(_Decimal128 __x);
_Decimal128 atanhd128(_Decimal128 __x);
_Decimal128 expd128(_Decimal128 __x);
_Decimal128 exp10d128(_Decimal128 __x);
_Decimal128 exp10m1d128(_Decimal128 __x);
_Decimal128 exp2d128(_Decimal128 __x);
_Decimal128 exp2m1d128(_Decimal128 __x);
_Decimal128 expm1d128(_Decimal128 __x);
_Decimal128 logd128(_Decimal128 __x);
_Decimal128 log10d128(_Decimal128 __x);
_Decimal128 log10p1d128(_Decimal128 __x);
_Decimal128 log1pd128(_Decimal128 __x);
_Decimal128 logp1d128(_Decimal128 __x);
_Decimal128 log2d128(_Decimal128 __x);
_Decimal128 log2p1d128(_Decimal128 __x);
_Decimal128 cbrtd128(_Decimal128 __x);
_Decimal128 rsqrtd128(_Decimal128 __x);
_Decimal128 powd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 powrd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 hypotd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 pownd128(_Decimal128 __x, long long __n);
_Decimal128 rootnd128(_Decimal128 __x, long long __n);
_Decimal128 compoundnd128(_Decimal128 __x, long long __n);
_Decimal128 erfd128(_Decimal128 __x);
_Decimal128 erfcd128(_Decimal128 __x);
_Decimal128 lgammad128(_Decimal128 __x);
_Decimal128 tgammad128(_Decimal128 __x);
_Decimal128 sqrtd128(_Decimal128 __x);
_Decimal128 ceild128(_Decimal128 __x);
_Decimal128 floord128(_Decimal128 __x);
_Decimal128 nearbyintd128(_Decimal128 __x);
_Decimal128 rintd128(_Decimal128 __x);
_Decimal128 roundd128(_Decimal128 __x);
_Decimal128 roundevend128(_Decimal128 __x);
_Decimal128 truncd128(_Decimal128 __x);
_Decimal128 fabsd128(_Decimal128 __x);
_Decimal128 nextupd128(_Decimal128 __x);
_Decimal128 nextdownd128(_Decimal128 __x);
_Decimal128 quantumd128(_Decimal128 __x);
_Decimal128 logbd128(_Decimal128 __x);
_Decimal128 fmodd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 remainderd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 copysignd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 nextafterd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 fdimd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 fmaxd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 fmind128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 fmaximumd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 fminimumd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 fmaximum_magd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 fminimum_magd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 fmaximum_numd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 fminimum_numd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 fmaximum_mag_numd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 fminimum_mag_numd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 quantized128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 fmad128(_Decimal128 __x, _Decimal128 __y, _Decimal128 __z);
_Decimal128 ldexpd128(_Decimal128 __x, int __n);
_Decimal128 scalbnd128(_Decimal128 __x, int __n);
_Decimal128 scalblnd128(_Decimal128 __x, long __n);
int ilogbd128(_Decimal128 __x);
long llogbd128(_Decimal128 __x);
long lrintd128(_Decimal128 __x);
long lroundd128(_Decimal128 __x);
long long llrintd128(_Decimal128 __x);
long long llroundd128(_Decimal128 __x);
long long llquantexpd128(_Decimal128 __x);
_Decimal128 frexpd128(_Decimal128 __x, int *__e);
_Decimal128 modfd128(_Decimal128 __x, _Decimal128 *__i);
_Decimal128 nexttowardd128(_Decimal128 __x, _Decimal128 __y);
bool samequantumd128(_Decimal128 __x, _Decimal128 __y);
_Decimal128 nand128(const char *__tag);
_Decimal128 fromfpd128(_Decimal128 __x, int __rnd, unsigned int __width);
_Decimal128 ufromfpd128(_Decimal128 __x, int __rnd, unsigned int __width);
_Decimal128 fromfpxd128(_Decimal128 __x, int __rnd, unsigned int __width);
_Decimal128 ufromfpxd128(_Decimal128 __x, int __rnd, unsigned int __width);
int canonicalized128(_Decimal128 *__cx, const _Decimal128 *__x);
int totalorderd128(const _Decimal128 *__x, const _Decimal128 *__y);
int totalordermagd128(const _Decimal128 *__x, const _Decimal128 *__y);
_Decimal128 getpayloadd128(const _Decimal128 *__x);
int setpayloadd128(_Decimal128 *__res, _Decimal128 __pl);
int setpayloadsigd128(_Decimal128 *__res, _Decimal128 __pl);
void encodedecd128(_Decimal128 *restrict __enc, const _Decimal128 *restrict __x);
void decodedecd128(_Decimal128 *restrict __x, const _Decimal128 *restrict __enc);
void encodebind128(_Decimal128 *restrict __enc, const _Decimal128 *restrict __x);
void decodebind128(_Decimal128 *restrict __x, const _Decimal128 *restrict __enc);
/* the narrowing operations (7.12.14): one rounding, to the narrower type */
_Decimal32 d32addd64(_Decimal64 __x, _Decimal64 __y);
_Decimal32 d32subd64(_Decimal64 __x, _Decimal64 __y);
_Decimal32 d32muld64(_Decimal64 __x, _Decimal64 __y);
_Decimal32 d32divd64(_Decimal64 __x, _Decimal64 __y);
_Decimal32 d32fmad64(_Decimal64 __x, _Decimal64 __y, _Decimal64 __z);
_Decimal32 d32sqrtd64(_Decimal64 __x);
_Decimal32 d32addd128(_Decimal128 __x, _Decimal128 __y);
_Decimal32 d32subd128(_Decimal128 __x, _Decimal128 __y);
_Decimal32 d32muld128(_Decimal128 __x, _Decimal128 __y);
_Decimal32 d32divd128(_Decimal128 __x, _Decimal128 __y);
_Decimal32 d32fmad128(_Decimal128 __x, _Decimal128 __y, _Decimal128 __z);
_Decimal32 d32sqrtd128(_Decimal128 __x);
_Decimal64 d64addd128(_Decimal128 __x, _Decimal128 __y);
_Decimal64 d64subd128(_Decimal128 __x, _Decimal128 __y);
_Decimal64 d64muld128(_Decimal128 __x, _Decimal128 __y);
_Decimal64 d64divd128(_Decimal128 __x, _Decimal128 __y);
_Decimal64 d64fmad128(_Decimal128 __x, _Decimal128 __y, _Decimal128 __z);
_Decimal64 d64sqrtd128(_Decimal128 __x);
#endif

#endif
