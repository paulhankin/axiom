/* flags: --c-std=gnu17 */
/* native-only */
/* The rest of the base instructions assembled from inline asm and run:
   literal pools (`ldr x0, =value`, the pool placed after the template with a
   branch around it), the memory-copy and memory-set instructions, flag
   manipulation, pointer authentication's sign-and-authenticate round
   trips and ldraa, casp, the unprivileged and RCpc loads and stores, the
   fixed-point conversions, frint32z and fjcvtzs. The results follow from the
   instructions' definitions; elsewhere the same values are printed so that
   the expectations hold on every target. (CSSC is not run: the emulator
   the tests use does not have it; asm_table_a64_base.c checks its bytes.) */
#include "host.h"

typedef unsigned long long u64;
typedef long long i64;

#ifdef __AXIOM_ASM_AARCH64__
static unsigned char src[16], dst[16];
static u64 pair[2] = {5, 6};
static u64 cell = 77;

int main(void) {
    u64 a, b;
    unsigned w;
    double d;
    /* literal pools: a 64-bit, a 32-bit and a double literal */
    asm volatile("ldr %0, =0x123456789abcdef0\n\tldr %w1, =4000000000\n\tldr %d2, =0x4004000000000000"
                 : "=r"(a), "=r"(w), "=w"(d));
    print_i64((i64)(a >> 32));
    print_i64((i64)(a & 0xffffffff));
    print_i64(w);
    print_f64(d);
    /* cpyfp/cpyfm/cpyfe copy 10 bytes; setp/setm/sete fill 4 with 0xab */
    for (int i = 0; i < 16; i++) src[i] = (unsigned char)(i + 1);
    {
        unsigned char *dp = dst, *sp = src;
        u64 n = 10;
        asm volatile("cpyfp [%0]!, [%1]!, %2!\n\tcpyfm [%0]!, [%1]!, %2!\n\tcpyfe [%0]!, [%1]!, %2!"
                     : "+r"(dp), "+r"(sp), "+r"(n)
                     :
                     : "memory");
        dp = dst + 12;
        n = 4;
        u64 v = 0xab;
        asm volatile("setp [%0]!, %1!, %2\n\tsetm [%0]!, %1!, %2\n\tsete [%0]!, %1!, %2"
                     : "+r"(dp), "+r"(n)
                     : "r"(v)
                     : "memory");
    }
    for (int i = 0; i < 16; i++) print_i64(dst[i]);
    /* flags: cmp sets Z and C; cfinv inverts C; rmif rotates x1 right by 1 and
       writes N only (mask 8); setf8 of 0x80 sets N and V (bit 8 xor bit 7) and
       clears Z */
    {
        u64 f1, f2, f3;
        asm volatile("cmp xzr, xzr\n\tcfinv\n\tmrs %0, nzcv\n\t"
                     "mov x1, #0x10\n\trmif x1, #1, #8\n\tmrs %1, nzcv\n\t"
                     "mov w1, #0x80\n\tsetf8 w1\n\tmrs %2, nzcv"
                     : "=r"(f1), "=r"(f2), "=r"(f3)
                     :
                     : "x1", "cc");
        print_i64((i64)(f1 >> 28));
        print_i64((i64)(f2 >> 28));
        print_i64((i64)(f3 >> 28));
    }
    /* pointer authentication: sign then authenticate (and strip) give the pointer back */
    {
        u64 p = (u64)&cell, s, q1, q2, mod = 1234, v;
        asm volatile("mov %0, %3\n\tpacia %0, %4\n\tmov %1, %0\n\tautia %1, %4\n\tmov %2, %0\n\txpaci %2"
                     : "=&r"(s), "=&r"(q1), "=&r"(q2)
                     : "r"(p), "r"(mod));
        print_i64(q1 == p);
        print_i64(q2 == p);
        /* ldraa loads through a pointer signed with key DA and modifier 0 */
        asm volatile("mov x2, %1\n\tpacdza x2\n\tldraa %0, [x2]" : "=r"(v) : "r"(p) : "x2", "memory");
        print_i64((i64)v);
    }
    /* casp: the pair matches (5, 6), so (8, 9) is stored; the old pair comes back */
    {
        register u64 o0 asm("x4") = 5, o1 asm("x5") = 6, n0 asm("x6") = 8, n1 asm("x7") = 9;
        asm volatile("caspal x4, x5, x6, x7, [%4]" : "+r"(o0), "+r"(o1), "+r"(n0), "+r"(n1) : "r"(pair) : "memory");
        print_i64((i64)(o0 * 10 + o1));
        print_i64((i64)(pair[0] * 10 + pair[1]));
    }
    /* ldtr/sttr (at EL0 as plain accesses), stlur/ldapur with negative offsets */
    {
        u64 r1, r2;
        u64 *p = &pair[1];
        asm volatile("ldtr %0, [%2, #-8]\n\tsttr %0, [%2]\n\tmov x3, #31\n\tstlur x3, [%2, #-8]\n\tldapur %1, [%2, #-8]"
                     : "=&r"(r1), "=&r"(r2)
                     : "r"(p)
                     : "x3", "memory");
        print_i64((i64)r1);
        print_i64((i64)r2);
        print_i64((i64)pair[1]);
    }
    /* fixed point: scvtf of 40 with 4 fraction bits is 2.5; fcvtzs of 2.5
       with 8 is 640; frint32z of 3.75 is 3; fjcvtzs of -1.5 is -1 */
    {
        double x, y, z = -1.5;
        u64 n;
        int j;
        asm("scvtf %d0, %3, #4\n\tfcvtzs %1, %d0, #8\n\tfmov d1, #3.75\n\tfrint32z %d2, d1"
            : "=&w"(x), "=&r"(n), "=&w"(y)
            : "r"((u64)40)
            : "d1");
        asm("fjcvtzs %w0, %d1" : "=r"(j) : "w"(z) : "cc");
        print_f64(x);
        print_i64((i64)n);
        print_f64(y);
        print_i64(j);
    }
    return 0;
}
#else
int main(void) {
    print_i64(0x12345678);
    print_i64(0x9abcdef0);
    print_i64(4000000000);
    print_f64(2.5);
    for (int i = 0; i < 10; i++) print_i64(i + 1);
    print_i64(0);
    print_i64(0);
    for (int i = 0; i < 4; i++) print_i64(0xab);
    print_i64(4);
    print_i64(12);
    print_i64(9);
    print_i64(1);
    print_i64(1);
    print_i64(77);
    print_i64(56);
    print_i64(89);
    print_i64(8);
    print_i64(31);
    print_i64(8);
    print_f64(2.5);
    print_i64(640);
    print_f64(3.0);
    print_i64(-1);
    return 0;
}
#endif
/* expect: 305419896 */
/* expect: 2596069104 */
/* expect: 4000000000 */
/* expect: 2.5 */
/* expect: 1 */
/* expect: 2 */
/* expect: 3 */
/* expect: 4 */
/* expect: 5 */
/* expect: 6 */
/* expect: 7 */
/* expect: 8 */
/* expect: 9 */
/* expect: 10 */
/* expect: 0 */
/* expect: 0 */
/* expect: 171 */
/* expect: 171 */
/* expect: 171 */
/* expect: 171 */
/* expect: 4 */
/* expect: 12 */
/* expect: 9 */
/* expect: 1 */
/* expect: 1 */
/* expect: 77 */
/* expect: 56 */
/* expect: 89 */
/* expect: 8 */
/* expect: 31 */
/* expect: 8 */
/* expect: 2.5 */
/* expect: 640 */
/* expect: 3 */
/* expect: -1 */
