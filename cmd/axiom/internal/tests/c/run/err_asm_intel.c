/* flags: --c-std=gnu17 --c-asm-dialect=intel */
/* native-only */
/* expect-error: C3097 */
/* The Intel syntax's errors: a size keyword without an operand, a
   register subtracted in an address, an unknown syntax. */
void f(void) {
#ifdef __AXIOM_ASM_X86_64__
    asm("mov eax, DWORD PTR");
    asm("mov eax, [rbx - rcx]");
    asm(".intel_syntax odd");
#else
    asm("frobnicate");
#endif
}
int main(void) { return 0; }
