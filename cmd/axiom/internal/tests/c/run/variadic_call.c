/* native-only */
/* linux-only */
/* Calls of a variadic function (the C library's snprintf, which the native
   tests link): the default argument promotions of the variable arguments. */
#include "host.h"

int snprintf(char *buf, size_t n, const char *fmt, ...);

int main(void) {
    char buf[100];
    char c = 'q';
    short s = -7;
    float f = 1.5f;
    long long ll = -1234567890123LL;
    unsigned u = 4000000000u;
    int n = snprintf(buf, sizeof buf, "%d %c %d %.2f %lld %u %s %p", 42, c, s, f, ll, u, "str", (void *)0);
    print_i64(n);
    print_str(buf, (size_t)n);
    n = snprintf(buf, sizeof buf, "%g|%5.1f|%x", 0.1, 2.25, 255);
    print_str(buf, (size_t)n);
    return 0;
}

/* expect: 48 */
/* expect: 42 q -7 1.50 -1234567890123 4000000000 str (nil) */
/* expect: 0.1|  2.2|ff */
