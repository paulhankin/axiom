/* native-only */
/* linux-only */
/* The calling convention of complex values against the system's C compiler
   (c/host_native.c's h_ functions): float and double _Complex arguments
   and results, in registers and on the stack, a structure with a complex
   member, variadic complex arguments both ways, and a callback. */
#include <complex.h>
#include <stdarg.h>
#include "host.h"

typedef struct { float _Complex z; double d; } h_cs;
float _Complex h_cf(float _Complex a, double _Complex b, int k);
double _Complex h_cd(double x, double _Complex a, float _Complex b, h_cs s, double _Complex c, double _Complex d,
                     double _Complex e, double _Complex f);
h_cs h_mk(float _Complex z, double d);
double h_cva(int n, ...);
double _Complex h_ccall(double _Complex (*f)(double _Complex, float _Complex, ...));

static double _Complex cb(double _Complex a, float _Complex b, ...) {
    va_list ap;
    double _Complex c;
    float _Complex d;
    va_start(ap, b);
    c = va_arg(ap, double _Complex);
    d = va_arg(ap, float _Complex);
    va_end(ap);
    return a * 1000 + b * 100 + c * 10 + d;
}

static void pc(double _Complex z) {
    print_f64(creal(z));
    print_f64(cimag(z));
}

int main(void) {
    float _Complex f = h_cf(CMPLXF(1, -2), CMPLX(0.5, 0.25), 3);
    h_cs s = h_mk(CMPLXF(2, 3), 0.5);
    print_f64(crealf(f));
    print_f64(cimagf(f));
    print_f64(crealf(s.z) + cimagf(s.z) * 10 + s.d * 100);
    pc(h_cd(2, CMPLX(1, 1), CMPLXF(-1, 4), s, CMPLX(10, 20), CMPLX(1, 2), CMPLX(0.5, 0.5), CMPLX(3, 7)));
    print_f64(h_cva(3, CMPLX(1, 0), CMPLXF(0, 0), CMPLX(0, 1), CMPLXF(1, 0), CMPLX(0, 0), CMPLXF(0, 1)));
    pc(h_ccall(cb));
    return 0;
}

/* expect: 3.5 */
/* expect: -5.75 */
/* expect: 82 */
/* expect: 5 */
/* expect: 29.5 */
/* expect: 154 */
/* expect: 1357 */
/* expect: 2468 */
