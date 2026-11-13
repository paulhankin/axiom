/* flags: --c-std=gnu17 */
/* native-only */
/* The rules the optimizer keeps for asm statements (docs/c.md "Inline
   assembly"), each case one that a wrong assumption gets wrong: a "memory"
   clobber is a barrier (a store before it is done, a load after it is
   done again); a memory input reads its object (the store before it
   stays); a memory output writes it (a load after it reads again); an asm
   without volatile and without memory operands is a function of its inputs
   (merged, moved out of a loop, removed when unused: its results are still
   right), but is never run where it was not (one that would trap, in a
   branch not taken); an early-clobber output is in no input's register; a
   volatile asm without outputs stays; asm goto's outputs on both paths;
   a sibling call from a function whose asm does not use the stack. */
#include "host.h"

#ifdef __AXIOM_ASM_X86_64__
#define STORE5 "movl $5, (%0)"
#define LOAD "movl %1, %0"
#define STORE9 "movl $9, %0"
#define ADD3 "movl %1, %0\n\taddl $3, %0"
#define DIV "xorl %%edx, %%edx\n\tdivl %2"
#define SUM2 "movl $0, %0\n\taddl %1, %0\n\taddl %2, %0"
#define INC "incl %0"
#define SCRATCH "rcx"
#else
#define STORE5 "mov w9, #5\n\tstr w9, [%0]"
#define LOAD "ldr %w0, %1"
#define STORE9 "mov w9, #9\n\tstr w9, %0"
#define ADD3 "add %w0, %w1, #3"
#define SUM2 "mov %w0, #0\n\tadd %w0, %w0, %w1\n\tadd %w0, %w0, %w2"
#define INC "add %w0, %w0, #1"
#define SCRATCH "x9"
#endif

int g;
static int calls;

__attribute__((noinline)) static int add3(int x) {
    int r;
    __asm__(ADD3 : "=r"(r) : "r"(x));
    return r;
}

__attribute__((noinline)) static unsigned divide(unsigned a, unsigned d) {
    unsigned q = 0;
    if (d != 0) {
#ifdef __AXIOM_ASM_X86_64__
        unsigned rem;
        __asm__(DIV : "=a"(q), "=&d"(rem) : "r"(d), "0"(a));
#else
        __asm__("udiv %w0, %w1, %w2" : "=r"(q) : "r"(a), "r"(d));
#endif
    }
    return q;
}

__attribute__((noinline)) static int count_down(int n, int acc) {
    int k = n;
    __asm__ volatile("" : "+r"(k));
    if (k == 0) {
        return acc;
    }
    calls++;
    return count_down(n - 1, acc + 1);
}

int main(void) {
    // a "memory" clobber: the store through the asm is seen
    g = 1;
    __asm__ volatile(STORE5 : : "r"(&g) : "memory", SCRATCH);
    print_i64(g);
    // a memory input: the store before it is done
    g = 7;
    int r;
    __asm__(LOAD : "=r"(r) : "m"(g));
    print_i64(r);
    // a memory output: the load after it reads it again
    int before = g;
    __asm__(STORE9 : "=m"(g) : : SCRATCH);
    print_i64(before * 100 + g);
    // pure asm: merged, hoisted, removed; results right
    int s = 0;
    for (int i = 0; i < 10; i++) {
        s += add3(4) + add3(i);
    }
    print_i64(s);
    int unused;
    __asm__(ADD3 : "=r"(unused) : "r"(s));
    // never run where it was not: a division by zero not done
    print_i64(divide(100, 7) + divide(5, 0));
    // an early-clobber output in no input's register
    int a = 30, b = 12, sum;
    __asm__(SUM2 : "=&r"(sum) : "r"(a), "r"(b));
    print_i64(sum);
    // a volatile asm without outputs, and one with "+r" in a loop
    int n = 0;
    for (int i = 0; i < 5; i++) {
        __asm__ volatile(INC : "+r"(n));
    }
    print_i64(n);
    // asm goto with an output, on both paths
    for (int t = 0; t < 2; t++) {
        int o = 0;
#ifdef __AXIOM_ASM_X86_64__
        __asm__ goto("movl $11, %0\n\tcmpl $0, %1\n\tjne %l2" : "=&r"(o) : "r"(t) : "cc" : taken);
#else
        __asm__ goto("mov %w0, #11\n\tcbnz %w1, %l2" : "=&r"(o) : "r"(t) : : taken);
#endif
        print_i64(o);
        continue;
    taken:
        print_i64(o + 100);
    }
    // a function with an asm statement that makes a sibling call
    print_i64(count_down(1000, 0) + calls);
    return 0;
}
/* expect: 5 */
/* expect: 7 */
/* expect: 709 */
/* expect: 145 */
/* expect: 14 */
/* expect: 42 */
/* expect: 5 */
/* expect: 11 */
/* expect: 111 */
/* expect: 2000 */
