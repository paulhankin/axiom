/* Floating types: arithmetic, conversions to and from integers, float
   rounding, comparisons, long double as double. */
#include "host.h"

static float fhalf(float f) { return f / 2; }

int main(void) {
    float f = 1.0f / 3.0f;
    double d = 1.0 / 3.0;
    long double ld = 2.5L;
    int i = 7;
    unsigned u = 4000000000u;
    long long big = -9007199254740993LL;
    print_f64(f);
    print_f64(d);
    print_f64(ld * 2);
    print_f64(f + d);
    print_f64(i / 2);
    print_f64(i / 2.0);
    print_f64((double)u);
    print_f64((float)u);
    print_f64((double)big);
    print_i64((int)3.99);
    print_i64((int)-3.99);
    print_u64((unsigned)3e9);
    print_i64((long long)1e15);
    print_i64((char)65.7);
    print_f64(fhalf(3));
    print_i64(0.1 + 0.2 == 0.3);
    print_i64(0.1f + 0.2f == 0.3f);
    print_i64(1.5 > 1);
    print_i64(-0.0 == 0.0);
    print_f64(-d);
    print_f64(1e300 * 1e10);
    print_f64(-1e300 * 1e10);
    d = 0.0;
    print_f64(d / d != d / d);
    print_i64(!d);
    print_i64(d ? 1 : 2);
    f = 16777217;
    print_f64(f);
    d = 3;
    d += 0.5;
    d *= 2;
    d -= 1;
    d /= 3;
    print_f64(d);
    f++;
    print_f64(f);
    print_i64(sizeof(float) + sizeof(double) * 10);
    {
        float a[3] = { 1.5f, 2.25f, 3 };
        double s = 0;
        int k;
        for (k = 0; k < 3; k++) s += a[k];
        print_f64(s);
    }
    {
        _Bool b1 = 0.5, b2 = 0.0;
        print_i64(b1);
        print_i64(b2);
    }
    return 0;
}

/* expect: 0.3333333432674408 */
/* expect: 0.3333333333333333 */
/* expect: 5 */
/* expect: 0.666666676600774 */
/* expect: 3 */
/* expect: 3.5 */
/* expect: 4000000000 */
/* expect: 4000000000 */
/* expect: -9007199254740992 */
/* expect: 3 */
/* expect: -3 */
/* expect: 3000000000 */
/* expect: 1000000000000000 */
/* expect: 65 */
/* expect: 1.5 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: -0.3333333333333333 */
/* expect: Infinity */
/* expect: -Infinity */
/* expect: 1 */
/* expect: 1 */
/* expect: 2 */
/* expect: 16777216 */
/* expect: 2 */
/* expect: 16777216 */
/* expect: 84 */
/* expect: 6.75 */
/* expect: 1 */
/* expect: 0 */
