/* _Generic over the complex types (C11 6.5.1.1), as a type-generic macro of
   <tgmath.h> selects: the six floating types and the integer types are told
   apart, qualifiers are dropped from the controlling expression, the
   usual arithmetic conversions of two operands give the complex type of the
   common real type, and the selected function is called. */
#include <complex.h>
#include "host.h"

#define KIND(x) _Generic((x), float: 1, double: 2, long double: 3, float complex: 4, double complex: 5, \
                         long double complex: 6, default: 0)
#define KIND2(x, y) KIND((x) + (y))

static double re_f(float complex z) { return crealf(z) + 0.125; }
static double re_d(double complex z) { return creal(z) + 0.25; }
static double re_l(long double complex z) { return (double)creall(z) + 0.5; }
static double re_r(double x) { return x; }
/* a type-generic "real part" in the manner of <tgmath.h> */
#define RE(z) _Generic((z), float complex: re_f, double complex: re_d, long double complex: re_l, default: re_r)(z)

int main(void) {
    float f = 1;
    double d = 2;
    long double l = 3;
    float complex fc = 4;
    double complex dc = 5;
    long double complex lc = 6;
    const double complex cdc = 7;
    volatile float complex vfc = 8;
    print_i64(KIND(f) * 100000 + KIND(d) * 10000 + KIND(l) * 1000 + KIND(fc) * 100 + KIND(dc) * 10 + KIND(lc));
    print_i64(KIND(cdc) * 10 + KIND(vfc));
    print_i64(KIND(I) * 10 + KIND(CMPLX(1, 2)));
    print_i64(KIND(CMPLXF(1, 2)) * 10 + KIND(CMPLXL(1, 2)));
    print_i64(KIND2(fc, 1) * 1000 + KIND2(fc, f) * 100 + KIND2(fc, d) * 10 + KIND2(fc, l));
    print_i64(KIND2(dc, fc) * 1000 + KIND2(lc, fc) * 100 + KIND2(d, fc) * 10 + KIND2(1LL, dc));
    print_i64(KIND2(f, d) * 10 + KIND2(1, f));
    print_i64(KIND(-fc) * 100 + KIND(fc * fc) * 10 + KIND(fc ? fc : 1));
    print_i64(KIND(fc == fc));
    print_f64(RE(fc));
    print_f64(RE(dc));
    print_f64(RE(lc));
    print_f64(RE(d));
    print_f64(RE(fc * 2.0));
    return 0;
}

/* expect: 123456 */
/* expect: 54 */
/* expect: 45 */
/* expect: 46 */
/* expect: 4456 */
/* expect: 5655 */
/* expect: 21 */
/* expect: 444 */
/* expect: 0 */
/* expect: 4.125 */
/* expect: 5.25 */
/* expect: 6.5 */
/* expect: 2 */
/* expect: 8.25 */
