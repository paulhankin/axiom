/* flags: --c-std=gnu17 */
/* native-only */
/* An absolute address in the four move-wide groups (:abs_g3: ... :abs_g0_nc:,
   in either order, the movz first), on every AArch64 target: ELF relocates
   the instructions; on Mach-O and PE, whose images have no such relocation,
   the sequence becomes adrp and add (and two nops), the same address
   wherever the loader puts the image. A difference of two addresses in a
   .word fits on every target. Elsewhere the same values are printed. */
#include "print.h"

long gv[4] = {1, 2, 3, 4};

#ifdef __AXIOM_ASM_AARCH64__
__asm__(".data\n.p2align 2\naxiom_test_diff: .word gv - axiom_test_diff\n.text\n");
extern int axiom_test_diff;
#endif

int main(void) {
#ifdef __AXIOM_ASM_AARCH64__
    long *p, *q;
    asm("movz %0, #:abs_g3:gv+8\n\tmovk %0, #:abs_g2_nc:gv+8\n\tmovk %0, #:abs_g1_nc:gv+8\n\tmovk %0, #:abs_g0_nc:gv+8"
        : "=r"(p));
    asm("movz %0, #:abs_g0_nc:gv\n\tmovk %0, #:abs_g1_nc:gv\n\tmovk %0, #:abs_g2_nc:gv\n\tmovk %0, #:abs_g3:gv"
        : "=r"(q));
    print_i64(*p);
    print_i64(*q);
    print_bool((char *)&axiom_test_diff + axiom_test_diff == (char *)gv);
#else
    print_i64(2);
    print_i64(1);
    print_bool(1);
#endif
    return 0;
}
/* expect: 2 */
/* expect: 1 */
/* expect: true */
