# The first primes and a greeting: the assembler's own language, not
# preprocessed.
    .section .rodata
    .globl asm_primes
    .p2align 3
asm_primes:
    .irp n, 2, 3, 5, 7, 11, 13
    .quad \n
    .endr
    .globl asm_greeting
asm_greeting:
    .asciz "hello"
