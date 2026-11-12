/* Special values in complex arithmetic (Annex G.5): a real operand is not
   made complex, so signed zeros of the other part survive; products and
   quotients with infinite operands are infinite, a quotient by zero is
   infinite, by an infinity zero, even where the formulas give NaN + iNaN;
   the division does not overflow forming the divisor's squared magnitude. */
#include <complex.h>
#include "host.h"

#define INF ((double)__builtin_inff())
#define NAN ((double)__builtin_nanf(""))

/* the class of x: NaN, an infinity, a signed zero, or the value */
static void show(double x) {
    if (x != x) print_str("nan", 3);
    else if (x == INF) print_str("inf", 3);
    else if (x == -INF) print_str("-inf", 4);
    else if (x == 0 && 1 / x < 0) print_str("-0", 2);
    else print_f64(x);
}

static void pc(double complex z) {
    show(creal(z));
    show(cimag(z));
}

static void pf(float complex z) {
    show(crealf(z));
    show(cimagf(z));
}

static volatile double vz = 0.0, vinf = __builtin_inff(), vnan = __builtin_nanf(""), vbig = 1e300;

int main(void) {
    double complex z0 = CMPLX(vz, vz);
    double complex in = CMPLX(vinf, vnan);
    double complex ni = CMPLX(vnan, vinf);
    double complex one = CMPLX(1.0, 1.0);
    /* G.5: signed zeros kept with a real operand */
    pc(1.0 + CMPLX(2.0, -0.0));
    pc(CMPLX(2.0, -0.0) - 1.0);
    pc(1.0 - CMPLX(2.0, 0.0));
    pc(2.0 * CMPLX(3.0, -0.0));
    pc(CMPLX(3.0, -0.0) / 2.0);
    pc(CMPLX(-0.0, -0.0) + CMPLX(-0.0, -0.0));
    pc(-CMPLX(0.0, 0.0));
    /* infinite factors: an infinite product */
    pc(in * one);
    pc(one * ni);
    pc(in * in);
    pc(CMPLX(vinf, vinf) * CMPLX(vz, 1.0));
    pc(CMPLX(vnan, vnan) * one);
    pc(2.0 * in);
    /* overflowing products */
    pc(CMPLX(vbig, vbig) * CMPLX(vbig, -vbig));
    pc(CMPLX(vbig, vnan) * CMPLX(vbig, vbig));
    /* quotients */
    pc(one / z0);
    pc(CMPLX(-1.0, vz) / z0);
    pc(CMPLX(vnan, vnan) / z0);
    pc(in / one);
    pc(CMPLX(vinf, vinf) / CMPLX(3.0, -4.0));
    pc(one / in);
    pc(CMPLX(2.0, -3.0) / CMPLX(vinf, vinf));
    pc(CMPLX(2.0, -3.0) / CMPLX(-vinf, 1.0));
    pc(1.0 / CMPLX(vz, vinf));
    pc(in / in);
    /* no spurious overflow or underflow in the divisor */
    pc(CMPLX(1e300, 1e300) / CMPLX(2e300, 2e300));
    pc(CMPLX(1e-300, 2e-300) / CMPLX(1e-300, 1e-300));
    pc(CMPLX(3.0, 4.0) / CMPLX(1e200, 1e-200));
    pc(CMPLX(6.0, 8.0) / CMPLX(2.0, 0.0));
    pc(CMPLX(-2.0, 2.0) / CMPLX(0.0, 2.0));
    /* float */
    pf(CMPLXF(1e30f, 1e30f) / CMPLXF(1e30f, 1e30f));
    pf(CMPLXF(__builtin_inff(), 0.0f) * CMPLXF(0.0f, 1.0f));
    pf(CMPLXF(1.0f, 2.0f) / CMPLXF(0.0f, 0.0f));
    pf(CMPLXF(3.0f, -0.0f) * 2.0f);
    /* the same, folded */
    pc(CMPLX(INF, NAN) * CMPLX(1.0, 1.0));
    pc(CMPLX(1.0, 1.0) / CMPLX(0.0, 0.0));
    pc(CMPLX(1.0, 1.0) / CMPLX(INF, NAN));
    pc(1.0 + CMPLX(2.0, -0.0));
    return 0;
}

/* expect: 3 */
/* expect: -0 */
/* expect: 1 */
/* expect: -0 */
/* expect: -1 */
/* expect: -0 */
/* expect: 6 */
/* expect: -0 */
/* expect: 1.5 */
/* expect: -0 */
/* expect: -0 */
/* expect: -0 */
/* expect: -0 */
/* expect: -0 */
/* expect: inf */
/* expect: inf */
/* expect: -inf */
/* expect: inf */
/* expect: inf */
/* expect: nan */
/* expect: -inf */
/* expect: inf */
/* expect: nan */
/* expect: nan */
/* expect: inf */
/* expect: nan */
/* expect: inf */
/* expect: nan */
/* expect: inf */
/* expect: inf */
/* expect: inf */
/* expect: inf */
/* expect: -inf */
/* expect: nan */
/* expect: nan */
/* expect: nan */
/* expect: inf */
/* expect: -inf */
/* expect: nan */
/* expect: inf */
/* expect: 0 */
/* expect: 0 */
/* expect: -0 */
/* expect: -0 */
/* expect: -0 */
/* expect: 0 */
/* expect: 0 */
/* expect: -0 */
/* expect: nan */
/* expect: nan */
/* expect: 0.5 */
/* expect: 0 */
/* expect: 1.5 */
/* expect: 0.5 */
/* expect: 3e-200 */
/* expect: 4e-200 */
/* expect: 3 */
/* expect: 4 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: nan */
/* expect: inf */
/* expect: inf */
/* expect: inf */
/* expect: 6 */
/* expect: -0 */
/* expect: inf */
/* expect: inf */
/* expect: inf */
/* expect: inf */
/* expect: 0 */
/* expect: 0 */
/* expect: 3 */
/* expect: -0 */
