/* <math.h>'s classification and comparison macros (7.12.3, 7.12.14): the
   compiler's builtins, which evaluate each operand once, on every floating
   type, folded for constants. */
/* flags: --c-std=c23 */
#include "print.h"
#include <float.h>
#include <math.h>

static int n;

static double count(double x)
{
    n++;
    return x;
}

int main(void)
{
    volatile double z = 0.0, inf = INFINITY, nan = NAN, sub = 0x1p-1074, one = 1.0;
    volatile float fsub = 0x1p-140f, fone = 1.0f;
    volatile long double lone = 1.0L;
    union { unsigned long long u; double d; } sn;
    print_i64(fpclassify(nan));
/* expect: 0 */
    print_i64(fpclassify(-inf));
/* expect: 1 */
    print_i64(fpclassify(-z));
/* expect: 2 */
    print_i64(fpclassify(sub));
/* expect: 3 */
    print_i64(fpclassify(one));
/* expect: 4 */
    print_i64(fpclassify(fsub));
/* expect: 3 */
    print_i64(fpclassify((double)fsub));
/* expect: 4 */
    print_i64(fpclassify(lone));
/* expect: 4 */
    print_i64(fpclassify(3));
/* expect: 4 */
    print_bool(isfinite(sub) && !isfinite(inf) && !isfinite(nan));
/* expect: true */
    print_bool(isinf(-inf) && !isinf(DBL_MAX));
/* expect: true */
    print_bool(isnan(nan) && !isnan(inf));
/* expect: true */
    print_bool(isnormal(DBL_MIN) && !isnormal(sub) && !isnormal(z) && isnormal(fone));
/* expect: true */
    print_bool(signbit(-z) && !signbit(z) && signbit(-nan) && signbit(-fone));
/* expect: true */
    print_bool(issubnormal(sub) && iszero(-z) && !iszero(sub));
/* expect: true */
    sn.u = 0x7ff0000000000001ULL;
    print_bool(issignaling(sn.d) && !issignaling(nan) && iscanonical(one));
/* expect: true */
    print_bool(isgreater(2.0, one) && !isgreater(nan, one) && isgreaterequal(one, 1));
/* expect: true */
    print_bool(isless(-inf, z) && islessequal(z, -z) && !isless(one, nan));
/* expect: true */
    print_bool(islessgreater(one, 2.0f) && !islessgreater(z, -z) && !islessgreater(nan, one));
/* expect: true */
    print_bool(isunordered(nan, one) && isunordered(one, nan) && !isunordered(one, inf));
/* expect: true */
    /* each operand once */
    n = 0;
    print_bool(isnan(count(one)) || isinf(count(one)) || isunordered(count(one), count(one)));
/* expect: false */
    print_bool(islessgreater(count(one), count(2.0)));
/* expect: true */
    print_i64(n);
/* expect: 6 */
    /* constants: integer constant expressions */
    {
        static const int k[] = { fpclassify(1e-310), isinf(HUGE_VAL), isnan(NAN), signbit(-0.0), isless(1, 2.0) };
        print_i64(k[0] * 10000 + k[1] * 1000 + k[2] * 100 + k[3] * 10 + k[4]);
/* expect: 31111 */
    }
    return 0;
}
