/* Conversions between floating and integer types (6.3.1.4, 6.3.1.5) in
   constant expressions (static initializers) and at run time: truncation
   toward zero, the full range of unsigned long long and long long, float
   rounding of wide integers, _Bool from any scalar; values out of range
   are undefined at run time. */
#include "host.h"

static unsigned long long u1 = 1.8e19;
static unsigned long long u2 = 18446744073709549568.0;
static long long s1 = -9.2e18;
static long long s2 = -9223372036854775808.0;
static unsigned u3 = 4294967295.0;
static int i1 = (int)-0.9;
static int i2 = 2.99;
static unsigned char c1 = 255.5;
static _Bool b1 = 0.25;
static _Bool b2 = -0.0;
static float f1 = 16777217;
static double d1 = 18446744073709551615ull;
static float f2 = 0x7FFFFFFFFFFFFFFFll;

volatile double vd = 1.8e19;
volatile double vneg = -2.5;
volatile float vf = 3.75f;

int main(void) {
    print_u64(u1);
    print_u64(u2);
    print_i64(s1);
    print_i64(s2);
    print_u64(u3);
    print_i64(i1);
    print_i64(i2);
    print_i64(c1);
    print_i64(b1);
    print_i64(b2);
    print_f64(f1);
    print_i64(d1 == 18446744073709551616.0);
    print_i64(f2 == 9223372036854775808.0);
    print_u64((unsigned long long)vd);
    print_i64((long long)vneg);
    print_i64((int)vf);
    print_i64((_Bool)vneg);
    print_i64((_Bool)(vneg + 2.5));
    print_f64((float)16777217);
    print_i64((double)(unsigned long long)vd == 1.8e19);
    return 0;
}
/* expect: 18000000000000000000 */
/* expect: 18446744073709549568 */
/* expect: -9200000000000000000 */
/* expect: -9223372036854775808 */
/* expect: 4294967295 */
/* expect: 0 */
/* expect: 2 */
/* expect: 255 */
/* expect: 1 */
/* expect: 0 */
/* expect: 16777216 */
/* expect: 1 */
/* expect: 1 */
/* expect: 18000000000000000000 */
/* expect: -2 */
/* expect: 3 */
/* expect: 1 */
/* expect: 0 */
/* expect: 16777216 */
/* expect: 1 */
