/* Variadic functions defined in C (va_start with the last parameter, va_arg,
   va_end): integers of every width after the promotions, pointers, doubles,
   more arguments than the registers hold, named parameters of each kind
   before the '...', and calls through a pointer. */
#include <stdarg.h>
#include "host.h"

static int count(int n, ...) { return n; }

static long sum_ints(int n, ...) {
    va_list ap;
    long s = 0;
    int i;
    va_start(ap, n);
    for (i = 0; i < n; i++) s += va_arg(ap, int);
    va_end(ap);
    return s;
}

static double sum_doubles(int n, ...) {
    va_list ap;
    double s = 0;
    int i;
    va_start(ap, n);
    for (i = 0; i < n; i++) s = s * 2 + va_arg(ap, double);
    va_end(ap);
    return s;
}

/* each letter says the next argument's type */
static long long fmt(const char *f, ...) {
    va_list ap;
    long long s = 0;
    va_start(ap, f);
    for (; *f; f++) {
        if (*f == 'i') s = s * 10 + va_arg(ap, int);
        else if (*f == 'u') s = s * 10 + va_arg(ap, unsigned);
        else if (*f == 'l') s = s * 10 + va_arg(ap, long);
        else if (*f == 'q') s = s * 10 + va_arg(ap, long long);
        else if (*f == 'd') s = s * 10 + (long long)va_arg(ap, double);
        else if (*f == 'p') s = s * 10 + *va_arg(ap, int *);
        else if (*f == 's') s = s * 10 + va_arg(ap, const char *)[1] - '0';
        else if (*f == 'z') s = s * 10 + (long long)va_arg(ap, size_t);
    }
    va_end(ap);
    return s;
}

/* named parameters of several kinds before the variadic ones */
static double named(char c, double d, short s, float f, ...) {
    va_list ap;
    double r = c + d + s + f;
    va_start(ap, f);
    r = r * 100 + va_arg(ap, int);
    r = r * 100 + va_arg(ap, double);
    va_end(ap);
    return r;
}

int main(void) {
    long long (*pf)(const char *, ...) = fmt;
    int seven = 7;
    char c = 3;
    short sh = -2;
    unsigned char uc = 200;
    float fl = 2.5f;
    print_i64(count(2, 1, 2));
    print_i64(count(0));
    print_i64(sum_ints(5, 1, 2, 3, 4, 5));
    print_i64(sum_ints(14, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14));
    print_i64(sum_ints(3, c, sh, uc));
    print_f64(sum_doubles(10, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.5));
    print_f64(sum_doubles(2, fl, 1.25f));
    print_i64(fmt("iuqdps", 1, 2u, 3LL, 4.75, &seven, "x5"));
    print_i64(pf("lzi", 9L, (size_t)8, -1));
    print_i64(fmt("ididididididididid", 1, 1.0, 2, 2.0, 3, 3.0, 4, 4.0, 5, 5.0, 6, 6.0, 7, 7.0, 8, 8.0, 9, 9.0));
    print_f64(named(1, 2.5, 3, 0.5f, 4, 5.0));
    return 0;
}

/* expect: 2 */
/* expect: 0 */
/* expect: 15 */
/* expect: 105 */
/* expect: 201 */
/* expect: 682.5 */
/* expect: 6.25 */
/* expect: 123475 */
/* expect: 979 */
/* expect: 112233445566778899 */
/* expect: 70405 */
