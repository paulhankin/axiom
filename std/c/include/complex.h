/* complex.h - complex arithmetic (C99 7.3, C11 7.3, C23 7.3), for the Axiom
   compiler's C front end. The types are the language's (_Complex); I and
   _Complex_I are a constant of type float _Complex made by the builtin
   __builtin_complex, which CMPLX, CMPLXF and CMPLXL also are (so they are
   constant expressions when their operands are). creal, cimag and conj
   (each type's) are also macros over builtins, which the compiler expands
   in line; the other functions are declared here and are the library's.
   The imaginary types are supported (7.3.1p3): imaginary and _Imaginary_I,
   the imaginary unit, a constant of type const float _Imaginary. I is
   _Imaginary_I where Annex G is claimed (__STDC_IEC_559_COMPLEX__: G.6),
   else _Complex_I. */
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
#ifdef __STDC_IEC_559_COMPLEX__
#define I _Imaginary_I
#else
#define I _Complex_I
#endif

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

#if defined(__STDC_WANT_IEC_60559_TYPES_EXT__)
/* C23 Annex H.11 (ISO/IEC TS 18661-3): the complex functions of the
   interchange and extended types: _Float16's and _Float128's are the
   library's (std/c/lib/math/complexf16.c, complexf128.c); those of
   _Float32, _Float64 and _Float32x are the float and double functions,
   and _Float64x's the long double functions where long double is wider
   than double, else _Float128's: macros that name them (as <math.h>'s
   real functions of those types are). */
_Float16 _Complex cacosf16(_Float16 _Complex __z);
_Float16 _Complex casinf16(_Float16 _Complex __z);
_Float16 _Complex catanf16(_Float16 _Complex __z);
_Float16 _Complex ccosf16(_Float16 _Complex __z);
_Float16 _Complex csinf16(_Float16 _Complex __z);
_Float16 _Complex ctanf16(_Float16 _Complex __z);
_Float16 _Complex cacoshf16(_Float16 _Complex __z);
_Float16 _Complex casinhf16(_Float16 _Complex __z);
_Float16 _Complex catanhf16(_Float16 _Complex __z);
_Float16 _Complex ccoshf16(_Float16 _Complex __z);
_Float16 _Complex csinhf16(_Float16 _Complex __z);
_Float16 _Complex ctanhf16(_Float16 _Complex __z);
_Float16 _Complex cexpf16(_Float16 _Complex __z);
_Float16 _Complex clogf16(_Float16 _Complex __z);
_Float16 cabsf16(_Float16 _Complex __z);
_Float16 _Complex cpowf16(_Float16 _Complex __x, _Float16 _Complex __y);
_Float16 _Complex csqrtf16(_Float16 _Complex __z);
_Float16 cargf16(_Float16 _Complex __z);
_Float16 cimagf16(_Float16 _Complex __z);
_Float16 _Complex conjf16(_Float16 _Complex __z);
_Float16 _Complex cprojf16(_Float16 _Complex __z);
_Float16 crealf16(_Float16 _Complex __z);
_Float128 _Complex cacosf128(_Float128 _Complex __z);
_Float128 _Complex casinf128(_Float128 _Complex __z);
_Float128 _Complex catanf128(_Float128 _Complex __z);
_Float128 _Complex ccosf128(_Float128 _Complex __z);
_Float128 _Complex csinf128(_Float128 _Complex __z);
_Float128 _Complex ctanf128(_Float128 _Complex __z);
_Float128 _Complex cacoshf128(_Float128 _Complex __z);
_Float128 _Complex casinhf128(_Float128 _Complex __z);
_Float128 _Complex catanhf128(_Float128 _Complex __z);
_Float128 _Complex ccoshf128(_Float128 _Complex __z);
_Float128 _Complex csinhf128(_Float128 _Complex __z);
_Float128 _Complex ctanhf128(_Float128 _Complex __z);
_Float128 _Complex cexpf128(_Float128 _Complex __z);
_Float128 _Complex clogf128(_Float128 _Complex __z);
_Float128 cabsf128(_Float128 _Complex __z);
_Float128 _Complex cpowf128(_Float128 _Complex __x, _Float128 _Complex __y);
_Float128 _Complex csqrtf128(_Float128 _Complex __z);
_Float128 cargf128(_Float128 _Complex __z);
_Float128 cimagf128(_Float128 _Complex __z);
_Float128 _Complex conjf128(_Float128 _Complex __z);
_Float128 _Complex cprojf128(_Float128 _Complex __z);
_Float128 crealf128(_Float128 _Complex __z);
#define cacosf32 cacosf
#define casinf32 casinf
#define catanf32 catanf
#define ccosf32 ccosf
#define csinf32 csinf
#define ctanf32 ctanf
#define cacoshf32 cacoshf
#define casinhf32 casinhf
#define catanhf32 catanhf
#define ccoshf32 ccoshf
#define csinhf32 csinhf
#define ctanhf32 ctanhf
#define cexpf32 cexpf
#define clogf32 clogf
#define cabsf32 cabsf
#define cpowf32 cpowf
#define csqrtf32 csqrtf
#define cargf32 cargf
#define cimagf32 cimagf
#define conjf32 conjf
#define cprojf32 cprojf
#define crealf32 crealf
#define cacosf64 cacos
#define casinf64 casin
#define catanf64 catan
#define ccosf64 ccos
#define csinf64 csin
#define ctanf64 ctan
#define cacoshf64 cacosh
#define casinhf64 casinh
#define catanhf64 catanh
#define ccoshf64 ccosh
#define csinhf64 csinh
#define ctanhf64 ctanh
#define cexpf64 cexp
#define clogf64 clog
#define cabsf64 cabs
#define cpowf64 cpow
#define csqrtf64 csqrt
#define cargf64 carg
#define cimagf64 cimag
#define conjf64 conj
#define cprojf64 cproj
#define crealf64 creal
#define cacosf32x cacos
#define casinf32x casin
#define catanf32x catan
#define ccosf32x ccos
#define csinf32x csin
#define ctanf32x ctan
#define cacoshf32x cacosh
#define casinhf32x casinh
#define catanhf32x catanh
#define ccoshf32x ccosh
#define csinhf32x csinh
#define ctanhf32x ctanh
#define cexpf32x cexp
#define clogf32x clog
#define cabsf32x cabs
#define cpowf32x cpow
#define csqrtf32x csqrt
#define cargf32x carg
#define cimagf32x cimag
#define conjf32x conj
#define cprojf32x cproj
#define crealf32x creal
#if __AXIOM_LDBL_WIDTH__ != 64
#define cacosf64x cacosl
#define casinf64x casinl
#define catanf64x catanl
#define ccosf64x ccosl
#define csinf64x csinl
#define ctanf64x ctanl
#define cacoshf64x cacoshl
#define casinhf64x casinhl
#define catanhf64x catanhl
#define ccoshf64x ccoshl
#define csinhf64x csinhl
#define ctanhf64x ctanhl
#define cexpf64x cexpl
#define clogf64x clogl
#define cabsf64x cabsl
#define cpowf64x cpowl
#define csqrtf64x csqrtl
#define cargf64x cargl
#define cimagf64x cimagl
#define conjf64x conjl
#define cprojf64x cprojl
#define crealf64x creall
#else
#define cacosf64x cacosf128
#define casinf64x casinf128
#define catanf64x catanf128
#define ccosf64x ccosf128
#define csinf64x csinf128
#define ctanf64x ctanf128
#define cacoshf64x cacoshf128
#define casinhf64x casinhf128
#define catanhf64x catanhf128
#define ccoshf64x ccoshf128
#define csinhf64x csinhf128
#define ctanhf64x ctanhf128
#define cexpf64x cexpf128
#define clogf64x clogf128
#define cabsf64x cabsf128
#define cpowf64x cpowf128
#define csqrtf64x csqrtf128
#define cargf64x cargf128
#define cimagf64x cimagf128
#define conjf64x conjf128
#define cprojf64x cprojf128
#define crealf64x crealf128
#endif
#define crealf16(z) __builtin_creal((_Float16 _Complex)(z))
#define cimagf16(z) __builtin_cimag((_Float16 _Complex)(z))
#define conjf16(z) __builtin_conj((_Float16 _Complex)(z))
#define crealf128(z) __builtin_creal((_Float128 _Complex)(z))
#define cimagf128(z) __builtin_cimag((_Float128 _Complex)(z))
#define conjf128(z) __builtin_conj((_Float128 _Complex)(z))
#endif

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
