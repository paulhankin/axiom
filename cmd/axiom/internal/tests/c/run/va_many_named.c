/* A variadic function with more named parameters than registers (and
   than eight: Windows x64's va_start is past every named argument's slot,
   the home area's and the stack's), each kind of variadic argument after
   them: integers, doubles, a structure passed by reference on Windows. */
#include <stdarg.h>
#include "host.h"

struct big {
    double d;
    long long x, y;
};

static long long nine(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, ...) {
    va_list ap;
    va_start(ap, a8);
    long long r = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8;
    struct big b = va_arg(ap, struct big);
    r = r * 1000 + (long long)b.d + b.x + b.y;
    r = r * 1000 + va_arg(ap, int);
    r = r * 1000 + (long long)va_arg(ap, double);
    va_end(ap);
    return r;
}

static long long ten(double a0, int a1, double a2, int a3, double a4, int a5, double a6, int a7, double a8, int a9, ...) {
    va_list ap;
    va_start(ap, a9);
    long long r = (long long)(a0 + a2 + a4 + a6 + a8) + a1 + a3 + a5 + a7 + a9;
    struct big b = va_arg(ap, struct big);
    r = r * 1000 + (long long)b.d + b.x + b.y;
    va_end(ap);
    return r;
}

int main(void) {
    struct big b = {100.0, 20, 3};
    print_i64(nine(1, 2, 3, 4, 5, 6, 7, 8, 9, b, 77, 55.5));
    print_i64(ten(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, b));
    return 0;
}
/* expect: 45123077055 */
/* expect: 55123 */
