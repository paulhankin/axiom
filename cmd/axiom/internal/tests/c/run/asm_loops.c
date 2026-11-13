/* flags: --c-std=gnu17 */
/* native-only */
/* Inline assembly in the loops the loop passes rewrite (idioms.ax,
   vectorize.ax): a pure asm (no volatile, register operands only) that
   computes each element, and one that defines a named label (ir.ASM_NODUP,
   so it must not be copied: a second copy would define the label twice and
   not assemble), stay in their loops, at each element, with the results of
   the loops without them. */
#include "host.h"

static void twice(int *restrict y, const int *restrict x, int n) {
    for (int i = 0; i < n; i++) {
        int t;
#if defined(__AXIOM_ASM_X86_64__)
        asm("leal (%1,%1), %0" : "=r"(t) : "r"(x[i]));
#elif defined(__AXIOM_ASM_AARCH64__)
        asm("add %w0, %w1, %w1" : "=r"(t) : "r"(x[i]));
#else
        t = x[i] * 2;
#endif
        y[i] = t + 1;
    }
}

static void labelled(int *restrict y, const int *restrict x, int n) {
    for (int i = 0; i < n; i++) {
        int t;
#if defined(__AXIOM_ASM_X86_64__)
        asm("asm_loops_once:\n\tmovl %1, %0" : "=r"(t) : "r"(x[i]));
#elif defined(__AXIOM_ASM_AARCH64__)
        asm("asm_loops_once:\n\tmov %w0, %w1" : "=r"(t) : "r"(x[i]));
#else
        t = x[i];
#endif
        y[i] = t;
    }
}

int main(void) {
    static int x[1000], y[1000], z[1000];
    for (int i = 0; i < 1000; i++) {
        x[i] = i * 7 - 300;
    }
    twice(y, x, 1000);
    labelled(z, x, 997);
    long long h = 0;
    for (int i = 0; i < 1000; i++) {
        h = h * 31 + y[i] + 3 * z[i];
    }
    print_i64(h);
    return 0;
}
/* expect: 2532452572606638210 */
