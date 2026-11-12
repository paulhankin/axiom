/* flags: --c-std=gnu17 */
/* native-only */
/* asm goto: a template that may jump to its labels (%l[name], %lN), with
   outputs (stored on every path), in a loop, and leaving a block. */
#include "host.h"

static int classify(int x) {
#ifdef __AXIOM_ASM_X86_64__
    asm goto("cmpl $0, %0\n\tjl %l[neg]\n\tje %l2" : : "r"(x) : "cc" : neg, zero);
#else
    asm goto("cmp %w0, #0\n\tb.lt %l[neg]\n\tb.eq %l2" : : "r"(x) : "cc" : neg, zero);
#endif
    return 1;
neg:
    return -1;
zero:
    return 0;
}

static int count_until(int limit) {
    int n = 0;
    for (;;) {
        n++;
#ifdef __AXIOM_ASM_X86_64__
        asm goto("cmpl %1, %0\n\tjge %l[done]" : : "r"(n), "r"(limit) : "cc" : done);
#else
        asm goto("cmp %w0, %w1\n\tb.ge %l[done]" : : "r"(n), "r"(limit) : "cc" : done);
#endif
    }
done:
    return n;
}

static int with_output(int x) {
    int y;
#ifdef __AXIOM_ASM_X86_64__
    asm goto("movl %1, %0\n\taddl %0, %0\n\ttestl %1, %1\n\tjz %l[z]" : "=&r"(y) : "r"(x) : "cc" : z);
#else
    asm goto("add %w0, %w1, %w1\n\tcbz %w1, %l[z]" : "=&r"(y) : "r"(x) : : z);
#endif
    return y;
z:
    return y + 1000;
}

int main(void) {
    print_i64(classify(-5));
    print_i64(classify(0));
    print_i64(classify(9));
    print_i64(count_until(17));
    print_i64(with_output(21));
    print_i64(with_output(0));
    {
        int k = 0;
        for (int i = 0; i < 10; i++) {
#ifdef __AXIOM_ASM_X86_64__
            asm goto("testl $1, %0\n\tjnz %l[odd]" : : "r"(i) : "cc" : odd);
#else
            asm goto("tbnz %w0, #0, %l[odd]" : : "r"(i) : : odd);
#endif
            k += 100;
            continue;
        odd:
            k += 1;
        }
        print_i64(k);
    }
    return 0;
}
/* expect: -1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 17 */
/* expect: 42 */
/* expect: 1000 */
/* expect: 505 */
