/* flags: --c-int=16 --c-long=32 */
/* A 16-bit int: its limits, the promotions (unsigned short promotes to
   unsigned int), constants' types, and arithmetic that wraps at 16 bits. */
#include "host.h"
#include <limits.h>

static int add(int a, int b) { return a + b; }
static unsigned umul(unsigned a, unsigned b) { return a * b; }

int main(void) {
    int i = INT_MAX;
    unsigned u = UINT_MAX;
    unsigned short us = 65535;
    short s = -1;
    long l = 70000;
    print_i64(sizeof(int));
    print_i64(sizeof(long));
    print_i64(INT_MIN);
    print_i64(INT_MAX);
    print_u64(UINT_MAX);
    print_i64(i + 1 < 0);
    print_u64(u + 1u);
    print_i64(us + 1);
    print_i64(sizeof(us + 1));
    print_i64(us + 1 > 0);
    print_i64(s < 1u);
    print_i64(sizeof 32767);
    print_i64(sizeof 32768);
    print_i64(sizeof 0x8000);
    print_i64(sizeof 65536);
    print_i64(add(30000, 30000));
    print_u64(umul(300, 300));
    print_i64(l * 2);
    print_i64((int)l);
    print_i64(1 << 15);
    print_i64('a' * 300);
    return 0;
}

/* expect: 2 */
/* expect: 4 */
/* expect: -32768 */
/* expect: 32767 */
/* expect: 65535 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */
/* expect: 2 */
/* expect: 0 */
/* expect: 0 */
/* expect: 2 */
/* expect: 4 */
/* expect: 2 */
/* expect: 4 */
/* expect: -5536 */
/* expect: 24464 */
/* expect: 140000 */
/* expect: 4464 */
/* expect: -32768 */
/* expect: 29100 */
