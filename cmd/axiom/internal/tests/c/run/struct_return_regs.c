/* Structures returned in registers (two to four of them, of each class),
   called directly (inlined or not), through function pointers, from
   several returns, and by recursion that changes the result after the
   call (not a tail call); their fields used one by one and as a whole. */
#include "host.h"

typedef struct { double a, b; } dd;
typedef struct { long long a; double b; } ld;
typedef struct { double a; long long b; } dl;
typedef struct { int a, b, c; } i3;
typedef struct { float a, b, c; } f3;
typedef struct { float a, b, c, d; } f4;
typedef struct { double a, b, c, d; } d4;
typedef struct { char c; short s; long long l; } csl;
typedef struct { unsigned char b[13]; } b13;

static dd mk_dd(double x) { dd r = { x, -x * 2 }; return r; }
static ld mk_ld(long long x) { ld r = { x * 3, (double)x / 4 }; return r; }
static dl mk_dl(double x) { dl r; r.a = x + 0.5; r.b = (long long)(x * 10); return r; }
static i3 mk_i3(int x) {
    i3 r = { x, x + 1, x * x };
    if (x < 0) {
        i3 z = { -1, -2, -3 };
        return z;
    }
    return r;
}
static f3 mk_f3(float x) { f3 r = { x, x / 2, x * 3 }; return r; }
static f4 mk_f4(float x) { f4 r = { x, x + 1, x + 2, x + 3 }; return r; }
static d4 mk_d4(double x) { d4 r = { x, x * x, -x, 1 / x }; return r; }
static csl mk_csl(int x) { csl r = { (char)(x & 0x7f), (short)(x * 7), (long long)x << 33 }; return r; }
static b13 mk_b13(int x) {
    b13 r;
    for (int i = 0; i < 13; i++) r.b[i] = (unsigned char)(x + i * 17);
    return r;
}

/* recursion with the result changed after the call */
static i3 walk(int n) {
    if (n == 0) {
        i3 r = { 0, 0, 1 };
        return r;
    }
    i3 r = walk(n - 1);
    r.a += n;
    r.c *= 2;
    return r;
}
static dd fib2(int n) {
    if (n < 2) {
        dd r = { n, 1 };
        return r;
    }
    dd x = fib2(n - 1), y = fib2(n - 2);
    dd r = { x.a + y.a, x.b + y.b };
    return r;
}

static dd (*volatile pdd)(double) = mk_dd;
static i3 (*volatile pi3)(int) = mk_i3;
static f3 (*volatile pf3)(float) = mk_f3;
static d4 (*volatile pd4)(double) = mk_d4;
static csl (*volatile pcsl)(int) = mk_csl;
static b13 (*volatile pb13)(int) = mk_b13;
static ld (*volatile pld)(long long) = mk_ld;
static dl (*volatile pdl)(double) = mk_dl;
static f4 (*volatile pf4)(float) = mk_f4;

static long long sum_b13(b13 v) {
    long long s = 0;
    for (int i = 0; i < 13; i++) s = s * 3 + v.b[i];
    return s;
}

int main(void) {
    dd a = mk_dd(1.5), a2 = pdd(2.5);
    print_f64(a.a + a.b);
    print_f64(a2.a * a2.b);
    ld b = mk_ld(9), b2 = pld(-8);
    print_i64(b.a + b2.a);
    print_f64(b.b + b2.b);
    dl c = mk_dl(1.25), c2 = pdl(3.0);
    print_f64(c.a + c2.a);
    print_i64(c.b + c2.b);
    i3 d = mk_i3(5), d2 = pi3(-5), d3 = pi3(7);
    print_i64(d.a + d.b + d.c);
    print_i64(d2.a * 100 + d2.b * 10 + d2.c);
    print_i64(d3.c);
    f3 e = mk_f3(2), e2 = pf3(4);
    print_f64(e.a + e.b + e.c + e2.c);
    f4 g = mk_f4(1), g2 = pf4(-1);
    print_f64(g.a * g.b * g.c * g.d + g2.d);
    d4 h = mk_d4(4), h2 = pd4(0.5);
    print_f64(h.a + h.b + h.c + h.d + h2.d);
    csl k = mk_csl(1000), k2 = pcsl(-3);
    print_i64(k.c + k.s + k.l);
    print_i64(k2.c + k2.s + (k2.l >> 33));
    b13 m = mk_b13(3), m2 = pb13(200);
    print_i64(sum_b13(m) + sum_b13(m2));
    i3 w = walk(10);
    print_i64(w.a * 10000 + w.b * 100 + w.c);
    dd f = fib2(15);
    print_f64(f.a);
    print_f64(f.b);
    /* a whole copy of a result */
    i3 cp;
    cp = pi3(3);
    i3 cp2 = cp;
    cp2.b = 0;
    print_i64(cp.a + cp.b + cp.c + cp2.a + cp2.b + cp2.c);
    return 0;
}
/* expect: -1.5 */
/* expect: -12.5 */
/* expect: 3 */
/* expect: 0.25 */
/* expect: 5.25 */
/* expect: 42 */
/* expect: 36 */
/* expect: -123 */
/* expect: 49 */
/* expect: 21 */
/* expect: 26 */
/* expect: 18.25 */
/* expect: 8589934599104 */
/* expect: 101 */
/* expect: 172855903 */
/* expect: 551024 */
/* expect: 610 */
/* expect: 987 */
/* expect: 28 */
