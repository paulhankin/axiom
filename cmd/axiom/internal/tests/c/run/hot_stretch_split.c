/* flags: --c-std=gnu17 -O2 */
/* Loops whose values are held in registers through the loop's hot stretch
   and in memory on its colder path, which calls (the register allocator's
   hot-stretch split; tests/run/hot_stretch_*.ax are the Axiom shape). The
   colder path (taken a quarter of the time by the static estimate, seldom
   when run) first works with more values at once than there are registers,
   so that the values live through the loop lose theirs there and are split
   again between the clusters of their uses. At the time of writing, with
   AXIOM_DEBUG_RA, each of these functions takes the split on x86-64 at -O2,
   and on AArch64 `wide` does (its other functions don't run out of
   registers there). Shapes: invariants and values defined in the stretch
   (one on one side of a branch only) read on the colder path, exits from the
   middle of the stretch (a return and a break), the stretch in an inner loop
   with the outer loop's values live through it, and more values than the
   first three for AArch64's registers. */
#include "host.h"

typedef unsigned long long u64;

static volatile u64 sink;
static u64 tab[64];

__attribute__((noinline)) static u64 grow(u64 cap) {
    sink += cap;
    return (cap * 2) + 1;
}

/* the cold path's work before its call: a chain of values all live at once
   (each is read again in reverse order), more than there are registers */
#define CHURN(r, a, b, c, d, e, f, g, h)                                                              \
    do {                                                                                              \
        u64 t0 = (a) ^ (b), t1 = (t0 * (c)) ^ (d), t2 = (t1 * (e)) ^ (f), t3 = (t2 * (b)) ^ (g);       \
        u64 t4 = (t3 * (c)) ^ (h), t5 = (t4 * (d)) ^ 5, t6 = (t5 * (e)) ^ 6, t7 = (t6 * (f)) ^ 7;      \
        u64 t8 = (t7 * (b)) ^ 8, t9 = (t8 * (c)) ^ 9, t10 = (t9 * (d)) ^ 10, t11 = (t10 * (e)) ^ 11;   \
        r = t11;                                                                                      \
        r = (r * 3) ^ t10;                                                                            \
        r = (r * 3) ^ t9;                                                                             \
        r = (r * 3) ^ t8;                                                                             \
        r = (r * 3) ^ t7;                                                                             \
        r = (r * 3) ^ t6;                                                                             \
        r = (r * 3) ^ t5;                                                                             \
        r = (r * 3) ^ t4;                                                                             \
        r = (r * 3) ^ t3;                                                                             \
        r = (r * 3) ^ t2;                                                                             \
        r = (r * 3) ^ t1;                                                                             \
        r = (r * 3) ^ t0;                                                                             \
    } while (0)

/* invariants, and values defined in the stretch (y on one side of a branch
   only) read on the colder path and after the loop */
__attribute__((noinline)) static u64 stretch(u64 n, const volatile u64 *p) {
    u64 k0 = p[0], k1 = p[1], k2 = p[2], k3 = p[3], k4 = p[4], k5 = p[5];
    u64 cap = 4, len = 0, x = k0, acc = 0, y = 7;
    for (u64 i = 0; i < n; i++) {
        if (len >= cap && (len & 1) == 0) {
            u64 r;
            CHURN(r, x, k1, k2, k3, k4, k5, y, acc);
            tab[r & 63] ^= r;
            cap = grow(cap);
            acc ^= y;
        }
        x = (x * k1) + k2;
        if (x & 4) {
            y = x ^ k3;
        }
        acc += (x ^ k4) + (k5 & x) + (len ^ i);
        tab[len & 63] = acc;
        len++;
    }
    return acc + len + cap + y;
}

/* exits from the middle of the stretch: a return and a break */
__attribute__((noinline)) static u64 exits(u64 n, const volatile u64 *p, u64 stop) {
    u64 k0 = p[0], k1 = p[1], k2 = p[2], k3 = p[3], k4 = p[4], k5 = p[5];
    u64 cap = 2, len = 0, x = k0, acc = 0, last = 1;
    for (u64 i = 0; i < n; i++) {
        if (len >= cap && (len & 1) == 0) {
            u64 r;
            CHURN(r, x, k1, k2, k3, k4, k5, last, acc);
            cap = grow(cap);
            last = r ^ cap;
        }
        x = (x * k1) + k2;
        if ((x >> 56) == stop) {
            return acc + 1000;
        }
        acc += (x >> 7) ^ (last * k3);
        if ((acc & 0x3ff) == stop) {
            break;
        }
        acc += (k4 & x) + (k5 | len);
        len++;
    }
    return acc + len + last;
}

/* the stretch in an inner loop, values of the outer one live through it */
__attribute__((noinline)) static u64 nested(u64 rounds, u64 n, const volatile u64 *p) {
    u64 k0 = p[0], k1 = p[1], k2 = p[2], k3 = p[3], k4 = p[4], k5 = p[5];
    u64 total = 0;
    for (u64 q = 0; q < rounds; q++) {
        u64 cap = 1, len = 0, x = k0 + q, acc = q;
        for (u64 i = 0; i < n; i++) {
            if (len >= cap && (len & 1) == 0) {
                u64 r;
                CHURN(r, x, k1, k2, k3, k4, k5, total, acc);
                cap = grow(cap);
                acc += r;
            }
            x = (x * k1) ^ k2;
            acc += (x & k3) + (k4 ^ i) + (total >> 3) + (k5 * len);
            len++;
        }
        total = (total * 31) + acc + cap;
    }
    return total;
}

/* more values live through the loop, for a target with more registers */
__attribute__((noinline)) static u64 wide(u64 n, const volatile u64 *p) {
    u64 k0 = p[0], k1 = p[1], k2 = p[2], k3 = p[3], k4 = p[4], k5 = p[5], k6 = p[6], k7 = p[7];
    u64 k8 = p[8], k9 = p[9], k10 = p[10], k11 = p[11], k12 = p[12], k13 = p[13];
    u64 cap = 4, len = 0, x = k0, acc = 0, y = 7;
    for (u64 i = 0; i < n; i++) {
        if (len >= cap && (len & 1) == 0) {
            u64 r, s;
            CHURN(r, x, k1, k2, k3, k4, k5, y, acc);
            CHURN(s, r, k6, k7, k8, k9, k10, k11, k12);
            tab[(r ^ s) & 63] ^= r + s;
            cap = grow(cap);
            acc ^= y;
        }
        x = (x * k1) + k2;
        if (x & 4) {
            y = x ^ k3;
        }
        acc += (x ^ k4) + (k5 & x) + (k6 | i) + (k7 * len) + (k8 >> (i & 7)) + (x ^ k9) + (k10 & y);
        acc += (k11 | x) + (k12 ^ y) + (k13 + x);
        tab[len & 63] = acc;
        len++;
    }
    return acc + len + cap + y;
}

static volatile u64 P[16] = {3000,
                             1,
                             6364136223846793005ull,
                             1442695040888963407ull,
                             0x5555,
                             0xff00ff,
                             0x1234,
                             3,
                             0xfffffff,
                             77,
                             2862933555777941757ull,
                             3037000493ull,
                             0xabcdef,
                             0xfff,
                             11,
                             5};

int main(void) {
    print_u64(stretch(P[0], P + 1));
    print_u64(exits(P[0], P + 1, 0x3c));
    print_u64(exits(P[0], P + 1, 999));
    print_u64(nested(5, P[0], P + 1));
    print_u64(wide(P[0], P + 1));
    print_u64(sink);
    return 0;
}
/* expect: 6738917858506884818 */
/* expect: 4347216761146279412 */
/* expect: 9156371388893918717 */
/* expect: 13886769683909319022 */
/* expect: 5163128916026240684 */
/* expect: 33772 */
