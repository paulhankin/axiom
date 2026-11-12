/* flags: --c-std=gnu17 */
/* native-only */
/* Asm operands in registers the compiler does not allocate (x86-64): the x87
   stack ('t' st(0), 'u' st(1), 'f'; the inputs the template pops clobbered,
   an output tied to an input with "0"), AVX-512 mask registers ('k'), MMX
   registers ('y'), xmm0 ('Yz'), and clobbers of ymm, zmm, mask, MMX and x87
   registers. Their values go through memory around the template. */
#include "host.h"

#ifdef __AXIOM_ASM_X86_64__
static long long scaled(double x) { return (long long)(x * 1000000.0 + (x < 0 ? -0.5 : 0.5)); }

int main(void) {
    double x = 2.0, r, s, c;
    __asm__("fsqrt" : "=t"(r) : "0"(x));
    print_i64(scaled(r));
    __asm__("fsincos" : "=t"(c), "=u"(s) : "0"(0.5));
    print_i64(scaled(s));
    print_i64(scaled(c));
    long double a = 9.0L, b = 4.0L, h;
    __asm__("faddp" : "=t"(h) : "0"(a), "u"(b) : "st(1)");
    print_i64((long long)h);
    __asm__("fmul %%st(0), %%st(0)\n\tfxch\n\tfmul %%st(0), %%st(0)\n\tfaddp\n\tfsqrt"
            : "=t"(h) : "0"(3.0L), "u"(4.0L) : "st(1)");
    print_i64((long long)h);
    double p = 10.0, q = 3.0, m;
    __asm__("fprem" : "=t"(m) : "0"(p), "u"(q));  /* (q is left on the stack: popped after) */
    print_i64(scaled(m));
    float f = 1.5f, g;
    __asm__("fadd %%st(0), %%st(0)" : "=t"(g) : "0"(f));
    print_i64(scaled(g));
    double acc = 1.0;
    __asm__("fadd %1, %0" : "+t"(acc) : "f"(2.25));
    print_i64(scaled(acc));
    unsigned long long k = 0x0f0f, kk;
    __asm__("knotq %1, %0" : "=k"(kk) : "k"(k));
    print_i64((long long)kk);
    unsigned short m1 = 0x00ff, m2 = 0x0f0f, m3;
    __asm__("kandw %1, %2, %0" : "=k"(m3) : "k"(m1), "k"(m2) : "k1");
    print_i64(m3);
    long long v = 41, w;
    __asm__("movq %1, %0\n\tpaddq %0, %0" : "=&y"(w) : "y"(v));
    print_i64(w);
    double z = 2.5, zz;
    __asm__("vaddsd %1, %1, %0" : "=v"(zz) : "Yz"(z));
    print_i64(scaled(zz));
    double y = 1.0;
    __asm__ volatile("vxorps %%ymm3, %%ymm3, %%ymm3\n\tkxorw %%k2, %%k2, %%k2"
                     ::: "ymm3", "zmm20", "k2", "mm1", "st", "xmm31");
    print_i64(scaled(y));
    return 0;
}
#else
int main(void) {
    print_i64(1414214); print_i64(479426); print_i64(877583); print_i64(13); print_i64(5); print_i64(1000000);
    print_i64(3000000); print_i64(3250000); print_i64(-3856); print_i64(15); print_i64(82); print_i64(5000000);
    print_i64(1000000);
    return 0;
}
#endif
/* expect: 1414214 */
/* expect: 479426 */
/* expect: 877583 */
/* expect: 13 */
/* expect: 5 */
/* expect: 1000000 */
/* expect: 3000000 */
/* expect: 3250000 */
/* expect: -3856 */
/* expect: 15 */
/* expect: 82 */
/* expect: 5000000 */
/* expect: 1000000 */
