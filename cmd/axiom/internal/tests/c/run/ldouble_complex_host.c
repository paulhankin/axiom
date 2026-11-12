/* native-only */
/* linux-only */
/* long double _Complex across the platform's C calling convention, against
   the system's C compiler (c/host_native.c's h_lc_ functions) in its format:
   on x86-64, COMPLEX_X87 (in memory; the result in st(0) and st(1)); on
   AArch64, a homogeneous aggregate of two binary128 values. */
#include <complex.h>
#include <float.h>
#include <stdarg.h>
#include "host.h"

typedef long double _Complex LC;
typedef struct { LC z; int k; } h_lcs;
LC h_lc_add(LC a, LC b);
LC h_lc_many(int i, LC a, double d, LC b, LC c, LC e, LC f, long double g, LC h);
h_lcs h_lc_s(h_lcs s);
LC h_lc_va(int n, ...);
LC h_lc_call(LC (*f)(LC, long double));

static void pr(LC z) {
    print_f64((double)creall(z));
    print_f64((double)cimagl(z));
}

static LC cb(LC z, long double k) {
    return z * k + CMPLXL(0, 1);
}

int main(void) {
    LC a = CMPLXL(0.75L, -1.5L), b = CMPLXL(2.0L, 0.125L);
    h_lcs s = { a, 4 };
    LC e = CMPLXL(1 + LDBL_EPSILON, -LDBL_EPSILON);
    pr(h_lc_add(a, b));
    pr(h_lc_many(1, a, 0.5, b, a, b, a, 0.25L, b));
    s = h_lc_s(s);
    pr(s.z);
    print_i64(s.k);
    pr(h_lc_va(3, a, 0.5, b, 1.5, a, 2.5));
    pr(h_lc_call(cb));
    /* (the extra precision travels: both parts) */
    e = h_lc_add(e, 0);
    print_i64(creall(e) - 1 == LDBL_EPSILON && cimagl(e) == -LDBL_EPSILON);
    return 0;
}

/* expect: 4.75 */
/* expect: -1.25 */
/* expect: 34.5 */
/* expect: -11.875 */
/* expect: 1.5 */
/* expect: -3 */
/* expect: 5 */
/* expect: 15.25 */
/* expect: -7.25 */
/* expect: 2.25 */
/* expect: -0.375 */
/* expect: 1 */
