/* flags: --c-std=gnu17 */
/* AMX's tile registers as clobbers (tmm0-tmm7, with or without %): accepted on x86-64, where the compiler keeps nothing in them. */
#include "host.h"

int main(void) {
    int x = 41;
#ifdef __AXIOM_ASM_X86_64__
    __asm__ volatile("nop" : "+r"(x) : : "tmm0", "%tmm7", "memory");
#endif
    x += 1;
    print_i64(x);
    return 0;
}
/* expect: 42 */
