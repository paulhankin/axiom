/* flags: --c-std=c23 */
/* Bit-precise integer types (C23 6.2.5p6, 6.3.1.1, 6.4.4.1): storage of the
   smallest standard integer that holds them, no integer promotion, the
   usual arithmetic conversions by rank, wrap-around at N bits (unsigned)
   and sign extension from N bits (signed), shifts, comparisons, wb and uwb
   constants of the least width, switch, arguments and results. */
#include <limits.h>
#include "host.h"

#define T(x) _Generic((x), int: 1, unsigned: 2, long: 3, unsigned long: 4, long long: 5, unsigned long long: 6, \
    _BitInt(2): 10, unsigned _BitInt(1): 11, _BitInt(3): 12, unsigned _BitInt(3): 13, _BitInt(8): 14,         \
    unsigned _BitInt(8): 15, _BitInt(9): 16, _BitInt(17): 17, unsigned _BitInt(17): 18, _BitInt(33): 19,        \
    unsigned _BitInt(33): 20, _BitInt(64): 21, unsigned _BitInt(64): 22, unsigned _BitInt(32): 23,             \
    _BitInt(32): 24, default: 0)

typedef unsigned _BitInt(5) u5;
typedef _BitInt(5) s5;

static u5 add5(u5 a, u5 b) { return a + b; }
static s5 neg5(s5 a) { return -a; }
static _BitInt(33) wide(_BitInt(33) x) { return x * 2; }

static int classify(_BitInt(12) v) {
    switch (v) {
    case -2048: return 1;
    case 2047: return 2;
    case 0: return 3;
    default: return 4;
    }
}

int main(void) {
    print_i64(BITINT_MAXWIDTH);
    print_i64(sizeof(_BitInt(2)) * 1000 + sizeof(_BitInt(9)) * 100 + sizeof(_BitInt(17)) * 10 + sizeof(_BitInt(33)));
    print_i64(_Alignof(unsigned _BitInt(24)));
    /* constants: the least width */
    print_i64(T(0wb));
    print_i64(T(0uwb));
    print_i64(T(1wb));
    print_i64(T(3wb));
    print_i64(T(7uwb));
    print_i64(T(255uwb));
    print_i64(T(0xFFFFFFFFFFFFFFFFuwb));
    print_i64(T(0x7FFFFFFFFFFFFFFFwb));
    print_i64(T(-1wb));
    print_i64(-1wb);
    /* no promotion; conversions by rank */
    unsigned _BitInt(8) u8 = 200;
    _BitInt(8) s8 = -100;
    print_i64(T(u8));
    print_i64(T(+u8));
    print_i64(T(u8 + u8));
    print_i64(u8 + u8);
    print_i64(T(u8 + 1));
    print_i64(u8 + 1);
    print_i64(T(s8 * s8));
    print_i64(s8 * s8);
    print_i64(T((unsigned _BitInt(32))1 + 1));
    print_i64(T((_BitInt(33))1 + 1u));
    print_i64(T((unsigned _BitInt(33))1 + 1LL));
    print_i64(T((_BitInt(64))1 + 1ull));
    print_i64(T(s8 + u8));
    print_i64(s8 + u8);
    print_i64(T((unsigned _BitInt(3))1 + (_BitInt(9))1));
    /* wrap-around and sign extension */
    print_i64(add5(30, 5));
    print_i64(neg5(-16));
    print_i64((u5)-1);
    print_i64((s5)17);
    print_i64((s5)(u5)31);
    u5 w = 31;
    w++;
    print_i64(w);
    s5 v = 15;
    v += 1;
    print_i64(v);
    print_i64(~(u5)0);
    print_i64(~(s5)0);
    print_i64((u5)3 - (u5)4);
    print_i64((u5)1 << 4);
    print_i64((u5)1 << 4 << 1);
    print_i64((s5)-16 >> 2);
    print_i64((u5)16 >> 2);
    print_i64((s5)-7 / 2);
    print_i64((s5)-7 % 2);
    print_i64((s5)-1 < (s5)0);
    print_i64((u5)31 > (u5)30);
    print_i64(wide(0x7FFFFFFFwb + 0x7FFFFFFFwb));
    print_i64(wide(-0x80000000wb));
    print_u64((unsigned _BitInt(64))-1);
    print_i64((_BitInt(64))0x8000000000000000uwb);
    print_i64((_BitInt(10))300.7);
    print_f64((unsigned _BitInt(40))0xFFFFFFFFFFuwb);
    print_i64((bool)(unsigned _BitInt(9))256);
    print_i64((unsigned _BitInt(9))256 != 0);
    print_i64(classify(-2048));
    print_i64(classify(2047));
    print_i64(classify((_BitInt(12))4095));
    print_i64(classify(5));
    _Static_assert(-1wb < 0);
    _Static_assert((u5)33 == 1);
    _Static_assert(sizeof(0x123456789wb) == 8);
    return 0;
}

/* expect: 65535 */
/* expect: 1248 */
/* expect: 4 */
/* expect: 10 */
/* expect: 11 */
/* expect: 10 */
/* expect: 12 */
/* expect: 13 */
/* expect: 15 */
/* expect: 22 */
/* expect: 21 */
/* expect: 10 */
/* expect: -1 */
/* expect: 15 */
/* expect: 15 */
/* expect: 15 */
/* expect: 144 */
/* expect: 1 */
/* expect: 201 */
/* expect: 14 */
/* expect: 16 */
/* expect: 2 */
/* expect: 19 */
/* expect: 5 */
/* expect: 6 */
/* expect: 15 */
/* expect: 100 */
/* expect: 16 */
/* expect: 3 */
/* expect: -16 */
/* expect: 31 */
/* expect: -15 */
/* expect: -1 */
/* expect: 0 */
/* expect: -16 */
/* expect: 31 */
/* expect: -1 */
/* expect: 31 */
/* expect: 16 */
/* expect: 0 */
/* expect: -4 */
/* expect: 4 */
/* expect: -3 */
/* expect: -1 */
/* expect: 1 */
/* expect: 1 */
/* expect: -4 */
/* expect: -4294967296 */
/* expect: 18446744073709551615 */
/* expect: -9223372036854775808 */
/* expect: 300 */
/* expect: 1099511627775 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 2 */
/* expect: 4 */
/* expect: 4 */
