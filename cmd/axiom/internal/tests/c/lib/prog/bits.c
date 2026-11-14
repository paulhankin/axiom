/* flags: --c-std=gnu23 */
/* Bit twiddling: bit-fields, unions for type punning, _BitInt of several
   widths, __int128 and the GNU bit builtins, <stdbit.h>, <stdckdint.h>,
   shifts and rotations at the edges of each type's width, and a popcount,
   CRC, Gray code and bit-matrix transpose checked against slow references.
   The output was compared with the system's compilers (C23 parts with one
   that has _BitInt) at -O0 and -O2; the sizes of bit-field structures,
   which Windows lays out otherwise, are not printed. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbit.h>
#include <stdckdint.h>


struct flags {
    unsigned a : 1;
    unsigned b : 3;
    signed c : 4;
    unsigned : 0;
    unsigned d : 7;
    unsigned long long e : 40;
    _Bool f : 1;
    int g : 5;
};

struct mixed {
    char tag;
    unsigned x : 9;
    unsigned short y : 5;
    unsigned char z : 3;
    unsigned int w : 17;
};


static void bitfields(void) {
    struct flags f = { 0 };
    f.a = 1; f.b = 7; f.c = -8; f.d = 127; f.e = 0xFFFFFFFFFFULL; f.f = 5; f.g = -16;
    printf("flags: a %u b %u c %d d %u e %llx f %d g %d\n", f.a, f.b, f.c, f.d, (unsigned long long)f.e, f.f, f.g);
    f.b++; f.c--; f.e++; f.g += 31;
    printf("wrapped: b %u c %d e %llx g %d\n", f.b, f.c, (unsigned long long)f.e, f.g);
    printf("promotions: %d %d\n", f.a - 2 < 0, (f.e - 1) > 0);
    struct mixed m = { 'q', 300, 31, 7, 100000 };
    printf("mixed: %c %u %u %u %u\n", m.tag, m.x, m.y, m.z, m.w);
    /* bytes of a bit-field struct (little-endian on every target here) */
    struct { unsigned lo : 4, hi : 4, mid : 8; } nib = { 0xA, 0x5, 0xC3 };
    unsigned char bytes[sizeof nib];
    memcpy(bytes, &nib, sizeof nib);
    printf("nibbles bytes %02x %02x\n", bytes[0], bytes[1]);
}

static void punning(void) {
    union { double d; uint64_t u; uint32_t w[2]; unsigned char b[8]; } u = { .d = -1.5 };
    printf("double -1.5: %016" PRIx64 " hi %08" PRIx32 " b7 %02x\n", u.u, u.w[1], u.b[7]);
    u.u = 0x7FF8000000000001ULL;
    printf("nan bits round trip %d\n", u.d != u.d);
    union { float f; int32_t i; } fi = { .f = 1.0f };
    /* fast inverse square root's constant */
    fi.i = 0x5f3759df - (fi.i >> 1);
    printf("rsqrt guess %.6f\n", fi.f);
    union { uint16_t s; uint8_t b[2]; } e = { 0x0102 };
    printf("little-endian %d\n", e.b[0] == 2);
}

static unsigned popcount_slow(uint64_t x) { unsigned n = 0; while (x) { n += x & 1; x >>= 1; } return n; }
static unsigned ctz_slow(unsigned x) { unsigned n = 0; while (!(x & 1)) { x >>= 1; n++; } return n; }
static uint64_t rotl64(uint64_t x, unsigned r) { r &= 63; return r ? (x << r) | (x >> (64 - r)) : x; }
static uint32_t rotr32(uint32_t x, unsigned r) { r &= 31; return (x >> r) | (x << ((32 - r) & 31)); }
static uint8_t rotl8(uint8_t x, unsigned r) { r &= 7; return (uint8_t)((x << r) | (x >> ((8 - r) & 7))); }

static void shifts(void) {
    uint64_t one = 1;
    for (unsigned s = 0; s < 64; s += 9) printf("%s%u:%016" PRIx64, s ? " " : "", s, one << s | (UINT64_MAX >> (63 - s)));
    putchar('\n');
    int64_t neg = INT64_MIN;
    printf("arith shift %" PRId64 " %" PRId64 " %" PRId64 "\n", neg >> 1, neg >> 63, (int64_t)-1 >> 40);
    int32_t n32 = -12345;
    printf("int32 >> %d %d %d, << %d\n", n32 >> 1, n32 >> 31, n32 >> 4, (int32_t)((uint32_t)n32 << 7));
    uint8_t b = 0x81;
    uint16_t h = 0x8001;
    printf("promoted: %d %d %u %d\n", b << 1, h << 16 >> 16, (unsigned)(uint8_t)(b << 1), ~b);
    for (unsigned r = 0; r <= 64; r += 13) printf("%srotl64(%u)=%016" PRIx64, r ? " " : "", r, rotl64(0x0123456789ABCDEFULL, r));
    putchar('\n');
    for (unsigned r = 0; r < 33; r += 11) printf("%srotr32(%u)=%08" PRIx32, r ? " " : "", r, rotr32(0xDEADBEEFu, r));
    for (unsigned r = 0; r < 9; r += 3) printf(" rotl8(%u)=%02x", r, rotl8(0x96, r));
    putchar('\n');
    volatile unsigned vs = 31;
    printf("1u << 31 = %u, -1 >> 31 = %d, 0xffffffff >> vs = %u\n", 1u << vs, -1 >> vs, 0xffffffffu >> vs);
    unsigned long long x = 0xF0F0F0F0F0F0F0F0ULL;
    printf("clz-ish:");
    for (int i = 0; i < 4; i++) { int z = 0; for (uint64_t t = x; !(t & (1ULL << 63)); t <<= 1) z++; printf(" %d", z); x >>= 7; }
    putchar('\n');
}

static void bitint(void) {
    _BitInt(7) s7 = -64;
    unsigned _BitInt(7) u7 = 127;
    u7 += 1;
    s7 = (_BitInt(7))(s7 + 0);
    printf("BitInt(7): %d %u width %d\n", (int)s7, (unsigned)u7, BITINT_MAXWIDTH >= 128);
    unsigned _BitInt(65) big = (unsigned _BitInt(65))1 << 64;
    big += big - 1;
    printf("u65: hi %d lo %016llx\n", (int)(big >> 64), (unsigned long long)big);
    unsigned _BitInt(128) w = 0;
    w = ~w;
    unsigned _BitInt(128) q = w / 1000000007u, r = w % 1000000007u;
    printf("u128 max / 1e9+7: %016llx%016llx rem %llu\n", (unsigned long long)(q >> 64), (unsigned long long)q, (unsigned long long)r);
    _BitInt(200) f = 1;
    for (int i = 2; i <= 50; i++) f *= i;
    /* print 50! in decimal */
    char digits[80]; int nd = 0;
    _BitInt(200) t = f;
    while (t) { digits[nd++] = (char)('0' + (int)(t % 10)); t /= 10; }
    printf("50! = ");
    while (nd) putchar(digits[--nd]);
    putchar('\n');
    _BitInt(13) m = -4096;
    printf("BitInt(13) %d %d %d\n", (int)(m / 3), (int)(m % 7), (int)(m >> 3));
    unsigned _BitInt(3) tiny = 5;
    tiny = (unsigned _BitInt(3))(tiny * 3);
    printf("u3 5*3 = %u, sizeof u3 %zu, u65 %zu, s200 %zu\n", (unsigned)tiny, sizeof tiny, sizeof big, sizeof f);
    printf("wb constant %d %d\n", (int)(3wb + 4wb), (int)sizeof(0x7fffffffffffffffffwb));
}

static void stdbit(void) {
    unsigned v[] = { 0, 1, 2, 3, 0x80000000u, 0xFFFFFFFFu, 0x00F0F000u };
    for (size_t i = 0; i < sizeof v / sizeof v[0]; i++)
        printf("%08x: lz %u tz %u lo %u to %u ones %u zeros %u single %d width %u floor %x ceil %x\n", v[i],
               stdc_leading_zeros(v[i]), stdc_trailing_zeros(v[i]), stdc_leading_ones(v[i]), stdc_trailing_ones(v[i]),
               stdc_count_ones(v[i]), stdc_count_zeros(v[i]), (int)stdc_has_single_bit(v[i]), stdc_bit_width(v[i]),
               stdc_bit_floor(v[i]), stdc_bit_ceil(v[i]));
    unsigned char c = 0x10; unsigned long long ll = 1ULL << 40; unsigned short sh = 0;
    printf("generic: %u %u %u %u %u\n", stdc_leading_zeros(c), stdc_trailing_zeros(ll), stdc_count_ones(sh), stdc_first_leading_one(ll), stdc_first_trailing_zero(c));
    printf("typed: %u %u %u\n", stdc_leading_zeros_uc(1), stdc_bit_width_ull(UINT64_MAX), stdc_first_leading_zero_us(0xFFFF));
    printf("endian little %d\n", __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_LITTLE__);
}

static void ckd(void) {
    int r;
    bool o1 = ckd_add(&r, INT_MAX, 1);
    printf("ckd_add INT_MAX+1: %d %d\n", o1, r);
    unsigned char uc;
    bool o2 = ckd_mul(&uc, 16, 17);
    printf("ckd_mul uchar 16*17: %d %u\n", o2, uc);
    long long ll;
    bool o3 = ckd_sub(&ll, LLONG_MIN, 1);
    printf("ckd_sub llong: %d %lld\n", o3, ll);
    unsigned u;
    bool o4 = ckd_sub(&u, 3, 5);
    printf("ckd_sub unsigned 3-5: %d %u\n", o4, u);
    bool o5 = ckd_add(&r, -5, 3u);
    printf("ckd_add mixed signs: %d %d\n", o5, r);
    short s;
    bool o6 = ckd_mul(&s, -256, 128);
    printf("ckd_mul short: %d %d\n", o6, s);
}

static void int128(void) {
    __int128 a = (__int128)INT64_MAX * INT64_MAX;
    unsigned __int128 b = (unsigned __int128)UINT64_MAX * UINT64_MAX;
    printf("int128: %016llx%016llx\n", (unsigned long long)((unsigned __int128)a >> 64), (unsigned long long)a);
    printf("uint128: %016llx%016llx\n", (unsigned long long)(b >> 64), (unsigned long long)b);
    __int128 n = -a / 12345;
    printf("div: %lld rem %lld sign %d\n", (long long)(n >> 64), (long long)(-a % 12345), n < 0);
    unsigned __int128 sh = (unsigned __int128)1 << 127;
    printf("1<<127 >> 100 = %llx, as signed >> 120 = %lld\n", (unsigned long long)(sh >> 100), (long long)((__int128)sh >> 120));
    /* 64x64 -> 128 multiply-high via int128 vs by hand */
    uint64_t x = 0x9E3779B97F4A7C15ULL, y = 0xC2B2AE3D27D4EB4FULL;
    unsigned __int128 p = (unsigned __int128)x * y;
    uint64_t xl = x & 0xFFFFFFFF, xh = x >> 32, yl = y & 0xFFFFFFFF, yh = y >> 32;
    uint64_t ll = xl * yl, lh = xl * yh, hl = xh * yl, hh = xh * yh;
    uint64_t mid = (ll >> 32) + (lh & 0xFFFFFFFF) + (hl & 0xFFFFFFFF);
    uint64_t hi = hh + (lh >> 32) + (hl >> 32) + (mid >> 32);
    printf("mulhi agree %d\n", (uint64_t)(p >> 64) == hi);
    printf("int128 to double %.17g, from double %llx\n", (double)b, (unsigned long long)((unsigned __int128)1e30 >> 64));
    printf("builtins: popcount %d clz %d ctz %d parity %d bswap %08x ffs %d\n", __builtin_popcountll(x), __builtin_clzll(1), __builtin_ctz(0x80), __builtin_parity(7), __builtin_bswap32(0x12345678), __builtin_ffs(0x10));
    long long ovr;
    int o1 = __builtin_add_overflow(LLONG_MAX, 1, &ovr), o2 = __builtin_mul_overflow(1LL << 31, 1LL << 31, &ovr);
    printf("overflow builtins: %d %d\n", o1, o2);
}

static uint32_t crc32c(const unsigned char *p, size_t n) {
    static uint32_t table[256];
    if (!table[1])
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int k = 0; k < 8; k++) c = c & 1 ? (c >> 1) ^ 0x82F63B78u : c >> 1;
            table[i] = c;
        }
    uint32_t c = ~0u;
    while (n--) c = table[(c ^ *p++) & 0xFF] ^ (c >> 8);
    return ~c;
}

static void transpose8(uint8_t m[8]) {
    uint64_t x = 0;
    for (int i = 0; i < 8; i++) x |= (uint64_t)m[i] << (8 * i);
    uint64_t t;
    t = (x ^ (x >> 7)) & 0x00AA00AA00AA00AAULL; x ^= t ^ (t << 7);
    t = (x ^ (x >> 14)) & 0x0000CCCC0000CCCCULL; x ^= t ^ (t << 14);
    t = (x ^ (x >> 28)) & 0x00000000F0F0F0F0ULL; x ^= t ^ (t << 28);
    for (int i = 0; i < 8; i++) m[i] = (uint8_t)(x >> (8 * i));
}

static void algorithms(void) {
    uint64_t s = 0x243F6A8885A308D3ULL, bad = 0, tot = 0;
    for (int i = 0; i < 10000; i++) {
        s ^= s << 13; s ^= s >> 7; s ^= s << 17;
        unsigned fast = 0;
        uint64_t v = s;
        v = v - ((v >> 1) & 0x5555555555555555ULL);
        v = (v & 0x3333333333333333ULL) + ((v >> 2) & 0x3333333333333333ULL);
        v = (v + (v >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
        fast = (unsigned)((v * 0x0101010101010101ULL) >> 56);
        if (fast != popcount_slow(s)) bad++;
        tot += fast;
    }
    printf("popcount: total %" PRIu64 " bad %" PRIu64 "\n", tot, bad);
    printf("crc32c(\"123456789\") = %08x\n", crc32c((const unsigned char *)"123456789", 9));
    uint8_t m[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x81 }, ref[8] = { 0 };
    for (int r = 0; r < 8; r++) for (int c = 0; c < 8; c++) if (m[r] >> c & 1) ref[c] |= (uint8_t)(1u << r);
    transpose8(m);
    printf("transpose ok %d: %02x %02x\n", !memcmp(m, ref, 8), m[0], m[7]);
    /* gray code and its inverse */
    unsigned ok = 1;
    for (unsigned i = 0; i < 4096; i++) {
        unsigned g = i ^ (i >> 1), b = g;
        for (unsigned sh = 1; sh < 32; sh <<= 1) b ^= b >> sh;
        if (b != i) ok = 0;
        if (i && popcount_slow(g ^ ((i - 1) ^ ((i - 1) >> 1))) != 1) ok = 0;
    }
    printf("gray ok %u\n", ok);
    /* next permutation of bits */
    unsigned v = 0x7, cnt = 0;
    while (v < (1u << 10)) { unsigned t = v | (v - 1); v = (t + 1) | (((~t & -~t) - 1) >> (ctz_slow(v) + 1)); cnt++; }
    printf("10 choose 3 = %u\n", cnt);
}

int main(void) {
    bitfields();
    punning();
    shifts();
    bitint();
    stdbit();
    ckd();
    int128();
    algorithms();
    return 0;
}
// expect: flags: a 1 b 7 c -8 d 127 e ffffffffff f 1 g -16
// expect: wrapped: b 0 c 7 e 0 g 15
// expect: promotions: 1 1
// expect: mixed: q 300 31 7 100000
// expect: nibbles bytes 5a c3
// expect: double -1.5: bff8000000000000 hi bff80000 b7 bf
// expect: nan bits round trip 1
// expect: rsqrt guess 0.966215
// expect: little-endian 1
// expect: 0:0000000000000001 9:00000000000003ff 18:000000000007ffff 27:000000000fffffff 36:0000001fffffffff 45:00003fffffffffff 54:007fffffffffffff 63:ffffffffffffffff
// expect: arith shift -4611686018427387904 -1 -1
// expect: int32 >> -6173 -1 -772, << -1580160
// expect: promoted: 258 -32767 2 -130
// expect: rotl64(0)=0123456789abcdef rotl64(13)=68acf13579bde024 rotl64(26)=9e26af37bc048d15 rotl64(39)=d5e6f78091a2b3c4 rotl64(52)=def0123456789abc
// expect: rotr32(0)=deadbeef rotr32(11)=ddfbd5b7 rotr32(22)=b6fbbf7a rotl8(0)=96 rotl8(3)=b4 rotl8(6)=a5
// expect: 1u << 31 = 2147483648, -1 >> 31 = -1, 0xffffffff >> vs = 1
// expect: clz-ish: 0 7 14 21
// expect: BitInt(7): -64 0 width 1
// expect: u65: hi 1 lo ffffffffffffffff
// expect: u128 max / 1e9+7: 000000044b82f98895147f23df9f377d rem 279632276
// expect: 50! = ,.-/*/')*+-0'0/+)0/0(-++*)*,/+)--+*00',,',-**,0)''-),'/,-++.
// expect: BitInt(13) -1365 -1 -512
// expect: u3 5*3 = 7, sizeof u3 1, u65 16, s200 32
// expect: wb constant 7 16
// expect: 00000000: lz 32 tz 32 lo 0 to 0 ones 0 zeros 32 single 0 width 0 floor 0 ceil 1
// expect: 00000001: lz 31 tz 0 lo 0 to 1 ones 1 zeros 31 single 1 width 1 floor 1 ceil 1
// expect: 00000002: lz 30 tz 1 lo 0 to 0 ones 1 zeros 31 single 1 width 2 floor 2 ceil 2
// expect: 00000003: lz 30 tz 0 lo 0 to 2 ones 2 zeros 30 single 0 width 2 floor 2 ceil 4
// expect: 80000000: lz 0 tz 31 lo 1 to 0 ones 1 zeros 31 single 1 width 32 floor 80000000 ceil 80000000
// expect: ffffffff: lz 0 tz 0 lo 32 to 32 ones 32 zeros 0 single 0 width 32 floor 80000000 ceil 0
// expect: 00f0f000: lz 8 tz 12 lo 0 to 0 ones 8 zeros 24 single 0 width 24 floor 800000 ceil 1000000
// expect: generic: 3 40 0 24 1
// expect: typed: 7 64 0
// expect: endian little 1
// expect: ckd_add INT_MAX+1: 1 -2147483648
// expect: ckd_mul uchar 16*17: 1 16
// expect: ckd_sub llong: 1 9223372036854775807
// expect: ckd_sub unsigned 3-5: 1 4294967294
// expect: ckd_add mixed signs: 0 -2
// expect: ckd_mul short: 0 -32768
// expect: int128: 3fffffffffffffff0000000000000001
// expect: uint128: fffffffffffffffe0000000000000001
// expect: div: -373567113683872 rem -859 sign 1
// expect: 1<<127 >> 100 = 8000000, as signed >> 120 = -128
// expect: mulhi agree 1
// expect: int128 to double 3.4028236692093846e+38, from double c9f2c9cd0
// expect: builtins: popcount 38 clz 63 ctz 7 parity 1 bswap 78563412 ffs 5
// expect: overflow builtins: 1 0
// expect: popcount: total 320077 bad 0
// expect: crc32c("123456789") = e3069283
// expect: transpose ok 1: 81 80
// expect: gray ok 1
// expect: 10 choose 3 = 120
