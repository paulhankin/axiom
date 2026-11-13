/* <complex.h>'s functions (7.3.5-7.3.9) and Annex G's special values (G.6). */
/* flags: --c-std=c11 */
#include "print.h"
#include <complex.h>
#include <math.h>

static void p(double x)
{
    if (x == 0 && signbit(x))
        print_str("-0", 2);
    else
        print_f64(x);
}

static void pc(double complex z)
{
    p(creal(z));
    p(cimag(z));
}

int main(void)
{
    volatile double inf = INFINITY, nan = NAN, z0 = 0.0;
    pc(cexp(CMPLX(0.0, 3.141592653589793)));
/* expect: -1 */
/* expect: 1.2246467991473532e-16 */
    pc(cexp(CMPLX(-inf, 2.0)));
/* expect: -0 */
/* expect: 0 */
    pc(cexp(CMPLX(1.0, -z0)));
/* expect: 2.718281828459045 */
/* expect: -0 */
    pc(cexp(CMPLX(710.0, 0.5)));
/* expect: inf */
/* expect: 1.0710341440060488e+308 */
    pc(clog(CMPLX(-z0, z0)));
/* expect: -inf */
/* expect: 3.141592653589793 */
    pc(clog(CMPLX(-1.0, -z0)));
/* expect: 0 */
/* expect: -3.141592653589793 */
    pc(clog(CMPLX(1.0, 1e-20)));
/* expect: 5e-41 */
/* expect: 1e-20 */
    pc(csqrt(CMPLX(-4.0, z0)));
/* expect: 0 */
/* expect: 2 */
    pc(csqrt(CMPLX(-4.0, -z0)));
/* expect: 0 */
/* expect: -2 */
    pc(csqrt(CMPLX(nan, inf)));
/* expect: inf */
/* expect: inf */
    pc(csqrt(CMPLX(1e308, 1e308)));
/* expect: 1.09868411346781e+154 */
/* expect: 4.5508986056222734e+153 */
    pc(cpow(CMPLX(2.0, 0.0), CMPLX(10.0, 0.0)));
/* expect: 1024 */
/* expect: 0 */
    print_f64(cabs(CMPLX(3.0, -4.0)));
/* expect: 5 */
    print_f64(carg(CMPLX(-1.0, -z0)));
/* expect: -3.141592653589793 */
    pc(cproj(CMPLX(inf, -1.0)));
/* expect: inf */
/* expect: -0 */
    pc(conj(CMPLX(1.0, z0)));
/* expect: 1 */
/* expect: -0 */
    pc(csin(CMPLX(1.0, 1.0)));
/* expect: 1.2984575814159773 */
/* expect: 0.6349639147847361 */
    pc(ccos(CMPLX(1.0, 1.0)));
/* expect: 0.833730025131149 */
/* expect: -0.9888977057628651 */
    pc(ctan(CMPLX(1.0, 1.0)));
/* expect: 0.27175258531951174 */
/* expect: 1.0839233273386946 */
    pc(ctanh(CMPLX(1000.0, 1.0)));
/* expect: 1 */
/* expect: 0 */
    pc(casin(CMPLX(2.0, z0)));
/* expect: 1.5707963267948966 */
/* expect: 1.3169578969248168 */
    pc(casin(CMPLX(2.0, -z0)));
/* expect: 1.5707963267948966 */
/* expect: -1.3169578969248168 */
    pc(cacos(CMPLX(2.0, z0)));
/* expect: 0 */
/* expect: -1.3169578969248168 */
    pc(catan(CMPLX(z0, 2.0)));
/* expect: 1.5707963267948966 */
/* expect: 0.5493061443340549 */
    pc(casinh(CMPLX(1.0, 1.0)));
/* expect: 1.0612750619050357 */
/* expect: 0.6662394324925153 */
    pc(cacosh(CMPLX(z0, z0)));
/* expect: 0 */
/* expect: 1.5707963267948966 */
    pc(catanh(CMPLX(1.0, z0)));
/* expect: inf */
/* expect: 0 */
    pc(catanh(CMPLX(0.5, 0.5)));
/* expect: 0.40235947810852507 */
/* expect: 0.5535743588970452 */
    pc(csinh(CMPLX(inf, z0)));
/* expect: inf */
/* expect: 0 */
    pc(ccosh(CMPLX(z0, z0)));
/* expect: 1 */
/* expect: 0 */
    print_f64(crealf(csqrtf(CMPLXF(-9.0f, 0.0f))) + cimagf(csqrtf(CMPLXF(-9.0f, 0.0f))));
/* expect: 3 */
    return 0;
}
