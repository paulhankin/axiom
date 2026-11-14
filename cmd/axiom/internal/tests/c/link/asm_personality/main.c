// An assembly procedure's personality routine and language-specific data
// area (.cfi_personality, .cfi_lsda) in .eh_frame: the platform's unwinder
// raises an exception (sys_eh.c) through them and a frame of this
// compiler's C code with a variable length array (our_mid), calls the
// personality routine (sys_eh.c) of each with its LSDA, and lands in
// asm_catch's handler with its callee-saved register restored. asm_catch's
// pointers are pc-relative 4-byte ones (0x1b); asm_pass's routine is
// indirect (0x9b: the address of a word that holds it) and its LSDA an
// absolute 4-byte one (0x03).
// flags: --c-lib=system --c-std=gnu17
// cc-variants: -O2 | -O0
// expect: 42 7
// expect: 1
int printf(const char *, ...);
int asm_catch(void);
void asm_pass(int);
extern int sys_seen;
extern char asm_pass_ret[];  // (.cfi_label: after the call in asm_pass)

#ifdef __AXIOM_ASM_X86_64__
__asm__(".text\n"
        ".globl asm_catch\n.type asm_catch, @function\nasm_catch:\n"
        ".cfi_startproc\n.cfi_personality 0x1b, sys_personality\n.cfi_lsda 0x1b, .Lcatch_lsda\n"
        "  pushq %rbp\n.cfi_def_cfa_offset 16\n.cfi_offset %rbp, -16\n"
        "  movq %rsp, %rbp\n.cfi_def_cfa_register %rbp\n"
        "  pushq %rbx\n.cfi_offset %rbx, -24\n  subq $8, %rsp\n"
        "  movl $7, %ebx\n  movl $40, %edi\n  call our_mid\n  movl $1, %eax\n  jmp 2f\n"
        ".Lpad:\n  leal 35(%rbx), %eax\n"
        "2:\n  addq $8, %rsp\n  popq %rbx\n.cfi_restore %rbx\n  popq %rbp\n.cfi_def_cfa %rsp, 8\n  ret\n"
        ".cfi_endproc\n.size asm_catch, .-asm_catch\n"
        ".globl asm_pass\n.type asm_pass, @function\nasm_pass:\n"
        ".cfi_startproc\n.cfi_personality 0x9b, .Lpers_ref\n.cfi_lsda 0x3, .Lpass_lsda\n"
        "  pushq %rbp\n.cfi_def_cfa_offset 16\n.cfi_offset %rbp, -16\n"
        "  movq %rsp, %rbp\n.cfi_def_cfa_register %rbp\n"
        "  call sys_raise\n.globl asm_pass_ret\n.cfi_label asm_pass_ret\n  popq %rbp\n.cfi_def_cfa %rsp, 8\n  ret\n"
        ".cfi_endproc\n.size asm_pass, .-asm_pass\n"
        ".section .rodata\n.balign 8\n"
        ".Lcatch_lsda:\n  .long 0x4c534441\n  .long 0\n  .quad .Lpad\n"
        ".Lpass_lsda:\n  .long 0x50415353\n  .long 0\n"
        ".data\n.balign 8\n.Lpers_ref:\n  .quad sys_personality\n");
#else
__asm__(".text\n"
        ".globl asm_catch\n.type asm_catch, %function\n.p2align 2\nasm_catch:\n"
        ".cfi_startproc\n.cfi_personality 0x1b, sys_personality\n.cfi_lsda 0x1b, .Lcatch_lsda\n"
        "  stp x29, x30, [sp, #-32]!\n.cfi_def_cfa_offset 32\n.cfi_offset x29, -32\n.cfi_offset x30, -24\n"
        "  mov x29, sp\n.cfi_def_cfa_register x29\n"
        "  str x19, [sp, #16]\n.cfi_offset x19, -16\n"
        "  mov x19, #7\n  mov w0, #40\n  bl our_mid\n  mov w0, #1\n  b 2f\n"
        ".Lpad:\n  add w0, w19, #35\n"
        "2:\n  ldr x19, [sp, #16]\n.cfi_restore x19\n  ldp x29, x30, [sp], #32\n.cfi_def_cfa sp, 0\n"
        ".cfi_restore x29\n.cfi_restore x30\n  ret\n"
        ".cfi_endproc\n.size asm_catch, .-asm_catch\n"
        ".globl asm_pass\n.type asm_pass, %function\n.p2align 2\nasm_pass:\n"
        ".cfi_startproc\n.cfi_personality 0x9b, .Lpers_ref\n.cfi_lsda 0x3, .Lpass_lsda\n"
        "  stp x29, x30, [sp, #-16]!\n.cfi_def_cfa_offset 16\n.cfi_offset x29, -16\n.cfi_offset x30, -8\n"
        "  mov x29, sp\n.cfi_def_cfa_register x29\n"
        "  bl sys_raise\n.globl asm_pass_ret\n.cfi_label asm_pass_ret\n  ldp x29, x30, [sp], #16\n.cfi_def_cfa sp, 0\n.cfi_restore x29\n.cfi_restore x30\n  ret\n"
        ".cfi_endproc\n.size asm_pass, .-asm_pass\n"
        ".section .rodata\n.balign 8\n"
        ".Lcatch_lsda:\n  .long 0x4c534441\n  .long 0\n  .quad .Lpad\n"
        ".Lpass_lsda:\n  .long 0x50415353\n  .long 0\n"
        ".data\n.balign 8\n.Lpers_ref:\n  .quad sys_personality\n");
#endif

// (between the two: a frame of this compiler's with a variable length array)
void our_mid(int n) {
    volatile char vla[n];
    vla[0] = 3;
    asm_pass(vla[0]);
    vla[n - 1] = vla[0];
}

int main(void) {
    int r = asm_catch();
    printf("%d %d\n", r, sys_seen);
#ifdef __AXIOM_ASM_X86_64__
    int ret_at = 9;  // (push, mov, call)
#else
    int ret_at = 12;
#endif
    printf("%d\n", (int)(asm_pass_ret - (char *)asm_pass) == ret_at);
    return 0;
}
