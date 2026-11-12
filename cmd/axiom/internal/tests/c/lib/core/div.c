/* abs, labs, llabs, div, ldiv, lldiv (C17 7.22.6): quotients truncated
   toward zero, the remainder with the sign of the dividend. */
#include <limits.h>
#include <stdlib.h>
#include "print.h"

int main(void) {
    div_t a = div(17, 5), b = div(-17, 5), c = div(17, -5), e = div(-17, -5);
    lldiv_t g = lldiv(-9000000000000000007LL, 10LL);
    ldiv_t h = ldiv(LONG_MIN + 1, -1L);
    print_i64(a.quot * 100 + a.rem); /* expect: 302 */
    print_i64(b.quot * 100 + b.rem); /* expect: -302 */
    print_i64(c.quot * 100 + c.rem); /* expect: -298 */
    print_i64(e.quot * 100 + e.rem); /* expect: 298 */
    print_i64(g.quot); /* expect: -900000000000000000 */
    print_i64(g.rem); /* expect: -7 */
    print_bool(h.quot == LONG_MAX && h.rem == 0); /* expect: true */
    print_i64(abs(INT_MIN + 1) == INT_MAX); /* expect: 1 */
    print_i64(llabs(-9000000000000000000LL)); /* expect: 9000000000000000000 */
    print_i64(labs(-2147483647L)); /* expect: 2147483647 */
    print_i64(abs(0) + abs(-0)); /* expect: 0 */
    return 0;
}
