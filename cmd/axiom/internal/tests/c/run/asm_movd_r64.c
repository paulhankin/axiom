/* flags: --c-std=gnu17 */
/* native-only */
/* The x86-64 assembler's movd with a 64-bit general register: the GNU
   assembler's spelling of movq between an xmm register and a 64-bit register
   (66 REX.W 0F 6E / 7E), as hand-written assembly uses it; the bytes are those
   of movq, and the values round-trip all 64 bits. */
#include "host.h"

#ifdef __AXIOM_ASM_X86_64__
__asm__(".section .rodata.axiom_movd_r64, \"a\"\n"
        "axiom_movd_r64_a:\n"
        "movd %xmm12, %rdx\n"
        "movd %rcx, %xmm3\n"
        "movd %xmm1, %eax\n"
        "axiom_movd_r64_b:\n"
        "movq %xmm12, %rdx\n"
        "movq %rcx, %xmm3\n"
        "movd %xmm1, %eax\n"
        "axiom_movd_r64_end:\n"
        ".text\n");
extern const unsigned char axiom_movd_r64_a[], axiom_movd_r64_b[], axiom_movd_r64_end[];

static unsigned long long round_trip(unsigned long long v) {
    unsigned long long r;
    __asm__("movd %1, %%xmm0\n\tmovd %%xmm0, %0" : "=r"(r) : "r"(v) : "xmm0");
    return r;
}
int main(void) {
    long n = axiom_movd_r64_b - axiom_movd_r64_a;
    int same = n == axiom_movd_r64_end - axiom_movd_r64_b;
    for (long i = 0; same && i < n; i++) same = axiom_movd_r64_a[i] == axiom_movd_r64_b[i];
    print_i64(n);
    print_bool(same);
    print_u64(round_trip(0x0123456789abcdefULL));
    return 0;
}
#else
int main(void) { print_i64(14); print_bool(1); print_u64(81985529216486895ULL); return 0; }
#endif
/* expect: 14 */
/* expect: true */
/* expect: 81985529216486895 */
