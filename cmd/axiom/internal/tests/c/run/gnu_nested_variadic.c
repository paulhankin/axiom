/* flags: --c-std=gnu17 */
/* Variadic nested functions (a GNU dialect): called directly (the chain
   after the named arguments, which va_start skips) and through pointers
   (trampolines of the C type, which start a va_list for it), with more
   arguments than the registers hold, floating and structure arguments,
   va_copy, a structure result, a va_list passed on, and nested again. */
#include <stdarg.h>
#include "host.h"

struct big { long a, b, c; };
struct two { double x; int y; };

typedef long (*sumfn)(int, ...);
static long call3(sumfn f) { return f(3, 100L, 200L, 300L); }
static long call12(sumfn f) {
    return f(12, 1L, 2L, 3L, 4L, 5L, 6L, 7L, 8L, 9L, 10L, 11L, 12L);
}

static long vsum(int n, va_list ap) {
    long s = 0;
    for (int i = 0; i < n; i++)
        s += va_arg(ap, long);
    return s;
}

int main(void) {
    long base = 1000;
    int count = 0;
    long sum(int n, ...) {
        va_list ap;
        va_start(ap, n);
        long s = base;
        for (int i = 0; i < n; i++)
            s += va_arg(ap, long);
        va_end(ap);
        count++;
        return s;
    }
    print_i64(sum(0));
    print_i64(sum(2, 5L, 7L));
    print_i64(sum(12, 1L, 2L, 3L, 4L, 5L, 6L, 7L, 8L, 9L, 10L, 11L, 12L));
    print_i64(call3(sum));
    print_i64(call12(sum));
    print_i64(count);

    /* floating arguments past the registers, mixed with integers */
    double fsum(const char *kinds, ...) {
        va_list ap, cp;
        va_start(ap, kinds);
        va_copy(cp, ap);
        double s = 0;
        for (const char *k = kinds; *k; k++)
            s += *k == 'd' ? va_arg(ap, double) : (double)va_arg(ap, int);
        /* the copy reads them again */
        double t = 0;
        for (const char *k = kinds; *k; k++)
            t += *k == 'd' ? va_arg(cp, double) : (double)va_arg(cp, int);
        va_end(cp);
        va_end(ap);
        return s * 1000 + t + base;
    }
    print_f64(fsum("dddddddddd", 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0));
    print_f64(fsum("didididi", 0.5, 1, 0.5, 2, 0.5, 3, 0.5, 4));
    double (*fp)(const char *, ...) = fsum;
    print_f64(fp("idddddddddd", 7, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0));

    /* structures as variable arguments, and a structure result */
    struct big pick(int which, ...) {
        va_list ap;
        va_start(ap, which);
        struct big r = { 0, 0, 0 };
        for (int i = 0; i <= which; i++)
            r = va_arg(ap, struct big);
        struct two t = va_arg(ap, struct two);
        va_end(ap);
        r.c += (long)t.x + t.y + base;
        return r;
    }
    struct big b1 = { 1, 2, 3 }, b2 = { 4, 5, 6 };
    struct two w = { 2.5, 10 };
    struct big r = pick(1, b1, b2, w);
    print_i64(r.a * 100 + r.b * 10 + r.c);
    struct big (*pp)(int, ...) = pick;
    r = pp(0, b1, w);
    print_i64(r.a * 100 + r.b * 10 + r.c);

    /* a va_list passed on to another function */
    long fwd(int n, ...) {
        va_list ap;
        va_start(ap, n);
        long s = vsum(n, ap) + base;
        va_end(ap);
        return s;
    }
    print_i64(fwd(3, 1L, 2L, 3L));

    /* nested again: the inner one sees both frames */
    int outer(int k) {
        int mul = 10;
        int inner(int n, ...) {
            va_list ap;
            va_start(ap, n);
            int s = 0;
            for (int i = 0; i < n; i++)
                s += va_arg(ap, int) * mul;
            va_end(ap);
            return s + k + (int)base;
        }
        int (*ip)(int, ...) = inner;
        return inner(2, 1, 2) + ip(9, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    }
    print_i64(outer(5));
    return 0;
}
/* expect: 1000 */
/* expect: 1012 */
/* expect: 1078 */
/* expect: 1600 */
/* expect: 1078 */
/* expect: 5 */
/* expect: 56055 */
/* expect: 13012 */
/* expect: 18017 */
/* expect: 1468 */
/* expect: 1135 */
/* expect: 1006 */
/* expect: 2130 */
