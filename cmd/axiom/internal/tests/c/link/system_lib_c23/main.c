/* --c-lib=system in C23: INFINITY and NAN are the compiler's <float.h>'s
   and the system's <math.h>'s too; either may come first. <limits.h> is the
   system's, with what C23 adds that it leaves to the compiler
   (BITINT_MAXWIDTH), and its POSIX limits. */
/* flags: --c-lib=system --c-std=c23 */
/* ours-flags: --c-lib=system --c-std=c23 */
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include "second.h"

int main(void) {
    printf("%g %d\n", INFINITY, isnan(NAN) != 0); /* expect: inf 1 */
    printf("%g\n", second()); /* expect: -inf */
    printf("%d %d %ld %d\n", BITINT_MAXWIDTH, BOOL_WIDTH, __STDC_VERSION_LIMITS_H__, _POSIX_ARG_MAX); /* expect: 65535 1 202311 4096 */
    return 0;
}
