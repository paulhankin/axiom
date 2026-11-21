#include <stdarg.h>
struct pt { int x, y; };
int noted;
/* the external function, which a call through a pointer reaches */
int sum(int n, ...) {
    va_list ap;
    int s = 0;
    va_start(ap, n);
    for (int i = 0; i < n; i++) s += va_arg(ap, int);
    va_end(ap);
    return s;
}
/* what the inlined wrapper calls */
int checked_sum(int n, ...) {
    va_list ap;
    int s = 1000;
    va_start(ap, n);
    for (int i = 0; i < n; i++) s += va_arg(ap, int);
    va_end(ap);
    return s;
}
/* n (4): a double, a long long, a string and a structure */
double mix(int n, ...) {
    va_list ap;
    va_start(ap, n);
    double d = va_arg(ap, double);
    long long l = va_arg(ap, long long);
    const char *s = va_arg(ap, const char *);
    struct pt p = va_arg(ap, struct pt);
    va_end(ap);
    int len = 0;
    while (s[len]) len++;
    return d + (double)l + len + p.x * 10 + p.y + (n == 4 ? 0 : 1000);
}
