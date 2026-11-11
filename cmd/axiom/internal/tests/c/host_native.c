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
