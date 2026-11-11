/* flags: --c-std=c99 */
/* A variadic function with a variable length array (its frame grows at
   run time): va_arg still finds the saved registers and the stack
   arguments, before and after the array is made, and a structure result
   in registers comes back from such a function. */
#include <stdarg.h>
#include "host.h"

struct dd { double a, b; };

static long long f(int n, ...) {
    va_list ap;
    long long s;
    int i;
    va_start(ap, n);
    s = va_arg(ap, int);
    {
        int v[n];
        for (i = 0; i < n; i++) v[i] = va_arg(ap, int);
        for (i = 0; i < n; i++) s = s * 10 + v[i];
    }
    s = s * 10 + (long long)va_arg(ap, double);
    va_end(ap);
    return s;
}

static struct dd g(int n, ...) {
    va_list ap;
    double w[n + 1];
    struct dd r;
    int i;
    va_start(ap, n);
    for (i = 0; i <= n; i++) w[i] = va_arg(ap, double);
    va_end(ap);
    r.a = w[0];
    r.b = w[n];
    return r;
}

int main(void) {
    struct dd r;
    print_i64(f(3, 9, 1, 2, 3, 4.5));
    print_i64(f(9, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 7.0));
    r = g(9, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.5);
    print_f64(r.a + r.b);
    return 0;
}

/* expect: 91234 */
/* expect: 11234567897 */
/* expect: 11.5 */
