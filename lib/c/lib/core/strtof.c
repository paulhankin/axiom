/* strtof (<stdlib.h>, C23 7.24.1.5): as strtod, rounded once to float (in the current rounding
   direction). */
#include <errno.h>
#include <stdlib.h>
#include "axc/core_impl.h"

/* (another rounding direction than to nearest: strtod.c) */
float __axc_strtof_dir(const char *s, char **end, int *range);
int __axc_fe_round(void);

float strtof(const char *restrict nptr, char **restrict endptr) {
    int range;
    unsigned long long b;
    float f;
    if (__axc_fe_round() != 0) {
        f = __axc_strtof_dir(nptr, endptr, &range);
        if (range)
            errno = ERANGE;
        return f;
    }
    b = __axc_strtofp(nptr, endptr, 32, &range);
    unsigned char *fp = (unsigned char *)&f;
    int k;
    for (k = 0; k < 4; k++)
        fp[k] = (unsigned char)(b >> (8 * k));
    if (range)
        errno = ERANGE;
    return f;
}
