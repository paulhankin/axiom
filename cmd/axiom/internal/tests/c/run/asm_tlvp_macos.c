/* flags: --c-std=gnu17 */
/* native-only */
/* macos-only */
/* AArch64 macOS: a thread-local's descriptor through @TLVPPAGE/@TLVPPAGEOFF
   (an adrp/ldr pair loading the descriptor's address from its GOT slot),
   then the platform's call through the descriptor's first word, which
   returns the variable's address in x0. It is the address C takes of it,
   and stores through it are the variable's. On x86-64 macOS the same
   values are printed. */
#include "host.h"

_Thread_local long tv = 40;

static long *tlv_address(void) {
    long *p;
#if defined(__AXIOM_ASM_AARCH64__)
    __asm__ volatile("adrp x0, tv@TLVPPAGE\n\t"
                     "ldr x0, [x0, tv@TLVPPAGEOFF]\n\t"
                     "ldr x1, [x0]\n\t"
                     "blr x1\n\t"
                     "mov %0, x0"
                     : "=r"(p)
                     :
                     : "x0", "x1", "x30", "cc", "memory");
#else
    p = &tv;
#endif
    return p;
}

int main(void) {
    long *p = tlv_address();
    print_i64(p == &tv);
    *p += 2;
    print_i64(tv);
    return 0;
}
/* expect: 1 */
/* expect: 42 */
