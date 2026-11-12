/* host_native.c - the output functions of host.h for the native C tests:
   those of ../native_rt.c, without its entry point (a C program's start-up
   is the compiler's). Compiled by the system's C compiler. */
#define main axiom_tests_native_rt_main
#include "../native_rt.c"
#undef main
void run(void) {}

/* (unbuffered, as native_rt.c's main makes it: output before a crash is kept) */
__attribute__((constructor)) static void unbuffered(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
}

/* For c/run/abi_*.c: functions of the system's C compiler that take and
   return structures by value, call back through pointers, and are
   variadic, so the C tests check the platform calling convention against
   it (the tests declare the same types). */
#include <stdarg.h>
typedef struct { char c[3]; } h_c3;
typedef struct { int a, b; } h_ii;
typedef struct { float a, b, c; } h_f3;
typedef struct { double a, b; } h_dd;
typedef struct { double d; int i; } h_di;
typedef struct { long long a[3]; } h_l3;
typedef union { double d; long long l; } h_ud;
h_c3 h_make_c3(int k) { h_c3 r = { { (char)k, (char)(k + 1), (char)(k + 2) } }; return r; }
h_ii h_make_ii(int k) { h_ii r = { k, -k }; return r; }
h_f3 h_make_f3(float k) { h_f3 r = { k, k * 2, k * 3 }; return r; }
h_dd h_make_dd(double k) { h_dd r = { k, k + 0.5 }; return r; }
h_di h_make_di(double k) { h_di r = { k, (int)k * 2 }; return r; }
h_l3 h_make_l3(long long k) { h_l3 r = { { k, k * 10, k * 100 } }; return r; }
h_ud h_make_ud(double k) { h_ud r; r.d = k; return r; }
long long h_sum(int n, h_c3 c, h_ii i, h_f3 f, double x, h_dd d, h_di m, h_l3 l, h_ud u, float y) {
    return n + c.c[0] + c.c[2] + i.a * 2 + i.b + (long long)(f.a + f.c) + (long long)x + (long long)(d.a * 4 + d.b)
        + (long long)m.d + m.i + l.a[0] + l.a[2] + (long long)u.d + (long long)(y * 2);
}
long long h_call(h_dd (*f)(h_dd, h_l3, int), h_l3 (*g)(h_ii, h_c3)) {
    h_dd d = { 1.5, 2.5 };
    h_l3 l = { { 3, 4, 5 } };
    h_ii i = { 6, 7 };
    h_c3 c = { { 8, 9, 10 } };
    h_dd r = f(d, l, 2);
    h_l3 s = g(i, c);
    return (long long)(r.a * 10 + r.b) * 1000 + s.a[0] + s.a[1] + s.a[2];
}
long long h_va(int n, ...) {
    va_list ap;
    long long s = 0;
    va_start(ap, n);
    while (n-- > 0) {
        h_dd d = va_arg(ap, h_dd);
        h_l3 l = va_arg(ap, h_l3);
        int k = va_arg(ap, int);
        h_f3 f = va_arg(ap, h_f3);
        s = s * 100 + (long long)(d.a + d.b) + l.a[1] + k + (long long)f.b;
    }
    va_end(ap);
    return s;
}
long long h_vcall(long long (*f)(int, ...)) {
    h_dd d = { 1.5, 2.5 };
    h_l3 l = { { 3, 4, 5 } };
    h_f3 g = { 1, 2, 3 };
    return f(2, d, l, 7, g, d, l, 8, g);
}
long long h_vlist(int n, va_list ap) {
    long long s = 0;
    while (n-- > 0) s = s * 10 + va_arg(ap, int) + (long long)va_arg(ap, double);
    return s;
}

/* For c/run/complex_abi_host.c: complex arguments, results, variadic
   arguments and callbacks of the system's C compiler. */
#include <complex.h>
typedef struct { float _Complex z; double d; } h_cs;
float _Complex h_cf(float _Complex a, double _Complex b, int k) {
    return a * k + (float _Complex)b;
}
double _Complex h_cd(double x, double _Complex a, float _Complex b, h_cs s, double _Complex c, double _Complex d,
                     double _Complex e, double _Complex f) {
    return x * a + b + s.z * s.d + c - d + e * 2 + f * I;
}
h_cs h_mk(float _Complex z, double d) { h_cs r = { z, d }; return r; }
double h_cva(int n, ...) {
    va_list ap;
    double s = 0;
    va_start(ap, n);
    while (n-- > 0) {
        double _Complex a = va_arg(ap, double _Complex);
        float _Complex b = va_arg(ap, float _Complex);
        s = s * 10 + creal(a) + cimag(a) * 2 + crealf(b) * 3 + cimagf(b) * 4;
    }
    va_end(ap);
    return s;
}
double _Complex h_ccall(double _Complex (*f)(double _Complex, float _Complex, ...)) {
    return f(CMPLX(1, 2), CMPLXF(3, 4), CMPLX(5, 6), CMPLXF(7, 8));
}

/* For c/run/ldouble_abi_host.c: long double (the platform's format: the
   x87's on x86-64, binary128 on AArch64) passed and returned alone, in
   structures and unions, among many arguments, as variadic arguments and
   through function pointers. */
#include <float.h>
typedef struct { long double x; } h_l1;
typedef struct { char c; long double x; } h_cl;
typedef struct { long double a, b; } h_l2;
typedef struct { double d; long double x; } h_dl;
typedef union { long double x; long long l[2]; } h_ul;
long double h_ld_many(int i1, long double a, double d1, long double b, int i2, long double c, long double d,
                      long double e, long double f, long double g, long double h, long double i, long double j,
                      float f1, long double k) {
    return i1 + a + d1 + b * 2 + i2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9 + j * 10 + f1 + k * 11;
}
h_l1 h_ld_l1(h_l1 s) { s.x = s.x * 2 + 1; return s; }
h_cl h_ld_cl(h_cl s) { s.x = s.x + s.c; s.c = 'z'; return s; }
h_l2 h_ld_l2(h_l2 s) { h_l2 r = { s.b - 1, s.a + 1 }; return r; }
h_dl h_ld_dl(h_dl s) { s.x = s.x - s.d; s.d = 4.5; return s; }
h_ul h_ld_ul(h_ul u) { u.x = u.x / 4; return u; }
long double h_ld_va(int n, ...) {
    va_list ap;
    long double s = 0;
    int k;
    va_start(ap, n);
    for (k = 0; k < n; k++) {
        if (k % 4 == 0) s = s * 2 + va_arg(ap, long double);
        else if (k % 4 == 1) s = s * 2 + va_arg(ap, double);
        else if (k % 4 == 2) s = s * 2 + va_arg(ap, int);
        else s = s * 2 + va_arg(ap, h_l2).b;
    }
    va_end(ap);
    return s;
}
long double h_ld_call(long double (*f)(long double, int, long double), long double a, long double b) {
    return f(a, 3, b) * 2;
}
long double h_ld_vcall(long double (*f)(int, ...)) {
    h_l2 p = { 0.5L, 1.75L };
    return f(4, 1.25L, 2.5, 3, p);
}
h_l2 h_ld_scall(h_l2 (*f)(h_l2, long double)) {
    h_l2 p = { 0.5L, 1.75L };
    h_l2 r = f(p, 0.25L);
    r.a = r.a * 100;
    return r;
}
int h_ld_order(long double a, long double b) { return (a < b) + (a == b) * 2 + (a > b) * 4; }
int h_ld_eps(long double x) { return x - 1 == LDBL_EPSILON; }

/* For c/run/ldouble_complex_host.c: long double _Complex (x86-64
   System V's COMPLEX_X87: in memory, the result in st(0) and st(1); an
   AArch64 homogeneous aggregate of two binary128 values). */
typedef struct { long double _Complex z; int k; } h_lcs;
long double _Complex h_lc_add(long double _Complex a, long double _Complex b) { return a + b * 2; }
long double _Complex h_lc_many(int i, long double _Complex a, double d, long double _Complex b, long double _Complex c,
                               long double _Complex e, long double _Complex f, long double g, long double _Complex h) {
    return i + a + d + b * 2 + c * 3 + e * 4 + f * 5 + g + h * 7;
}
h_lcs h_lc_s(h_lcs s) { s.z = s.z * 2; s.k++; return s; }
long double _Complex h_lc_va(int n, ...) {
    va_list ap;
    long double _Complex s = 0;
    va_start(ap, n);
    while (n-- > 0) {
        s = s * 2 + va_arg(ap, long double _Complex);
        s += va_arg(ap, double);
    }
    va_end(ap);
    return s;
}
long double _Complex h_lc_call(long double _Complex (*f)(long double _Complex, long double)) {
    return f(CMPLXL(1.5L, -2.25L), 0.5L) * 3;
}

/* For c/run/wint_abi_host.c: __int128 (x86-64 System V: two INTEGER
   eightbytes, both in registers or both on the stack, aligned to 16 there;
   AAPCS64: a pair of x registers from an even one, the stack aligned to
   16), alone, in structures, beyond the registers, variadic. */
#ifdef __SIZEOF_INT128__
typedef struct { __int128 x; } h_q1;
typedef struct { char c; __int128 x; } h_cq;
typedef struct { long l; unsigned __int128 x; } h_lq;
__int128 h_q_many(int i1, __int128 a, long l1, __int128 b, int i2, __int128 c, __int128 d, int i3, __int128 e,
                  double f, __int128 g) {
    return i1 + a * 2 + l1 * 3 + b * 4 + i2 * 5 + c * 6 + d * 7 + i3 * 8 + e * 9 + (__int128)f * 10 + g * 11;
}
unsigned __int128 h_q_shift(unsigned __int128 x, int k) { return (x << k) | (x >> (128 - k)); }
h_q1 h_q_q1(h_q1 s) { s.x = s.x * 3 - 1; return s; }
h_cq h_q_cq(h_cq s) { s.x += s.c; s.c = 'q'; return s; }
h_lq h_q_lq(h_lq s) { s.x = s.x * s.l; s.l = -1; return s; }
__int128 h_q_va(int n, ...) {
    va_list ap;
    __int128 s = 0;
    va_start(ap, n);
    while (n-- > 0) {
        s = s * 3 + va_arg(ap, __int128);
        s += va_arg(ap, int);
    }
    va_end(ap);
    return s;
}
__int128 h_q_call(__int128 (*f)(__int128, int, __int128)) {
    return f((__int128)1 << 100, 7, -((__int128)3 << 70)) + 1;
}
__int128 h_q_vcall(__int128 (*f)(int, ...)) {
    return f(3, (__int128)5 << 64, 1, (__int128)-7, 2, (__int128)1 << 120, 3);
}
#endif
