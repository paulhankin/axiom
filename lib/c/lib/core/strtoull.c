/* strtoull (<stdlib.h>, C23 7.24.1.7): the subject sequence (__axc_strtoint.c)
   as unsigned long long, negated in its type if it had a minus sign; out of range,
   ULLONG_MAX and errno ERANGE. */
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "axc/core_impl.h"

unsigned long long strtoull(const char *restrict nptr, char **restrict endptr, int base) {
    int neg, over;
    unsigned long long v = __axc_strtoint(nptr, endptr, base, ULLONG_MAX, &neg, &over, 0);
    if (over) {
        errno = ERANGE;
        return ULLONG_MAX;
    }
    return neg ? (unsigned long long)(0 - (unsigned long long)v) : (unsigned long long)v;
}
