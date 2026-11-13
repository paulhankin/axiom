/* flags: --c-std=gnu17 --c-asm-dialect=intel */
/* native-only */
/* --c-asm-dialect=intel: templates and file-scope asm in the Intel syntax;
   the `{att|intel}` alternatives give their second; operands are written
   as that syntax has them (registers without %, `DWORD PTR [rbp-8]` memory
   with the object's size, constants without $, `OFFSET FLAT:sym` for an
   address, `sym[rip]` for %a of one, %z nothing); the moves around
   operands in mask and x87 registers, and asm goto, in that syntax too. */
#include "host.h"

#ifdef __AXIOM_ASM_X86_64__
int g = 40;
static long long tab[4] = {1, 2, 3, 4};
static long double ld = 2.5L;
static unsigned char cb = 7;
static short sh = -3;

/* file-scope asm: Intel too */
#if __AXIOM_BITFIELD_LAYOUT__ == 2
__asm__(".text\n.globl add3\nadd3:\n  lea eax, [rcx + rdx*2]\n  add eax, DWORD PTR [rip + three]\n  ret\n"
        ".data\nthree: .long 3\n.text\n");
#else
__asm__(".text\n.globl add3\nadd3:\n  lea eax, [rdi + rsi*2]\n  add eax, DWORD PTR [rip + three]\n  ret\n"
        ".data\nthree: .long 3\n.text\n");
#endif
int add3(int, int);

int main(void) {
    int a = 5, b;
    __asm__("mov %0, %1\n\tadd %0, 2" : "=r"(b) : "r"(a));
    print_i64(b);
    __asm__("add %0, %1" : "+m"(g) : "ri"(2));
    print_i64(g);
    long long x;
    __asm__("mov %0, QWORD PTR [%1 + 8]" : "=r"(x) : "r"(tab));
    print_i64(x);
    __asm__("lea %0, %a1" : "=r"(x) : "i"(&tab[2]));
    print_i64(*(long long *)x);
    __asm__("mov %0, QWORD PTR %a1" : "=r"(x) : "i"(&tab[3]));
    print_i64(x);
    __asm__("{movl $7, %0|mov %0, 7}" : "=r"(a));
    print_i64(a);
    __asm__("movzx %0, %1" : "=r"(a) : "m"(cb));
    print_i64(a);
    __asm__("movsx %q0, %1" : "=r"(x) : "m"(sh));
    print_i64(x);
    __asm__("shl %0, %1" : "+r"(x) : "J"(4));
    print_i64(x);
    __asm__("neg%z0 %0" : "+m"(g));
    print_i64(g);
    double d;
    __asm__("fld %1\n\tfadd st, st\n\tfstp %0" : "=m"(d) : "m"(ld));
    print_i64((long)(d * 100));
    long double r;
    __asm__("fmul st, st" : "=t"(r) : "0"(ld));
    print_i64((long)(r * 100));
    unsigned short k = 0x1234, k2;
    __asm__("kmovw %0, %1" : "=k"(k2) : "k"(k));
    print_i64(k2);
    print_i64(add3(10, 20));
    __asm__ goto("cmp %0, 7\n\tje %l1" : : "r"(a) : "cc" : yes);
    print_i64(0);
yes:
    print_i64(1);
    return 0;
}
#else
int main(void) {
    long v[] = {7, 42, 2, 3, 4, 7, 7, -3, -48, -42, 500, 625, 4660, 53, 1};
    for (int i = 0; i < 15; i++) print_i64(v[i]);
    return 0;
}
#endif
/* expect: 7 */
/* expect: 42 */
/* expect: 2 */
/* expect: 3 */
/* expect: 4 */
/* expect: 7 */
/* expect: 7 */
/* expect: -3 */
/* expect: -48 */
/* expect: -42 */
/* expect: 500 */
/* expect: 625 */
/* expect: 4660 */
/* expect: 53 */
/* expect: 1 */
