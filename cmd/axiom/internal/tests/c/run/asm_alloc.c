/* flags: --c-std=gnu17 */
/* native-only */
/* Inline assembly and register allocation: values live across asm
   statements in every register class survive what the asm clobbers; an
   early-clobber output is never in an input's register; matching operands
   share their output's register; many operands at once. */
#include "host.h"

typedef long long i64;

/* Values in general and floating-point registers live across an asm that
   clobbers every allocatable register but one of each class. */
static i64 across(i64 a, i64 b, double x, double y) {
    i64 s = a * 3, t = b * 5, u = a + b, v = a - b;
    double p = x * 1.5, q = y * 2.5, r = x + y;
#ifdef __AXIOM_ASM_X86_64__
    asm volatile("" ::: "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "rbx", "r12", "r13", "r14",
                 "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                 "xmm12", "xmm13", "xmm14", "memory");
#else
    asm volatile("" ::: "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14",
                 "x15", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "v0", "v1", "v2", "v3",
                 "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",
                 "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "memory");
#endif
    return s + t + u + v + (i64)(p * 100) + (i64)(q * 100) + (i64)(r * 100);
}

/* Clobbered callee-saved registers are saved: the caller's values in them
   survive (a loop keeps several values live across the call). */
static void clobber_saved(void) {
#ifdef __AXIOM_ASM_X86_64__
    asm volatile("movq $-1, %%rbx\n\tmovq $-1, %%r12\n\tmovq $-1, %%r15" ::: "rbx", "r12", "r15");
#else
    asm volatile("mov x19, #-1\n\tmov x20, #-1\n\tmov x28, #-1\n\tfmov d8, #1.0\n\tfmov d15, #2.0" :::
                 "x19", "x20", "x28", "d8", "v15");
#endif
}

static i64 saved_users(int n) {
    i64 a = 1, b = 2, c = 3, d = 4, e = 5;
    double f = 0.5, h = 0.25;
    for (int i = 0; i < n; i++) {
        clobber_saved();
        a += b; b += c; c += d; d += e; e += a;
        f += h; h += f;
    }
    return a + b + c + d + e + (i64)(f * 1000) + (i64)(h * 1000);
}

/* An early-clobber output written before the inputs are read: correct only
   if it is in a register of its own. */
static i64 early(i64 a, i64 b, i64 c) {
    i64 r;
#ifdef __AXIOM_ASM_X86_64__
    asm("movq $0, %0\n\taddq %1, %0\n\taddq %2, %0\n\taddq %3, %0" : "=&r"(r) : "r"(a), "r"(b), "r"(c));
#else
    asm("mov %0, #0\n\tadd %0, %0, %1\n\tadd %0, %0, %2\n\tadd %0, %0, %3" : "=&r"(r) : "r"(a), "r"(b), "r"(c));
#endif
    return r;
}

/* Two outputs and inputs tied to them, of the same values. */
static i64 swap_sum(i64 a, i64 b) {
    i64 x = a, y = b;
#ifdef __AXIOM_ASM_X86_64__
    asm("xchgq %0, %1" : "=r"(x), "=r"(y) : "0"(x), "1"(y));
#else
    asm("mov x9, %0\n\tmov %0, %1\n\tmov %1, x9" : "+r"(x), "+r"(y) : : "x9");
#endif
    return x * 1000 + y;
}

/* Many inputs at once (the value of each used after, too). */
static i64 many(i64 a, i64 b, i64 c, i64 d, i64 e, i64 f, i64 g, i64 h) {
    i64 r;
#ifdef __AXIOM_ASM_X86_64__
    asm("leaq (%1,%2), %0\n\taddq %3, %0\n\taddq %4, %0\n\taddq %5, %0\n\taddq %6, %0\n\taddq %7, %0\n\taddq %8, %0"
        : "=&r"(r) : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h));
#else
    asm("add %0, %1, %2\n\tadd %0, %0, %3\n\tadd %0, %0, %4\n\tadd %0, %0, %5\n\tadd %0, %0, %6\n\tadd %0, %0, %7\n\t"
        "add %0, %0, %8" : "=&r"(r) : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h));
#endif
    return r * 100 + a + b + c + d + e + f + g + h;
}

/* The same value as two inputs, and an input that is also an output's
   initial value but read after the output is written. */
static i64 same_value(i64 a) {
    i64 r;
#ifdef __AXIOM_ASM_X86_64__
    asm("movq %1, %0\n\timulq %2, %0" : "=&r"(r) : "r"(a), "r"(a));
#else
    asm("mul %0, %1, %2" : "=&r"(r) : "r"(a), "r"(a));
#endif
    return r;
}

/* Operands in specific registers, with values live across in others. */
static i64 pinned(i64 a, i64 b) {
    i64 keep1 = a * 7, keep2 = b * 11;
    i64 r;
#ifdef __AXIOM_ASM_X86_64__
    asm("movq %%rcx, %%rax\n\tsubq %%rdx, %%rax" : "=a"(r) : "c"(a), "d"(b));
#else
    register i64 x3 asm("x3") = a;
    register i64 x4 asm("x4") = b;
    register i64 x5 asm("x5");
    asm("sub %0, %1, %2" : "=r"(x5) : "r"(x3), "r"(x4));
    r = x5;
#endif
    return r * 10000 + keep1 + keep2;
}

/* A loop whose body is an asm statement (outputs stored each time round). */
static i64 loop(int n) {
    i64 acc = 0;
    for (int i = 0; i < n; i++) {
#ifdef __AXIOM_ASM_X86_64__
        asm("addq %1, %0" : "+r"(acc) : "r"((i64)i));
#else
        asm("add %0, %0, %1" : "+r"(acc) : "r"((i64)i));
#endif
    }
    return acc;
}

int main(void) {
    print_i64(across(10, 3, 1.0, 2.0));
    print_i64(saved_users(6));
    print_i64(early(1, 20, 300));
    print_i64(swap_sum(3, 4));
    print_i64(many(1, 2, 3, 4, 5, 6, 7, 8));
    print_i64(same_value(12));
    print_i64(pinned(9, 4));
    print_i64(loop(100));
    return 0;
}
/* expect: 1015 */
/* expect: 212311 */
/* expect: 321 */
/* expect: 4003 */
/* expect: 3636 */
/* expect: 144 */
/* expect: 50107 */
/* expect: 4950 */
