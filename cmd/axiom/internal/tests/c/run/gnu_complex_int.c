/* flags: --c-std=gnu17 */
/* In a GNU dialect _Complex of an integer type (char, short, int, long,
   long long, signed or unsigned; not _Bool) is a complex type whose parts
   are of that type, and an integer constant with the suffix i or j is the
   imaginary value of the complex type of its type (2i: _Complex int 0 +
   2i). The usual arithmetic conversions take the complex type of the
   common real type: _Complex char + _Complex char is _Complex int. + - *
   act on the parts in their integer arithmetic, / divides by the squared
   magnitude (integer quotients), == and != compare both parts, ~ is the
   conjugate, a value is true when either part is nonzero; conversions to
   and from the real and floating complex types convert the parts.
   Constants fold, in static initializers too. */
#include "host.h"
typedef struct { _Complex signed char a; _Complex short b; } pair;
static pair s = { 1 + 2i, 3 - 4i };
static _Complex unsigned short us = 100 + 200i;
static _Complex long long big = 5000000000LL * 3i + 7;
static const int folded = __imag__ ((2 + 3i) * (4 - 1i));
static _Complex int id(_Complex int z) { return z; }
static _Complex long twice(_Complex long z) { return z * 2; }
#define SHOW(z) (print_i64(__real__ (z)), print_i64(__imag__ (z)))
int main(void) {
    _Complex int a = 1 + 2i;
    _Complex int b = 3 - 1i;
    SHOW(a * b);
    SHOW(a - b);
    SHOW((10 + 20i) / b);
    SHOW((7 + 9i) / 2);
    SHOW(~a);
    SHOW(-a);
    print_bool(a == 1 + 2i);
    print_bool(a != a);
    print_bool(!(0 * 1i));
    print_i64(sizeof(_Complex short));
    print_i64(_Alignof(_Complex long long));
    print_i64(sizeof(s.a + s.a));
    SHOW(s.a);
    SHOW(s.b);
    print_u64(__real__ us + __imag__ us);
    SHOW(big);
    print_i64(folded);
    _Complex unsigned u = 7u + 9ui;
    u /= 2u;
    SHOW(u);
    double _Complex d = a;
    print_f64(__real__ d + __imag__ d);
    _Complex int t = 2.75 - 1.5i;
    SHOW(t);
    int r = b;
    print_i64(r);
    _Complex signed char c = 5 + 6i;
    c *= 2i;
    SHOW(c);
    __real__ c = 9;
    __imag__ c += 1;
    SHOW(c);
    SHOW(id(a) ?: b);
    SHOW(0 ? a : 4);
    SHOW(twice(3 + 4l * 1i));
    return 0;
}
/* expect: 5 */
/* expect: 5 */
/* expect: -2 */
/* expect: 3 */
/* expect: 1 */
/* expect: 7 */
/* expect: 3 */
/* expect: 4 */
/* expect: 1 */
/* expect: -2 */
/* expect: -1 */
/* expect: -2 */
/* expect: true */
/* expect: false */
/* expect: true */
/* expect: 4 */
/* expect: 8 */
/* expect: 8 */
/* expect: 1 */
/* expect: 2 */
/* expect: 3 */
/* expect: -4 */
/* expect: 300 */
/* expect: 7 */
/* expect: 15000000000 */
/* expect: 10 */
/* expect: 3 */
/* expect: 4 */
/* expect: 3 */
/* expect: 2 */
/* expect: -1 */
/* expect: 3 */
/* expect: -12 */
/* expect: 10 */
/* expect: 9 */
/* expect: 11 */
/* expect: 1 */
/* expect: 2 */
/* expect: 4 */
/* expect: 0 */
/* expect: 6 */
/* expect: 8 */
