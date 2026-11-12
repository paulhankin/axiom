/* __axc_strtoul_c23 (<stdlib.h>, C23 7.24.1.7): the subject sequence (__axc_strtoint.c)
   as unsigned long, negated in its type if it had a minus sign; out of range,
   ULONG_MAX and errno ERANGE. In C23 mode, with the 0b prefixes
   (<stdlib.h> maps the name here). */
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "axc/core_impl.h"

unsigned long __axc_strtoul_c23(const char *restrict nptr, char **restrict endptr, int base) {
    int neg, over;
    unsigned long long v = __axc_strtoint(nptr, endptr, base, ULONG_MAX, &neg, &over, 1);
    if (over) {
        errno = ERANGE;
        return ULONG_MAX;
    }
    return neg ? (unsigned long)(0 - (unsigned long)v) : (unsigned long)v;
}
