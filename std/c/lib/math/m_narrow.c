/* m_narrow.c - the long double forms of C23's narrowing operations
   (7.12.14, F.10.11): faddl, fsubl, fmull, fdivl, ffmal, fsqrtl to float and
   daddl, dsubl, dmull, ddivl, dfmal, dsqrtl to double, each correctly
   rounded once in the current rounding direction. Where long double is
   double, the float ones are the double ones' (fadd ...) and the double ones
   are the operations themselves; where it is wider, the exact operation
   and the rounding of std/c/rt/math_narrow.ax on the format's bits. */
#if __STDC_VERSION__ >= 199901L
#include <math.h>

float fadd(double, double), fsub(double, double), fmul(double, double), fdiv(double, double);
float ffma(double, double, double), fsqrt(double);

#if __AXIOM_LDBL_WIDTH__ != 64
#define FMT (__AXIOM_LDBL_WIDTH__ == 80)
unsigned long long __axc_narrow(unsigned long long op, unsigned long long f, unsigned long long dbl,
                                const long double *x, const long double *y, const long double *z);

static float to_f(unsigned long long b)
{
    union { unsigned int u; float f; } v;
    v.u = (unsigned int)b;
    return v.f;
}

static double to_d(unsigned long long b)
{
    union { unsigned long long u; double d; } v;
    v.u = b;
    return v.d;
}

float faddl(long double x, long double y) { return to_f(__axc_narrow(0, FMT, 0, &x, &y, &y)); }
float fsubl(long double x, long double y) { return to_f(__axc_narrow(1, FMT, 0, &x, &y, &y)); }
float fmull(long double x, long double y) { return to_f(__axc_narrow(2, FMT, 0, &x, &y, &y)); }
float fdivl(long double x, long double y) { return to_f(__axc_narrow(3, FMT, 0, &x, &y, &y)); }
float ffmal(long double x, long double y, long double z) { return to_f(__axc_narrow(4, FMT, 0, &x, &y, &z)); }
float fsqrtl(long double x) { return to_f(__axc_narrow(5, FMT, 0, &x, &x, &x)); }
double daddl(long double x, long double y) { return to_d(__axc_narrow(0, FMT, 1, &x, &y, &y)); }
double dsubl(long double x, long double y) { return to_d(__axc_narrow(1, FMT, 1, &x, &y, &y)); }
double dmull(long double x, long double y) { return to_d(__axc_narrow(2, FMT, 1, &x, &y, &y)); }
double ddivl(long double x, long double y) { return to_d(__axc_narrow(3, FMT, 1, &x, &y, &y)); }
double dfmal(long double x, long double y, long double z) { return to_d(__axc_narrow(4, FMT, 1, &x, &y, &z)); }
double dsqrtl(long double x) { return to_d(__axc_narrow(5, FMT, 1, &x, &x, &x)); }
#else
float faddl(long double x, long double y) { return fadd((double)x, (double)y); }
float fsubl(long double x, long double y) { return fsub((double)x, (double)y); }
float fmull(long double x, long double y) { return fmul((double)x, (double)y); }
float fdivl(long double x, long double y) { return fdiv((double)x, (double)y); }
float ffmal(long double x, long double y, long double z) { return ffma((double)x, (double)y, (double)z); }
float fsqrtl(long double x) { return fsqrt((double)x); }
/* (the double operations, which round in the current direction) */
double daddl(long double x, long double y) { return (double)x + (double)y; }
double dsubl(long double x, long double y) { return (double)x - (double)y; }
double dmull(long double x, long double y) { return (double)x * (double)y; }
double ddivl(long double x, long double y) { return (double)x / (double)y; }
double dfmal(long double x, long double y, long double z) { return fma((double)x, (double)y, (double)z); }
double dsqrtl(long double x) { return sqrt((double)x); }
#endif

#else
typedef int __axc_narrow_c89;
#endif
