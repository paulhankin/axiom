/* complexf16.c - <complex.h>'s _Float16 functions (C23 Annex H.11:
   cexpf16 ...): the double functions' values, each part rounded once to
   binary16 (the double parts within a few ulps of double: so within half an
   ulp of binary16 and a tiny fraction more). */
#if __STDC_VERSION__ >= 199901L
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <complex.h>
#include <math.h>

#define W(z) __builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z))
#define H(w) __builtin_complex((_Float16)__builtin_creal(w), (_Float16)__builtin_cimag(w))
#define CX1(name) \
    _Float16 _Complex name##f16(_Float16 _Complex z) { return H(name(W(z))); }
CX1(cacos) CX1(casin) CX1(catan) CX1(ccos) CX1(csin) CX1(ctan) CX1(cacosh) CX1(casinh) CX1(catanh) CX1(ccosh)
CX1(csinh) CX1(ctanh) CX1(cexp) CX1(clog) CX1(csqrt) CX1(cproj)

_Float16 _Complex cpowf16(_Float16 _Complex z, _Float16 _Complex w) { return H(cpow(W(z), W(w))); }
_Float16 cabsf16(_Float16 _Complex z) { return (_Float16)cabs(W(z)); }
_Float16 cargf16(_Float16 _Complex z) { return (_Float16)carg(W(z)); }
_Float16 (crealf16)(_Float16 _Complex z) { return __builtin_creal(z); }
_Float16 (cimagf16)(_Float16 _Complex z) { return __builtin_cimag(z); }
_Float16 _Complex (conjf16)(_Float16 _Complex z) { return __builtin_conj(z); }
#else
typedef int __axc_complexf16_c89;
#endif
