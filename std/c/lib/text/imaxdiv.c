/* imaxdiv (C17 7.8.2.2): the quotient truncated toward zero, and the
   remainder with the dividend's sign. */
#include <inttypes.h>

imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom) {
    imaxdiv_t r;
    r.quot = numer / denom;
    r.rem = numer % denom;
    return r;
}
