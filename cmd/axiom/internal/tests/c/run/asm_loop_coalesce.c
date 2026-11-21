/* flags: --c-std=gnu17 */
/* native-only */
/* Inline assembly in a loop whose variable is replaced by the asm's output
   each iteration: the phi, its copy and the output may be joined into one
   register only where none is defined while another is live. An
   early-clobber output (`=&r`, or any output of an asm with a register
   clobber) is written before the inputs are read, so it must not share the
   register of the input it replaces. */
#include "host.h"

typedef unsigned long long u64;

#ifdef __AXIOM_ASM_X86_64__
#define TWICE "mov $0, %0\n\tadd %1, %0\n\tadd %1, %0"
#define CLOB "rdx"
#else
#define TWICE "mov %0, #0\n\tadd %0, %0, %1\n\tadd %0, %0, %1"
#define CLOB "x9"
#endif

__attribute__((noinline)) static u64 early(u64 v, int n) {
    for (int i = 0; i < n; i++) {
        u64 o;
        __asm__(TWICE : "=&r"(o) : "r"(v));
        v = o;
    }
    return v;
}

__attribute__((noinline)) static u64 clobber(u64 v, int n) {
    for (int i = 0; i < n; i++) {
        u64 o;
        __asm__(TWICE : "=r"(o) : "r"(v) : "cc", CLOB);
        v = o;
    }
    return v;
}

/* the same with a second value carried around the loop beside it */
__attribute__((noinline)) static u64 two(u64 v, u64 w, int n) {
    for (int i = 0; i < n; i++) {
        u64 o;
        __asm__(TWICE : "=&r"(o) : "r"(v));
        v = o;
        w = w * 3 + v;
    }
    return v + w;
}

static volatile int five = 5;

int main(void) {
    print_i64((long long)early(1, five));
    print_i64((long long)clobber(3, five));
    print_i64((long long)two(1, 1, five));
    return 0;
}
/* expect: 32 */
/* expect: 96 */
/* expect: 697 */
