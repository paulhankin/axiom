/* The calling convention of complex values (cgen_abi.ax: each target's C ABI
   treats them as the structure of their two parts): functions of 1 to 14
   parameters mixing float, double and long double _Complex with integers,
   reals and structures holding complex members, so that registers run out
   and arguments go on the stack; complex results; variadic complex
   arguments read with va_arg; called through volatile function pointers so
   that no call is inlined. Both sides are compiled here. Generated (with a
   fixed seed); the expected output was checked against another
   implementation where long double is double's format. */
#include <complex.h>
#include <stdarg.h>
#include <stddef.h>
#include "host.h"

typedef struct { float _Complex z; } S1;
typedef struct { double _Complex z; float f; } S2;
_Static_assert(sizeof(S1) == 8 && _Alignof(S1) == 4, "S1");
_Static_assert(sizeof(S2) == 24 && offsetof(S2, f) == 16, "S2");
_Static_assert(sizeof(float _Complex) == 8 && sizeof(double _Complex) == 16, "sizes");
_Static_assert(_Alignof(double _Complex) == 8 && _Alignof(float _Complex) == 4, "alignments");
_Static_assert(sizeof(long double _Complex) == 2 * sizeof(long double), "long double");

static double _Complex f0(float p0, S1 p1, int p2, double p3, long long p4, double p5) {
    double s = 0;
    s += p0 * 1;
    s += crealf(p1.z) * 2 + cimagf(p1.z) * 4;
    s += (double)p2 * 3;
    s += p3 * 4;
    s += (double)p4 * 5;
    s += p5 * 6;
    return CMPLX(s, -s * 0.5);
}
static double _Complex (*volatile pf0)(float, S1, int, double, long long, double) = f0;
static S1 f1(int p0, long long p1, float _Complex p2, int p3, double p4, S1 p5, S1 p6, double p7, float _Complex p8, double p9) {
    double s = 0;
    s += (double)p0 * 1;
    s += (double)p1 * 2;
    s += crealf(p2) * 3 + cimagf(p2) * 4;
    s += (double)p3 * 4;
    s += p4 * 5;
    s += crealf(p5.z) * 6 + cimagf(p5.z) * 8;
    s += crealf(p6.z) * 7 + cimagf(p6.z) * 9;
    s += p7 * 8;
    s += crealf(p8) * 9 + cimagf(p8) * 10;
    s += p9 * 10;
    S1 r = { CMPLXF((float)s, (float)(s + 1)) }; return r;
}
static S1 (*volatile pf1)(int, long long, float _Complex, int, double, S1, S1, double, float _Complex, double) = f1;
static double f2(int p0, double p1, float _Complex p2, int p3, S1 p4, int p5, float _Complex p6) {
    double s = 0;
    s += (double)p0 * 1;
    s += p1 * 2;
    s += crealf(p2) * 3 + cimagf(p2) * 4;
    s += (double)p3 * 4;
    s += crealf(p4.z) * 5 + cimagf(p4.z) * 7;
    s += (double)p5 * 6;
    s += crealf(p6) * 7 + cimagf(p6) * 8;
    return s;
}
static double (*volatile pf2)(int, double, float _Complex, int, S1, int, float _Complex) = f2;
static double f3(float p0, double _Complex p1, S1 p2, float p3, long long p4, double p5, double _Complex p6, long long p7, float p8) {
    double s = 0;
    s += p0 * 1;
    s += creal(p1) * 2 + cimag(p1) * 3;
    s += crealf(p2.z) * 3 + cimagf(p2.z) * 5;
    s += p3 * 4;
    s += (double)p4 * 5;
    s += p5 * 6;
    s += creal(p6) * 7 + cimag(p6) * 8;
    s += (double)p7 * 8;
    s += p8 * 9;
    return s;
}
static double (*volatile pf3)(float, double _Complex, S1, float, long long, double, double _Complex, long long, float) = f3;
static double _Complex f4(float _Complex p0, long double _Complex p1, double p2, long long p3, double p4, int p5, float _Complex p6, S2 p7, long long p8, S1 p9) {
    double s = 0;
    s += crealf(p0) * 1 + cimagf(p0) * 2;
    s += (double)creall(p1) * 2 + (double)cimagl(p1) * 3;
    s += p2 * 3;
    s += (double)p3 * 4;
    s += p4 * 5;
    s += (double)p5 * 6;
    s += crealf(p6) * 7 + cimagf(p6) * 8;
    s += creal(p7.z) * 8 + cimag(p7.z) * 11 + p7.f;
    s += (double)p8 * 9;
    s += crealf(p9.z) * 10 + cimagf(p9.z) * 12;
    return CMPLX(s, -s * 0.5);
}
static double _Complex (*volatile pf4)(float _Complex, long double _Complex, double, long long, double, int, float _Complex, S2, long long, S1) = f4;
static S1 f5(S2 p0, long double _Complex p1, double _Complex p2, float _Complex p3, float p4, float _Complex p5, double p6, double _Complex p7) {
    double s = 0;
    s += creal(p0.z) * 1 + cimag(p0.z) * 4 + p0.f;
    s += (double)creall(p1) * 2 + (double)cimagl(p1) * 3;
    s += creal(p2) * 3 + cimag(p2) * 4;
    s += crealf(p3) * 4 + cimagf(p3) * 5;
    s += p4 * 5;
    s += crealf(p5) * 6 + cimagf(p5) * 7;
    s += p6 * 7;
    s += creal(p7) * 8 + cimag(p7) * 9;
    S1 r = { CMPLXF((float)s, (float)(s + 1)) }; return r;
}
static S1 (*volatile pf5)(S2, long double _Complex, double _Complex, float _Complex, float, float _Complex, double, double _Complex) = f5;
static double _Complex f6(long double _Complex p0, S2 p1, double _Complex p2, double p3, double p4, long long p5, S1 p6, float p7) {
    double s = 0;
    s += (double)creall(p0) * 1 + (double)cimagl(p0) * 2;
    s += creal(p1.z) * 2 + cimag(p1.z) * 5 + p1.f;
    s += creal(p2) * 3 + cimag(p2) * 4;
    s += p3 * 4;
    s += p4 * 5;
    s += (double)p5 * 6;
    s += crealf(p6.z) * 7 + cimagf(p6.z) * 9;
    s += p7 * 8;
    return CMPLX(s, -s * 0.5);
}
static double _Complex (*volatile pf6)(long double _Complex, S2, double _Complex, double, double, long long, S1, float) = f6;
static S2 f7(S2 p0, S1 p1, int p2) {
    double s = 0;
    s += creal(p0.z) * 1 + cimag(p0.z) * 4 + p0.f;
    s += crealf(p1.z) * 2 + cimagf(p1.z) * 4;
    s += (double)p2 * 3;
    S2 r = { CMPLX(s, s * 2), (float)(s - 1) }; return r;
}
static S2 (*volatile pf7)(S2, S1, int) = f7;
static double _Complex f8(long long p0, long double _Complex p1) {
    double s = 0;
    s += (double)p0 * 1;
    s += (double)creall(p1) * 2 + (double)cimagl(p1) * 3;
    return CMPLX(s, -s * 0.5);
}
static double _Complex (*volatile pf8)(long long, long double _Complex) = f8;
static S2 f9(long double _Complex p0, S2 p1, S2 p2, double p3, double p4, double _Complex p5, S2 p6, double p7, int p8, double _Complex p9, S2 p10, double _Complex p11) {
    double s = 0;
    s += (double)creall(p0) * 1 + (double)cimagl(p0) * 2;
    s += creal(p1.z) * 2 + cimag(p1.z) * 5 + p1.f;
    s += creal(p2.z) * 3 + cimag(p2.z) * 6 + p2.f;
    s += p3 * 4;
    s += p4 * 5;
    s += creal(p5) * 6 + cimag(p5) * 7;
    s += creal(p6.z) * 7 + cimag(p6.z) * 10 + p6.f;
    s += p7 * 8;
    s += (double)p8 * 9;
    s += creal(p9) * 10 + cimag(p9) * 11;
    s += creal(p10.z) * 11 + cimag(p10.z) * 14 + p10.f;
    s += creal(p11) * 12 + cimag(p11) * 13;
    S2 r = { CMPLX(s, s * 2), (float)(s - 1) }; return r;
}
static S2 (*volatile pf9)(long double _Complex, S2, S2, double, double, double _Complex, S2, double, int, double _Complex, S2, double _Complex) = f9;
static double f10(long double _Complex p0, int p1, S2 p2, long double _Complex p3, float p4, double p5, S2 p6) {
    double s = 0;
    s += (double)creall(p0) * 1 + (double)cimagl(p0) * 2;
    s += (double)p1 * 2;
    s += creal(p2.z) * 3 + cimag(p2.z) * 6 + p2.f;
    s += (double)creall(p3) * 4 + (double)cimagl(p3) * 5;
    s += p4 * 5;
    s += p5 * 6;
    s += creal(p6.z) * 7 + cimag(p6.z) * 10 + p6.f;
    return s;
}
static double (*volatile pf10)(long double _Complex, int, S2, long double _Complex, float, double, S2) = f10;
static long double _Complex f11(double _Complex p0, float p1, float _Complex p2, S1 p3) {
    double s = 0;
    s += creal(p0) * 1 + cimag(p0) * 2;
    s += p1 * 2;
    s += crealf(p2) * 3 + cimagf(p2) * 4;
    s += crealf(p3.z) * 4 + cimagf(p3.z) * 6;
    return CMPLXL(s, s * 0.25);
}
static long double _Complex (*volatile pf11)(double _Complex, float, float _Complex, S1) = f11;
static float _Complex f12(S2 p0, double p1, float p2, S2 p3, S1 p4, long long p5, double _Complex p6, float p7, S1 p8, long long p9, double _Complex p10, S1 p11, long double _Complex p12, S1 p13) {
    double s = 0;
    s += creal(p0.z) * 1 + cimag(p0.z) * 4 + p0.f;
    s += p1 * 2;
    s += p2 * 3;
    s += creal(p3.z) * 4 + cimag(p3.z) * 7 + p3.f;
    s += crealf(p4.z) * 5 + cimagf(p4.z) * 7;
    s += (double)p5 * 6;
    s += creal(p6) * 7 + cimag(p6) * 8;
    s += p7 * 8;
    s += crealf(p8.z) * 9 + cimagf(p8.z) * 11;
    s += (double)p9 * 10;
    s += creal(p10) * 11 + cimag(p10) * 12;
    s += crealf(p11.z) * 12 + cimagf(p11.z) * 14;
    s += (double)creall(p12) * 13 + (double)cimagl(p12) * 14;
    s += crealf(p13.z) * 14 + cimagf(p13.z) * 16;
    return CMPLXF((float)s, (float)(-s * 0.5));
}
static float _Complex (*volatile pf12)(S2, double, float, S2, S1, long long, double _Complex, float, S1, long long, double _Complex, S1, long double _Complex, S1) = f12;
static float _Complex f13(double p0, float p1, float p2) {
    double s = 0;
    s += p0 * 1;
    s += p1 * 2;
    s += p2 * 3;
    return CMPLXF((float)s, (float)(-s * 0.5));
}
static float _Complex (*volatile pf13)(double, float, float) = f13;
static S1 f14(float _Complex p0, int p1, S2 p2, float p3, double _Complex p4, double _Complex p5, int p6, float p7, S1 p8, long long p9, long double _Complex p10) {
    double s = 0;
    s += crealf(p0) * 1 + cimagf(p0) * 2;
    s += (double)p1 * 2;
    s += creal(p2.z) * 3 + cimag(p2.z) * 6 + p2.f;
    s += p3 * 4;
    s += creal(p4) * 5 + cimag(p4) * 6;
    s += creal(p5) * 6 + cimag(p5) * 7;
    s += (double)p6 * 7;
    s += p7 * 8;
    s += crealf(p8.z) * 9 + cimagf(p8.z) * 11;
    s += (double)p9 * 10;
    s += (double)creall(p10) * 11 + (double)cimagl(p10) * 12;
    S1 r = { CMPLXF((float)s, (float)(s + 1)) }; return r;
}
static S1 (*volatile pf14)(float _Complex, int, S2, float, double _Complex, double _Complex, int, float, S1, long long, long double _Complex) = f14;
static double f15(long double _Complex p0, float p1, long long p2, int p3, S2 p4, long long p5, S1 p6, S1 p7, S1 p8, S1 p9) {
    double s = 0;
    s += (double)creall(p0) * 1 + (double)cimagl(p0) * 2;
    s += p1 * 2;
    s += (double)p2 * 3;
    s += (double)p3 * 4;
    s += creal(p4.z) * 5 + cimag(p4.z) * 8 + p4.f;
    s += (double)p5 * 6;
    s += crealf(p6.z) * 7 + cimagf(p6.z) * 9;
    s += crealf(p7.z) * 8 + cimagf(p7.z) * 10;
    s += crealf(p8.z) * 9 + cimagf(p8.z) * 11;
    s += crealf(p9.z) * 10 + cimagf(p9.z) * 12;
    return s;
}
static double (*volatile pf15)(long double _Complex, float, long long, int, S2, long long, S1, S1, S1, S1) = f15;
static double _Complex f16(S1 p0, int p1, float _Complex p2, double p3, float _Complex p4, S2 p5, float p6, double p7) {
    double s = 0;
    s += crealf(p0.z) * 1 + cimagf(p0.z) * 3;
    s += (double)p1 * 2;
    s += crealf(p2) * 3 + cimagf(p2) * 4;
    s += p3 * 4;
    s += crealf(p4) * 5 + cimagf(p4) * 6;
    s += creal(p5.z) * 6 + cimag(p5.z) * 9 + p5.f;
    s += p6 * 7;
    s += p7 * 8;
    return CMPLX(s, -s * 0.5);
}
static double _Complex (*volatile pf16)(S1, int, float _Complex, double, float _Complex, S2, float, double) = f16;
static S1 f17(int p0, double p1, int p2, float p3, long long p4, double p5, long double _Complex p6, int p7, double p8, float _Complex p9) {
    double s = 0;
    s += (double)p0 * 1;
    s += p1 * 2;
    s += (double)p2 * 3;
    s += p3 * 4;
    s += (double)p4 * 5;
    s += p5 * 6;
    s += (double)creall(p6) * 7 + (double)cimagl(p6) * 8;
    s += (double)p7 * 8;
    s += p8 * 9;
    s += crealf(p9) * 10 + cimagf(p9) * 11;
    S1 r = { CMPLXF((float)s, (float)(s + 1)) }; return r;
}
static S1 (*volatile pf17)(int, double, int, float, long long, double, long double _Complex, int, double, float _Complex) = f17;
static long double _Complex f18(float p0, double _Complex p1, long double _Complex p2, long double _Complex p3, S2 p4, double p5, double p6) {
    double s = 0;
    s += p0 * 1;
    s += creal(p1) * 2 + cimag(p1) * 3;
    s += (double)creall(p2) * 3 + (double)cimagl(p2) * 4;
    s += (double)creall(p3) * 4 + (double)cimagl(p3) * 5;
    s += creal(p4.z) * 5 + cimag(p4.z) * 8 + p4.f;
    s += p5 * 6;
    s += p6 * 7;
    return CMPLXL(s, s * 0.25);
}
static long double _Complex (*volatile pf18)(float, double _Complex, long double _Complex, long double _Complex, S2, double, double) = f18;
static long double _Complex f19(S2 p0, S2 p1, double _Complex p2, double p3, float p4, double p5, long double _Complex p6, double _Complex p7) {
    double s = 0;
    s += creal(p0.z) * 1 + cimag(p0.z) * 4 + p0.f;
    s += creal(p1.z) * 2 + cimag(p1.z) * 5 + p1.f;
    s += creal(p2) * 3 + cimag(p2) * 4;
    s += p3 * 4;
    s += p4 * 5;
    s += p5 * 6;
    s += (double)creall(p6) * 7 + (double)cimagl(p6) * 8;
    s += creal(p7) * 8 + cimag(p7) * 9;
    return CMPLXL(s, s * 0.25);
}
static long double _Complex (*volatile pf19)(S2, S2, double _Complex, double, float, double, long double _Complex, double _Complex) = f19;
static double _Complex f20(float p0, long long p1, int p2, float _Complex p3, long long p4, long double _Complex p5, float p6, long long p7, int p8, long long p9, double _Complex p10, double p11, double _Complex p12, long long p13) {
    double s = 0;
    s += p0 * 1;
    s += (double)p1 * 2;
    s += (double)p2 * 3;
    s += crealf(p3) * 4 + cimagf(p3) * 5;
    s += (double)p4 * 5;
    s += (double)creall(p5) * 6 + (double)cimagl(p5) * 7;
    s += p6 * 7;
    s += (double)p7 * 8;
    s += (double)p8 * 9;
    s += (double)p9 * 10;
    s += creal(p10) * 11 + cimag(p10) * 12;
    s += p11 * 12;
    s += creal(p12) * 13 + cimag(p12) * 14;
    s += (double)p13 * 14;
    return CMPLX(s, -s * 0.5);
}
static double _Complex (*volatile pf20)(float, long long, int, float _Complex, long long, long double _Complex, float, long long, int, long long, double _Complex, double, double _Complex, long long) = f20;
static S1 f21(long double _Complex p0, float _Complex p1, long long p2) {
    double s = 0;
    s += (double)creall(p0) * 1 + (double)cimagl(p0) * 2;
    s += crealf(p1) * 2 + cimagf(p1) * 3;
    s += (double)p2 * 3;
    S1 r = { CMPLXF((float)s, (float)(s + 1)) }; return r;
}
static S1 (*volatile pf21)(long double _Complex, float _Complex, long long) = f21;
static double _Complex f22(long long p0, long double _Complex p1, float _Complex p2, float _Complex p3, float _Complex p4, S1 p5, float _Complex p6, float _Complex p7, long long p8, S2 p9, long double _Complex p10, int p11, int p12) {
    double s = 0;
    s += (double)p0 * 1;
    s += (double)creall(p1) * 2 + (double)cimagl(p1) * 3;
    s += crealf(p2) * 3 + cimagf(p2) * 4;
    s += crealf(p3) * 4 + cimagf(p3) * 5;
    s += crealf(p4) * 5 + cimagf(p4) * 6;
    s += crealf(p5.z) * 6 + cimagf(p5.z) * 8;
    s += crealf(p6) * 7 + cimagf(p6) * 8;
    s += crealf(p7) * 8 + cimagf(p7) * 9;
    s += (double)p8 * 9;
    s += creal(p9.z) * 10 + cimag(p9.z) * 13 + p9.f;
    s += (double)creall(p10) * 11 + (double)cimagl(p10) * 12;
    s += (double)p11 * 12;
    s += (double)p12 * 13;
    return CMPLX(s, -s * 0.5);
}
static double _Complex (*volatile pf22)(long long, long double _Complex, float _Complex, float _Complex, float _Complex, S1, float _Complex, float _Complex, long long, S2, long double _Complex, int, int) = f22;
static double f23(double _Complex p0, float _Complex p1, long double _Complex p2, S2 p3, long double _Complex p4, long double _Complex p5, double p6, float _Complex p7) {
    double s = 0;
    s += creal(p0) * 1 + cimag(p0) * 2;
    s += crealf(p1) * 2 + cimagf(p1) * 3;
    s += (double)creall(p2) * 3 + (double)cimagl(p2) * 4;
    s += creal(p3.z) * 4 + cimag(p3.z) * 7 + p3.f;
    s += (double)creall(p4) * 5 + (double)cimagl(p4) * 6;
    s += (double)creall(p5) * 6 + (double)cimagl(p5) * 7;
    s += p6 * 7;
    s += crealf(p7) * 8 + cimagf(p7) * 9;
    return s;
}
static double (*volatile pf23)(double _Complex, float _Complex, long double _Complex, S2, long double _Complex, long double _Complex, double, float _Complex) = f23;

/* n (tag, value) pairs: the tag says the value's type (VT's order) */
static double vsum(int n, ...) {
    va_list ap;
    double s = 0;
    int i;
    va_start(ap, n);
    for (i = 0; i < n; i++) {
        int tag = va_arg(ap, int);
        if (tag == 0) {
            s += va_arg(ap, int) * (i + 1);
        } else if (tag == 1) {
            s += va_arg(ap, double) * (i + 1);
        } else if (tag == 2) {
            float _Complex z = va_arg(ap, float _Complex);
            s += crealf(z) * (i + 1) + cimagf(z) * (i + 2);
        } else if (tag == 3) {
            double _Complex z = va_arg(ap, double _Complex);
            s += creal(z) * (i + 1) + cimag(z) * (i + 2);
        } else if (tag == 4) {
            long double _Complex z = va_arg(ap, long double _Complex);
            s += (double)creall(z) * (i + 1) + (double)cimagl(z) * (i + 2);
        } else if (tag == 5) {
            S1 x = va_arg(ap, S1);
            s += crealf(x.z) * (i + 1) + cimagf(x.z) * (i + 3);
        } else {
            S2 x = va_arg(ap, S2);
            s += creal(x.z) * (i + 1) + cimag(x.z) * (i + 3) + x.f;
        }
    }
    va_end(ap);
    return s;
}
static double (*volatile pv)(int, ...) = vsum;

int main(void) {
    { double _Complex r = pf0(0.25f, (S1){ CMPLXF(0.75, -1.125) }, -1, 1.75, 5, 2.75); print_f64(creal(r)); print_f64(cimag(r)); }
    { S1 r = pf1(-4, -1, CMPLXF(1.75, -1.375), 5, 2.75, (S1){ CMPLXF(3.25, -1.75) }, (S1){ CMPLXF(3.75, -1.875) }, 4.25, CMPLXF(4.75, -2.125), 5.25); print_f64(crealf(r.z)); print_f64(cimagf(r.z)); }
    print_f64(pf2(-1, 1.75, CMPLXF(2.25, -1.5), 8, (S1){ CMPLXF(3.25, -1.75) }, 14, CMPLXF(4.25, -2.0)));
    print_f64(pf3(1.75f, CMPLX(2.25, -1.5), (S1){ CMPLXF(2.75, -1.625) }, 3.25f, 14, 4.25, CMPLX(4.75, -2.125), 23, 5.75f));
    { double _Complex r = pf4(CMPLXF(2.25, -1.5), CMPLXL(2.75, -1.625), 3.25, 14, 4.25, 20, CMPLXF(5.25, -2.25), (S2){ CMPLX(5.75, -2.375), -7.125 }, 29, (S1){ CMPLXF(6.75, -2.625) }); print_f64(creal(r)); print_f64(cimag(r)); }
    { S1 r = pf5((S2){ CMPLX(2.75, -1.625), -4.875 }, CMPLXL(3.25, -1.75), CMPLX(3.75, -1.875), CMPLXF(4.25, -2.0), 4.75f, CMPLXF(5.25, -2.25), 5.75, CMPLX(6.25, -2.5)); print_f64(crealf(r.z)); print_f64(cimagf(r.z)); }
    { double _Complex r = pf6(CMPLXL(3.25, -1.75), (S2){ CMPLX(3.75, -1.875), -5.625 }, CMPLX(4.25, -2.0), 4.75, 5.25, 26, (S1){ CMPLXF(6.25, -2.5) }, 6.75f); print_f64(creal(r)); print_f64(cimag(r)); }
    { S2 r = pf7((S2){ CMPLX(3.75, -1.875), -5.625 }, (S1){ CMPLXF(4.25, -2.0) }, 20); print_f64(creal(r.z)); print_f64(cimag(r.z)); print_f64(r.f); }
    { double _Complex r = pf8(17, CMPLXL(4.75, -2.125)); print_f64(creal(r)); print_f64(cimag(r)); }
    { S2 r = pf9(CMPLXL(4.75, -2.125), (S2){ CMPLX(5.25, -2.25), -6.75 }, (S2){ CMPLX(5.75, -2.375), -7.125 }, 6.25, 6.75, CMPLX(7.25, -2.75), (S2){ CMPLX(7.75, -2.875), -8.625 }, 8.25, 44, CMPLX(9.25, -3.25), (S2){ CMPLX(9.75, -3.375), -10.125 }, CMPLX(10.25, -3.5)); print_f64(creal(r.z)); print_f64(cimag(r.z)); print_f64(r.f); }
    print_f64(pf10(CMPLXL(5.25, -2.25), 26, (S2){ CMPLX(6.25, -2.5), -7.5 }, CMPLXL(6.75, -2.625), 7.25f, 7.75, (S2){ CMPLX(8.25, -3.0), -9.0 }));
    { long double _Complex r = pf11(CMPLX(5.75, -2.375), 6.25f, CMPLXF(6.75, -2.625), (S1){ CMPLXF(7.25, -2.75) }); print_f64((double)creall(r)); print_f64((double)cimagl(r)); }
    { float _Complex r = pf12((S2){ CMPLX(6.25, -2.5), -7.5 }, 6.75, 7.25f, (S2){ CMPLX(7.75, -2.875), -8.625 }, (S1){ CMPLXF(8.25, -3.0) }, 44, CMPLX(9.25, -3.25), 9.75f, (S1){ CMPLXF(10.25, -3.5) }, 56, CMPLX(11.25, -3.75), (S1){ CMPLXF(11.75, -3.875) }, CMPLXL(12.25, -4.0), (S1){ CMPLXF(12.75, -4.125) }); print_f64(crealf(r)); print_f64(cimagf(r)); }
    { float _Complex r = pf13(6.75, 7.25f, 7.75f); print_f64(crealf(r)); print_f64(cimagf(r)); }
    { S1 r = pf14(CMPLXF(7.25, -2.75), 38, (S2){ CMPLX(8.25, -3.0), -9.0 }, 8.75f, CMPLX(9.25, -3.25), CMPLX(9.75, -3.375), 53, 10.75f, (S1){ CMPLXF(11.25, -3.75) }, 62, CMPLXL(12.25, -4.0)); print_f64(crealf(r.z)); print_f64(cimagf(r.z)); }
    print_f64(pf15(CMPLXL(7.75, -2.875), 8.25f, 44, 47, (S2){ CMPLX(9.75, -3.375), -10.125 }, 53, (S1){ CMPLXF(10.75, -3.625) }, (S1){ CMPLXF(11.25, -3.75) }, (S1){ CMPLXF(11.75, -3.875) }, (S1){ CMPLXF(12.25, -4.0) }));
    { double _Complex r = pf16((S1){ CMPLXF(8.25, -3.0) }, 44, CMPLXF(9.25, -3.25), 9.75, CMPLXF(10.25, -3.5), (S2){ CMPLX(10.75, -3.625), -10.875 }, 11.25f, 11.75); print_f64(creal(r)); print_f64(cimag(r)); }
    { S1 r = pf17(44, 9.25, 50, 10.25f, 56, 11.25, CMPLXL(11.75, -3.875), 65, 12.75, CMPLXF(13.25, -4.25)); print_f64(crealf(r.z)); print_f64(cimagf(r.z)); }
    { long double _Complex r = pf18(9.25f, CMPLX(9.75, -3.375), CMPLXL(10.25, -3.5), CMPLXL(10.75, -3.625), (S2){ CMPLX(11.25, -3.75), -11.25 }, 11.75, 12.25); print_f64((double)creall(r)); print_f64((double)cimagl(r)); }
    { long double _Complex r = pf19((S2){ CMPLX(9.75, -3.375), -10.125 }, (S2){ CMPLX(10.25, -3.5), -10.5 }, CMPLX(10.75, -3.625), 11.25, 11.75f, 12.25, CMPLXL(12.75, -4.125), CMPLX(13.25, -4.25)); print_f64((double)creall(r)); print_f64((double)cimagl(r)); }
    { double _Complex r = pf20(10.25f, 56, 59, CMPLXF(11.75, -3.875), 65, CMPLXL(12.75, -4.125), 13.25f, 74, 77, 80, CMPLX(15.25, -4.75), 15.75, CMPLX(16.25, -5.0), 92); print_f64(creal(r)); print_f64(cimag(r)); }
    { S1 r = pf21(CMPLXL(10.75, -3.625), CMPLXF(11.25, -3.75), 62); print_f64(crealf(r.z)); print_f64(cimagf(r.z)); }
    { double _Complex r = pf22(59, CMPLXL(11.75, -3.875), CMPLXF(12.25, -4.0), CMPLXF(12.75, -4.125), CMPLXF(13.25, -4.25), (S1){ CMPLXF(13.75, -4.375) }, CMPLXF(14.25, -4.5), CMPLXF(14.75, -4.625), 83, (S2){ CMPLX(15.75, -4.875), -14.625 }, CMPLXL(16.25, -5.0), 92, 95); print_f64(creal(r)); print_f64(cimag(r)); }
    print_f64(pf23(CMPLX(11.75, -3.875), CMPLXF(12.25, -4.0), CMPLXL(12.75, -4.125), (S2){ CMPLX(13.25, -4.25), -12.75 }, CMPLXL(13.75, -4.375), CMPLXL(14.25, -4.5), 14.75, CMPLXF(15.25, -4.75)));
    print_f64(pv(4, 3, CMPLX(0.25, -1.0), 1, 0.75, 2, CMPLXF(1.25, -1.25), 1, 1.75));
    print_f64(pv(8, 4, CMPLXL(0.75, -1.125), 4, CMPLXL(1.25, -1.25), 6, (S2){ CMPLX(1.75, -1.375), -4.125 }, 0, 5, 3, CMPLX(2.75, -1.625), 5, (S1){ CMPLXF(3.25, -1.75) }, 2, CMPLXF(3.75, -1.875), 6, (S2){ CMPLX(4.25, -2.0), -6.0 }));
    print_f64(pv(11, 0, -1, 6, (S2){ CMPLX(1.75, -1.375), -4.125 }, 5, (S1){ CMPLXF(2.25, -1.5) }, 0, 8, 3, CMPLX(3.25, -1.75), 6, (S2){ CMPLX(3.75, -1.875), -5.625 }, 5, (S1){ CMPLXF(4.25, -2.0) }, 6, (S2){ CMPLX(4.75, -2.125), -6.375 }, 1, 5.25, 3, CMPLX(5.75, -2.375), 1, 6.25));
    print_f64(pv(7, 6, (S2){ CMPLX(1.75, -1.375), -4.125 }, 5, (S1){ CMPLXF(2.25, -1.5) }, 2, CMPLXF(2.75, -1.625), 0, 11, 6, (S2){ CMPLX(3.75, -1.875), -5.625 }, 5, (S1){ CMPLXF(4.25, -2.0) }, 3, CMPLX(4.75, -2.125)));
    print_f64(pv(8, 3, CMPLX(2.25, -1.5), 5, (S1){ CMPLXF(2.75, -1.625) }, 0, 11, 5, (S1){ CMPLXF(3.75, -1.875) }, 1, 4.25, 1, 4.75, 1, 5.25, 0, 26));
    print_f64(pv(3, 4, CMPLXL(2.75, -1.625), 3, CMPLX(3.25, -1.75), 6, (S2){ CMPLX(3.75, -1.875), -5.625 }));
    print_f64(pv(11, 1, 3.25, 4, CMPLXL(3.75, -1.875), 6, (S2){ CMPLX(4.25, -2.0), -6.0 }, 4, CMPLXL(4.75, -2.125), 3, CMPLX(5.25, -2.25), 5, (S1){ CMPLXF(5.75, -2.375) }, 2, CMPLXF(6.25, -2.5), 1, 6.75, 4, CMPLXL(7.25, -2.75), 4, CMPLXL(7.75, -2.875), 1, 8.25));
    print_f64(pv(1, 0, 14));
    print_f64(pv(12, 5, (S1){ CMPLXF(4.25, -2.0) }, 0, 20, 4, CMPLXL(5.25, -2.25), 5, (S1){ CMPLXF(5.75, -2.375) }, 1, 6.25, 3, CMPLX(6.75, -2.625), 6, (S2){ CMPLX(7.25, -2.75), -8.25 }, 1, 7.75, 6, (S2){ CMPLX(8.25, -3.0), -9.0 }, 6, (S2){ CMPLX(8.75, -3.125), -9.375 }, 1, 9.25, 0, 50));
    print_f64(pv(5, 1, 4.75, 2, CMPLXF(5.25, -2.25), 4, CMPLXL(5.75, -2.375), 1, 6.25, 6, (S2){ CMPLX(6.75, -2.625), -7.875 }));
    print_f64(pv(10, 2, CMPLXF(5.25, -2.25), 2, CMPLXF(5.75, -2.375), 4, CMPLXL(6.25, -2.5), 3, CMPLX(6.75, -2.625), 6, (S2){ CMPLX(7.25, -2.75), -8.25 }, 1, 7.75, 0, 41, 5, (S1){ CMPLXF(8.75, -3.125) }, 2, CMPLXF(9.25, -3.25), 3, CMPLX(9.75, -3.375)));
    print_f64(pv(11, 4, CMPLXL(5.75, -2.375), 6, (S2){ CMPLX(6.25, -2.5), -7.5 }, 4, CMPLXL(6.75, -2.625), 3, CMPLX(7.25, -2.75), 6, (S2){ CMPLX(7.75, -2.875), -8.625 }, 4, CMPLXL(8.25, -3.0), 1, 8.75, 4, CMPLXL(9.25, -3.25), 1, 9.75, 4, CMPLXL(10.25, -3.5), 4, CMPLXL(10.75, -3.625)));
    return 0;
}

/* expect: 42.75 */
/* expect: -21.375 */
/* expect: 150.375 */
/* expect: 151.375 */
/* expect: 137 */
/* expect: 362.375 */
/* expect: 535.375 */
/* expect: -267.6875 */
/* expect: 110.625 */
/* expect: 111.625 */
/* expect: 273.5 */
/* expect: -136.75 */
/* expect: 51.125 */
/* expect: 102.25 */
/* expect: 50.125 */
/* expect: 20.125 */
/* expect: -10.0625 */
/* expect: 734.875 */
/* expect: 1469.75 */
/* expect: 733.875 */
/* expect: 164.375 */
/* expect: 35.75 */
/* expect: 8.9375 */
/* expect: 1422.25 */
/* expect: -711.125 */
/* expect: 44.5 */
/* expect: -22.25 */
/* expect: 1395.875 */
/* expect: 1396.875 */
/* expect: 900.875 */
/* expect: 365 */
/* expect: -182.5 */
/* expect: 1372.75 */
/* expect: 1373.75 */
/* expect: 231.5 */
/* expect: 57.875 */
/* expect: 297.625 */
/* expect: 74.40625 */
/* expect: 4606.25 */
/* expect: -2303.125 */
/* expect: 200.75 */
/* expect: 201.75 */
/* expect: 3634.625 */
/* expect: -1817.3125 */
/* expect: 327.75 */
/* expect: 5.5 */
/* expect: 40.25 */
/* expect: 201.25 */
/* expect: 63.5 */
/* expect: 329.5 */
/* expect: -3 */
/* expect: 290.625 */
/* expect: 14 */
/* expect: 961.5 */
/* expect: 48.75 */
/* expect: 519.875 */
/* expect: 392 */
