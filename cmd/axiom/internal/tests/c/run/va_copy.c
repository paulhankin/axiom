/* flags: --c-std=c99 */
/* va_copy, a va_list passed to another function (which takes it as a
   parameter, and va_arg goes on there), va_start again after va_end, and
   a va_list member of a structure. */
#include <stdarg.h>
#include "host.h"

static long long next_two(va_list ap) {
    long long a = va_arg(ap, int);
    return a * 10 + va_arg(ap, int);
}

static long long vsum(int n, va_list ap) {
    long long s = 0;
    while (n-- > 0) s = s * 10 + va_arg(ap, int);
    return s;
}

struct holder { va_list ap; int n; };

static long long use_holder(struct holder *h) { return vsum(h->n, h->ap); }

static long long twice(int n, ...) {
    va_list ap, aq;
    long long r;
    va_start(ap, n);
    va_copy(aq, ap);
    r = vsum(n, ap);
    r = r * 1000 + vsum(n, aq);
    va_end(aq);
    va_end(ap);
    va_start(ap, n);
    r = r * 100 + next_two(ap);
    va_end(ap);
    return r;
}

static long long held(int n, ...) {
    struct holder h;
    long long r;
    h.n = n;
    va_start(h.ap, n);
    r = use_holder(&h);
    va_end(h.ap);
    return r;
}

static double mid_copy(int n, ...) {
    va_list ap, aq;
    double r;
    va_start(ap, n);
    (void)va_arg(ap, double);
    va_copy(aq, ap);
    r = va_arg(ap, double) * 10;
    r += va_arg(aq, double);
    va_end(aq);
    va_end(ap);
    return r;
}

int main(void) {
    print_i64(twice(3, 1, 2, 3));
    print_i64(held(4, 4, 3, 2, 1));
    print_f64(mid_copy(3, 1.0, 2.5, 4.0));
    return 0;
}

/* expect: 12312312 */
/* expect: 4321 */
/* expect: 27.5 */
