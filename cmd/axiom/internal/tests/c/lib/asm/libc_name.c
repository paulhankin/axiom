/* flags: --c-std=gnu17 */
/* File-scope asm that defines a name the C library defines too (raise)
   is the program's definition, as a C unit's would be: it keeps the
   library's out (docs/c.md "Assembly sources"), on every target. */
#include <signal.h>
#include <stdio.h>

#if defined(__AXIOM_ASM_WASM__)
__asm__("(func $raise (param i32) (result i32) (i32.add (local.get 0) (i32.const 40)))");
#elif defined(__AXIOM_ASM_X86_64__) && (__AXIOM_BITFIELD_LAYOUT__ == 2)
/* (Windows x64: the first argument in rcx) */
__asm__(".globl raise\nraise:\n leal 40(%rcx), %eax\n ret\n");
#elif defined(__AXIOM_ASM_X86_64__)
__asm__(".globl raise\nraise:\n leal 40(%rdi), %eax\n ret\n");
#else
__asm__(".globl raise\nraise:\n add w0, w0, #40\n ret\n");
#endif

int main(void) {
    signal(SIGINT, SIG_DFL);  /* (the library's unit of raise, linked for signal) */
    printf("%d\n", raise(2));
    return 0;
}
/* expect: 42 */
