/* flags: --c-std=c17 */
/* The imaginary types (C17 Annex G): float, double and long double
   _Imaginary, _Imaginary_I; the result's domain (real, imaginary or
   complex) and value of each operator +, -, *, / of every pair of a real,
   an imaginary and a complex operand (G.5); conversions (G.4), compound
   assignment, comparison and the conditional operator, sizes, constants
   of static storage, and cimag, creal and conj of an imaginary value. */
#include <complex.h>
#include "host.h"

#define KIND(x) _Generic((x), double: 0, double _Imaginary: 1, double _Complex: 2, default: 9)

static void show(int kind, double _Complex v) {
    print_i64(kind);
    print_f64(creal(v));
    print_f64(cimag(v));
}

int main(void) {
    double r = 3.0;
    double _Imaginary i = 2.0 * _Imaginary_I;
    double _Complex z = 1.0 + 4.0 * I;
    show(KIND(r + r), r + r);
    show(KIND(r - r), r - r);
    show(KIND(r * r), r * r);
    show(KIND(r / r), r / r);
    show(KIND(r + i), r + i);
    show(KIND(r - i), r - i);
    show(KIND(r * i), r * i);
    show(KIND(r / i), r / i);
    show(KIND(r + z), r + z);
    show(KIND(r - z), r - z);
    show(KIND(r * z), r * z);
    show(KIND(r / z), r / z);
    show(KIND(i + r), i + r);
    show(KIND(i - r), i - r);
    show(KIND(i * r), i * r);
    show(KIND(i / r), i / r);
    show(KIND(i + i), i + i);
    show(KIND(i - i), i - i);
    show(KIND(i * i), i * i);
    show(KIND(i / i), i / i);
    show(KIND(i + z), i + z);
    show(KIND(i - z), i - z);
    show(KIND(i * z), i * z);
    show(KIND(i / z), i / z);
    show(KIND(z + r), z + r);
    show(KIND(z - r), z - r);
    show(KIND(z * r), z * r);
    show(KIND(z / r), z / r);
    show(KIND(z + i), z + i);
    show(KIND(z - i), z - i);
    show(KIND(z * i), z * i);
    show(KIND(z / i), z / i);
    show(KIND(z + z), z + z);
    show(KIND(z - z), z - z);
    show(KIND(z * z), z * z);
    show(KIND(z / z), z / z);
    double _Imaginary w = i;
    w *= 3.0;
    w += i;
    show(KIND(w), w);
    double toreal = i;
    print_f64(toreal);
    double _Imaginary fromreal = r;
    show(KIND(fromreal), fromreal);
    double _Imaginary fromz = (double _Imaginary)z;
    show(KIND(fromz), fromz);
    double _Complex zi = i;
    show(KIND(zi), zi);
    print_i64((i == 2.0 * _Imaginary_I) + (i != w) * 10 + ((double _Complex)i == 2.0 * I) * 100);
    print_i64(i == 0.0);
    show(KIND(r > 1 ? i : w), r > 1 ? i : w);
    print_i64(sizeof(float _Imaginary) + sizeof(double _Imaginary) * 10 + (sizeof(long double _Imaginary) == sizeof(long double)) * 100);
    static const double _Imaginary s = 3.0 * _Imaginary_I;
    show(KIND(-s), -s);
    long double _Imaginary li = 2.5L * _Imaginary_I;
    print_f64((double)(li * li));
    float _Imaginary fi = 0.5f * _Imaginary_I;
    print_f64((double)cimagf(fi * 4.0f) + (double)crealf(fi));
    show(KIND(conj(i)), conj(i));
    print_f64(cimag(i) + creal(i));
    print_i64(_Generic(i * 1.0f, double _Imaginary: 1, default: 0));
    return 0;
}
/* expect: 0 */
/* expect: 6 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: 9 */
/* expect: 0 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 2 */
/* expect: 3 */
/* expect: 2 */
/* expect: 2 */
/* expect: 3 */
/* expect: -2 */
/* expect: 1 */
/* expect: 0 */
/* expect: 6 */
/* expect: 1 */
/* expect: 0 */
/* expect: -1.5 */
/* expect: 2 */
/* expect: 4 */
/* expect: 4 */
/* expect: 2 */
/* expect: 2 */
/* expect: -4 */
/* expect: 2 */
/* expect: 3 */
/* expect: 12 */
/* expect: 2 */
/* expect: 0.17647058823529413 */
/* expect: -0.7058823529411765 */
/* expect: 2 */
/* expect: 3 */
/* expect: 2 */
/* expect: 2 */
/* expect: -3 */
/* expect: 2 */
/* expect: 1 */
/* expect: 0 */
/* expect: 6 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0.6666666666666666 */
/* expect: 1 */
/* expect: 0 */
/* expect: 4 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: -4 */
/* expect: 0 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 2 */
/* expect: 1 */
/* expect: 6 */
/* expect: 2 */
/* expect: -1 */
/* expect: -2 */
/* expect: 2 */
/* expect: -8 */
/* expect: 2 */
/* expect: 2 */
/* expect: 0.47058823529411764 */
/* expect: 0.11764705882352941 */
/* expect: 2 */
/* expect: 4 */
/* expect: 4 */
/* expect: 2 */
/* expect: -2 */
/* expect: 4 */
/* expect: 2 */
/* expect: 3 */
/* expect: 12 */
/* expect: 2 */
/* expect: 0.3333333333333333 */
/* expect: 1.3333333333333333 */
/* expect: 2 */
/* expect: 1 */
/* expect: 6 */
/* expect: 2 */
/* expect: 1 */
/* expect: 2 */
/* expect: 2 */
/* expect: -8 */
/* expect: 2 */
/* expect: 2 */
/* expect: 2 */
/* expect: -0.5 */
/* expect: 2 */
/* expect: 2 */
/* expect: 8 */
/* expect: 2 */
/* expect: 0 */
/* expect: 0 */
/* expect: 2 */
/* expect: -15 */
/* expect: 8 */
/* expect: 2 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 8 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 4 */
/* expect: 2 */
/* expect: 0 */
/* expect: 2 */
/* expect: 111 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 2 */
/* expect: 184 */
/* expect: 1 */
/* expect: 0 */
/* expect: -3 */
/* expect: -6.25 */
/* expect: 2 */
/* expect: 2 */
/* expect: 0 */
/* expect: -2 */
/* expect: 2 */
/* expect: 1 */
