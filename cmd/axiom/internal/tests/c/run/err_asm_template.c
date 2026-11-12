/* flags: --c-std=gnu17 */
/* native-only */
/* expect-error: C3097 */
/* A template the assembler rejects (an unknown instruction, operands an
   instruction does not take): an error at the statement. */
void f(int x) {
    asm("frobnicate %0" :: "r"(x));
#ifdef __AXIOM_ASM_X86_64__
    asm("movl $1, $2");
#else
    asm("add x0, #1, #2");
#endif
}
int main(void) { return 0; }
