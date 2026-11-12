/* flags: --c-std=gnu17 */
/* native-only */
/* expect-error: C3096 */
/* expect-error: C3021 */
/* Asm statements the GNU dialect's rules reject: constraints, operands,
   clobbers and template references. */
int g;
void f(int x) {
    int r;
    asm("" : "r"(r));                         /* an output without '=' or '+' */
    asm("" : "=r"(x + 1));                    /* an output that is not an lvalue */
    asm("%3" : "=r"(r) : "r"(x));             /* no operand 3 */
    asm("%[y]" : "=r"(r) : [x] "r"(x));       /* no operand named y */
    asm("" :: "i"(x));                        /* a constant that is not one */
    asm("" :: "5"(x));                        /* a matching constraint of no output */
    asm("" ::: "bogus");                      /* no such register */
    asm("" : [a] "=r"(r) : [a] "r"(x));       /* two operands named a */
#ifdef __AXIOM_ASM_X86_64__
    asm("" ::: "rsp");                        /* the stack pointer */
    asm("" :: "I"(100));                      /* out of range: 0 to 31 */
    asm("" : "=a"(r) :: "eax");               /* an operand in a clobbered register */
    register int v asm("frob") = 1;           /* no such register */
#else
    asm("" ::: "sp");
    asm("" :: "I"(5000));
    asm("" : "=r"(r) :: "x0");
    register int v asm("frob") = 1;
#endif
    asm("" :: "x"((char)x));                  /* a char in a vector register */
}
asm("nop" : : "r"(g));                        /* file-scope asm with operands */
int main(void) { return 0; }
