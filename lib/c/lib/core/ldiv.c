/* ldiv (<stdlib.h>, C23 7.24.6.2): the quotient (truncated toward zero) and
   the remainder of numer / denom, in one structure. */
#include <stdlib.h>

ldiv_t ldiv(long numer, long denom) {
    ldiv_t r;
    r.quot = numer / denom;
    r.rem = numer % denom;
    return r;
}
