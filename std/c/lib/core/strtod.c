/* strtod (<stdlib.h>, C23 7.24.1.5): the subject sequence's value
   (__axc_strtofp.c), correctly rounded, in the current rounding
   direction (F.5); HUGE_VAL with the sign, or a value
   no greater than DBL_MIN, and errno ERANGE out of range. */
#include <errno.h>
#include <stdlib.h>
#include "axc/core_impl.h"

/* (the conversion in a rounding direction other than to nearest, F.5: the
   standard I/O's, std/c/lib/stdio/scanfloat.c; the mathematics part's
   reading of the direction) */
double __axc_strtod_dir(const char *s, char **end, int *range);
int __axc_fe_round(void);

double strtod(const char *restrict nptr, char **restrict endptr) {
    int range;
    unsigned long long b;
    double d;
    if (__axc_fe_round() != 0) {
        d = __axc_strtod_dir(nptr, endptr, &range);
        if (range)
            errno = ERANGE;
        return d;
    }
    b = __axc_strtofp(nptr, endptr, 64, &range);
    unsigned char *dp = (unsigned char *)&d;
    int k;
    for (k = 0; k < 8; k++)
        dp[k] = (unsigned char)(b >> (8 * k));
    if (range)
        errno = ERANGE;
    return d;
}
