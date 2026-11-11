/* flags: --target=wasm64 */
/* wasm-only */
/* wasm64 (pointers and size_t of 64 bits): main's arguments, variadic
   functions with pointers, longs and structures in the argument buffer, and
   structures by value. */
#include <stdarg.h>
#include "host.h"

struct pl { char *p; long n; };
struct one { double d; };

static long long f(int n, ...) {
    va_list ap;
    long long s = 0;
    va_start(ap, n);
    s = va_arg(ap, long);
    s = s * 10 + *va_arg(ap, char *);
    {
        struct pl x = va_arg(ap, struct pl);
        s = s * 10 + x.p[1] + x.n;
    }
    s = s * 10 + (long long)va_arg(ap, struct one).d;
    s = s * 10 + (long long)va_arg(ap, size_t);
    va_end(ap);
    return s;
}

static struct pl make(char *p, long n) {
    struct pl r;
    r.p = p;
    r.n = n;
    return r;
}

int main(int argc, char **argv) {
    char s[] = { 1, 2, 3 };
    struct one o = { 4.0 };
    print_i64(sizeof(void *) * 10 + sizeof(size_t));
    print_i64(argc >= 0 && (argc == 0 || argv[0] != 0));
    print_i64(f(0, 5L, s, make(s, 1), o, (size_t)6));
    print_str("wide", 4);
    return 0;
}

/* expect: 88 */
/* expect: 1 */
/* expect: 51346 */
/* expect: wide */
