/* flags: --c-std=gnu17 -O2 */
/* native-only */
/* Inline assembly in a loop's hot stretch, held in registers there and in
   memory on the loop's colder path, which calls (see hot_stretch_split.c,
   whose shapes these are): an early-clobber output (`=&r`, written before
   the inputs are read), an asm clobbering two registers and the flags (they
   hold nothing live across it), and a colder path whose pressure is an asm
   with eight early-clobber outputs. At the time of writing, with
   AXIOM_DEBUG_RA, each of these functions takes the hot-stretch split on
   x86-64 at -O2 (in `clobbers` and `spread`, for values defined in the
   stretch too), and on AArch64 `wide` does. Also: a value defined on one
   side of a branch in the stretch and read on the colder path, and exits
   from the middle of the stretch (a break and a return). */
#include "host.h"

typedef unsigned long long u64;

#ifdef __AXIOM_ASM_X86_64__
#define ADD2 "mov %1, %0\n\tadd %2, %0"
#define MIX2 "mov %1, %%rdx\n\timul %2, %%rdx\n\tmov %%rdx, %%rcx\n\tshr $7, %%rcx\n\txor %%rcx, %%rdx\n\tmov %%rdx, %0"
#define CLOB1 "rdx"
#define CLOB2 "rcx"
#define SPREAD                                                                                          \
    "mov %8, %0\n\tmov %8, %1\n\tmov %8, %2\n\tmov %8, %3\n\tmov %8, %4\n\tmov %8, %5\n\tmov %8, %6\n\t" \
    "mov %8, %7\n\tadd %9, %0\n\tadd %10, %1\n\tadd %11, %2\n\tadd %12, %3\n\tadd %13, %4\n\t"           \
    "xor %9, %5\n\txor %10, %6\n\txor %11, %7"
#else
#define ADD2 "add %0, %1, %2"
#define MIX2 "mul x9, %1, %2\n\teor x10, x9, x9, lsr #7\n\tmov %0, x10"
#define CLOB1 "x9"
#define CLOB2 "x10"
#define SPREAD                                                                                          \
    "add %0, %8, %9\n\tadd %1, %8, %10\n\tadd %2, %8, %11\n\tadd %3, %8, %12\n\tadd %4, %8, %13\n\t"     \
    "eor %5, %8, %9\n\teor %6, %8, %10\n\teor %7, %8, %11"
#endif

static u64 tab[64];
static volatile u64 sink;

__attribute__((noinline)) static u64 grow(u64 cap) {
    sink += cap;
    return (cap * 2) + 1;
}

/* the cold path's work before its call: a chain of values all live at once
   (each is read again in reverse order), more than there are registers */
#define CHURN(r, a, b, c, d, e, f, g, h)                                                        \
    do {                                                                                        \
        u64 t0 = (a) ^ (b), t1 = (t0 * (c)) ^ (d), t2 = (t1 * (e)) ^ (f), t3 = (t2 * (b)) ^ (g); \
        u64 t4 = (t3 * (c)) ^ (h), t5 = (t4 * (d)) ^ 5, t6 = (t5 * (e)) ^ 6, t7 = (t6 * (f)) ^ 7; \
        u64 t8 = (t7 * (b)) ^ 8, t9 = (t8 * (c)) ^ 9, t10 = (t9 * (d)) ^ 10;                    \
        u64 t11 = (t10 * (e)) ^ 11;                                                             \
        r = t11;                                                                                \
        r = (r * 3) ^ t10;                                                                      \
        r = (r * 3) ^ t9;                                                                       \
        r = (r * 3) ^ t8;                                                                       \
        r = (r * 3) ^ t7;                                                                       \
        r = (r * 3) ^ t6;                                                                       \
        r = (r * 3) ^ t5;                                                                       \
        r = (r * 3) ^ t4;                                                                       \
        r = (r * 3) ^ t3;                                                                       \
        r = (r * 3) ^ t2;                                                                       \
        r = (r * 3) ^ t1;                                                                       \
        r = (r * 3) ^ t0;                                                                       \
    } while (0)

/* an early-clobber output in the stretch, read with a value (y) defined
   there on one side of a branch, both read on the cold path */
__attribute__((noinline)) static u64 early(u64 n, const volatile u64 *p) {
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
        u64 o;
        __asm__(ADD2 : "=&r"(o) : "r"(x), "r"(k4));
        acc += (o ^ y) + (k5 & x) + (len ^ i);
        tab[len & 63] = acc;
        len++;
    }
    return acc + len + cap + y;
}

/* an asm clobbering two registers (and the flags) in the stretch, exits
   from the middle of it (a break and a return) */
__attribute__((noinline)) static u64 clobbers(u64 n, const volatile u64 *p, u64 stop) {
    u64 k0 = p[0], k1 = p[1], k2 = p[2], k3 = p[3], k4 = p[4], k5 = p[5];
    u64 cap = 2, len = 0, x = k0, acc = 0, y = 7;
    for (u64 i = 0; i < n; i++) {
        if (len >= cap && (len & 1) == 0) {
            u64 r;
            CHURN(r, x, k1, k2, k3, k4, k5, y, acc);
            cap = grow(cap);
            acc = r ^ cap;
        }
        x = (x * k1) ^ k2;
        u64 o;
        __asm__(MIX2 : "=r"(o) : "r"(x), "r"(k3) : "cc", CLOB1, CLOB2);
        if ((o & 0xff) == stop) {
            break;
        }
        if ((x >> 56) == stop) {
            return acc ^ 1000;
        }
        if (o & 8) {
            y += o ^ k4;
        }
        acc += o + (y >> 3) + (k5 | len);
        tab[len & 63] ^= acc;
        len++;
    }
    return acc ^ (y * 3) ^ cap;
}

/* the cold path's pressure from an asm with many early-clobber outputs,
   and both asms in the stretch */
__attribute__((noinline)) static u64 spread(u64 n, const volatile u64 *p) {
    u64 k0 = p[0], k1 = p[1], k2 = p[2], k3 = p[3], k4 = p[4], k5 = p[5];
    u64 cap = 4, len = 0, x = k0, acc = 0, y = 7;
    for (u64 i = 0; i < n; i++) {
        if (__builtin_expect(len == cap, 0)) {
            u64 a0, a1, a2, a3, a4, a5, a6, a7;
            __asm__(SPREAD
                    : "=&r"(a0), "=&r"(a1), "=&r"(a2), "=&r"(a3), "=&r"(a4), "=&r"(a5), "=&r"(a6), "=&r"(a7)
                    : "r"(x), "r"(k1), "r"(k2), "r"(k3), "r"(k4), "r"(k5));
            cap = grow(cap + (((a0 ^ a1) + (a2 ^ a3) + (a4 ^ a5) + (a6 ^ a7)) & 1));
            acc ^= y + a7;
        }
        x = (x * k1) + k2;
        if (x & 4) {
            y = x ^ k3;
        }
        u64 o, m;
        __asm__(ADD2 : "=&r"(o) : "r"(x), "r"(k4));
        __asm__(MIX2 : "=r"(m) : "r"(o), "r"(k5) : "cc", CLOB1, CLOB2);
        acc += (o ^ y) + m + (len ^ i);
        tab[len & 63] = acc;
        len++;
    }
    return acc + len + cap + y;
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
        u64 o, m;
        __asm__(ADD2 : "=&r"(o) : "r"(x), "r"(k4));
        __asm__(MIX2 : "=r"(m) : "r"(o), "r"(k5) : "cc", CLOB1, CLOB2);
        acc += (o ^ y) + m + (k6 | i) + (k7 * len) + (k8 >> (i & 7)) + (x ^ k9) + (k10 & y);
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
    print_u64(early(P[0], P + 1));
    print_u64(clobbers(P[0], P + 1, 0x3c));
    print_u64(clobbers(P[0], P + 1, 0x1ff));
    print_u64(spread(P[0], P + 1));
    print_u64(wide(P[0], P + 1));
    print_u64(sink);
    return 0;
}
/* expect: 2911557719205581977 */
/* expect: 14251693606736787124 */
/* expect: 17987959771154834266 */
/* expect: 11104081066121384105 */
/* expect: 5604296462458012704 */
/* expect: 17011 */
