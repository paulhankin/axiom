/* Integer arithmetic: the operators, signedness, wrap-around of unsigned
   types, division and remainder signs, shifts. */
#include "host.h"

int main(void) {
    int a = 17, b = -5;
    unsigned int u = 7u;
    long l = -100000L;
    print_i64(a + b);
    print_i64(a - b);
    print_i64(a * b);
    print_i64(a / b);
    print_i64(a % b);
    print_i64(-a / 5);
    print_i64(-a % 5);
    print_i64(b / 2);
    print_i64(a << 3);
    print_i64(b >> 1);
    print_i64(a & 12);
    print_i64(a | 12);
    print_i64(a ^ 12);
    print_i64(~a);
    print_i64(-b);
    print_i64(+b);
    print_i64(!a);
    print_i64(!0);
    print_u64(u - 8u);
    print_u64(0u - 1u);
    print_u64(u * 0x80000000u);
    print_u64(0xFFFFFFFFu >> 4);
    print_i64(l * 3);
    print_i64(l / 7);
    print_i64(l % 7);
    {
        unsigned char c = 200;
        signed char s = -100;
        unsigned short us = 65535;
        short ss = -32768;
        print_i64(c + c);
        print_i64(s * 2);
        print_i64(us + 1);
        print_i64(ss - 1);
        c += 100;
        print_i64(c);
        us++;
        print_i64(us);
        s -= 100;
        print_i64(s);
    }
    print_i64(1000000 * 3 / 3);
    print_i64((unsigned)-1 > 0);
    print_i64(-1 < 0u);
    print_i64(5 / 2 * 2 + 5 % 2);
    return 0;
}

/* expect: 12 */
/* expect: 22 */
/* expect: -85 */
/* expect: -3 */
/* expect: 2 */
/* expect: -3 */
/* expect: -2 */
/* expect: -2 */
/* expect: 136 */
/* expect: -3 */
/* expect: 0 */
/* expect: 29 */
/* expect: 29 */
/* expect: -18 */
/* expect: 5 */
/* expect: -5 */
/* expect: 0 */
/* expect: 1 */
/* expect: 4294967295 */
/* expect: 4294967295 */
/* expect: 2147483648 */
/* expect: 268435455 */
/* expect: -300000 */
/* expect: -14285 */
/* expect: -5 */
/* expect: 400 */
/* expect: -200 */
/* expect: 65536 */
/* expect: -32769 */
/* expect: 44 */
/* expect: 0 */
/* expect: 56 */
/* expect: 1000000 */
/* expect: 1 */
/* expect: 0 */
/* expect: 5 */
