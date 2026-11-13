/* flags: --c-std=gnu17 */
/* native-only */
/* SVE memory forms and SVE2 instructions from inline asm, run under the
   user-mode emulator; results derived from the instructions' definitions. A
   gather load and a scatter store with 32-bit scaled vector offsets; a
   first-fault load whose last two elements lie in an unmapped page (the
   system's mmap and munmap called directly): the first two load and the first
   fault register keeps only their lanes; eor3 and smullb (SVE2). Elsewhere
   the same values are printed so the expectations hold on every target. */
#include "host.h"

typedef unsigned int u32;
typedef int i32;
typedef unsigned long long u64;
typedef long long i64;

#ifdef __AXIOM_ASM_AARCH64__
static u32 data[8] = {100, 101, 102, 103, 104, 105, 106, 107};
static u32 idx[4] = {6, 0, 3, 5};
static u32 out[4];
static u32 sc[8];
static u64 x3[2] = {0x00ff00ff00ff00ffull, 0x123456789abcdef0ull};
static u64 y3[2] = {0x0f0f0f0f0f0f0f0full, 0xffffffff00000000ull};
static u64 w3[2] = {0x3333333333333333ull, 0x0000ffff0000ffffull};
static u64 r3[2];
static i32 ma[4] = {3, 1000, -5, 1000};
static i32 mb[4] = {7, 1000, 9, 1000};
static i64 mr[2];

/* The system calls (Linux AArch64: number in x8, svc #0). */
static u64 sys6(u64 n, u64 a, u64 b, u64 c, u64 d, u64 e, u64 f) {
    register u64 x0 asm("x0") = a;
    register u64 x1 asm("x1") = b;
    register u64 x2 asm("x2") = c;
    register u64 xx3 asm("x3") = d;
    register u64 x4 asm("x4") = e;
    register u64 x5 asm("x5") = f;
    register u64 x8 asm("x8") = n;
    asm volatile("svc #0" : "+r"(x0) : "r"(x1), "r"(x2), "r"(xx3), "r"(x4), "r"(x5), "r"(x8) : "memory");
    return x0;
}

int main(void) {
    /* gather: out[i] = data[idx[i]] for four lanes (uxtw #2 scales the offsets by 4) */
    asm volatile(
        "mov x3, #4\n\t"
        "whilelo p0.s, xzr, x3\n\t"
        "ld1w {z1.s}, p0/z, [%1]\n\t"
        "ld1w {z0.s}, p0/z, [%2, z1.s, uxtw #2]\n\t"
        "st1w {z0.s}, p0, [%0]"
        :
        : "r"(out), "r"(idx), "r"(data)
        : "p0", "z0", "z1", "x3", "memory");
    for (int i = 0; i < 4; i++) print_i64(out[i]); /* 106 100 103 105 */
    /* scatter: sc[idx[i]] = data[i] */
    asm volatile(
        "mov x3, #4\n\t"
        "whilelo p0.s, xzr, x3\n\t"
        "ld1w {z1.s}, p0/z, [%1]\n\t"
        "ld1w {z0.s}, p0/z, [%2]\n\t"
        "st1w {z0.s}, p0, [%0, z1.s, uxtw #2]"
        :
        : "r"(sc), "r"(idx), "r"(data)
        : "p0", "z0", "z1", "x3", "memory");
    for (int i = 0; i < 8; i++) print_i64(sc[i]); /* 101 0 0 102 0 103 100 0 */
    /* first fault: map 128 KiB (two pages of any size up to 64 KiB), unmap the
       upper half, and load four words starting 8 bytes below the hole */
    u64 map = sys6(222, 0, 131072, 3, 0x22, (u64)-1, 0); /* mmap(PROT_READ|WRITE, PRIVATE|ANONYMOUS) */
    u64 unm = sys6(215, map + 65536, 65536, 0, 0, 0, 0); /* munmap */
    u32 *edge = (u32 *)(map + 65536 - 8);
    edge[0] = 41;
    edge[1] = 42;
    u64 loaded;
    asm volatile(
        "mov x3, #4\n\t"
        "whilelo p0.s, xzr, x3\n\t"
        "setffr\n\t"
        "ldff1w {z0.s}, p0/z, [%1]\n\t"
        "rdffr p1.b, p0/z\n\t"
        "cntp %0, p0, p1.s\n\t"
        "st1w {z0.s}, p1, [%2]"
        : "=r"(loaded)
        : "r"(edge), "r"(out)
        : "p0", "p1", "z0", "x3", "ffr", "memory");
    print_i64(map > 0 && map < (u64)-4096);
    print_i64((i64)unm);
    print_i64((i64)loaded); /* 2 */
    print_i64(out[0]);      /* 41 */
    print_i64(out[1]);      /* 42 */
    /* SVE2 eor3 on two doublewords, and smullb (even word lanes, widened) */
    asm volatile(
        "mov x3, #2\n\t"
        "whilelo p0.d, xzr, x3\n\t"
        "ld1d {z0.d}, p0/z, [%1]\n\t"
        "ld1d {z1.d}, p0/z, [%2]\n\t"
        "ld1d {z2.d}, p0/z, [%3]\n\t"
        "eor3 z0.d, z0.d, z1.d, z2.d\n\t"
        "st1d {z0.d}, p0, [%0]"
        :
        : "r"(r3), "r"(x3), "r"(y3), "r"(w3)
        : "p0", "z0", "z1", "z2", "x3", "memory");
    print_i64((i64)(r3[0] == (x3[0] ^ y3[0] ^ w3[0])));
    print_i64((i64)(r3[1] == (x3[1] ^ y3[1] ^ w3[1])));
    asm volatile(
        "mov x3, #4\n\t"
        "whilelo p0.s, xzr, x3\n\t"
        "ld1w {z1.s}, p0/z, [%1]\n\t"
        "ld1w {z2.s}, p0/z, [%2]\n\t"
        "smullb z0.d, z1.s, z2.s\n\t"
        "mov x3, #2\n\t"
        "whilelo p1.d, xzr, x3\n\t"
        "st1d {z0.d}, p1, [%0]"
        :
        : "r"(mr), "r"(ma), "r"(mb)
        : "p0", "p1", "z0", "z1", "z2", "x3", "memory");
    print_i64(mr[0]); /* 21 */
    print_i64(mr[1]); /* -45 */
    return 0;
}
#else
int main(void) {
    static const long long v[] = {106, 100, 103, 105, 101, 0, 0, 102, 0, 103, 100, 0, 1, 0, 2, 41, 42, 1, 1, 21, -45};
    for (unsigned i = 0; i < sizeof v / sizeof v[0]; i++) print_i64(v[i]);
    return 0;
}
#endif
/* expect: 106 */
/* expect: 100 */
/* expect: 103 */
/* expect: 105 */
/* expect: 101 */
/* expect: 0 */
/* expect: 0 */
/* expect: 102 */
/* expect: 0 */
/* expect: 103 */
/* expect: 100 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 2 */
/* expect: 41 */
/* expect: 42 */
/* expect: 1 */
/* expect: 1 */
/* expect: 21 */
/* expect: -45 */
