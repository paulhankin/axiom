/* Call frame information of assembly (run.mjs "debug: asm call frame
   information"): a file-scope asm procedure whose .cfi_* directives describe
   its frame (asmf), and an inline template that saves a register on the
   stack and calls (inl): its function's frame description says where the
   register is while it is saved. */
#ifdef __APPLE__
#define SYM(x) "_" #x
#define TYPE(x) ""
#else
#define SYM(x) #x
#define TYPE(x) ".type " #x ", @function\n"
#endif

void cb(void) {}

#ifdef __AXIOM_ASM_X86_64__
__asm__(".text\n.globl " SYM(asmf) "\n" TYPE(asmf) SYM(asmf) ":\n.cfi_startproc\n"
        "  push %rbx\n.cfi_adjust_cfa_offset 8\n.cfi_rel_offset %rbx, 0\n"
        "  push %r12\n.cfi_def_cfa_offset 24\n.cfi_offset %r12, -24\n"
        "  sub $8, %rsp\n.cfi_adjust_cfa_offset 8\n"
        "  call *%rdi\n"
        "  add $8, %rsp\n.cfi_adjust_cfa_offset -8\n"
        "  pop %r12\n.cfi_restore %r12\n.cfi_adjust_cfa_offset -8\n"
        "  pop %rbx\n.cfi_restore %rbx\n.cfi_adjust_cfa_offset -8\n"
        "  ret\n.cfi_endproc\n.size " SYM(asmf) ", .-" SYM(asmf) "\n");

void inl(void (*f0)(void)) {
    register void (*f)(void) __asm__("r12") = f0;
    __asm__ volatile("push %%rbx\n.cfi_adjust_cfa_offset 8\n.cfi_rel_offset %%rbx, 0\nmov $77, %%ebx\n"
                     "push %%rax\n.cfi_adjust_cfa_offset 8\ncall *%0\npop %%rax\n.cfi_adjust_cfa_offset -8\n"
                     "pop %%rbx\n.cfi_restore %%rbx\n.cfi_adjust_cfa_offset -8"
                     : : "r"(f) : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}
#else
__asm__(".text\n.p2align 2\n.globl " SYM(asmf) "\n" TYPE(asmf) SYM(asmf) ":\n.cfi_startproc\n"
        "  stp x29, x30, [sp, #-32]!\n.cfi_def_cfa_offset 32\n.cfi_offset x29, -32\n.cfi_offset x30, -24\n"
        "  mov x29, sp\n.cfi_def_cfa_register x29\n"
        "  str x19, [sp, #16]\n.cfi_rel_offset x19, 16\n"
        "  blr x0\n"
        "  ldr x19, [sp, #16]\n.cfi_restore x19\n"
        "  ldp x29, x30, [sp], #32\n.cfi_def_cfa sp, 0\n.cfi_restore x29\n.cfi_restore x30\n"
        "  ret\n.cfi_endproc\n.size " SYM(asmf) ", .-" SYM(asmf) "\n");

void inl(void (*f0)(void)) {
    register void (*f)(void) __asm__("x20") = f0;
    __asm__ volatile("str x19, [sp, #-16]!\n.cfi_adjust_cfa_offset 16\n.cfi_rel_offset x19, 0\nmov x19, #77\n"
                     "blr %0\n"
                     "ldr x19, [sp], #16\n.cfi_restore x19\n.cfi_adjust_cfa_offset -16"
                     : : "r"(f) : "memory", "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10",
                       "x11", "x12", "x13", "x14", "x15", "x16", "x17");
}
#endif

void asmf(void (*)(void));

int main(void) {
    asmf(cb);
    inl(cb);
    return 0;
}
