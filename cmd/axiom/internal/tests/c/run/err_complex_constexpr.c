/* The parts of a complex constexpr object's initializer are exactly representable (C23 6.7.1). */
/* expect-error: C3073 */
/* flags: --c-std=c23 */
#include <complex.h>
constexpr float complex k = CMPLX(0.1, 0);
constexpr double r = CMPLX(1, 2);
