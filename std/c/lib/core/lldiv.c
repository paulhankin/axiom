/* lldiv (<stdlib.h>, C23 7.24.6.2): the quotient (truncated toward zero) and
   the remainder of numer / denom, in one structure. */
#include <stdlib.h>

lldiv_t lldiv(long long numer, long long denom) {
    lldiv_t r;
    r.quot = numer / denom;
    r.rem = numer % denom;
    return r;
}
