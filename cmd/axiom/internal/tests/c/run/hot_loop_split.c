/* flags: --c-std=gnu17 -O2 */
/* Loops whose values outnumber the callee-saved registers, with calls on a
   path through the loop that seldom runs (a grow path): the shapes the
   register allocator's hot-stretch splitting is for. They check the output,
   not the allocation: at the time of writing the allocator takes other
   splits in these functions (tests/run/hot_stretch_*.ax take the new one).
   Shapes: a value changed only on the hot path and read on the cold one, a
   value changed only on the cold path and read on the hot one, exits from
   the middle of the loop, a loop inside an outer loop, and a cold path with
   a loop of its own. */
#include "host.h"

typedef unsigned long long u64;

static volatile u64 sink;

__attribute__((noinline)) static u64 grow(u64 cap) {
    sink += cap;
    return (cap * 2) + 1;
}

__attribute__((noinline)) static u64 note(u64 a, u64 b) {
    sink ^= a + b;
    return a ^ (b >> 3);
}

/* many invariants, a cold call, and a value (y) changed on the hot path on
   one side of a branch only, read on the cold path and after the loop */
__attribute__((noinline)) static u64 invariants(u64 n, u64 a, u64 b, u64 c, u64 d, u64 e, u64 g, u64 h,
                                                u64 k) {
    u64 cap = 4, len = 0, x = a, acc = 0, y = 7;
    for (u64 i = 0; i < n; i++) {
        if (__builtin_expect(len == cap, 0)) {
            cap = grow(cap);
            acc ^= note(y, a * b);
        }
        x = (x * b) + c;
        if (x & 4) {
            y = x ^ d;
        }
        acc += (x ^ d) + (e & x) + (g | i) + (h * len) + (k >> (i & 7));
        len++;
    }
    return acc + len + cap + y;
}

/* a value (last) changed only on the cold path, read on the hot one; exits
   from the middle of the loop */
__attribute__((noinline)) static u64 exits(u64 n, u64 a, u64 b, u64 c, u64 d, u64 e, u64 g, u64 stop) {
    u64 cap = 2, len = 0, x = a, acc = 0, last = 1;
    for (u64 i = 0; i < n; i++) {
        if (__builtin_expect(len >= cap, 0)) {
            cap = grow(cap);
            last = note(cap, x);
        }
        x = (x * 6364136223846793005ull) + b;
        if ((x >> 56) == stop) {
            return acc + 1000;
        }
        acc += (x >> 7) ^ (last * c);
        if ((acc & 0x3ff) == stop) {
            break;
        }
        acc += (d & x) + (e | len) + g;
        len++;
    }
    return acc + len + last;
}

/* the same stretch in an inner loop, values of the outer one live through */
__attribute__((noinline)) static u64 nested(u64 rounds, u64 n, u64 a, u64 b, u64 c, u64 d, u64 e) {
    u64 total = 0;
    for (u64 r = 0; r < rounds; r++) {
        u64 cap = 1, len = 0, x = a + r, acc = r;
        for (u64 i = 0; i < n; i++) {
            if (__builtin_expect(len == cap, 0)) {
                cap = grow(cap);
                acc += note(total, r);
            }
            x = (x * b) ^ c;
            acc += (x & d) + (e ^ i) + (total >> 3);
            len++;
        }
        total = (total * 31) + acc + cap;
    }
    return total;
}

/* a cold path with a loop and calls of its own (a copy into a new buffer) */
__attribute__((noinline)) static u64 copying(u64 n, u64 a, u64 b, u64 c, u64 d, u64 e, u64 g) {
    u64 buf[64];
    u64 cap = 2, len = 0, x = a, acc = 0;
    for (u64 i = 0; i < n; i++) {
        if (__builtin_expect(len == cap, 0)) {
            u64 nc = grow(cap);
            for (u64 j = 0; j < len && j < 64; j++) {
                acc += note(buf[j], j);
            }
            cap = nc;
        }
        x = (x * b) + c;
        if (len < 64) {
            buf[len] = x ^ d;
        }
        acc += (x & e) + (g ^ len);
        len++;
    }
    return acc + len + cap;
}

static volatile u64 P[16] = {300,
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
    u64 n = P[0];
    print_u64(invariants(n, P[1], P[2], P[3], P[4], P[5], P[6], P[7], P[8]));
    print_u64(exits(n, P[7], P[3], P[15], P[4], P[9], P[14], P[9]));
    print_u64(exits(n, P[7], P[3], P[15], P[4], P[9], P[14], n));
    print_u64(nested(P[15], n, P[7], P[10], P[11], P[13], P[9]));
    print_u64(copying(n, P[15], P[10], P[11], P[12], P[13], P[14]));
    print_u64(sink);
    return 0;
}
/* expect: 15792383430627634139 */
/* expect: 2991699413079285500 */
/* expect: 2907026499592387734 */
/* expect: 22487329765941 */
/* expect: 8718417590152267263 */
/* expect: 8664349408834993565 */
