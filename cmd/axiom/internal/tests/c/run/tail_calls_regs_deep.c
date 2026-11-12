/* native-only */
/* linux-only */
/* flags: -O2 */
/* Ten million calls in tail position in a row (cmd/axiom/src/tailrec.ax:
   sibling calls) whose result, a structure of 9 to 16 bytes, comes back in
   registers (System V and AAPCS64; Windows returns these through memory):
   the caller returns the callee's registers as they are, so at -O2 each
   call is a jump and the stack does not grow. An integer and a double,
   three floats, and a double and a float. */
#include "host.h"

#define N 10000000

struct ld { long a; double x; };
static struct ld ld_b(long n, double x);
static struct ld ld_a(long n, double x) {
    if (n == 0) { struct ld r = { 1, x }; return r; }
    return ld_b(n - 1, x + 1);
}
static struct ld ld_b(long n, double x) {
    if (n == 0) { struct ld r = { 2, x }; return r; }
    return ld_a(n - 1, x - 0.5);
}

struct f3 { float x, y, z; };
static struct f3 f3_b(long n, float x, float y);
static struct f3 f3_a(long n, float x, float y) {
    if (n == 0) { struct f3 r = { x, y, x + y }; return r; }
    return f3_b(n - 1, y, x);
}
static struct f3 f3_b(long n, float x, float y) {
    if (n == 0) { struct f3 r = { y, x, 0.5f }; return r; }
    return f3_a(n - 1, x, y);
}

struct df { double d; float f; };
static struct df df_b(long n, double d);
static struct df df_a(long n, double d) {
    if (n == 0) { struct df r = { d, 1.5f }; return r; }
    return df_b(n - 1, d + 2);
}
static struct df df_b(long n, double d) {
    if (n == 0) { struct df r = { d, 2.5f }; return r; }
    return df_a(n - 1, d - 1);
}

int main(void) {
    struct ld r1 = ld_a(N, 0.0);
    print_i64(r1.a);
    print_f64(r1.x);
    struct f3 r3 = f3_a(N + 1, 1.5f, 2.5f);
    print_f64(r3.x * 100 + r3.y * 10 + r3.z);
    struct df r4 = df_a(N, 0.0);
    print_f64(r4.d + r4.f);
    return 0;
}
/* expect: 1 */
/* expect: 2500000 */
/* expect: 175.5 */
/* expect: 5000001.5 */
