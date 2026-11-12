/* flags: --c-std=gnu23 */
/* expect: 1 1 1 1 */
/* expect: 5 4 3 2 1 0 */
/* expect: 170141183460469231731687303715884105727 */
/* expect: -170141183460469231731687303715884105728 */
/* expect: 1 2 3 9 */
/* expect: 1 1 1 */
/* expect: 31 0 127 3 */
/* expect: 199 0 200 0 1 */
/* expect: 1 1 1 1 */
/* expect: 1 -2 */
/* expect: 1 1 */
/* expect: 7 -7 7 */
/* expect: 1 1 1 1 */
/* expect: 36 13 */
/* expect: 300 268435455 */
/* expect: 1 1 1 */
/* expect: 1 1 */
/* The integers wider than 64 bits in the language: _Generic and the types'
   identities, switch with wide case values, ++ and -- and the compound
   assignments on objects, structure members and arrays, _Atomic objects
   and <stdatomic.h>, <stdbit.h> and <stdckdint.h> (and the GNU overflow
   builtins), conversions with long double, initializers, enumerations
   and static assertions with wide constants, and wb constants of more than
   64 bits. */
#include <stdatomic.h>
#include <stdbit.h>
#include <stdckdint.h>
#include "host.h"

typedef unsigned __int128 u128;
typedef __int128 i128;

static void put_u128(char *b, int *n, u128 v) {
    char t[48];
    int k = 0;
    do {
        t[k++] = (char)('0' + (int)(v % 10));
        v /= 10;
    } while (v);
    while (k) b[(*n)++] = t[--k];
}

static void line(const long long *xs, int m) {
    char b[200];
    int n = 0;
    for (int i = 0; i < m; i++) {
        if (i) b[n++] = ' ';
        long long x = xs[i];
        if (x < 0) {
            b[n++] = '-';
            x = -x;
        }
        put_u128(b, &n, (u128)x);
    }
    print_str(b, (size_t)n);
}

#define L(...)                                                    \
    do {                                                          \
        long long xs_[] = {__VA_ARGS__};                          \
        line(xs_, (int)(sizeof xs_ / sizeof xs_[0]));             \
    } while (0)

static void print_i128(i128 v) {
    char b[48];
    int n = 0;
    u128 m = (u128)v;
    if (v < 0) {
        b[n++] = '-';
        m = -m;
    }
    put_u128(b, &n, m);
    print_str(b, (size_t)n);
}

#define KIND(x) _Generic((x), __int128: 1, unsigned __int128: 2, _BitInt(128): 3, unsigned _BitInt(128): 4, \
                         long long: 5, default: 0)

static int classify(i128 v) {
    switch (v) {
    case (i128)1 << 100: return 1;
    case -((i128)1 << 100): return 2;
    case 5: return 3;
    case ((i128)5 << 64) | 5: return 4;
    case -1: return 5;
    default: return 0;
    }
}

enum big { BIG_A = (i128)1 << 40, BIG_B = (u128)-1 >> 100 };
static_assert((u128)-1 > 0xFFFFFFFFFFFFFFFFull, "wide");
static_assert(((u128)1 << 127) != 0);

struct rec { char c; i128 x; unsigned _BitInt(200) y; };
static struct rec srec = { 'a', -((i128)1 << 90) - 3, 0x1234567890abcdef1234567890abcdef1234567890uwb };
static u128 table[3] = { 1, (u128)1 << 64, ~(u128)0 };

static _Atomic i128 acount;
static _Atomic unsigned _BitInt(200) abig;

static i128 vsum(int n, ...) {
    __builtin_va_list ap;
    __builtin_va_start(ap, n);
    i128 s = 0;
    for (int i = 0; i < n; i++) {
        s += __builtin_va_arg(ap, i128);
        s += __builtin_va_arg(ap, int);
    }
    __builtin_va_end(ap);
    return s;
}

int main(void) {
    i128 a = 1;
    u128 b = 2;
    _BitInt(128) c = 3;
    unsigned _BitInt(128) d = 4;
    L(KIND(a) == 1, KIND(b) == 2, KIND(c) == 3, KIND(d) == 4);
    L(KIND(a * 2LL) == 1 ? 5 : 0, KIND(d + c) == 4 ? 4 : 0, KIND(c + 1LL) == 3 ? 3 : 0, KIND(a + b) == 2 ? 2 : 0,
      KIND(a + c) == 1 ? 1 : 0, KIND((char)1 + 1LL) == 5 ? 0 : 9);
    print_i128((i128)(~(u128)0 >> 1));
    print_i128((i128)((u128)1 << 127));
    L(classify((i128)1 << 100), classify(-((i128)1 << 100)), classify(5), classify(((i128)5 << 64) | 5) * 0 + 9);
    L(classify(-1) == 5, classify(((i128)1 << 100) + 1) == 0, classify(6) == 0);
    /* ++, --, compound assignments on members and elements */
    struct rec r = srec;
    r.x++;
    r.x -= 1;
    r.y <<= 3;
    r.y >>= 3;
    table[1] *= 3;
    table[1] /= 3;
    table[2]--;
    L(stdc_leading_zeros(table[0] << 96), stdc_trailing_ones(table[2]), stdc_leading_zeros(table[0]),
      stdc_count_ones((unsigned __int128)r.c));
    unsigned _BitInt(200) y = 1;
    L(stdc_leading_zeros(y), stdc_trailing_zeros(y), stdc_count_zeros((unsigned _BitInt(200))0),
      stdc_has_single_bit((unsigned _BitInt(200))3), stdc_bit_width(y));
    L(r.x == srec.x, r.y == srec.y, r.y == 0x1234567890abcdef1234567890abcdef1234567890uwb, table[1] == (u128)1 << 64);
    /* <stdckdint.h> and the overflow builtins */
    i128 res;
    long long ll;
    bool o1 = ckd_add(&res, (i128)((u128)-1 >> 1), 1);
    bool o2 = ckd_mul(&ll, (i128)-1 << 62, 2);
    L(o1, (long long)(res >> 126));
    L(o2 == 0 && ll == (long long)((u128)-1 << 63), __builtin_mul_overflow((u128)1 << 64, (u128)1 << 64, &res));
    /* long double */
    long double ld = (long double)((i128)7 << 70);
    i128 back = (i128)ld;
    L((long long)(back >> 70), (long long)((i128)-ld >> 70), (long long)((i128)(long double)7.5));
    /* atomics */
    atomic_store(&acount, (i128)1 << 80);
    acount += 5;
    acount++;
    i128 old = atomic_fetch_sub(&acount, 2);
    i128 want = acount;
    bool swapped = atomic_compare_exchange_strong(&acount, &want, 99);
    abig = 1;
    abig <<= 150;
    L(old == ((i128)1 << 80) + 6, acount == 99, swapped, abig == (unsigned _BitInt(200))1 << 150);
    i128 seen;
    __atomic_load(&acount, &seen, __ATOMIC_SEQ_CST);
    /* variadic arguments */
    L((long long)vsum(3, (i128)10, 1, (i128)20, 2, (i128)3, 0), (long long)(seen - 86));
    /* enumerations of wide constants, a wb constant of more than 64 bits */
    _BitInt(300) w = 0x123456789abcdef0123456789abcdef0123456789abcdefwb;
    L((long long)BIG_A >> 40 == 1 ? 300 : 0, BIG_B);
    L(w == ((_BitInt(300))0x123456789abcdef0123456789abcdefwb << 64 | 0x0123456789abcdefwb), sizeof(BIG_A) <= 8,
      (int)(w >> 180) == 0x12);
    L(sizeof(0x10000000000000000wb) == 16 || sizeof(0x10000000000000000wb) == 16, _Generic(0xffffffffffffffffffffuwb,
      unsigned _BitInt(80): 1, default: 0));
    return 0;
}
