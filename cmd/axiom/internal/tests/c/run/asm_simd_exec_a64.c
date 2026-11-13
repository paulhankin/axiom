/* flags: --c-std=gnu17 */
/* native-only */
/* Advanced SIMD and cryptographic instructions assembled from inline asm and
   run: results derived from the instructions' definitions (a dot product, a
   structure load's de-interleaving, a table lookup, ext, a sum across lanes,
   a carry-less multiply, a byte-mask movi, the copy instructions, a complex
   add, one AES round of a zero state and key). Elsewhere the same lines are
   printed so that the expectations hold on every target. */
#include "host.h"

typedef unsigned long long u64;
typedef unsigned int u32;

#ifdef __AXIOM_ASM_AARCH64__
static unsigned char seq[48], twos[16], rev[16], tab[16];

int main(void) {
    for (int i = 0; i < 48; i++) seq[i] = (unsigned char)i;
    for (int i = 0; i < 16; i++) {
        twos[i] = 2;
        rev[i] = (unsigned char)(15 - i);
        tab[i] = (unsigned char)(i * 10);
    }
    u32 dot[4];
    /* sdot: each 32-bit lane gets the sum of four byte products: 2 * (4i+1 + ... + 4i+4), i.e. 2 * (16i + 10) with bytes 1..16 */
    asm volatile("ld1 {v1.16b}, [%1]\n\tld1 {v2.16b}, [%2]\n\tmovi v0.4s, #0\n\t"
                 "sdot v0.4s, v1.16b, v2.16b\n\tst1 {v0.4s}, [%0]"
                 :
                 : "r"(dot), "r"(seq + 1), "r"(twos)
                 : "v0", "v1", "v2", "memory");
    for (int i = 0; i < 4; i++) print_i64(dot[i]);
    /* ld3 de-interleaves: the second register gets bytes 1, 4, ..., 46 (sum 376); uaddlv adds them */
    u32 s3;
    asm volatile("ld3 {v0.16b, v1.16b, v2.16b}, [%1]\n\tuaddlv h3, v1.16b\n\tumov %w0, v3.h[0]"
                 : "=r"(s3)
                 : "r"(seq)
                 : "v0", "v1", "v2", "v3", "memory");
    print_i64(s3);
    /* tbl with the indexes reversed: the first byte is tab[15] = 150; ext #3 of seq: seq[3] */
    u32 t0, e0;
    asm volatile("ld1 {v0.16b}, [%2]\n\tld1 {v1.16b}, [%3]\n\ttbl v2.16b, {v0.16b}, v1.16b\n\t"
                 "umov %w0, v2.b[0]\n\tld1 {v3.16b, v4.16b}, [%4]\n\text v5.16b, v3.16b, v4.16b, #3\n\t"
                 "umov %w1, v5.b[0]"
                 : "=r"(t0), "=r"(e0)
                 : "r"(tab), "r"(rev), "r"(seq)
                 : "v0", "v1", "v2", "v3", "v4", "v5", "memory");
    print_i64(t0);
    print_i64(e0);
    /* pmull: 0b101 times 0b11 without carries is 0b1111 */
    u64 pm;
    asm volatile("mov x9, #5\n\tfmov d1, x9\n\tmov x9, #3\n\tfmov d2, x9\n\t"
                 "pmull v0.1q, v1.1d, v2.1d\n\tumov %0, v0.d[0]"
                 : "=r"(pm)
                 :
                 : "x9", "v0", "v1", "v2");
    print_i64((long long)pm);
    /* movi of a byte mask; dup of a general register and addp of the two lanes */
    u64 mk, sum;
    asm volatile("movi v0.2d, #0xff00ff00ff00ff00\n\tumov %0, v0.d[1]\n\t"
                 "mov x9, #21\n\tdup v1.2d, x9\n\taddp d2, v1.2d\n\tmov %1, v2.d[0]"
                 : "=r"(mk), "=r"(sum)
                 :
                 : "x9", "v0", "v1", "v2");
    print_i64(mk == 0xff00ff00ff00ff00ull);
    print_i64((long long)sum);
    /* fcadd #90 of (1 + 2i) and (3 + 4i): (1 - 4) + (2 + 3)i */
    float cx[2];
    asm volatile("fmov s0, #1.0\n\tfmov s1, #2.0\n\tins v0.s[1], v1.s[0]\n\t"
                 "fmov s2, #3.0\n\tfmov s3, #4.0\n\tins v2.s[1], v3.s[0]\n\t"
                 "fcadd v4.2s, v0.2s, v2.2s, #90\n\tst1 {v4.2s}, [%0]"
                 :
                 : "r"(cx)
                 : "v0", "v1", "v2", "v3", "v4", "memory");
    print_i64((long long)cx[0]);
    print_i64((long long)cx[1]);
    /* aese of a zero state with a zero key: SubBytes(0) is 0x63 in every byte */
    u64 ae;
    asm volatile("movi v0.16b, #0\n\tmovi v1.16b, #0\n\taese v0.16b, v1.16b\n\tumov %0, v0.d[0]"
                 : "=r"(ae)
                 :
                 : "v0", "v1");
    print_i64(ae == 0x6363636363636363ull);
    return 0;
}
#else
int main(void) {
    long long want[] = {20, 52, 84, 116, 376, 150, 3, 15, 1, 42, -3, 5, 1};
    for (int i = 0; i < 13; i++) print_i64(want[i]);
    return 0;
}
#endif
/* expect: 20 */
/* expect: 52 */
/* expect: 84 */
/* expect: 116 */
/* expect: 376 */
/* expect: 150 */
/* expect: 3 */
/* expect: 15 */
/* expect: 1 */
/* expect: 42 */
/* expect: -3 */
/* expect: 5 */
/* expect: 1 */
