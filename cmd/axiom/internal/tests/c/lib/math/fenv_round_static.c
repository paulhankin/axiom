/* #pragma STDC FENV_ROUND's static direction at translation time (C23
   7.6.2, F.8.2): floating constants and the constant expressions of static
   initializers are rounded in it, as the same operations are at run time in
   that direction (checked here against the run time's, computed with
   fesetround under FENV_ACCESS ON), for float, double, long double,
   _Float128 and _Float16, complex operations, conversions from integers and
   from decimal values; outside the pragma's scope, to nearest. */
/* flags: --c-std=gnu23 */
/* native-only */
#include "print.h"
#include <fenv.h>
#include <string.h>

#pragma STDC FENV_ROUND FE_UPWARD
static double u_c = 0.1, u_q = 1.0 / 3.0, u_ov = 1e300 * 1e300, u_i = 9007199254740993LL;
static float u_f = 0.1f, u_fq = 1.0f / 3.0f, u_fd = 0.1;
static long double u_l = 0.1L, u_lq = 1.0L / 3.0L;
static _Float128 u_x = 0.1f128, u_xq = 1.0f128 / 3;
static _Float16 u_h = 0.1f16, u_hq = (_Float16)1 / (_Float16)3;
static double _Complex u_z = (1.0 + 2.0i) / (3.0 + 1.0i);
static double u_dd = 0.1dd;
#pragma STDC FENV_ROUND FE_DOWNWARD
static double d_c = 0.1, d_q = 1.0 / 3.0, d_ov = 1e300 * 1e300, d_i = 9007199254740993LL, d_neg = -1.0 / 3.0;
static float d_f = 0.1f, d_fq = 1.0f / 3.0f, d_fd = 0.1;
static long double d_l = 0.1L, d_lq = 1.0L / 3.0L;
static _Float128 d_x = 0.1f128, d_xq = 1.0f128 / 3;
static _Float16 d_h = 0.1f16, d_hq = (_Float16)1 / (_Float16)3;
static double _Complex d_z = (1.0 + 2.0i) / (3.0 + 1.0i);
static double d_dd = 0.1dd;
static double d_sub = 0x1p-1074 / 3, d_zero = 1.0 - 1.0;
#pragma STDC FENV_ROUND FE_TOWARDZERO
static double z_q = -1.0 / 3.0, z_ov = -1e300 * 1e300;
#pragma STDC FENV_ROUND FE_DYNAMIC
static double n_q = 1.0 / 3.0;

static int same(const void *a, const void *b, size_t n)
{
    return memcmp(a, b, n) == 0;
}

#pragma STDC FENV_ACCESS ON

/* the run time's values in the dynamic mode m */
static void check(int m, double c, double q, double ov, double i, float f, float fq, float fd, long double l,
                  long double lq, _Float128 x, _Float128 xq, _Float16 h, _Float16 hq, double _Complex z, double dd)
{
    volatile double one = 1.0, three = 3.0, big = 1e300, tenth = 0.1;
    volatile long long n = 9007199254740993LL;
    volatile float fone = 1.0f, fthree = 3.0f;
    volatile long double lone = 1.0L, lthree = 3.0L;
    volatile _Float128 xone = 1, xthree = 3;
    volatile _Float16 hone = 1, hthree = 3;
    volatile double _Complex za = 1.0 + 2.0i, zb = 3.0 + 1.0i;
    volatile _Decimal64 dtenth = 0.1dd;
    fesetround(m);
    double rc = tenth, rq = one / three, rov = big * big, ri = n;
    float rf = (float)tenth, rfq = fone / fthree, rfd = (float)tenth;
    long double rl = lone / (lthree + lthree + lthree + lone), rlq = lone / lthree;
    _Float128 rx = xone / (xthree + xthree + xthree + xone), rxq = xone / xthree;
    _Float16 rh = hone / (hthree + hthree + hthree + hone), rhq = hone / hthree;
    double _Complex rz = za / zb;
    double rdd = dtenth;
    fesetround(FE_TONEAREST);
    (void)rc;
    print_bool(q == rq && ov == rov && i == ri);
    print_bool(same(&f, &rf, 4) && same(&fq, &rfq, 4) && same(&fd, &rfd, 4));
    print_bool(same(&l, &rl, __AXIOM_LDBL_WIDTH__ == 80 ? 10 : sizeof l) && same(&lq, &rlq, __AXIOM_LDBL_WIDTH__ == 80 ? 10 : sizeof l));
    print_bool(same(&x, &rx, 16) && same(&xq, &rxq, 16));
    print_bool(same(&h, &rh, 2) && same(&hq, &rhq, 2));
    print_bool(same(&z, &rz, sizeof z));
    print_bool(dd == rdd);
    print_bool(c != 0);
}

int main(void)
{
    check(FE_UPWARD, u_c, u_q, u_ov, u_i, u_f, u_fq, u_fd, u_l, u_lq, u_x, u_xq, u_h, u_hq, u_z, u_dd);
    check(FE_DOWNWARD, d_c, d_q, d_ov, d_i, d_f, d_fq, d_fd, d_l, d_lq, d_x, d_xq, d_h, d_hq, d_z, d_dd);
    /* the constant 0.1 itself: rounded up and down, neighbours */
    print_bool(u_c > d_c && u_c - d_c < 2e-17);
    print_bool(d_neg < -n_q && z_q == -d_q && z_ov == -0x1.fffffffffffffp+1023 && d_ov == 0x1.fffffffffffffp+1023);
    print_bool(d_sub == 0 && u_ov > d_ov && d_zero == 0 && 1 / d_zero < 0);
    print_bool(n_q == d_q || n_q == u_q);
    return 0;
}
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
