#include <stdarg.h>
int sum(int n, ...) {
    va_list ap;
    int s = 0;
    va_start(ap, n);
    for (int i = 0; i < n; i++) s += va_arg(ap, int);
    va_end(ap);
    return s;
}
int checked_sum(int n, ...) { return -1000; }
