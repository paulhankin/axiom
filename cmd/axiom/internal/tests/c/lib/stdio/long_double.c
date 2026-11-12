/* flags: --c-std=c23 */
/* long double in its format, whichever the target has (LDBL_MANT_DIG: 53
   binary64, 64 the x87 80-bit format, 113 binary128): strtold, sscanf's
   %Lf, printf's %La and %Le, and strfroml, bit-exact, against values
   derived exactly (with rational arithmetic) for each format. A value out
   of the format's range is HUGE_VALL, or rounds to zero. */
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static int same(long double a, long double b) {
    return memcmp(&a, &b, LDBL_MANT_DIG == 64 ? 10 : sizeof a) == 0;
}

static int bad;

static void check_scan(const char *s, long double want) {
    long double v = strtold(s, NULL), w = 0;
    sscanf(s, "%Lf", &w);
    if (!same(v, want) || !same(w, want)) {
        printf("strtold %s: %La %La, want %La\n", s, v, w, want);
        bad++;
    }
}

static void check_print(long double x, const char *fmt, const char *want) {
    char buf[200];
    snprintf(buf, sizeof buf, fmt, x);
    if (strcmp(buf, want) != 0) {
        printf("%s: %s, want %s\n", fmt, buf, want);
        bad++;
    }
}

#ifndef HUGE_VALL
#define HUGE_VALL (1.0L / 0.0L)
#endif
#define HUGE HUGE_VALL
int main(void) {
#if LDBL_MANT_DIG == 53
    check_scan("0.1", 0x1999999999999ap-56L);
    check_scan("1e-4950", 0.0L);
    check_scan("3.6451995318824746025e-4951", 0.0L);
    check_scan("1.18973149535723176502e+4932", HUGE);
    check_scan("1e4932", HUGE);
    check_scan("-2.5", -0x14000000000000p-51L);
    check_scan("0x1.fffffffffffffffffffffffffffffp0", 0x10000000000000p-51L);
    check_scan("123456789012345678901234567890", 0x18ee90ff6c373ep+44L);
    check_scan("0.333333333333333333333333333333333333333", 0x15555555555555p-54L);
    check_scan("4.9406564584124654e-324", 0x1p-1074L);
    check_scan("2.2250738585072014e-308", 0x10000000000000p-1074L);
    check_scan("1.000000000000000000000000000000000000000000001", 0x10000000000000p-52L);
    check_scan("0x1p-16494", 0.0L);
    check_scan("0x1p-16446", 0.0L);
    check_scan("6.5e-4966", 0.0L);
    check_print(0x1p+0L, "%.0Le", "1e+00");
    check_print(0x1p+0L, "%.5Le", "1.00000e+00");
    check_print(0x1p+0L, "%.20Le", "1.00000000000000000000e+00");
    check_print(0x1p+0L, "%.36Le", "1.000000000000000000000000000000000000e+00");
    check_print(0x3p-2L, "%.0Le", "8e-01");
    check_print(0x3p-2L, "%.5Le", "7.50000e-01");
    check_print(0x3p-2L, "%.20Le", "7.50000000000000000000e-01");
    check_print(0x3p-2L, "%.36Le", "7.500000000000000000000000000000000000e-01");
    check_print(0x1fffffffffffffp-53L, "%.0Le", "1e+00");
    check_print(0x1fffffffffffffp-53L, "%.5Le", "1.00000e+00");
    check_print(0x1fffffffffffffp-53L, "%.20Le", "9.99999999999999888978e-01");
    check_print(0x1fffffffffffffp-53L, "%.36Le", "9.999999999999998889776975374843459576e-01");
    check_print(0x1p-1074L, "%.0Le", "5e-324");
    check_print(0x1p-1074L, "%.5Le", "4.94066e-324");
    check_print(0x1p-1074L, "%.20Le", "4.94065645841246544177e-324");
    check_print(0x1p-1074L, "%.36Le", "4.940656458412465441765687928682213724e-324");
    check_print(0x1fffffffffffffp+971L, "%.0Le", "2e+308");
    check_print(0x1fffffffffffffp+971L, "%.5Le", "1.79769e+308");
    check_print(0x1fffffffffffffp+971L, "%.20Le", "1.79769313486231570815e+308");
    check_print(0x1fffffffffffffp+971L, "%.36Le", "1.797693134862315708145274237317043568e+308");
    check_print(0x19999999999999p-56L, "%.0Le", "1e-01");
    check_print(0x19999999999999p-56L, "%.5Le", "1.00000e-01");
    check_print(0x19999999999999p-56L, "%.20Le", "9.99999999999999916733e-02");
    check_print(0x19999999999999p-56L, "%.36Le", "9.999999999999999167332731531132594682e-02");
    check_print(1.0L, "%La", "0x1p+0");
    check_print(-0x3p-2L, "%.3La", "-0x1.800p-1");
#elif LDBL_MANT_DIG == 64
    check_scan("0.1", 0xcccccccccccccccdp-67L);
    check_scan("1e-4950", 0x3p-16445L);
    check_scan("3.6451995318824746025e-4951", 0x1p-16445L);
    check_scan("1.18973149535723176502e+4932", 0xffffffffffffffffp+16320L);
    check_scan("1e4932", 0xd72cb2a95c7ef6cdp+16320L);
    check_scan("-2.5", -0xa000000000000000p-62L);
    check_scan("0x1.fffffffffffffffffffffffffffffp0", 0x8000000000000000p-62L);
    check_scan("123456789012345678901234567890", 0xc77487fb61b9f077p+33L);
    check_scan("0.333333333333333333333333333333333333333", 0xaaaaaaaaaaaaaaabp-65L);
    check_scan("4.9406564584124654e-324", 0xffffffffffffff64p-1138L);
    check_scan("2.2250738585072014e-308", 0x8000000000000046p-1085L);
    check_scan("1.000000000000000000000000000000000000000000001", 0x8000000000000000p-63L);
    check_scan("0x1p-16494", 0.0L);
    check_scan("0x1p-16446", 0.0L);
    check_scan("6.5e-4966", 0.0L);
    check_print(0x1p+0L, "%.0Le", "1e+00");
    check_print(0x1p+0L, "%.5Le", "1.00000e+00");
    check_print(0x1p+0L, "%.20Le", "1.00000000000000000000e+00");
    check_print(0x1p+0L, "%.36Le", "1.000000000000000000000000000000000000e+00");
    check_print(0x3p-2L, "%.0Le", "8e-01");
    check_print(0x3p-2L, "%.5Le", "7.50000e-01");
    check_print(0x3p-2L, "%.20Le", "7.50000000000000000000e-01");
    check_print(0x3p-2L, "%.36Le", "7.500000000000000000000000000000000000e-01");
    check_print(0xffffffffffffffffp-64L, "%.0Le", "1e+00");
    check_print(0xffffffffffffffffp-64L, "%.5Le", "1.00000e+00");
    check_print(0xffffffffffffffffp-64L, "%.20Le", "9.99999999999999999946e-01");
    check_print(0xffffffffffffffffp-64L, "%.36Le", "9.999999999999999999457898913757247783e-01");
    check_print(0x1p-16445L, "%.0Le", "4e-4951");
    check_print(0x1p-16445L, "%.5Le", "3.64520e-4951");
    check_print(0x1p-16445L, "%.20Le", "3.64519953188247460253e-4951");
    check_print(0x1p-16445L, "%.36Le", "3.645199531882474602528405933619419816e-4951");
    check_print(0xffffffffffffffffp+16320L, "%.0Le", "1e+4932");
    check_print(0xffffffffffffffffp+16320L, "%.5Le", "1.18973e+4932");
    check_print(0xffffffffffffffffp+16320L, "%.20Le", "1.18973149535723176502e+4932");
    check_print(0xffffffffffffffffp+16320L, "%.36Le", "1.189731495357231765021263853030970205e+4932");
    check_print(0xccccccccccccccccp-67L, "%.0Le", "1e-01");
    check_print(0xccccccccccccccccp-67L, "%.5Le", "1.00000e-01");
    check_print(0xccccccccccccccccp-67L, "%.20Le", "9.99999999999999999946e-02");
    check_print(0xccccccccccccccccp-67L, "%.36Le", "9.999999999999999999457898913757247783e-02");
    check_print(1.0L, "%La", "0x1p+0");
    check_print(-0x3p-2L, "%.3La", "-0x1.800p-1");
#elif LDBL_MANT_DIG == 113
    check_scan("0.1", 0x1999999999999999999999999999ap-116L);
    check_scan("1e-4950", 0x57c9647e1a018p-16494L);
    check_scan("3.6451995318824746025e-4951", 0x2000000000000p-16494L);
    check_scan("1.18973149535723176502e+4932", 0x1fffffffffffffffdf5f7837da5b2p+16271L);
    check_scan("1e4932", 0x1ae596552b8fded99d037e3d04b75p+16271L);
    check_scan("-2.5", -0x14000000000000000000000000000p-111L);
    check_scan("0x1.fffffffffffffffffffffffffffffp0", 0x10000000000000000000000000000p-111L);
    check_scan("123456789012345678901234567890", 0x18ee90ff6c373e0ee4e3f0ad20000p-16L);
    check_scan("0.333333333333333333333333333333333333333", 0x15555555555555555555555555555p-114L);
    check_scan("4.9406564584124654e-324", 0x1fffffffffffffec81f3d47cee6c9p-1187L);
    check_scan("2.2250738585072014e-308", 0x1000000000000008c304ccf867de0p-1134L);
    check_scan("1.000000000000000000000000000000000000000000001", 0x10000000000000000000000000000p-112L);
    check_scan("0x1p-16494", 0x1p-16494L);
    check_scan("0x1p-16446", 0x1000000000000p-16494L);
    check_scan("6.5e-4966", 0x1p-16494L);
    check_print(0x1p+0L, "%.0Le", "1e+00");
    check_print(0x1p+0L, "%.5Le", "1.00000e+00");
    check_print(0x1p+0L, "%.20Le", "1.00000000000000000000e+00");
    check_print(0x1p+0L, "%.36Le", "1.000000000000000000000000000000000000e+00");
    check_print(0x3p-2L, "%.0Le", "8e-01");
    check_print(0x3p-2L, "%.5Le", "7.50000e-01");
    check_print(0x3p-2L, "%.20Le", "7.50000000000000000000e-01");
    check_print(0x3p-2L, "%.36Le", "7.500000000000000000000000000000000000e-01");
    check_print(0x1ffffffffffffffffffffffffffffp-113L, "%.0Le", "1e+00");
    check_print(0x1ffffffffffffffffffffffffffffp-113L, "%.5Le", "1.00000e+00");
    check_print(0x1ffffffffffffffffffffffffffffp-113L, "%.20Le", "1.00000000000000000000e+00");
    check_print(0x1ffffffffffffffffffffffffffffp-113L, "%.36Le", "9.999999999999999999999999999999999037e-01");
    check_print(0x1p-16494L, "%.0Le", "6e-4966");
    check_print(0x1p-16494L, "%.5Le", "6.47518e-4966");
    check_print(0x1p-16494L, "%.20Le", "6.47517511943802511092e-4966");
    check_print(0x1p-16494L, "%.36Le", "6.475175119438025110924438958227646552e-4966");
    check_print(0x1ffffffffffffffffffffffffffffp+16271L, "%.0Le", "1e+4932");
    check_print(0x1ffffffffffffffffffffffffffffp+16271L, "%.5Le", "1.18973e+4932");
    check_print(0x1ffffffffffffffffffffffffffffp+16271L, "%.20Le", "1.18973149535723176509e+4932");
    check_print(0x1ffffffffffffffffffffffffffffp+16271L, "%.36Le", "1.189731495357231765085759326628007016e+4932");
    check_print(0x1999999999999999999999999999ap-116L, "%.0Le", "1e-01");
    check_print(0x1999999999999999999999999999ap-116L, "%.5Le", "1.00000e-01");
    check_print(0x1999999999999999999999999999ap-116L, "%.20Le", "1.00000000000000000000e-01");
    check_print(0x1999999999999999999999999999ap-116L, "%.36Le", "1.000000000000000000000000000000000048e-01");
    check_print(1.0L, "%La", "0x1p+0");
    check_print(-0x3p-2L, "%.3La", "-0x1.800p-1");
#endif
    if (!same(wcstold(L" 0.1x", NULL), strtold("0.1", NULL))) {
        printf("wcstold\n");
        bad++;
    }
    {
        char buf[64];
        strfroml(buf, sizeof buf, "%.3e", 2.5L);
        if (strcmp(buf, "2.500e+00") != 0) {
            printf("strfroml: %s\n", buf);
            bad++;
        }
    }
    printf("%d %d\n", LDBL_MANT_DIG == 53 || LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113, bad);
    return 0;
}
/* expect: 1 0 */
