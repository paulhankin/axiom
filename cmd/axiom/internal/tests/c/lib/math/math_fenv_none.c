/* <fenv.h> on WebAssembly, which has no floating-point environment:
   rounding to nearest only, no flags (docs/c.md). */
/* flags: --c-std=c11 */
/* wasm-only */
#include "print.h"
#include <fenv.h>
#include <math.h>

int main(void)
{
    volatile double z = 0.0, one = 1.0, r;
    fenv_t env;
    fexcept_t f;
    print_bool(math_errhandling & MATH_ERREXCEPT);
/* expect: false */
    print_bool(math_errhandling & MATH_ERRNO);
/* expect: true */
    r = one / z;
    print_i64(fetestexcept(FE_ALL_EXCEPT));
/* expect: 0 */
    print_i64(feclearexcept(FE_ALL_EXCEPT));
/* expect: 0 */
    print_i64(feraiseexcept(FE_INVALID));
/* expect: 1 */
    print_i64(feraiseexcept(0));
/* expect: 0 */
    print_i64(fegetround() == FE_TONEAREST);
/* expect: 1 */
    print_i64(fesetround(FE_TONEAREST));
/* expect: 0 */
    print_i64(fesetround(FE_UPWARD));
/* expect: 1 */
    print_i64(fegetexceptflag(&f, FE_ALL_EXCEPT));
/* expect: 0 */
    print_i64(fesetexceptflag(&f, FE_ALL_EXCEPT));
/* expect: 0 */
    print_i64(fegetenv(&env));
/* expect: 0 */
    print_i64(fesetenv(&env));
/* expect: 0 */
    print_i64(feholdexcept(&env));
/* expect: 0 */
    print_i64(feupdateenv(FE_DFL_ENV));
/* expect: 0 */
    (void)r;
    return 0;
}
