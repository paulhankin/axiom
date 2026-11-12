/* flags: --c-std=gnu17 */
/* native-only */
/* Inline assembly: each kind of operand (registers of a class, specific
   registers, memory, constants, matching and read-write operands, named
   operands, explicit register variables), with the template's modifiers. */
#include "host.h"

typedef long long i64;
static int g = 7;
static i64 table[4] = {10, 20, 30, 40};

#ifdef __AXIOM_ASM_X86_64__
static int add_r(int a, int b) {
    int r;
    asm("movl %1, %0\n\taddl %2, %0" : "=&r"(r) : "r"(a), "r"(b));
    return r;
}
static i64 shl_plus(i64 x) {
    asm("shlq $3, %0" : "+r"(x));
    return x;
}
static int imm(int x) {
    asm("addl %1, %0\n\taddl $%c2, %0" : "+rm"(x) : "i"(100), "n"(7));
    return x;
}
static int in_mem(int *p) {
    int r;
    asm("movl %1, %0" : "=r"(r) : "m"(*p));
    return r;
}
static void out_mem(i64 *p, i64 v) {
    asm("movq %1, %0" : "=m"(*p) : "r"(v));
}
static i64 g_mem(int k) {
    i64 r;
    asm("movq %1, %0" : "=r"(r) : "g"(table[k]));
    return r;
}
static int match(int a) {
    int r;
    asm("incl %0" : "=r"(r) : "0"(a));
    return r;
}
static int named(int a, int b) {
    int r;
    asm("movl %[x], %[out]\n\timull %[y], %[out]" : [out] "=&r"(r) : [x] "r"(a), [y] "rm"(b));
    return r;
}
static unsigned fixed(unsigned a, unsigned b) {
    unsigned q, rem;
    asm("divl %4" : "=a"(q), "=d"(rem) : "a"(a), "d"(0u), "r"(b));
    return q * 1000 + rem;
}
static int widths(int x) {
    int r = x;
    asm("addb $1, %b0\n\taddw $256, %w0" : "+q"(r));
    return r;
}
static int high(int x) {
    int r;
    asm("movl %1, %0\n\tmovb %h1, %b0" : "=&Q"(r) : "Q"(x));
    return r;
}
static double fadd(double a, double b) {
    asm("addsd %1, %0" : "+x"(a) : "x"(b));
    return a;
}
static float fmul_gpr(float a) {
    float r;
    /* a float in a general register: its bits */
    asm("movl %1, %0" : "=r"(r) : "r"(a));
    return r * 2.0f;
}
static int regvar(int x) {
    register int v asm("ebx") = x;
    asm("addl $5, %0" : "+r"(v));
    return v;
}
static int glob(void) {
    int r;
    asm("movl %1, %0" : "=r"(r) : "m"(g));
    return r;
}
static i64 addr_imm(void) {
    i64 p;
    asm("leaq %1, %0" : "=r"(p) : "m"(table[2]));
    return *(i64 *)p;
}
static int cc_flags(int a, int b) {
    int r = 0;
    asm("cmpl %2, %1\n\tsetl %b0" : "+q"(r) : "r"(a), "r"(b) : "cc");
    return r;
}
static int pct(int a) {
    asm("addl %%eax, %%eax\n\taddl %0, %%eax" : : "r"(a) : "eax");
    asm("negl %0 # a comment %% here" : "+r"(a));
    return a;
}
#else
static int add_r(int a, int b) {
    int r;
    asm("add %w0, %w1, %w2" : "=&r"(r) : "r"(a), "r"(b));
    return r;
}
static i64 shl_plus(i64 x) {
    asm("lsl %0, %0, #3" : "+r"(x));
    return x;
}
static int imm(int x) {
    asm("add %w0, %w0, %1\n\tadd %w0, %w0, #%c2" : "+r"(x) : "i"(100), "n"(7));
    return x;
}
static int in_mem(int *p) {
    int r;
    asm("ldr %w0, %1" : "=r"(r) : "m"(*p));
    return r;
}
static void out_mem(i64 *p, i64 v) {
    asm("str %1, %0" : "=m"(*p) : "r"(v));
}
static i64 g_mem(int k) {
    i64 r;
    asm("mov %0, %1" : "=r"(r) : "g"(table[k]));
    return r;
}
static int match(int a) {
    int r;
    asm("add %w0, %w0, #1" : "=r"(r) : "0"(a));
    return r;
}
static int named(int a, int b) {
    int r;
    asm("mul %w[out], %w[x], %w[y]" : [out] "=&r"(r) : [x] "r"(a), [y] "r"(b));
    return r;
}
static unsigned fixed(unsigned a, unsigned b) {
    register unsigned x0 asm("x0") = a;
    register unsigned x1 asm("x1") = b;
    register unsigned x2 asm("x2");
    asm("udiv %w2, %w0, %w1\n\tmsub %w1, %w2, %w1, %w0\n\tmov %w0, %w2" : "+r"(x0), "+r"(x1), "=&r"(x2));
    return x0 * 1000 + x1;
}
static int widths(int x) {
    int r = x;
    asm("add %w0, %w0, #1\n\tadd %w0, %w0, #256" : "+r"(r));
    return r;
}
static int high(int x) {
    int r;
    asm("mov %w0, %w1\n\tbfxil %w0, %w1, #8, #8" : "=&r"(r) : "r"(x));
    return r;
}
static double fadd(double a, double b) {
    asm("fadd %d0, %d0, %d1" : "+w"(a) : "w"(b));
    return a;
}
static float fmul_gpr(float a) {
    float r;
    asm("mov %w0, %w1" : "=r"(r) : "r"(a));
    return r * 2.0f;
}
static int regvar(int x) {
    register int v asm("w19") = x;
    asm("add %w0, %w0, #5" : "+r"(v));
    return v;
}
static int glob(void) {
    int r;
    asm("ldr %w0, %1" : "=r"(r) : "m"(g));
    return r;
}
static i64 addr_imm(void) {
    i64 v;
    asm("ldr %0, %1" : "=r"(v) : "Q"(table[2]));
    return v;
}
static int cc_flags(int a, int b) {
    int r;
    asm("cmp %w1, %w2\n\tcset %w0, lt" : "=r"(r) : "r"(a), "r"(b) : "cc");
    return r;
}
static int pct(int a) {
    asm("add w9, w9, %w0" : : "r"(a) : "x9");
    asm("neg %w0, %w0 // a comment" : "+r"(a));
    return a;
}
#endif

int main(void) {
    i64 m = 0;
    print_i64(add_r(40, 2));
    print_i64(shl_plus(5));
    print_i64(imm(1));
    print_i64(in_mem(&g));
    out_mem(&m, 99);
    print_i64(m);
    print_i64(g_mem(3));
    print_i64(match(41));
    print_i64(named(6, 7));
    print_i64(fixed(100, 7));
    print_i64(widths(0x10fe));
    print_i64(high(0x1234));
    print_f64(fadd(1.5, 2.25));
    print_f64(fmul_gpr(1.25f));
    print_i64(regvar(37));
    print_i64(glob());
    print_i64(addr_imm());
    print_i64(cc_flags(1, 2) * 10 + cc_flags(3, 2));
    print_i64(pct(5));
    return 0;
}
/* expect: 42 */
/* expect: 40 */
/* expect: 108 */
/* expect: 7 */
/* expect: 99 */
/* expect: 40 */
/* expect: 42 */
/* expect: 42 */
/* expect: 14002 */
/* expect: 4607 */
/* expect: 4626 */
/* expect: 3.75 */
/* expect: 2.5 */
/* expect: 42 */
/* expect: 7 */
/* expect: 30 */
/* expect: 10 */
/* expect: -5 */
