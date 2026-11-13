/* flags: --c-std=gnu17 */
/* native-only */
/* not-windows */
/* The Mach-O spellings of AArch64's relocation operators, on every target
   (the same relocations as adrp's page, :lo12:, :got: and :got_lo12:):
   sym@PAGE, sym@PAGEOFF in add and in a load, sym@GOTPAGE and
   sym@GOTPAGEOFF; and adrp/add to a label of the template itself. */
#include "host.h"

typedef unsigned long long u64;

#ifdef __AXIOM_ASM_AARCH64__
long gv = 42;

int main(void) {
    long a;
    u64 p, q, r;
    asm("adrp %0, %1@PAGE\n\tldr %0, [%0, %1@PAGEOFF]" : "=r"(a) : "i"(&gv));
    print_i64(a);
    asm("adrp %0, %1@PAGE\n\tadd %0, %0, %1@PAGEOFF" : "=r"(p) : "i"(&gv));
    print_i64(p == (u64)&gv);
    asm("adrp %0, %1@GOTPAGE\n\tldr %0, [%0, %1@GOTPAGEOFF]" : "=r"(p) : "i"(&gv));
    print_i64(p == (u64)&gv);
    /* a label of the template: adrp/add and adr give its address */
    asm("adrp %0, 1f\n\tadd %0, %0, :lo12:1f\n\tadr %1, 1f\n1:\n\tadr %2, ." : "=&r"(q), "=&r"(r), "=r"(p));
    print_i64((q == r) + (r == p));
    return 0;
}
#else
int main(void) {
    print_i64(42);
    print_i64(1);
    print_i64(1);
    print_i64(2);
    return 0;
}
#endif
/* expect: 42 */
/* expect: 1 */
/* expect: 1 */
/* expect: 2 */
