/* flags: --c-std=gnu17 */
/* native-only */
/* windows-only */
/* Windows unwind information of assembly: a file-scope asm function with
   .seh_proc/.seh_pushreg/.seh_stackalloc/.seh_setframe/.seh_endprologue/
   .seh_endproc (its .xdata record and .pdata entry), whose body moves rsp
   again, and an inline template that pushes and calls (its function gets
   rbp as its frame register): the system's stack walk
   (RtlCaptureStackBackTrace, which unwinds by .pdata) goes through both to
   main. */
#include "host.h"

#ifdef __AXIOM_ASM_X86_64__

unsigned short RtlCaptureStackBackTrace(unsigned long skip, unsigned long n, void **frames, unsigned long *hash);
extern char sf_start[], sf_end[];
static void *fr[32];
static int nfr;
void cb(void) { nfr = RtlCaptureStackBackTrace(0, 32, fr, 0); }
__asm__(".text\n.globl sf\n.globl sf_start\nsf:\nsf_start:\n.seh_proc sf\n"
        "  push %rbp\n.seh_pushreg %rbp\n  push %rbx\n.seh_pushreg %rbx\n"
        "  sub $0x28, %rsp\n.seh_stackalloc 0x28\n  lea 0x20(%rsp), %rbp\n.seh_setframe %rbp, 0x20\n.seh_endprologue\n"
        "  sub $0x40, %rsp\n  call *%rcx\n  lea 8(%rbp), %rsp\n  pop %rbx\n  pop %rbp\n  ret\n.seh_endproc\n"
        ".globl sf_end\nsf_end:\n");
void sf(void (*)(void));

__attribute__((noinline)) static void inl(void (*f)(void)) {
    __asm__ volatile("push %%rbx\nsub $0x38, %%rsp\ncall *%0\nadd $0x38, %%rsp\npop %%rbx"
                     : : "r"(f) : "memory", "rax", "rcx", "rdx", "r8", "r9", "r10", "r11");
}

static int found(void *lo, void *hi) {
    for (int i = 0; i < nfr; i++)
        if ((unsigned long long)fr[i] > (unsigned long long)lo && (unsigned long long)fr[i] < (unsigned long long)hi)
            return i;
    return 0;
}

int main(void) {
    sf(cb);
    print_i64(found(sf_start, sf_end));
    print_i64(found((void *)main, (char *)(void *)main + 400) > 1);
    inl(cb);
    print_i64(found((void *)inl, (char *)(void *)inl + 200));
    print_i64(found((void *)main, (char *)(void *)main + 400) > 1);
    return 0;
}
#else
int main(void) { for (int i = 0; i < 4; i++) print_i64(1); return 0; }
#endif
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
