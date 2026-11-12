/* strtoll (<stdlib.h>, C23 7.24.1.7): the subject sequence (__axc_strtoint.c)
   as long long; out of range, LLONG_MIN or LLONG_MAX and errno ERANGE. */
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "axc/core_impl.h"

long long strtoll(const char *restrict nptr, char **restrict endptr, int base) {
    int neg, over;
    /* (the magnitude, up to the most negative value's) */
    unsigned long long max = (unsigned long long)LLONG_MAX + 1;
    unsigned long long v = __axc_strtoint(nptr, endptr, base, max, &neg, &over, 0);
    if (over || (!neg && v == max)) {
        errno = ERANGE;
        return neg ? LLONG_MIN : LLONG_MAX;
    }
    return neg ? (long long)(0 - v) : (long long)v;
}
