/* strtod and strtof (C17 7.22.1.3): the subject sequence (decimal,
   hexadecimal, infinities and NaNs), where it ends, the value correctly
   rounded (the bits of the double, then of the float: the expected values
   are the exact values rounded to nearest, ties to even), and errno
   (ERANGE for a result that overflows, or is zero or subnormal from a
   nonzero subject: the library's choice, <stdlib.h>). */
#include <errno.h>
#include <stdlib.h>
#include "print.h"

static void t(const char *s) {
    char *e;
    union {
        double d;
        unsigned long long b;
    } u;
    union {
        float f;
        unsigned int b;
    } v;
    errno = 0;
    u.d = strtod(s, &e);
    print_u64(u.b);
    print_i64(e - s);
    print_i64(errno);
    errno = 0;
    v.f = strtof(s, &e);
    print_u64(v.b);
    print_i64(errno);
}

int main(void) {
    char *e;
    const char *s;
    double d;
    t("1.5e3xyz");
    /* expect: 4654311885213007872 */ /* expect: 5 */ /* expect: 0 */ /* expect: 1153138688 */ /* expect: 0 */
    t("  -0.0");
    /* expect: 9223372036854775808 */ /* expect: 6 */ /* expect: 0 */ /* expect: 2147483648 */ /* expect: 0 */
    t("+.5");
    /* expect: 4602678819172646912 */ /* expect: 3 */ /* expect: 0 */ /* expect: 1056964608 */ /* expect: 0 */
    t("5.");
    /* expect: 4617315517961601024 */ /* expect: 2 */ /* expect: 0 */ /* expect: 1084227584 */ /* expect: 0 */
    t("1e");
    /* expect: 4607182418800017408 */ /* expect: 1 */ /* expect: 0 */ /* expect: 1065353216 */ /* expect: 0 */
    t("1e+");
    /* expect: 4607182418800017408 */ /* expect: 1 */ /* expect: 0 */ /* expect: 1065353216 */ /* expect: 0 */
    t("1e-5");
    /* expect: 4532020583610935537 */ /* expect: 4 */ /* expect: 0 */ /* expect: 925353388 */ /* expect: 0 */
    t("0x1.8p1");
    /* expect: 4613937818241073152 */ /* expect: 7 */ /* expect: 0 */ /* expect: 1077936128 */ /* expect: 0 */
    t("0x");
    /* expect: 0 */ /* expect: 1 */ /* expect: 0 */ /* expect: 0 */ /* expect: 0 */
    t("0x.8");
    /* expect: 4602678819172646912 */ /* expect: 4 */ /* expect: 0 */ /* expect: 1056964608 */ /* expect: 0 */
    t("0X1P-1074");
    /* expect: 1 */ /* expect: 9 */ /* expect: 34 */ /* expect: 0 */ /* expect: 34 */
    t("0x1p-1075");
    /* expect: 0 */ /* expect: 9 */ /* expect: 34 */ /* expect: 0 */ /* expect: 34 */
    t("0x1.8p-1074");
    /* expect: 2 */ /* expect: 11 */ /* expect: 34 */ /* expect: 0 */ /* expect: 34 */
    t("0x1.fffffffffffff8p1023");
    /* expect: 9218868437227405312 */ /* expect: 23 */ /* expect: 34 */ /* expect: 2139095040 */ /* expect: 34 */
    t("0x1.fffffffffffff7ffp1023");
    /* expect: 9218868437227405311 */ /* expect: 25 */ /* expect: 0 */ /* expect: 2139095040 */ /* expect: 34 */
    t("9007199254740993");
    /* expect: 4845873199050653696 */ /* expect: 16 */ /* expect: 0 */ /* expect: 1509949440 */ /* expect: 0 */
    t("9007199254740995");
    /* expect: 4845873199050653698 */ /* expect: 16 */ /* expect: 0 */ /* expect: 1509949440 */ /* expect: 0 */
    t("2.2250738585072011e-308");
    /* expect: 4503599627370495 */ /* expect: 23 */ /* expect: 34 */ /* expect: 0 */ /* expect: 34 */
    t("4.9406564584124654e-324");
    /* expect: 1 */ /* expect: 23 */ /* expect: 34 */ /* expect: 0 */ /* expect: 34 */
    t("2.4703282292062327e-324");
    /* expect: 0 */ /* expect: 23 */ /* expect: 34 */ /* expect: 0 */ /* expect: 34 */
    t("2.4703282292062328e-324");
    /* expect: 1 */ /* expect: 23 */ /* expect: 34 */ /* expect: 0 */ /* expect: 34 */
    t("1.7976931348623158e308");
    /* expect: 9218868437227405311 */ /* expect: 22 */ /* expect: 0 */ /* expect: 2139095040 */ /* expect: 34 */
    t("1.7976931348623159e308");
    /* expect: 9218868437227405312 */ /* expect: 22 */ /* expect: 34 */ /* expect: 2139095040 */ /* expect: 34 */
    t("0.1000000000000000055511151231257827021181583404541015625");
    /* expect: 4591870180066957722 */ /* expect: 57 */ /* expect: 0 */ /* expect: 1036831949 */ /* expect: 0 */
    t("1e400");
    /* expect: 9218868437227405312 */ /* expect: 5 */ /* expect: 34 */ /* expect: 2139095040 */ /* expect: 34 */
    t("-1e-400");
    /* expect: 9223372036854775808 */ /* expect: 7 */ /* expect: 34 */ /* expect: 2147483648 */ /* expect: 34 */
    t("123456789012345678901234567890");
    /* expect: 5042042089369253694 */ /* expect: 30 */ /* expect: 0 */ /* expect: 1875342472 */ /* expect: 0 */
    t("0e999");
    /* expect: 0 */ /* expect: 5 */ /* expect: 0 */ /* expect: 0 */ /* expect: 0 */
    t("3.4028235677973366e38");
    /* expect: 5183643170835005440 */ /* expect: 21 */ /* expect: 0 */ /* expect: 2139095039 */ /* expect: 0 */
    t("1.4e-45");
    /* expect: 3936137728114922858 */ /* expect: 7 */ /* expect: 0 */ /* expect: 1 */ /* expect: 34 */
    t("7e-46");
    /* expect: 3931634128487552362 */ /* expect: 5 */ /* expect: 0 */ /* expect: 0 */ /* expect: 34 */
    t("0x1.000001p0");
    /* expect: 4607182419068452864 */ /* expect: 12 */ /* expect: 0 */ /* expect: 1065353216 */ /* expect: 0 */
    t("0x1.0000018p0");
    /* expect: 4607182419202670592 */ /* expect: 13 */ /* expect: 0 */ /* expect: 1065353217 */ /* expect: 0 */
    t("0x1.0000028p0");
    /* expect: 4607182419471106048 */ /* expect: 13 */ /* expect: 0 */ /* expect: 1065353217 */ /* expect: 0 */
    t("0x123456789abcdef0123p-80");
    /* expect: 4571774117729713375 */ /* expect: 25 */ /* expect: 0 */ /* expect: 999400116 */ /* expect: 0 */
    /* infinities and NaNs, in any case; a NaN's n-char-sequence */
    s = "-INFinity!";
    d = strtod(s, &e);
    print_bool(d < 0 && d * 0 != 0 && e - s == 9); /* expect: true */
    s = "infin";
    d = strtod(s, &e);
    print_bool(d > 0 && e - s == 3); /* expect: true */
    s = "nAn(1_x)z";
    d = strtod(s, &e);
    print_bool(d != d && e - s == 8); /* expect: true */
    s = "nan(1_x";
    d = strtod(s, &e);
    print_bool(d != d && e - s == 3); /* expect: true */
    /* no subject: nothing converted, the end is the string */
    s = " \t+.e5";
    print_bool(strtod(s, &e) == 0 && e == s); /* expect: true */
    s = "-";
    print_bool(strtof(s, &e) == 0 && e == s); /* expect: true */
    print_bool(strtold("2.5", &e) == 2.5L && *e == 0); /* expect: true */
    print_f64(atof("  -12.75e1junk")); /* expect: -127.5 */
    return 0;
}
