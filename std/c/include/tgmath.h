/* tgmath.h - type-generic math (C99 7.22, C11 7.25, C17 7.25, C23 7.27), for
   the Axiom compiler's C library. Each macro is a call of the compiler's
   builtin __builtin_axiom_tg (cmd/axiom/src/c/csema_fp.ax), which works in
   every standard from C99 on (where _Generic is not available): it calls the
   float, double or long double function, or the complex one, that the types
   of the generic arguments choose (7.25p3: any complex argument makes it
   complex; then long double if any is long double, else double if any is
   double or an integer, else float), each argument evaluated once. */
#ifndef __AXIOM_TGMATH_H
#define __AXIOM_TGMATH_H

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
#error "<tgmath.h> is a header of C99"
#endif

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_TGMATH_H__ 202311L
#endif

#include <math.h>
#include <complex.h>

/* <math.h>'s with a complex counterpart (fabs's is cabs) */
#define acos(x) __builtin_axiom_tg(1, acosf, acos, acosl, cacosf, cacos, cacosl, x)
#define asin(x) __builtin_axiom_tg(1, asinf, asin, asinl, casinf, casin, casinl, x)
#define atan(x) __builtin_axiom_tg(1, atanf, atan, atanl, catanf, catan, catanl, x)
#define acosh(x) __builtin_axiom_tg(1, acoshf, acosh, acoshl, cacoshf, cacosh, cacoshl, x)
#define asinh(x) __builtin_axiom_tg(1, asinhf, asinh, asinhl, casinhf, casinh, casinhl, x)
#define atanh(x) __builtin_axiom_tg(1, atanhf, atanh, atanhl, catanhf, catanh, catanhl, x)
#define cos(x) __builtin_axiom_tg(1, cosf, cos, cosl, ccosf, ccos, ccosl, x)
#define sin(x) __builtin_axiom_tg(1, sinf, sin, sinl, csinf, csin, csinl, x)
#define tan(x) __builtin_axiom_tg(1, tanf, tan, tanl, ctanf, ctan, ctanl, x)
#define cosh(x) __builtin_axiom_tg(1, coshf, cosh, coshl, ccoshf, ccosh, ccoshl, x)
#define sinh(x) __builtin_axiom_tg(1, sinhf, sinh, sinhl, csinhf, csinh, csinhl, x)
#define tanh(x) __builtin_axiom_tg(1, tanhf, tanh, tanhl, ctanhf, ctanh, ctanhl, x)
#define exp(x) __builtin_axiom_tg(1, expf, exp, expl, cexpf, cexp, cexpl, x)
#define log(x) __builtin_axiom_tg(1, logf, log, logl, clogf, clog, clogl, x)
#define sqrt(x) __builtin_axiom_tg(1, sqrtf, sqrt, sqrtl, csqrtf, csqrt, csqrtl, x)
#define fabs(x) __builtin_axiom_tg(1, fabsf, fabs, fabsl, cabsf, cabs, cabsl, x)
#define pow(x, y) __builtin_axiom_tg(2, powf, pow, powl, cpowf, cpow, cpowl, x, y)

/* <math.h>'s alone */
#define cbrt(x) __builtin_axiom_tg(1, cbrtf, cbrt, cbrtl, 0, 0, 0, x)
#define ceil(x) __builtin_axiom_tg(1, ceilf, ceil, ceill, 0, 0, 0, x)
#define erf(x) __builtin_axiom_tg(1, erff, erf, erfl, 0, 0, 0, x)
#define erfc(x) __builtin_axiom_tg(1, erfcf, erfc, erfcl, 0, 0, 0, x)
#define exp2(x) __builtin_axiom_tg(1, exp2f, exp2, exp2l, 0, 0, 0, x)
#define expm1(x) __builtin_axiom_tg(1, expm1f, expm1, expm1l, 0, 0, 0, x)
#define floor(x) __builtin_axiom_tg(1, floorf, floor, floorl, 0, 0, 0, x)
#define ilogb(x) __builtin_axiom_tg(1, ilogbf, ilogb, ilogbl, 0, 0, 0, x)
#define lgamma(x) __builtin_axiom_tg(1, lgammaf, lgamma, lgammal, 0, 0, 0, x)
#define llrint(x) __builtin_axiom_tg(1, llrintf, llrint, llrintl, 0, 0, 0, x)
#define llround(x) __builtin_axiom_tg(1, llroundf, llround, llroundl, 0, 0, 0, x)
#define log10(x) __builtin_axiom_tg(1, log10f, log10, log10l, 0, 0, 0, x)
#define log1p(x) __builtin_axiom_tg(1, log1pf, log1p, log1pl, 0, 0, 0, x)
#define log2(x) __builtin_axiom_tg(1, log2f, log2, log2l, 0, 0, 0, x)
#define logb(x) __builtin_axiom_tg(1, logbf, logb, logbl, 0, 0, 0, x)
#define lrint(x) __builtin_axiom_tg(1, lrintf, lrint, lrintl, 0, 0, 0, x)
#define lround(x) __builtin_axiom_tg(1, lroundf, lround, lroundl, 0, 0, 0, x)
#define nearbyint(x) __builtin_axiom_tg(1, nearbyintf, nearbyint, nearbyintl, 0, 0, 0, x)
#define rint(x) __builtin_axiom_tg(1, rintf, rint, rintl, 0, 0, 0, x)
#define round(x) __builtin_axiom_tg(1, roundf, round, roundl, 0, 0, 0, x)
#define tgamma(x) __builtin_axiom_tg(1, tgammaf, tgamma, tgammal, 0, 0, 0, x)
#define trunc(x) __builtin_axiom_tg(1, truncf, trunc, truncl, 0, 0, 0, x)
#define atan2(x, y) __builtin_axiom_tg(2, atan2f, atan2, atan2l, 0, 0, 0, x, y)
#define copysign(x, y) __builtin_axiom_tg(2, copysignf, copysign, copysignl, 0, 0, 0, x, y)
#define fdim(x, y) __builtin_axiom_tg(2, fdimf, fdim, fdiml, 0, 0, 0, x, y)
#define fmax(x, y) __builtin_axiom_tg(2, fmaxf, fmax, fmaxl, 0, 0, 0, x, y)
#define fmin(x, y) __builtin_axiom_tg(2, fminf, fmin, fminl, 0, 0, 0, x, y)
#define fmod(x, y) __builtin_axiom_tg(2, fmodf, fmod, fmodl, 0, 0, 0, x, y)
#define hypot(x, y) __builtin_axiom_tg(2, hypotf, hypot, hypotl, 0, 0, 0, x, y)
#define nextafter(x, y) __builtin_axiom_tg(2, nextafterf, nextafter, nextafterl, 0, 0, 0, x, y)
#define remainder(x, y) __builtin_axiom_tg(2, remainderf, remainder, remainderl, 0, 0, 0, x, y)
#define fma(x, y, z) __builtin_axiom_tg(3, fmaf, fma, fmal, 0, 0, 0, x, y, z)
#define frexp(x, e) __builtin_axiom_tg(1, frexpf, frexp, frexpl, 0, 0, 0, x, e)
#define ldexp(x, e) __builtin_axiom_tg(1, ldexpf, ldexp, ldexpl, 0, 0, 0, x, e)
#define scalbn(x, n) __builtin_axiom_tg(1, scalbnf, scalbn, scalbnl, 0, 0, 0, x, n)
#define scalbln(x, n) __builtin_axiom_tg(1, scalblnf, scalbln, scalblnl, 0, 0, 0, x, n)
#define nexttoward(x, y) __builtin_axiom_tg(1, nexttowardf, nexttoward, nexttowardl, 0, 0, 0, x, y)
#define remquo(x, y, q) __builtin_axiom_tg(2, remquof, remquo, remquol, 0, 0, 0, x, y, q)

/* <complex.h>'s alone: a real argument calls the function of its type's complex type */
#undef cimag
#undef conj
#undef creal
#define carg(z) __builtin_axiom_tg(1, 0, 0, 0, cargf, carg, cargl, z)
#define cimag(z) __builtin_axiom_tg(1, 0, 0, 0, cimagf, cimag, cimagl, z)
#define conj(z) __builtin_axiom_tg(1, 0, 0, 0, conjf, conj, conjl, z)
#define cproj(z) __builtin_axiom_tg(1, 0, 0, 0, cprojf, cproj, cprojl, z)
#define creal(z) __builtin_axiom_tg(1, 0, 0, 0, crealf, creal, creall, z)

#if __STDC_VERSION__ >= 202311L
#define exp10(x) __builtin_axiom_tg(1, exp10f, exp10, exp10l, 0, 0, 0, x)
#define roundeven(x) __builtin_axiom_tg(1, roundevenf, roundeven, roundevenl, 0, 0, 0, x)
#define nextup(x) __builtin_axiom_tg(1, nextupf, nextup, nextupl, 0, 0, 0, x)
#define nextdown(x) __builtin_axiom_tg(1, nextdownf, nextdown, nextdownl, 0, 0, 0, x)
#define sinpi(x) __builtin_axiom_tg(1, sinpif, sinpi, sinpil, 0, 0, 0, x)
#define cospi(x) __builtin_axiom_tg(1, cospif, cospi, cospil, 0, 0, 0, x)
#define fmaximum(x, y) __builtin_axiom_tg(2, fmaximumf, fmaximum, fmaximuml, 0, 0, 0, x, y)
#define fminimum(x, y) __builtin_axiom_tg(2, fminimumf, fminimum, fminimuml, 0, 0, 0, x, y)
#define fmaximum_mag(x, y) __builtin_axiom_tg(2, fmaximum_magf, fmaximum_mag, fmaximum_magl, 0, 0, 0, x, y)
#define fminimum_mag(x, y) __builtin_axiom_tg(2, fminimum_magf, fminimum_mag, fminimum_magl, 0, 0, 0, x, y)
#define fmaximum_num(x, y) __builtin_axiom_tg(2, fmaximum_numf, fmaximum_num, fmaximum_numl, 0, 0, 0, x, y)
#define fminimum_num(x, y) __builtin_axiom_tg(2, fminimum_numf, fminimum_num, fminimum_numl, 0, 0, 0, x, y)
#define fmaximum_mag_num(x, y) __builtin_axiom_tg(2, fmaximum_mag_numf, fmaximum_mag_num, fmaximum_mag_numl, 0, 0, 0, x, y)
#define fminimum_mag_num(x, y) __builtin_axiom_tg(2, fminimum_mag_numf, fminimum_mag_num, fminimum_mag_numl, 0, 0, 0, x, y)
#define exp2m1(x) __builtin_axiom_tg(1, exp2m1f, exp2m1, exp2m1l, 0, 0, 0, x)
#define exp10m1(x) __builtin_axiom_tg(1, exp10m1f, exp10m1, exp10m1l, 0, 0, 0, x)
#define log2p1(x) __builtin_axiom_tg(1, log2p1f, log2p1, log2p1l, 0, 0, 0, x)
#define log10p1(x) __builtin_axiom_tg(1, log10p1f, log10p1, log10p1l, 0, 0, 0, x)
#define logp1(x) __builtin_axiom_tg(1, logp1f, logp1, logp1l, 0, 0, 0, x)
#define rsqrt(x) __builtin_axiom_tg(1, rsqrtf, rsqrt, rsqrtl, 0, 0, 0, x)
#define tanpi(x) __builtin_axiom_tg(1, tanpif, tanpi, tanpil, 0, 0, 0, x)
#define asinpi(x) __builtin_axiom_tg(1, asinpif, asinpi, asinpil, 0, 0, 0, x)
#define acospi(x) __builtin_axiom_tg(1, acospif, acospi, acospil, 0, 0, 0, x)
#define atanpi(x) __builtin_axiom_tg(1, atanpif, atanpi, atanpil, 0, 0, 0, x)
#define llogb(x) __builtin_axiom_tg(1, llogbf, llogb, llogbl, 0, 0, 0, x)
#define atan2pi(x, y) __builtin_axiom_tg(2, atan2pif, atan2pi, atan2pil, 0, 0, 0, x, y)
#define powr(x, y) __builtin_axiom_tg(2, powrf, powr, powrl, 0, 0, 0, x, y)
#define rootn(x, n) __builtin_axiom_tg(1, rootnf, rootn, rootnl, 0, 0, 0, x, n)
#define pown(x, n) __builtin_axiom_tg(1, pownf, pown, pownl, 0, 0, 0, x, n)
#define compoundn(x, n) __builtin_axiom_tg(1, compoundnf, compoundn, compoundnl, 0, 0, 0, x, n)
#define fromfp(x, r, w) __builtin_axiom_tg(1, fromfpf, fromfp, fromfpl, 0, 0, 0, x, r, w)
#define ufromfp(x, r, w) __builtin_axiom_tg(1, ufromfpf, ufromfp, ufromfpl, 0, 0, 0, x, r, w)
#define fromfpx(x, r, w) __builtin_axiom_tg(1, fromfpxf, fromfpx, fromfpxl, 0, 0, 0, x, r, w)
#define ufromfpx(x, r, w) __builtin_axiom_tg(1, ufromfpxf, ufromfpx, ufromfpxl, 0, 0, 0, x, r, w)
/* the narrowing macros (7.27p7): the long double function if an argument is
   long double; else fadd ... of double (d...: daddl of the exact conversions) */
#define fadd(x, y) __builtin_axiom_tg(2, fadd, fadd, faddl, 0, 0, 0, x, y)
#define dadd(x, y) __builtin_axiom_tg(2, daddl, daddl, daddl, 0, 0, 0, x, y)
#define fsub(x, y) __builtin_axiom_tg(2, fsub, fsub, fsubl, 0, 0, 0, x, y)
#define dsub(x, y) __builtin_axiom_tg(2, dsubl, dsubl, dsubl, 0, 0, 0, x, y)
#define fmul(x, y) __builtin_axiom_tg(2, fmul, fmul, fmull, 0, 0, 0, x, y)
#define dmul(x, y) __builtin_axiom_tg(2, dmull, dmull, dmull, 0, 0, 0, x, y)
#define fdiv(x, y) __builtin_axiom_tg(2, fdiv, fdiv, fdivl, 0, 0, 0, x, y)
#define ddiv(x, y) __builtin_axiom_tg(2, ddivl, ddivl, ddivl, 0, 0, 0, x, y)
#define ffma(x, y, z) __builtin_axiom_tg(3, ffma, ffma, ffmal, 0, 0, 0, x, y, z)
#define dfma(x, y, z) __builtin_axiom_tg(3, dfmal, dfmal, dfmal, 0, 0, 0, x, y, z)
#define fsqrt(x) __builtin_axiom_tg(1, fsqrt, fsqrt, fsqrtl, 0, 0, 0, x)
#define dsqrt(x) __builtin_axiom_tg(1, dsqrtl, dsqrtl, dsqrtl, 0, 0, 0, x)
#endif

#endif
