/* flags: --c-std=c23 */
/* Complex named constants (C23 constexpr: each part exactly representable,
   6.7.1), typeof of complex expressions, auto, and the header's version
   macro. */
#include <complex.h>
#include "host.h"

constexpr double complex K = CMPLX(1.5, -2.0);
constexpr float complex KF = K;
constexpr double complex K2 = K * K;
static_assert(sizeof(typeof(K + 1.0f)) == sizeof(double complex));

int main(void) {
    auto z = KF * 2;
    typeof(z) w = z + I;
    static double complex arr[] = { K, K2, KF };
    print_i64(__STDC_VERSION_COMPLEX_H__);
    print_f64(creal(K2));
    print_f64(cimag(K2));
    print_f64(crealf(w));
    print_f64(cimagf(w));
    print_i64(_Generic(z, float complex: 1, default: 0));
    print_f64(creal(arr[0]) + creal(arr[1]) + cimag(arr[2]));
    return 0;
}

/* expect: 202311 */
/* expect: -1.75 */
/* expect: -6 */
/* expect: 3 */
/* expect: -3 */
/* expect: 1 */
/* expect: -2.25 */
