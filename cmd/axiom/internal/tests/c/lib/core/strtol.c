/* strtol, strtoll, strtoul, strtoull (C17 7.22.1.4): the subject sequence
   (white space, a sign, the base's digits and prefixes), the end pointer
   (the subject's end, or the string itself when there is none), and the
   clamped value with ERANGE out of range. */
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "print.h"

static const char *s;
static char *end;

static long long ll(const char *str, int base) {
    s = str;
    errno = 0;
    return strtoll(str, &end, base);
}

static unsigned long long ull(const char *str, int base) {
    s = str;
    errno = 0;
    return strtoull(str, &end, base);
}

/* the value, where the subject ended, errno */
static void show(long long v) {
    print_i64(v);
    print_i64(end - s);
    print_i64(errno);
}

int main(void) {
    show(ll("  \t\n+42abc", 10)); /* expect: 42 */ /* expect: 7 */ /* expect: 0 */
    show(ll("-0x1fG", 0)); /* expect: -31 */ /* expect: 5 */ /* expect: 0 */
    show(ll("0x", 0)); /* expect: 0 */ /* expect: 1 */ /* expect: 0 */
    show(ll("0xg", 16)); /* expect: 0 */ /* expect: 1 */ /* expect: 0 */
    show(ll("0X7f", 16)); /* expect: 127 */ /* expect: 4 */ /* expect: 0 */
    show(ll("0755", 0)); /* expect: 493 */ /* expect: 4 */ /* expect: 0 */
    show(ll("089", 0)); /* expect: 0 */ /* expect: 1 */ /* expect: 0 */
    show(ll("zZ", 36)); /* expect: 1295 */ /* expect: 2 */ /* expect: 0 */
    show(ll("1012", 2)); /* expect: 5 */ /* expect: 3 */ /* expect: 0 */
    show(ll("0b101", 0)); /* expect: 0 */ /* expect: 1 */ /* expect: 0 */
    show(ll("   ", 10)); /* expect: 0 */ /* expect: 0 */ /* expect: 0 */
    show(ll(" - 5", 10)); /* expect: 0 */ /* expect: 0 */ /* expect: 0 */
    show(ll("+", 10)); /* expect: 0 */ /* expect: 0 */ /* expect: 0 */
    show(ll("12", 1)); /* expect: 0 */ /* expect: 0 */ /* expect: 0 */
    show(ll("12", 37)); /* expect: 0 */ /* expect: 0 */ /* expect: 0 */
    show(ll("9223372036854775807", 10)); /* expect: 9223372036854775807 */ /* expect: 19 */ /* expect: 0 */
    show(ll("9223372036854775808", 10)); /* expect: 9223372036854775807 */ /* expect: 19 */ /* expect: 34 */
    show(ll("-9223372036854775808", 10)); /* expect: -9223372036854775808 */ /* expect: 20 */ /* expect: 0 */
    show(ll("-9223372036854775809xyz", 10)); /* expect: -9223372036854775808 */ /* expect: 20 */ /* expect: 34 */
    show(ll("99999999999999999999999999", 0)); /* expect: 9223372036854775807 */ /* expect: 26 */ /* expect: 34 */
    show((long long)ull("18446744073709551615", 10)); /* expect: -1 */ /* expect: 20 */ /* expect: 0 */
    print_u64(ull("18446744073709551616", 10)); /* expect: 18446744073709551615 */
    print_i64(errno); /* expect: 34 */
    print_u64(ull("-1", 10)); /* expect: 18446744073709551615 */
    print_u64(ull("-18446744073709551615", 10)); /* expect: 1 */
    print_u64(ull("0xFFFFFFFFFFFFFFFF", 0)); /* expect: 18446744073709551615 */
    print_u64(ull("1777777777777777777777", 8)); /* expect: 18446744073709551615 */
    /* long and unsigned long: their own ranges (32 or 64 bits by target) */
    errno = 0;
    print_bool(strtol("99999999999999999999", &end, 10) == LONG_MAX && errno == ERANGE); /* expect: true */
    errno = 0;
    print_bool(strtol("-99999999999999999999", &end, 10) == LONG_MIN && errno == ERANGE); /* expect: true */
    errno = 0;
    print_bool(strtoul("99999999999999999999", &end, 10) == ULONG_MAX && errno == ERANGE); /* expect: true */
    errno = 0;
    print_bool(strtoul("-1", &end, 10) == ULONG_MAX && errno == 0); /* expect: true */
    print_bool(strtol("2147483647", 0, 10) == 2147483647L); /* expect: true */
    print_bool(strtol("-2147483648", 0, 10) == -2147483647L - 1); /* expect: true */
    print_i64(atoi("  -123x")); /* expect: -123 */
    print_i64(atol("77")); /* expect: 77 */
    print_i64(atoll("-9000000000")); /* expect: -9000000000 */
    {
        /* a null end pointer is not stored through */
        print_i64(strtol("12", (char **)0, 0)); /* expect: 12 */
    }
    return 0;
}
