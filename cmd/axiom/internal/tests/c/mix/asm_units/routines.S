/* long long asm_sum(const long long *p, long long n): the sum of p[0..n);
   long long asm_len(const char *s): the C library's strlen (linked for it). */
#ifndef __ASSEMBLER__
#error "not preprocessed as assembly"
#endif
#if defined(__AXIOM_ASM_X86_64__)
#if __AXIOM_BITFIELD_LAYOUT__ == 2
#define A0 %rcx
#define A1 %rdx
#else
#define A0 %rdi
#define A1 %rsi
#endif
    .text
    .globl asm_sum
asm_sum:
    xorl %eax, %eax
    testq A1, A1
    jle 2f
1:  addq (A0), %rax
    addq $8, A0
    decq A1
    jnz 1b
2:  ret
/* long long asm_len(const char *s): the C library's strlen (linked for it) */
    .globl asm_len
asm_len:
    jmp strlen
#else
    .text
    .p2align 2
    .globl asm_sum
asm_sum:
    mov x2, #0
    cbz x1, 2f
1:  ldr x3, [x0], #8
    add x2, x2, x3
    subs x1, x1, #1
    b.ne 1b
2:  mov x0, x2
    ret
    .globl asm_len
asm_len:
    b strlen
#endif
