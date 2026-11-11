/* flags: --c-std=c11 --c-int=64 --c-long=64 */
/* expect-warning: C4102 */
/* A program written for a 64-bit int: hashing and pseudo-random numbers in
   unsigned int (wrapping at 2^64), modular arithmetic in int with
   products kept below 2^63 (Montgomery-free: Russian-peasant
   multiplication), a Collatz record search, a sieve with a bitmap of
   unsigned words, perfect powers, integer square and cube roots, and a
   Gray-code / bit-reversal table. The first section checks what a 64-bit
   int implies: limits, that unsigned short and the character types
   promote to int without loss, that 32-bit constants are ints, that there
   is no 32-bit standard type, and _Generic on int and long. */
#include "host.h"
#include <limits.h>
#include <stdint.h>
#include <stddef.h>

static unsigned fnv1a(const char *s) {
    unsigned h = 14695981039346656037u;
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 1099511628211u;
    }
    return h;
}

static unsigned xs_state = 88172645463325252u;
static unsigned xorshift(void) {
    xs_state ^= xs_state << 13;
    xs_state ^= xs_state >> 7;
    xs_state ^= xs_state << 17;
    return xs_state;
}

/* a * b mod m for m < 2^62 without overflow: doubling */
static int mulmod(int a, int b, int m) {
    int r = 0;
    a %= m;
    while (b > 0) {
        if (b & 1) {
            r += a;
            if (r >= m) r -= m;
        }
        a += a;
        if (a >= m) a -= m;
        b >>= 1;
    }
    return r;
}

static int powmod(int b, int e, int m) {
    int r = 1 % m;
    b %= m;
    while (e > 0) {
        if (e & 1) r = mulmod(r, b, m);
        b = mulmod(b, b, m);
        e >>= 1;
    }
    return r;
}

static int is_prime_mr(int n) {
    static const int bases[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37 };
    if (n < 2) return 0;
    for (int i = 0; i < 12; i++) {
        if (n == bases[i]) return 1;
        if (n % bases[i] == 0) return 0;
    }
    int d = n - 1, s = 0;
    while ((d & 1) == 0) { d >>= 1; s++; }
    for (int i = 0; i < 12; i++) {
        int x = powmod(bases[i], d, n);
        if (x == 1 || x == n - 1) continue;
        int comp = 1;
        for (int k = 1; k < s; k++) {
            x = mulmod(x, x, n);
            if (x == n - 1) { comp = 0; break; }
        }
        if (comp) return 0;
    }
    return 1;
}

static unsigned isqrt(unsigned x) {
    unsigned r = 0, bit = 1u << 62;
    while (bit > x) bit >>= 2;
    while (bit) {
        if (x >= r + bit) { x -= r + bit; r = (r >> 1) + bit; }
        else r >>= 1;
        bit >>= 2;
    }
    return r;
}

static int icbrt(int x) {
    int lo = 0, hi = 2097152;   /* 2^21: hi^3 = 2^63 overflows, so compare by division */
    while (lo < hi) {
        int mid = lo + (hi - lo + 1) / 2;
        if (mid <= x / mid / mid) lo = mid;
        else hi = mid - 1;
    }
    return lo;
}

static unsigned sieve_bits[1 << 10];   /* 65536 odd numbers */

static int sieve_count(int limit) {
    for (int i = 0; i < (1 << 10); i++) sieve_bits[i] = 0;
    int count = limit >= 2;
    for (int i = 3; i < limit; i += 2) {
        int k = i >> 1;
        if (sieve_bits[k >> 6] >> (k & 63) & 1u) continue;
        count++;
        for (int j = i * i; j < limit; j += 2 * i) {
            int m = j >> 1;
            sieve_bits[m >> 6] |= 1u << (m & 63);
        }
    }
    return count;
}

static int popcount(unsigned x) {
    int n = 0;
    while (x) { x &= x - 1; n++; }
    return n;
}

static unsigned reverse_bits(unsigned x) {
    x = (x >> 1 & 0x5555555555555555u) | (x & 0x5555555555555555u) << 1;
    x = (x >> 2 & 0x3333333333333333u) | (x & 0x3333333333333333u) << 2;
    x = (x >> 4 & 0x0F0F0F0F0F0F0F0Fu) | (x & 0x0F0F0F0F0F0F0F0Fu) << 4;
    x = (x >> 8 & 0x00FF00FF00FF00FFu) | (x & 0x00FF00FF00FF00FFu) << 8;
    x = (x >> 16 & 0x0000FFFF0000FFFFu) | (x & 0x0000FFFF0000FFFFu) << 16;
    return x >> 32 | x << 32;
}

#define KIND(x) _Generic((x), int: 1, unsigned: 2, long: 3, unsigned long: 4, long long: 5, short: 6, default: 0)

int main(void) {
    /* what a 64-bit int means */
    {
        unsigned short us = 65535;
        unsigned char uc = 255;
        print_i64(sizeof(int));
        print_i64(INT_MAX);
        print_i64(INT_MIN);
        print_u64(UINT_MAX);
        print_i64(KIND(us * us));            /* unsigned short promotes to int */
        print_i64(us * us);                  /* no overflow in 64 bits */
        print_i64(KIND(0xFFFFFFFF));         /* a 32-bit constant is an int */
        print_i64(KIND(4294967296));
        print_i64(KIND(9223372036854775807));
        print_i64(KIND(0xFFFFFFFFFFFFFFFF)); /* only unsigned int holds it */
        print_i64(KIND(1L + 1));
        print_i64(KIND(uc + uc));
        print_i64(1 << 40);
        print_i64(-1 < 0u);                  /* -1 converts to UINT_MAX: false */
        print_i64(-1 < (unsigned short)0);   /* unsigned short promotes to int: true */
        print_i64(LONG_MAX == INT_MAX);
        print_i64(sizeof(int_least32_t));
        print_i64(INT_LEAST32_MAX == INT_MAX);
        print_i64('ab');                     /* a multicharacter constant (implementation-defined) is an int */
    }

    /* hashing */
    {
        print_u64(fnv1a(""));
        print_u64(fnv1a("a"));
        print_u64(fnv1a("the quick brown fox jumps over the lazy dog"));
        unsigned acc = 0;
        for (int i = 0; i < 10000; i++) acc += xorshift() >> 3;
        print_u64(acc);
        print_u64(xorshift());
    }

    /* primes and modular arithmetic */
    {
        int m = 1000000007;
        print_i64(powmod(2, 1000000006, m));
        print_i64(powmod(3, 200, 1000000000000000003));
        int big_primes = 0, last = 0;
        for (int n = 1000000000000000000; n < 1000000000000000000 + 400; n++)
            if (is_prime_mr(n)) { big_primes++; last = n; }
        print_i64(big_primes);
        print_i64(last);
        print_i64(is_prime_mr(4611686018427387847));   /* 2^62 - 57 */
        print_i64(sieve_count(131072));
    }

    /* Collatz records below 30000 */
    {
        int best = 0, best_n = 0, peak = 0;
        for (int n = 1; n < 30000; n++) {
            int x = n, steps = 0;
            while (x != 1) {
                x = x & 1 ? 3 * x + 1 : x / 2;
                if (x > peak) peak = x;
                steps++;
            }
            if (steps > best) { best = steps; best_n = n; }
        }
        print_i64(best);
        print_i64(best_n);
        print_i64(peak);
    }

    /* roots and perfect powers */
    {
        print_u64(isqrt(UINT_MAX));
        print_u64(isqrt(1u << 62));
        print_i64(icbrt(INT_MAX));
        print_i64(icbrt(1000000000000000000));
        int count = 0;
        for (int b = 2; b < 1000; b++) {
            int p = b * b;
            while (p <= INT_MAX / b) {
                p *= b;
                count++;
            }
        }
        print_i64(count);
    }

    /* bits */
    {
        unsigned x = 0x0123456789ABCDEFu;
        print_u64(reverse_bits(x));
        print_u64(reverse_bits(reverse_bits(x)) == x);
        int total = 0;
        unsigned gh = 0;
        for (unsigned i = 0; i < 4096; i++) {
            unsigned g = i ^ (i >> 1);
            total += popcount(g);
            gh = gh * 31 + g;
        }
        print_i64(total);
        print_u64(gh);
        print_i64(popcount(UINT_MAX));
        print_u64(~0u >> 63);
    }
    return 0;
}

/* expect: 8 */
/* expect: 9223372036854775807 */
/* expect: -9223372036854775808 */
/* expect: 18446744073709551615 */
/* expect: 1 */
/* expect: 4294836225 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 2 */
/* expect: 3 */
/* expect: 1 */
/* expect: 1099511627776 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 8 */
/* expect: 1 */
/* expect: 24930 */
/* expect: 14695981039346656037 */
/* expect: 12638187200555641996 */
/* expect: 8360034000264797104 */
/* expect: 14765938880241026033 */
/* expect: 11553976524772547768 */
/* expect: 1 */
/* expect: 775637464017619056 */
/* expect: 10 */
/* expect: 1000000000000000387 */
/* expect: 1 */
/* expect: 12251 */
/* expect: 307 */
/* expect: 26623 */
/* expect: 106358020 */
/* expect: 4294967295 */
/* expect: 2147483648 */
/* expect: 2097151 */
/* expect: 1000000 */
/* expect: 5227 */
/* expect: 17848844570815808640 */
/* expect: 1 */
/* expect: 24576 */
/* expect: 16729571146031071232 */
/* expect: 64 */
/* expect: 1 */
