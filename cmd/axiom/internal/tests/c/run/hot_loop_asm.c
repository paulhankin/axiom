/* flags: --c-std=gnu17 -O2 */
/* native-only */
/* Inline assembly in loops with calls on a path that seldom runs (see
   hot_loop_split.c): an early-clobber output (`=&r`) and register clobbers
   in the hot stretch, beside values held in registers through it and in
   memory on the cold path, and the same asm on the cold path. The output
   is written before the inputs are read, and the clobbered registers hold
   nothing live across the asm. */
#include "host.h"

typedef unsigned long long u64;

#ifdef __AXIOM_ASM_X86_64__
#define ADD2 "mov %1, %0\n\tadd %2, %0"
#define CLOB1 "rdx"
#define CLOB2 "rcx"
#else
#define ADD2 "add %0, %1, %2"
#define CLOB1 "x9"
#define CLOB2 "x10"
#endif

static u64 tab[64];
static volatile u64 sink;

__attribute__((noinline)) static u64 mix(u64 a, u64 b) {
    sink += a;
    return (a * 31) + b;
}

/* a cold path with a loop of many values of its own, then a call */
static inline u64 rebuild(u64 seed, u64 n) {
    u64 s0 = seed, s1 = seed ^ 1, s2 = seed ^ 2, s3 = seed ^ 3, s4 = seed ^ 4, s5 = seed ^ 5;
    u64 s6 = seed ^ 6, s7 = seed ^ 7, s8 = seed ^ 8, s9 = seed ^ 9, s10 = seed ^ 10, s11 = seed ^ 11;
    for (u64 j = 0; j < n && j < 64; j++) {
        u64 t = tab[j];
        s0 += t;
        s1 ^= t * 3;
        s2 += s0 >> 3;
        s3 ^= s1 + t;
        s4 += s2 * 5;
        s5 ^= s3 >> 1;
        s6 += s4 ^ t;
        s7 ^= s5 + 9;
        s8 += s6 >> 2;
        s9 ^= s7 * 7;
        s10 += s8 ^ s9;
        s11 ^= s10 + s0;
        tab[j] = s11 ^ s1;
    }
    return mix(s0 ^ s1 ^ s2 ^ s3 ^ s4 ^ s5, s6 ^ s7 ^ s8 ^ s9 ^ s10 ^ s11);
}

__attribute__((noinline)) static u64 early(u64 n, u64 a, u64 b, u64 c, u64 d, u64 e) {
    u64 x = a, y = 7, cnt = 0, lim = 4, acc = 0;
    for (u64 i = 0; i < n; i++) {
        if (__builtin_expect(cnt == lim, 0)) {
            lim *= 2;
            acc ^= rebuild(acc + i, lim);
        }
        x = (x * b) + c;
        if (x & 4) {
            y = x ^ d;
        }
        u64 o;
        __asm__(ADD2 : "=&r"(o) : "r"(x), "r"(e));
        acc += o ^ y;
        tab[cnt & 63] = acc;
        cnt++;
    }
    return acc + y + lim;
}

__attribute__((noinline)) static u64 clobbers(u64 n, u64 a, u64 b, u64 c, u64 d, u64 e, u64 stop) {
    u64 x = a, y = 7, cnt = 0, lim = 2, acc = 0;
    for (u64 i = 0; i < n; i++) {
        if (__builtin_expect(cnt >= lim, 0)) {
            lim = (lim * 2) + 1;
            u64 p;
            __asm__(ADD2 : "=&r"(p) : "r"(acc), "r"(lim) : "cc", CLOB1, CLOB2);
            acc = rebuild(p, lim) ^ mix(y, x);
        }
        x = (x * b) ^ c;
        u64 o;
        __asm__(ADD2 : "=r"(o) : "r"(x), "r"(e) : "cc", CLOB1, CLOB2);
        if ((o & 0xff) == stop) {
            break;
        }
        if (o & 8) {
            y += o ^ d;
        }
        acc += o + (y >> 3);
        tab[cnt & 63] ^= acc;
        cnt++;
    }
    return acc ^ (y * 3) ^ lim;
}

/* the asm's output replaces a loop variable each time around */
__attribute__((noinline)) static u64 carried(u64 n, u64 v, u64 a, u64 b, u64 c) {
    u64 cnt = 0, lim = 3, acc = 0;
    for (u64 i = 0; i < n; i++) {
        if (__builtin_expect(cnt == lim, 0)) {
            lim *= 3;
            acc += rebuild(v ^ acc, lim);
        }
        u64 o;
        __asm__(ADD2 : "=&r"(o) : "r"(v), "r"(a));
        v = (o * b) + c;
        acc ^= v;
        cnt++;
    }
    return acc + v;
}

static volatile u64 P[8] = {3000, 1, 6364136223846793005ull, 1442695040888963407ull, 0x5555, 77, 0x3c, 9};

int main(void) {
    print_u64(early(P[0], P[1], P[2], P[3], P[4], P[5]));
    print_u64(clobbers(P[0], P[1], P[2], P[3], P[4], P[5], P[6]));
    print_u64(clobbers(P[0], P[7], P[2], P[3], P[4], P[5], 256));
    print_u64(carried(P[0], P[7], P[5], P[2], P[3]));
    print_u64(sink);
    return 0;
}
/* expect: 13762033029520925513 */
/* expect: 16736142370364993619 */
/* expect: 15709782780013064412 */
/* expect: 1268888948040395736 */
/* expect: 2977321041807598806 */
