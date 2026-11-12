/* native-only */
/* linux-only */
/* long double across the platform's C calling convention, against the
   system's C compiler (c/host_native.c's h_ld_ functions) in its format
   (the x87's on x86-64 Linux, binary128 on AArch64 Linux): alone, in
   structures and unions of each class, beyond the argument registers, as
   variadic arguments both ways, through function pointers, and through
   the C library's formatted output and conversion. */
#include <float.h>
#include "host.h"

typedef struct { long double x; } h_l1;
typedef struct { char c; long double x; } h_cl;
typedef struct { long double a, b; } h_l2;
typedef struct { double d; long double x; } h_dl;
typedef union { long double x; long long l[2]; } h_ul;
long double h_ld_many(int i1, long double a, double d1, long double b, int i2, long double c, long double d,
                      long double e, long double f, long double g, long double h, long double i, long double j,
                      float f1, long double k);
h_l1 h_ld_l1(h_l1 s);
h_cl h_ld_cl(h_cl s);
h_l2 h_ld_l2(h_l2 s);
h_dl h_ld_dl(h_dl s);
h_ul h_ld_ul(h_ul u);
long double h_ld_va(int n, ...);
long double h_ld_call(long double (*f)(long double, int, long double), long double a, long double b);
long double h_ld_vcall(long double (*f)(int, ...));
h_l2 h_ld_scall(h_l2 (*f)(h_l2, long double));
int h_ld_order(long double a, long double b);
int h_ld_eps(long double x);
int snprintf(char *s, size_t n, const char *f, ...);
long double strtold(const char *s, char **end);

static long double cb(long double a, int k, long double b) {
    return a * k + b;
}

#include <stdarg.h>
static long double vcb(int n, ...) {
    va_list ap;
    long double s;
    double d;
    int k;
    h_l2 p;
    va_start(ap, n);
    s = va_arg(ap, long double);
    d = va_arg(ap, double);
    k = va_arg(ap, int);
    p = va_arg(ap, h_l2);
    va_end(ap);
    return s * 1000 + d * 100 + k * 10 + p.a + p.b + n;
}

static h_l2 scb(h_l2 p, long double k) {
    h_l2 r = { p.a + k, p.b * k };
    return r;
}

int main(void) {
    h_l1 l1 = { 2.25L };
    h_cl cl = { 3, 0.5L };
    h_l2 l2 = { 1.5L, -2.5L };
    h_dl dl = { 0.25L, 8.0L };
    h_ul ul;
    char buf[80];
    long double third = 1.0L / 3;
    print_f64((double)h_ld_many(1, 0.5L, 0.25, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0.125f, 1));
    print_f64((double)h_ld_l1(l1).x);
    cl = h_ld_cl(cl);
    print_i64(cl.c);
    print_f64((double)cl.x);
    l2 = h_ld_l2(l2);
    print_f64((double)l2.a);
    print_f64((double)l2.b);
    dl = h_ld_dl(dl);
    print_f64(dl.d);
    print_f64((double)dl.x);
    ul.x = 3.0L;
    print_f64((double)h_ld_ul(ul).x);
    print_f64((double)h_ld_va(8, 1.0L, 2.0, 3, l2, 0.5L, 0.25, -1, l2));
    print_f64((double)h_ld_call(cb, 1.5L, 0.25L));
    print_f64((double)h_ld_vcall(vcb));
    l2 = h_ld_scall(scb);
    print_f64((double)l2.a);
    print_f64((double)l2.b);
    print_i64(h_ld_order(third, 0.5L) + h_ld_order(0.5L, third) * 10 + h_ld_order(third, third) * 100);
    print_i64(h_ld_eps(1 + LDBL_EPSILON));
    /* the C library's: printed with enough digits and read back, the same value */
    snprintf(buf, sizeof buf, "%.*Lg", DECIMAL_DIG, third);
    print_i64(strtold(buf, 0) == third);
    snprintf(buf, sizeof buf, "%.3Lf %Lg", 2.5L, -LDBL_MIN * LDBL_EPSILON / LDBL_MIN);
    print_i64(buf[0] == '2' && buf[4] == '0' && buf[6] == '-');
    return 0;
}

/* expect: 68.875 */
/* expect: 5.5 */
/* expect: 122 */
/* expect: 3.5 */
/* expect: -3.5 */
/* expect: 2.5 */
/* expect: 4.5 */
/* expect: 7.75 */
/* expect: 0.75 */
/* expect: 397.5 */
/* expect: 9.5 */
/* expect: 1536.25 */
/* expect: 75 */
/* expect: 0.4375 */
/* expect: 241 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
