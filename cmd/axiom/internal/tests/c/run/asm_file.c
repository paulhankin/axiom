/* flags: --c-std=gnu17 */
/* native-only */
/* linux-only */
/* File-scope asm: a function, data and a function that calls C, defined in
   assembly; a naked function (its body asm only: no prologue or epilogue). */
#include "host.h"

#ifdef __AXIOM_ASM_X86_64__
__asm__(".text\n"
        ".globl asm_add\n"
        ".type asm_add, @function\n"
        "asm_add:\n"
        "\tleal (%rdi,%rsi), %eax\n"
        "\tret\n"
        ".size asm_add, .-asm_add\n"
        ".data\n"
        ".globl asm_value\n"
        ".p2align 2\n"
        "asm_value: .long 1234\n"
        "asm_table: .quad asm_value, 0\n"
        ".globl asm_table_ptr\n"
        "asm_table_ptr: .quad asm_table\n"
        ".section .rodata\n"
        "asm_text: .asciz \"from asm\"\n"
        ".text\n"
        ".globl asm_string\n"
        "asm_string:\n"
        "\tleaq asm_text(%rip), %rax\n"
        "\tret\n");
#else
__asm__(".text\n"
        ".globl asm_add\n"
        ".type asm_add, %function\n"
        "asm_add:\n"
        "\tadd w0, w0, w1\n"
        "\tret\n"
        ".size asm_add, .-asm_add\n"
        ".data\n"
        ".globl asm_value\n"
        ".p2align 2\n"
        "asm_value: .word 1234\n"
        ".p2align 3\n"
        "asm_table: .quad asm_value, 0\n"
        ".globl asm_table_ptr\n"
        "asm_table_ptr: .quad asm_table\n"
        ".section .rodata\n"
        "asm_text: .asciz \"from asm\"\n"
        ".text\n"
        ".globl asm_string\n"
        "asm_string:\n"
        "\tadrp x0, asm_text\n"
        "\tadd x0, x0, :lo12:asm_text\n"
        "\tret\n");
#endif

/* A function in assembly that calls one in C (whose name it uses). */
int c_callback(int x) { return x * 10; }
#ifdef __AXIOM_ASM_X86_64__
__asm__(".text\n.globl asm_calls_c\nasm_calls_c:\n\tpushq %rbx\n\tmovl %edi, %ebx\n\tcall c_callback\n"
        "\taddl %ebx, %eax\n\tpopq %rbx\n\tret\n");
#else
__asm__(".text\n.globl asm_calls_c\nasm_calls_c:\n\tstp x29, x30, [sp, #-32]!\n\tmov x29, sp\n\tstr x19, [sp, #16]\n"
        "\tmov w19, w0\n\tbl c_callback\n\tadd w0, w0, w19\n\tldr x19, [sp, #16]\n\tldp x29, x30, [sp], #32\n\tret\n");
#endif

#ifdef __AXIOM_ASM_X86_64__
__attribute__((naked)) static int naked_sub(int a, int b) {
    __asm__("movl %edi, %eax\n\tsubl %esi, %eax");
    __asm__("ret");
}
#else
__attribute__((naked)) static int naked_sub(int a, int b) {
    __asm__("sub w0, w0, w1");
    __asm__("ret");
}
#endif

int asm_add(int, int);
extern int asm_value;
extern int **asm_table_ptr;
const char *asm_string(void);
int asm_calls_c(int);

int main(void) {
    print_i64(asm_add(40, 2));
    print_i64(asm_value);
    print_i64(*asm_table_ptr[0]);
    print_str(asm_string(), 8);
    print_i64(asm_calls_c(4));
    print_i64(naked_sub(50, 8));
    return 0;
}
/* expect: 42 */
/* expect: 1234 */
/* expect: 1234 */
/* expect: from asm */
/* expect: 44 */
/* expect: 42 */
