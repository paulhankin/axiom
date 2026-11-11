/* flags: --c-std=c89 */
/* <stdarg.h> in C89 (no va_copy): va_arg of promoted types, and of char,
   short and float read as the types they are promoted to. */
#include <stdarg.h>
#include "host.h"

#ifdef va_copy
#error "va_copy is C99's"
#endif

static long f(int n, ...) {
    va_list ap;
    long r;
    int c;
    unsigned u;
    double d;
    va_start(ap, n);
    c = va_arg(ap, int);
    u = va_arg(ap, unsigned int);
    d = va_arg(ap, double);
    r = n + c + (long)u + (long)d + *va_arg(ap, char *);
    va_end(ap);
    return r;
}

int main(void) {
    char c = 'a';
    unsigned short us = 65535;
    float fl = 1.5f;
    print_i64(f(1, c, us, fl, "\002"));
    return 0;
}

/* expect: 65636 */
