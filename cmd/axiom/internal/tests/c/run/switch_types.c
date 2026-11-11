/* switch on each kind of integer: long long values beyond 32 bits,
   unsigned values, negative cases, enums, and case values converted to the
   promoted type of the controlling expression. */
#include "host.h"

enum e { A = -2, B = 0, C = 7 };

static int big(long long v) {
    switch (v) {
    case 0x100000000LL: return 1;
    case -0x100000000LL: return 2;
    case 1: return 3;
    default: return 0;
    }
}

static int uns(unsigned u) {
    switch (u) {
    case 0xFFFFFFFFu: return 1;
    case 0x80000000u: return 2;
    case -2: return 3;
    }
    return 0;
}

static int en(enum e x) {
    switch (x) {
    case A: return 10;
    case B: return 20;
    case C: return 30;
    }
    return 0;
}

static int small(unsigned char c) {
    switch (c) {
    case 255: return 1;
    case 256 + 1: return 2;
    }
    return 0;
}

int main(void) {
    print_i64(big(0x100000000LL) + big(-0x100000000LL) * 10 + big(1) * 100 + big(2) * 1000);
    print_i64(uns(-1) + uns(0x80000000u) * 10 + uns(-2) * 100 + uns(5) * 1000);
    print_i64(en(A) + en(B) + en(C) + en((enum e)3));
    print_i64(small(255) + small(1) * 10);
    return 0;
}

/* expect: 321 */
/* expect: 321 */
/* expect: 60 */
/* expect: 1 */
