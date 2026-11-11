/* The integer promotions and usual arithmetic conversions in the default
   settings: signed and unsigned of each rank, char, enums, _Bool. */
#include "host.h"
#include <limits.h>

enum color { RED, GREEN = 5, BLUE };
enum neg { MINUS = -1, PLUS = 1 };

int main(void) {
    unsigned char uc = 255;
    signed char sc = -1;
    unsigned short us = 65535;
    short ss = -1;
    unsigned u = 1;
    int i = -1;
    long long ll = -1;
    unsigned long long ull = 1;
    enum color col = BLUE;
    _Bool b = 1;
    print_i64(uc + 1);
    print_i64(sc + 1);
    print_i64(us * us > 0);
    print_i64(ss < 0);
    print_i64(i < u);
    print_i64(i < (int)u);
    print_i64(ll < ull);
    print_i64(ll < (long long)ull);
    print_i64(u + i);
    print_i64(sizeof(uc + uc));
    print_i64(sizeof(b + b));
    print_i64(sizeof(1 ? uc : sc));
    print_i64(sizeof(char) + sizeof(short) * 10 + sizeof(int) * 100 + sizeof(long long) * 1000);
    print_i64(col);
    print_i64(col + 1);
    print_i64(GREEN * BLUE);
    print_i64(MINUS < 0);
    print_i64(sizeof(enum color));
    print_i64(b + b);
    b = 2;
    print_i64(b);
    b = 0.25;
    print_i64(b);
    b++;
    print_i64(b);
    b--;
    print_i64(b);
    b--;
    print_i64(b);
    print_i64(CHAR_BIT);
    print_i64(SCHAR_MIN);
    print_i64(SCHAR_MAX);
    print_i64(UCHAR_MAX);
    print_i64(SHRT_MIN);
    print_i64(USHRT_MAX);
    print_i64(INT_MIN);
    print_i64(INT_MAX);
    print_u64(UINT_MAX);
    print_i64(LLONG_MIN);
    print_u64(ULLONG_MAX);
    print_i64(INT_MAX + 1u > 0);
    print_i64((unsigned char)-1);
    print_i64((signed char)200);
    print_i64((short)40000);
    print_u64((unsigned short)-2);
    print_i64((int)4294967297LL);
    print_u64((unsigned long long)-1);
    print_i64(0x7fffffff + 0);
    print_i64(sizeof 0x7fffffff);
    print_i64(sizeof 0x80000000);
    print_i64(sizeof 2147483648);
    print_i64(sizeof 'a');
    print_i64('\377' < 0 ? -1 : 1);
    print_i64(-INT_MAX - 1 == INT_MIN);
    print_i64(1 << 31 < 0);
    print_i64(-7 >> 1);
    print_u64(0x80000000u >> 31);
    print_i64((uc << 24) < 0);
    return 0;
}

/* expect: 256 */
/* expect: 0 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 4 */
/* expect: 4 */
/* expect: 4 */
/* expect: 8421 */
/* expect: 6 */
/* expect: 7 */
/* expect: 30 */
/* expect: 1 */
/* expect: 4 */
/* expect: 2 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 8 */
/* expect: -128 */
/* expect: 127 */
/* expect: 255 */
/* expect: -32768 */
/* expect: 65535 */
/* expect: -2147483648 */
/* expect: 2147483647 */
/* expect: 4294967295 */
/* expect: -9223372036854775808 */
/* expect: 18446744073709551615 */
/* expect: 1 */
/* expect: 255 */
/* expect: -56 */
/* expect: -25536 */
/* expect: 65534 */
/* expect: 1 */
/* expect: 18446744073709551615 */
/* expect: 2147483647 */
/* expect: 4 */
/* expect: 4 */
/* expect: 8 */
/* expect: 4 */
/* expect: -1 */
/* expect: 1 */
/* expect: 1 */
/* expect: -4 */
/* expect: 1 */
/* expect: 1 */
