/* strtod (<stdlib.h>, C23 7.24.1.5): the subject sequence's value
   (__axc_strtofp.c), correctly rounded; HUGE_VAL with the sign, or a value
   no greater than DBL_MIN, and errno ERANGE out of range. */
#include <errno.h>
#include <stdlib.h>
#include "axc/core_impl.h"

double strtod(const char *restrict nptr, char **restrict endptr) {
    int range;
    unsigned long long b = __axc_strtofp(nptr, endptr, 64, &range);
    double d;
    unsigned char *dp = (unsigned char *)&d;
    int k;
    for (k = 0; k < 8; k++)
        dp[k] = (unsigned char)(b >> (8 * k));
    if (range)
        errno = ERANGE;
    return d;
}
