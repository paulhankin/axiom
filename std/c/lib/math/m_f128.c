/* m_f128.c - <math.h>'s _Float128 functions (ISO/IEC TS 18661-3, C23
   Annex H: the f128 suffix), binary128 on every target whatever long
   double is: the exact ones are m_ldwide.c's, instantiated for _Float128
   (computed on binary128's bits, with its arithmetic in software:
   std/c/rt/ldouble.ax); the elementary ones std/c/rt/math_ld.ax's kernels
   in binary128 (format 0), as the long double functions of a binary128
   long double are (m_ldouble.c). */
#if __STDC_VERSION__ >= 199901L
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#define LDT _Float128
#define FN(name) name##f128
#define K(c) c##F128
#define X87 0
#define P 113
#define LD_NEXTTOWARD 0
#include "m_ldwide.c"

void __axc_ld1(unsigned long long op, unsigned long long f, void *r, const void *x);
void __axc_ld2(unsigned long long op, unsigned long long f, void *r, const void *x, const void *y);
void __axc_ld_fma(unsigned long long f, void *r, const void *x, const void *y, const void *z);
#define Q1(name, op) \
    _Float128 name(_Float128 x) \
    { \
        _Float128 r; \
        __axc_ld1(op, 0, &r, &x); \
        return r; \
    }
#define Q2(name, op) \
    _Float128 name(_Float128 x, _Float128 y) \
    { \
        _Float128 r; \
        __axc_ld2(op, 0, &r, &x, &y); \
        return r; \
    }
Q1(expf128, 0)
Q1(exp2f128, 1)
Q1(exp10f128, 2)
Q1(expm1f128, 3)
Q1(logf128, 4)
Q1(log2f128, 5)
Q1(log10f128, 6)
Q1(log1pf128, 7)
Q1(sinf128, 8)
Q1(cosf128, 9)
Q1(tanf128, 10)
Q1(atanf128, 11)
Q1(sinhf128, 12)
Q1(coshf128, 13)
Q1(tanhf128, 14)
Q1(cbrtf128, 15)
Q1(asinf128, 16)
Q1(acosf128, 17)
Q1(asinhf128, 18)
Q1(acoshf128, 19)
Q1(atanhf128, 20)
Q2(powf128, 0)
Q2(atan2f128, 1)
Q2(hypotf128, 2)
_Float128 fmaf128(_Float128 x, _Float128 y, _Float128 z)
{
    _Float128 r;
    __axc_ld_fma(0, &r, &x, &y, &z);
    return r;
}
/* (math_ld.ax's, in binary128's precision) */
void __axc_ld_sf(unsigned long long op, unsigned long long f, _Float128 *r, const _Float128 *x);
#define QSF(name, op) \
    _Float128 name(_Float128 x) \
    { \
        _Float128 r; \
        __axc_ld_sf(op, 0, &r, &x); \
        return r; \
    }
QSF(erff128, 0)
QSF(erfcf128, 1)
QSF(lgammaf128, 2)
QSF(tgammaf128, 3)
#else
typedef int __axc_f128_c89;
#endif
