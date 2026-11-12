/* strtoul (<stdlib.h>, C23 7.24.1.7): the subject sequence (__axc_strtoint.c)
   as unsigned long, negated in its type if it had a minus sign; out of range,
   ULONG_MAX and errno ERANGE. */
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "axc/core_impl.h"

unsigned long strtoul(const char *restrict nptr, char **restrict endptr, int base) {
    int neg, over;
    unsigned long long v = __axc_strtoint(nptr, endptr, base, ULONG_MAX, &neg, &over, 0);
    if (over) {
        errno = ERANGE;
        return ULONG_MAX;
    }
    return neg ? (unsigned long)(0 - (unsigned long)v) : (unsigned long)v;
}
