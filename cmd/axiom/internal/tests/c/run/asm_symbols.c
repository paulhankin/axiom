/* flags: --c-std=gnu17 */
/* native-only */
/* Symbols and sections of inline assembly: a template's .globl label is a
   symbol of the object (C refers to it), its named labels are local symbols
   that other templates and C refer to by name, .pushsection/.popsection put
   bytes (with relocations back to the template's code) into another section;
   templates and file-scope asm name static functions and objects (the
   compiler's own symbols for them, which are kept though only asm uses them);
   a function whose asm defines symbols is not inlined (one copy of them). */
#include "host.h"

static int counter = 5;
static int twice(int x) { return 2 * x; }
static int helper_calls;
static void helper(void) { helper_calls++; }
extern char glabel[];
void tmpl_fn(void);
extern const char mytab[];

void f(void) {
#ifdef __AXIOM_ASM_X86_64__
    __asm__ volatile(".globl glabel\nglabel:\n nop\n"
                     ".pushsection .rodata.axiom_asm_sym, \"a\"\n.p2align 3\nmytab: .quad 1f, 42\n.popsection\n1: nop");
#else
    __asm__ volatile(".globl glabel\nglabel:\n nop\n"
                     ".pushsection .rodata.axiom_asm_sym, \"a\"\n.p2align 3\nmytab: .quad 1f, 42\n.popsection\n1: nop");
#endif
}

static const unsigned long long *tab(void) {
    const unsigned long long *r;
#ifdef __AXIOM_ASM_X86_64__
    __asm__("lea mytab(%%rip), %0" : "=r"(r));
#else
    __asm__("adrp %0, mytab\n\tadd %0, %0, :lo12:mytab" : "=r"(r));
#endif
    return r;
}

static int use_static(void) {
    int r;
#if defined(__AXIOM_ASM_X86_64__) && __AXIOM_BITFIELD_LAYOUT__ == 2
    __asm__ volatile("movl counter(%%rip), %%ecx\n\tsub $40, %%rsp\n\tcall twice\n\tadd $40, %%rsp\n\tmovl %%eax, %0"
                     : "=r"(r) : : "rax", "rcx", "rdx", "r8", "r9", "r10", "r11", "memory");
#elif defined(__AXIOM_ASM_X86_64__)
    __asm__ volatile("movl counter(%%rip), %%edi\n\tcall twice\n\tmovl %%eax, %0"
                     : "=r"(r) : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory");
#else
    __asm__ volatile("adrp x9, counter\n\tldr w0, [x9, :lo12:counter]\n\tbl twice\n\tmov %w0, w0"
                     : "=r"(r) : : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12",
                       "x13", "x14", "x15", "x16", "x17", "x30", "memory");
#endif
    return r;
}

#if defined(__AXIOM_ASM_X86_64__) && __AXIOM_BITFIELD_LAYOUT__ == 2
__asm__(".text\n.globl tmpl_fn\ntmpl_fn:\n  sub $40, %rsp\n  call helper\n  add $40, %rsp\n  ret\n");
#elif defined(__AXIOM_ASM_X86_64__)
__asm__(".text\n.globl tmpl_fn\ntmpl_fn:\n  push %rax\n  call helper\n  pop %rax\n  ret\n");
#else
__asm__(".text\n.p2align 2\n.globl tmpl_fn\ntmpl_fn:\n.seh_proc tmpl_fn\n  stp x29, x30, [sp, #-16]!\n.seh_save_fplr_x 16\n.seh_endprologue\n"
        "  bl helper\n.seh_startepilogue\n  ldp x29, x30, [sp], #16\n.seh_save_fplr_x 16\n.seh_endepilogue\n  ret\n.seh_endproc\n"
        ".size tmpl_fn, .-tmpl_fn\n");
#endif

int main(void) {
    f();
    f();
    const unsigned long long *t = tab();
    print_i64(t == (const unsigned long long *)(const void *)mytab);
    print_i64(t[0] > (unsigned long long)glabel && t[0] < (unsigned long long)glabel + 16);
    print_i64((long long)t[1]);
    print_i64(use_static());
    tmpl_fn();
    print_i64(helper_calls);
    return 0;
}
/* expect: 1 */
/* expect: 1 */
/* expect: 42 */
/* expect: 10 */
/* expect: 1 */
