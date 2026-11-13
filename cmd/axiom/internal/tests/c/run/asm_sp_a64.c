/* flags: --c-std=gnu17 */
/* native-only */
/* The AArch64 constraint 'k': the stack pointer, as the operand that is the
   register variable declared asm("sp") (written `sp` in the template), read
   by mov and add; it is what `mov x, sp` reads. Elsewhere the same values
   are printed. */
#include "host.h"

int main(void) {
#ifdef __AXIOM_ASM_AARCH64__
    register void *spv asm("sp");
    unsigned long long a, b, c;
    asm("mov %0, %1" : "=r"(a) : "k"(spv));
    asm("mov %0, sp" : "=r"(b));
    asm("add %0, %1, #16" : "=r"(c) : "k"(spv));
    print_bool(a == b);
    print_bool(c == b + 16);
    print_bool((b & 15) == 0);
#else
    print_bool(1);
    print_bool(1);
    print_bool(1);
#endif
    return 0;
}
/* expect: true */
/* expect: true */
/* expect: true */
