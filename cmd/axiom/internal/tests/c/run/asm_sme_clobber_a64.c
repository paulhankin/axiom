/* flags: --c-std=gnu17 */
/* native-only */
/* The SME state clobbers `za` (the ZA array) and `zt0` (the ZT0 lookup
   table) on AArch64: a statement that enters and leaves streaming/ZA mode
   names them, and the compiler, which keeps nothing in them across an asm,
   saves nothing for them. The value around the asm is unchanged. Elsewhere
   the same value is printed. */
#include "host.h"

static long around(long x) {
    long r = x + 1;
#ifdef __AXIOM_ASM_AARCH64__
    __asm__ volatile(".arch_extension sme\n\tsmstart za\n\tsmstop za" : : : "za", "zt0", "memory");
#endif
    return r + 1;
}

int main(void) {
    print_i64(around(40));
    return 0;
}
/* expect: 42 */
