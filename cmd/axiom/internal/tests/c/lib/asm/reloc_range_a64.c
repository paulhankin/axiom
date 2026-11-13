/* flags: --c-std=gnu17 */
/* native-only */
/* linux-only */
/* A relocation whose value does not fit its field is the linker's error:
   movz of :abs_g0: (checked: the address must fit 16 bits) of an address
   beyond 64 KiB. */
/* expect-error: E9004 */
#include "print.h"

#ifdef __AXIOM_ASM_AARCH64__
long gv = 1;

int main(void) {
    long a;
    asm("movz %0, #:abs_g0:gv" : "=r"(a));
    print_i64(a);
    return 0;
}
#else
/* (elsewhere a link error of another kind keeps the expectation) */
void axiom_test_undefined(void);
int main(void) {
    axiom_test_undefined();
    return 0;
}
#endif
