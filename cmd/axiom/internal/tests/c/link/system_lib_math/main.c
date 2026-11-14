/* --c-lib=system before C23: the system's <math.h> spells HUGE_VAL,
   HUGE_VALF, HUGE_VALL and INFINITY, for a compiler that is not its own, as
   floating constants beyond the range of their types (infinities under
   Annex F): from its macros they are the infinities it means. Its
   comparison macros (7.12.14), statement expressions for such a compiler,
   are the compiler's built-ins under an ISO standard too. */
/* flags: --c-lib=system --c-std=c11 */
#include <math.h>
#include <stdio.h>

int main(void) {
    float f = HUGE_VALF;
    long double l = HUGE_VALL;
    volatile double n = NAN, one = 1.0;
    printf("%g %g %Lg %g\n", HUGE_VAL, f, l, -INFINITY); /* expect: inf inf inf -inf */
    printf("%d %d\n", isinf(HUGE_VAL) != 0, isnan(NAN) != 0); /* expect: 1 1 */
    printf("%d %d %d %d %d %d\n", isgreater(2.0, one), isless(n, one), isgreaterequal(one, 1.0f),
           islessequal(one, n), islessgreater(one, 2.0L), isunordered(n, one)); /* expect: 1 0 1 0 1 1 */
    return 0;
}
