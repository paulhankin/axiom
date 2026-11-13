/* flags: --c-std=gnu17 */
/* native-only */
/* linux-only */
/* The literal pool of file-scope AArch64 asm: `ldr x0, =sym` and constants,
   one pool placed by .ltorg between the functions and one at the section's
   end, the addresses relocations in the pool (R_AARCH64_ABS64). */
#include "host.h"

typedef unsigned long long u64;

#ifdef __AXIOM_ASM_AARCH64__
u64 cell = 77;
__asm__(".text\n.globl lit_addr\nlit_addr:\n\tldr x0, =cell\n\tret\n.ltorg\n"
        ".globl lit_sum\nlit_sum:\n\tldr x0, =cell + 8\n\tldr w1, =-3\n\tldrsw x2, =-3\n"
        "\tadd x0, x0, x2\n\tadd x0, x0, w1, sxtw\n\tret\n");
u64 lit_addr(void);
u64 lit_sum(void);

int main(void) {
    print_i64(lit_addr() == (u64)&cell);
    print_i64(lit_sum() == (u64)&cell + 2);
    print_i64(*(u64 *)lit_addr());
    return 0;
}
#else
int main(void) {
    print_i64(1);
    print_i64(1);
    print_i64(77);
    return 0;
}
#endif
/* expect: 1 */
/* expect: 1 */
/* expect: 77 */
