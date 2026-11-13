/* m_ldouble.c - the long double forms of <math.h>'s functions (C17 7.12)
   but the exact ones of a wide long double: where long double is double
   (Windows, macOS, --c-long-double=64), the double functions; where it is
   wider, the elementary functions in its precision (std/c/rt/math_ld.ax),
   erfl, erfcl, lgammal and tgammal too; C23's are m_c23.c's. The exact
   functions of a wide long double are m_ldwide.c's. */
#if __STDC_VERSION__ >= 199901L

#if __AXIOM_LDBL_WIDTH__ != 64
/* the elementary functions in the format's precision: std/c/rt/math_ld.ax's
   kernels, by address, with the format (0 binary128, 1 the x87's) */
#define F (__AXIOM_LDBL_WIDTH__ == 80)
void __axc_ld1(unsigned long long op, unsigned long long f, long double *r, const long double *x);
void __axc_ld2(unsigned long long op, unsigned long long f, long double *r, const long double *x,
               const long double *y);
void __axc_ld_fma(unsigned long long f, long double *r, const long double *x, const long double *y,
                  const long double *z);
#define LD1(name, op) \
    long double name(long double x) \
    { \
        long double r; \
        __axc_ld1(op, F, &r, &x); \
        return r; \
    }
#define LD2(name, op) \
    long double name(long double x, long double y) \
    { \
        long double r; \
        __axc_ld2(op, F, &r, &x, &y); \
        return r; \
    }
LD1(expl, 0)
LD1(exp2l, 1)
LD1(expm1l, 3)
LD1(logl, 4)
LD1(log2l, 5)
LD1(log10l, 6)
LD1(log1pl, 7)
LD1(sinl, 8)
LD1(cosl, 9)
LD1(tanl, 10)
LD1(atanl, 11)
LD1(sinhl, 12)
LD1(coshl, 13)
LD1(tanhl, 14)
LD1(cbrtl, 15)
LD1(asinl, 16)
LD1(acosl, 17)
LD1(asinhl, 18)
LD1(acoshl, 19)
LD1(atanhl, 20)
LD2(powl, 0)
LD2(atan2l, 1)
LD2(hypotl, 2)
long double fmal(long double x, long double y, long double z)
{
    long double r;
    __axc_ld_fma(F, &r, &x, &y, &z);
    return r;
}
LD1(exp10l, 2)
#else
double acos(double);
long double acosl(long double x) { return (long double)acos((double)x); }
double asin(double);
long double asinl(long double x) { return (long double)asin((double)x); }
double atan(double);
long double atanl(long double x) { return (long double)atan((double)x); }
double cos(double);
long double cosl(long double x) { return (long double)cos((double)x); }
double sin(double);
long double sinl(long double x) { return (long double)sin((double)x); }
double tan(double);
long double tanl(long double x) { return (long double)tan((double)x); }
double cosh(double);
long double coshl(long double x) { return (long double)cosh((double)x); }
double sinh(double);
long double sinhl(long double x) { return (long double)sinh((double)x); }
double tanh(double);
long double tanhl(long double x) { return (long double)tanh((double)x); }
double exp(double);
long double expl(long double x) { return (long double)exp((double)x); }
double log(double);
long double logl(long double x) { return (long double)log((double)x); }
double log10(double);
long double log10l(long double x) { return (long double)log10((double)x); }
double acosh(double);
long double acoshl(long double x) { return (long double)acosh((double)x); }
double asinh(double);
long double asinhl(long double x) { return (long double)asinh((double)x); }
double atanh(double);
long double atanhl(long double x) { return (long double)atanh((double)x); }
double exp2(double);
long double exp2l(long double x) { return (long double)exp2((double)x); }
double expm1(double);
long double expm1l(long double x) { return (long double)expm1((double)x); }
double log1p(double);
long double log1pl(long double x) { return (long double)log1p((double)x); }
double log2(double);
long double log2l(long double x) { return (long double)log2((double)x); }
double cbrt(double);
long double cbrtl(long double x) { return (long double)cbrt((double)x); }
double atan2(double, double);
long double atan2l(long double x, long double y) { return (long double)atan2((double)x, (double)y); }
double pow(double, double);
long double powl(long double x, long double y) { return (long double)pow((double)x, (double)y); }
double hypot(double, double);
long double hypotl(long double x, long double y) { return (long double)hypot((double)x, (double)y); }
double fma(double, double, double);
long double fmal(long double x, long double y, long double z) { return (long double)fma((double)x, (double)y, (double)z); }
double exp10(double);
long double exp10l(long double x) { return (long double)exp10((double)x); }
#endif
#if __AXIOM_LDBL_WIDTH__ != 64
/* the error and gamma functions in the format's precision (math_ld.ax) */
void __axc_ld_sf(unsigned long long op, unsigned long long f, long double *r, const long double *x);
#define LDSF(name, op) \
    long double name(long double x) \
    { \
        long double r; \
        __axc_ld_sf(op, F, &r, &x); \
        return r; \
    }
LDSF(erfl, 0)
LDSF(erfcl, 1)
LDSF(lgammal, 2)
LDSF(tgammal, 3)
#else
double erf(double);
long double erfl(long double x) { return (long double)erf((double)x); }
double erfc(double);
long double erfcl(long double x) { return (long double)erfc((double)x); }
double lgamma(double);
long double lgammal(long double x) { return (long double)lgamma((double)x); }
double tgamma(double);
long double tgammal(long double x) { return (long double)tgamma((double)x); }
#endif

#else
typedef int __axc_ldouble_c89;
#endif
