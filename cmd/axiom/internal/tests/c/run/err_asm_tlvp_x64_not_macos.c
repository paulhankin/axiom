/* flags: --c-std=gnu17 --target=x86_64 */
/* native-only */
/* expect-error: C3097 */
/* sym@TLVP is Mach-O's thread-local descriptor reference; on other targets
   it is an error. */
extern _Thread_local long tv;
long *f(void) {
    long *p;
    __asm__("movq tv@TLVP(%%rip), %%rdi\n\tcall *(%%rdi)\n\tmovq %%rax, %0" : "=r"(p) : : "rax", "rdi");
    return p;
}
int main(void) { return f() == 0; }
