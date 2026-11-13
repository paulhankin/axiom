/* <complex.h>'s double functions at arguments where each part needs more
   than double's precision on the way (cancellation, the thresholds of the
   methods, results near overflow and underflow): every part correctly
   rounded here (the expected values from an arbitrary-precision
   reference). */
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
    pc(clog(CMPLX(-1.3290307421236445e-80, 3.376690060421641e+276)));
/* expect: 636.7303816239868 */
/* expect: 1.5707963267948966 */
    pc(clog(CMPLX(0.7071067811865475, 0.7071067811865475)));
/* expect: -8.865115929175828e-17 */
/* expect: 0.7853981633974483 */
    pc(cacos(CMPLX(1.1668483191274013e+73, -0.051566503568654554)));
/* expect: 4.419297926162099e-75 */
/* expect: 168.93616533894144 */
    pc(casinh(CMPLX(-1.3290307421236445e-80, 3.376690060421641e+276)));
/* expect: -637.4235288045468 */
/* expect: 1.5707963267948966 */
    pc(cexp(CMPLX(707.7957269146877, -3.803958363546476)));
/* expect: -1.943578127660355e+307 */
/* expect: 1.5158015602327619e+307 */
    pc(cexp(CMPLX(-740.0, 1.0)));
/* expect: 2.27e-322 */
/* expect: 3.5e-322 */
    pc(csin(CMPLX(-5.924451344356829, -709.9141535537685)));
/* expect: 3.599041284513751e+307 */
/* expect: -9.59851510230319e+307 */
    pc(ccos(CMPLX(-4.903626023354285, 707.9432574923491)));
/* expect: 2.7148252382238857e+306 */
/* expect: -1.4022644626760083e+307 */
    pc(csinh(CMPLX(-708.4499470687044, -2.6314965042909293)));
/* expect: 2.0688856412885676e+307 */
/* expect: -1.157510390196075e+307 */
    pc(ctan(CMPLX(2.84455568566392, -2.8307128078801034)));
/* expect: -0.0038707096182971175 */
/* expect: -0.9942455755006169 */
    pc(ctan(CMPLX(3.8156780231241565e+29, 3.48576223182921e-06)));
/* expect: -2.7826008927862937 */
/* expect: 0.00003047555812874972 */
    pc(ctan(CMPLX(1.5707963267948966, 1e-10)));
/* expect: 6123.233995734469 */
/* expect: 9999999999.99625 */
    pc(ctanh(CMPLX(-0.002855087244176086, -1.5111183850318612)));
/* expect: -0.8007844668877703 */
/* expect: -16.69844749809133 */
    pc(ctanh(CMPLX(30.0, 1e+300)));
/* expect: 1 */
/* expect: 1.648318207568626e-26 */
    pc(casin(CMPLX(-5.419954366159287e-166, 0.33046054781369705)));
/* expect: -5.146238179213934e-166 */
/* expect: 0.3247236145552432 */
    pc(casin(CMPLX(1.0, 1e-300)));
/* expect: 1.5707963267948966 */
/* expect: 1e-150 */
    pc(casinh(CMPLX(-0.17841936495634286, -3.6701300396968807e-06)));
/* expect: -0.17748605589733654 */
/* expect: -0.0000036130722558684875 */
    pc(cacosh(CMPLX(1.0000000000000002, 1e-20)));
/* expect: 2.1073424260789764e-8 */
/* expect: 4.745313280009497e-13 */
    pc(catanh(CMPLX(1.0, 1e-300)));
/* expect: 345.73433753938684 */
/* expect: 0.7853981633974483 */
    pc(catanh(CMPLX(0.9999999999999999, 1e-200)));
/* expect: 18.714973875118524 */
/* expect: 4.503599627370496e-185 */
    pc(catanh(CMPLX(-1.4528790515853155e-139, 1.0766867645456593e-11)));
/* expect: -1.4528790515853155e-139 */
/* expect: 1.0766867645456593e-11 */
    pc(catan(CMPLX(1e-20, 0.9999999999999999)));
/* expect: 0.00004503599615191316 */
/* expect: 18.714973873090283 */
    pc(csqrt(CMPLX(-1.0739665571472054e-256, -5.695688037311239e-308)));
/* expect: 2.7480258875369123e-180 */
/* expect: -1.0363235774347728e-128 */
    pc(csqrt(CMPLX(1e+308, 1e+308)));
/* expect: 1.09868411346781e+154 */
/* expect: 4.5508986056222734e+153 */
    pc(cpow(CMPLX(-2.0, -0.0), CMPLX(1.5, 0.5)));
/* expect: -4.621675737279188 */
/* expect: 12.797095547102828 */
    return 0;
}
