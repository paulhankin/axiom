/* native-only */
/* linux-only */
/* The platform's C calling convention against the system's C compiler
   (c/host_native.c's h_ functions): structures and unions of each size
   class passed and returned by value both ways, callbacks through function
   pointers, variadic calls both ways, and a va_list passed to it. */
#include <stdarg.h>
#include "host.h"

typedef struct { char c[3]; } h_c3;
typedef struct { int a, b; } h_ii;
typedef struct { float a, b, c; } h_f3;
typedef struct { double a, b; } h_dd;
typedef struct { double d; int i; } h_di;
typedef struct { long long a[3]; } h_l3;
typedef union { double d; long long l; } h_ud;
h_c3 h_make_c3(int k);
h_ii h_make_ii(int k);
h_f3 h_make_f3(float k);
h_dd h_make_dd(double k);
h_di h_make_di(double k);
h_l3 h_make_l3(long long k);
h_ud h_make_ud(double k);
long long h_sum(int n, h_c3 c, h_ii i, h_f3 f, double x, h_dd d, h_di m, h_l3 l, h_ud u, float y);
long long h_call(h_dd (*f)(h_dd, h_l3, int), h_l3 (*g)(h_ii, h_c3));
long long h_va(int n, ...);
long long h_vcall(long long (*f)(int, ...));
long long h_vlist(int n, va_list ap);

static h_dd cb1(h_dd d, h_l3 l, int k) {
    h_dd r;
    r.a = d.a * k + l.a[0];
    r.b = d.b + l.a[2];
    return r;
}

static h_l3 cb2(h_ii i, h_c3 c) {
    h_l3 r;
    r.a[0] = i.a * 100;
    r.a[1] = i.b * 10;
    r.a[2] = c.c[0] + c.c[1] + c.c[2];
    return r;
}

static long long mine(int n, ...) {
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

static long long list(int n, ...) {
    va_list ap;
    long long r;
    va_start(ap, n);
    r = h_vlist(n, ap);
    va_end(ap);
    return r;
}

int main(void) {
    h_c3 c = h_make_c3(5);
    h_ii i = h_make_ii(7);
    h_f3 f = h_make_f3(1.5f);
    h_dd d = h_make_dd(2.0);
    h_di m = h_make_di(3.0);
    h_l3 l = h_make_l3(4);
    h_ud u = h_make_ud(9.0);
    print_i64(c.c[0] * 100 + c.c[1] * 10 + c.c[2]);
    print_i64(i.a * 10 - i.b);
    print_f64(f.a + f.b + f.c);
    print_f64(d.a * 10 + d.b);
    print_f64(m.d + m.i);
    print_i64(l.a[0] + l.a[1] + l.a[2]);
    print_f64(u.d);
    print_i64(h_sum(1, c, i, f, 2.5, d, m, l, u, 0.5f));
    print_i64(h_call(cb1, cb2));
    print_i64(h_va(2, d, l, 3, f, d, l, 4, f));
    print_i64(h_vcall(mine));
    print_i64(list(3, 1, 0.5, 2, 1.5, 3, 2.25));
    return 0;
}

/* expect: 567 */
/* expect: 77 */
/* expect: 9 */
/* expect: 22.5 */
/* expect: 9 */
/* expect: 444 */
/* expect: 9 */
/* expect: 461 */
/* expect: 67697 */
/* expect: 5051 */
/* expect: 1718 */
/* expect: 135 */
