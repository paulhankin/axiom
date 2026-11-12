/* <tgmath.h> (7.25): the function the arguments' types choose, in C99
   (no _Generic) as in later standards. */
/* flags: --c-std=c99 */
#include "print.h"
#include <tgmath.h>

int main(void)
{
    float f = 0.5f;
    double d = 0.5;
    long double l = 0.5L;
    int i = 2;
    double complex z = 1.0 + 1.0 * I;
    float complex zf = 1.0f + 1.0f * I;
    /* the result types: float, double, long double, double for an integer */
    print_i64(sizeof(sin(f)));
/* expect: 4 */
    print_i64(sizeof(sin(d)));
/* expect: 8 */
    print_i64(sizeof(sin(l)) == sizeof(long double));
/* expect: 1 */
    print_i64(sizeof(sin(i)));
/* expect: 8 */
    print_i64(sizeof(pow(f, f)));
/* expect: 4 */
    print_i64(sizeof(pow(f, d)));
/* expect: 8 */
    print_i64(sizeof(pow(f, i)));
/* expect: 8 */
    print_i64(sizeof(fma(f, f, f)));
/* expect: 4 */
    print_i64(sizeof(ldexp(f, 3)));
/* expect: 4 */
    print_i64(sizeof(sqrt(zf)));
/* expect: 8 */
    print_i64(sizeof(sqrt(z)));
/* expect: 16 */
    print_i64(sizeof(fabs(zf)));
/* expect: 4 */
    print_i64(sizeof(carg(f)));
/* expect: 4 */
    print_i64(sizeof(creal(i)));
/* expect: 8 */
    print_i64(sizeof(pow(z, f)));
/* expect: 16 */
    /* the values */
    print_f64(sin(f));
/* expect: 0.4794255495071411 */
    print_f64(sin(d));
/* expect: 0.479425538604203 */
    print_f64(sqrt(i));
/* expect: 1.4142135623730951 */
    print_f64(fabs(z));
/* expect: 1.4142135623730951 */
    print_f64(cimag(sqrt(-4.0 + 0.0 * I)));
/* expect: 2 */
    print_f64(creal(exp(z)));
/* expect: 1.4686939399158851 */
    print_f64(carg(-1.0));
/* expect: 3.141592653589793 */
    print_f64(remainder(7, 2));
/* expect: -1 */
    return 0;
}
