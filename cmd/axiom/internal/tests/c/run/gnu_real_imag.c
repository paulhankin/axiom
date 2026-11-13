/* flags: --c-std=gnu17 */
/* __real__ and __imag__ (a GNU dialect): the parts of a complex value, as
   lvalues when the operand is one (assignment, compound assignment, their
   addresses), of a value that is not (a call's result), of every complex
   type's parts (long double, _Float16), constant in constant expressions;
   of a real operand, the value and 0. */
#include <complex.h>
#include "host.h"

static double _Complex g(void) {
    return 3.0 + 4.0i;
}

static const double c = __real__ (2.0 + 3.0i) + __imag__ (2.0 + 3.0i);

int main(void) {
    double _Complex z = 1.0 + 2.0i;
    __real__ z = 5.0;
    __imag__ z *= 3;
    print_f64(__real__ z);
    print_f64(__imag__ z);
    print_f64(__real__ g());
    print_f64(__imag__ g());
    long double _Complex w = 1.5L - 2.5Li;
    __imag__ w = 7.25L;
    print_f64((double)__real__ w);
    print_f64((double)__imag__ w);
    _Float16 _Complex h = 0.5f16 + 0.25f16 * 1.0if16;
    print_f64((double)__real__ h);
    print_f64((double)__imag__ h);
    double x = 2.5;
    __real__ x = 3.5;
    print_f64(__real__ x);
    print_f64(__imag__ x);
    float _Complex f = 1.0f + 1.0if;
    float *pi = &__imag__ f;
    *pi = 9;
    print_f64(c);
    print_f64(cimagf(f));
    return 0;
}
/* expect: 5 */
/* expect: 6 */
/* expect: 3 */
/* expect: 4 */
/* expect: 1.5 */
/* expect: 7.25 */
/* expect: 0.5 */
/* expect: 0.25 */
/* expect: 3.5 */
/* expect: 0 */
/* expect: 5 */
/* expect: 9 */
