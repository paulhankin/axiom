/* complex.h - complex arithmetic (C99 7.3, C11 7.3, C23 7.3), for the Axiom
   compiler's C front end. The types are the language's (_Complex); I and
   _Complex_I are a constant of type float _Complex made by the builtin
   __builtin_complex, which CMPLX, CMPLXF and CMPLXL also are (so they are
   constant expressions when their operands are). creal, cimag and conj
   (each type's) are also macros over builtins, which the compiler expands
   in line; the other functions are declared here and are the library's.
   The imaginary types are supported (7.3.1p3): imaginary and _Imaginary_I,
   the imaginary unit, a constant of type const float _Imaginary. I is
   _Complex_I: __STDC_IEC_559_COMPLEX__ is not defined (Annex G, which would
   make I _Imaginary_I, needs Annex F, which is not claimed). */
#ifndef __AXIOM_COMPLEX_H
#define __AXIOM_COMPLEX_H

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
#error "<complex.h> is a header of C99"
#endif

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_COMPLEX_H__ 202311L
#endif

#define complex _Complex
#define _Complex_I (__builtin_complex(0.0f, 1.0f))
#define imaginary _Imaginary
#define _Imaginary_I __builtin_axiom_imaginary_i
#define I _Complex_I

#if __STDC_VERSION__ >= 201112L
#define CMPLX(x, y) __builtin_complex((double)(x), (double)(y))
#define CMPLXF(x, y) __builtin_complex((float)(x), (float)(y))
#define CMPLXL(x, y) __builtin_complex((long double)(x), (long double)(y))
#endif

/* double _Complex */
double _Complex cacos(double _Complex __z);
double _Complex casin(double _Complex __z);
double _Complex catan(double _Complex __z);
double _Complex ccos(double _Complex __z);
double _Complex csin(double _Complex __z);
double _Complex ctan(double _Complex __z);
double _Complex cacosh(double _Complex __z);
double _Complex casinh(double _Complex __z);
double _Complex catanh(double _Complex __z);
double _Complex ccosh(double _Complex __z);
double _Complex csinh(double _Complex __z);
double _Complex ctanh(double _Complex __z);
double _Complex cexp(double _Complex __z);
double _Complex clog(double _Complex __z);
double cabs(double _Complex __z);
double _Complex cpow(double _Complex __x, double _Complex __y);
double _Complex csqrt(double _Complex __z);
double carg(double _Complex __z);
double cimag(double _Complex __z);
double _Complex conj(double _Complex __z);
double _Complex cproj(double _Complex __z);
double creal(double _Complex __z);

/* float _Complex */
float _Complex cacosf(float _Complex __z);
float _Complex casinf(float _Complex __z);
float _Complex catanf(float _Complex __z);
float _Complex ccosf(float _Complex __z);
float _Complex csinf(float _Complex __z);
float _Complex ctanf(float _Complex __z);
float _Complex cacoshf(float _Complex __z);
float _Complex casinhf(float _Complex __z);
float _Complex catanhf(float _Complex __z);
float _Complex ccoshf(float _Complex __z);
float _Complex csinhf(float _Complex __z);
float _Complex ctanhf(float _Complex __z);
float _Complex cexpf(float _Complex __z);
float _Complex clogf(float _Complex __z);
float cabsf(float _Complex __z);
float _Complex cpowf(float _Complex __x, float _Complex __y);
float _Complex csqrtf(float _Complex __z);
float cargf(float _Complex __z);
float cimagf(float _Complex __z);
float _Complex conjf(float _Complex __z);
float _Complex cprojf(float _Complex __z);
float crealf(float _Complex __z);

/* long double _Complex */
long double _Complex cacosl(long double _Complex __z);
long double _Complex casinl(long double _Complex __z);
long double _Complex catanl(long double _Complex __z);
long double _Complex ccosl(long double _Complex __z);
long double _Complex csinl(long double _Complex __z);
long double _Complex ctanl(long double _Complex __z);
long double _Complex cacoshl(long double _Complex __z);
long double _Complex casinhl(long double _Complex __z);
long double _Complex catanhl(long double _Complex __z);
long double _Complex ccoshl(long double _Complex __z);
long double _Complex csinhl(long double _Complex __z);
long double _Complex ctanhl(long double _Complex __z);
long double _Complex cexpl(long double _Complex __z);
long double _Complex clogl(long double _Complex __z);
long double cabsl(long double _Complex __z);
long double _Complex cpowl(long double _Complex __x, long double _Complex __y);
long double _Complex csqrtl(long double _Complex __z);
long double cargl(long double _Complex __z);
long double cimagl(long double _Complex __z);
long double _Complex conjl(long double _Complex __z);
long double _Complex cprojl(long double _Complex __z);
long double creall(long double _Complex __z);

/* (in line: the builtins take the operand converted to the function's type) */
#define creal(z) __builtin_creal((double _Complex)(z))
#define cimag(z) __builtin_cimag((double _Complex)(z))
#define conj(z) __builtin_conj((double _Complex)(z))
#define crealf(z) __builtin_creal((float _Complex)(z))
#define cimagf(z) __builtin_cimag((float _Complex)(z))
#define conjf(z) __builtin_conj((float _Complex)(z))
#define creall(z) __builtin_creal((long double _Complex)(z))
#define cimagl(z) __builtin_cimag((long double _Complex)(z))
#define conjl(z) __builtin_conj((long double _Complex)(z))

#endif
