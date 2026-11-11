/* flags: --c-std=c23 */
/* expect-warning: C4040 */
/* expect-warning: C4050 */
/* C23 together: constexpr objects (scalars, arrays and structures) used in
   constant expressions, auto type inference, nullptr and nullptr_t,
   typeof and typeof_unqual, the bool keywords, attributes ([[nodiscard]],
   [[maybe_unused]], [[deprecated]], [[fallthrough]], [[noreturn]]),
   _BitInt of several widths with wraparound and widening, binary
   literals and digit separators, <stdbit.h> and <stdckdint.h>, #embed
   of a data file (with limit, prefix, suffix and if_empty), enumerations
   with a fixed underlying type, static_assert without a message, empty
   initializers, labels before declarations and at the end of blocks,
   unnamed parameters in definitions, and single-argument _Static_assert.
   The workload: a CRC-32 and an Adler-32 of the embedded data, a bit
   matrix transpose, and checked arithmetic accumulations. */
#include "host.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbit.h>
#include <stdckdint.h>
#include <limits.h>

constexpr int TABLE_BITS = 8;
constexpr unsigned TABLE_SIZE = 1u << TABLE_BITS;
constexpr uint32_t CRC_POLY = 0xEDB8'8320;
constexpr double SCALE = 0.25;
constexpr struct { int lo, hi; } RANGE = { -3, 0b1'0000 };
constexpr char TAG[] = "c23";
static_assert(TABLE_SIZE == 256);
static_assert(RANGE.hi - RANGE.lo == 19, "constexpr members are constants");
static_assert(sizeof TAG == 4);
_Static_assert(sizeof RANGE == 2 * sizeof(int));

static const unsigned char data[] = {
#embed "data/c23_embed.bin"
};
static const unsigned char head[] = {
#embed "data/c23_embed.bin" limit(5) suffix(, 0)
};
static const int wrapped[] = {
#embed "data/c23_embed.bin" limit(3) prefix(-1, ) suffix(, -2)
};
static const int nothing[] = {
#embed "data/c23_embed.bin" limit(0) if_empty(42)
};
static_assert(sizeof head == 6);
static_assert(sizeof wrapped / sizeof wrapped[0] == 5);

enum color : unsigned char { RED, GREEN = 0x7f, BLUE, WHITE = 255 };
enum big : long long { HUGE = 0x1'0000'0000, NEGATIVE = -5 };
enum small : signed char { S_MIN = -128, S_MAX = 127 };
static_assert(sizeof(enum color) == 1 && sizeof(enum big) == 8);

static uint32_t crc_table[TABLE_SIZE];

static void crc_init(void) {
    for (uint32_t i = 0; i < TABLE_SIZE; i++) {
        uint32_t c = i;
        for (int k = 0; k < 8; k++) c = c & 1 ? CRC_POLY ^ (c >> 1) : c >> 1;
        crc_table[i] = c;
    }
}

[[nodiscard]] static uint32_t crc32(const unsigned char *p, size_t n) {
    uint32_t c = 0xFFFF'FFFF;
    for (size_t i = 0; i < n; i++) c = crc_table[(c ^ p[i]) & 0xff] ^ (c >> 8);
    return c ^ 0xFFFF'FFFF;
}

static uint32_t adler32(const unsigned char *p, size_t n) {
    uint32_t a = 1, b = 0;
    for (size_t i = 0; i < n; i++) {
        a = (a + p[i]) % 65'521;
        b = (b + a) % 65'521;
    }
    return b << 16 | a;
}

/* transpose an 8x8 bit matrix held in a uint64_t, by block swaps */
static uint64_t transpose8(uint64_t x) {
    uint64_t t;
    t = (x ^ (x >> 7)) & 0x00AA'00AA'00AA'00AAull;
    x = x ^ t ^ (t << 7);
    t = (x ^ (x >> 14)) & 0x0000'CCCC'0000'CCCCull;
    x = x ^ t ^ (t << 14);
    t = (x ^ (x >> 28)) & 0x0000'0000'F0F0'F0F0ull;
    x = x ^ t ^ (t << 28);
    return x;
}

static int classify(int x) {
    int r = 0;
    switch (x % 4) {
    case 0:
        r += 1;
        [[fallthrough]];
    case 1:
        r += 10;
        break;
    case 2:
        r += 100;
        [[fallthrough]];
    default:
        r += 1000;
    }
    return r;
}

[[deprecated("use twice")]] static int old_twice(int x) { return x + x; }
static int twice(int x, int) { return x * 2; }   /* an unnamed parameter */

[[noreturn]] static void spin(volatile int *p) {
    for (;;) (*p)++;
}

typedef _BitInt(7) i7;
typedef unsigned _BitInt(12) u12;
typedef unsigned _BitInt(33) u33;
typedef _BitInt(48) i48;

static u12 u12_mul(u12 a, u12 b) { return a * b; }   /* wraps modulo 2^12 */

int main(void) {
    crc_init();
    print_u64(sizeof data);
    print_u64(crc32(data, sizeof data));
    print_u64(adler32(data, sizeof data));
    print_str((const char *)head, 5);
    print_i64(head[5] + wrapped[0] + wrapped[4] + wrapped[1] + nothing[0]);
    print_i64(data[sizeof data - 1] + data[sizeof data - 11] * 1000);

    /* constexpr and auto */
    {
        constexpr int n = TABLE_BITS * 3;
        int arr[n] = {};   /* an empty initializer */
        auto count = sizeof arr / sizeof arr[0];
        auto half = SCALE * count;
        auto flag = count > 10;
        print_i64((long long)count);
        print_f64(half);
        print_i64(flag + arr[n - 1]);
        typeof(half) h2 = half * 2;
        typeof_unqual(const volatile int) plain = 7;
        plain++;
        print_f64(h2 + plain);
        constexpr long long BIG = 9'223'372'036'854'775'807;
        print_i64(BIG);
        print_i64(RANGE.lo * 100 + RANGE.hi);
    }

    /* nullptr */
    {
        int *p = nullptr;
        nullptr_t np = nullptr;
        void *q = np;
        print_bool(p == nullptr && q == nullptr && !np);
        bool b = true;
        print_i64(b + true + false + (bool)2);
    }

    /* bit tricks with binary literals */
    {
        uint64_t m = 0b10000000'01000000'00100000'00010000'00001000'00000100'00000010'00000001;
        print_u64(transpose8(m) == m);
        uint64_t r = 0b11111111;
        print_u64(transpose8(r));
        print_u64(transpose8(transpose8(0x0123'4567'89AB'CDEFull)) == 0x0123'4567'89AB'CDEFull);
    }

    /* stdbit */
    {
        unsigned int v = 0b0001'0110'0000;
        print_i64(stdc_leading_zeros(v));
        print_i64(stdc_trailing_zeros(v));
        print_i64(stdc_count_ones(v));
        print_i64(stdc_count_zeros((unsigned char)v));
        print_i64(stdc_first_leading_one(v));
        print_i64(stdc_first_trailing_one(v));
        print_i64(stdc_bit_width(v));
        print_u64(stdc_bit_floor(v));
        print_u64(stdc_bit_ceil(v));
        print_bool(stdc_has_single_bit(256u) && !stdc_has_single_bit(v));
        print_i64(stdc_leading_ones((unsigned short)0xFF00));
        print_i64(stdc_trailing_ones(0b0111ull));
        print_i64(stdc_leading_zeros_ull(1ull));
    }

    /* stdckdint */
    {
        int r;
        long long acc = 0;
        int overflows = 0;
        for (int i = 0; i < 40; i++) {
            if (ckd_mul(&r, 1 << (i % 31), 3)) overflows++;
            acc += r;
            unsigned u;
            if (ckd_sub(&u, i, 20)) overflows += 10;
            acc += u % 1000;
        }
        print_i64(acc);
        print_i64(overflows);
        unsigned char uc;
        bool o1 = ckd_add(&uc, 200, 100);
        long long ll;
        bool o2 = ckd_mul(&ll, LLONG_MAX, -1);
        bool o3 = ckd_mul(&ll, LLONG_MIN, -1);
        print_i64(uc + o1 * 1000 + o2 * 10000 + o3 * 100000);
    }

    /* _BitInt */
    {
        i7 a = 63;
        a = (i7)(a + 1);   /* 64 does not fit: the conversion wraps */
        print_i64(a);
        u12 b = 4000;
        print_u64(u12_mul(b, 3));
        u33 c = 0x1'FFFF'FFFFuwb;
        c += 2;
        print_u64(c);
        i48 d = -140737488355328wb;
        print_i64(d);
        print_i64(d / 3);
        unsigned _BitInt(1) bit = 1;
        bit++;
        print_u64(bit);
        _BitInt(64) w = 5wb * -7;
        print_i64(w);
        print_i64(sizeof(u12) + sizeof(i48));
    }

    /* enumerations with fixed types */
    {
        enum color c = WHITE;
        c++;   /* wraps in unsigned char */
        print_i64(c);
        print_i64(BLUE + RED);
        print_i64(HUGE + NEGATIVE);
        enum small s = S_MIN;
        print_i64(s + S_MAX);
        print_i64(_Generic(GREEN, enum color: 1, default: 0) * 10 + _Generic((enum big)0, long long: 2, default: 3));
    }

    /* labels before declarations, at the end of a block; attributes */
    {
        int total = 0;
        int i = 0;
    again:
        int sq = i * i;
        total += sq + classify(i);
        if (++i < 9) goto again;
        print_i64(total);
        [[maybe_unused]] int unused = 3;
        if (total < 0) {
        end_label:
        }
        print_i64(twice(21, 0));
        print_i64(old_twice(4));
    }
    volatile int z = 0;
    if (z) spin(&z);
    return 0;
}

/* expect: 62 */
/* expect: 1266172681 */
/* expect: 1031999931 */
/* expect: Axiom */
/* expect: 104 */
/* expect: 127010 */
/* expect: 24 */
/* expect: 6 */
/* expect: 1 */
/* expect: 20 */
/* expect: 9223372036854775807 */
/* expect: -284 */
/* expect: true */
/* expect: 3 */
/* expect: 1 */
/* expect: 72340172838076673 */
/* expect: 1 */
/* expect: 23 */
/* expect: 5 */
/* expect: 3 */
/* expect: 6 */
/* expect: 24 */
/* expect: 6 */
/* expect: 9 */
/* expect: 256 */
/* expect: 512 */
/* expect: true */
/* expect: 8 */
/* expect: 3 */
/* expect: 63 */
/* expect: 2147491078 */
/* expect: 201 */
/* expect: 101044 */
/* expect: -64 */
/* expect: 3808 */
/* expect: 1 */
/* expect: -140737488355328 */
/* expect: -46912496118442 */
/* expect: 0 */
/* expect: -35 */
/* expect: 10 */
/* expect: 0 */
/* expect: 128 */
/* expect: 4294967291 */
/* expect: -1 */
/* expect: 12 */
/* expect: 4457 */
/* expect: 42 */
/* expect: 8 */
