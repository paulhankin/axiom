/* flags: --c-std=gnu17 */
/* native-only */
/* Constraints of several alternatives: the one chosen is the same for all
   the operands, the cheapest for them (a constant where one is allowed, then
   a register, then memory; '?' and '!' disparage an alternative); the
   templates here assemble only with the alternative the rules choose. */
#include "host.h"

int g = 40;

int main(void) {
    int r;
#ifdef __AXIOM_ASM_X86_64__
    // ('!': memory, which lea needs)
    __asm__("lea %1, %0" : "=r,r"(r) : "!r,m"(g));
    print_i64(r == (int)(long long)&g);
    // (a constant where one is allowed: $5)
    __asm__("movl %1, %0" : "=r,r"(r) : "r,i"(5));
    print_i64(r);
    // ('??' on the register alternative (2 + 1), memory (2) chosen: addl to memory)
    int m = 2;
    __asm__("addl $3, %0" : "+??r,m"(m));
    print_i64(m);
    // (a register, cheaper than memory)
    __asm__("imull %2, %0" : "=r,r"(r) : "0,0"(6), "r,m"(7));
    print_i64(r);
#else
    __asm__("ldr %w0, %1" : "=r,r"(r) : "!r,m"(g));
    print_i64(r == 40);
    __asm__("mov %w0, %1" : "=r,r"(r) : "r,i"(5));
    print_i64(r);
    int m = 2;
    __asm__("ldr w9, %0\n\tadd w9, w9, #3\n\tstr w9, %0" : "+??r,m"(m) : : "x9");
    print_i64(m);
    __asm__("mul %w0, %w1, %w2" : "=r,r"(r) : "r,r"(6), "r,m"(7));
    print_i64(r);
#endif
    return 0;
}
/* expect: 1 */
/* expect: 5 */
/* expect: 5 */
/* expect: 42 */
