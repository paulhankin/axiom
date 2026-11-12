/* div (<stdlib.h>, C23 7.24.6.2): the quotient (truncated toward zero) and
   the remainder of numer / denom, in one structure. */
#include <stdlib.h>

div_t div(int numer, int denom) {
    div_t r;
    r.quot = numer / denom;
    r.rem = numer % denom;
    return r;
}
