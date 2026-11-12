/* flags: --c-std=gnu17 */
/* native-only */
/* The x86-64 assembler against a table: 3486 instruction lines (the general-purpose
   instructions in their operand forms, BMI, SSE through SSE4.1), each with the bytes its
   encoding has (from the architecture manual's tables), assembled here as
   file-scope asm in a section of their own; the bytes are compared. */
#include "host.h"

#ifdef __AXIOM_ASM_X86_64__
__asm__(".section .rodata.axiom_asm_table, \"a\"\n"
        ".globl axiom_asm_table\n"
        "axiom_asm_table:\n"
        "0:\n1:\n2:\n3:\n4:\n"
        "add %spl, %dl\n"
        "addb %r8b, -129(%r15)\n"
        "add (%rbp), %r15b\n"
        "add $0, %r12b\n"
        "addb $-128, 16(%r8d)\n"
        "add $1, %al\n"
        "add %bx, %r10w\n"
        "addw %ax, (%rax)\n"
        "add (%rsp), %si\n"
        "add $128, %ax\n"
        "addw $32767, -8(%r14,%r8,4)\n"
        "add $1, %ax\n"
        "add %edi, %r14d\n"
        "addl %r9d, (%r13)\n"
        "add (%r12), %r9d\n"
        "add $0, %r10d\n"
        "addl $-1, (%r12)\n"
        "add $-1, %eax\n"
        "add %rsi, %r9\n"
        "addq %r9, 16(%ecx)\n"
        "add (%r13), %r13\n"
        "add $-1, %rbp\n"
        "addq $127, (%rsp)\n"
        "add $305419896, %rax\n"
        "or %bl, %bpl\n"
        "orb %r12b, %fs:0x28\n"
        "or (%r9), %r12b\n"
        "or $1, %bpl\n"
        "orb $255, -8(%rax,%r13,2)\n"
        "or $-128, %al\n"
        "or %r11w, %r11w\n"
        "orw %r14w, 0x1000(,%rdi,2)\n"
        "or (%r12), %r15w\n"
        "or $127, %si\n"
        "orw $32767, 0x1234\n"
        "or $128, %ax\n"
        "or %r11d, %eax\n"
        "orl %r10d, (%rbp)\n"
        "or (%rbp,%r14,1), %r8d\n"
        "or $0, %edx\n"
        "orl $0, (%r14)\n"
        "or $127, %eax\n"
        "or %rdi, %r8\n"
        "orq %rbx, 16(%edx)\n"
        "or (%rbp,%rbp,2), %r8\n"
        "or $-1, %r9\n"
        "orq $128, %fs:0x0\n"
        "or $127, %rax\n"
        "adc %r12b, %r10b\n"
        "adcb %r13b, 1024(%r8,%rcx,2)\n"
        "adc 0(%edi), %al\n"
        "adc $-128, %spl\n"
        "adcb $0, (%r14,%r12,8)\n"
        "adc $255, %al\n"
        "adc %di, %r14w\n"
        "adcw %di, (%r12)\n"
        "adc %gs:0x0, %si\n"
        "adc $0, %r9w\n"
        "adcw $0, 127(%r9)\n"
        "adc $-1, %ax\n"
        "adc %ebp, %r13d\n"
        "adcl %r8d, (%rax,%r9,1)\n"
        "adc 16(%ebp), %ecx\n"
        "adc $128, %esi\n"
        "adcl $127, 128(%rsi)\n"
        "adc $305419896, %eax\n"
        "adc %rsi, %r15\n"
        "adcq %rbx, 0x1234\n"
        "adc (%r10), %r12\n"
        "adc $127, %rax\n"
        "adcq $1, -8(%r10,%r13,4)\n"
        "adc $128, %rax\n"
        "sbb %sil, %r8b\n"
        "sbbb %bl, 0x7fffffff\n"
        "sbb (%r12), %dil\n"
        "sbb $0, %cl\n"
        "sbbb $0, (%rbp,%rdx,2)\n"
        "sbb $127, %al\n"
        "sbb %r10w, %r8w\n"
        "sbbw %r11w, %gs:0x0\n"
        "sbb 0(%r10d), %cx\n"
        "sbb $128, %dx\n"
        "sbbw $128, (%rsp,%rsi,1)\n"
        "sbb $32767, %ax\n"
        "sbb %r12d, %edx\n"
        "sbbl %edi, 16(%r9d)\n"
        "sbb 16(%ebx), %ecx\n"
        "sbb $127, %eax\n"
        "sbbl $305419896, (%rdx)\n"
        "sbb $0, %eax\n"
        "sbb %rcx, %rsi\n"
        "sbbq %rdi, 0(%r14d)\n"
        "sbb (%rdi,%rdx,1), %r13\n"
        "sbb $128, %r9\n"
        "sbbq $305419896, 0x0(,%rsi,2)\n"
        "sbb $-1, %rax\n"
        "and %r10b, %cl\n"
        "andb %al, (%r9)\n"
        "and 16(%r10d), %r12b\n"
        "and $0, %dl\n"
        "andb $127, 16(%esi)\n"
        "and $255, %al\n"
        "and %r15w, %r11w\n"
        "andw %r8w, (%rsi,%rbp,2)\n"
        "and 1024(%r11,%rcx,1), %r14w\n"
        "and $0, %r10w\n"
        "andw $1, 0x1234\n"
        "and $1, %ax\n"
        "and %r10d, %r9d\n"
        "andl %edi, %fs:0x0\n"
        "and -8(%rax,%r13,8), %edx\n"
        "and $127, %edx\n"
        "andl $-1, 1(%rax)\n"
        "and $127, %eax\n"
        "and %r11, %r15\n"
        "andq %r15, (%rbx,%r9,4)\n"
        "and 8(%rbp), %rsp\n"
        "and $-2147483648, %r10\n"
        "andq $127, -128(%r9)\n"
        "and $1, %rax\n"
        "sub %cl, %r10b\n"
        "subb %sil, (%r9,%rdi,2)\n"
        "sub (%rdi), %dl\n"
        "sub $-128, %r13b\n"
        "subb $255, 0x1000(,%r14,8)\n"
        "sub $0, %al\n"
        "sub %r12w, %r10w\n"
        "subw %bp, 0x1000(,%rax,8)\n"
        "sub 16(%esp), %sp\n"
        "sub $1, %r8w\n"
        "subw $127, 0x7fffffff\n"
        "sub $0, %ax\n"
        "sub %edi, %r15d\n"
        "subl %eax, (%r10,%r9,8)\n"
        "sub 100000(%rdi,%rsi,8), %edi\n"
        "sub $-1, %r13d\n"
        "subl $127, (%rsp)\n"
        "sub $0, %eax\n"
        "sub %r11, %rbp\n"
        "subq %rsi, 0x10(,%r12,4)\n"
        "sub (%r14,%r10,1), %rbx\n"
        "sub $305419896, %r12\n"
        "subq $1, (%r8,%rdi,2)\n"
        "sub $305419896, %rax\n"
        "xor %cl, %r15b\n"
        "xorb %r12b, %fs:0x1000\n"
        "xor (%rdx), %r8b\n"
        "xor $0, %r8b\n"
        "xorb $0, (%rdx,%r8,2)\n"
        "xor $-128, %al\n"
        "xor %r13w, %r12w\n"
        "xorw %bp, %gs:0x0\n"
        "xor 4660(%r13), %r13w\n"
        "xor $0, %r9w\n"
        "xorw $127, 4(%r12,%r12,2)\n"
        "xor $32767, %ax\n"
        "xor %r14d, %eax\n"
        "xorl %eax, 16(%esi)\n"
        "xor (%r9,%rdx,2), %r8d\n"
        "xor $127, %r8d\n"
        "xorl $-1, (%rbp)\n"
        "xor $128, %eax\n"
        "xor %rbx, %rsi\n"
        "xorq %r12, 4(%r9,%r13,1)\n"
        "xor 127(%rax), %rsp\n"
        "xor $0, %r11\n"
        "xorq $305419896, 0x1000(,%r9,4)\n"
        "xor $-2147483648, %rax\n"
        "cmp %r10b, %al\n"
        "cmpb %bl, (%rbp)\n"
        "cmp 0x7fffffff, %r15b\n"
        "cmp $0, %r12b\n"
        "cmpb $-128, -8(%rax,%rbp,8)\n"
        "cmp $255, %al\n"
        "cmp %r13w, %r9w\n"
        "cmpw %bp, (%r13)\n"
        "cmp 0x1234, %dx\n"
        "cmp $128, %di\n"
        "cmpw $-1, 0x1000(,%rdi,2)\n"
        "cmp $-1, %ax\n"
        "cmp %r12d, %r8d\n"
        "cmpl %ebp, 127(%rax)\n"
        "cmp 0x1234, %r8d\n"
        "cmp $128, %ebp\n"
        "cmpl $128, (%r12)\n"
        "cmp $-1, %eax\n"
        "cmp %r13, %rdx\n"
        "cmpq %r11, 8(%r14)\n"
        "cmp (%rsp), %r8\n"
        "cmp $305419896, %r9\n"
        "cmpq $1, (%r12)\n"
        "cmp $127, %rax\n"
        "test %dl, %dl\n"
        "test %r11b, (%rbp)\n"
        "test $255, %r8b\n"
        "testb $127, 16(%ebp)\n"
        "mov %r15b, %r8b\n"
        "mov %r10b, -8(%r8,%r10,1)\n"
        "mov 16(%edi), %r8b\n"
        "mov $1, %dl\n"
        "movb $1, 0(%r8d)\n"
        "not %r14b\n"
        "notb (%r12)\n"
        "neg %r14b\n"
        "negb %gs:0x0\n"
        "mul %bl\n"
        "mulb 4(%r9,%rcx,2)\n"
        "imul %r12b\n"
        "imulb %fs:0x0\n"
        "div %cl\n"
        "divb (%rax)\n"
        "idiv %sil\n"
        "idivb (%r15)\n"
        "inc %r14b\n"
        "incb %fs:0x0\n"
        "dec %dil\n"
        "decb 0x1234\n"
        "rol %r12b\n"
        "rol $1, %dil\n"
        "rol %cl, %dil\n"
        "rolb $2, (%rbp)\n"
        "ror %r8b\n"
        "ror $3, %r15b\n"
        "ror %cl, %bl\n"
        "rorb $1, 1(%rcx)\n"
        "rcl %r15b\n"
        "rcl $3, %r12b\n"
        "rcl %cl, %r9b\n"
        "rclb $1, 0x7fffffff\n"
        "rcr %spl\n"
        "rcr $1, %al\n"
        "rcr %cl, %r12b\n"
        "rcrb $1, (%rsp)\n"
        "shl %spl\n"
        "shl $1, %r14b\n"
        "shl %cl, %r9b\n"
        "shlb $1, (%rcx)\n"
        "shr %spl\n"
        "shr $1, %r8b\n"
        "shr %cl, %bl\n"
        "shrb $2, -129(%rsi)\n"
        "sar %spl\n"
        "sar $7, %r8b\n"
        "sar %cl, %sil\n"
        "sarb $5, (%r13)\n"
        "sal %r8b\n"
        "sal $7, %dil\n"
        "sal %cl, %dil\n"
        "salb $1, 16(%ecx)\n"
        "test %r11w, %r13w\n"
        "test %si, (%r12)\n"
        "test $-1, %r8w\n"
        "testw $-1, 0(%ebx)\n"
        "mov %sp, %cx\n"
        "mov %si, 0x7fffffff\n"
        "mov 127(%r15), %bx\n"
        "mov $127, %cx\n"
        "movw $1, (%r12)\n"
        "not %r12w\n"
        "notw (%r13)\n"
        "neg %dx\n"
        "negw 0x10(,%rcx,1)\n"
        "mul %r12w\n"
        "mulw (%r8)\n"
        "imul %sp\n"
        "imulw 0x0(,%r13,4)\n"
        "div %bx\n"
        "divw 0x7fffffff\n"
        "idiv %si\n"
        "idivw %gs:0x1000\n"
        "inc %r15w\n"
        "incw -129(%rsp)\n"
        "dec %ax\n"
        "decw 0x10(,%rbx,8)\n"
        "rol %r10w\n"
        "rol $1, %r13w\n"
        "rol %cl, %r11w\n"
        "rolw $1, 16(%r10d)\n"
        "ror %r13w\n"
        "ror $3, %r10w\n"
        "ror %cl, %r11w\n"
        "rorw $2, %fs:0x0\n"
        "rcl %r10w\n"
        "rcl $7, %r10w\n"
        "rcl %cl, %r10w\n"
        "rclw $5, -129(%r14)\n"
        "rcr %r14w\n"
        "rcr $3, %r12w\n"
        "rcr %cl, %dx\n"
        "rcrw $5, (%rsp)\n"
        "shl %r15w\n"
        "shl $7, %r8w\n"
        "shl %cl, %di\n"
        "shlw $5, 16(%r12d)\n"
        "shr %r9w\n"
        "shr $3, %r10w\n"
        "shr %cl, %bp\n"
        "shrw $1, (%r8,%r11,2)\n"
        "sar %sp\n"
        "sar $1, %bp\n"
        "sar %cl, %r13w\n"
        "sarw $5, 0(%r8d)\n"
        "sal %bx\n"
        "sal $1, %r8w\n"
        "sal %cl, %dx\n"
        "salw $5, 0(%esi)\n"
        "test %ebp, %r13d\n"
        "test %eax, 16(%r9d)\n"
        "test $1, %esi\n"
        "testl $305419896, (%rbp)\n"
        "mov %r11d, %esi\n"
        "mov %r15d, -128(%r8)\n"
        "mov (%r12), %r15d\n"
        "mov $0, %r12d\n"
        "movl $305419896, (%rsp)\n"
        "not %r14d\n"
        "notl (%rsi)\n"
        "neg %eax\n"
        "negl (%rsp)\n"
        "mul %r10d\n"
        "mull (%rsp)\n"
        "imul %r13d\n"
        "imull 16(%esp)\n"
        "div %r14d\n"
        "divl 0(%r8d)\n"
        "idiv %eax\n"
        "idivl 0x1234\n"
        "inc %r12d\n"
        "incl 0x0(,%r15,1)\n"
        "dec %eax\n"
        "decl 0x1234\n"
        "rol %r11d\n"
        "rol $7, %r15d\n"
        "rol %cl, %r10d\n"
        "roll $2, (%rbp)\n"
        "ror %r13d\n"
        "ror $1, %eax\n"
        "ror %cl, %ebp\n"
        "rorl $2, %gs:0x28\n"
        "rcl %r8d\n"
        "rcl $7, %r9d\n"
        "rcl %cl, %r13d\n"
        "rcll $5, 0x10(,%rsi,2)\n"
        "rcr %r15d\n"
        "rcr $3, %r13d\n"
        "rcr %cl, %edx\n"
        "rcrl $1, (%rsi,%rdx,2)\n"
        "shl %eax\n"
        "shl $1, %r8d\n"
        "shl %cl, %esp\n"
        "shll $2, 8(%r12)\n"
        "shr %eax\n"
        "shr $1, %r13d\n"
        "shr %cl, %ecx\n"
        "shrl $5, 4(%r13,%rsi,1)\n"
        "sar %r13d\n"
        "sar $7, %ebx\n"
        "sar %cl, %r8d\n"
        "sarl $5, 0x1000(,%r8,1)\n"
        "sal %esi\n"
        "sal $7, %edx\n"
        "sal %cl, %r12d\n"
        "sall $1, (%r13)\n"
        "test %r12, %rbx\n"
        "test %r15, -305419896(%rsp)\n"
        "test $-1, %rsi\n"
        "testq $1, (%r12)\n"
        "mov %r9, %r15\n"
        "mov %rsi, 16(%ebx)\n"
        "mov (%r11), %r8\n"
        "mov $0, %r14\n"
        "movq $127, 127(%rdi)\n"
        "not %r8\n"
        "notq -8(%r13,%rdx,4)\n"
        "neg %rsi\n"
        "negq 0x7fffffff\n"
        "mul %rsp\n"
        "mulq 0x1234\n"
        "imul %r9\n"
        "imulq 0x10(,%rbx,4)\n"
        "div %r15\n"
        "divq -8(%r10,%rdx,8)\n"
        "idiv %rsp\n"
        "idivq (%r10)\n"
        "inc %rsp\n"
        "incq 100000(%r10,%r10,8)\n"
        "dec %r10\n"
        "decq 8(%rsp)\n"
        "rol %r8\n"
        "rol $1, %rdx\n"
        "rol %cl, %rcx\n"
        "rolq $5, (%rbx,%rbx,2)\n"
        "ror %r9\n"
        "ror $3, %r10\n"
        "ror %cl, %rax\n"
        "rorq $1, 0x1000(,%rcx,2)\n"
        "rcl %r8\n"
        "rcl $7, %r10\n"
        "rcl %cl, %r8\n"
        "rclq $5, (%rsp)\n"
        "rcr %r11\n"
        "rcr $1, %rbx\n"
        "rcr %cl, %r8\n"
        "rcrq $1, 0(%edx)\n"
        "shl %rbx\n"
        "shl $3, %r10\n"
        "shl %cl, %rdi\n"
        "shlq $2, (%rsp)\n"
        "shr %rsp\n"
        "shr $3, %r11\n"
        "shr %cl, %rdi\n"
        "shrq $1, %gs:0x0\n"
        "sar %r11\n"
        "sar $7, %rsp\n"
        "sar %cl, %r8\n"
        "sarq $2, 128(%r15)\n"
        "sal %r12\n"
        "sal $3, %rdi\n"
        "sal %cl, %r9\n"
        "salq $5, (%rcx,%r10,1)\n"
        "imul %bp, %di\n"
        "imul 4(%r13,%rbp,4), %r8w\n"
        "imul $-128, %r8w, %r15w\n"
        "imul $1000, (%r12,%r12,1), %r11w\n"
        "lea 4660(%r11), %ax\n"
        "cmovnz %r9w, %r14w\n"
        "cmove (%rsp,%rcx,2), %si\n"
        "bt %r15w, %r10w\n"
        "bt %r11w, 0x10(,%rdx,8)\n"
        "bt $31, %bx\n"
        "btw $3, 16(%eax)\n"
        "bts %ax, %sp\n"
        "bts %r12w, (%rsp)\n"
        "bts $31, %r13w\n"
        "btsw $3, 0x10(,%rdi,8)\n"
        "btr %cx, %bx\n"
        "btr %r15w, (%rax)\n"
        "btr $1, %bx\n"
        "btrw $3, 16(%r8d)\n"
        "btc %r11w, %r15w\n"
        "btc %di, 16(%ebp)\n"
        "btc $1, %cx\n"
        "btcw $3, %gs:0x28\n"
        "bsf %cx, %r13w\n"
        "bsf 0x1234, %r10w\n"
        "bsr %r14w, %di\n"
        "bsr 16(%ebx), %bp\n"
        "tzcnt %r15w, %r10w\n"
        "tzcnt -128(%rax), %r12w\n"
        "lzcnt %bp, %r12w\n"
        "lzcnt 1024(%rbx,%rbx,4), %di\n"
        "popcnt %r14w, %r15w\n"
        "popcnt %fs:0x28, %r14w\n"
        "shld $3, %r12w, %bx\n"
        "shrd %cl, %r15w, 0x0(,%rdx,8)\n"
        "shld %r13w, %bx\n"
        "xadd %ax, 128(%rbp)\n"
        "cmpxchg %r9w, (%rsp,%r9,1)\n"
        "lock cmpxchg %r8w, (%r14)\n"
        "lock xadd %di, (%r12)\n"
        "xchg %di, %r13w\n"
        "xchg %bp, (%r10,%r11,2)\n"
        "movbe 128(%rbp), %ax\n"
        "movbe %si, 0x10\n"
        "imul %esi, %edx\n"
        "imul 100000(%r12,%r13,1), %ecx\n"
        "imul $127, %edx, %ebx\n"
        "imul $1000, (%rbp), %eax\n"
        "lea (%r9), %r8d\n"
        "cmovge %ebp, %esp\n"
        "cmove (%r12), %r14d\n"
        "bt %r13d, %ebp\n"
        "bt %r12d, 0x1234\n"
        "bt $31, %esp\n"
        "btl $3, 0x0(,%r14,1)\n"
        "bts %r11d, %r10d\n"
        "bts %esp, 0x10(,%rbp,8)\n"
        "bts $31, %r14d\n"
        "btsl $3, (%rsp)\n"
        "btr %r8d, %edi\n"
        "btr %esi, 128(%rsi)\n"
        "btr $1, %esp\n"
        "btrl $3, (%r12)\n"
        "btc %edx, %edx\n"
        "btc %esp, (%rax)\n"
        "btc $31, %r12d\n"
        "btcl $3, 0x7fffffff\n"
        "bsf %esp, %edx\n"
        "bsf 1024(%r12,%rdx,2), %r12d\n"
        "bsr %r11d, %ebp\n"
        "bsr -8(%r9,%r12,4), %r15d\n"
        "tzcnt %r9d, %edx\n"
        "tzcnt (%r12), %eax\n"
        "lzcnt %r9d, %ebx\n"
        "lzcnt 16(%r8d), %ecx\n"
        "popcnt %ecx, %r10d\n"
        "popcnt (%rsp,%r15,1), %ebx\n"
        "shld $3, %r13d, %edi\n"
        "shrd %cl, %esi, (%r12)\n"
        "shld %esi, %r12d\n"
        "xadd %esp, 0(%ebx)\n"
        "cmpxchg %esi, 16(%edi)\n"
        "lock cmpxchg %r8d, (%rbp)\n"
        "lock xadd %edi, 0x7fffffff\n"
        "xchg %r13d, %r12d\n"
        "xchg %r8d, (%r13)\n"
        "movbe (%rbp,%r9,1), %r14d\n"
        "movbe %ecx, (%r13)\n"
        "imul %r10, %rdi\n"
        "imul 1(%rdx), %r13\n"
        "imul $127, %rsi, %rbp\n"
        "imul $1000, 16(%r11d), %rsi\n"
        "lea 4(%r11,%r11,4), %rcx\n"
        "cmovle %rcx, %rbp\n"
        "cmove (%r9,%r8,1), %rdx\n"
        "bt %r12, %rdx\n"
        "bt %r12, (%rsp)\n"
        "bt $31, %r15\n"
        "btq $3, (%r15)\n"
        "bts %r13, %r9\n"
        "bts %r10, (%r8,%rcx,4)\n"
        "bts $31, %r12\n"
        "btsq $3, (%r12)\n"
        "btr %rcx, %rax\n"
        "btr %rbx, %gs:0x1000\n"
        "btr $1, %r11\n"
        "btrq $3, 0(%r14d)\n"
        "btc %r10, %rax\n"
        "btc %rbp, %fs:0x1000\n"
        "btc $1, %rbx\n"
        "btcq $3, 0x7fffffff\n"
        "bsf %r13, %r11\n"
        "bsf %gs:0x0, %rdx\n"
        "bsr %rdi, %r8\n"
        "bsr 0x7fffffff, %rdx\n"
        "tzcnt %rdx, %rbp\n"
        "tzcnt 0x0(,%rcx,4), %r8\n"
        "lzcnt %rdi, %rsi\n"
        "lzcnt -129(%r8), %rcx\n"
        "popcnt %r9, %rsi\n"
        "popcnt (%rsp), %r9\n"
        "shld $3, %rsp, %rcx\n"
        "shrd %cl, %r14, %gs:0x28\n"
        "shld %rbp, %rcx\n"
        "xadd %r13, (%rsi,%rbx,1)\n"
        "cmpxchg %rsp, 16(%r14d)\n"
        "lock cmpxchg %rsi, (%r11)\n"
        "lock xadd %r10, 0(%eax)\n"
        "xchg %r15, %rcx\n"
        "xchg %rbp, 0x0(,%rax,1)\n"
        "movbe (%r12), %rsi\n"
        "movbe %rsi, (%rbp)\n"
        "bswap %r11d\n"
        "crc32w 0x7fffffff, %ebp\n"
        "crc32b %al, %ecx\n"
        "rdrand %esi\n"
        "rdseed %r9d\n"
        "adcx %r13d, %r15d\n"
        "adox %fs:0x0, %r13d\n"
        "andn %r10d, %r10d, %edx\n"
        "andn 0x7fffffff, %r15d, %r15d\n"
        "bextr %r14d, %r15d, %ebp\n"
        "bextr %r8d, (%r12), %r12d\n"
        "bzhi %r8d, %r8d, %r9d\n"
        "bzhi %eax, 16(%r14d), %r11d\n"
        "sarx %edi, %r14d, %esi\n"
        "sarx %r15d, %gs:0x0, %ebx\n"
        "shlx %r11d, %eax, %r8d\n"
        "shlx %ecx, 0x10(,%rax,4), %r9d\n"
        "shrx %ecx, %esi, %edx\n"
        "shrx %r10d, 127(%rdx), %r13d\n"
        "pdep %r10d, %edi, %eax\n"
        "pext %ebp, %r11d, %r9d\n"
        "mulx %r9d, %r12d, %r13d\n"
        "blsr %r14d, %edx\n"
        "blsmsk %esi, %r13d\n"
        "blsi %edi, %ecx\n"
        "rorx $5, %edi, %edi\n"
        "movnti %edi, 0x7fffffff\n"
        "bswap %rsp\n"
        "crc32q %gs:0x28, %r15\n"
        "rdrand %rbp\n"
        "rdseed %rsp\n"
        "adcx %rax, %r11\n"
        "adox 0x7fffffff, %r15\n"
        "andn %r10, %rbx, %r9\n"
        "andn (%rsp), %r9, %rdx\n"
        "bextr %r15, %rbx, %rdi\n"
        "bextr %r8, 0x10, %rcx\n"
        "bzhi %rbx, %rbp, %rsp\n"
        "bzhi %r9, (%rdx), %rax\n"
        "sarx %rcx, %r13, %rax\n"
        "sarx %rdx, (%rax), %r10\n"
        "shlx %r10, %rax, %rax\n"
        "shlx %rsi, (%rbp), %r8\n"
        "shrx %rdi, %rbp, %rsi\n"
        "shrx %r12, (%rdi), %r14\n"
        "pdep %rcx, %r10, %r10\n"
        "pext %r13, %rbx, %rax\n"
        "mulx %rbp, %rdx, %rbp\n"
        "blsr %rsi, %rdi\n"
        "blsmsk %rbp, %r9\n"
        "blsi %rbx, %rcx\n"
        "rorx $5, %r10, %rsp\n"
        "movnti %rdx, (%rbp)\n"
        "movzbw %r9b, %r11w\n"
        "movzbw (%rdx), %si\n"
        "movsbw %dil, %bp\n"
        "movsbw 1(%rcx), %bx\n"
        "movzbl %dl, %edi\n"
        "movzbl 0x10(,%r9,2), %ecx\n"
        "movsbl %r8b, %esi\n"
        "movsbl %gs:0x1000, %r12d\n"
        "movzbq %r12b, %rdx\n"
        "movzbq 0x1234, %r10\n"
        "movsbq %bpl, %rbx\n"
        "movsbq 100000(%rdx,%r13,4), %r9\n"
        "movzwl %r10w, %r11d\n"
        "movzwl 0x1234, %r10d\n"
        "movswl %r12w, %r15d\n"
        "movswl (%rsp), %ebp\n"
        "movzwq %r9w, %rsp\n"
        "movzwq (%rsp), %rsp\n"
        "movswq %sp, %rbp\n"
        "movswq 127(%r8), %r10\n"
        "movslq %ebp, %r8\n"
        "movslq (%rbp), %r11\n"
        "movzx %al, %ecx\n"
        "movsx %bx, %rdx\n"
        "movsxd %eax, %r9\n"
        "movabs $0x123456789abcdef0, %rax\n"
        "movq $0x100000000, %r11\n"
        "mov %ah, %bl\n"
        "mov %ch, (%rax)\n"
        "add %dh, %al\n"
        "movzbl %ah, %eax\n"
        "push %rax\n"
        "pop %rax\n"
        "push %rcx\n"
        "pop %rcx\n"
        "push %rdx\n"
        "pop %rdx\n"
        "push %rbx\n"
        "pop %rbx\n"
        "push %rsp\n"
        "pop %rsp\n"
        "push %rbp\n"
        "pop %rbp\n"
        "push %rsi\n"
        "pop %rsi\n"
        "push %rdi\n"
        "pop %rdi\n"
        "push %r8\n"
        "pop %r8\n"
        "push %r9\n"
        "pop %r9\n"
        "push %r10\n"
        "pop %r10\n"
        "push %r11\n"
        "pop %r11\n"
        "push %r12\n"
        "pop %r12\n"
        "push %r13\n"
        "pop %r13\n"
        "push %r14\n"
        "pop %r14\n"
        "push %r15\n"
        "pop %r15\n"
        "push %ax\n"
        "pop %ax\n"
        "push %cx\n"
        "pop %cx\n"
        "push %dx\n"
        "pop %dx\n"
        "push %bx\n"
        "pop %bx\n"
        "push %sp\n"
        "pop %sp\n"
        "push %bp\n"
        "pop %bp\n"
        "push %si\n"
        "pop %si\n"
        "push %di\n"
        "pop %di\n"
        "push $1\n"
        "push $1000\n"
        "pushq (%rax)\n"
        "popq 8(%rsp)\n"
        "pushw $5\n"
        "seto %r14b\n"
        "seto 127(%rsp)\n"
        "setno %dl\n"
        "setno (%r15,%r9,1)\n"
        "setb %cl\n"
        "setb 1024(%r11,%r12,4)\n"
        "setae %r11b\n"
        "setae %gs:0x0\n"
        "sete %r8b\n"
        "sete 0(%r9d)\n"
        "setne %r10b\n"
        "setne 16(%ecx)\n"
        "setbe %dil\n"
        "setbe 0x0(,%rbx,2)\n"
        "seta %dil\n"
        "seta %fs:0x0\n"
        "sets %dl\n"
        "sets 0x10(,%r14,8)\n"
        "setns %bpl\n"
        "setns (%rsp)\n"
        "setp %dl\n"
        "setp 0x0(,%rbx,2)\n"
        "setnp %sil\n"
        "setnp (%rbp)\n"
        "setl %r11b\n"
        "setl (%rcx,%r13,4)\n"
        "setge %dl\n"
        "setge 0(%edx)\n"
        "setle %r14b\n"
        "setle 4(%r10,%rdx,2)\n"
        "setg %r10b\n"
        "setg (%rbp)\n"
        "nop\n"
        "pause\n"
        "cpuid\n"
        "rdtsc\n"
        "rdtscp\n"
        "syscall\n"
        "ud2\n"
        "int3\n"
        "mfence\n"
        "lfence\n"
        "sfence\n"
        "clc\n"
        "stc\n"
        "cmc\n"
        "cld\n"
        "std\n"
        "cbtw\n"
        "cwtl\n"
        "cltq\n"
        "cwtd\n"
        "cltd\n"
        "cqto\n"
        "lahf\n"
        "sahf\n"
        "leave\n"
        "ret\n"
        "xgetbv\n"
        "endbr64\n"
        "pushfq\n"
        "popfq\n"
        "ret $16\n"
        "int $0x80\n"
        "enter $16, $0\n"
        "movsb\n"
        "rep movsb\n"
        "movsw\n"
        "rep movsw\n"
        "movsl\n"
        "rep movsl\n"
        "movsq\n"
        "rep movsq\n"
        "cmpsb\n"
        "rep cmpsb\n"
        "cmpsw\n"
        "rep cmpsw\n"
        "cmpsl\n"
        "rep cmpsl\n"
        "cmpsq\n"
        "rep cmpsq\n"
        "stosb\n"
        "rep stosb\n"
        "stosw\n"
        "rep stosw\n"
        "stosl\n"
        "rep stosl\n"
        "stosq\n"
        "rep stosq\n"
        "lodsb\n"
        "rep lodsb\n"
        "lodsw\n"
        "rep lodsw\n"
        "lodsl\n"
        "rep lodsl\n"
        "lodsq\n"
        "rep lodsq\n"
        "scasb\n"
        "rep scasb\n"
        "scasw\n"
        "rep scasw\n"
        "scasl\n"
        "rep scasl\n"
        "scasq\n"
        "rep scasq\n"
        "repne scasb\n"
        "repe cmpsb\n"
        "nopw 0(%rax,%rax,1)\n"
        "nopl 0x0(%rax)\n"
        "nopw %cs:0(%rax,%rax,1)\n"
        "prefetcht0 (%rax)\n"
        "prefetchnta 64(%rdi)\n"
        "prefetchw (%rsi)\n"
        "clflush (%rax)\n"
        "cmpxchg8b (%rax)\n"
        "cmpxchg16b 8(%rdi)\n"
        "lock cmpxchg16b (%r8)\n"
        "jmp *%rax\n"
        "jmp *(%rax)\n"
        "call *%r11\n"
        "call *8(%rax,%rbx,8)\n"
        "jmp *%r15\n"
        "ldmxcsr (%rax)\n"
        "stmxcsr -4(%rsp)\n"
        "addps %xmm11, %xmm15\n"
        "addps (%rsp), %xmm10\n"
        "addpd %xmm9, %xmm10\n"
        "addpd 16(%r13d), %xmm2\n"
        "addss %xmm8, %xmm2\n"
        "addss %gs:0x0, %xmm10\n"
        "addsd %xmm8, %xmm8\n"
        "addsd 0x0(,%rdi,4), %xmm5\n"
        "mulps %xmm9, %xmm1\n"
        "mulps -305419896(%r13), %xmm6\n"
        "mulpd %xmm8, %xmm11\n"
        "mulpd 16(%r8d), %xmm5\n"
        "mulss %xmm10, %xmm4\n"
        "mulss %gs:0x1000, %xmm6\n"
        "mulsd %xmm12, %xmm14\n"
        "mulsd (%r15,%r12,2), %xmm1\n"
        "subps %xmm7, %xmm2\n"
        "subps 4660(%rcx), %xmm15\n"
        "subpd %xmm15, %xmm10\n"
        "subpd (%r12), %xmm12\n"
        "subss %xmm0, %xmm12\n"
        "subss (%r12), %xmm11\n"
        "subsd %xmm1, %xmm11\n"
        "subsd %gs:0x0, %xmm14\n"
        "minps %xmm11, %xmm6\n"
        "minps (%rsp,%r8,1), %xmm11\n"
        "minpd %xmm10, %xmm5\n"
        "minpd 0(%edi), %xmm1\n"
        "minss %xmm14, %xmm5\n"
        "minss (%r12), %xmm3\n"
        "minsd %xmm10, %xmm8\n"
        "minsd (%rbp,%rsi,2), %xmm5\n"
        "divps %xmm9, %xmm7\n"
        "divps (%rsp), %xmm9\n"
        "divpd %xmm5, %xmm9\n"
        "divpd 0(%eax), %xmm10\n"
        "divss %xmm3, %xmm13\n"
        "divss 0x1234, %xmm14\n"
        "divsd %xmm14, %xmm11\n"
        "divsd 4(%rcx,%rcx,1), %xmm12\n"
        "maxps %xmm4, %xmm14\n"
        "maxps 0x1234, %xmm1\n"
        "maxpd %xmm6, %xmm14\n"
        "maxpd (%r13), %xmm5\n"
        "maxss %xmm8, %xmm5\n"
        "maxss (%rcx), %xmm2\n"
        "maxsd %xmm7, %xmm14\n"
        "maxsd %fs:0x28, %xmm1\n"
        "sqrtps %xmm14, %xmm8\n"
        "sqrtps 0x7fffffff, %xmm3\n"
        "sqrtpd %xmm5, %xmm12\n"
        "sqrtpd (%r12), %xmm15\n"
        "sqrtss %xmm4, %xmm10\n"
        "sqrtss (%r11,%rdx,2), %xmm7\n"
        "sqrtsd %xmm6, %xmm14\n"
        "sqrtsd (%rbx,%r12,1), %xmm13\n"
        "movaps %xmm1, %xmm14\n"
        "movaps (%r11,%r9,4), %xmm8\n"
        "movaps %xmm12, (%r12)\n"
        "movapd %xmm14, %xmm9\n"
        "movapd 0x10(,%rsi,2), %xmm3\n"
        "movapd %xmm15, (%r14,%rdx,8)\n"
        "movups %xmm3, %xmm3\n"
        "movups (%r12), %xmm15\n"
        "movups %xmm10, 16(%r10d)\n"
        "movupd %xmm15, %xmm12\n"
        "movupd (%rsp), %xmm6\n"
        "movupd %xmm7, (%r10)\n"
        "movss %xmm1, %xmm10\n"
        "movss 0x1234, %xmm11\n"
        "movss %xmm13, -8(%r9,%r15,4)\n"
        "movsd %xmm12, %xmm12\n"
        "movsd (%rax,%rdi,4), %xmm7\n"
        "movsd %xmm7, -128(%r10)\n"
        "movdqa %xmm9, %xmm3\n"
        "movdqa 0x1234, %xmm2\n"
        "movdqa %xmm13, (%rbx,%r9,2)\n"
        "movdqu %xmm10, %xmm4\n"
        "movdqu 0x7fffffff, %xmm8\n"
        "movdqu %xmm6, -8(%rbp,%rdx,2)\n"
        "movhps 4660(%r14), %xmm4\n"
        "movhps %xmm13, (%r10,%r10,4)\n"
        "movlps 16(%ebp), %xmm7\n"
        "movlps %xmm7, (%r13)\n"
        "movhpd -129(%rsp), %xmm4\n"
        "movhpd %xmm6, %gs:0x0\n"
        "movlpd 0x7fffffff, %xmm14\n"
        "movlpd %xmm6, 4(%rsi,%r12,4)\n"
        "movhlps %xmm1, %xmm8\n"
        "movlhps %xmm6, %xmm2\n"
        "unpcklps %xmm3, %xmm3\n"
        "unpcklps 0x1234, %xmm15\n"
        "unpckhps %xmm8, %xmm4\n"
        "unpckhps 0x1234, %xmm12\n"
        "unpcklpd %xmm13, %xmm13\n"
        "unpcklpd %fs:0x0, %xmm7\n"
        "unpckhpd %xmm7, %xmm0\n"
        "unpckhpd 100000(%r12,%r8,1), %xmm1\n"
        "andps %xmm5, %xmm0\n"
        "andps (%r13), %xmm8\n"
        "andpd %xmm13, %xmm4\n"
        "andpd 100000(%r11,%r15,4), %xmm1\n"
        "andnps %xmm14, %xmm4\n"
        "andnps (%rsp), %xmm3\n"
        "andnpd %xmm7, %xmm3\n"
        "andnpd 0x10, %xmm11\n"
        "orps %xmm4, %xmm4\n"
        "orps 0x1000(,%r8,1), %xmm15\n"
        "orpd %xmm2, %xmm13\n"
        "orpd -305419896(%r15), %xmm3\n"
        "xorps %xmm4, %xmm12\n"
        "xorps 16(%r15d), %xmm10\n"
        "xorpd %xmm14, %xmm3\n"
        "xorpd -305419896(%rsi), %xmm11\n"
        "comiss %xmm3, %xmm3\n"
        "comiss %fs:0x0, %xmm2\n"
        "comisd %xmm0, %xmm13\n"
        "comisd 100000(%rdx,%rbp,1), %xmm13\n"
        "ucomiss %xmm9, %xmm12\n"
        "ucomiss (%rax), %xmm15\n"
        "ucomisd %xmm14, %xmm7\n"
        "ucomisd 0x10(,%r13,8), %xmm1\n"
        "cvtps2pd %xmm8, %xmm5\n"
        "cvtps2pd (%r13), %xmm5\n"
        "cvtpd2ps %xmm10, %xmm12\n"
        "cvtpd2ps 0x7fffffff, %xmm7\n"
        "cvtss2sd %xmm9, %xmm0\n"
        "cvtss2sd -1(%rsp), %xmm11\n"
        "cvtsd2ss %xmm8, %xmm9\n"
        "cvtsd2ss (%rsp), %xmm4\n"
        "cvtdq2ps %xmm14, %xmm1\n"
        "cvtdq2ps (%r13), %xmm10\n"
        "cvtps2dq %xmm11, %xmm4\n"
        "cvtps2dq (%rsi), %xmm8\n"
        "cvttps2dq %xmm2, %xmm14\n"
        "cvttps2dq 0x10(,%r11,1), %xmm12\n"
        "cvtdq2pd %xmm3, %xmm3\n"
        "cvtdq2pd %gs:0x1000, %xmm10\n"
        "cvtpd2dq %xmm1, %xmm6\n"
        "cvtpd2dq (%rcx,%r10,1), %xmm1\n"
        "cvttpd2dq %xmm3, %xmm9\n"
        "cvttpd2dq -8(%rbp,%r9,2), %xmm6\n"
        "rcpps %xmm2, %xmm11\n"
        "rcpps 0(%r9d), %xmm7\n"
        "rcpss %xmm2, %xmm8\n"
        "rcpss (%rax), %xmm9\n"
        "rsqrtps %xmm15, %xmm13\n"
        "rsqrtps 0x10, %xmm1\n"
        "rsqrtss %xmm13, %xmm13\n"
        "rsqrtss %fs:0x0, %xmm7\n"
        "movddup %xmm7, %xmm1\n"
        "movddup %gs:0x28, %xmm6\n"
        "movshdup %xmm7, %xmm8\n"
        "movshdup (%r12,%rcx,8), %xmm0\n"
        "movsldup %xmm15, %xmm9\n"
        "movsldup 0x0(,%r15,2), %xmm12\n"
        "cmpps $0, %xmm12, %xmm14\n"
        "cmpps $3, (%rsp), %xmm15\n"
        "cmppd $0, %xmm9, %xmm13\n"
        "cmppd $3, -8(%r10,%rcx,2), %xmm15\n"
        "cmpss $0, %xmm5, %xmm15\n"
        "cmpss $3, %gs:0x0, %xmm12\n"
        "cmpsd $1, %xmm13, %xmm4\n"
        "cmpsd $3, (%r9), %xmm13\n"
        "shufps $0, %xmm6, %xmm8\n"
        "shufps $3, (%r13), %xmm3\n"
        "shufpd $4, %xmm4, %xmm8\n"
        "shufpd $3, (%rbx), %xmm11\n"
        "pshufd $27, %xmm8, %xmm3\n"
        "pshufd $3, 0x0(,%r13,8), %xmm12\n"
        "pshufhw $0, %xmm15, %xmm4\n"
        "pshufhw $3, (%r12), %xmm7\n"
        "pshuflw $0, %xmm12, %xmm1\n"
        "pshuflw $3, 0x7fffffff, %xmm1\n"
        "roundps $27, %xmm2, %xmm9\n"
        "roundps $3, 0(%edx), %xmm2\n"
        "roundpd $0, %xmm9, %xmm11\n"
        "roundpd $3, 0x10(,%r9,4), %xmm10\n"
        "roundss $1, %xmm11, %xmm7\n"
        "roundss $3, %fs:0x28, %xmm9\n"
        "roundsd $4, %xmm9, %xmm0\n"
        "roundsd $3, (%r13), %xmm7\n"
        "blendps $1, %xmm7, %xmm5\n"
        "blendps $3, -128(%r8), %xmm4\n"
        "blendpd $1, %xmm2, %xmm10\n"
        "blendpd $3, 0x10, %xmm14\n"
        "pblendw $1, %xmm12, %xmm3\n"
        "pblendw $3, 0x1000(,%r12,4), %xmm14\n"
        "palignr $4, %xmm2, %xmm2\n"
        "palignr $3, 4660(%rdi), %xmm14\n"
        "dpps $27, %xmm0, %xmm5\n"
        "dpps $3, (%rbp), %xmm0\n"
        "pclmulqdq $1, %xmm9, %xmm6\n"
        "pclmulqdq $3, (%rsp), %xmm11\n"
        "insertps $4, %xmm9, %xmm1\n"
        "insertps $3, (%rax), %xmm14\n"
        "cmpeqps %xmm1, %xmm6\n"
        "cmpeqpd %xmm2, %xmm10\n"
        "cmpeqss %xmm14, %xmm9\n"
        "cmpeqsd %xmm3, %xmm7\n"
        "cmpltps %xmm3, %xmm6\n"
        "cmpltpd %xmm0, %xmm6\n"
        "cmpltss %xmm4, %xmm0\n"
        "cmpltsd %xmm14, %xmm0\n"
        "cmpleps %xmm7, %xmm15\n"
        "cmplepd %xmm5, %xmm0\n"
        "cmpless %xmm7, %xmm4\n"
        "cmplesd %xmm2, %xmm0\n"
        "cmpunordps %xmm4, %xmm10\n"
        "cmpunordpd %xmm2, %xmm8\n"
        "cmpunordss %xmm6, %xmm12\n"
        "cmpunordsd %xmm0, %xmm8\n"
        "cmpneqps %xmm11, %xmm8\n"
        "cmpneqpd %xmm12, %xmm12\n"
        "cmpneqss %xmm14, %xmm8\n"
        "cmpneqsd %xmm2, %xmm5\n"
        "cmpnltps %xmm15, %xmm12\n"
        "cmpnltpd %xmm4, %xmm6\n"
        "cmpnltss %xmm0, %xmm1\n"
        "cmpnltsd %xmm10, %xmm4\n"
        "cmpnleps %xmm7, %xmm10\n"
        "cmpnlepd %xmm12, %xmm1\n"
        "cmpnless %xmm13, %xmm15\n"
        "cmpnlesd %xmm2, %xmm1\n"
        "cmpordps %xmm4, %xmm13\n"
        "cmpordpd %xmm12, %xmm8\n"
        "cmpordss %xmm1, %xmm6\n"
        "cmpordsd %xmm6, %xmm9\n"
        "punpcklbw %xmm12, %xmm9\n"
        "punpcklbw (%r12), %xmm6\n"
        "punpcklwd %xmm5, %xmm7\n"
        "punpcklwd 8(%rsi), %xmm1\n"
        "punpckldq %xmm12, %xmm9\n"
        "punpckldq (%rsp), %xmm3\n"
        "packsswb %xmm1, %xmm13\n"
        "packsswb (%rbp), %xmm14\n"
        "pcmpgtb %xmm3, %xmm12\n"
        "pcmpgtb 1024(%rdx,%rdx,8), %xmm15\n"
        "pcmpgtw %xmm11, %xmm13\n"
        "pcmpgtw 16(%r8d), %xmm12\n"
        "pcmpgtd %xmm11, %xmm12\n"
        "pcmpgtd 0(%ebp), %xmm11\n"
        "packuswb %xmm2, %xmm0\n"
        "packuswb 0x1234, %xmm3\n"
        "punpckhbw %xmm7, %xmm14\n"
        "punpckhbw %gs:0x1000, %xmm1\n"
        "punpckhwd %xmm8, %xmm10\n"
        "punpckhwd (%rbp,%rdi,4), %xmm14\n"
        "punpckhdq %xmm7, %xmm15\n"
        "punpckhdq 0x1234, %xmm3\n"
        "packssdw %xmm9, %xmm8\n"
        "packssdw (%rdx), %xmm5\n"
        "punpcklqdq %xmm7, %xmm9\n"
        "punpcklqdq 127(%rbp), %xmm12\n"
        "punpckhqdq %xmm14, %xmm15\n"
        "punpckhqdq -1(%r15), %xmm1\n"
        "pcmpeqb %xmm1, %xmm0\n"
        "pcmpeqb 0x10(,%rbp,2), %xmm15\n"
        "pcmpeqw %xmm10, %xmm0\n"
        "pcmpeqw (%r13), %xmm7\n"
        "pcmpeqd %xmm11, %xmm1\n"
        "pcmpeqd (%r14), %xmm6\n"
        "paddq %xmm12, %xmm4\n"
        "paddq (%rdi,%rcx,8), %xmm1\n"
        "pmullw %xmm5, %xmm10\n"
        "pmullw (%r14), %xmm5\n"
        "psubusb %xmm1, %xmm13\n"
        "psubusb 100000(%r8,%r11,2), %xmm1\n"
        "psubusw %xmm12, %xmm13\n"
        "psubusw 0x1234, %xmm10\n"
        "pminub %xmm0, %xmm2\n"
        "pminub (%r13), %xmm5\n"
        "pand %xmm5, %xmm8\n"
        "pand 0x1234, %xmm14\n"
        "paddusb %xmm12, %xmm12\n"
        "paddusb 0x1000(,%rsi,2), %xmm8\n"
        "paddusw %xmm0, %xmm2\n"
        "paddusw 0x10(,%rdx,4), %xmm15\n"
        "pmaxub %xmm0, %xmm5\n"
        "pmaxub (%rbp), %xmm12\n"
        "pandn %xmm1, %xmm5\n"
        "pandn 4660(%rbx), %xmm14\n"
        "pavgb %xmm0, %xmm1\n"
        "pavgb 0x10(,%r9,2), %xmm11\n"
        "pavgw %xmm14, %xmm3\n"
        "pavgw %gs:0x1000, %xmm12\n"
        "pmulhuw %xmm9, %xmm2\n"
        "pmulhuw 1024(%r14,%r9,8), %xmm13\n"
        "pmulhw %xmm13, %xmm8\n"
        "pmulhw (%rsp,%r14,1), %xmm10\n"
        "psubsb %xmm11, %xmm12\n"
        "psubsb 8(%r10), %xmm4\n"
        "psubsw %xmm3, %xmm6\n"
        "psubsw (%r13), %xmm5\n"
        "pminsw %xmm6, %xmm9\n"
        "pminsw (%rsp,%r11,8), %xmm13\n"
        "por %xmm15, %xmm11\n"
        "por (%rdx), %xmm11\n"
        "paddsb %xmm7, %xmm4\n"
        "paddsb 1024(%r12,%r8,8), %xmm10\n"
        "paddsw %xmm15, %xmm10\n"
        "paddsw 8(%rcx), %xmm15\n"
        "pmaxsw %xmm5, %xmm2\n"
        "pmaxsw (%rdx), %xmm5\n"
        "pxor %xmm8, %xmm6\n"
        "pxor (%r13), %xmm8\n"
        "pmuludq %xmm8, %xmm12\n"
        "pmuludq -128(%r12), %xmm2\n"
        "pmaddwd %xmm10, %xmm4\n"
        "pmaddwd 16(%ecx), %xmm9\n"
        "psadbw %xmm11, %xmm0\n"
        "psadbw 0(%r10d), %xmm12\n"
        "psubb %xmm1, %xmm14\n"
        "psubb -1(%r13), %xmm0\n"
        "psubw %xmm0, %xmm13\n"
        "psubw %fs:0x0, %xmm9\n"
        "psubd %xmm13, %xmm5\n"
        "psubd 16(%r8d), %xmm1\n"
        "psubq %xmm12, %xmm13\n"
        "psubq (%r10,%rdx,8), %xmm12\n"
        "paddb %xmm4, %xmm2\n"
        "paddb 16(%r15d), %xmm1\n"
        "paddw %xmm9, %xmm5\n"
        "paddw 0x0(,%rbp,4), %xmm0\n"
        "paddd %xmm3, %xmm3\n"
        "paddd (%rbp), %xmm7\n"
        "pshufb %xmm1, %xmm7\n"
        "pshufb -1(%rbx), %xmm1\n"
        "phaddw %xmm3, %xmm1\n"
        "phaddw 0x10(,%rdx,1), %xmm1\n"
        "phaddd %xmm12, %xmm7\n"
        "phaddd (%r15,%r15,2), %xmm11\n"
        "pmaddubsw %xmm12, %xmm5\n"
        "pmaddubsw %fs:0x0, %xmm9\n"
        "psignb %xmm3, %xmm14\n"
        "psignb 1(%rax), %xmm8\n"
        "pmulhrsw %xmm9, %xmm8\n"
        "pmulhrsw (%r13), %xmm7\n"
        "pabsb %xmm9, %xmm4\n"
        "pabsb (%r12), %xmm14\n"
        "pabsw %xmm3, %xmm13\n"
        "pabsw (%r8,%rcx,4), %xmm8\n"
        "pabsd %xmm6, %xmm10\n"
        "pabsd 0(%eax), %xmm7\n"
        "pmovsxbw %xmm15, %xmm11\n"
        "pmovsxbw (%r13,%r11,4), %xmm13\n"
        "pmovzxbd %xmm14, %xmm3\n"
        "pmovzxbd 0x10(,%r9,4), %xmm6\n"
        "pmovzxdq %xmm6, %xmm7\n"
        "pmovzxdq 1024(%r12,%rsi,1), %xmm15\n"
        "pmuldq %xmm4, %xmm13\n"
        "pmuldq (%r13), %xmm8\n"
        "pcmpeqq %xmm3, %xmm7\n"
        "pcmpeqq 8(%r13), %xmm3\n"
        "packusdw %xmm14, %xmm6\n"
        "packusdw (%rsi,%rbp,4), %xmm10\n"
        "pcmpgtq %xmm11, %xmm8\n"
        "pcmpgtq 0(%r8d), %xmm15\n"
        "pminsb %xmm0, %xmm10\n"
        "pminsb (%rbp), %xmm6\n"
        "pminsd %xmm8, %xmm7\n"
        "pminsd 127(%r13), %xmm11\n"
        "pminuw %xmm6, %xmm3\n"
        "pminuw (%r12), %xmm10\n"
        "pminud %xmm13, %xmm10\n"
        "pminud 16(%r11d), %xmm2\n"
        "pmaxsb %xmm13, %xmm7\n"
        "pmaxsb 16(%r9d), %xmm0\n"
        "pmaxsd %xmm3, %xmm1\n"
        "pmaxsd (%rdi,%r13,8), %xmm9\n"
        "pmaxuw %xmm13, %xmm12\n"
        "pmaxuw 16(%esp), %xmm6\n"
        "pmaxud %xmm15, %xmm12\n"
        "pmaxud (%r13), %xmm5\n"
        "pmulld %xmm15, %xmm6\n"
        "pmulld 0x10(,%r14,4), %xmm4\n"
        "ptest %xmm8, %xmm9\n"
        "ptest (%rbp), %xmm10\n"
        "phminposuw %xmm6, %xmm8\n"
        "phminposuw (%r9), %xmm9\n"
        "aesenc %xmm15, %xmm9\n"
        "aesenc 0x10(,%rbp,4), %xmm4\n"
        "aesenclast %xmm8, %xmm12\n"
        "aesenclast (%rbp), %xmm12\n"
        "aesdec %xmm1, %xmm2\n"
        "aesdec 16(%ecx), %xmm9\n"
        "aesdeclast %xmm1, %xmm13\n"
        "aesdeclast 1(%r10), %xmm11\n"
        "aesimc %xmm7, %xmm11\n"
        "aesimc (%rsp), %xmm2\n"
        "haddps %xmm1, %xmm10\n"
        "haddps (%r14), %xmm5\n"
        "hsubpd %xmm8, %xmm6\n"
        "hsubpd 0x10, %xmm1\n"
        "addsubps %xmm1, %xmm15\n"
        "addsubps 0x1234, %xmm13\n"
        "psrlw $1, %xmm7\n"
        "psrlw %xmm10, %xmm13\n"
        "psrlw 0(%edx), %xmm10\n"
        "psrld $31, %xmm12\n"
        "psrld %xmm5, %xmm7\n"
        "psrld (%rsp), %xmm13\n"
        "psrlq $31, %xmm12\n"
        "psrlq %xmm6, %xmm8\n"
        "psrlq (%r9), %xmm8\n"
        "psraw $1, %xmm5\n"
        "psraw %xmm8, %xmm14\n"
        "psraw 0x1234, %xmm1\n"
        "psrad $7, %xmm5\n"
        "psrad %xmm8, %xmm6\n"
        "psrad (%rcx,%r11,8), %xmm0\n"
        "psllw $31, %xmm9\n"
        "psllw %xmm0, %xmm12\n"
        "psllw %fs:0x1000, %xmm6\n"
        "pslld $1, %xmm5\n"
        "pslld %xmm12, %xmm0\n"
        "pslld 4(%r12,%r11,1), %xmm13\n"
        "psllq $31, %xmm5\n"
        "psllq %xmm1, %xmm12\n"
        "psllq 0x10, %xmm2\n"
        "psrldq $4, %xmm3\n"
        "pslldq $8, %xmm12\n"
        "movd %eax, %xmm1\n"
        "movd %xmm2, %r9d\n"
        "movd (%rax), %xmm9\n"
        "movd %xmm9, 8(%rsp)\n"
        "movq %rax, %xmm1\n"
        "movq %xmm2, %r9\n"
        "movq (%rax), %xmm9\n"
        "movq %xmm9, 8(%rsp)\n"
        "movq %xmm3, %xmm12\n"
        "movmskps %xmm3, %eax\n"
        "movmskpd %xmm13, %r10d\n"
        "pmovmskb %xmm1, %ecx\n"
        "pextrw $3, %xmm2, %eax\n"
        "pinsrw $5, %eax, %xmm9\n"
        "pinsrw $1, (%rax), %xmm1\n"
        "pextrd $1, %xmm3, %eax\n"
        "pextrq $1, %xmm3, %r8\n"
        "pextrb $7, %xmm1, (%rax)\n"
        "pinsrd $2, %ecx, %xmm4\n"
        "pinsrq $1, %r9, %xmm15\n"
        "pinsrb $3, (%rdi), %xmm2\n"
        "extractps $1, %xmm2, %eax\n"
        "cvtsi2ss %eax, %xmm14\n"
        "cvtsi2ss %r10, %xmm10\n"
        "cvtsi2ssl (%rax), %xmm8\n"
        "cvtsi2ssq 8(%rbx), %xmm6\n"
        "cvtsi2sd %eax, %xmm8\n"
        "cvtsi2sd %r10, %xmm12\n"
        "cvtsi2sdl (%rax), %xmm7\n"
        "cvtsi2sdq 8(%rbx), %xmm9\n"
        "cvttss2si %xmm8, %eax\n"
        "cvttss2si %xmm4, %r11\n"
        "cvttss2si (%rax), %ecx\n"
        "cvttsd2si %xmm8, %eax\n"
        "cvttsd2si %xmm11, %r11\n"
        "cvttsd2si (%rax), %ecx\n"
        "cvtss2si %xmm8, %eax\n"
        "cvtss2si %xmm7, %r11\n"
        "cvtss2si (%rax), %ecx\n"
        "cvtsd2si %xmm6, %eax\n"
        "cvtsd2si %xmm0, %r11\n"
        "cvtsd2si (%rax), %ecx\n"
        "movntps %xmm1, (%rax)\n"
        "movntdq %xmm9, 16(%rdi)\n"
        "lddqu (%rax), %xmm1\n"
        "movntdqa (%rsi), %xmm2\n"
        "blendvps %xmm0, %xmm2, %xmm3\n"
        "pblendvb %xmm0, (%rax), %xmm1\n"
        "aeskeygenassist $1, %xmm2, %xmm3\n"
        "lock addl $1, (%rax)\n"
        "lock incq 8(%rdi)\n"
        "lock orb $1, (%rdx)\n"
        "lock btsq $5, (%rax)\n"
        "xchg %rax, %rbx\n"
        "xchg %eax, %eax\n"
        "xchg %ax, %r9w\n"
        "xchgl %ecx, (%rsp)\n"
        "mov %fs:0x28, %rax\n"
        "movq %gs:(%rax), %rcx\n"
        "movl $5, %fs:8\n"
        "add 0x10(%eax), %ecx\n"
        "lea (%esi,%edi,2), %eax\n"
        "add %cl, %dl\n"
        "addb %dl, %gs:0x28\n"
        "add 0(%ebp), %r13b\n"
        "add $-128, %r11b\n"
        "addb $255, (%rbp)\n"
        "add $0, %al\n"
        "add %r11w, %r14w\n"
        "addw %r10w, 0x7fffffff\n"
        "add (%rbp,%rbx,2), %ax\n"
        "add $1, %r10w\n"
        "addw $1, (%r11,%r9,2)\n"
        "add %r13d, %r11d\n"
        "addl %r11d, %gs:0x1000\n"
        "add (%rbp), %r15d\n"
        "add $305419896, %r11d\n"
        "addl $-1, (%r13)\n"
        "add $305419896, %eax\n"
        "add %r14, %r15\n"
        "addq %rdi, %gs:0x28\n"
        "add 0x1000(,%r13,8), %r9\n"
        "add $-1, %rsi\n"
        "addq $128, (%r12)\n"
        "add $0, %rax\n"
        "or %r10b, %al\n"
        "orb %sil, 1(%rcx)\n"
        "or 0x0(,%r11,2), %r8b\n"
        "or $1, %sil\n"
        "orb $0, 0x1234\n"
        "or $127, %al\n"
        "or %bp, %di\n"
        "orw %ax, 1(%rbx)\n"
        "or (%rax), %r8w\n"
        "or $1, %bp\n"
        "orw $-1, (%rax,%rdi,1)\n"
        "or $1, %ax\n"
        "or %esp, %ecx\n"
        "orl %eax, %gs:0x28\n"
        "or (%r9), %ecx\n"
        "or $127, %r12d\n"
        "orl $305419896, (%r15,%rbx,1)\n"
        "or $-1, %eax\n"
        "or %r10, %rbx\n"
        "orq %rax, (%r13)\n"
        "or 0x1234, %rsp\n"
        "or $-2147483648, %r10\n"
        "orq $127, 0x0(,%r11,2)\n"
        "or $-1, %rax\n"
        "adc %cl, %r8b\n"
        "adcb %cl, (%rbp,%rdx,1)\n"
        "adc (%r13), %cl\n"
        "adc $1, %dil\n"
        "adcb $-128, -305419896(%r8)\n"
        "adc $1, %al\n"
        "adc %r11w, %r8w\n"
        "adcw %r13w, 0x0(,%rdx,8)\n"
        "adc 0x7fffffff, %dx\n"
        "adc $1, %bx\n"
        "adcw $0, (%rbp)\n"
        "adc $1, %ax\n"
        "adc %ebx, %esi\n"
        "adcl %eax, (%rsp)\n"
        "adc 0x10, %r13d\n"
        "adc $128, %eax\n"
        "adcl $305419896, 16(%ebp)\n"
        "adc $0, %eax\n"
        "adc %r15, %r11\n"
        "adcq %rax, (%rsp)\n"
        "adc %gs:0x0, %rbx\n"
        "adc $127, %rsi\n"
        "adcq $-2147483648, (%r14)\n"
        "sbb %r15b, %r14b\n"
        "sbbb %sil, 16(%eax)\n"
        "sbb %fs:0x0, %r15b\n"
        "sbb $1, %bl\n"
        "sbbb $255, %gs:0x0\n"
        "sbb %r12w, %bx\n"
        "sbbw %r8w, -305419896(%rbx)\n"
        "sbb %fs:0x0, %r15w\n"
        "sbb $0, %r15w\n"
        "sbbw $127, %gs:0x28\n"
        "sbb $128, %ax\n"
        "sbb %r15d, %r13d\n"
        "sbbl %r15d, 0x0(,%rbx,8)\n"
        "sbb 16(%edx), %ebx\n"
        "sbb $0, %r11d\n"
        "sbbl $1, (%rsp)\n"
        "sbb %rdx, %rcx\n"
        "sbbq %rsp, 0x1000(,%rbx,4)\n"
        "sbb (%rbp), %rsp\n"
        "sbb $305419896, %r13\n"
        "sbbq $0, %gs:0x0\n"
        "sbb $1, %rax\n"
        "and %r14b, %dil\n"
        "andb %r12b, %gs:0x1000\n"
        "and (%r12), %bpl\n"
        "and $-128, %cl\n"
        "andb $-128, 0x1000(,%rbp,8)\n"
        "and $-128, %al\n"
        "and %r11w, %r10w\n"
        "andw %dx, 100000(%rsi,%rdi,1)\n"
        "and %gs:0x1000, %bp\n"
        "and $0, %ax\n"
        "andw $128, -8(%r12,%r15,1)\n"
        "and $128, %ax\n"
        "and %esi, %r8d\n"
        "andl %esi, (%r13)\n"
        "and 0x7fffffff, %ebp\n"
        "and $128, %esi\n"
        "andl $0, %gs:0x1000\n"
        "and $-1, %eax\n"
        "and %rdx, %r15\n"
        "andq %r10, (%r13)\n"
        "and (%rdx), %r11\n"
        "and $1, %r12\n"
        "andq $127, (%rcx,%rdx,8)\n"
        "and $128, %rax\n"
        "sub %r14b, %r9b\n"
        "subb %spl, (%r9)\n"
        "sub (%rbp), %r12b\n"
        "sub $255, %r14b\n"
        "subb $1, 0x0(,%r11,8)\n"
        "sub $255, %al\n"
        "sub %r9w, %dx\n"
        "subw %r8w, %gs:0x1000\n"
        "sub 0x7fffffff, %si\n"
        "sub $128, %sp\n"
        "subw $32767, -128(%r9)\n"
        "sub $128, %ax\n"
        "sub %edi, %r8d\n"
        "subl %ecx, 128(%rbx)\n"
        "sub %gs:0x0, %r10d\n"
        "sub $128, %r11d\n"
        "subl $1, (%rbp)\n"
        "sub $1, %eax\n"
        "sub %r14, %rsi\n"
        "subq %r8, %fs:0x28\n"
        "sub 1024(%r11,%rdx,2), %rsp\n"
        "sub $1, %r8\n"
        "subq $-2147483648, (%r12)\n"
        "sub $-1, %rax\n"
        "xor %r10b, %r8b\n"
        "xorb %r10b, 0x7fffffff\n"
        "xor 128(%r11), %r15b\n"
        "xor $1, %r8b\n"
        "xorb $127, (%r13)\n"
        "xor $1, %al\n"
        "xor %r10w, %r12w\n"
        "xorw %sp, (%rbx)\n"
        "xor (%r11,%rcx,8), %ax\n"
        "xor $32767, %r10w\n"
        "xorw $1, 16(%r15d)\n"
        "xor $-1, %ax\n"
        "xor %esp, %r11d\n"
        "xorl %r10d, -8(%r15,%rcx,2)\n"
        "xor %gs:0x28, %r8d\n"
        "xor $0, %r10d\n"
        "xorl $1, 4(%rdi,%r12,4)\n"
        "xor $127, %eax\n"
        "xor %rcx, %rsp\n"
        "xorq %rbp, 127(%r13)\n"
        "xor (%r10,%r9,1), %r10\n"
        "xor $-1, %r12\n"
        "xorq $1, (%r12)\n"
        "xor $128, %rax\n"
        "cmp %r15b, %r10b\n"
        "cmpb %dl, 16(%ebp)\n"
        "cmp 0x1234, %cl\n"
        "cmp $0, %r14b\n"
        "cmpb $255, (%rbx,%r15,2)\n"
        "cmp $0, %al\n"
        "cmp %r12w, %r9w\n"
        "cmpw %r14w, (%r8)\n"
        "cmp %fs:0x0, %r13w\n"
        "cmp $-1, %dx\n"
        "cmpw $127, (%r13)\n"
        "cmp $1, %ax\n"
        "cmp %edx, %r15d\n"
        "cmpl %esp, (%r12)\n"
        "cmp (%rsp), %eax\n"
        "cmp $305419896, %r10d\n"
        "cmpl $-1, (%rsp)\n"
        "cmp $0, %eax\n"
        "cmp %rsp, %rbx\n"
        "cmpq %rdi, 1(%r14)\n"
        "cmp 16(%r10d), %r12\n"
        "cmp $-1, %rbp\n"
        "cmpq $305419896, -128(%rsp)\n"
        "cmp $1, %rax\n"
        "test %r12b, %sil\n"
        "test %r9b, %gs:0x0\n"
        "test $-128, %r10b\n"
        "testb $127, 127(%rbx)\n"
        "mov %r9b, %r15b\n"
        "mov %r8b, -8(%r13,%r12,1)\n"
        "mov (%rsi), %sil\n"
        "mov $-128, %cl\n"
        "movb $1, (%r8)\n"
        "not %r15b\n"
        "neg %dil\n"
        "negb (%r10,%rax,2)\n"
        "mulb (%rbp,%r13,1)\n"
        "imul %r14b\n"
        "imulb 0x10(,%rdx,4)\n"
        "div %dl\n"
        "divb 0x7fffffff\n"
        "idiv %cl\n"
        "idivb (%r13)\n"
        "inc %r8b\n"
        "incb (%rsi)\n"
        "dec %r10b\n"
        "decb 0x1000(,%r10,2)\n"
        "rol %r13b\n"
        "rol $1, %bpl\n"
        "rol %cl, %r14b\n"
        "rolb $2, %gs:0x1000\n"
        "ror %sil\n"
        "ror $7, %r15b\n"
        "ror %cl, %r14b\n"
        "rorb $1, (%rbp)\n"
        "rcl %r11b\n"
        "rcl $7, %r15b\n"
        "rcl %cl, %dil\n"
        "rclb $5, 16(%esi)\n"
        "rcr %r9b\n"
        "rcr %cl, %al\n"
        "rcrb $1, %gs:0x1000\n"
        "shl %r10b\n"
        "shl $3, %dl\n"
        "shl %cl, %r13b\n"
        "shlb $2, (%r9)\n"
        "shr $1, %spl\n"
        "shr %cl, %bpl\n"
        "shrb $5, 0x7fffffff\n"
        "sar %r14b\n"
        "sar $3, %dl\n"
        "sar %cl, %r15b\n"
        "sarb $2, 1024(%rsp,%r10,2)\n"
        "sal %sil\n"
        "sal $7, %r10b\n"
        "sal %cl, %r12b\n"
        "salb $5, 0x10\n"
        "test %cx, %r8w\n"
        "test %di, (%r12,%r14,8)\n"
        "test $0, %r14w\n"
        "testw $128, 0x1234\n"
        "mov %si, %di\n"
        "mov %di, (%rdx)\n"
        "mov (%rsp), %r13w\n"
        "mov $128, %di\n"
        "movw $32767, 0x1000(,%rsi,4)\n"
        "not %r15w\n"
        "notw 16(%edi)\n"
        "neg %r8w\n"
        "negw (%rax)\n"
        "mul %cx\n"
        "mulw (%rsp,%rbx,4)\n"
        "imul %di\n"
        "imulw (%rsp)\n"
        "div %r9w\n"
        "divw 8(%rdx)\n"
        "idiv %r13w\n"
        "idivw (%rcx,%rbp,4)\n"
        "incw (%r11)\n"
        "dec %r10w\n"
        "decw -305419896(%r11)\n"
        "rol %r11w\n"
        "rol $3, %r8w\n"
        "rol %cl, %r15w\n"
        "rolw $2, (%rsp)\n"
        "ror $7, %ax\n"
        "rorw $5, (%r12)\n"
        "rcl %r13w\n"
        "rcl $3, %r13w\n"
        "rcl %cl, %di\n"
        "rclw $1, (%rcx,%rax,4)\n"
        "rcr %bp\n"
        "rcr $3, %ax\n"
        "rcr %cl, %cx\n"
        "rcrw $1, 0(%r11d)\n"
        "shl %bx\n"
        "shl $7, %dx\n"
        "shlw $1, (%rsp)\n"
        "shr %r12w\n"
        "shr $1, %r8w\n"
        "shr %cl, %di\n"
        "shrw $1, (%r12)\n"
        "sar $1, %r13w\n"
        "sar %cl, %sp\n"
        "sarw $2, %fs:0x1000\n"
        "sal %r14w\n"
        "sal $3, %r14w\n"
        "sal %cl, %bp\n"
        "salw $2, 16(%esp)\n"
        "test %eax, %r8d\n"
        "test %ebp, (%r13,%r10,4)\n"
        "test $128, %r15d\n"
        "testl $128, 1024(%r13,%rdi,2)\n"
        "mov %r11d, %ecx\n"
        "mov %r12d, 16(%eax)\n"
        "mov (%rsp), %r8d\n"
        "mov $1, %r14d\n"
        "movl $-1, (%r13)\n"
        "notl 1024(%rbp,%r8,4)\n"
        "neg %r13d\n"
        "negl 128(%rdi)\n"
        "mul %ebx\n"
        "mull 0x7fffffff\n"
        "imul %r14d\n"
        "imull -305419896(%r12)\n"
        "div %r11d\n"
        "divl (%rsp)\n"
        "idiv %ebp\n"
        "idivl 0x7fffffff\n"
        "inc %ebp\n"
        "incl 0x1234\n"
        "dec %r12d\n"
        "decl 0x10(,%r11,1)\n"
        "rol %r12d\n"
        "rol $7, %ecx\n"
        "rol %cl, %esi\n"
        "roll $2, 127(%rbx)\n"
        "ror %r10d\n"
        "ror $7, %r10d\n"
        "ror %cl, %r12d\n"
        "rorl $1, %gs:0x28\n"
        "rcl %edi\n"
        "rcl $3, %ebx\n"
        "rcl %cl, %eax\n"
        "rcll $2, (%r9)\n"
        "rcr %esp\n"
        "rcr $7, %ecx\n"
        "rcr %cl, %eax\n"
        "rcrl $2, 0x7fffffff\n"
        "shl $3, %esi\n"
        "shl %cl, %edi\n"
        "shll $5, 16(%r9d)\n"
        "shr %r12d\n"
        "shr $7, %r15d\n"
        "shr %cl, %r14d\n"
        "shrl $2, 127(%rsi)\n"
        "sar %edi\n"
        "sar $3, %r11d\n"
        "sar %cl, %ebp\n"
        "sarl $1, (%r12)\n"
        "sal %edi\n"
        "sal $3, %r8d\n"
        "sal %cl, %esi\n"
        "sall $5, (%rax,%rdi,8)\n"
        "test %r11, %rbp\n"
        "test %rsi, 16(%eax)\n"
        "test $-2147483648, %rsp\n"
        "testq $1, 4(%rax,%r10,8)\n"
        "mov %rsi, %rbp\n"
        "mov %r8, 0x1234\n"
        "mov -305419896(%r9), %r11\n"
        "mov $128, %r12\n"
        "movq $0, 8(%r14)\n"
        "not %r14\n"
        "notq -129(%r8)\n"
        "neg %rdx\n"
        "negq (%r12)\n"
        "mul %rsi\n"
        "mulq 8(%r13)\n"
        "imul %rbp\n"
        "imulq 0(%ecx)\n"
        "div %r10\n"
        "divq (%r13)\n"
        "idiv %r11\n"
        "idivq 0x0(,%r14,2)\n"
        "inc %rdi\n"
        "incq (%r10)\n"
        "dec %r8\n"
        "decq -1(%r11)\n"
        "rol %rsp\n"
        "rol $3, %rsp\n"
        "rol %cl, %r13\n"
        "rolq $5, 4(%r9,%r9,1)\n"
        "ror %r10\n"
        "ror $7, %rsp\n"
        "ror %cl, %rdx\n"
        "rorq $5, (%rsp)\n"
        "rcl %r15\n"
        "rcl $3, %r11\n"
        "rcl %cl, %r15\n"
        "rclq $5, 0x1000(,%rcx,2)\n"
        "rcr %rbx\n"
        "rcr $1, %rdx\n"
        "rcr %cl, %rsp\n"
        "rcrq $2, (%r9,%r13,2)\n"
        "shl %rdx\n"
        "shl $1, %rdi\n"
        "shl %cl, %r10\n"
        "shlq $1, (%r13,%rbx,8)\n"
        "shr %rcx\n"
        "shr $3, %r13\n"
        "shr %cl, %r15\n"
        "shrq $5, (%rsp)\n"
        "sar %rbx\n"
        "sar $7, %rcx\n"
        "sar %cl, %rsp\n"
        "sarq $5, 0x7fffffff\n"
        "sal %rdi\n"
        "sal $7, %r8\n"
        "sal %cl, %rbx\n"
        "salq $1, (%r12)\n"
        "imul %dx, %r10w\n"
        "imul (%rdi,%rbx,8), %r12w\n"
        "imul $127, %bx, %r10w\n"
        "imul $1000, 4(%rcx,%rsi,8), %ax\n"
        "lea 16(%r11d), %di\n"
        "cmovp %r13w, %di\n"
        "cmove (%rsp), %sp\n"
        "bt %bp, %r12w\n"
        "bt %sp, 128(%r9)\n"
        "bt $31, %ax\n"
        "btw $3, -1(%r13)\n"
        "bts %cx, %si\n"
        "bts %r13w, (%rsi)\n"
        "bts $31, %r10w\n"
        "btsw $3, (%rbp,%rbp,4)\n"
        "btr %r8w, %di\n"
        "btr %r9w, 0x10(,%r8,8)\n"
        "btr $31, %r12w\n"
        "btrw $3, (%r13,%rbp,8)\n"
        "btc %r13w, %r10w\n"
        "btc %dx, 16(%r14d)\n"
        "btc $1, %r15w\n"
        "btcw $3, (%r12)\n"
        "bsf %r11w, %bx\n"
        "bsf -8(%rsi,%rdi,2), %r12w\n"
        "bsr %bp, %dx\n"
        "bsr 0x1000(,%rdx,1), %r11w\n"
        "tzcnt %bp, %di\n"
        "tzcnt (%r12), %r12w\n"
        "lzcnt %r9w, %di\n"
        "lzcnt (%r12,%r13,2), %si\n"
        "popcnt %di, %r8w\n"
        "popcnt (%rsp), %ax\n"
        "shld $3, %r12w, %cx\n"
        "shrd %cl, %cx, (%rbp)\n"
        "shld %r9w, %cx\n"
        "xadd %r14w, %fs:0x0\n"
        "cmpxchg %r11w, 0x1000(,%rbp,4)\n"
        "lock cmpxchg %cx, %fs:0x0\n"
        "lock xadd %r12w, (%r13)\n"
        "xchg %r9w, %dx\n"
        "xchg %bp, (%r13)\n"
        "movbe 1(%rsp), %r11w\n"
        "movbe %si, 16(%ebx)\n"
        "imul %eax, %edx\n"
        "imul 4(%rax,%r12,2), %eax\n"
        "imul $5, %r8d, %ebx\n"
        "imul $1000, (%rax), %r11d\n"
        "lea 0x1000(,%rdi,2), %r13d\n"
        "cmovo %edi, %esi\n"
        "cmove (%r13,%r9,1), %r8d\n"
        "bt %ebx, %edi\n"
        "bt %edi, (%rsi)\n"
        "bt $1, %ebp\n"
        "btl $3, 0(%esi)\n"
        "bts %edi, %r9d\n"
        "bts %r10d, 8(%rsi)\n"
        "bts $1, %ebx\n"
        "btsl $3, %fs:0x0\n"
        "btr %esi, %r12d\n"
        "btr %r9d, (%rdx)\n"
        "btr $31, %r11d\n"
        "btrl $3, 0x0(,%r11,4)\n"
        "btc %edx, %r15d\n"
        "btc %esp, 16(%r13d)\n"
        "btc $1, %ebp\n"
        "btcl $3, 0(%edx)\n"
        "bsf %r14d, %edx\n"
        "bsf 0(%esp), %r12d\n"
        "bsr %esi, %r15d\n"
        "bsr %fs:0x0, %r10d\n"
        "tzcnt %r10d, %r9d\n"
        "tzcnt 0x7fffffff, %r8d\n"
        "lzcnt %r15d, %r10d\n"
        "lzcnt (%rcx), %ebx\n"
        "popcnt %esi, %r9d\n"
        "popcnt (%r14), %edi\n"
        "shld $3, %esi, %edi\n"
        "shrd %cl, %edx, (%rax)\n"
        "shld %r12d, %ebx\n"
        "xadd %r13d, 0(%eax)\n"
        "cmpxchg %ebp, -128(%rsi)\n"
        "lock cmpxchg %edx, 0x0(,%rcx,4)\n"
        "lock xadd %esi, 0(%eax)\n"
        "xchg %ebx, %r14d\n"
        "xchg %r14d, (%rax)\n"
        "movbe 4(%r10,%r11,2), %r8d\n"
        "movbe %ebx, (%rsi,%rcx,8)\n"
        "imul %r8, %r15\n"
        "imul -305419896(%r8), %rsp\n"
        "imul $5, %r12, %rdi\n"
        "imul $1000, 16(%r14d), %rbx\n"
        "lea (%r13), %rcx\n"
        "cmovle %rcx, %r13\n"
        "cmove (%r13), %rcx\n"
        "bt %r12, %r8\n"
        "bt %rbx, (%r13)\n"
        "bt $31, %rdi\n"
        "btq $3, (%rdi,%r14,1)\n"
        "bts %rax, %r8\n"
        "bts %rsp, 0x7fffffff\n"
        "bts $1, %rsp\n"
        "btsq $3, (%rcx)\n"
        "btr %rsi, %rcx\n"
        "btr %r8, (%rsp)\n"
        "btr $31, %rdx\n"
        "btrq $3, 0x1234\n"
        "btc %r9, %rdi\n"
        "btc %rax, -8(%r8,%rax,2)\n"
        "btc $31, %r15\n"
        "btcq $3, (%rbp)\n"
        "bsf %rsi, %r13\n"
        "bsf 4660(%r14), %rbp\n"
        "bsr %rbp, %r9\n"
        "bsr 0(%ebp), %rax\n"
        "tzcnt %r10, %rcx\n"
        "tzcnt 16(%r10d), %rsp\n"
        "lzcnt %rdx, %rdi\n"
        "lzcnt (%r10,%r12,4), %rcx\n"
        "popcnt %rsi, %rcx\n"
        "popcnt 4(%r11,%rax,8), %r12\n"
        "shrd %cl, %r13, 4660(%r10)\n"
        "shld %rdi, %rbx\n"
        "xadd %r8, 0x1000(,%r8,4)\n"
        "cmpxchg %r14, 4(%r10,%r9,1)\n"
        "lock cmpxchg %rbx, 0x7fffffff\n"
        "lock xadd %r14, 0(%r8d)\n"
        "xchg %rax, %rax\n"
        "xchg %rbp, 0(%edx)\n"
        "movbe (%r12), %r10\n"
        "movbe %rcx, 127(%r10)\n"
        "bswap %ebp\n"
        "crc32l (%r8,%rbp,8), %r8d\n"
        "rdrand %r9d\n"
        "rdseed %r10d\n"
        "adcx %r14d, %r13d\n"
        "adox 0x10, %esp\n"
        "andn %ebx, %r10d, %r12d\n"
        "andn (%rbp), %r14d, %r8d\n"
        "bextr %r15d, %eax, %r15d\n"
        "bextr %r11d, 0x10, %eax\n"
        "bzhi %ebp, %r8d, %eax\n"
        "bzhi %r9d, (%r13), %r9d\n"
        "sarx %r12d, %edi, %r13d\n"
        "sarx %r8d, 16(%eax), %r8d\n"
        "shlx %ebp, %r9d, %eax\n"
        "shlx %esi, (%rsi), %r10d\n"
        "shrx %r8d, %esi, %r10d\n"
        "shrx %r12d, 0(%ebx), %edi\n"
        "pdep %r13d, %ebx, %r12d\n"
        "pext %r9d, %esp, %r8d\n"
        "mulx %esi, %r12d, %ecx\n"
        "blsr %r13d, %r13d\n"
        "blsmsk %r10d, %edx\n"
        "blsi %esi, %r14d\n"
        "rorx $5, %esp, %r14d\n"
        "movnti %r12d, (%r13)\n"
        "bswap %rdi\n"
        "crc32q (%r13), %r15\n"
        "rdrand %r8\n"
        "rdseed %rsi\n"
        "adcx %r15, %r12\n"
        "adox (%r8), %r8\n"
        "andn %r9, %r10, %rsi\n"
        "andn 0(%ebx), %rbp, %rbp\n"
        "bextr %r10, %r9, %rdi\n"
        "bextr %rsi, 127(%rbx), %r14\n"
        "bzhi %r10, %rsp, %rsi\n"
        "bzhi %rdi, -129(%r9), %r15\n"
        "sarx %r14, %r10, %r9\n"
        "sarx %rsp, (%r13), %r12\n"
        "shlx %rbp, %rdi, %r10\n"
        "shlx %r13, 0x10(,%rdx,4), %r13\n"
        "shrx %rcx, %rbp, %r11\n"
        "shrx %rsi, 0(%r8d), %rsp\n"
        "pdep %r9, %rdi, %rdx\n"
        "pext %rax, %r15, %rax\n"
        "mulx %rsi, %rax, %rdx\n"
        "blsr %r10, %rbp\n"
        "blsmsk %r12, %rbp\n"
        "blsi %r9, %r13\n"
        "movnti %rax, (%r12)\n"
        "movzbw %al, %r12w\n"
        "movzbw (%rbp,%rbx,1), %r14w\n"
        "movsbw %sil, %r10w\n"
        "movsbw 4(%rdx,%rbp,1), %r8w\n"
        "movzbl %r13b, %ebx\n"
        "movzbl 128(%rsi), %esi\n"
        "movsbl %r13b, %edi\n"
        "movsbl (%rbp), %edx\n"
        "movzbq %r11b, %rsi\n"
        "movzbq 100000(%r11,%r12,4), %r8\n"
        "movsbq %bpl, %r14\n"
        "movsbq (%r12), %rdx\n"
        "movzwl %ax, %esp\n"
        "movzwl %fs:0x0, %r13d\n"
        "movswl %r13w, %r9d\n"
        "movswl (%r9,%r11,1), %r13d\n"
        "movzwq %r14w, %rsp\n"
        "movzwq 0x1000(,%r9,1), %rsp\n"
        "movswq %r12w, %r11\n"
        "movswq (%r13), %r12\n"
        "movslq %edx, %rdx\n"
        "movslq (%r13), %rbp\n"
        "seto %dil\n"
        "seto %gs:0x1000\n"
        "setno %r8b\n"
        "setno 16(%r13d)\n"
        "setb %spl\n"
        "setb (%r12)\n"
        "setae %sil\n"
        "setae 4(%rbx,%rax,8)\n"
        "sete %dl\n"
        "sete 0x1234\n"
        "setne %r12b\n"
        "setne (%rax,%rax,8)\n"
        "setbe %r13b\n"
        "setbe 16(%esp)\n"
        "seta %bl\n"
        "seta -128(%rbp)\n"
        "sets %sil\n"
        "sets -128(%r10)\n"
        "setns %r10b\n"
        "setp %cl\n"
        "setp (%rsp)\n"
        "setnp %dil\n"
        "setnp 100000(%r14,%rax,4)\n"
        "setl %r15b\n"
        "setl (%r12)\n"
        "setge %dil\n"
        "setge -129(%r11)\n"
        "setle %al\n"
        "setle 0x1234\n"
        "setg 0(%r14d)\n"
        "addps %xmm2, %xmm7\n"
        "addps (%rax,%r8,8), %xmm10\n"
        "addpd %xmm14, %xmm15\n"
        "addpd (%r13), %xmm9\n"
        "addss %xmm15, %xmm9\n"
        "addss 128(%rbx), %xmm3\n"
        "addsd %xmm10, %xmm1\n"
        "addsd 0x1000(,%rbx,2), %xmm11\n"
        "mulps %xmm11, %xmm4\n"
        "mulps -1(%r8), %xmm2\n"
        "mulpd %xmm4, %xmm7\n"
        "mulpd (%r9), %xmm5\n"
        "mulss %xmm1, %xmm12\n"
        "mulss %gs:0x1000, %xmm14\n"
        "mulsd %xmm5, %xmm1\n"
        "mulsd 1024(%r12,%r12,4), %xmm5\n"
        "subps %xmm14, %xmm10\n"
        "subps 127(%r9), %xmm6\n"
        "subpd %xmm2, %xmm2\n"
        "subpd %fs:0x1000, %xmm4\n"
        "subss %xmm11, %xmm11\n"
        "subss 0x0(,%r9,2), %xmm11\n"
        "subsd %xmm5, %xmm8\n"
        "subsd 0x1000(,%rdx,1), %xmm13\n"
        "minps %xmm4, %xmm12\n"
        "minps 0x7fffffff, %xmm11\n"
        "minpd %xmm13, %xmm10\n"
        "minpd 0x0(,%r8,4), %xmm12\n"
        "minss %xmm9, %xmm2\n"
        "minss 8(%r15), %xmm3\n"
        "minsd %xmm13, %xmm8\n"
        "minsd (%rsp), %xmm14\n"
        "divps %xmm1, %xmm4\n"
        "divps 100000(%rax,%rdx,8), %xmm0\n"
        "divpd %xmm7, %xmm7\n"
        "divpd (%r12), %xmm4\n"
        "divss %xmm6, %xmm14\n"
        "divss 4(%r9,%r8,2), %xmm15\n"
        "divsd %xmm1, %xmm7\n"
        "divsd 0x0(,%r10,1), %xmm10\n"
        "maxps %xmm8, %xmm12\n"
        "maxps (%r13), %xmm11\n"
        "maxpd %xmm7, %xmm8\n"
        "maxpd (%rcx), %xmm12\n"
        "maxss %xmm0, %xmm7\n"
        "maxss (%r13), %xmm0\n"
        "maxsd %xmm14, %xmm10\n"
        "maxsd (%r14,%rsi,4), %xmm10\n"
        "sqrtps %xmm8, %xmm4\n"
        "sqrtps (%rsp), %xmm10\n"
        "sqrtpd %xmm13, %xmm9\n"
        "sqrtpd (%rdx,%r14,2), %xmm9\n"
        "sqrtss %xmm12, %xmm11\n"
        "sqrtss (%r12), %xmm8\n"
        "sqrtsd %xmm14, %xmm9\n"
        "sqrtsd -1(%r8), %xmm3\n"
        "movaps %xmm13, %xmm1\n"
        "movaps (%r13), %xmm6\n"
        "movaps %xmm10, 1024(%r14,%r14,8)\n"
        "movapd %xmm10, %xmm11\n"
        "movapd 0x7fffffff, %xmm15\n"
        "movapd %xmm7, (%r13)\n"
        "movups %xmm14, %xmm6\n"
        "movups 8(%r12), %xmm12\n"
        "movups %xmm12, (%r12,%r15,4)\n"
        "movupd %xmm0, %xmm12\n"
        "movupd 0x1234, %xmm3\n"
        "movupd %xmm7, (%r12)\n"
        "movss (%r11), %xmm9\n"
        "movss %xmm12, 0x7fffffff\n"
        "movsd %xmm1, %xmm15\n"
        "movsd 0(%esi), %xmm13\n"
        "movsd %xmm15, (%r12)\n"
        "movdqa %xmm2, %xmm13\n"
        "movdqa 0x10(,%rsi,2), %xmm13\n"
        "movdqa %xmm2, -129(%rsi)\n"
        "movdqu %xmm2, %xmm13\n"
        "movdqu 0(%r11d), %xmm13\n"
        "movdqu %xmm15, 0(%esi)\n"
        "movhps (%r15), %xmm1\n"
        "movhps %xmm4, 16(%r11d)\n"
        "movlps (%rsi,%r13,4), %xmm12\n"
        "movlps %xmm13, (%r13)\n"
        "movhpd 0x7fffffff, %xmm11\n"
        "movhpd %xmm4, (%rbp)\n"
        "movlpd 0(%ebx), %xmm4\n"
        "movlpd %xmm5, (%r13)\n"
        "movhlps %xmm3, %xmm10\n"
        "movlhps %xmm0, %xmm15\n"
        "unpcklps %xmm9, %xmm8\n"
        "unpcklps (%r12), %xmm14\n"
        "unpckhps %xmm5, %xmm6\n"
        "unpckhps 0x7fffffff, %xmm1\n"
        "unpcklpd %xmm10, %xmm0\n"
        "unpcklpd %gs:0x1000, %xmm14\n"
        "unpckhpd %xmm11, %xmm8\n"
        "unpckhpd 1024(%r12,%rdx,8), %xmm11\n"
        "andps %xmm3, %xmm14\n"
        "andps 0x1234, %xmm15\n"
        "andpd %xmm0, %xmm6\n"
        "andpd 0x1000(,%r15,1), %xmm2\n"
        "andnps %xmm4, %xmm2\n"
        "andnps (%rax), %xmm5\n"
        "andnpd %xmm0, %xmm11\n"
        "andnpd (%rsp), %xmm7\n"
        "orps %xmm13, %xmm8\n"
        "orps -8(%r13,%rax,4), %xmm7\n"
        "orpd %xmm9, %xmm13\n"
        "orpd 0x10, %xmm5\n"
        "xorps %xmm12, %xmm15\n"
        "xorps 0x1000(,%rdi,8), %xmm7\n"
        "xorpd %xmm6, %xmm3\n"
        "xorpd (%rsp), %xmm1\n"
        "comiss %xmm3, %xmm7\n"
        "comiss (%rbp), %xmm11\n"
        "comisd %xmm1, %xmm11\n"
        "comisd (%r9), %xmm6\n"
        "ucomiss %xmm9, %xmm5\n"
        "ucomiss (%r8), %xmm15\n"
        "ucomisd %xmm5, %xmm13\n"
        "ucomisd (%rdx,%rax,2), %xmm12\n"
        "cvtps2pd %xmm14, %xmm2\n"
        "cvtps2pd %fs:0x1000, %xmm9\n"
        "cvtpd2ps %xmm12, %xmm8\n"
        "cvtpd2ps (%rbx,%r14,8), %xmm5\n"
        "cvtss2sd %xmm11, %xmm4\n"
        "cvtss2sd (%r12), %xmm5\n"
        "cvtsd2ss %xmm3, %xmm8\n"
        "cvtsd2ss (%rbp), %xmm5\n"
        "cvtdq2ps %xmm6, %xmm12\n"
        "cvtdq2ps (%rsp), %xmm15\n"
        "cvtps2dq %xmm8, %xmm0\n"
        "cvtps2dq 0x1234, %xmm0\n"
        "cvttps2dq %xmm8, %xmm8\n"
        "cvttps2dq 16(%esi), %xmm2\n"
        "cvtdq2pd %xmm13, %xmm10\n"
        "cvtdq2pd 127(%r8), %xmm3\n"
        "cvtpd2dq %xmm10, %xmm9\n"
        "cvtpd2dq 4(%rsp,%rcx,8), %xmm4\n"
        "cvttpd2dq %xmm5, %xmm4\n"
        "cvttpd2dq 0x1234, %xmm2\n"
        "rcpps %xmm13, %xmm10\n"
        "rcpps 8(%rdx), %xmm13\n"
        "rcpss %xmm7, %xmm10\n"
        "rcpss 0x7fffffff, %xmm7\n"
        "rsqrtps %xmm11, %xmm15\n"
        "rsqrtps 16(%esi), %xmm1\n"
        "rsqrtss %xmm10, %xmm9\n"
        "rsqrtss (%rsp), %xmm9\n"
        "movddup %xmm2, %xmm11\n"
        "movddup 0x10, %xmm6\n"
        "movshdup %xmm7, %xmm7\n"
        "movshdup (%r13), %xmm11\n"
        "movsldup %xmm6, %xmm1\n"
        "movsldup (%r10,%r9,4), %xmm10\n"
        "cmpps $27, %xmm11, %xmm15\n"
        "cmpps $3, (%rsi,%rbp,8), %xmm11\n"
        "cmppd $1, %xmm8, %xmm6\n"
        "cmppd $3, (%rsp), %xmm7\n"
        "cmpss $27, %xmm10, %xmm9\n"
        "cmpss $3, %gs:0x1000, %xmm13\n"
        "cmpsd $4, %xmm0, %xmm8\n"
        "cmpsd $3, 4(%r12,%r13,2), %xmm1\n"
        "shufps $27, %xmm5, %xmm11\n"
        "shufps $3, (%r13), %xmm15\n"
        "shufpd $0, %xmm6, %xmm7\n"
        "shufpd $3, %fs:0x0, %xmm7\n"
        "pshufd $1, %xmm9, %xmm3\n"
        "pshufd $3, (%r12), %xmm1\n"
        "pshufhw $1, %xmm0, %xmm14\n"
        "pshufhw $3, 0(%r11d), %xmm11\n"
        "pshuflw $0, %xmm8, %xmm6\n"
        "pshuflw $3, 0(%ebp), %xmm10\n"
        "roundps $27, %xmm7, %xmm4\n"
        "roundps $3, (%r9,%rbx,8), %xmm8\n"
        "roundpd $4, %xmm7, %xmm13\n"
        "roundpd $3, %fs:0x0, %xmm3\n"
        "roundss $1, %xmm11, %xmm12\n"
        "roundss $3, 0x10, %xmm14\n"
        "roundsd $1, %xmm15, %xmm13\n"
        "roundsd $3, 4660(%r13), %xmm5\n"
        "blendps $4, %xmm7, %xmm3\n"
        "blendps $3, -129(%rdi), %xmm8\n"
        "blendpd $1, %xmm14, %xmm11\n"
        "blendpd $3, %fs:0x0, %xmm3\n"
        "pblendw $27, %xmm0, %xmm10\n"
        "pblendw $3, 0x10(,%rcx,8), %xmm4\n"
        "palignr $1, %xmm9, %xmm6\n"
        "palignr $3, (%rbp), %xmm6\n"
        "dpps $1, %xmm6, %xmm8\n"
        "dpps $3, 16(%edi), %xmm4\n"
        "pclmulqdq $0, %xmm7, %xmm8\n"
        "pclmulqdq $3, (%r12), %xmm11\n"
        "insertps $4, %xmm7, %xmm10\n"
        "insertps $3, (%rbp), %xmm12\n"
        "cmpeqps %xmm6, %xmm2\n"
        "cmpeqpd %xmm14, %xmm9\n"
        "cmpeqss %xmm10, %xmm5\n"
        "cmpeqsd %xmm15, %xmm3\n"
        "cmpltps %xmm0, %xmm3\n"
        "cmpltpd %xmm11, %xmm10\n"
        "cmpltss %xmm13, %xmm10\n"
        "cmpltsd %xmm8, %xmm5\n"
        "cmpleps %xmm3, %xmm5\n"
        "cmplepd %xmm10, %xmm7\n"
        "cmpless %xmm9, %xmm9\n"
        "cmplesd %xmm12, %xmm10\n"
        "cmpunordps %xmm14, %xmm7\n"
        "cmpunordpd %xmm3, %xmm11\n"
        "cmpunordss %xmm10, %xmm3\n"
        "cmpunordsd %xmm2, %xmm6\n"
        "cmpneqps %xmm10, %xmm8\n"
        "cmpneqpd %xmm2, %xmm14\n"
        "cmpneqss %xmm8, %xmm9\n"
        "cmpneqsd %xmm2, %xmm3\n"
        "cmpnltps %xmm5, %xmm0\n"
        "cmpnltpd %xmm3, %xmm14\n"
        "cmpnltss %xmm7, %xmm14\n"
        "cmpnltsd %xmm3, %xmm11\n"
        "cmpnleps %xmm15, %xmm8\n"
        "cmpnlepd %xmm2, %xmm8\n"
        "cmpnless %xmm4, %xmm10\n"
        "cmpnlesd %xmm13, %xmm12\n"
        "cmpordps %xmm6, %xmm15\n"
        "cmpordpd %xmm9, %xmm13\n"
        "cmpordss %xmm3, %xmm13\n"
        "cmpordsd %xmm12, %xmm2\n"
        "punpcklbw %xmm0, %xmm15\n"
        "punpcklbw 0x0(,%rax,4), %xmm5\n"
        "punpcklwd %xmm10, %xmm4\n"
        "punpcklwd (%r13), %xmm8\n"
        "punpckldq %xmm0, %xmm11\n"
        "punpckldq %gs:0x28, %xmm13\n"
        "packsswb %xmm0, %xmm7\n"
        "packsswb -305419896(%r9), %xmm0\n"
        "pcmpgtb %xmm9, %xmm15\n"
        "pcmpgtb 0x1234, %xmm3\n"
        "pcmpgtw %xmm11, %xmm11\n"
        "pcmpgtw 16(%ebx), %xmm4\n"
        "pcmpgtd %xmm12, %xmm11\n"
        "pcmpgtd (%rbp), %xmm5\n"
        "packuswb %xmm9, %xmm5\n"
        "packuswb (%rcx,%rdx,1), %xmm6\n"
        "punpckhbw %xmm0, %xmm8\n"
        "punpckhbw (%rsp), %xmm3\n"
        "punpckhwd %xmm8, %xmm13\n"
        "punpckhwd %fs:0x0, %xmm3\n"
        "punpckhdq %xmm1, %xmm15\n"
        "punpckhdq (%r15,%r15,4), %xmm12\n"
        "packssdw %xmm15, %xmm3\n"
        "packssdw 4(%r12,%rsi,4), %xmm9\n"
        "punpcklqdq %xmm14, %xmm0\n"
        "punpcklqdq (%r15), %xmm5\n"
        "punpckhqdq %xmm15, %xmm14\n"
        "punpckhqdq (%r12), %xmm7\n"
        "pcmpeqb %xmm1, %xmm2\n"
        "pcmpeqb (%rbp), %xmm15\n"
        "pcmpeqw %xmm8, %xmm7\n"
        "pcmpeqw 0x7fffffff, %xmm7\n"
        "pcmpeqd %xmm3, %xmm6\n"
        "pcmpeqd -305419896(%rcx), %xmm9\n"
        "paddq %xmm3, %xmm8\n"
        "paddq 0(%r8d), %xmm2\n"
        "pmullw %xmm7, %xmm12\n"
        "pmullw 0x10(,%rdx,8), %xmm1\n"
        "psubusb %xmm13, %xmm2\n"
        "psubusb 4(%r8,%r14,4), %xmm9\n"
        "psubusw %xmm4, %xmm5\n"
        "psubusw (%rcx,%rcx,2), %xmm14\n"
        "pminub %xmm15, %xmm4\n"
        "pminub 0(%r13d), %xmm6\n"
        "pand %xmm4, %xmm7\n"
        "pand %gs:0x28, %xmm2\n"
        "paddusb %xmm12, %xmm3\n"
        "paddusb (%r8,%rcx,4), %xmm12\n"
        "paddusw %xmm15, %xmm14\n"
        "paddusw 0x10(,%rsi,1), %xmm15\n"
        "pmaxub %xmm3, %xmm11\n"
        "pmaxub 16(%r12d), %xmm2\n"
        "pandn %xmm12, %xmm6\n"
        "pandn -128(%r8), %xmm15\n"
        "pavgb %xmm7, %xmm4\n"
        "pavgb %gs:0x1000, %xmm0\n"
        "pavgw %xmm10, %xmm5\n"
        "pavgw (%rdx,%rsi,2), %xmm0\n"
        "pmulhuw %xmm13, %xmm2\n"
        "pmulhuw (%rsp), %xmm9\n"
        "pmulhw %xmm11, %xmm10\n"
        "pmulhw 0x10(,%rdi,8), %xmm2\n"
        "psubsb %xmm3, %xmm10\n"
        "psubsb 1(%rcx), %xmm14\n"
        "psubsw %xmm5, %xmm4\n"
        "psubsw %fs:0x0, %xmm3\n"
        "pminsw %xmm6, %xmm4\n"
        "pminsw (%rbp), %xmm13\n"
        "por %xmm14, %xmm3\n"
        "por 0x7fffffff, %xmm15\n"
        "paddsb %xmm2, %xmm11\n"
        "paddsb (%rsp), %xmm2\n"
        "paddsw %xmm0, %xmm7\n"
        "paddsw (%r13), %xmm8\n"
        "pmaxsw %xmm9, %xmm9\n"
        "pmaxsw 8(%rsp), %xmm2\n"
        "pxor %xmm13, %xmm6\n"
        "pxor (%r8), %xmm8\n"
        "pmuludq %xmm0, %xmm5\n"
        "pmuludq (%rbx,%rdi,1), %xmm0\n"
        "pmaddwd %xmm0, %xmm14\n"
        "pmaddwd 0x0(,%rsi,4), %xmm14\n"
        "psadbw %xmm11, %xmm5\n"
        "psadbw 0x10(,%r11,1), %xmm7\n"
        "psubb %xmm9, %xmm13\n"
        "psubb (%r15), %xmm2\n"
        "psubw %xmm7, %xmm1\n"
        "psubw 0x1234, %xmm7\n"
        "psubd %xmm2, %xmm3\n"
        "psubd 0x1000(,%rbp,8), %xmm14\n"
        "psubq %xmm15, %xmm6\n"
        "psubq 8(%rdx), %xmm12\n"
        "paddb %xmm11, %xmm0\n"
        "paddb 100000(%r13,%r12,8), %xmm0\n"
        "paddw %xmm6, %xmm2\n"
        "paddw (%r12), %xmm14\n"
        "paddd %xmm5, %xmm12\n"
        "paddd (%r13), %xmm10\n"
        "pshufb %xmm5, %xmm15\n"
        "pshufb (%r9), %xmm0\n"
        "phaddw %xmm8, %xmm6\n"
        "phaddw (%rax), %xmm9\n"
        "phaddd %xmm12, %xmm3\n"
        "phaddd 16(%ecx), %xmm2\n"
        "pmaddubsw %xmm12, %xmm4\n"
        "pmaddubsw (%rbx,%rbp,4), %xmm1\n"
        "psignb %xmm7, %xmm14\n"
        "psignb (%r15), %xmm14\n"
        "pmulhrsw %xmm5, %xmm4\n"
        "pmulhrsw (%r12), %xmm9\n"
        "pabsb %xmm10, %xmm4\n"
        "pabsb (%rbp), %xmm8\n"
        "pabsw %xmm3, %xmm5\n"
        "pabsw %gs:0x0, %xmm11\n"
        "pabsd %xmm8, %xmm0\n"
        "pabsd -8(%rdx,%r14,1), %xmm6\n"
        "pmovsxbw %xmm3, %xmm8\n"
        "pmovsxbw 0x10, %xmm3\n"
        "pmovzxbd %xmm4, %xmm4\n"
        "pmovzxbd 4(%rdi,%rbp,8), %xmm10\n"
        "pmovzxdq %xmm4, %xmm9\n"
        "pmovzxdq 0x10(,%rdi,8), %xmm12\n"
        "pmuldq %xmm3, %xmm14\n"
        "pmuldq %fs:0x28, %xmm1\n"
        "pcmpeqq %xmm5, %xmm7\n"
        "pcmpeqq (%rsp), %xmm8\n"
        "packusdw %xmm1, %xmm2\n"
        "packusdw (%r13), %xmm8\n"
        "pcmpgtq %xmm2, %xmm9\n"
        "pcmpgtq (%r13,%rdi,8), %xmm7\n"
        "pminsb %xmm5, %xmm0\n"
        "pminsb 1024(%r13,%rdi,4), %xmm0\n"
        "pminsd %xmm9, %xmm15\n"
        "pminsd -8(%rsp,%r14,2), %xmm9\n"
        "pminuw %xmm8, %xmm10\n"
        "pminuw %fs:0x28, %xmm10\n"
        "pminud %xmm12, %xmm8\n"
        "pminud (%r13), %xmm4\n"
        "pmaxsb %xmm7, %xmm2\n"
        "pmaxsb (%rbp), %xmm10\n"
        "pmaxsd %xmm1, %xmm10\n"
        "pmaxsd -129(%rsi), %xmm12\n"
        "pmaxuw %xmm1, %xmm12\n"
        "pmaxuw (%rbp), %xmm9\n"
        "pmaxud %xmm13, %xmm0\n"
        "pmaxud (%r12), %xmm1\n"
        "pmulld %xmm3, %xmm0\n"
        "pmulld (%r11,%rdi,2), %xmm5\n"
        "ptest %xmm6, %xmm5\n"
        "ptest (%rsp), %xmm11\n"
        "phminposuw %xmm14, %xmm11\n"
        "phminposuw (%r11,%r14,4), %xmm4\n"
        "aesenc %xmm11, %xmm0\n"
        "aesenc 0(%edi), %xmm8\n"
        "aesenclast %xmm10, %xmm12\n"
        "aesenclast (%r13), %xmm11\n"
        "aesdec %xmm15, %xmm2\n"
        "aesdec 16(%r13d), %xmm5\n"
        "aesdeclast %xmm5, %xmm4\n"
        "aesdeclast (%r8,%r13,1), %xmm8\n"
        "aesimc %xmm10, %xmm10\n"
        "aesimc 1024(%r14,%rbp,8), %xmm6\n"
        "haddps %xmm12, %xmm8\n"
        "haddps (%rbp), %xmm15\n"
        "hsubpd %xmm2, %xmm12\n"
        "hsubpd %fs:0x28, %xmm15\n"
        "addsubps %xmm2, %xmm1\n"
        "addsubps 0x1000(,%r10,1), %xmm1\n"
        "psrlw $1, %xmm1\n"
        "psrlw %xmm15, %xmm12\n"
        "psrlw (%rbp), %xmm1\n"
        "psrld $7, %xmm8\n"
        "psrld %xmm14, %xmm7\n"
        "psrld -129(%rbx), %xmm7\n"
        "psrlq $31, %xmm8\n"
        "psrlq %xmm12, %xmm11\n"
        "psrlq (%rbp), %xmm2\n"
        "psraw $7, %xmm12\n"
        "psraw %xmm6, %xmm14\n"
        "psraw (%rsp), %xmm2\n"
        "psrad $1, %xmm14\n"
        "psrad %xmm11, %xmm1\n"
        "psrad 1(%rsi), %xmm15\n"
        "psllw $1, %xmm15\n"
        "psllw %xmm5, %xmm13\n"
        "psllw 127(%r8), %xmm6\n"
        "pslld $7, %xmm8\n"
        "pslld %xmm11, %xmm7\n"
        "pslld 0x7fffffff, %xmm3\n"
        "psllq $31, %xmm6\n"
        "psllq %xmm0, %xmm15\n"
        "psllq 100000(%rdi,%rbp,1), %xmm13\n"
        "cvtsi2ss %r10, %xmm1\n"
        "cvtsi2ssl (%rax), %xmm13\n"
        "cvtsi2ssq 8(%rbx), %xmm7\n"
        "cvtsi2sd %eax, %xmm15\n"
        "cvtsi2sd %r10, %xmm10\n"
        "cvtsi2sdl (%rax), %xmm1\n"
        "cvtsi2sdq 8(%rbx), %xmm6\n"
        "cvttss2si %xmm3, %eax\n"
        "cvttss2si %xmm1, %r11\n"
        "cvttsd2si %xmm1, %eax\n"
        "cvttsd2si %xmm15, %r11\n"
        "cvtss2si %xmm6, %eax\n"
        "cvtss2si %xmm3, %r11\n"
        "cvtsd2si %xmm10, %eax\n"
        "cvtsd2si %xmm11, %r11\n"
        "add %dil, %spl\n"
        "addb %r11b, 0(%eax)\n"
        "add $255, %r15b\n"
        "addb $-128, (%rdi,%r11,2)\n"
        "add $255, %al\n"
        "add %r12w, %ax\n"
        "addw %dx, (%rcx,%rbp,1)\n"
        "add 0x1000(,%r10,8), %r13w\n"
        "add $128, %r14w\n"
        "addw $1, %fs:0x28\n"
        "add %r8d, %r13d\n"
        "addl %r9d, 0x1234\n"
        "add (%rsp), %r10d\n"
        "addl $127, 16(%ebx)\n"
        "add %rsi, %r8\n"
        "addq %r9, -129(%rdx)\n"
        "add -1(%r11), %r13\n"
        "add $1, %rax\n"
        "addq $127, 0x10\n"
        "or %cl, %r12b\n"
        "orb %r10b, (%rsp)\n"
        "or (%r9), %dl\n"
        "or $0, %cl\n"
        "orb $1, 0x1234\n"
        "or $1, %al\n"
        "or %cx, %r10w\n"
        "orw %r10w, %gs:0x28\n"
        "or (%r13), %bx\n"
        "or $32767, %r8w\n"
        "orw $128, 1024(%r9,%rdi,4)\n"
        "or $32767, %ax\n"
        "or %r10d, %eax\n"
        "orl %r13d, 16(%esp)\n"
        "or (%r10), %r11d\n"
        "or $-1, %r11d\n"
        "orl $305419896, 0x1000(,%rax,1)\n"
        "or %rax, %r11\n"
        "orq %r8, (%r13)\n"
        "or (%r11,%rdx,4), %r11\n"
        "or $-2147483648, %r8\n"
        "orq $127, 0x10\n"
        "or $305419896, %rax\n"
        "adc %spl, %r9b\n"
        "adcb %dil, 0x0(,%rsi,8)\n"
        "adc 127(%rbx), %r10b\n"
        "adc $1, %r14b\n"
        "adcb $1, -128(%r10)\n"
        "adc %r14w, %r8w\n"
        "adcw %di, -128(%rcx)\n"
        "adc %gs:0x28, %dx\n"
        "adc $32767, %r11w\n"
        "adcw $32767, (%r13,%rbp,4)\n"
        "adc $128, %ax\n"
        "adc %r11d, %r13d\n"
        "adcl %r9d, 0x1234\n"
        "adc (%rsi,%rax,8), %r13d\n"
        "adc $305419896, %edi\n"
        "adcl $0, (%rbp)\n"
        "adc $1, %eax\n"
        "adc %rdx, %r9\n"
        "adcq %rbx, -8(%rcx,%rax,8)\n"
        "adc 16(%ebx), %rbp\n"
        "adc $305419896, %r9\n"
        "adcq $1, (%r13)\n"
        "adc $305419896, %rax\n"
        "sbb %bl, %r10b\n"
        "sbbb %spl, 0x0(,%r13,4)\n"
        "sbb 4(%rsi,%rcx,2), %dil\n"
        "sbb $127, %spl\n"
        "sbbb $0, (%r13)\n"
        "sbb $1, %al\n"
        "sbb %r8w, %r13w\n"
        "sbbw %cx, (%r13)\n"
        "sbb (%rsp), %bx\n"
        "sbb $0, %dx\n"
        "sbbw $128, (%rdx)\n"
        "sbb %r15d, %r10d\n"
        "sbbl %ebp, %gs:0x1000\n"
        "sbb 0x1234, %esi\n"
        "sbb $127, %r13d\n"
        "sbbl $0, (%rax,%r12,8)\n"
        "sbb %rbp, %rcx\n"
        "sbbq %r11, (%r13)\n"
        "sbb 16(%r15d), %r10\n"
        "sbb $128, %r13\n"
        "sbbq $128, (%rdi)\n"
        "sbb $305419896, %rax\n"
        "and %r8b, %dl\n"
        "andb %r13b, 4(%r13,%rdx,4)\n"
        "and %gs:0x1000, %bl\n"
        "and $255, %r12b\n"
        "andb $0, %fs:0x28\n"
        "and $1, %al\n"
        "and %di, %r12w\n"
        "andw %cx, (%rsp)\n"
        "and 0x10, %r10w\n"
        "andw $0, (%rbp)\n"
        "and %ecx, %edi\n"
        "andl %ebx, (%r12)\n"
        "and (%r12), %edx\n"
        "and $1, %ebp\n"
        "andl $-1, 100000(%r14,%r10,4)\n"
        "and %r12, %r11\n"
        "andq %r13, -128(%r12)\n"
        "and 0x7fffffff, %r15\n"
        "andq $1, (%rbx,%r8,8)\n"
        "and $-1, %rax\n"
        "sub %r14b, %bpl\n"
        "subb %spl, 0x1000(,%rdx,4)\n"
        "sub 100000(%r9,%r11,4), %sil\n"
        "sub $127, %al\n"
        "subb $127, (%rbp)\n"
        "sub %di, %r10w\n"
        "subw %r15w, (%r13,%r12,8)\n"
        "sub 0(%r15d), %dx\n"
        "sub $128, %cx\n"
        "subw $128, 4(%rdi,%r11,2)\n"
        "sub $127, %ax\n"
        "sub %edi, %esi\n"
        "subl %r8d, (%rbp,%r10,1)\n"
        "sub 0x0(,%r14,4), %ebp\n"
        "sub $128, %edx\n"
        "subl $-1, 127(%rbx)\n"
        "sub $127, %eax\n"
        "sub %rcx, %r11\n"
        "subq %r14, 0(%r10d)\n"
        "sub %gs:0x0, %rsi\n"
        "sub $-1, %r15\n"
        "subq $128, (%r10,%rcx,4)\n"
        "sub $0, %rax\n"
        "xor %r13b, %bl\n"
        "xorb %r14b, (%r12)\n"
        "xor %gs:0x28, %r8b\n"
        "xor $0, %r10b\n"
        "xorb $255, (%rsp)\n"
        "xor $127, %al\n"
        "xor %cx, %r9w\n"
        "xorw %bp, (%rbp,%rsi,8)\n"
        "xor 4660(%rbx), %sp\n"
        "xor $127, %r13w\n"
        "xorw $32767, 0x10(,%r8,4)\n"
        "xor $1, %ax\n"
        "xor %edx, %ebx\n"
        "xorl %ebp, (%rsp)\n"
        "xor 0x0(,%rax,1), %r15d\n"
        "xor $305419896, %r8d\n"
        "xorl $1, (%rsp)\n"
        "xor %r14, %rdx\n"
        "xorq %r11, (%rbp)\n"
        "xor -1(%rbx), %rbp\n"
        "xor $-1, %rsi\n"
        "xorq $305419896, 0x7fffffff\n"
        "xor $1, %rax\n"
        "cmp %spl, %r12b\n"
        "cmpb %sil, (%rsp)\n"
        "cmp 0x10(,%r13,1), %r14b\n"
        "cmp $-128, %r12b\n"
        "cmpb $127, (%rsp)\n"
        "cmp %r9w, %r15w\n"
        "cmpw %ax, 0(%ebx)\n"
        "cmp 100000(%r15,%rdx,2), %si\n"
        "cmp $32767, %si\n"
        "cmpw $0, (%r12)\n"
        "cmp %esp, %esp\n"
        "cmpl %r14d, 127(%rcx)\n"
        "cmp 0(%r15d), %eax\n"
        "cmp $127, %r10d\n"
        "cmpl $127, %fs:0x1000\n"
        "cmp %r10, %rsi\n"
        "cmpq %rdx, -8(%r13,%r12,8)\n"
        "cmp %fs:0x28, %rdx\n"
        "cmp $-2147483648, %rsi\n"
        "cmpq $-1, (%r12,%r8,8)\n"
        "cmp $-1, %rax\n"
        "test %dl, %r13b\n"
        "test %sil, (%rbp)\n"
        "test $-128, %r14b\n"
        "testb $1, (%rbp)\n"
        "mov %r11b, %r14b\n"
        "mov %r11b, 0(%esi)\n"
        "mov 0x10(,%rdx,2), %bpl\n"
        "mov $1, %al\n"
        "movb $1, 0(%eax)\n"
        "not %spl\n"
        "notb -129(%rbp)\n"
        "neg %bpl\n"
        "negb (%rax)\n"
        "mul %r14b\n"
        "mulb %fs:0x0\n"
        "imulb (%r12)\n"
        "div %al\n"
        "divb 100000(%rdi,%rcx,8)\n"
        "idivb (%r10,%r10,1)\n"
        "inc %r11b\n"
        "incb 127(%rcx)\n"
        "dec %r8b\n"
        "decb (%rbp)\n"
        "rol %r8b\n"
        "rol $1, %r10b\n"
        "rol %cl, %r11b\n"
        "rolb $2, 128(%rcx)\n"
        "ror %dl\n"
        "ror $7, %al\n"
        "rorb $1, 4660(%rax)\n"
        "rcl %cl\n"
        "rcl $3, %cl\n"
        "rcl %cl, %sil\n"
        "rclb $5, (%r12)\n"
        "rcr %r15b\n"
        "rcr $3, %r15b\n"
        "rcr %cl, %r11b\n"
        "rcrb $5, (%r12)\n"
        "shl %r12b\n"
        "shl $1, %r9b\n"
        "shl %cl, %bpl\n"
        "shlb $2, 127(%r12)\n"
        "shr %r12b\n"
        "shr $1, %r12b\n"
        "shr %cl, %r11b\n"
        "shrb $1, %fs:0x28\n"
        "sar %r15b\n"
        "sar $3, %r8b\n"
        "sar %cl, %bpl\n"
        "sarb $5, 0(%esp)\n"
        "sal %bpl\n"
        "sal $7, %al\n"
        "sal %cl, %r14b\n"
        "salb $1, -305419896(%r10)\n"
        "test %cx, %cx\n"
        "test %r14w, (%rbp)\n"
        "test $1, %r12w\n"
        "testw $0, %gs:0x0\n"
        "mov %cx, (%rsp,%r10,1)\n"
        "mov -128(%rdi), %di\n"
        "mov $32767, %r13w\n"
        "movw $32767, 0(%r14d)\n"
        "not %bp\n"
        "notw 0(%eax)\n"
        "neg %bx\n"
        "negw 4(%r8,%rcx,8)\n"
        "mulw 100000(%rbx,%r11,4)\n"
        "imul %r12w\n"
        "imulw 0(%r9d)\n"
        "div %r14w\n"
        "idiv %ax\n"
        "idivw (%r13)\n"
        "inc %r10w\n"
        "incw %fs:0x1000\n"
        "dec %r11w\n"
        "decw 0x7fffffff\n"
        "rol %si\n"
        "rol $1, %r11w\n"
        "rol %cl, %cx\n"
        "rolw $2, 4(%r13,%r8,4)\n"
        "ror %si\n"
        "ror $3, %bp\n"
        "ror %cl, %si\n"
        "rorw $5, 100000(%r14,%r15,4)\n"
        "rcl %si\n"
        "rcl $7, %r13w\n"
        "rcl %cl, %r15w\n"
        "rclw $2, (%r13)\n"
        "rcr %ax\n"
        "rcr $1, %bx\n"
        "rcr %cl, %ax\n"
        "rcrw $5, (%r9,%r9,1)\n"
        "shl $1, %si\n"
        "shl %cl, %si\n"
        "shlw $2, (%rbp)\n"
        "shr %si\n"
        "shr $7, %r9w\n"
        "shr %cl, %sp\n"
        "shrw $1, (%rbp)\n"
        "sar %dx\n"
        "sar $1, %sp\n"
        "sar %cl, %r15w\n"
        "sarw $5, 0x1000(,%r13,4)\n"
        "sal %bp\n"
        "sal $3, %r9w\n"
        "sal %cl, %r14w\n"
        "salw $2, (%r12)\n"
        "test %r9d, %r9d\n"
        "test %eax, (%rbp)\n"
        "test $127, %edi\n"
        "testl $305419896, (%rax)\n"
        "mov %esp, %eax\n"
        "mov %ebp, (%rax)\n"
        "mov (%rbp), %ecx\n"
        "mov $128, %ebp\n"
        "movl $1, (%r8)\n"
        "not %r10d\n"
        "notl (%rbx)\n"
        "neg %r9d\n"
        "negl 0x10(,%r11,8)\n"
        "mul %r8d\n"
        "mull %gs:0x0\n"
        "imul %ebp\n"
        "imull 16(%r10d)\n"
        "div %esp\n"
        "divl 0x0(,%rdx,1)\n"
        "idiv %r15d\n"
        "idivl (%r14)\n"
        "inc %r13d\n"
        "incl %fs:0x0\n"
        "dec %esp\n"
        "decl (%r12)\n"
        "rol %r14d\n"
        "rol $3, %r8d\n"
        "rol %cl, %r8d\n"
        "roll $2, 0x10(,%r10,2)\n"
        "ror %ecx\n"
        "ror $3, %r15d\n"
        "ror %cl, %edi\n"
        "rorl $2, 16(%ebx)\n"
        "rcl %r15d\n"
        "rcl $1, %r8d\n"
        "rcl %cl, %r8d\n"
        "rcll $1, 0x10\n"
        "rcr $7, %r14d\n"
        "rcr %cl, %esi\n"
        "rcrl $2, %gs:0x28\n"
        "shl %r10d\n"
        "shl %cl, %esi\n"
        "shll $1, %gs:0x1000\n"
        "shr %r9d\n"
        "shr $1, %esp\n"
        "shr %cl, %r13d\n"
        "shrl $2, 0x10\n"
        "sar %esi\n"
        "sar $7, %edx\n"
        "sar %cl, %ebx\n"
        "sarl $5, -1(%rax)\n"
        "sal %r13d\n"
        "sal $3, %r15d\n"
        "sal %cl, %r14d\n"
        "sall $2, 0x0(,%rdx,8)\n"
        "test %rbp, %rsp\n"
        "test %rsp, (%r14,%r11,8)\n"
        "test $-1, %rax\n"
        "testq $1, 0x7fffffff\n"
        "mov %rbp, %r9\n"
        "mov %rsi, (%rsp,%rax,2)\n"
        "mov (%r12), %r13\n"
        "mov $128, %rbp\n"
        "movq $0, 0x0(,%rdx,2)\n"
        "not %r9\n"
        "notq (%r12,%rsi,1)\n"
        "negq (%r11)\n"
        "mul %r15\n"
        "mulq 100000(%rdx,%rsi,1)\n"
        "imul %r10\n"
        "imulq (%rbp)\n"
        "div %r14\n"
        "divq 16(%r14d)\n"
        "idivq (%rsp)\n"
        "inc %r9\n"
        "incq (%r9,%r14,2)\n"
        "decq 0x0(,%rdx,1)\n"
        "rol %r13\n"
        "rol $7, %rbp\n"
        "rol %cl, %rbx\n"
        "rolq $5, (%rbp)\n"
        "ror %rbx\n"
        "ror $3, %r11\n"
        "ror %cl, %r8\n"
        "rorq $1, (%rbp)\n"
        "rcl %rsi\n"
        "rcl $3, %r14\n"
        "rcl %cl, %r11\n"
        "rclq $5, %gs:0x1000\n"
        "rcr %r10\n"
        "rcr $3, %r9\n"
        "rcrq $1, 127(%rdi)\n"
        "shl %rdi\n"
        "shl $3, %r13\n"
        "shl %cl, %r8\n"
        "shlq $2, (%r15,%rsi,2)\n"
        "shr %r14\n"
        "shr $7, %rcx\n"
        "shr %cl, %rdx\n"
        "shrq $2, 0x10\n"
        "sar %r12\n"
        "sar $3, %rsi\n"
        "sar %cl, %rdx\n"
        "sarq $2, (%r12)\n"
        "sal %rbx\n"
        "sal $3, %r12\n"
        "sal %cl, %r11\n"
        "salq $1, 0x10(,%r9,8)\n"
        "imul %r14w, %r11w\n"
        "imul 127(%r15), %di\n"
        "imul $0, %r11w, %dx\n"
        "imul $1000, 0(%esi), %r10w\n"
        "lea 0x10, %ax\n"
        "cmovnc %r10w, %r14w\n"
        "cmove %gs:0x1000, %bx\n"
        "bt %r12w, %di\n"
        "bt %r15w, (%r9,%rbp,2)\n"
        "bt $1, %bp\n"
        "btw $3, 0(%r11d)\n"
        "bts %r12w, %ax\n"
        "bts %r15w, (%r15,%rbp,2)\n"
        "bts $31, %si\n"
        "btsw $3, (%r12)\n"
        "btr %r9w, %r13w\n"
        "btr %r9w, -129(%rdx)\n"
        "btr $31, %dx\n"
        "btrw $3, (%r10)\n"
        "btc %r13w, %bx\n"
        "btc %r10w, 0x1234\n"
        "btc $1, %r12w\n"
        "btcw $3, 4660(%rcx)\n"
        "bsf %ax, %di\n"
        "bsf 0(%r15d), %r12w\n"
        "bsr %r9w, %r11w\n"
        "bsr (%r13), %r10w\n"
        "tzcnt %sp, %r11w\n"
        "tzcnt 0(%r14d), %cx\n"
        "lzcnt %r13w, %r9w\n"
        "lzcnt -8(%r15,%r14,2), %bx\n"
        "popcnt %r14w, %bx\n"
        "popcnt (%rsp), %bp\n"
        "shld $3, %cx, %r10w\n"
        "shrd %cl, %r15w, 4(%r12,%r15,2)\n"
        "shld %r12w, %r11w\n"
        "xadd %r12w, 1024(%rbp,%rbp,4)\n"
        "cmpxchg %r8w, (%r10)\n"
        "lock cmpxchg %bx, 100000(%rdi,%rbp,2)\n"
        "lock xadd %bp, -8(%r15,%r12,2)\n"
        "xchg %r11w, %di\n"
        "xchg %bp, (%r12)\n"
        "movbe 16(%r14d), %r11w\n"
        "movbe %cx, 1(%r14)\n"
        "imul %ebp, %r8d\n"
        "imul (%rsp), %r12d\n"
        "imul $-128, %r15d, %r11d\n"
        "imul $1000, 0x10, %ebx\n"
        "lea 0x7fffffff, %esp\n"
        "cmovz %r10d, %ebx\n"
        "cmove %gs:0x28, %esp\n"
        "bt %r14d, %r11d\n"
        "bt %r15d, (%r11)\n"
        "bt $1, %edx\n"
        "btl $3, (%r12)\n"
        "bts %r9d, %r10d\n"
        "bts %r13d, 0x1000(,%rdi,4)\n"
        "bts $31, %r15d\n"
        "btsl $3, %gs:0x1000\n"
        "btr %ebx, %gs:0x0\n"
        "btr $31, %r14d\n"
        "btrl $3, 1024(%r13,%r9,1)\n"
        "btc %ecx, %r15d\n"
        "btc %esi, (%rbp,%rcx,1)\n"
        "btc $1, %r12d\n"
        "btcl $3, (%rsp)\n"
        "bsf %r15d, %r8d\n"
        "bsf %fs:0x0, %ebx\n"
        "bsr %r15d, %r11d\n"
        "bsr 0x1234, %ebp\n"
        "tzcnt %r8d, %esi\n"
        "tzcnt 1024(%rdi,%rbp,4), %ebx\n"
        "lzcnt %ebx, %esp\n"
        "lzcnt 127(%rdi), %ebx\n"
        "popcnt %r10d, %esp\n"
        "popcnt (%rcx,%r11,8), %r8d\n"
        "shld $3, %esp, %r11d\n"
        "shrd %cl, %r14d, 0x1234\n"
        "shld %r8d, %r9d\n"
        "xadd %r8d, 0(%ecx)\n"
        "cmpxchg %ebp, 0x10\n"
        "lock cmpxchg %esi, %gs:0x0\n"
        "lock xadd %ebx, (%rsi,%r12,1)\n"
        "xchg %r15d, %r13d\n"
        "xchg %r9d, (%rbx,%r9,4)\n"
        "movbe 0(%edx), %edx\n"
        "movbe %esp, -1(%rax)\n"
        "imul %r12, %r14\n"
        "imul (%rsp), %rax\n"
        "imul $0, %r9, %r13\n"
        "imul $1000, (%rax,%rax,4), %r10\n"
        "lea %fs:0x28, %rax\n"
        "cmovnc %r14, %r10\n"
        "cmove -8(%r14,%r8,1), %r11\n"
        "bt %rsi, %rdi\n"
        "bt %rbp, 0x1234\n"
        "bt $31, %rbx\n"
        "btq $3, (%rsp)\n"
        "bts %r12, %rax\n"
        "bts %r10, 1024(%rsi,%r8,8)\n"
        "bts $1, %r13\n"
        "btsq $3, 0x10(,%rdx,1)\n"
        "btr %r9, %rsp\n"
        "btr %r14, (%r12)\n"
        "btr $1, %rdi\n"
        "btrq $3, %fs:0x28\n"
        "btc %rdx, %r15\n"
        "btc %rdi, 0x10\n"
        "btc $31, %rax\n"
        "btcq $3, (%r15)\n"
        "bsf %rax, %rcx\n"
        "bsf %gs:0x28, %r9\n"
        "bsr %r9, %rsp\n"
        "bsr 0x1000(,%rdi,1), %rbx\n"
        "tzcnt %rsi, %rdi\n"
        "tzcnt 128(%rbp), %rsp\n"
        "lzcnt %rbp, %rdx\n"
        "lzcnt 0x0(,%r8,4), %r15\n"
        "popcnt %rdx, %r13\n"
        "popcnt %gs:0x28, %rax\n"
        "shld $3, %r10, %r9\n"
        "shrd %cl, %r15, 0x10(,%r12,2)\n"
        "shld %r11, %rbx\n"
        "xadd %r8, (%r13)\n"
        "cmpxchg %r10, 0(%ecx)\n"
        "lock cmpxchg %r12, 1024(%r12,%rdx,1)\n"
        "lock xadd %rbp, (%rsp)\n"
        "xchg %r13, %r10\n"
        "xchg %r12, 0x7fffffff\n"
        "movbe 0x7fffffff, %r12\n"
        "movbe %r8, (%rsp)\n"
        "bswap %esi\n"
        "crc32l (%r13), %ebx\n"
        "rdrand %r12d\n"
        "rdseed %r15d\n"
        "adcx %r15d, %r12d\n"
        "adox (%rsp), %r12d\n"
        "andn %ecx, %ebp, %eax\n"
        "andn -8(%rbx,%r14,8), %ebx, %ebx\n"
        "bextr %ebx, %esi, %r9d\n"
        "bextr %esp, 0x10, %esi\n"
        "bzhi %r13d, %r15d, %r10d\n"
        "bzhi %r14d, (%r12), %r10d\n"
        "sarx %r10d, %r12d, %esi\n"
        "sarx %ebx, 0x0(,%rcx,8), %edi\n"
        "shlx %edi, %r10d, %edi\n"
        "shlx %edx, %fs:0x1000, %esi\n"
        "shrx %r12d, %eax, %r9d\n"
        "shrx %edi, (%r8,%r14,1), %r8d\n"
        "pdep %r11d, %esi, %r9d\n"
        "pext %esp, %r11d, %esi\n"
        "mulx %esi, %ebx, %r14d\n"
        "blsr %r13d, %r8d\n"
        "blsmsk %ecx, %r12d\n"
        "blsi %r8d, %r9d\n"
        "rorx $5, %esi, %r14d\n"
        "movnti %esi, 0x0(,%r11,1)\n"
        "bswap %r13\n"
        "crc32q %fs:0x1000, %rbx\n"
        "rdrand %rax\n"
        "rdseed %rbp\n"
        "adcx %r10, %r11\n"
        "adox 16(%ecx), %rbx\n"
        "andn %r10, %rdx, %r13\n"
        "andn %gs:0x28, %r14, %rbp\n"
        "bextr %r15, %rax, %rbx\n"
        "bextr %rbp, %gs:0x1000, %rsi\n"
        "bzhi %r14, %r12, %r15\n"
        "bzhi %r11, %gs:0x28, %r15\n"
        "sarx %r14, %rax, %r15\n"
        "sarx %rdx, 0x1000(,%rbx,1), %rbp\n"
        "shlx %rbp, %r8, %r10\n"
        "shlx %rdx, %fs:0x28, %rax\n"
        "shrx %rcx, %r12, %rbp\n"
        "shrx %rbx, (%r12), %rax\n"
        "pdep %r14, %r13, %rdx\n"
        "pext %rbx, %rdx, %rdx\n"
        "mulx %rsi, %rbx, %r8\n"
        "blsr %rdx, %r10\n"
        "blsmsk %rax, %rsi\n"
        "blsi %rcx, %r14\n"
        "rorx $5, %rcx, %rbx\n"
        "movnti %rsi, %gs:0x28\n"
        "movzbw %al, %si\n"
        "movzbw (%r12), %r13w\n"
        "movsbw %spl, %r11w\n"
        "movsbw (%rbp), %r9w\n"
        "movzbl %r9b, %edx\n"
        "movzbl 1024(%rbp,%r9,8), %r12d\n"
        "movsbl %dil, %r10d\n"
        "movsbl 0x0(,%rsi,4), %r11d\n"
        "movzbq %dl, %rbp\n"
        "movzbq 0x1234, %r12\n"
        "movsbq %r13b, %rsi\n"
        "movsbq -305419896(%r11), %r11\n"
        "movzwl %r14w, %ecx\n"
        "movzwl (%r13), %r15d\n"
        "movswl %si, %r9d\n"
        "movswl -129(%rdx), %r14d\n"
        "movzwq %r12w, %rbp\n"
        "movzwq -8(%r8,%r12,4), %rax\n"
        "movswq %bp, %rsi\n"
        "movswq 4(%r14,%r11,4), %r11\n"
        "movslq %esi, %rbx\n"
        "movslq (%r10), %r9\n"
        "seto %r15b\n"
        "seto 16(%r9d)\n"
        "setno %bpl\n"
        "setno (%r13)\n"
        "setb %r15b\n"
        "setb %fs:0x1000\n"
        "setae %cl\n"
        "setae %fs:0x0\n"
        "sete %cl\n"
        "sete (%rbp)\n"
        "setne %r11b\n"
        "setne 0(%eax)\n"
        "setbe %gs:0x1000\n"
        "seta %gs:0x0\n"
        "sets %r14b\n"
        "sets 0x1234\n"
        "setns %r12b\n"
        "setns (%rdx)\n"
        "setp %r8b\n"
        "setp 0x10\n"
        "setnp %al\n"
        "setnp (%rbx,%rdi,1)\n"
        "setl %r12b\n"
        "setl 0x10\n"
        "setge %r13b\n"
        "setge (%r8)\n"
        "setle %bpl\n"
        "setg %al\n"
        "setg -8(%r11,%r9,8)\n"
        "addps %xmm11, %xmm11\n"
        "addps 0x10(,%rbx,1), %xmm7\n"
        "addpd %xmm5, %xmm15\n"
        "addpd 0x1000(,%rdx,8), %xmm14\n"
        "addss %xmm5, %xmm12\n"
        "addss 0x1000(,%r13,8), %xmm2\n"
        "addsd %xmm6, %xmm12\n"
        "addsd -305419896(%rsi), %xmm11\n"
        "mulps %xmm11, %xmm8\n"
        "mulps 0x1000(,%r11,8), %xmm4\n"
        "mulpd %xmm11, %xmm2\n"
        "mulpd 0x1000(,%r15,4), %xmm6\n"
        "mulss %xmm6, %xmm12\n"
        "mulss -8(%rsi,%r9,4), %xmm8\n"
        "mulsd %xmm14, %xmm7\n"
        "mulsd -305419896(%rbx), %xmm0\n"
        "subps %xmm13, %xmm1\n"
        "subps 16(%esp), %xmm4\n"
        "subpd %xmm8, %xmm5\n"
        "subpd (%r13), %xmm5\n"
        "subss %xmm15, %xmm2\n"
        "subss 100000(%r11,%rax,1), %xmm13\n"
        "subsd %xmm13, %xmm3\n"
        "subsd 16(%r9d), %xmm11\n"
        "minps %xmm12, %xmm2\n"
        "minps (%r13), %xmm14\n"
        "minpd %xmm3, %xmm8\n"
        "minpd (%rsp), %xmm11\n"
        "minss %xmm8, %xmm7\n"
        "minss 16(%ecx), %xmm2\n"
        "minsd %xmm0, %xmm4\n"
        "minsd 4(%rdi,%r11,2), %xmm11\n"
        "divps %xmm5, %xmm2\n"
        "divps (%r12,%r9,8), %xmm0\n"
        "divpd %xmm3, %xmm10\n"
        "divpd 100000(%r12,%r13,2), %xmm3\n"
        "divss %xmm1, %xmm2\n"
        "divss -128(%r9), %xmm5\n"
        "divsd %xmm14, %xmm5\n"
        "divsd -129(%r11), %xmm4\n"
        "maxps %xmm4, %xmm7\n"
        "maxps 100000(%r15,%rbp,1), %xmm7\n"
        "maxpd %xmm9, %xmm11\n"
        "maxpd -305419896(%rbp), %xmm0\n"
        "maxss %xmm14, %xmm3\n"
        "maxss -8(%rbx,%r14,2), %xmm10\n"
        "maxsd %xmm6, %xmm13\n"
        "maxsd (%r9), %xmm14\n"
        "sqrtps %xmm7, %xmm14\n"
        "sqrtps -8(%rbx,%rdi,2), %xmm8\n"
        "sqrtpd %xmm3, %xmm4\n"
        "sqrtpd (%r13), %xmm3\n"
        "sqrtss %xmm7, %xmm13\n"
        "sqrtss (%rdx,%rcx,8), %xmm0\n"
        "sqrtsd %xmm5, %xmm7\n"
        "sqrtsd %gs:0x28, %xmm2\n"
        "movaps %xmm3, %xmm11\n"
        "movaps 16(%r10d), %xmm1\n"
        "movaps %xmm2, (%r8,%rcx,1)\n"
        "movapd %xmm9, %xmm15\n"
        "movapd (%r13), %xmm12\n"
        "movapd %xmm1, 0x1000(,%rdx,4)\n"
        "movups %xmm6, %xmm2\n"
        "movups %gs:0x0, %xmm12\n"
        "movups %xmm8, 4660(%rsi)\n"
        "movupd %xmm2, %xmm12\n"
        "movupd 0x1000(,%r10,4), %xmm7\n"
        "movupd %xmm6, 1024(%r12,%r8,2)\n"
        "movss %xmm4, %xmm0\n"
        "movss 100000(%rbp,%r9,4), %xmm13\n"
        "movss %xmm12, (%r13)\n"
        "movsd %xmm9, %xmm2\n"
        "movsd 0(%edx), %xmm12\n"
        "movsd %xmm13, 0x0(,%rbp,2)\n"
        "movdqa %xmm14, %xmm15\n"
        "movdqa (%rbx,%r10,8), %xmm15\n"
        "movdqa %xmm6, (%r13)\n"
        "movdqu %xmm9, %xmm9\n"
        "movdqu -128(%rsp), %xmm1\n"
        "movdqu %xmm1, (%rbp)\n"
        "movhps 0x10, %xmm4\n"
        "movhps %xmm0, 0x10\n"
        "movlps -8(%rbp,%r14,4), %xmm3\n"
        "movlps %xmm11, %gs:0x1000\n"
        "movhpd (%r8), %xmm10\n"
        "movhpd %xmm0, 4(%r12,%r12,4)\n"
        "movlpd (%rdx), %xmm10\n"
        "movlpd %xmm15, 128(%r15)\n"
        "movhlps %xmm0, %xmm12\n"
        "movlhps %xmm0, %xmm9\n"
        "unpcklps %xmm14, %xmm4\n"
        "unpcklps 0x1234, %xmm3\n"
        "unpckhps %xmm2, %xmm2\n"
        "unpckhps 1(%rdi), %xmm7\n"
        "unpcklpd %xmm10, %xmm10\n"
        "unpcklpd (%rbp), %xmm2\n"
        "unpckhpd %xmm10, %xmm3\n"
        "unpckhpd (%rbp), %xmm10\n"
        "andps %xmm5, %xmm11\n"
        "andps %fs:0x0, %xmm14\n"
        "andpd %xmm2, %xmm10\n"
        "andpd (%r11,%rbx,1), %xmm4\n"
        "andnps %xmm15, %xmm9\n"
        "andnps 16(%ebx), %xmm12\n"
        "andnpd %xmm2, %xmm6\n"
        "andnpd -305419896(%rdx), %xmm7\n"
        "orps %xmm8, %xmm10\n"
        "orps 4(%rbp,%rdx,2), %xmm6\n"
        "orpd %xmm10, %xmm10\n"
        "orpd -8(%rdx,%r11,8), %xmm0\n"
        "xorps %xmm7, %xmm8\n"
        "xorps 0x10(,%rsi,4), %xmm10\n"
        "xorpd %xmm9, %xmm10\n"
        "xorpd 0(%r10d), %xmm6\n"
        "comiss %xmm8, %xmm15\n"
        "comiss -8(%r14,%rbp,4), %xmm3\n"
        "comisd %xmm15, %xmm3\n"
        "comisd (%rsp), %xmm4\n"
        "ucomiss %xmm1, %xmm10\n"
        "ucomiss (%r8), %xmm7\n"
        "ucomisd %xmm9, %xmm11\n"
        "ucomisd 0x10(,%r10,2), %xmm4\n"
        "cvtps2pd %xmm10, %xmm11\n"
        "cvtps2pd 16(%r10d), %xmm13\n"
        "cvtpd2ps %xmm2, %xmm13\n"
        "cvtpd2ps 4(%rbx,%rbp,2), %xmm0\n"
        "cvtss2sd %xmm15, %xmm14\n"
        "cvtss2sd 0x0(,%rbp,4), %xmm12\n"
        "cvtsd2ss %xmm2, %xmm0\n"
        "cvtsd2ss 4660(%r13), %xmm4\n"
        "cvtdq2ps %xmm5, %xmm14\n"
        "cvtdq2ps 100000(%r8,%rsi,1), %xmm12\n"
        "cvtps2dq %xmm7, %xmm0\n"
        "cvtps2dq 0x7fffffff, %xmm8\n"
        "cvttps2dq %xmm5, %xmm11\n"
        "cvttps2dq (%rdi), %xmm2\n"
        "cvtdq2pd %xmm5, %xmm4\n"
        "cvtdq2pd 4(%rdx,%rdi,8), %xmm13\n"
        "cvtpd2dq %xmm15, %xmm7\n"
        "cvtpd2dq 4660(%r10), %xmm8\n"
        "cvttpd2dq %xmm11, %xmm0\n"
        "cvttpd2dq 100000(%rdi,%r9,8), %xmm0\n"
        "rcpps %xmm15, %xmm14\n"
        "rcpps %fs:0x28, %xmm0\n"
        "rcpss %xmm8, %xmm13\n"
        "rcpss (%rsp), %xmm3\n"
        "rsqrtps %xmm0, %xmm8\n"
        "rsqrtps (%rbx,%rbp,8), %xmm9\n"
        "rsqrtss %xmm14, %xmm15\n"
        "rsqrtss 1(%r8), %xmm11\n"
        "movddup %xmm13, %xmm8\n"
        "movddup -8(%r10,%rdx,8), %xmm6\n"
        "movshdup %xmm11, %xmm2\n"
        "movshdup (%rax), %xmm15\n"
        "movsldup %xmm13, %xmm9\n"
        "movsldup 16(%r8d), %xmm11\n"
        "cmpps $27, %xmm1, %xmm13\n"
        "cmpps $3, (%r12), %xmm4\n"
        "cmppd $4, %xmm11, %xmm2\n"
        "cmppd $3, 100000(%r9,%r9,4), %xmm11\n"
        "cmpss $4, %xmm6, %xmm0\n"
        "cmpss $3, (%rsp), %xmm10\n"
        "cmpsd $4, %xmm13, %xmm3\n"
        "cmpsd $3, (%rbx,%rsi,1), %xmm5\n"
        "shufps $1, %xmm6, %xmm6\n"
        "shufps $3, (%r13), %xmm10\n"
        "shufpd $4, %xmm11, %xmm11\n"
        "shufpd $3, 16(%esp), %xmm9\n"
        "pshufd $4, %xmm6, %xmm13\n"
        "pshufd $3, 16(%ebx), %xmm2\n"
        "pshufhw $0, %xmm13, %xmm14\n"
        "pshufhw $3, (%rbp), %xmm14\n"
        "pshuflw $4, %xmm1, %xmm8\n"
        "pshuflw $3, (%rbx,%rax,1), %xmm1\n"
        "roundps $4, %xmm11, %xmm14\n"
        "roundps $3, (%r12), %xmm10\n"
        "roundpd $0, %xmm0, %xmm6\n"
        "roundpd $3, 0x1000(,%rbp,8), %xmm7\n"
        "roundss $4, %xmm12, %xmm11\n"
        "roundss $3, -8(%r15,%r11,2), %xmm6\n"
        "roundsd $1, %xmm2, %xmm8\n"
        "roundsd $3, (%r13), %xmm15\n"
        "blendps $0, %xmm13, %xmm12\n"
        "blendps $3, (%rbp), %xmm2\n"
        "blendpd $4, %xmm2, %xmm7\n"
        "blendpd $3, 0x1234, %xmm4\n"
        "pblendw $27, %xmm1, %xmm3\n"
        "pblendw $3, (%r13), %xmm10\n"
        "palignr $0, %xmm9, %xmm3\n"
        "palignr $3, (%r8), %xmm11\n"
        "dpps $4, %xmm7, %xmm3\n"
        "dpps $3, (%r12), %xmm2\n"
        "pclmulqdq $27, %xmm8, %xmm2\n"
        "pclmulqdq $3, 0x10, %xmm0\n"
        "insertps $4, %xmm10, %xmm10\n"
        "insertps $3, %fs:0x28, %xmm11\n"
        "cmpeqps %xmm8, %xmm7\n"
        "cmpeqpd %xmm14, %xmm3\n"
        "cmpeqss %xmm4, %xmm11\n"
        "cmpeqsd %xmm7, %xmm11\n"
        "cmpltps %xmm2, %xmm11\n"
        "cmpltpd %xmm6, %xmm9\n"
        "cmpltss %xmm6, %xmm4\n"
        "cmpltsd %xmm0, %xmm2\n"
        "cmpleps %xmm12, %xmm3\n"
        "cmplepd %xmm1, %xmm6\n"
        "cmpless %xmm15, %xmm13\n"
        "cmplesd %xmm11, %xmm11\n"
        "cmpunordps %xmm7, %xmm12\n"
        "cmpunordpd %xmm2, %xmm13\n"
        "cmpunordss %xmm12, %xmm6\n"
        "cmpunordsd %xmm6, %xmm12\n"
        "cmpneqps %xmm15, %xmm6\n"
        "cmpneqpd %xmm0, %xmm11\n"
        "cmpneqss %xmm11, %xmm1\n"
        "cmpneqsd %xmm1, %xmm10\n"
        "cmpnltps %xmm10, %xmm3\n"
        "cmpnltpd %xmm6, %xmm14\n"
        "cmpnltss %xmm6, %xmm8\n"
        "cmpnltsd %xmm10, %xmm11\n"
        "cmpnleps %xmm11, %xmm13\n"
        "cmpnlepd %xmm13, %xmm12\n"
        "cmpnless %xmm5, %xmm12\n"
        "cmpnlesd %xmm8, %xmm12\n"
        "cmpordps %xmm15, %xmm9\n"
        "cmpordpd %xmm0, %xmm2\n"
        "cmpordss %xmm14, %xmm3\n"
        "cmpordsd %xmm3, %xmm3\n"
        "punpcklbw %xmm11, %xmm15\n"
        "punpcklbw 0x0(,%r13,1), %xmm15\n"
        "punpcklwd %xmm15, %xmm9\n"
        "punpcklwd (%r13), %xmm7\n"
        "punpckldq %xmm13, %xmm11\n"
        "punpckldq (%r13), %xmm11\n"
        "packsswb %xmm6, %xmm5\n"
        "packsswb 0x10(,%r9,1), %xmm2\n"
        "pcmpgtb %xmm5, %xmm12\n"
        "pcmpgtb %gs:0x0, %xmm1\n"
        "pcmpgtw %xmm8, %xmm7\n"
        "pcmpgtw 16(%ebx), %xmm2\n"
        "pcmpgtd %xmm9, %xmm10\n"
        "pcmpgtd 4(%r15,%r14,8), %xmm8\n"
        "packuswb %xmm2, %xmm6\n"
        "packuswb (%rbp), %xmm12\n"
        "punpckhbw %xmm2, %xmm3\n"
        "punpckhbw (%r12), %xmm0\n"
        "punpckhwd %xmm8, %xmm15\n"
        "punpckhwd 1024(%rsp,%r11,8), %xmm10\n"
        "punpckhdq %xmm14, %xmm8\n"
        "punpckhdq (%r12), %xmm1\n"
        "packssdw %xmm12, %xmm14\n"
        "packssdw 16(%esp), %xmm0\n"
        "punpcklqdq %xmm8, %xmm6\n"
        "punpcklqdq (%rdx,%r15,4), %xmm4\n"
        "punpckhqdq %xmm15, %xmm7\n"
        "punpckhqdq (%r12), %xmm5\n"
        "pcmpeqb %xmm9, %xmm5\n"
        "pcmpeqb (%rsp), %xmm6\n"
        "pcmpeqw %xmm12, %xmm0\n"
        "pcmpeqw (%rbp), %xmm14\n"
        "pcmpeqd %xmm12, %xmm9\n"
        "pcmpeqd %fs:0x1000, %xmm11\n"
        "paddq %xmm3, %xmm7\n"
        "paddq -1(%rdx), %xmm6\n"
        "pmullw %xmm13, %xmm3\n"
        "pmullw -8(%rdi,%r10,2), %xmm9\n"
        "psubusb %xmm2, %xmm5\n"
        "psubusb 0x10(,%rdx,4), %xmm1\n"
        "psubusw %xmm1, %xmm6\n"
        "psubusw 16(%r10d), %xmm9\n"
        "pminub %xmm12, %xmm1\n"
        "pminub (%r12), %xmm12\n"
        "pand %xmm12, %xmm2\n"
        "pand (%rbp,%r11,2), %xmm4\n"
        "paddusb %xmm11, %xmm8\n"
        "paddusb -129(%rbx), %xmm7\n"
        "paddusw %xmm12, %xmm3\n"
        "paddusw (%r8), %xmm14\n"
        "pmaxub %xmm9, %xmm1\n"
        "pmaxub 100000(%r8,%r11,8), %xmm5\n"
        "pandn %xmm10, %xmm5\n"
        "pandn 16(%ebp), %xmm13\n"
        "pavgb %xmm3, %xmm14\n"
        "pavgb 0x0(,%r14,2), %xmm15\n"
        "pavgw %xmm6, %xmm1\n"
        "pavgw 16(%r8d), %xmm10\n"
        "pmulhuw %xmm9, %xmm9\n"
        "pmulhuw -8(%rax,%rbp,4), %xmm11\n"
        "pmulhw %xmm13, %xmm12\n"
        "pmulhw (%rsp), %xmm14\n"
        "psubsb %xmm12, %xmm6\n"
        "psubsb %fs:0x1000, %xmm9\n"
        "psubsw %xmm2, %xmm8\n"
        "psubsw (%r13), %xmm6\n"
        "pminsw %xmm4, %xmm10\n"
        "pminsw 4(%rsi,%r12,4), %xmm9\n"
        "por %xmm6, %xmm0\n"
        "por 0x7fffffff, %xmm9\n"
        "paddsb %xmm10, %xmm10\n"
        "paddsb (%r14,%rsi,4), %xmm7\n"
        "paddsw %xmm5, %xmm8\n"
        "paddsw 127(%rbp), %xmm11\n"
        "pmaxsw %xmm13, %xmm14\n"
        "pmaxsw 0x10, %xmm1\n"
        "pxor %xmm12, %xmm15\n"
        "pxor 0x1000(,%r9,1), %xmm5\n"
        "pmuludq %xmm14, %xmm1\n"
        "pmuludq 127(%rbp), %xmm7\n"
        "pmaddwd %xmm4, %xmm9\n"
        "pmaddwd (%r9,%r15,2), %xmm9\n"
        "psadbw %xmm11, %xmm11\n"
        "psadbw 128(%r10), %xmm10\n"
        "psubb %xmm8, %xmm1\n"
        "psubb (%r13), %xmm14\n"
        "psubw %xmm9, %xmm3\n"
        "psubw (%rcx,%r12,8), %xmm5\n"
        "psubd %xmm15, %xmm12\n"
        "psubd 0x0(,%rbx,2), %xmm10\n"
        "psubq %xmm2, %xmm12\n"
        "psubq 0x10, %xmm6\n"
        "paddb %xmm1, %xmm9\n"
        "paddb (%r13), %xmm1\n"
        "paddw %xmm7, %xmm4\n"
        "paddw 1(%r13), %xmm9\n"
        "paddd %xmm8, %xmm9\n"
        "paddd (%r10,%r11,1), %xmm7\n"
        "pshufb %xmm4, %xmm7\n"
        "pshufb (%r13), %xmm4\n"
        "phaddw %xmm11, %xmm14\n"
        "phaddw -1(%r13), %xmm14\n"
        "phaddd %xmm7, %xmm7\n"
        "phaddd 0x0(,%r9,1), %xmm5\n"
        "pmaddubsw %xmm11, %xmm2\n"
        "pmaddubsw (%r10), %xmm3\n"
        "psignb %xmm5, %xmm4\n"
        "psignb -8(%r9,%r14,8), %xmm9\n"
        "pmulhrsw %xmm6, %xmm4\n"
        "pmulhrsw 0(%ebx), %xmm13\n"
        "pabsb %xmm7, %xmm2\n"
        "pabsb (%rbp), %xmm14\n"
        "pabsw %xmm4, %xmm3\n"
        "pabsw (%rsp), %xmm0\n"
        "pabsd %xmm1, %xmm7\n"
        "pabsd %gs:0x0, %xmm3\n"
        "pmovsxbw %xmm15, %xmm14\n"
        "pmovsxbw 0(%r12d), %xmm1\n"
        "pmovzxbd %xmm5, %xmm12\n"
        "pmovzxbd 1(%r15), %xmm5\n"
        "pmovzxdq %xmm8, %xmm8\n"
        "pmovzxdq 100000(%r9,%r13,4), %xmm10\n"
        "pmuldq %xmm14, %xmm0\n"
        "pmuldq 0x7fffffff, %xmm14\n"
        "pcmpeqq %xmm15, %xmm9\n"
        "pcmpeqq 8(%r11), %xmm13\n"
        "packusdw %xmm6, %xmm5\n"
        "packusdw (%rbp), %xmm11\n"
        "pcmpgtq %xmm15, %xmm13\n"
        "pcmpgtq (%rax), %xmm6\n"
        "pminsb %xmm11, %xmm12\n"
        "pminsb (%rax,%r9,2), %xmm5\n"
        "pminsd %xmm6, %xmm3\n"
        "pminsd -8(%r9,%r10,1), %xmm15\n"
        "pminuw %xmm7, %xmm3\n"
        "pminuw (%r12), %xmm7\n"
        "pminud %xmm7, %xmm5\n"
        "pminud (%r10,%r9,4), %xmm0\n"
        "pmaxsb %xmm2, %xmm3\n"
        "pmaxsb 0x1000(,%rdi,4), %xmm4\n"
        "pmaxsd %xmm8, %xmm14\n"
        "pmaxsd %gs:0x0, %xmm7\n"
        "pmaxuw %xmm13, %xmm0\n"
        "pmaxuw (%rdx,%rdx,8), %xmm6\n"
        "pmaxud %xmm3, %xmm3\n"
        "pmaxud (%rsp), %xmm15\n"
        "pmulld %xmm1, %xmm13\n"
        "pmulld 1(%rbp), %xmm5\n"
        "ptest %xmm13, %xmm11\n"
        "ptest (%rdx,%rdi,8), %xmm2\n"
        "phminposuw %xmm11, %xmm11\n"
        "phminposuw 100000(%r10,%rsi,4), %xmm13\n"
        "aesenc %xmm2, %xmm0\n"
        "aesenc (%r13,%rdx,8), %xmm14\n"
        "aesenclast %xmm3, %xmm12\n"
        "aesenclast (%r11,%r15,1), %xmm14\n"
        "aesdec %xmm11, %xmm14\n"
        "aesdec 0x1000(,%rbp,4), %xmm9\n"
        "aesdeclast %xmm8, %xmm13\n"
        "aesdeclast %fs:0x28, %xmm0\n"
        "aesimc %xmm3, %xmm13\n"
        "aesimc %gs:0x0, %xmm8\n"
        "haddps %xmm6, %xmm13\n"
        "haddps (%r10), %xmm14\n"
        "hsubpd %xmm8, %xmm3\n"
        "hsubpd 16(%eax), %xmm3\n"
        "addsubps %xmm11, %xmm4\n"
        "addsubps %fs:0x0, %xmm6\n"
        "psrlw $1, %xmm12\n"
        "psrlw %xmm1, %xmm2\n"
        "psrlw %fs:0x28, %xmm8\n"
        "psrld $31, %xmm15\n"
        "psrld %xmm14, %xmm2\n"
        "psrld 100000(%r8,%r8,2), %xmm9\n"
        "psrlq $7, %xmm6\n"
        "psrlq %xmm4, %xmm12\n"
        "psrlq (%r13), %xmm8\n"
        "psraw $7, %xmm14\n"
        "psraw %xmm14, %xmm3\n"
        "psraw (%r13,%r8,4), %xmm14\n"
        "psrad $31, %xmm11\n"
        "psrad %xmm11, %xmm8\n"
        "psrad 0(%esi), %xmm6\n"
        "psllw $31, %xmm15\n"
        "psllw %xmm0, %xmm11\n"
        "psllw (%r12), %xmm7\n"
        "pslld $1, %xmm0\n"
        "pslld %xmm2, %xmm13\n"
        "pslld %fs:0x28, %xmm9\n"
        "psllq $7, %xmm12\n"
        "psllq %xmm4, %xmm5\n"
        "psllq (%r12), %xmm7\n"
        "cvtsi2ss %eax, %xmm13\n"
        "cvtsi2ss %r10, %xmm9\n"
        "cvtsi2ssl (%rax), %xmm0\n"
        "cvtsi2ssq 8(%rbx), %xmm15\n"
        "cvtsi2sd %eax, %xmm14\n"
        "cvtsi2sd %r10, %xmm9\n"
        "cvtsi2sdl (%rax), %xmm2\n"
        "cvtsi2sdq 8(%rbx), %xmm12\n"
        "cvttss2si %xmm13, %eax\n"
        "cvttss2si %xmm5, %r11\n"
        "cvttsd2si %xmm14, %r11\n"
        "cvtss2si %xmm4, %eax\n"
        "cvtss2si %xmm15, %r11\n"
        "cvtsd2si %xmm15, %eax\n"
        "cvtsd2si %xmm10, %r11\n"
        "1: loop 1b\n"
        "jmp 0f\n"
        "jno 4f\n"
        "jmp 4b\n"
        "jno 0b\n"
        "lea 3b(%rip), %rax\n"
        "jle 2f\n"
        "js 2b\n"
        "jae 3b\n"
        "jo 0b\n"
        "jbe 2f\n"
        "jns 2b\n"
        "je 1b\n"
        "jl 3b\n"
        "jno 1b\n"
        "lea 2f(%rip), %rax\n"
        "jb 0b\n"
        "jmp 4f\n"
        "jmp 3b\n"
        "jmp 2b\n"
        "jge 0b\n"
        "jmp 0b\n"
        "js 1b\n"
        "jmp 2f\n"
        "jmp 3f\n"
        "lea 2b(%rip), %rax\n"
        "jg 0f\n"
        "lea 1b(%rip), %rax\n"
        "jle 4f\n"
        "lea 3f(%rip), %rax\n"
        "jo 2b\n"
        "jle 3f\n"
        "lea 4f(%rip), %rax\n"
        "lea 0b(%rip), %rax\n"
        "lea 4b(%rip), %rax\n"
        "jbe 1f\n"
        "jns 4f\n"
        "jno 3f\n"
        "lea 1f(%rip), %rax\n"
        "ja 3f\n"
        "jo 4f\n"
        "jg 0b\n"
        "jg 4b\n"
        "jmp 1f\n"
        "jmp 1b\n"
        "js 3b\n"
        "jbe 1b\n"
        "jno 1f\n"
        "jae 2f\n"
        "lea 0f(%rip), %rax\n"
        "jne 3b\n"
        "ja 2f\n"
        "jle 0b\n"
        "jle 1b\n"
        "jnp 3b\n"
        "jne 4b\n"
        "jb 3f\n"
        "jns 3f\n"
        "0:\n1:\n2:\n3:\n4:\n"
        ".globl axiom_asm_table_end\n"
        "axiom_asm_table_end:\n"
        ".text\n");

extern const unsigned char axiom_asm_table[], axiom_asm_table_end[];

/* each line: its bytes */
static const char *const want[] = {
    "4000e2",  /* add %spl, %dl */
    "4500877fffffff",  /* addb %r8b, -129(%r15) */
    "44027d00",  /* add (%rbp), %r15b */
    "4180c400",  /* add $0, %r12b */
    "674180401080",  /* addb $-128, 16(%r8d) */
    "0401",  /* add $1, %al */
    "664101da",  /* add %bx, %r10w */
    "660100",  /* addw %ax, (%rax) */
    "66033424",  /* add (%rsp), %si */
    "66058000",  /* add $128, %ax */
    "6643814486f8ff7f",  /* addw $32767, -8(%r14,%r8,4) */
    "6683c001",  /* add $1, %ax */
    "4101fe",  /* add %edi, %r14d */
    "45014d00",  /* addl %r9d, (%r13) */
    "45030c24",  /* add (%r12), %r9d */
    "4183c200",  /* add $0, %r10d */
    "41830424ff",  /* addl $-1, (%r12) */
    "83c0ff",  /* add $-1, %eax */
    "4901f1",  /* add %rsi, %r9 */
    "674c014910",  /* addq %r9, 16(%ecx) */
    "4d036d00",  /* add (%r13), %r13 */
    "4883c5ff",  /* add $-1, %rbp */
    "488304247f",  /* addq $127, (%rsp) */
    "480578563412",  /* add $305419896, %rax */
    "4008dd",  /* or %bl, %bpl */
    "644408242528000000",  /* orb %r12b, %fs:0x28 */
    "450a21",  /* or (%r9), %r12b */
    "4080cd01",  /* or $1, %bpl */
    "42804c68f8ff",  /* orb $255, -8(%rax,%r13,2) */
    "0c80",  /* or $-128, %al */
    "664509db",  /* or %r11w, %r11w */
    "664409347d00100000",  /* orw %r14w, 0x1000(,%rdi,2) */
    "66450b3c24",  /* or (%r12), %r15w */
    "6683ce7f",  /* or $127, %si */
    "66810c2534120000ff7f",  /* orw $32767, 0x1234 */
    "660d8000",  /* or $128, %ax */
    "4409d8",  /* or %r11d, %eax */
    "44095500",  /* orl %r10d, (%rbp) */
    "460b443500",  /* or (%rbp,%r14,1), %r8d */
    "83ca00",  /* or $0, %edx */
    "41830e00",  /* orl $0, (%r14) */
    "83c87f",  /* or $127, %eax */
    "4909f8",  /* or %rdi, %r8 */
    "6748095a10",  /* orq %rbx, 16(%edx) */
    "4c0b446d00",  /* or (%rbp,%rbp,2), %r8 */
    "4983c9ff",  /* or $-1, %r9 */
    "6448810c250000000080000000",  /* orq $128, %fs:0x0 */
    "4883c87f",  /* or $127, %rax */
    "4510e2",  /* adc %r12b, %r10b */
    "4510ac4800040000",  /* adcb %r13b, 1024(%r8,%rcx,2) */
    "671207",  /* adc 0(%edi), %al */
    "4080d480",  /* adc $-128, %spl */
    "438014e600",  /* adcb $0, (%r14,%r12,8) */
    "14ff",  /* adc $255, %al */
    "664111fe",  /* adc %di, %r14w */
    "6641113c24",  /* adcw %di, (%r12) */
    "656613342500000000",  /* adc %gs:0x0, %si */
    "664183d100",  /* adc $0, %r9w */
    "664183517f00",  /* adcw $0, 127(%r9) */
    "6683d0ff",  /* adc $-1, %ax */
    "4111ed",  /* adc %ebp, %r13d */
    "46110408",  /* adcl %r8d, (%rax,%r9,1) */
    "67134d10",  /* adc 16(%ebp), %ecx */
    "81d680000000",  /* adc $128, %esi */
    "8396800000007f",  /* adcl $127, 128(%rsi) */
    "1578563412",  /* adc $305419896, %eax */
    "4911f7",  /* adc %rsi, %r15 */
    "48111c2534120000",  /* adcq %rbx, 0x1234 */
    "4d1322",  /* adc (%r10), %r12 */
    "4883d07f",  /* adc $127, %rax */
    "4b8354aaf801",  /* adcq $1, -8(%r10,%r13,4) */
    "481580000000",  /* adc $128, %rax */
    "4118f0",  /* sbb %sil, %r8b */
    "181c25ffffff7f",  /* sbbb %bl, 0x7fffffff */
    "411a3c24",  /* sbb (%r12), %dil */
    "80d900",  /* sbb $0, %cl */
    "805c550000",  /* sbbb $0, (%rbp,%rdx,2) */
    "1c7f",  /* sbb $127, %al */
    "664519d0",  /* sbb %r10w, %r8w */
    "656644191c2500000000",  /* sbbw %r11w, %gs:0x0 */
    "6766411b0a",  /* sbb 0(%r10d), %cx */
    "6681da8000",  /* sbb $128, %dx */
    "66811c348000",  /* sbbw $128, (%rsp,%rsi,1) */
    "661dff7f",  /* sbb $32767, %ax */
    "4419e2",  /* sbb %r12d, %edx */
    "6741197910",  /* sbbl %edi, 16(%r9d) */
    "671b4b10",  /* sbb 16(%ebx), %ecx */
    "83d87f",  /* sbb $127, %eax */
    "811a78563412",  /* sbbl $305419896, (%rdx) */
    "83d800",  /* sbb $0, %eax */
    "4819ce",  /* sbb %rcx, %rsi */
    "6749193e",  /* sbbq %rdi, 0(%r14d) */
    "4c1b2c17",  /* sbb (%rdi,%rdx,1), %r13 */
    "4981d980000000",  /* sbb $128, %r9 */
    "48811c750000000078563412",  /* sbbq $305419896, 0x0(,%rsi,2) */
    "4883d8ff",  /* sbb $-1, %rax */
    "4420d1",  /* and %r10b, %cl */
    "412001",  /* andb %al, (%r9) */
    "6745226210",  /* and 16(%r10d), %r12b */
    "80e200",  /* and $0, %dl */
    "678066107f",  /* andb $127, 16(%esi) */
    "24ff",  /* and $255, %al */
    "664521fb",  /* and %r15w, %r11w */
    "664421046e",  /* andw %r8w, (%rsi,%rbp,2) */
    "664523b40b00040000",  /* and 1024(%r11,%rcx,1), %r14w */
    "664183e200",  /* and $0, %r10w */
    "668324253412000001",  /* andw $1, 0x1234 */
    "6683e001",  /* and $1, %ax */
    "4521d1",  /* and %r10d, %r9d */
    "64213c2500000000",  /* andl %edi, %fs:0x0 */
    "422354e8f8",  /* and -8(%rax,%r13,8), %edx */
    "83e27f",  /* and $127, %edx */
    "836001ff",  /* andl $-1, 1(%rax) */
    "83e07f",  /* and $127, %eax */
    "4d21df",  /* and %r11, %r15 */
    "4e213c8b",  /* andq %r15, (%rbx,%r9,4) */
    "48236508",  /* and 8(%rbp), %rsp */
    "4981e200000080",  /* and $-2147483648, %r10 */
    "498361807f",  /* andq $127, -128(%r9) */
    "4883e001",  /* and $1, %rax */
    "4128ca",  /* sub %cl, %r10b */
    "41283479",  /* subb %sil, (%r9,%rdi,2) */
    "2a17",  /* sub (%rdi), %dl */
    "4180ed80",  /* sub $-128, %r13b */
    "42802cf500100000ff",  /* subb $255, 0x1000(,%r14,8) */
    "2c00",  /* sub $0, %al */
    "664529e2",  /* sub %r12w, %r10w */
    "66292cc500100000",  /* subw %bp, 0x1000(,%rax,8) */
    "67662b642410",  /* sub 16(%esp), %sp */
    "664183e801",  /* sub $1, %r8w */
    "66832c25ffffff7f7f",  /* subw $127, 0x7fffffff */
    "6683e800",  /* sub $0, %ax */
    "4129ff",  /* sub %edi, %r15d */
    "432904ca",  /* subl %eax, (%r10,%r9,8) */
    "2bbcf7a0860100",  /* sub 100000(%rdi,%rsi,8), %edi */
    "4183edff",  /* sub $-1, %r13d */
    "832c247f",  /* subl $127, (%rsp) */
    "83e800",  /* sub $0, %eax */
    "4c29dd",  /* sub %r11, %rbp */
    "4a2934a510000000",  /* subq %rsi, 0x10(,%r12,4) */
    "4b2b1c16",  /* sub (%r14,%r10,1), %rbx */
    "4981ec78563412",  /* sub $305419896, %r12 */
    "49832c7801",  /* subq $1, (%r8,%rdi,2) */
    "482d78563412",  /* sub $305419896, %rax */
    "4130cf",  /* xor %cl, %r15b */
    "644430242500100000",  /* xorb %r12b, %fs:0x1000 */
    "443202",  /* xor (%rdx), %r8b */
    "4180f000",  /* xor $0, %r8b */
    "4280344200",  /* xorb $0, (%rdx,%r8,2) */
    "3480",  /* xor $-128, %al */
    "664531ec",  /* xor %r13w, %r12w */
    "6566312c2500000000",  /* xorw %bp, %gs:0x0 */
    "664533ad34120000",  /* xor 4660(%r13), %r13w */
    "664183f100",  /* xor $0, %r9w */
    "6643837464047f",  /* xorw $127, 4(%r12,%r12,2) */
    "6635ff7f",  /* xor $32767, %ax */
    "4431f0",  /* xor %r14d, %eax */
    "67314610",  /* xorl %eax, 16(%esi) */
    "45330451",  /* xor (%r9,%rdx,2), %r8d */
    "4183f07f",  /* xor $127, %r8d */
    "837500ff",  /* xorl $-1, (%rbp) */
    "3580000000",  /* xor $128, %eax */
    "4831de",  /* xor %rbx, %rsi */
    "4f31642904",  /* xorq %r12, 4(%r9,%r13,1) */
    "4833607f",  /* xor 127(%rax), %rsp */
    "4983f300",  /* xor $0, %r11 */
    "4a81348d0010000078563412",  /* xorq $305419896, 0x1000(,%r9,4) */
    "483500000080",  /* xor $-2147483648, %rax */
    "4438d0",  /* cmp %r10b, %al */
    "385d00",  /* cmpb %bl, (%rbp) */
    "443a3c25ffffff7f",  /* cmp 0x7fffffff, %r15b */
    "4180fc00",  /* cmp $0, %r12b */
    "807ce8f880",  /* cmpb $-128, -8(%rax,%rbp,8) */
    "3cff",  /* cmp $255, %al */
    "664539e9",  /* cmp %r13w, %r9w */
    "6641396d00",  /* cmpw %bp, (%r13) */
    "663b142534120000",  /* cmp 0x1234, %dx */
    "6681ff8000",  /* cmp $128, %di */
    "66833c7d00100000ff",  /* cmpw $-1, 0x1000(,%rdi,2) */
    "6683f8ff",  /* cmp $-1, %ax */
    "4539e0",  /* cmp %r12d, %r8d */
    "39687f",  /* cmpl %ebp, 127(%rax) */
    "443b042534120000",  /* cmp 0x1234, %r8d */
    "81fd80000000",  /* cmp $128, %ebp */
    "41813c2480000000",  /* cmpl $128, (%r12) */
    "83f8ff",  /* cmp $-1, %eax */
    "4c39ea",  /* cmp %r13, %rdx */
    "4d395e08",  /* cmpq %r11, 8(%r14) */
    "4c3b0424",  /* cmp (%rsp), %r8 */
    "4981f978563412",  /* cmp $305419896, %r9 */
    "49833c2401",  /* cmpq $1, (%r12) */
    "4883f87f",  /* cmp $127, %rax */
    "84d2",  /* test %dl, %dl */
    "44845d00",  /* test %r11b, (%rbp) */
    "41f6c0ff",  /* test $255, %r8b */
    "67f645107f",  /* testb $127, 16(%ebp) */
    "4588f8",  /* mov %r15b, %r8b */
    "47885410f8",  /* mov %r10b, -8(%r8,%r10,1) */
    "67448a4710",  /* mov 16(%edi), %r8b */
    "b201",  /* mov $1, %dl */
    "6741c60001",  /* movb $1, 0(%r8d) */
    "41f6d6",  /* not %r14b */
    "41f61424",  /* notb (%r12) */
    "41f6de",  /* neg %r14b */
    "65f61c2500000000",  /* negb %gs:0x0 */
    "f6e3",  /* mul %bl */
    "41f6644904",  /* mulb 4(%r9,%rcx,2) */
    "41f6ec",  /* imul %r12b */
    "64f62c2500000000",  /* imulb %fs:0x0 */
    "f6f1",  /* div %cl */
    "f630",  /* divb (%rax) */
    "40f6fe",  /* idiv %sil */
    "41f63f",  /* idivb (%r15) */
    "41fec6",  /* inc %r14b */
    "64fe042500000000",  /* incb %fs:0x0 */
    "40fecf",  /* dec %dil */
    "fe0c2534120000",  /* decb 0x1234 */
    "41d0c4",  /* rol %r12b */
    "40d0c7",  /* rol $1, %dil */
    "40d2c7",  /* rol %cl, %dil */
    "c0450002",  /* rolb $2, (%rbp) */
    "41d0c8",  /* ror %r8b */
    "41c0cf03",  /* ror $3, %r15b */
    "d2cb",  /* ror %cl, %bl */
    "d04901",  /* rorb $1, 1(%rcx) */
    "41d0d7",  /* rcl %r15b */
    "41c0d403",  /* rcl $3, %r12b */
    "41d2d1",  /* rcl %cl, %r9b */
    "d01425ffffff7f",  /* rclb $1, 0x7fffffff */
    "40d0dc",  /* rcr %spl */
    "d0d8",  /* rcr $1, %al */
    "41d2dc",  /* rcr %cl, %r12b */
    "d01c24",  /* rcrb $1, (%rsp) */
    "40d0e4",  /* shl %spl */
    "41d0e6",  /* shl $1, %r14b */
    "41d2e1",  /* shl %cl, %r9b */
    "d021",  /* shlb $1, (%rcx) */
    "40d0ec",  /* shr %spl */
    "41d0e8",  /* shr $1, %r8b */
    "d2eb",  /* shr %cl, %bl */
    "c0ae7fffffff02",  /* shrb $2, -129(%rsi) */
    "40d0fc",  /* sar %spl */
    "41c0f807",  /* sar $7, %r8b */
    "40d2fe",  /* sar %cl, %sil */
    "41c07d0005",  /* sarb $5, (%r13) */
    "41d0e0",  /* sal %r8b */
    "40c0e707",  /* sal $7, %dil */
    "40d2e7",  /* sal %cl, %dil */
    "67d06110",  /* salb $1, 16(%ecx) */
    "664585dd",  /* test %r11w, %r13w */
    "6641853424",  /* test %si, (%r12) */
    "6641f7c0ffff",  /* test $-1, %r8w */
    "6766f703ffff",  /* testw $-1, 0(%ebx) */
    "6689e1",  /* mov %sp, %cx */
    "66893425ffffff7f",  /* mov %si, 0x7fffffff */
    "66418b5f7f",  /* mov 127(%r15), %bx */
    "66b97f00",  /* mov $127, %cx */
    "6641c704240100",  /* movw $1, (%r12) */
    "6641f7d4",  /* not %r12w */
    "6641f75500",  /* notw (%r13) */
    "66f7da",  /* neg %dx */
    "66f71c0d10000000",  /* negw 0x10(,%rcx,1) */
    "6641f7e4",  /* mul %r12w */
    "6641f720",  /* mulw (%r8) */
    "66f7ec",  /* imul %sp */
    "6642f72cad00000000",  /* imulw 0x0(,%r13,4) */
    "66f7f3",  /* div %bx */
    "66f73425ffffff7f",  /* divw 0x7fffffff */
    "66f7fe",  /* idiv %si */
    "6566f73c2500100000",  /* idivw %gs:0x1000 */
    "6641ffc7",  /* inc %r15w */
    "66ff84247fffffff",  /* incw -129(%rsp) */
    "66ffc8",  /* dec %ax */
    "66ff0cdd10000000",  /* decw 0x10(,%rbx,8) */
    "6641d1c2",  /* rol %r10w */
    "6641d1c5",  /* rol $1, %r13w */
    "6641d3c3",  /* rol %cl, %r11w */
    "676641d14210",  /* rolw $1, 16(%r10d) */
    "6641d1cd",  /* ror %r13w */
    "6641c1ca03",  /* ror $3, %r10w */
    "6641d3cb",  /* ror %cl, %r11w */
    "6466c10c250000000002",  /* rorw $2, %fs:0x0 */
    "6641d1d2",  /* rcl %r10w */
    "6641c1d207",  /* rcl $7, %r10w */
    "6641d3d2",  /* rcl %cl, %r10w */
    "6641c1967fffffff05",  /* rclw $5, -129(%r14) */
    "6641d1de",  /* rcr %r14w */
    "6641c1dc03",  /* rcr $3, %r12w */
    "66d3da",  /* rcr %cl, %dx */
    "66c11c2405",  /* rcrw $5, (%rsp) */
    "6641d1e7",  /* shl %r15w */
    "6641c1e007",  /* shl $7, %r8w */
    "66d3e7",  /* shl %cl, %di */
    "676641c164241005",  /* shlw $5, 16(%r12d) */
    "6641d1e9",  /* shr %r9w */
    "6641c1ea03",  /* shr $3, %r10w */
    "66d3ed",  /* shr %cl, %bp */
    "6643d12c58",  /* shrw $1, (%r8,%r11,2) */
    "66d1fc",  /* sar %sp */
    "66d1fd",  /* sar $1, %bp */
    "6641d3fd",  /* sar %cl, %r13w */
    "676641c13805",  /* sarw $5, 0(%r8d) */
    "66d1e3",  /* sal %bx */
    "6641d1e0",  /* sal $1, %r8w */
    "66d3e2",  /* sal %cl, %dx */
    "6766c12605",  /* salw $5, 0(%esi) */
    "4185ed",  /* test %ebp, %r13d */
    "6741854110",  /* test %eax, 16(%r9d) */
    "f7c601000000",  /* test $1, %esi */
    "f7450078563412",  /* testl $305419896, (%rbp) */
    "4489de",  /* mov %r11d, %esi */
    "45897880",  /* mov %r15d, -128(%r8) */
    "458b3c24",  /* mov (%r12), %r15d */
    "41bc00000000",  /* mov $0, %r12d */
    "c7042478563412",  /* movl $305419896, (%rsp) */
    "41f7d6",  /* not %r14d */
    "f716",  /* notl (%rsi) */
    "f7d8",  /* neg %eax */
    "f71c24",  /* negl (%rsp) */
    "41f7e2",  /* mul %r10d */
    "f72424",  /* mull (%rsp) */
    "41f7ed",  /* imul %r13d */
    "67f76c2410",  /* imull 16(%esp) */
    "41f7f6",  /* div %r14d */
    "6741f730",  /* divl 0(%r8d) */
    "f7f8",  /* idiv %eax */
    "f73c2534120000",  /* idivl 0x1234 */
    "41ffc4",  /* inc %r12d */
    "42ff043d00000000",  /* incl 0x0(,%r15,1) */
    "ffc8",  /* dec %eax */
    "ff0c2534120000",  /* decl 0x1234 */
    "41d1c3",  /* rol %r11d */
    "41c1c707",  /* rol $7, %r15d */
    "41d3c2",  /* rol %cl, %r10d */
    "c1450002",  /* roll $2, (%rbp) */
    "41d1cd",  /* ror %r13d */
    "d1c8",  /* ror $1, %eax */
    "d3cd",  /* ror %cl, %ebp */
    "65c10c252800000002",  /* rorl $2, %gs:0x28 */
    "41d1d0",  /* rcl %r8d */
    "41c1d107",  /* rcl $7, %r9d */
    "41d3d5",  /* rcl %cl, %r13d */
    "c114751000000005",  /* rcll $5, 0x10(,%rsi,2) */
    "41d1df",  /* rcr %r15d */
    "41c1dd03",  /* rcr $3, %r13d */
    "d3da",  /* rcr %cl, %edx */
    "d11c56",  /* rcrl $1, (%rsi,%rdx,2) */
    "d1e0",  /* shl %eax */
    "41d1e0",  /* shl $1, %r8d */
    "d3e4",  /* shl %cl, %esp */
    "41c164240802",  /* shll $2, 8(%r12) */
    "d1e8",  /* shr %eax */
    "41d1ed",  /* shr $1, %r13d */
    "d3e9",  /* shr %cl, %ecx */
    "41c16c350405",  /* shrl $5, 4(%r13,%rsi,1) */
    "41d1fd",  /* sar %r13d */
    "c1fb07",  /* sar $7, %ebx */
    "41d3f8",  /* sar %cl, %r8d */
    "42c13c050010000005",  /* sarl $5, 0x1000(,%r8,1) */
    "d1e6",  /* sal %esi */
    "c1e207",  /* sal $7, %edx */
    "41d3e4",  /* sal %cl, %r12d */
    "41d16500",  /* sall $1, (%r13) */
    "4c85e3",  /* test %r12, %rbx */
    "4c85bc2488a9cbed",  /* test %r15, -305419896(%rsp) */
    "48f7c6ffffffff",  /* test $-1, %rsi */
    "49f7042401000000",  /* testq $1, (%r12) */
    "4d89cf",  /* mov %r9, %r15 */
    "6748897310",  /* mov %rsi, 16(%ebx) */
    "4d8b03",  /* mov (%r11), %r8 */
    "49c7c600000000",  /* mov $0, %r14 */
    "48c7477f7f000000",  /* movq $127, 127(%rdi) */
    "49f7d0",  /* not %r8 */
    "49f75495f8",  /* notq -8(%r13,%rdx,4) */
    "48f7de",  /* neg %rsi */
    "48f71c25ffffff7f",  /* negq 0x7fffffff */
    "48f7e4",  /* mul %rsp */
    "48f7242534120000",  /* mulq 0x1234 */
    "49f7e9",  /* imul %r9 */
    "48f72c9d10000000",  /* imulq 0x10(,%rbx,4) */
    "49f7f7",  /* div %r15 */
    "49f774d2f8",  /* divq -8(%r10,%rdx,8) */
    "48f7fc",  /* idiv %rsp */
    "49f73a",  /* idivq (%r10) */
    "48ffc4",  /* inc %rsp */
    "4bff84d2a0860100",  /* incq 100000(%r10,%r10,8) */
    "49ffca",  /* dec %r10 */
    "48ff4c2408",  /* decq 8(%rsp) */
    "49d1c0",  /* rol %r8 */
    "48d1c2",  /* rol $1, %rdx */
    "48d3c1",  /* rol %cl, %rcx */
    "48c1045b05",  /* rolq $5, (%rbx,%rbx,2) */
    "49d1c9",  /* ror %r9 */
    "49c1ca03",  /* ror $3, %r10 */
    "48d3c8",  /* ror %cl, %rax */
    "48d10c4d00100000",  /* rorq $1, 0x1000(,%rcx,2) */
    "49d1d0",  /* rcl %r8 */
    "49c1d207",  /* rcl $7, %r10 */
    "49d3d0",  /* rcl %cl, %r8 */
    "48c1142405",  /* rclq $5, (%rsp) */
    "49d1db",  /* rcr %r11 */
    "48d1db",  /* rcr $1, %rbx */
    "49d3d8",  /* rcr %cl, %r8 */
    "6748d11a",  /* rcrq $1, 0(%edx) */
    "48d1e3",  /* shl %rbx */
    "49c1e203",  /* shl $3, %r10 */
    "48d3e7",  /* shl %cl, %rdi */
    "48c1242402",  /* shlq $2, (%rsp) */
    "48d1ec",  /* shr %rsp */
    "49c1eb03",  /* shr $3, %r11 */
    "48d3ef",  /* shr %cl, %rdi */
    "6548d12c2500000000",  /* shrq $1, %gs:0x0 */
    "49d1fb",  /* sar %r11 */
    "48c1fc07",  /* sar $7, %rsp */
    "49d3f8",  /* sar %cl, %r8 */
    "49c1bf8000000002",  /* sarq $2, 128(%r15) */
    "49d1e4",  /* sal %r12 */
    "48c1e703",  /* sal $3, %rdi */
    "49d3e1",  /* sal %cl, %r9 */
    "4ac1241105",  /* salq $5, (%rcx,%r10,1) */
    "660faffd",  /* imul %bp, %di */
    "66450faf44ad04",  /* imul 4(%r13,%rbp,4), %r8w */
    "66456bf880",  /* imul $-128, %r8w, %r15w */
    "6647691c24e803",  /* imul $1000, (%r12,%r12,1), %r11w */
    "66418d8334120000",  /* lea 4660(%r11), %ax */
    "66450f45f1",  /* cmovnz %r9w, %r14w */
    "660f44344c",  /* cmove (%rsp,%rcx,2), %si */
    "66450fa3fa",  /* bt %r15w, %r10w */
    "66440fa31cd510000000",  /* bt %r11w, 0x10(,%rdx,8) */
    "660fbae31f",  /* bt $31, %bx */
    "67660fba601003",  /* btw $3, 16(%eax) */
    "660fabc4",  /* bts %ax, %sp */
    "66440fab2424",  /* bts %r12w, (%rsp) */
    "66410fbaed1f",  /* bts $31, %r13w */
    "660fba2cfd1000000003",  /* btsw $3, 0x10(,%rdi,8) */
    "660fb3cb",  /* btr %cx, %bx */
    "66440fb338",  /* btr %r15w, (%rax) */
    "660fbaf301",  /* btr $1, %bx */
    "6766410fba701003",  /* btrw $3, 16(%r8d) */
    "66450fbbdf",  /* btc %r11w, %r15w */
    "67660fbb7d10",  /* btc %di, 16(%ebp) */
    "660fbaf901",  /* btc $1, %cx */
    "65660fba3c252800000003",  /* btcw $3, %gs:0x28 */
    "66440fbce9",  /* bsf %cx, %r13w */
    "66440fbc142534120000",  /* bsf 0x1234, %r10w */
    "66410fbdfe",  /* bsr %r14w, %di */
    "67660fbd6b10",  /* bsr 16(%ebx), %bp */
    "66f3450fbcd7",  /* tzcnt %r15w, %r10w */
    "66f3440fbc6080",  /* tzcnt -128(%rax), %r12w */
    "66f3440fbde5",  /* lzcnt %bp, %r12w */
    "66f30fbdbc9b00040000",  /* lzcnt 1024(%rbx,%rbx,4), %di */
    "66f3450fb8fe",  /* popcnt %r14w, %r15w */
    "6466f3440fb8342528000000",  /* popcnt %fs:0x28, %r14w */
    "66440fa4e303",  /* shld $3, %r12w, %bx */
    "66440fad3cd500000000",  /* shrd %cl, %r15w, 0x0(,%rdx,8) */
    "66440fa5eb",  /* shld %r13w, %bx */
    "660fc18580000000",  /* xadd %ax, 128(%rbp) */
    "66460fb10c0c",  /* cmpxchg %r9w, (%rsp,%r9,1) */
    "66f0450fb106",  /* lock cmpxchg %r8w, (%r14) */
    "66f0410fc13c24",  /* lock xadd %di, (%r12) */
    "664187fd",  /* xchg %di, %r13w */
    "6643872c5a",  /* xchg %bp, (%r10,%r11,2) */
    "660f38f08580000000",  /* movbe 128(%rbp), %ax */
    "660f38f1342510000000",  /* movbe %si, 0x10 */
    "0fafd6",  /* imul %esi, %edx */
    "430faf8c2ca0860100",  /* imul 100000(%r12,%r13,1), %ecx */
    "6bda7f",  /* imul $127, %edx, %ebx */
    "694500e8030000",  /* imul $1000, (%rbp), %eax */
    "458d01",  /* lea (%r9), %r8d */
    "0f4de5",  /* cmovge %ebp, %esp */
    "450f443424",  /* cmove (%r12), %r14d */
    "440fa3ed",  /* bt %r13d, %ebp */
    "440fa3242534120000",  /* bt %r12d, 0x1234 */
    "0fbae41f",  /* bt $31, %esp */
    "420fba24350000000003",  /* btl $3, 0x0(,%r14,1) */
    "450fabda",  /* bts %r11d, %r10d */
    "0fab24ed10000000",  /* bts %esp, 0x10(,%rbp,8) */
    "410fbaee1f",  /* bts $31, %r14d */
    "0fba2c2403",  /* btsl $3, (%rsp) */
    "440fb3c7",  /* btr %r8d, %edi */
    "0fb3b680000000",  /* btr %esi, 128(%rsi) */
    "0fbaf401",  /* btr $1, %esp */
    "410fba342403",  /* btrl $3, (%r12) */
    "0fbbd2",  /* btc %edx, %edx */
    "0fbb20",  /* btc %esp, (%rax) */
    "410fbafc1f",  /* btc $31, %r12d */
    "0fba3c25ffffff7f03",  /* btcl $3, 0x7fffffff */
    "0fbcd4",  /* bsf %esp, %edx */
    "450fbca45400040000",  /* bsf 1024(%r12,%rdx,2), %r12d */
    "410fbdeb",  /* bsr %r11d, %ebp */
    "470fbd7ca1f8",  /* bsr -8(%r9,%r12,4), %r15d */
    "f3410fbcd1",  /* tzcnt %r9d, %edx */
    "f3410fbc0424",  /* tzcnt (%r12), %eax */
    "f3410fbdd9",  /* lzcnt %r9d, %ebx */
    "67f3410fbd4810",  /* lzcnt 16(%r8d), %ecx */
    "f3440fb8d1",  /* popcnt %ecx, %r10d */
    "f3420fb81c3c",  /* popcnt (%rsp,%r15,1), %ebx */
    "440fa4ef03",  /* shld $3, %r13d, %edi */
    "410fad3424",  /* shrd %cl, %esi, (%r12) */
    "410fa5f4",  /* shld %esi, %r12d */
    "670fc123",  /* xadd %esp, 0(%ebx) */
    "670fb17710",  /* cmpxchg %esi, 16(%edi) */
    "f0440fb14500",  /* lock cmpxchg %r8d, (%rbp) */
    "f00fc13c25ffffff7f",  /* lock xadd %edi, 0x7fffffff */
    "4587ec",  /* xchg %r13d, %r12d */
    "45874500",  /* xchg %r8d, (%r13) */
    "460f38f0740d00",  /* movbe (%rbp,%r9,1), %r14d */
    "410f38f14d00",  /* movbe %ecx, (%r13) */
    "490faffa",  /* imul %r10, %rdi */
    "4c0faf6a01",  /* imul 1(%rdx), %r13 */
    "486bee7f",  /* imul $127, %rsi, %rbp */
    "6749697310e8030000",  /* imul $1000, 16(%r11d), %rsi */
    "4b8d4c9b04",  /* lea 4(%r11,%r11,4), %rcx */
    "480f4ee9",  /* cmovle %rcx, %rbp */
    "4b0f441401",  /* cmove (%r9,%r8,1), %rdx */
    "4c0fa3e2",  /* bt %r12, %rdx */
    "4c0fa32424",  /* bt %r12, (%rsp) */
    "490fbae71f",  /* bt $31, %r15 */
    "490fba2703",  /* btq $3, (%r15) */
    "4d0fabe9",  /* bts %r13, %r9 */
    "4d0fab1488",  /* bts %r10, (%r8,%rcx,4) */
    "490fbaec1f",  /* bts $31, %r12 */
    "490fba2c2403",  /* btsq $3, (%r12) */
    "480fb3c8",  /* btr %rcx, %rax */
    "65480fb31c2500100000",  /* btr %rbx, %gs:0x1000 */
    "490fbaf301",  /* btr $1, %r11 */
    "67490fba3603",  /* btrq $3, 0(%r14d) */
    "4c0fbbd0",  /* btc %r10, %rax */
    "64480fbb2c2500100000",  /* btc %rbp, %fs:0x1000 */
    "480fbafb01",  /* btc $1, %rbx */
    "480fba3c25ffffff7f03",  /* btcq $3, 0x7fffffff */
    "4d0fbcdd",  /* bsf %r13, %r11 */
    "65480fbc142500000000",  /* bsf %gs:0x0, %rdx */
    "4c0fbdc7",  /* bsr %rdi, %r8 */
    "480fbd1425ffffff7f",  /* bsr 0x7fffffff, %rdx */
    "f3480fbcea",  /* tzcnt %rdx, %rbp */
    "f34c0fbc048d00000000",  /* tzcnt 0x0(,%rcx,4), %r8 */
    "f3480fbdf7",  /* lzcnt %rdi, %rsi */
    "f3490fbd887fffffff",  /* lzcnt -129(%r8), %rcx */
    "f3490fb8f1",  /* popcnt %r9, %rsi */
    "f34c0fb80c24",  /* popcnt (%rsp), %r9 */
    "480fa4e103",  /* shld $3, %rsp, %rcx */
    "654c0fad342528000000",  /* shrd %cl, %r14, %gs:0x28 */
    "480fa5e9",  /* shld %rbp, %rcx */
    "4c0fc12c1e",  /* xadd %r13, (%rsi,%rbx,1) */
    "67490fb16610",  /* cmpxchg %rsp, 16(%r14d) */
    "f0490fb133",  /* lock cmpxchg %rsi, (%r11) */
    "67f04c0fc110",  /* lock xadd %r10, 0(%eax) */
    "4c87f9",  /* xchg %r15, %rcx */
    "48872c0500000000",  /* xchg %rbp, 0x0(,%rax,1) */
    "490f38f03424",  /* movbe (%r12), %rsi */
    "480f38f17500",  /* movbe %rsi, (%rbp) */
    "410fcb",  /* bswap %r11d */
    "66f20f38f12c25ffffff7f",  /* crc32w 0x7fffffff, %ebp */
    "f20f38f0c8",  /* crc32b %al, %ecx */
    "0fc7f6",  /* rdrand %esi */
    "410fc7f9",  /* rdseed %r9d */
    "66450f38f6fd",  /* adcx %r13d, %r15d */
    "64f3440f38f62c2500000000",  /* adox %fs:0x0, %r13d */
    "c4c228f2d2",  /* andn %r10d, %r10d, %edx */
    "c46200f23c25ffffff7f",  /* andn 0x7fffffff, %r15d, %r15d */
    "c4c208f7ef",  /* bextr %r14d, %r15d, %ebp */
    "c44238f72424",  /* bextr %r8d, (%r12), %r12d */
    "c44238f5c8",  /* bzhi %r8d, %r8d, %r9d */
    "67c44278f55e10",  /* bzhi %eax, 16(%r14d), %r11d */
    "c4c242f7f6",  /* sarx %edi, %r14d, %esi */
    "65c4e202f71c2500000000",  /* sarx %r15d, %gs:0x0, %ebx */
    "c46221f7c0",  /* shlx %r11d, %eax, %r8d */
    "c46271f70c8510000000",  /* shlx %ecx, 0x10(,%rax,4), %r9d */
    "c4e273f7d6",  /* shrx %ecx, %esi, %edx */
    "c4622bf76a7f",  /* shrx %r10d, 127(%rdx), %r13d */
    "c4c243f5c2",  /* pdep %r10d, %edi, %eax */
    "c46222f5cd",  /* pext %ebp, %r11d, %r9d */
    "c4421bf6e9",  /* mulx %r9d, %r12d, %r13d */
    "c4c268f3ce",  /* blsr %r14d, %edx */
    "c4e210f3d6",  /* blsmsk %esi, %r13d */
    "c4e270f3df",  /* blsi %edi, %ecx */
    "c4e37bf0ff05",  /* rorx $5, %edi, %edi */
    "0fc33c25ffffff7f",  /* movnti %edi, 0x7fffffff */
    "480fcc",  /* bswap %rsp */
    "65f24c0f38f13c2528000000",  /* crc32q %gs:0x28, %r15 */
    "480fc7f5",  /* rdrand %rbp */
    "480fc7fc",  /* rdseed %rsp */
    "664c0f38f6d8",  /* adcx %rax, %r11 */
    "f34c0f38f63c25ffffff7f",  /* adox 0x7fffffff, %r15 */
    "c442e0f2ca",  /* andn %r10, %rbx, %r9 */
    "c4e2b0f21424",  /* andn (%rsp), %r9, %rdx */
    "c4e280f7fb",  /* bextr %r15, %rbx, %rdi */
    "c4e2b8f70c2510000000",  /* bextr %r8, 0x10, %rcx */
    "c4e2e0f5e5",  /* bzhi %rbx, %rbp, %rsp */
    "c4e2b0f502",  /* bzhi %r9, (%rdx), %rax */
    "c4c2f2f7c5",  /* sarx %rcx, %r13, %rax */
    "c462eaf710",  /* sarx %rdx, (%rax), %r10 */
    "c4e2a9f7c0",  /* shlx %r10, %rax, %rax */
    "c462c9f74500",  /* shlx %rsi, (%rbp), %r8 */
    "c4e2c3f7f5",  /* shrx %rdi, %rbp, %rsi */
    "c4629bf737",  /* shrx %r12, (%rdi), %r14 */
    "c462abf5d1",  /* pdep %rcx, %r10, %r10 */
    "c4c2e2f5c5",  /* pext %r13, %rbx, %rax */
    "c4e2ebf6ed",  /* mulx %rbp, %rdx, %rbp */
    "c4e2c0f3ce",  /* blsr %rsi, %rdi */
    "c4e2b0f3d5",  /* blsmsk %rbp, %r9 */
    "c4e2f0f3db",  /* blsi %rbx, %rcx */
    "c4c3fbf0e205",  /* rorx $5, %r10, %rsp */
    "480fc35500",  /* movnti %rdx, (%rbp) */
    "66450fb6d9",  /* movzbw %r9b, %r11w */
    "660fb632",  /* movzbw (%rdx), %si */
    "66400fbeef",  /* movsbw %dil, %bp */
    "660fbe5901",  /* movsbw 1(%rcx), %bx */
    "0fb6fa",  /* movzbl %dl, %edi */
    "420fb60c4d10000000",  /* movzbl 0x10(,%r9,2), %ecx */
    "410fbef0",  /* movsbl %r8b, %esi */
    "65440fbe242500100000",  /* movsbl %gs:0x1000, %r12d */
    "490fb6d4",  /* movzbq %r12b, %rdx */
    "4c0fb6142534120000",  /* movzbq 0x1234, %r10 */
    "480fbedd",  /* movsbq %bpl, %rbx */
    "4e0fbe8caaa0860100",  /* movsbq 100000(%rdx,%r13,4), %r9 */
    "450fb7da",  /* movzwl %r10w, %r11d */
    "440fb7142534120000",  /* movzwl 0x1234, %r10d */
    "450fbffc",  /* movswl %r12w, %r15d */
    "0fbf2c24",  /* movswl (%rsp), %ebp */
    "490fb7e1",  /* movzwq %r9w, %rsp */
    "480fb72424",  /* movzwq (%rsp), %rsp */
    "480fbfec",  /* movswq %sp, %rbp */
    "4d0fbf507f",  /* movswq 127(%r8), %r10 */
    "4c63c5",  /* movslq %ebp, %r8 */
    "4c635d00",  /* movslq (%rbp), %r11 */
    "0fb6c8",  /* movzx %al, %ecx */
    "480fbfd3",  /* movsx %bx, %rdx */
    "4c63c8",  /* movsxd %eax, %r9 */
    "48b8f0debc9a78563412",  /* movabs $0x123456789abcdef0, %rax */
    "49bb0000000001000000",  /* movq $0x100000000, %r11 */
    "88e3",  /* mov %ah, %bl */
    "8828",  /* mov %ch, (%rax) */
    "00f0",  /* add %dh, %al */
    "0fb6c4",  /* movzbl %ah, %eax */
    "50",  /* push %rax */
    "58",  /* pop %rax */
    "51",  /* push %rcx */
    "59",  /* pop %rcx */
    "52",  /* push %rdx */
    "5a",  /* pop %rdx */
    "53",  /* push %rbx */
    "5b",  /* pop %rbx */
    "54",  /* push %rsp */
    "5c",  /* pop %rsp */
    "55",  /* push %rbp */
    "5d",  /* pop %rbp */
    "56",  /* push %rsi */
    "5e",  /* pop %rsi */
    "57",  /* push %rdi */
    "5f",  /* pop %rdi */
    "4150",  /* push %r8 */
    "4158",  /* pop %r8 */
    "4151",  /* push %r9 */
    "4159",  /* pop %r9 */
    "4152",  /* push %r10 */
    "415a",  /* pop %r10 */
    "4153",  /* push %r11 */
    "415b",  /* pop %r11 */
    "4154",  /* push %r12 */
    "415c",  /* pop %r12 */
    "4155",  /* push %r13 */
    "415d",  /* pop %r13 */
    "4156",  /* push %r14 */
    "415e",  /* pop %r14 */
    "4157",  /* push %r15 */
    "415f",  /* pop %r15 */
    "6650",  /* push %ax */
    "6658",  /* pop %ax */
    "6651",  /* push %cx */
    "6659",  /* pop %cx */
    "6652",  /* push %dx */
    "665a",  /* pop %dx */
    "6653",  /* push %bx */
    "665b",  /* pop %bx */
    "6654",  /* push %sp */
    "665c",  /* pop %sp */
    "6655",  /* push %bp */
    "665d",  /* pop %bp */
    "6656",  /* push %si */
    "665e",  /* pop %si */
    "6657",  /* push %di */
    "665f",  /* pop %di */
    "6a01",  /* push $1 */
    "68e8030000",  /* push $1000 */
    "ff30",  /* pushq (%rax) */
    "8f442408",  /* popq 8(%rsp) */
    "666a05",  /* pushw $5 */
    "410f90c6",  /* seto %r14b */
    "0f9044247f",  /* seto 127(%rsp) */
    "0f91c2",  /* setno %dl */
    "430f91040f",  /* setno (%r15,%r9,1) */
    "0f92c1",  /* setb %cl */
    "430f9284a300040000",  /* setb 1024(%r11,%r12,4) */
    "410f93c3",  /* setae %r11b */
    "650f93042500000000",  /* setae %gs:0x0 */
    "410f94c0",  /* sete %r8b */
    "67410f9401",  /* sete 0(%r9d) */
    "410f95c2",  /* setne %r10b */
    "670f954110",  /* setne 16(%ecx) */
    "400f96c7",  /* setbe %dil */
    "0f96045d00000000",  /* setbe 0x0(,%rbx,2) */
    "400f97c7",  /* seta %dil */
    "640f97042500000000",  /* seta %fs:0x0 */
    "0f98c2",  /* sets %dl */
    "420f9804f510000000",  /* sets 0x10(,%r14,8) */
    "400f99c5",  /* setns %bpl */
    "0f990424",  /* setns (%rsp) */
    "0f9ac2",  /* setp %dl */
    "0f9a045d00000000",  /* setp 0x0(,%rbx,2) */
    "400f9bc6",  /* setnp %sil */
    "0f9b4500",  /* setnp (%rbp) */
    "410f9cc3",  /* setl %r11b */
    "420f9c04a9",  /* setl (%rcx,%r13,4) */
    "0f9dc2",  /* setge %dl */
    "670f9d02",  /* setge 0(%edx) */
    "410f9ec6",  /* setle %r14b */
    "410f9e445204",  /* setle 4(%r10,%rdx,2) */
    "410f9fc2",  /* setg %r10b */
    "0f9f4500",  /* setg (%rbp) */
    "90",  /* nop */
    "f390",  /* pause */
    "0fa2",  /* cpuid */
    "0f31",  /* rdtsc */
    "0f01f9",  /* rdtscp */
    "0f05",  /* syscall */
    "0f0b",  /* ud2 */
    "cc",  /* int3 */
    "0faef0",  /* mfence */
    "0faee8",  /* lfence */
    "0faef8",  /* sfence */
    "f8",  /* clc */
    "f9",  /* stc */
    "f5",  /* cmc */
    "fc",  /* cld */
    "fd",  /* std */
    "6698",  /* cbtw */
    "98",  /* cwtl */
    "4898",  /* cltq */
    "6699",  /* cwtd */
    "99",  /* cltd */
    "4899",  /* cqto */
    "9f",  /* lahf */
    "9e",  /* sahf */
    "c9",  /* leave */
    "c3",  /* ret */
    "0f01d0",  /* xgetbv */
    "f30f1efa",  /* endbr64 */
    "9c",  /* pushfq */
    "9d",  /* popfq */
    "c21000",  /* ret $16 */
    "cd80",  /* int $0x80 */
    "c8100000",  /* enter $16, $0 */
    "a4",  /* movsb */
    "f3a4",  /* rep movsb */
    "66a5",  /* movsw */
    "66f3a5",  /* rep movsw */
    "a5",  /* movsl */
    "f3a5",  /* rep movsl */
    "48a5",  /* movsq */
    "f348a5",  /* rep movsq */
    "a6",  /* cmpsb */
    "f3a6",  /* rep cmpsb */
    "66a7",  /* cmpsw */
    "66f3a7",  /* rep cmpsw */
    "a7",  /* cmpsl */
    "f3a7",  /* rep cmpsl */
    "48a7",  /* cmpsq */
    "f348a7",  /* rep cmpsq */
    "aa",  /* stosb */
    "f3aa",  /* rep stosb */
    "66ab",  /* stosw */
    "66f3ab",  /* rep stosw */
    "ab",  /* stosl */
    "f3ab",  /* rep stosl */
    "48ab",  /* stosq */
    "f348ab",  /* rep stosq */
    "ac",  /* lodsb */
    "f3ac",  /* rep lodsb */
    "66ad",  /* lodsw */
    "66f3ad",  /* rep lodsw */
    "ad",  /* lodsl */
    "f3ad",  /* rep lodsl */
    "48ad",  /* lodsq */
    "f348ad",  /* rep lodsq */
    "ae",  /* scasb */
    "f3ae",  /* rep scasb */
    "66af",  /* scasw */
    "66f3af",  /* rep scasw */
    "af",  /* scasl */
    "f3af",  /* rep scasl */
    "48af",  /* scasq */
    "f348af",  /* rep scasq */
    "f2ae",  /* repne scasb */
    "f3a6",  /* repe cmpsb */
    "660f1f0400",  /* nopw 0(%rax,%rax,1) */
    "0f1f00",  /* nopl 0x0(%rax) */
    "2e660f1f0400",  /* nopw %cs:0(%rax,%rax,1) */
    "0f1808",  /* prefetcht0 (%rax) */
    "0f184740",  /* prefetchnta 64(%rdi) */
    "0f0d0e",  /* prefetchw (%rsi) */
    "0fae38",  /* clflush (%rax) */
    "0fc708",  /* cmpxchg8b (%rax) */
    "480fc74f08",  /* cmpxchg16b 8(%rdi) */
    "f0490fc708",  /* lock cmpxchg16b (%r8) */
    "ffe0",  /* jmp *%rax */
    "ff20",  /* jmp *(%rax) */
    "41ffd3",  /* call *%r11 */
    "ff54d808",  /* call *8(%rax,%rbx,8) */
    "41ffe7",  /* jmp *%r15 */
    "0fae10",  /* ldmxcsr (%rax) */
    "0fae5c24fc",  /* stmxcsr -4(%rsp) */
    "450f58fb",  /* addps %xmm11, %xmm15 */
    "440f581424",  /* addps (%rsp), %xmm10 */
    "66450f58d1",  /* addpd %xmm9, %xmm10 */
    "6766410f585510",  /* addpd 16(%r13d), %xmm2 */
    "f3410f58d0",  /* addss %xmm8, %xmm2 */
    "65f3440f58142500000000",  /* addss %gs:0x0, %xmm10 */
    "f2450f58c0",  /* addsd %xmm8, %xmm8 */
    "f20f582cbd00000000",  /* addsd 0x0(,%rdi,4), %xmm5 */
    "410f59c9",  /* mulps %xmm9, %xmm1 */
    "410f59b588a9cbed",  /* mulps -305419896(%r13), %xmm6 */
    "66450f59d8",  /* mulpd %xmm8, %xmm11 */
    "6766410f596810",  /* mulpd 16(%r8d), %xmm5 */
    "f3410f59e2",  /* mulss %xmm10, %xmm4 */
    "65f30f59342500100000",  /* mulss %gs:0x1000, %xmm6 */
    "f2450f59f4",  /* mulsd %xmm12, %xmm14 */
    "f2430f590c67",  /* mulsd (%r15,%r12,2), %xmm1 */
    "0f5cd7",  /* subps %xmm7, %xmm2 */
    "440f5cb934120000",  /* subps 4660(%rcx), %xmm15 */
    "66450f5cd7",  /* subpd %xmm15, %xmm10 */
    "66450f5c2424",  /* subpd (%r12), %xmm12 */
    "f3440f5ce0",  /* subss %xmm0, %xmm12 */
    "f3450f5c1c24",  /* subss (%r12), %xmm11 */
    "f2440f5cd9",  /* subsd %xmm1, %xmm11 */
    "65f2440f5c342500000000",  /* subsd %gs:0x0, %xmm14 */
    "410f5df3",  /* minps %xmm11, %xmm6 */
    "460f5d1c04",  /* minps (%rsp,%r8,1), %xmm11 */
    "66410f5dea",  /* minpd %xmm10, %xmm5 */
    "67660f5d0f",  /* minpd 0(%edi), %xmm1 */
    "f3410f5dee",  /* minss %xmm14, %xmm5 */
    "f3410f5d1c24",  /* minss (%r12), %xmm3 */
    "f2450f5dc2",  /* minsd %xmm10, %xmm8 */
    "f20f5d6c7500",  /* minsd (%rbp,%rsi,2), %xmm5 */
    "410f5ef9",  /* divps %xmm9, %xmm7 */
    "440f5e0c24",  /* divps (%rsp), %xmm9 */
    "66440f5ecd",  /* divpd %xmm5, %xmm9 */
    "6766440f5e10",  /* divpd 0(%eax), %xmm10 */
    "f3440f5eeb",  /* divss %xmm3, %xmm13 */
    "f3440f5e342534120000",  /* divss 0x1234, %xmm14 */
    "f2450f5ede",  /* divsd %xmm14, %xmm11 */
    "f2440f5e640904",  /* divsd 4(%rcx,%rcx,1), %xmm12 */
    "440f5ff4",  /* maxps %xmm4, %xmm14 */
    "0f5f0c2534120000",  /* maxps 0x1234, %xmm1 */
    "66440f5ff6",  /* maxpd %xmm6, %xmm14 */
    "66410f5f6d00",  /* maxpd (%r13), %xmm5 */
    "f3410f5fe8",  /* maxss %xmm8, %xmm5 */
    "f30f5f11",  /* maxss (%rcx), %xmm2 */
    "f2440f5ff7",  /* maxsd %xmm7, %xmm14 */
    "64f20f5f0c2528000000",  /* maxsd %fs:0x28, %xmm1 */
    "450f51c6",  /* sqrtps %xmm14, %xmm8 */
    "0f511c25ffffff7f",  /* sqrtps 0x7fffffff, %xmm3 */
    "66440f51e5",  /* sqrtpd %xmm5, %xmm12 */
    "66450f513c24",  /* sqrtpd (%r12), %xmm15 */
    "f3440f51d4",  /* sqrtss %xmm4, %xmm10 */
    "f3410f513c53",  /* sqrtss (%r11,%rdx,2), %xmm7 */
    "f2440f51f6",  /* sqrtsd %xmm6, %xmm14 */
    "f2460f512c23",  /* sqrtsd (%rbx,%r12,1), %xmm13 */
    "440f28f1",  /* movaps %xmm1, %xmm14 */
    "470f28048b",  /* movaps (%r11,%r9,4), %xmm8 */
    "450f292424",  /* movaps %xmm12, (%r12) */
    "66450f28ce",  /* movapd %xmm14, %xmm9 */
    "660f281c7510000000",  /* movapd 0x10(,%rsi,2), %xmm3 */
    "66450f293cd6",  /* movapd %xmm15, (%r14,%rdx,8) */
    "0f10db",  /* movups %xmm3, %xmm3 */
    "450f103c24",  /* movups (%r12), %xmm15 */
    "67450f115210",  /* movups %xmm10, 16(%r10d) */
    "66450f10e7",  /* movupd %xmm15, %xmm12 */
    "660f103424",  /* movupd (%rsp), %xmm6 */
    "66410f113a",  /* movupd %xmm7, (%r10) */
    "f3440f10d1",  /* movss %xmm1, %xmm10 */
    "f3440f101c2534120000",  /* movss 0x1234, %xmm11 */
    "f3470f116cb9f8",  /* movss %xmm13, -8(%r9,%r15,4) */
    "f2450f10e4",  /* movsd %xmm12, %xmm12 */
    "f20f103cb8",  /* movsd (%rax,%rdi,4), %xmm7 */
    "f2410f117a80",  /* movsd %xmm7, -128(%r10) */
    "66410f6fd9",  /* movdqa %xmm9, %xmm3 */
    "660f6f142534120000",  /* movdqa 0x1234, %xmm2 */
    "66460f7f2c4b",  /* movdqa %xmm13, (%rbx,%r9,2) */
    "f3410f6fe2",  /* movdqu %xmm10, %xmm4 */
    "f3440f6f0425ffffff7f",  /* movdqu 0x7fffffff, %xmm8 */
    "f30f7f7455f8",  /* movdqu %xmm6, -8(%rbp,%rdx,2) */
    "410f16a634120000",  /* movhps 4660(%r14), %xmm4 */
    "470f172c92",  /* movhps %xmm13, (%r10,%r10,4) */
    "670f127d10",  /* movlps 16(%ebp), %xmm7 */
    "410f137d00",  /* movlps %xmm7, (%r13) */
    "660f16a4247fffffff",  /* movhpd -129(%rsp), %xmm4 */
    "65660f17342500000000",  /* movhpd %xmm6, %gs:0x0 */
    "66440f123425ffffff7f",  /* movlpd 0x7fffffff, %xmm14 */
    "66420f1374a604",  /* movlpd %xmm6, 4(%rsi,%r12,4) */
    "440f12c1",  /* movhlps %xmm1, %xmm8 */
    "0f16d6",  /* movlhps %xmm6, %xmm2 */
    "0f14db",  /* unpcklps %xmm3, %xmm3 */
    "440f143c2534120000",  /* unpcklps 0x1234, %xmm15 */
    "410f15e0",  /* unpckhps %xmm8, %xmm4 */
    "440f15242534120000",  /* unpckhps 0x1234, %xmm12 */
    "66450f14ed",  /* unpcklpd %xmm13, %xmm13 */
    "64660f143c2500000000",  /* unpcklpd %fs:0x0, %xmm7 */
    "660f15c7",  /* unpckhpd %xmm7, %xmm0 */
    "66430f158c04a0860100",  /* unpckhpd 100000(%r12,%r8,1), %xmm1 */
    "0f54c5",  /* andps %xmm5, %xmm0 */
    "450f544500",  /* andps (%r13), %xmm8 */
    "66410f54e5",  /* andpd %xmm13, %xmm4 */
    "66430f548cbba0860100",  /* andpd 100000(%r11,%r15,4), %xmm1 */
    "410f55e6",  /* andnps %xmm14, %xmm4 */
    "0f551c24",  /* andnps (%rsp), %xmm3 */
    "660f55df",  /* andnpd %xmm7, %xmm3 */
    "66440f551c2510000000",  /* andnpd 0x10, %xmm11 */
    "0f56e4",  /* orps %xmm4, %xmm4 */
    "460f563c0500100000",  /* orps 0x1000(,%r8,1), %xmm15 */
    "66440f56ea",  /* orpd %xmm2, %xmm13 */
    "66410f569f88a9cbed",  /* orpd -305419896(%r15), %xmm3 */
    "440f57e4",  /* xorps %xmm4, %xmm12 */
    "67450f575710",  /* xorps 16(%r15d), %xmm10 */
    "66410f57de",  /* xorpd %xmm14, %xmm3 */
    "66440f579e88a9cbed",  /* xorpd -305419896(%rsi), %xmm11 */
    "0f2fdb",  /* comiss %xmm3, %xmm3 */
    "640f2f142500000000",  /* comiss %fs:0x0, %xmm2 */
    "66440f2fe8",  /* comisd %xmm0, %xmm13 */
    "66440f2fac2aa0860100",  /* comisd 100000(%rdx,%rbp,1), %xmm13 */
    "450f2ee1",  /* ucomiss %xmm9, %xmm12 */
    "440f2e38",  /* ucomiss (%rax), %xmm15 */
    "66410f2efe",  /* ucomisd %xmm14, %xmm7 */
    "66420f2e0ced10000000",  /* ucomisd 0x10(,%r13,8), %xmm1 */
    "410f5ae8",  /* cvtps2pd %xmm8, %xmm5 */
    "410f5a6d00",  /* cvtps2pd (%r13), %xmm5 */
    "66450f5ae2",  /* cvtpd2ps %xmm10, %xmm12 */
    "660f5a3c25ffffff7f",  /* cvtpd2ps 0x7fffffff, %xmm7 */
    "f3410f5ac1",  /* cvtss2sd %xmm9, %xmm0 */
    "f3440f5a5c24ff",  /* cvtss2sd -1(%rsp), %xmm11 */
    "f2450f5ac8",  /* cvtsd2ss %xmm8, %xmm9 */
    "f20f5a2424",  /* cvtsd2ss (%rsp), %xmm4 */
    "410f5bce",  /* cvtdq2ps %xmm14, %xmm1 */
    "450f5b5500",  /* cvtdq2ps (%r13), %xmm10 */
    "66410f5be3",  /* cvtps2dq %xmm11, %xmm4 */
    "66440f5b06",  /* cvtps2dq (%rsi), %xmm8 */
    "f3440f5bf2",  /* cvttps2dq %xmm2, %xmm14 */
    "f3460f5b241d10000000",  /* cvttps2dq 0x10(,%r11,1), %xmm12 */
    "f30fe6db",  /* cvtdq2pd %xmm3, %xmm3 */
    "65f3440fe6142500100000",  /* cvtdq2pd %gs:0x1000, %xmm10 */
    "f20fe6f1",  /* cvtpd2dq %xmm1, %xmm6 */
    "f2420fe60c11",  /* cvtpd2dq (%rcx,%r10,1), %xmm1 */
    "66440fe6cb",  /* cvttpd2dq %xmm3, %xmm9 */
    "66420fe6744df8",  /* cvttpd2dq -8(%rbp,%r9,2), %xmm6 */
    "440f53da",  /* rcpps %xmm2, %xmm11 */
    "67410f5339",  /* rcpps 0(%r9d), %xmm7 */
    "f3440f53c2",  /* rcpss %xmm2, %xmm8 */
    "f3440f5308",  /* rcpss (%rax), %xmm9 */
    "450f52ef",  /* rsqrtps %xmm15, %xmm13 */
    "0f520c2510000000",  /* rsqrtps 0x10, %xmm1 */
    "f3450f52ed",  /* rsqrtss %xmm13, %xmm13 */
    "64f30f523c2500000000",  /* rsqrtss %fs:0x0, %xmm7 */
    "f20f12cf",  /* movddup %xmm7, %xmm1 */
    "65f20f12342528000000",  /* movddup %gs:0x28, %xmm6 */
    "f3440f16c7",  /* movshdup %xmm7, %xmm8 */
    "f3410f1604cc",  /* movshdup (%r12,%rcx,8), %xmm0 */
    "f3450f12cf",  /* movsldup %xmm15, %xmm9 */
    "f3460f12247d00000000",  /* movsldup 0x0(,%r15,2), %xmm12 */
    "450fc2f400",  /* cmpps $0, %xmm12, %xmm14 */
    "440fc23c2403",  /* cmpps $3, (%rsp), %xmm15 */
    "66450fc2e900",  /* cmppd $0, %xmm9, %xmm13 */
    "66450fc27c4af803",  /* cmppd $3, -8(%r10,%rcx,2), %xmm15 */
    "f3440fc2fd00",  /* cmpss $0, %xmm5, %xmm15 */
    "65f3440fc224250000000003",  /* cmpss $3, %gs:0x0, %xmm12 */
    "f2410fc2e501",  /* cmpsd $1, %xmm13, %xmm4 */
    "f2450fc22903",  /* cmpsd $3, (%r9), %xmm13 */
    "440fc6c600",  /* shufps $0, %xmm6, %xmm8 */
    "410fc65d0003",  /* shufps $3, (%r13), %xmm3 */
    "66440fc6c404",  /* shufpd $4, %xmm4, %xmm8 */
    "66440fc61b03",  /* shufpd $3, (%rbx), %xmm11 */
    "66410f70d81b",  /* pshufd $27, %xmm8, %xmm3 */
    "66460f7024ed0000000003",  /* pshufd $3, 0x0(,%r13,8), %xmm12 */
    "f3410f70e700",  /* pshufhw $0, %xmm15, %xmm4 */
    "f3410f703c2403",  /* pshufhw $3, (%r12), %xmm7 */
    "f2410f70cc00",  /* pshuflw $0, %xmm12, %xmm1 */
    "f20f700c25ffffff7f03",  /* pshuflw $3, 0x7fffffff, %xmm1 */
    "66440f3a08ca1b",  /* roundps $27, %xmm2, %xmm9 */
    "67660f3a081203",  /* roundps $3, 0(%edx), %xmm2 */
    "66450f3a09d900",  /* roundpd $0, %xmm9, %xmm11 */
    "66460f3a09148d1000000003",  /* roundpd $3, 0x10(,%r9,4), %xmm10 */
    "66410f3a0afb01",  /* roundss $1, %xmm11, %xmm7 */
    "6466440f3a0a0c252800000003",  /* roundss $3, %fs:0x28, %xmm9 */
    "66410f3a0bc104",  /* roundsd $4, %xmm9, %xmm0 */
    "66410f3a0b7d0003",  /* roundsd $3, (%r13), %xmm7 */
    "660f3a0cef01",  /* blendps $1, %xmm7, %xmm5 */
    "66410f3a0c608003",  /* blendps $3, -128(%r8), %xmm4 */
    "66440f3a0dd201",  /* blendpd $1, %xmm2, %xmm10 */
    "66440f3a0d34251000000003",  /* blendpd $3, 0x10, %xmm14 */
    "66410f3a0edc01",  /* pblendw $1, %xmm12, %xmm3 */
    "66460f3a0e34a50010000003",  /* pblendw $3, 0x1000(,%r12,4), %xmm14 */
    "660f3a0fd204",  /* palignr $4, %xmm2, %xmm2 */
    "66440f3a0fb73412000003",  /* palignr $3, 4660(%rdi), %xmm14 */
    "660f3a40e81b",  /* dpps $27, %xmm0, %xmm5 */
    "660f3a40450003",  /* dpps $3, (%rbp), %xmm0 */
    "66410f3a44f101",  /* pclmulqdq $1, %xmm9, %xmm6 */
    "66440f3a441c2403",  /* pclmulqdq $3, (%rsp), %xmm11 */
    "66410f3a21c904",  /* insertps $4, %xmm9, %xmm1 */
    "66440f3a213003",  /* insertps $3, (%rax), %xmm14 */
    "0fc2f100",  /* cmpeqps %xmm1, %xmm6 */
    "66440fc2d200",  /* cmpeqpd %xmm2, %xmm10 */
    "f3450fc2ce00",  /* cmpeqss %xmm14, %xmm9 */
    "f20fc2fb00",  /* cmpeqsd %xmm3, %xmm7 */
    "0fc2f301",  /* cmpltps %xmm3, %xmm6 */
    "660fc2f001",  /* cmpltpd %xmm0, %xmm6 */
    "f30fc2c401",  /* cmpltss %xmm4, %xmm0 */
    "f2410fc2c601",  /* cmpltsd %xmm14, %xmm0 */
    "440fc2ff02",  /* cmpleps %xmm7, %xmm15 */
    "660fc2c502",  /* cmplepd %xmm5, %xmm0 */
    "f30fc2e702",  /* cmpless %xmm7, %xmm4 */
    "f20fc2c202",  /* cmplesd %xmm2, %xmm0 */
    "440fc2d403",  /* cmpunordps %xmm4, %xmm10 */
    "66440fc2c203",  /* cmpunordpd %xmm2, %xmm8 */
    "f3440fc2e603",  /* cmpunordss %xmm6, %xmm12 */
    "f2440fc2c003",  /* cmpunordsd %xmm0, %xmm8 */
    "450fc2c304",  /* cmpneqps %xmm11, %xmm8 */
    "66450fc2e404",  /* cmpneqpd %xmm12, %xmm12 */
    "f3450fc2c604",  /* cmpneqss %xmm14, %xmm8 */
    "f20fc2ea04",  /* cmpneqsd %xmm2, %xmm5 */
    "450fc2e705",  /* cmpnltps %xmm15, %xmm12 */
    "660fc2f405",  /* cmpnltpd %xmm4, %xmm6 */
    "f30fc2c805",  /* cmpnltss %xmm0, %xmm1 */
    "f2410fc2e205",  /* cmpnltsd %xmm10, %xmm4 */
    "440fc2d706",  /* cmpnleps %xmm7, %xmm10 */
    "66410fc2cc06",  /* cmpnlepd %xmm12, %xmm1 */
    "f3450fc2fd06",  /* cmpnless %xmm13, %xmm15 */
    "f20fc2ca06",  /* cmpnlesd %xmm2, %xmm1 */
    "440fc2ec07",  /* cmpordps %xmm4, %xmm13 */
    "66450fc2c407",  /* cmpordpd %xmm12, %xmm8 */
    "f30fc2f107",  /* cmpordss %xmm1, %xmm6 */
    "f2440fc2ce07",  /* cmpordsd %xmm6, %xmm9 */
    "66450f60cc",  /* punpcklbw %xmm12, %xmm9 */
    "66410f603424",  /* punpcklbw (%r12), %xmm6 */
    "660f61fd",  /* punpcklwd %xmm5, %xmm7 */
    "660f614e08",  /* punpcklwd 8(%rsi), %xmm1 */
    "66450f62cc",  /* punpckldq %xmm12, %xmm9 */
    "660f621c24",  /* punpckldq (%rsp), %xmm3 */
    "66440f63e9",  /* packsswb %xmm1, %xmm13 */
    "66440f637500",  /* packsswb (%rbp), %xmm14 */
    "66440f64e3",  /* pcmpgtb %xmm3, %xmm12 */
    "66440f64bcd200040000",  /* pcmpgtb 1024(%rdx,%rdx,8), %xmm15 */
    "66450f65eb",  /* pcmpgtw %xmm11, %xmm13 */
    "6766450f656010",  /* pcmpgtw 16(%r8d), %xmm12 */
    "66450f66e3",  /* pcmpgtd %xmm11, %xmm12 */
    "6766440f665d00",  /* pcmpgtd 0(%ebp), %xmm11 */
    "660f67c2",  /* packuswb %xmm2, %xmm0 */
    "660f671c2534120000",  /* packuswb 0x1234, %xmm3 */
    "66440f68f7",  /* punpckhbw %xmm7, %xmm14 */
    "65660f680c2500100000",  /* punpckhbw %gs:0x1000, %xmm1 */
    "66450f69d0",  /* punpckhwd %xmm8, %xmm10 */
    "66440f6974bd00",  /* punpckhwd (%rbp,%rdi,4), %xmm14 */
    "66440f6aff",  /* punpckhdq %xmm7, %xmm15 */
    "660f6a1c2534120000",  /* punpckhdq 0x1234, %xmm3 */
    "66450f6bc1",  /* packssdw %xmm9, %xmm8 */
    "660f6b2a",  /* packssdw (%rdx), %xmm5 */
    "66440f6ccf",  /* punpcklqdq %xmm7, %xmm9 */
    "66440f6c657f",  /* punpcklqdq 127(%rbp), %xmm12 */
    "66450f6dfe",  /* punpckhqdq %xmm14, %xmm15 */
    "66410f6d4fff",  /* punpckhqdq -1(%r15), %xmm1 */
    "660f74c1",  /* pcmpeqb %xmm1, %xmm0 */
    "66440f743c6d10000000",  /* pcmpeqb 0x10(,%rbp,2), %xmm15 */
    "66410f75c2",  /* pcmpeqw %xmm10, %xmm0 */
    "66410f757d00",  /* pcmpeqw (%r13), %xmm7 */
    "66410f76cb",  /* pcmpeqd %xmm11, %xmm1 */
    "66410f7636",  /* pcmpeqd (%r14), %xmm6 */
    "66410fd4e4",  /* paddq %xmm12, %xmm4 */
    "660fd40ccf",  /* paddq (%rdi,%rcx,8), %xmm1 */
    "66440fd5d5",  /* pmullw %xmm5, %xmm10 */
    "66410fd52e",  /* pmullw (%r14), %xmm5 */
    "66440fd8e9",  /* psubusb %xmm1, %xmm13 */
    "66430fd88c58a0860100",  /* psubusb 100000(%r8,%r11,2), %xmm1 */
    "66450fd9ec",  /* psubusw %xmm12, %xmm13 */
    "66440fd9142534120000",  /* psubusw 0x1234, %xmm10 */
    "660fdad0",  /* pminub %xmm0, %xmm2 */
    "66410fda6d00",  /* pminub (%r13), %xmm5 */
    "66440fdbc5",  /* pand %xmm5, %xmm8 */
    "66440fdb342534120000",  /* pand 0x1234, %xmm14 */
    "66450fdce4",  /* paddusb %xmm12, %xmm12 */
    "66440fdc047500100000",  /* paddusb 0x1000(,%rsi,2), %xmm8 */
    "660fddd0",  /* paddusw %xmm0, %xmm2 */
    "66440fdd3c9510000000",  /* paddusw 0x10(,%rdx,4), %xmm15 */
    "660fdee8",  /* pmaxub %xmm0, %xmm5 */
    "66440fde6500",  /* pmaxub (%rbp), %xmm12 */
    "660fdfe9",  /* pandn %xmm1, %xmm5 */
    "66440fdfb334120000",  /* pandn 4660(%rbx), %xmm14 */
    "660fe0c8",  /* pavgb %xmm0, %xmm1 */
    "66460fe01c4d10000000",  /* pavgb 0x10(,%r9,2), %xmm11 */
    "66410fe3de",  /* pavgw %xmm14, %xmm3 */
    "6566440fe3242500100000",  /* pavgw %gs:0x1000, %xmm12 */
    "66410fe4d1",  /* pmulhuw %xmm9, %xmm2 */
    "66470fe4acce00040000",  /* pmulhuw 1024(%r14,%r9,8), %xmm13 */
    "66450fe5c5",  /* pmulhw %xmm13, %xmm8 */
    "66460fe51434",  /* pmulhw (%rsp,%r14,1), %xmm10 */
    "66450fe8e3",  /* psubsb %xmm11, %xmm12 */
    "66410fe86208",  /* psubsb 8(%r10), %xmm4 */
    "660fe9f3",  /* psubsw %xmm3, %xmm6 */
    "66410fe96d00",  /* psubsw (%r13), %xmm5 */
    "66440feace",  /* pminsw %xmm6, %xmm9 */
    "66460fea2cdc",  /* pminsw (%rsp,%r11,8), %xmm13 */
    "66450febdf",  /* por %xmm15, %xmm11 */
    "66440feb1a",  /* por (%rdx), %xmm11 */
    "660fece7",  /* paddsb %xmm7, %xmm4 */
    "66470fec94c400040000",  /* paddsb 1024(%r12,%r8,8), %xmm10 */
    "66450fedd7",  /* paddsw %xmm15, %xmm10 */
    "66440fed7908",  /* paddsw 8(%rcx), %xmm15 */
    "660feed5",  /* pmaxsw %xmm5, %xmm2 */
    "660fee2a",  /* pmaxsw (%rdx), %xmm5 */
    "66410feff0",  /* pxor %xmm8, %xmm6 */
    "66450fef4500",  /* pxor (%r13), %xmm8 */
    "66450ff4e0",  /* pmuludq %xmm8, %xmm12 */
    "66410ff4542480",  /* pmuludq -128(%r12), %xmm2 */
    "66410ff5e2",  /* pmaddwd %xmm10, %xmm4 */
    "6766440ff54910",  /* pmaddwd 16(%ecx), %xmm9 */
    "66410ff6c3",  /* psadbw %xmm11, %xmm0 */
    "6766450ff622",  /* psadbw 0(%r10d), %xmm12 */
    "66440ff8f1",  /* psubb %xmm1, %xmm14 */
    "66410ff845ff",  /* psubb -1(%r13), %xmm0 */
    "66440ff9e8",  /* psubw %xmm0, %xmm13 */
    "6466440ff90c2500000000",  /* psubw %fs:0x0, %xmm9 */
    "66410ffaed",  /* psubd %xmm13, %xmm5 */
    "6766410ffa4810",  /* psubd 16(%r8d), %xmm1 */
    "66450ffbec",  /* psubq %xmm12, %xmm13 */
    "66450ffb24d2",  /* psubq (%r10,%rdx,8), %xmm12 */
    "660ffcd4",  /* paddb %xmm4, %xmm2 */
    "6766410ffc4f10",  /* paddb 16(%r15d), %xmm1 */
    "66410ffde9",  /* paddw %xmm9, %xmm5 */
    "660ffd04ad00000000",  /* paddw 0x0(,%rbp,4), %xmm0 */
    "660ffedb",  /* paddd %xmm3, %xmm3 */
    "660ffe7d00",  /* paddd (%rbp), %xmm7 */
    "660f3800f9",  /* pshufb %xmm1, %xmm7 */
    "660f38004bff",  /* pshufb -1(%rbx), %xmm1 */
    "660f3801cb",  /* phaddw %xmm3, %xmm1 */
    "660f38010c1510000000",  /* phaddw 0x10(,%rdx,1), %xmm1 */
    "66410f3802fc",  /* phaddd %xmm12, %xmm7 */
    "66470f38021c7f",  /* phaddd (%r15,%r15,2), %xmm11 */
    "66410f3804ec",  /* pmaddubsw %xmm12, %xmm5 */
    "6466440f38040c2500000000",  /* pmaddubsw %fs:0x0, %xmm9 */
    "66440f3808f3",  /* psignb %xmm3, %xmm14 */
    "66440f38084001",  /* psignb 1(%rax), %xmm8 */
    "66450f380bc1",  /* pmulhrsw %xmm9, %xmm8 */
    "66410f380b7d00",  /* pmulhrsw (%r13), %xmm7 */
    "66410f381ce1",  /* pabsb %xmm9, %xmm4 */
    "66450f381c3424",  /* pabsb (%r12), %xmm14 */
    "66440f381deb",  /* pabsw %xmm3, %xmm13 */
    "66450f381d0488",  /* pabsw (%r8,%rcx,4), %xmm8 */
    "66440f381ed6",  /* pabsd %xmm6, %xmm10 */
    "67660f381e38",  /* pabsd 0(%eax), %xmm7 */
    "66450f3820df",  /* pmovsxbw %xmm15, %xmm11 */
    "66470f38206c9d00",  /* pmovsxbw (%r13,%r11,4), %xmm13 */
    "66410f3831de",  /* pmovzxbd %xmm14, %xmm3 */
    "66420f3831348d10000000",  /* pmovzxbd 0x10(,%r9,4), %xmm6 */
    "660f3835fe",  /* pmovzxdq %xmm6, %xmm7 */
    "66450f3835bc3400040000",  /* pmovzxdq 1024(%r12,%rsi,1), %xmm15 */
    "66440f3828ec",  /* pmuldq %xmm4, %xmm13 */
    "66450f38284500",  /* pmuldq (%r13), %xmm8 */
    "660f3829fb",  /* pcmpeqq %xmm3, %xmm7 */
    "66410f38295d08",  /* pcmpeqq 8(%r13), %xmm3 */
    "66410f382bf6",  /* packusdw %xmm14, %xmm6 */
    "66440f382b14ae",  /* packusdw (%rsi,%rbp,4), %xmm10 */
    "66450f3837c3",  /* pcmpgtq %xmm11, %xmm8 */
    "6766450f383738",  /* pcmpgtq 0(%r8d), %xmm15 */
    "66440f3838d0",  /* pminsb %xmm0, %xmm10 */
    "660f38387500",  /* pminsb (%rbp), %xmm6 */
    "66410f3839f8",  /* pminsd %xmm8, %xmm7 */
    "66450f38395d7f",  /* pminsd 127(%r13), %xmm11 */
    "660f383ade",  /* pminuw %xmm6, %xmm3 */
    "66450f383a1424",  /* pminuw (%r12), %xmm10 */
    "66450f383bd5",  /* pminud %xmm13, %xmm10 */
    "6766410f383b5310",  /* pminud 16(%r11d), %xmm2 */
    "66410f383cfd",  /* pmaxsb %xmm13, %xmm7 */
    "6766410f383c4110",  /* pmaxsb 16(%r9d), %xmm0 */
    "660f383dcb",  /* pmaxsd %xmm3, %xmm1 */
    "66460f383d0cef",  /* pmaxsd (%rdi,%r13,8), %xmm9 */
    "66450f383ee5",  /* pmaxuw %xmm13, %xmm12 */
    "67660f383e742410",  /* pmaxuw 16(%esp), %xmm6 */
    "66450f383fe7",  /* pmaxud %xmm15, %xmm12 */
    "66410f383f6d00",  /* pmaxud (%r13), %xmm5 */
    "66410f3840f7",  /* pmulld %xmm15, %xmm6 */
    "66420f384024b510000000",  /* pmulld 0x10(,%r14,4), %xmm4 */
    "66450f3817c8",  /* ptest %xmm8, %xmm9 */
    "66440f38175500",  /* ptest (%rbp), %xmm10 */
    "66440f3841c6",  /* phminposuw %xmm6, %xmm8 */
    "66450f384109",  /* phminposuw (%r9), %xmm9 */
    "66450f38dccf",  /* aesenc %xmm15, %xmm9 */
    "660f38dc24ad10000000",  /* aesenc 0x10(,%rbp,4), %xmm4 */
    "66450f38dde0",  /* aesenclast %xmm8, %xmm12 */
    "66440f38dd6500",  /* aesenclast (%rbp), %xmm12 */
    "660f38ded1",  /* aesdec %xmm1, %xmm2 */
    "6766440f38de4910",  /* aesdec 16(%ecx), %xmm9 */
    "66440f38dfe9",  /* aesdeclast %xmm1, %xmm13 */
    "66450f38df5a01",  /* aesdeclast 1(%r10), %xmm11 */
    "66440f38dbdf",  /* aesimc %xmm7, %xmm11 */
    "660f38db1424",  /* aesimc (%rsp), %xmm2 */
    "f2440f7cd1",  /* haddps %xmm1, %xmm10 */
    "f2410f7c2e",  /* haddps (%r14), %xmm5 */
    "66410f7df0",  /* hsubpd %xmm8, %xmm6 */
    "660f7d0c2510000000",  /* hsubpd 0x10, %xmm1 */
    "f2440fd0f9",  /* addsubps %xmm1, %xmm15 */
    "f2440fd02c2534120000",  /* addsubps 0x1234, %xmm13 */
    "660f71d701",  /* psrlw $1, %xmm7 */
    "66450fd1ea",  /* psrlw %xmm10, %xmm13 */
    "6766440fd112",  /* psrlw 0(%edx), %xmm10 */
    "66410f72d41f",  /* psrld $31, %xmm12 */
    "660fd2fd",  /* psrld %xmm5, %xmm7 */
    "66440fd22c24",  /* psrld (%rsp), %xmm13 */
    "66410f73d41f",  /* psrlq $31, %xmm12 */
    "66440fd3c6",  /* psrlq %xmm6, %xmm8 */
    "66450fd301",  /* psrlq (%r9), %xmm8 */
    "660f71e501",  /* psraw $1, %xmm5 */
    "66450fe1f0",  /* psraw %xmm8, %xmm14 */
    "660fe10c2534120000",  /* psraw 0x1234, %xmm1 */
    "660f72e507",  /* psrad $7, %xmm5 */
    "66410fe2f0",  /* psrad %xmm8, %xmm6 */
    "66420fe204d9",  /* psrad (%rcx,%r11,8), %xmm0 */
    "66410f71f11f",  /* psllw $31, %xmm9 */
    "66440ff1e0",  /* psllw %xmm0, %xmm12 */
    "64660ff1342500100000",  /* psllw %fs:0x1000, %xmm6 */
    "660f72f501",  /* pslld $1, %xmm5 */
    "66410ff2c4",  /* pslld %xmm12, %xmm0 */
    "66470ff26c1c04",  /* pslld 4(%r12,%r11,1), %xmm13 */
    "660f73f51f",  /* psllq $31, %xmm5 */
    "66440ff3e1",  /* psllq %xmm1, %xmm12 */
    "660ff3142510000000",  /* psllq 0x10, %xmm2 */
    "660f73db04",  /* psrldq $4, %xmm3 */
    "66410f73fc08",  /* pslldq $8, %xmm12 */
    "660f6ec8",  /* movd %eax, %xmm1 */
    "66410f7ed1",  /* movd %xmm2, %r9d */
    "66440f6e08",  /* movd (%rax), %xmm9 */
    "66440f7e4c2408",  /* movd %xmm9, 8(%rsp) */
    "66480f6ec8",  /* movq %rax, %xmm1 */
    "66490f7ed1",  /* movq %xmm2, %r9 */
    "f3440f7e08",  /* movq (%rax), %xmm9 */
    "66440fd64c2408",  /* movq %xmm9, 8(%rsp) */
    "f3440f7ee3",  /* movq %xmm3, %xmm12 */
    "0f50c3",  /* movmskps %xmm3, %eax */
    "66450f50d5",  /* movmskpd %xmm13, %r10d */
    "660fd7c9",  /* pmovmskb %xmm1, %ecx */
    "660fc5c203",  /* pextrw $3, %xmm2, %eax */
    "66440fc4c805",  /* pinsrw $5, %eax, %xmm9 */
    "660fc40801",  /* pinsrw $1, (%rax), %xmm1 */
    "660f3a16d801",  /* pextrd $1, %xmm3, %eax */
    "66490f3a16d801",  /* pextrq $1, %xmm3, %r8 */
    "660f3a140807",  /* pextrb $7, %xmm1, (%rax) */
    "660f3a22e102",  /* pinsrd $2, %ecx, %xmm4 */
    "664d0f3a22f901",  /* pinsrq $1, %r9, %xmm15 */
    "660f3a201703",  /* pinsrb $3, (%rdi), %xmm2 */
    "660f3a17d001",  /* extractps $1, %xmm2, %eax */
    "f3440f2af0",  /* cvtsi2ss %eax, %xmm14 */
    "f34d0f2ad2",  /* cvtsi2ss %r10, %xmm10 */
    "f3440f2a00",  /* cvtsi2ssl (%rax), %xmm8 */
    "f3480f2a7308",  /* cvtsi2ssq 8(%rbx), %xmm6 */
    "f2440f2ac0",  /* cvtsi2sd %eax, %xmm8 */
    "f24d0f2ae2",  /* cvtsi2sd %r10, %xmm12 */
    "f20f2a38",  /* cvtsi2sdl (%rax), %xmm7 */
    "f24c0f2a4b08",  /* cvtsi2sdq 8(%rbx), %xmm9 */
    "f3410f2cc0",  /* cvttss2si %xmm8, %eax */
    "f34c0f2cdc",  /* cvttss2si %xmm4, %r11 */
    "f30f2c08",  /* cvttss2si (%rax), %ecx */
    "f2410f2cc0",  /* cvttsd2si %xmm8, %eax */
    "f24d0f2cdb",  /* cvttsd2si %xmm11, %r11 */
    "f20f2c08",  /* cvttsd2si (%rax), %ecx */
    "f3410f2dc0",  /* cvtss2si %xmm8, %eax */
    "f34c0f2ddf",  /* cvtss2si %xmm7, %r11 */
    "f30f2d08",  /* cvtss2si (%rax), %ecx */
    "f20f2dc6",  /* cvtsd2si %xmm6, %eax */
    "f24c0f2dd8",  /* cvtsd2si %xmm0, %r11 */
    "f20f2d08",  /* cvtsd2si (%rax), %ecx */
    "0f2b08",  /* movntps %xmm1, (%rax) */
    "66440fe74f10",  /* movntdq %xmm9, 16(%rdi) */
    "f20ff008",  /* lddqu (%rax), %xmm1 */
    "660f382a16",  /* movntdqa (%rsi), %xmm2 */
    "660f3814da",  /* blendvps %xmm0, %xmm2, %xmm3 */
    "660f381008",  /* pblendvb %xmm0, (%rax), %xmm1 */
    "660f3adfda01",  /* aeskeygenassist $1, %xmm2, %xmm3 */
    "f0830001",  /* lock addl $1, (%rax) */
    "f048ff4708",  /* lock incq 8(%rdi) */
    "f0800a01",  /* lock orb $1, (%rdx) */
    "f0480fba2805",  /* lock btsq $5, (%rax) */
    "4893",  /* xchg %rax, %rbx */
    "87c0",  /* xchg %eax, %eax */
    "664191",  /* xchg %ax, %r9w */
    "870c24",  /* xchgl %ecx, (%rsp) */
    "64488b042528000000",  /* mov %fs:0x28, %rax */
    "65488b08",  /* movq %gs:(%rax), %rcx */
    "64c704250800000005000000",  /* movl $5, %fs:8 */
    "67034810",  /* add 0x10(%eax), %ecx */
    "678d047e",  /* lea (%esi,%edi,2), %eax */
    "00ca",  /* add %cl, %dl */
    "6500142528000000",  /* addb %dl, %gs:0x28 */
    "6744026d00",  /* add 0(%ebp), %r13b */
    "4180c380",  /* add $-128, %r11b */
    "804500ff",  /* addb $255, (%rbp) */
    "0400",  /* add $0, %al */
    "664501de",  /* add %r11w, %r14w */
    "6644011425ffffff7f",  /* addw %r10w, 0x7fffffff */
    "6603445d00",  /* add (%rbp,%rbx,2), %ax */
    "664183c201",  /* add $1, %r10w */
    "664383044b01",  /* addw $1, (%r11,%r9,2) */
    "4501eb",  /* add %r13d, %r11d */
    "6544011c2500100000",  /* addl %r11d, %gs:0x1000 */
    "44037d00",  /* add (%rbp), %r15d */
    "4181c378563412",  /* add $305419896, %r11d */
    "41834500ff",  /* addl $-1, (%r13) */
    "0578563412",  /* add $305419896, %eax */
    "4d01f7",  /* add %r14, %r15 */
    "6548013c2528000000",  /* addq %rdi, %gs:0x28 */
    "4e030ced00100000",  /* add 0x1000(,%r13,8), %r9 */
    "4883c6ff",  /* add $-1, %rsi */
    "4981042480000000",  /* addq $128, (%r12) */
    "4883c000",  /* add $0, %rax */
    "4408d0",  /* or %r10b, %al */
    "40087101",  /* orb %sil, 1(%rcx) */
    "460a045d00000000",  /* or 0x0(,%r11,2), %r8b */
    "4080ce01",  /* or $1, %sil */
    "800c253412000000",  /* orb $0, 0x1234 */
    "0c7f",  /* or $127, %al */
    "6609ef",  /* or %bp, %di */
    "66094301",  /* orw %ax, 1(%rbx) */
    "66440b00",  /* or (%rax), %r8w */
    "6683cd01",  /* or $1, %bp */
    "66830c38ff",  /* orw $-1, (%rax,%rdi,1) */
    "6683c801",  /* or $1, %ax */
    "09e1",  /* or %esp, %ecx */
    "6509042528000000",  /* orl %eax, %gs:0x28 */
    "410b09",  /* or (%r9), %ecx */
    "4183cc7f",  /* or $127, %r12d */
    "41810c1f78563412",  /* orl $305419896, (%r15,%rbx,1) */
    "83c8ff",  /* or $-1, %eax */
    "4c09d3",  /* or %r10, %rbx */
    "49094500",  /* orq %rax, (%r13) */
    "480b242534120000",  /* or 0x1234, %rsp */
    "4981ca00000080",  /* or $-2147483648, %r10 */
    "4a830c5d000000007f",  /* orq $127, 0x0(,%r11,2) */
    "4883c8ff",  /* or $-1, %rax */
    "4110c8",  /* adc %cl, %r8b */
    "104c1500",  /* adcb %cl, (%rbp,%rdx,1) */
    "41124d00",  /* adc (%r13), %cl */
    "4080d701",  /* adc $1, %dil */
    "41809088a9cbed80",  /* adcb $-128, -305419896(%r8) */
    "1401",  /* adc $1, %al */
    "664511d8",  /* adc %r11w, %r8w */
    "6644112cd500000000",  /* adcw %r13w, 0x0(,%rdx,8) */
    "66131425ffffff7f",  /* adc 0x7fffffff, %dx */
    "6683d301",  /* adc $1, %bx */
    "6683550000",  /* adcw $0, (%rbp) */
    "6683d001",  /* adc $1, %ax */
    "11de",  /* adc %ebx, %esi */
    "110424",  /* adcl %eax, (%rsp) */
    "44132c2510000000",  /* adc 0x10, %r13d */
    "1580000000",  /* adc $128, %eax */
    "6781551078563412",  /* adcl $305419896, 16(%ebp) */
    "83d000",  /* adc $0, %eax */
    "4d11fb",  /* adc %r15, %r11 */
    "48110424",  /* adcq %rax, (%rsp) */
    "6548131c2500000000",  /* adc %gs:0x0, %rbx */
    "4883d67f",  /* adc $127, %rsi */
    "49811600000080",  /* adcq $-2147483648, (%r14) */
    "4518fe",  /* sbb %r15b, %r14b */
    "6740187010",  /* sbbb %sil, 16(%eax) */
    "64441a3c2500000000",  /* sbb %fs:0x0, %r15b */
    "80db01",  /* sbb $1, %bl */
    "65801c2500000000ff",  /* sbbb $255, %gs:0x0 */
    "664419e3",  /* sbb %r12w, %bx */
    "6644198388a9cbed",  /* sbbw %r8w, -305419896(%rbx) */
    "6466441b3c2500000000",  /* sbb %fs:0x0, %r15w */
    "664183df00",  /* sbb $0, %r15w */
    "6566831c25280000007f",  /* sbbw $127, %gs:0x28 */
    "661d8000",  /* sbb $128, %ax */
    "4519fd",  /* sbb %r15d, %r13d */
    "44193cdd00000000",  /* sbbl %r15d, 0x0(,%rbx,8) */
    "671b5a10",  /* sbb 16(%edx), %ebx */
    "4183db00",  /* sbb $0, %r11d */
    "831c2401",  /* sbbl $1, (%rsp) */
    "4819d1",  /* sbb %rdx, %rcx */
    "4819249d00100000",  /* sbbq %rsp, 0x1000(,%rbx,4) */
    "481b6500",  /* sbb (%rbp), %rsp */
    "4981dd78563412",  /* sbb $305419896, %r13 */
    "6548831c250000000000",  /* sbbq $0, %gs:0x0 */
    "4883d801",  /* sbb $1, %rax */
    "4420f7",  /* and %r14b, %dil */
    "654420242500100000",  /* andb %r12b, %gs:0x1000 */
    "41222c24",  /* and (%r12), %bpl */
    "80e180",  /* and $-128, %cl */
    "8024ed0010000080",  /* andb $-128, 0x1000(,%rbp,8) */
    "2480",  /* and $-128, %al */
    "664521da",  /* and %r11w, %r10w */
    "6621943ea0860100",  /* andw %dx, 100000(%rsi,%rdi,1) */
    "6566232c2500100000",  /* and %gs:0x1000, %bp */
    "6683e000",  /* and $0, %ax */
    "664381643cf88000",  /* andw $128, -8(%r12,%r15,1) */
    "66258000",  /* and $128, %ax */
    "4121f0",  /* and %esi, %r8d */
    "41217500",  /* andl %esi, (%r13) */
    "232c25ffffff7f",  /* and 0x7fffffff, %ebp */
    "81e680000000",  /* and $128, %esi */
    "658324250010000000",  /* andl $0, %gs:0x1000 */
    "83e0ff",  /* and $-1, %eax */
    "4921d7",  /* and %rdx, %r15 */
    "4d215500",  /* andq %r10, (%r13) */
    "4c231a",  /* and (%rdx), %r11 */
    "4983e401",  /* and $1, %r12 */
    "488324d17f",  /* andq $127, (%rcx,%rdx,8) */
    "482580000000",  /* and $128, %rax */
    "4528f1",  /* sub %r14b, %r9b */
    "412821",  /* subb %spl, (%r9) */
    "442a6500",  /* sub (%rbp), %r12b */
    "4180eeff",  /* sub $255, %r14b */
    "42802cdd0000000001",  /* subb $1, 0x0(,%r11,8) */
    "2cff",  /* sub $255, %al */
    "664429ca",  /* sub %r9w, %dx */
    "65664429042500100000",  /* subw %r8w, %gs:0x1000 */
    "662b3425ffffff7f",  /* sub 0x7fffffff, %si */
    "6681ec8000",  /* sub $128, %sp */
    "6641816980ff7f",  /* subw $32767, -128(%r9) */
    "662d8000",  /* sub $128, %ax */
    "4129f8",  /* sub %edi, %r8d */
    "298b80000000",  /* subl %ecx, 128(%rbx) */
    "65442b142500000000",  /* sub %gs:0x0, %r10d */
    "4181eb80000000",  /* sub $128, %r11d */
    "836d0001",  /* subl $1, (%rbp) */
    "83e801",  /* sub $1, %eax */
    "4c29f6",  /* sub %r14, %rsi */
    "644c29042528000000",  /* subq %r8, %fs:0x28 */
    "492ba45300040000",  /* sub 1024(%r11,%rdx,2), %rsp */
    "4983e801",  /* sub $1, %r8 */
    "49812c2400000080",  /* subq $-2147483648, (%r12) */
    "4883e8ff",  /* sub $-1, %rax */
    "4530d0",  /* xor %r10b, %r8b */
    "44301425ffffff7f",  /* xorb %r10b, 0x7fffffff */
    "4532bb80000000",  /* xor 128(%r11), %r15b */
    "4180f001",  /* xor $1, %r8b */
    "418075007f",  /* xorb $127, (%r13) */
    "3401",  /* xor $1, %al */
    "664531d4",  /* xor %r10w, %r12w */
    "663123",  /* xorw %sp, (%rbx) */
    "66413304cb",  /* xor (%r11,%rcx,8), %ax */
    "664181f2ff7f",  /* xor $32767, %r10w */
    "67664183771001",  /* xorw $1, 16(%r15d) */
    "6683f0ff",  /* xor $-1, %ax */
    "4131e3",  /* xor %esp, %r11d */
    "4531544ff8",  /* xorl %r10d, -8(%r15,%rcx,2) */
    "654433042528000000",  /* xor %gs:0x28, %r8d */
    "4183f200",  /* xor $0, %r10d */
    "428374a70401",  /* xorl $1, 4(%rdi,%r12,4) */
    "83f07f",  /* xor $127, %eax */
    "4831cc",  /* xor %rcx, %rsp */
    "49316d7f",  /* xorq %rbp, 127(%r13) */
    "4f33140a",  /* xor (%r10,%r9,1), %r10 */
    "4983f4ff",  /* xor $-1, %r12 */
    "4983342401",  /* xorq $1, (%r12) */
    "483580000000",  /* xor $128, %rax */
    "4538fa",  /* cmp %r15b, %r10b */
    "67385510",  /* cmpb %dl, 16(%ebp) */
    "3a0c2534120000",  /* cmp 0x1234, %cl */
    "4180fe00",  /* cmp $0, %r14b */
    "42803c7bff",  /* cmpb $255, (%rbx,%r15,2) */
    "3c00",  /* cmp $0, %al */
    "664539e1",  /* cmp %r12w, %r9w */
    "66453930",  /* cmpw %r14w, (%r8) */
    "6466443b2c2500000000",  /* cmp %fs:0x0, %r13w */
    "6683faff",  /* cmp $-1, %dx */
    "6641837d007f",  /* cmpw $127, (%r13) */
    "6683f801",  /* cmp $1, %ax */
    "4139d7",  /* cmp %edx, %r15d */
    "41392424",  /* cmpl %esp, (%r12) */
    "3b0424",  /* cmp (%rsp), %eax */
    "4181fa78563412",  /* cmp $305419896, %r10d */
    "833c24ff",  /* cmpl $-1, (%rsp) */
    "83f800",  /* cmp $0, %eax */
    "4839e3",  /* cmp %rsp, %rbx */
    "49397e01",  /* cmpq %rdi, 1(%r14) */
    "674d3b6210",  /* cmp 16(%r10d), %r12 */
    "4883fdff",  /* cmp $-1, %rbp */
    "48817c248078563412",  /* cmpq $305419896, -128(%rsp) */
    "4883f801",  /* cmp $1, %rax */
    "4484e6",  /* test %r12b, %sil */
    "6544840c2500000000",  /* test %r9b, %gs:0x0 */
    "41f6c280",  /* test $-128, %r10b */
    "f6437f7f",  /* testb $127, 127(%rbx) */
    "4588cf",  /* mov %r9b, %r15b */
    "47884425f8",  /* mov %r8b, -8(%r13,%r12,1) */
    "408a36",  /* mov (%rsi), %sil */
    "b180",  /* mov $-128, %cl */
    "41c60001",  /* movb $1, (%r8) */
    "41f6d7",  /* not %r15b */
    "40f6df",  /* neg %dil */
    "41f61c42",  /* negb (%r10,%rax,2) */
    "42f6642d00",  /* mulb (%rbp,%r13,1) */
    "41f6ee",  /* imul %r14b */
    "f62c9510000000",  /* imulb 0x10(,%rdx,4) */
    "f6f2",  /* div %dl */
    "f63425ffffff7f",  /* divb 0x7fffffff */
    "f6f9",  /* idiv %cl */
    "41f67d00",  /* idivb (%r13) */
    "41fec0",  /* inc %r8b */
    "fe06",  /* incb (%rsi) */
    "41feca",  /* dec %r10b */
    "42fe0c5500100000",  /* decb 0x1000(,%r10,2) */
    "41d0c5",  /* rol %r13b */
    "40d0c5",  /* rol $1, %bpl */
    "41d2c6",  /* rol %cl, %r14b */
    "65c004250010000002",  /* rolb $2, %gs:0x1000 */
    "40d0ce",  /* ror %sil */
    "41c0cf07",  /* ror $7, %r15b */
    "41d2ce",  /* ror %cl, %r14b */
    "d04d00",  /* rorb $1, (%rbp) */
    "41d0d3",  /* rcl %r11b */
    "41c0d707",  /* rcl $7, %r15b */
    "40d2d7",  /* rcl %cl, %dil */
    "67c0561005",  /* rclb $5, 16(%esi) */
    "41d0d9",  /* rcr %r9b */
    "d2d8",  /* rcr %cl, %al */
    "65d01c2500100000",  /* rcrb $1, %gs:0x1000 */
    "41d0e2",  /* shl %r10b */
    "c0e203",  /* shl $3, %dl */
    "41d2e5",  /* shl %cl, %r13b */
    "41c02102",  /* shlb $2, (%r9) */
    "40d0ec",  /* shr $1, %spl */
    "40d2ed",  /* shr %cl, %bpl */
    "c02c25ffffff7f05",  /* shrb $5, 0x7fffffff */
    "41d0fe",  /* sar %r14b */
    "c0fa03",  /* sar $3, %dl */
    "41d2ff",  /* sar %cl, %r15b */
    "42c0bc540004000002",  /* sarb $2, 1024(%rsp,%r10,2) */
    "40d0e6",  /* sal %sil */
    "41c0e207",  /* sal $7, %r10b */
    "41d2e4",  /* sal %cl, %r12b */
    "c024251000000005",  /* salb $5, 0x10 */
    "664185c8",  /* test %cx, %r8w */
    "6643853cf4",  /* test %di, (%r12,%r14,8) */
    "6641f7c60000",  /* test $0, %r14w */
    "66f70425341200008000",  /* testw $128, 0x1234 */
    "6689f7",  /* mov %si, %di */
    "66893a",  /* mov %di, (%rdx) */
    "66448b2c24",  /* mov (%rsp), %r13w */
    "66bf8000",  /* mov $128, %di */
    "66c704b500100000ff7f",  /* movw $32767, 0x1000(,%rsi,4) */
    "6641f7d7",  /* not %r15w */
    "6766f75710",  /* notw 16(%edi) */
    "6641f7d8",  /* neg %r8w */
    "66f718",  /* negw (%rax) */
    "66f7e1",  /* mul %cx */
    "66f7249c",  /* mulw (%rsp,%rbx,4) */
    "66f7ef",  /* imul %di */
    "66f72c24",  /* imulw (%rsp) */
    "6641f7f1",  /* div %r9w */
    "66f77208",  /* divw 8(%rdx) */
    "6641f7fd",  /* idiv %r13w */
    "66f73ca9",  /* idivw (%rcx,%rbp,4) */
    "6641ff03",  /* incw (%r11) */
    "6641ffca",  /* dec %r10w */
    "6641ff8b88a9cbed",  /* decw -305419896(%r11) */
    "6641d1c3",  /* rol %r11w */
    "6641c1c003",  /* rol $3, %r8w */
    "6641d3c7",  /* rol %cl, %r15w */
    "66c1042402",  /* rolw $2, (%rsp) */
    "66c1c807",  /* ror $7, %ax */
    "6641c10c2405",  /* rorw $5, (%r12) */
    "6641d1d5",  /* rcl %r13w */
    "6641c1d503",  /* rcl $3, %r13w */
    "66d3d7",  /* rcl %cl, %di */
    "66d11481",  /* rclw $1, (%rcx,%rax,4) */
    "66d1dd",  /* rcr %bp */
    "66c1d803",  /* rcr $3, %ax */
    "66d3d9",  /* rcr %cl, %cx */
    "676641d11b",  /* rcrw $1, 0(%r11d) */
    "66d1e3",  /* shl %bx */
    "66c1e207",  /* shl $7, %dx */
    "66d12424",  /* shlw $1, (%rsp) */
    "6641d1ec",  /* shr %r12w */
    "6641d1e8",  /* shr $1, %r8w */
    "66d3ef",  /* shr %cl, %di */
    "6641d12c24",  /* shrw $1, (%r12) */
    "6641d1fd",  /* sar $1, %r13w */
    "66d3fc",  /* sar %cl, %sp */
    "6466c13c250010000002",  /* sarw $2, %fs:0x1000 */
    "6641d1e6",  /* sal %r14w */
    "6641c1e603",  /* sal $3, %r14w */
    "66d3e5",  /* sal %cl, %bp */
    "6766c164241002",  /* salw $2, 16(%esp) */
    "4185c0",  /* test %eax, %r8d */
    "43856c9500",  /* test %ebp, (%r13,%r10,4) */
    "41f7c780000000",  /* test $128, %r15d */
    "41f7847d0004000080000000",  /* testl $128, 1024(%r13,%rdi,2) */
    "4489d9",  /* mov %r11d, %ecx */
    "6744896010",  /* mov %r12d, 16(%eax) */
    "448b0424",  /* mov (%rsp), %r8d */
    "41be01000000",  /* mov $1, %r14d */
    "41c74500ffffffff",  /* movl $-1, (%r13) */
    "42f7948500040000",  /* notl 1024(%rbp,%r8,4) */
    "41f7dd",  /* neg %r13d */
    "f79f80000000",  /* negl 128(%rdi) */
    "f7e3",  /* mul %ebx */
    "f72425ffffff7f",  /* mull 0x7fffffff */
    "41f7ee",  /* imul %r14d */
    "41f7ac2488a9cbed",  /* imull -305419896(%r12) */
    "41f7f3",  /* div %r11d */
    "f73424",  /* divl (%rsp) */
    "f7fd",  /* idiv %ebp */
    "f73c25ffffff7f",  /* idivl 0x7fffffff */
    "ffc5",  /* inc %ebp */
    "ff042534120000",  /* incl 0x1234 */
    "41ffcc",  /* dec %r12d */
    "42ff0c1d10000000",  /* decl 0x10(,%r11,1) */
    "41d1c4",  /* rol %r12d */
    "c1c107",  /* rol $7, %ecx */
    "d3c6",  /* rol %cl, %esi */
    "c1437f02",  /* roll $2, 127(%rbx) */
    "41d1ca",  /* ror %r10d */
    "41c1ca07",  /* ror $7, %r10d */
    "41d3cc",  /* ror %cl, %r12d */
    "65d10c2528000000",  /* rorl $1, %gs:0x28 */
    "d1d7",  /* rcl %edi */
    "c1d303",  /* rcl $3, %ebx */
    "d3d0",  /* rcl %cl, %eax */
    "41c11102",  /* rcll $2, (%r9) */
    "d1dc",  /* rcr %esp */
    "c1d907",  /* rcr $7, %ecx */
    "d3d8",  /* rcr %cl, %eax */
    "c11c25ffffff7f02",  /* rcrl $2, 0x7fffffff */
    "c1e603",  /* shl $3, %esi */
    "d3e7",  /* shl %cl, %edi */
    "6741c1611005",  /* shll $5, 16(%r9d) */
    "41d1ec",  /* shr %r12d */
    "41c1ef07",  /* shr $7, %r15d */
    "41d3ee",  /* shr %cl, %r14d */
    "c16e7f02",  /* shrl $2, 127(%rsi) */
    "d1ff",  /* sar %edi */
    "41c1fb03",  /* sar $3, %r11d */
    "d3fd",  /* sar %cl, %ebp */
    "41d13c24",  /* sarl $1, (%r12) */
    "d1e7",  /* sal %edi */
    "41c1e003",  /* sal $3, %r8d */
    "d3e6",  /* sal %cl, %esi */
    "c124f805",  /* sall $5, (%rax,%rdi,8) */
    "4c85dd",  /* test %r11, %rbp */
    "6748857010",  /* test %rsi, 16(%eax) */
    "48f7c400000080",  /* test $-2147483648, %rsp */
    "4af744d00401000000",  /* testq $1, 4(%rax,%r10,8) */
    "4889f5",  /* mov %rsi, %rbp */
    "4c89042534120000",  /* mov %r8, 0x1234 */
    "4d8b9988a9cbed",  /* mov -305419896(%r9), %r11 */
    "49c7c480000000",  /* mov $128, %r12 */
    "49c7460800000000",  /* movq $0, 8(%r14) */
    "49f7d6",  /* not %r14 */
    "49f7907fffffff",  /* notq -129(%r8) */
    "48f7da",  /* neg %rdx */
    "49f71c24",  /* negq (%r12) */
    "48f7e6",  /* mul %rsi */
    "49f76508",  /* mulq 8(%r13) */
    "48f7ed",  /* imul %rbp */
    "6748f729",  /* imulq 0(%ecx) */
    "49f7f2",  /* div %r10 */
    "49f77500",  /* divq (%r13) */
    "49f7fb",  /* idiv %r11 */
    "4af73c7500000000",  /* idivq 0x0(,%r14,2) */
    "48ffc7",  /* inc %rdi */
    "49ff02",  /* incq (%r10) */
    "49ffc8",  /* dec %r8 */
    "49ff4bff",  /* decq -1(%r11) */
    "48d1c4",  /* rol %rsp */
    "48c1c403",  /* rol $3, %rsp */
    "49d3c5",  /* rol %cl, %r13 */
    "4bc144090405",  /* rolq $5, 4(%r9,%r9,1) */
    "49d1ca",  /* ror %r10 */
    "48c1cc07",  /* ror $7, %rsp */
    "48d3ca",  /* ror %cl, %rdx */
    "48c10c2405",  /* rorq $5, (%rsp) */
    "49d1d7",  /* rcl %r15 */
    "49c1d303",  /* rcl $3, %r11 */
    "49d3d7",  /* rcl %cl, %r15 */
    "48c1144d0010000005",  /* rclq $5, 0x1000(,%rcx,2) */
    "48d1db",  /* rcr %rbx */
    "48d1da",  /* rcr $1, %rdx */
    "48d3dc",  /* rcr %cl, %rsp */
    "4bc11c6902",  /* rcrq $2, (%r9,%r13,2) */
    "48d1e2",  /* shl %rdx */
    "48d1e7",  /* shl $1, %rdi */
    "49d3e2",  /* shl %cl, %r10 */
    "49d164dd00",  /* shlq $1, (%r13,%rbx,8) */
    "48d1e9",  /* shr %rcx */
    "49c1ed03",  /* shr $3, %r13 */
    "49d3ef",  /* shr %cl, %r15 */
    "48c12c2405",  /* shrq $5, (%rsp) */
    "48d1fb",  /* sar %rbx */
    "48c1f907",  /* sar $7, %rcx */
    "48d3fc",  /* sar %cl, %rsp */
    "48c13c25ffffff7f05",  /* sarq $5, 0x7fffffff */
    "48d1e7",  /* sal %rdi */
    "49c1e007",  /* sal $7, %r8 */
    "48d3e3",  /* sal %cl, %rbx */
    "49d12424",  /* salq $1, (%r12) */
    "66440fafd2",  /* imul %dx, %r10w */
    "66440faf24df",  /* imul (%rdi,%rbx,8), %r12w */
    "66446bd37f",  /* imul $127, %bx, %r10w */
    "666944f104e803",  /* imul $1000, 4(%rcx,%rsi,8), %ax */
    "6766418d7b10",  /* lea 16(%r11d), %di */
    "66410f4afd",  /* cmovp %r13w, %di */
    "660f442424",  /* cmove (%rsp), %sp */
    "66410fa3ec",  /* bt %bp, %r12w */
    "66410fa3a180000000",  /* bt %sp, 128(%r9) */
    "660fbae01f",  /* bt $31, %ax */
    "66410fba65ff03",  /* btw $3, -1(%r13) */
    "660fabce",  /* bts %cx, %si */
    "66440fab2e",  /* bts %r13w, (%rsi) */
    "66410fbaea1f",  /* bts $31, %r10w */
    "660fba6cad0003",  /* btsw $3, (%rbp,%rbp,4) */
    "66440fb3c7",  /* btr %r8w, %di */
    "66460fb30cc510000000",  /* btr %r9w, 0x10(,%r8,8) */
    "66410fbaf41f",  /* btr $31, %r12w */
    "66410fba74ed0003",  /* btrw $3, (%r13,%rbp,8) */
    "66450fbbea",  /* btc %r13w, %r10w */
    "6766410fbb5610",  /* btc %dx, 16(%r14d) */
    "66410fbaff01",  /* btc $1, %r15w */
    "66410fba3c2403",  /* btcw $3, (%r12) */
    "66410fbcdb",  /* bsf %r11w, %bx */
    "66440fbc647ef8",  /* bsf -8(%rsi,%rdi,2), %r12w */
    "660fbdd5",  /* bsr %bp, %dx */
    "66440fbd1c1500100000",  /* bsr 0x1000(,%rdx,1), %r11w */
    "66f30fbcfd",  /* tzcnt %bp, %di */
    "66f3450fbc2424",  /* tzcnt (%r12), %r12w */
    "66f3410fbdf9",  /* lzcnt %r9w, %di */
    "66f3430fbd346c",  /* lzcnt (%r12,%r13,2), %si */
    "66f3440fb8c7",  /* popcnt %di, %r8w */
    "66f30fb80424",  /* popcnt (%rsp), %ax */
    "66440fa4e103",  /* shld $3, %r12w, %cx */
    "660fad4d00",  /* shrd %cl, %cx, (%rbp) */
    "66440fa5c9",  /* shld %r9w, %cx */
    "6466440fc1342500000000",  /* xadd %r14w, %fs:0x0 */
    "66440fb11cad00100000",  /* cmpxchg %r11w, 0x1000(,%rbp,4) */
    "6466f00fb10c2500000000",  /* lock cmpxchg %cx, %fs:0x0 */
    "66f0450fc16500",  /* lock xadd %r12w, (%r13) */
    "664487ca",  /* xchg %r9w, %dx */
    "6641876d00",  /* xchg %bp, (%r13) */
    "66440f38f05c2401",  /* movbe 1(%rsp), %r11w */
    "67660f38f17310",  /* movbe %si, 16(%ebx) */
    "0fafd0",  /* imul %eax, %edx */
    "420faf446004",  /* imul 4(%rax,%r12,2), %eax */
    "416bd805",  /* imul $5, %r8d, %ebx */
    "446918e8030000",  /* imul $1000, (%rax), %r11d */
    "448d2c7d00100000",  /* lea 0x1000(,%rdi,2), %r13d */
    "0f40f7",  /* cmovo %edi, %esi */
    "470f44440d00",  /* cmove (%r13,%r9,1), %r8d */
    "0fa3df",  /* bt %ebx, %edi */
    "0fa33e",  /* bt %edi, (%rsi) */
    "0fbae501",  /* bt $1, %ebp */
    "670fba2603",  /* btl $3, 0(%esi) */
    "410fabf9",  /* bts %edi, %r9d */
    "440fab5608",  /* bts %r10d, 8(%rsi) */
    "0fbaeb01",  /* bts $1, %ebx */
    "640fba2c250000000003",  /* btsl $3, %fs:0x0 */
    "410fb3f4",  /* btr %esi, %r12d */
    "440fb30a",  /* btr %r9d, (%rdx) */
    "410fbaf31f",  /* btr $31, %r11d */
    "420fba349d0000000003",  /* btrl $3, 0x0(,%r11,4) */
    "410fbbd7",  /* btc %edx, %r15d */
    "67410fbb6510",  /* btc %esp, 16(%r13d) */
    "0fbafd01",  /* btc $1, %ebp */
    "670fba3a03",  /* btcl $3, 0(%edx) */
    "410fbcd6",  /* bsf %r14d, %edx */
    "67440fbc2424",  /* bsf 0(%esp), %r12d */
    "440fbdfe",  /* bsr %esi, %r15d */
    "64440fbd142500000000",  /* bsr %fs:0x0, %r10d */
    "f3450fbcca",  /* tzcnt %r10d, %r9d */
    "f3440fbc0425ffffff7f",  /* tzcnt 0x7fffffff, %r8d */
    "f3450fbdd7",  /* lzcnt %r15d, %r10d */
    "f30fbd19",  /* lzcnt (%rcx), %ebx */
    "f3440fb8ce",  /* popcnt %esi, %r9d */
    "f3410fb83e",  /* popcnt (%r14), %edi */
    "0fa4f703",  /* shld $3, %esi, %edi */
    "0fad10",  /* shrd %cl, %edx, (%rax) */
    "440fa5e3",  /* shld %r12d, %ebx */
    "67440fc128",  /* xadd %r13d, 0(%eax) */
    "0fb16e80",  /* cmpxchg %ebp, -128(%rsi) */
    "f00fb1148d00000000",  /* lock cmpxchg %edx, 0x0(,%rcx,4) */
    "67f00fc130",  /* lock xadd %esi, 0(%eax) */
    "4187de",  /* xchg %ebx, %r14d */
    "448730",  /* xchg %r14d, (%rax) */
    "470f38f0445a04",  /* movbe 4(%r10,%r11,2), %r8d */
    "0f38f11cce",  /* movbe %ebx, (%rsi,%rcx,8) */
    "4d0faff8",  /* imul %r8, %r15 */
    "490fafa088a9cbed",  /* imul -305419896(%r8), %rsp */
    "496bfc05",  /* imul $5, %r12, %rdi */
    "6749695e10e8030000",  /* imul $1000, 16(%r14d), %rbx */
    "498d4d00",  /* lea (%r13), %rcx */
    "4c0f4ee9",  /* cmovle %rcx, %r13 */
    "490f444d00",  /* cmove (%r13), %rcx */
    "4d0fa3e0",  /* bt %r12, %r8 */
    "490fa35d00",  /* bt %rbx, (%r13) */
    "480fbae71f",  /* bt $31, %rdi */
    "4a0fba243703",  /* btq $3, (%rdi,%r14,1) */
    "490fabc0",  /* bts %rax, %r8 */
    "480fab2425ffffff7f",  /* bts %rsp, 0x7fffffff */
    "480fbaec01",  /* bts $1, %rsp */
    "480fba2903",  /* btsq $3, (%rcx) */
    "480fb3f1",  /* btr %rsi, %rcx */
    "4c0fb30424",  /* btr %r8, (%rsp) */
    "480fbaf21f",  /* btr $31, %rdx */
    "480fba34253412000003",  /* btrq $3, 0x1234 */
    "4c0fbbcf",  /* btc %r9, %rdi */
    "490fbb4440f8",  /* btc %rax, -8(%r8,%rax,2) */
    "490fbaff1f",  /* btc $31, %r15 */
    "480fba7d0003",  /* btcq $3, (%rbp) */
    "4c0fbcee",  /* bsf %rsi, %r13 */
    "490fbcae34120000",  /* bsf 4660(%r14), %rbp */
    "4c0fbdcd",  /* bsr %rbp, %r9 */
    "67480fbd4500",  /* bsr 0(%ebp), %rax */
    "f3490fbcca",  /* tzcnt %r10, %rcx */
    "67f3490fbc6210",  /* tzcnt 16(%r10d), %rsp */
    "f3480fbdfa",  /* lzcnt %rdx, %rdi */
    "f34b0fbd0ca2",  /* lzcnt (%r10,%r12,4), %rcx */
    "f3480fb8ce",  /* popcnt %rsi, %rcx */
    "f34d0fb864c304",  /* popcnt 4(%r11,%rax,8), %r12 */
    "4d0fadaa34120000",  /* shrd %cl, %r13, 4660(%r10) */
    "480fa5fb",  /* shld %rdi, %rbx */
    "4e0fc1048500100000",  /* xadd %r8, 0x1000(,%r8,4) */
    "4f0fb1740a04",  /* cmpxchg %r14, 4(%r10,%r9,1) */
    "f0480fb11c25ffffff7f",  /* lock cmpxchg %rbx, 0x7fffffff */
    "67f04d0fc130",  /* lock xadd %r14, 0(%r8d) */
    "90",  /* xchg %rax, %rax */
    "6748872a",  /* xchg %rbp, 0(%edx) */
    "4d0f38f01424",  /* movbe (%r12), %r10 */
    "490f38f14a7f",  /* movbe %rcx, 127(%r10) */
    "0fcd",  /* bswap %ebp */
    "f2450f38f104e8",  /* crc32l (%r8,%rbp,8), %r8d */
    "410fc7f1",  /* rdrand %r9d */
    "410fc7fa",  /* rdseed %r10d */
    "66450f38f6ee",  /* adcx %r14d, %r13d */
    "f30f38f6242510000000",  /* adox 0x10, %esp */
    "c46228f2e3",  /* andn %ebx, %r10d, %r12d */
    "c46208f24500",  /* andn (%rbp), %r14d, %r8d */
    "c46200f7f8",  /* bextr %r15d, %eax, %r15d */
    "c4e220f7042510000000",  /* bextr %r11d, 0x10, %eax */
    "c4c250f5c0",  /* bzhi %ebp, %r8d, %eax */
    "c44230f54d00",  /* bzhi %r9d, (%r13), %r9d */
    "c4621af7ef",  /* sarx %r12d, %edi, %r13d */
    "67c4623af74010",  /* sarx %r8d, 16(%eax), %r8d */
    "c4c251f7c1",  /* shlx %ebp, %r9d, %eax */
    "c46249f716",  /* shlx %esi, (%rsi), %r10d */
    "c4623bf7d6",  /* shrx %r8d, %esi, %r10d */
    "67c4e21bf73b",  /* shrx %r12d, 0(%ebx), %edi */
    "c44263f5e5",  /* pdep %r13d, %ebx, %r12d */
    "c4425af5c1",  /* pext %r9d, %esp, %r8d */
    "c4e21bf6ce",  /* mulx %esi, %r12d, %ecx */
    "c4c210f3cd",  /* blsr %r13d, %r13d */
    "c4c268f3d2",  /* blsmsk %r10d, %edx */
    "c4e208f3de",  /* blsi %esi, %r14d */
    "c4637bf0f405",  /* rorx $5, %esp, %r14d */
    "450fc36500",  /* movnti %r12d, (%r13) */
    "480fcf",  /* bswap %rdi */
    "f24d0f38f17d00",  /* crc32q (%r13), %r15 */
    "490fc7f0",  /* rdrand %r8 */
    "480fc7fe",  /* rdseed %rsi */
    "664d0f38f6e7",  /* adcx %r15, %r12 */
    "f34d0f38f600",  /* adox (%r8), %r8 */
    "c4c2a8f2f1",  /* andn %r9, %r10, %rsi */
    "67c4e2d0f22b",  /* andn 0(%ebx), %rbp, %rbp */
    "c4c2a8f7f9",  /* bextr %r10, %r9, %rdi */
    "c462c8f7737f",  /* bextr %rsi, 127(%rbx), %r14 */
    "c4e2a8f5f4",  /* bzhi %r10, %rsp, %rsi */
    "c442c0f5b97fffffff",  /* bzhi %rdi, -129(%r9), %r15 */
    "c4428af7ca",  /* sarx %r14, %r10, %r9 */
    "c442daf76500",  /* sarx %rsp, (%r13), %r12 */
    "c462d1f7d7",  /* shlx %rbp, %rdi, %r10 */
    "c46291f72c9510000000",  /* shlx %r13, 0x10(,%rdx,4), %r13 */
    "c462f3f7dd",  /* shrx %rcx, %rbp, %r11 */
    "67c4c2cbf720",  /* shrx %rsi, 0(%r8d), %rsp */
    "c4c2c3f5d1",  /* pdep %r9, %rdi, %rdx */
    "c4e282f5c0",  /* pext %rax, %r15, %rax */
    "c4e2fbf6d6",  /* mulx %rsi, %rax, %rdx */
    "c4c2d0f3ca",  /* blsr %r10, %rbp */
    "c4c2d0f3d4",  /* blsmsk %r12, %rbp */
    "c4c290f3d9",  /* blsi %r9, %r13 */
    "490fc30424",  /* movnti %rax, (%r12) */
    "66440fb6e0",  /* movzbw %al, %r12w */
    "66440fb6741d00",  /* movzbw (%rbp,%rbx,1), %r14w */
    "66440fbed6",  /* movsbw %sil, %r10w */
    "66440fbe442a04",  /* movsbw 4(%rdx,%rbp,1), %r8w */
    "410fb6dd",  /* movzbl %r13b, %ebx */
    "0fb6b680000000",  /* movzbl 128(%rsi), %esi */
    "410fbefd",  /* movsbl %r13b, %edi */
    "0fbe5500",  /* movsbl (%rbp), %edx */
    "490fb6f3",  /* movzbq %r11b, %rsi */
    "4f0fb684a3a0860100",  /* movzbq 100000(%r11,%r12,4), %r8 */
    "4c0fbef5",  /* movsbq %bpl, %r14 */
    "490fbe1424",  /* movsbq (%r12), %rdx */
    "0fb7e0",  /* movzwl %ax, %esp */
    "64440fb72c2500000000",  /* movzwl %fs:0x0, %r13d */
    "450fbfcd",  /* movswl %r13w, %r9d */
    "470fbf2c19",  /* movswl (%r9,%r11,1), %r13d */
    "490fb7e6",  /* movzwq %r14w, %rsp */
    "4a0fb7240d00100000",  /* movzwq 0x1000(,%r9,1), %rsp */
    "4d0fbfdc",  /* movswq %r12w, %r11 */
    "4d0fbf6500",  /* movswq (%r13), %r12 */
    "4863d2",  /* movslq %edx, %rdx */
    "49636d00",  /* movslq (%r13), %rbp */
    "400f90c7",  /* seto %dil */
    "650f90042500100000",  /* seto %gs:0x1000 */
    "410f91c0",  /* setno %r8b */
    "67410f914510",  /* setno 16(%r13d) */
    "400f92c4",  /* setb %spl */
    "410f920424",  /* setb (%r12) */
    "400f93c6",  /* setae %sil */
    "0f9344c304",  /* setae 4(%rbx,%rax,8) */
    "0f94c2",  /* sete %dl */
    "0f94042534120000",  /* sete 0x1234 */
    "410f95c4",  /* setne %r12b */
    "0f9504c0",  /* setne (%rax,%rax,8) */
    "410f96c5",  /* setbe %r13b */
    "670f96442410",  /* setbe 16(%esp) */
    "0f97c3",  /* seta %bl */
    "0f974580",  /* seta -128(%rbp) */
    "400f98c6",  /* sets %sil */
    "410f984280",  /* sets -128(%r10) */
    "410f99c2",  /* setns %r10b */
    "0f9ac1",  /* setp %cl */
    "0f9a0424",  /* setp (%rsp) */
    "400f9bc7",  /* setnp %dil */
    "410f9b8486a0860100",  /* setnp 100000(%r14,%rax,4) */
    "410f9cc7",  /* setl %r15b */
    "410f9c0424",  /* setl (%r12) */
    "400f9dc7",  /* setge %dil */
    "410f9d837fffffff",  /* setge -129(%r11) */
    "0f9ec0",  /* setle %al */
    "0f9e042534120000",  /* setle 0x1234 */
    "67410f9f06",  /* setg 0(%r14d) */
    "0f58fa",  /* addps %xmm2, %xmm7 */
    "460f5814c0",  /* addps (%rax,%r8,8), %xmm10 */
    "66450f58fe",  /* addpd %xmm14, %xmm15 */
    "66450f584d00",  /* addpd (%r13), %xmm9 */
    "f3450f58cf",  /* addss %xmm15, %xmm9 */
    "f30f589b80000000",  /* addss 128(%rbx), %xmm3 */
    "f2410f58ca",  /* addsd %xmm10, %xmm1 */
    "f2440f581c5d00100000",  /* addsd 0x1000(,%rbx,2), %xmm11 */
    "410f59e3",  /* mulps %xmm11, %xmm4 */
    "410f5950ff",  /* mulps -1(%r8), %xmm2 */
    "660f59fc",  /* mulpd %xmm4, %xmm7 */
    "66410f5929",  /* mulpd (%r9), %xmm5 */
    "f3440f59e1",  /* mulss %xmm1, %xmm12 */
    "65f3440f59342500100000",  /* mulss %gs:0x1000, %xmm14 */
    "f20f59cd",  /* mulsd %xmm5, %xmm1 */
    "f2430f59aca400040000",  /* mulsd 1024(%r12,%r12,4), %xmm5 */
    "450f5cd6",  /* subps %xmm14, %xmm10 */
    "410f5c717f",  /* subps 127(%r9), %xmm6 */
    "660f5cd2",  /* subpd %xmm2, %xmm2 */
    "64660f5c242500100000",  /* subpd %fs:0x1000, %xmm4 */
    "f3450f5cdb",  /* subss %xmm11, %xmm11 */
    "f3460f5c1c4d00000000",  /* subss 0x0(,%r9,2), %xmm11 */
    "f2440f5cc5",  /* subsd %xmm5, %xmm8 */
    "f2440f5c2c1500100000",  /* subsd 0x1000(,%rdx,1), %xmm13 */
    "440f5de4",  /* minps %xmm4, %xmm12 */
    "440f5d1c25ffffff7f",  /* minps 0x7fffffff, %xmm11 */
    "66450f5dd5",  /* minpd %xmm13, %xmm10 */
    "66460f5d248500000000",  /* minpd 0x0(,%r8,4), %xmm12 */
    "f3410f5dd1",  /* minss %xmm9, %xmm2 */
    "f3410f5d5f08",  /* minss 8(%r15), %xmm3 */
    "f2450f5dc5",  /* minsd %xmm13, %xmm8 */
    "f2440f5d3424",  /* minsd (%rsp), %xmm14 */
    "0f5ee1",  /* divps %xmm1, %xmm4 */
    "0f5e84d0a0860100",  /* divps 100000(%rax,%rdx,8), %xmm0 */
    "660f5eff",  /* divpd %xmm7, %xmm7 */
    "66410f5e2424",  /* divpd (%r12), %xmm4 */
    "f3440f5ef6",  /* divss %xmm6, %xmm14 */
    "f3470f5e7c4104",  /* divss 4(%r9,%r8,2), %xmm15 */
    "f20f5ef9",  /* divsd %xmm1, %xmm7 */
    "f2460f5e141500000000",  /* divsd 0x0(,%r10,1), %xmm10 */
    "450f5fe0",  /* maxps %xmm8, %xmm12 */
    "450f5f5d00",  /* maxps (%r13), %xmm11 */
    "66440f5fc7",  /* maxpd %xmm7, %xmm8 */
    "66440f5f21",  /* maxpd (%rcx), %xmm12 */
    "f30f5ff8",  /* maxss %xmm0, %xmm7 */
    "f3410f5f4500",  /* maxss (%r13), %xmm0 */
    "f2450f5fd6",  /* maxsd %xmm14, %xmm10 */
    "f2450f5f14b6",  /* maxsd (%r14,%rsi,4), %xmm10 */
    "410f51e0",  /* sqrtps %xmm8, %xmm4 */
    "440f511424",  /* sqrtps (%rsp), %xmm10 */
    "66450f51cd",  /* sqrtpd %xmm13, %xmm9 */
    "66460f510c72",  /* sqrtpd (%rdx,%r14,2), %xmm9 */
    "f3450f51dc",  /* sqrtss %xmm12, %xmm11 */
    "f3450f510424",  /* sqrtss (%r12), %xmm8 */
    "f2450f51ce",  /* sqrtsd %xmm14, %xmm9 */
    "f2410f5158ff",  /* sqrtsd -1(%r8), %xmm3 */
    "410f28cd",  /* movaps %xmm13, %xmm1 */
    "410f287500",  /* movaps (%r13), %xmm6 */
    "470f2994f600040000",  /* movaps %xmm10, 1024(%r14,%r14,8) */
    "66450f28da",  /* movapd %xmm10, %xmm11 */
    "66440f283c25ffffff7f",  /* movapd 0x7fffffff, %xmm15 */
    "66410f297d00",  /* movapd %xmm7, (%r13) */
    "410f10f6",  /* movups %xmm14, %xmm6 */
    "450f10642408",  /* movups 8(%r12), %xmm12 */
    "470f1124bc",  /* movups %xmm12, (%r12,%r15,4) */
    "66440f10e0",  /* movupd %xmm0, %xmm12 */
    "660f101c2534120000",  /* movupd 0x1234, %xmm3 */
    "66410f113c24",  /* movupd %xmm7, (%r12) */
    "f3450f100b",  /* movss (%r11), %xmm9 */
    "f3440f112425ffffff7f",  /* movss %xmm12, 0x7fffffff */
    "f2440f10f9",  /* movsd %xmm1, %xmm15 */
    "67f2440f102e",  /* movsd 0(%esi), %xmm13 */
    "f2450f113c24",  /* movsd %xmm15, (%r12) */
    "66440f6fea",  /* movdqa %xmm2, %xmm13 */
    "66440f6f2c7510000000",  /* movdqa 0x10(,%rsi,2), %xmm13 */
    "660f7f967fffffff",  /* movdqa %xmm2, -129(%rsi) */
    "f3440f6fea",  /* movdqu %xmm2, %xmm13 */
    "67f3450f6f2b",  /* movdqu 0(%r11d), %xmm13 */
    "67f3440f7f3e",  /* movdqu %xmm15, 0(%esi) */
    "410f160f",  /* movhps (%r15), %xmm1 */
    "67410f176310",  /* movhps %xmm4, 16(%r11d) */
    "460f1224ae",  /* movlps (%rsi,%r13,4), %xmm12 */
    "450f136d00",  /* movlps %xmm13, (%r13) */
    "66440f161c25ffffff7f",  /* movhpd 0x7fffffff, %xmm11 */
    "660f176500",  /* movhpd %xmm4, (%rbp) */
    "67660f1223",  /* movlpd 0(%ebx), %xmm4 */
    "66410f136d00",  /* movlpd %xmm5, (%r13) */
    "440f12d3",  /* movhlps %xmm3, %xmm10 */
    "440f16f8",  /* movlhps %xmm0, %xmm15 */
    "450f14c1",  /* unpcklps %xmm9, %xmm8 */
    "450f143424",  /* unpcklps (%r12), %xmm14 */
    "0f15f5",  /* unpckhps %xmm5, %xmm6 */
    "0f150c25ffffff7f",  /* unpckhps 0x7fffffff, %xmm1 */
    "66410f14c2",  /* unpcklpd %xmm10, %xmm0 */
    "6566440f14342500100000",  /* unpcklpd %gs:0x1000, %xmm14 */
    "66450f15c3",  /* unpckhpd %xmm11, %xmm8 */
    "66450f159cd400040000",  /* unpckhpd 1024(%r12,%rdx,8), %xmm11 */
    "440f54f3",  /* andps %xmm3, %xmm14 */
    "440f543c2534120000",  /* andps 0x1234, %xmm15 */
    "660f54f0",  /* andpd %xmm0, %xmm6 */
    "66420f54143d00100000",  /* andpd 0x1000(,%r15,1), %xmm2 */
    "0f55d4",  /* andnps %xmm4, %xmm2 */
    "0f5528",  /* andnps (%rax), %xmm5 */
    "66440f55d8",  /* andnpd %xmm0, %xmm11 */
    "660f553c24",  /* andnpd (%rsp), %xmm7 */
    "450f56c5",  /* orps %xmm13, %xmm8 */
    "410f567c85f8",  /* orps -8(%r13,%rax,4), %xmm7 */
    "66450f56e9",  /* orpd %xmm9, %xmm13 */
    "660f562c2510000000",  /* orpd 0x10, %xmm5 */
    "450f57fc",  /* xorps %xmm12, %xmm15 */
    "0f573cfd00100000",  /* xorps 0x1000(,%rdi,8), %xmm7 */
    "660f57de",  /* xorpd %xmm6, %xmm3 */
    "660f570c24",  /* xorpd (%rsp), %xmm1 */
    "0f2ffb",  /* comiss %xmm3, %xmm7 */
    "440f2f5d00",  /* comiss (%rbp), %xmm11 */
    "66440f2fd9",  /* comisd %xmm1, %xmm11 */
    "66410f2f31",  /* comisd (%r9), %xmm6 */
    "410f2ee9",  /* ucomiss %xmm9, %xmm5 */
    "450f2e38",  /* ucomiss (%r8), %xmm15 */
    "66440f2eed",  /* ucomisd %xmm5, %xmm13 */
    "66440f2e2442",  /* ucomisd (%rdx,%rax,2), %xmm12 */
    "410f5ad6",  /* cvtps2pd %xmm14, %xmm2 */
    "64440f5a0c2500100000",  /* cvtps2pd %fs:0x1000, %xmm9 */
    "66450f5ac4",  /* cvtpd2ps %xmm12, %xmm8 */
    "66420f5a2cf3",  /* cvtpd2ps (%rbx,%r14,8), %xmm5 */
    "f3410f5ae3",  /* cvtss2sd %xmm11, %xmm4 */
    "f3410f5a2c24",  /* cvtss2sd (%r12), %xmm5 */
    "f2440f5ac3",  /* cvtsd2ss %xmm3, %xmm8 */
    "f20f5a6d00",  /* cvtsd2ss (%rbp), %xmm5 */
    "440f5be6",  /* cvtdq2ps %xmm6, %xmm12 */
    "440f5b3c24",  /* cvtdq2ps (%rsp), %xmm15 */
    "66410f5bc0",  /* cvtps2dq %xmm8, %xmm0 */
    "660f5b042534120000",  /* cvtps2dq 0x1234, %xmm0 */
    "f3450f5bc0",  /* cvttps2dq %xmm8, %xmm8 */
    "67f30f5b5610",  /* cvttps2dq 16(%esi), %xmm2 */
    "f3450fe6d5",  /* cvtdq2pd %xmm13, %xmm10 */
    "f3410fe6587f",  /* cvtdq2pd 127(%r8), %xmm3 */
    "f2450fe6ca",  /* cvtpd2dq %xmm10, %xmm9 */
    "f20fe664cc04",  /* cvtpd2dq 4(%rsp,%rcx,8), %xmm4 */
    "660fe6e5",  /* cvttpd2dq %xmm5, %xmm4 */
    "660fe6142534120000",  /* cvttpd2dq 0x1234, %xmm2 */
    "450f53d5",  /* rcpps %xmm13, %xmm10 */
    "440f536a08",  /* rcpps 8(%rdx), %xmm13 */
    "f3440f53d7",  /* rcpss %xmm7, %xmm10 */
    "f30f533c25ffffff7f",  /* rcpss 0x7fffffff, %xmm7 */
    "450f52fb",  /* rsqrtps %xmm11, %xmm15 */
    "670f524e10",  /* rsqrtps 16(%esi), %xmm1 */
    "f3450f52ca",  /* rsqrtss %xmm10, %xmm9 */
    "f3440f520c24",  /* rsqrtss (%rsp), %xmm9 */
    "f2440f12da",  /* movddup %xmm2, %xmm11 */
    "f20f12342510000000",  /* movddup 0x10, %xmm6 */
    "f30f16ff",  /* movshdup %xmm7, %xmm7 */
    "f3450f165d00",  /* movshdup (%r13), %xmm11 */
    "f30f12ce",  /* movsldup %xmm6, %xmm1 */
    "f3470f12148a",  /* movsldup (%r10,%r9,4), %xmm10 */
    "450fc2fb1b",  /* cmpps $27, %xmm11, %xmm15 */
    "440fc21cee03",  /* cmpps $3, (%rsi,%rbp,8), %xmm11 */
    "66410fc2f001",  /* cmppd $1, %xmm8, %xmm6 */
    "660fc23c2403",  /* cmppd $3, (%rsp), %xmm7 */
    "f3450fc2ca1b",  /* cmpss $27, %xmm10, %xmm9 */
    "65f3440fc22c250010000003",  /* cmpss $3, %gs:0x1000, %xmm13 */
    "f2440fc2c004",  /* cmpsd $4, %xmm0, %xmm8 */
    "f2430fc24c6c0403",  /* cmpsd $3, 4(%r12,%r13,2), %xmm1 */
    "440fc6dd1b",  /* shufps $27, %xmm5, %xmm11 */
    "450fc67d0003",  /* shufps $3, (%r13), %xmm15 */
    "660fc6fe00",  /* shufpd $0, %xmm6, %xmm7 */
    "64660fc63c250000000003",  /* shufpd $3, %fs:0x0, %xmm7 */
    "66410f70d901",  /* pshufd $1, %xmm9, %xmm3 */
    "66410f700c2403",  /* pshufd $3, (%r12), %xmm1 */
    "f3440f70f001",  /* pshufhw $1, %xmm0, %xmm14 */
    "67f3450f701b03",  /* pshufhw $3, 0(%r11d), %xmm11 */
    "f2410f70f000",  /* pshuflw $0, %xmm8, %xmm6 */
    "67f2440f70550003",  /* pshuflw $3, 0(%ebp), %xmm10 */
    "660f3a08e71b",  /* roundps $27, %xmm7, %xmm4 */
    "66450f3a0804d903",  /* roundps $3, (%r9,%rbx,8), %xmm8 */
    "66440f3a09ef04",  /* roundpd $4, %xmm7, %xmm13 */
    "64660f3a091c250000000003",  /* roundpd $3, %fs:0x0, %xmm3 */
    "66450f3a0ae301",  /* roundss $1, %xmm11, %xmm12 */
    "66440f3a0a34251000000003",  /* roundss $3, 0x10, %xmm14 */
    "66450f3a0bef01",  /* roundsd $1, %xmm15, %xmm13 */
    "66410f3a0bad3412000003",  /* roundsd $3, 4660(%r13), %xmm5 */
    "660f3a0cdf04",  /* blendps $4, %xmm7, %xmm3 */
    "66440f3a0c877fffffff03",  /* blendps $3, -129(%rdi), %xmm8 */
    "66450f3a0dde01",  /* blendpd $1, %xmm14, %xmm11 */
    "64660f3a0d1c250000000003",  /* blendpd $3, %fs:0x0, %xmm3 */
    "66440f3a0ed01b",  /* pblendw $27, %xmm0, %xmm10 */
    "660f3a0e24cd1000000003",  /* pblendw $3, 0x10(,%rcx,8), %xmm4 */
    "66410f3a0ff101",  /* palignr $1, %xmm9, %xmm6 */
    "660f3a0f750003",  /* palignr $3, (%rbp), %xmm6 */
    "66440f3a40c601",  /* dpps $1, %xmm6, %xmm8 */
    "67660f3a40671003",  /* dpps $3, 16(%edi), %xmm4 */
    "66440f3a44c700",  /* pclmulqdq $0, %xmm7, %xmm8 */
    "66450f3a441c2403",  /* pclmulqdq $3, (%r12), %xmm11 */
    "66440f3a21d704",  /* insertps $4, %xmm7, %xmm10 */
    "66440f3a21650003",  /* insertps $3, (%rbp), %xmm12 */
    "0fc2d600",  /* cmpeqps %xmm6, %xmm2 */
    "66450fc2ce00",  /* cmpeqpd %xmm14, %xmm9 */
    "f3410fc2ea00",  /* cmpeqss %xmm10, %xmm5 */
    "f2410fc2df00",  /* cmpeqsd %xmm15, %xmm3 */
    "0fc2d801",  /* cmpltps %xmm0, %xmm3 */
    "66450fc2d301",  /* cmpltpd %xmm11, %xmm10 */
    "f3450fc2d501",  /* cmpltss %xmm13, %xmm10 */
    "f2410fc2e801",  /* cmpltsd %xmm8, %xmm5 */
    "0fc2eb02",  /* cmpleps %xmm3, %xmm5 */
    "66410fc2fa02",  /* cmplepd %xmm10, %xmm7 */
    "f3450fc2c902",  /* cmpless %xmm9, %xmm9 */
    "f2450fc2d402",  /* cmplesd %xmm12, %xmm10 */
    "410fc2fe03",  /* cmpunordps %xmm14, %xmm7 */
    "66440fc2db03",  /* cmpunordpd %xmm3, %xmm11 */
    "f3410fc2da03",  /* cmpunordss %xmm10, %xmm3 */
    "f20fc2f203",  /* cmpunordsd %xmm2, %xmm6 */
    "450fc2c204",  /* cmpneqps %xmm10, %xmm8 */
    "66440fc2f204",  /* cmpneqpd %xmm2, %xmm14 */
    "f3450fc2c804",  /* cmpneqss %xmm8, %xmm9 */
    "f20fc2da04",  /* cmpneqsd %xmm2, %xmm3 */
    "0fc2c505",  /* cmpnltps %xmm5, %xmm0 */
    "66440fc2f305",  /* cmpnltpd %xmm3, %xmm14 */
    "f3440fc2f705",  /* cmpnltss %xmm7, %xmm14 */
    "f2440fc2db05",  /* cmpnltsd %xmm3, %xmm11 */
    "450fc2c706",  /* cmpnleps %xmm15, %xmm8 */
    "66440fc2c206",  /* cmpnlepd %xmm2, %xmm8 */
    "f3440fc2d406",  /* cmpnless %xmm4, %xmm10 */
    "f2450fc2e506",  /* cmpnlesd %xmm13, %xmm12 */
    "440fc2fe07",  /* cmpordps %xmm6, %xmm15 */
    "66450fc2e907",  /* cmpordpd %xmm9, %xmm13 */
    "f3440fc2eb07",  /* cmpordss %xmm3, %xmm13 */
    "f2410fc2d407",  /* cmpordsd %xmm12, %xmm2 */
    "66440f60f8",  /* punpcklbw %xmm0, %xmm15 */
    "660f602c8500000000",  /* punpcklbw 0x0(,%rax,4), %xmm5 */
    "66410f61e2",  /* punpcklwd %xmm10, %xmm4 */
    "66450f614500",  /* punpcklwd (%r13), %xmm8 */
    "66440f62d8",  /* punpckldq %xmm0, %xmm11 */
    "6566440f622c2528000000",  /* punpckldq %gs:0x28, %xmm13 */
    "660f63f8",  /* packsswb %xmm0, %xmm7 */
    "66410f638188a9cbed",  /* packsswb -305419896(%r9), %xmm0 */
    "66450f64f9",  /* pcmpgtb %xmm9, %xmm15 */
    "660f641c2534120000",  /* pcmpgtb 0x1234, %xmm3 */
    "66450f65db",  /* pcmpgtw %xmm11, %xmm11 */
    "67660f656310",  /* pcmpgtw 16(%ebx), %xmm4 */
    "66450f66dc",  /* pcmpgtd %xmm12, %xmm11 */
    "660f666d00",  /* pcmpgtd (%rbp), %xmm5 */
    "66410f67e9",  /* packuswb %xmm9, %xmm5 */
    "660f673411",  /* packuswb (%rcx,%rdx,1), %xmm6 */
    "66440f68c0",  /* punpckhbw %xmm0, %xmm8 */
    "660f681c24",  /* punpckhbw (%rsp), %xmm3 */
    "66450f69e8",  /* punpckhwd %xmm8, %xmm13 */
    "64660f691c2500000000",  /* punpckhwd %fs:0x0, %xmm3 */
    "66440f6af9",  /* punpckhdq %xmm1, %xmm15 */
    "66470f6a24bf",  /* punpckhdq (%r15,%r15,4), %xmm12 */
    "66410f6bdf",  /* packssdw %xmm15, %xmm3 */
    "66450f6b4cb404",  /* packssdw 4(%r12,%rsi,4), %xmm9 */
    "66410f6cc6",  /* punpcklqdq %xmm14, %xmm0 */
    "66410f6c2f",  /* punpcklqdq (%r15), %xmm5 */
    "66450f6df7",  /* punpckhqdq %xmm15, %xmm14 */
    "66410f6d3c24",  /* punpckhqdq (%r12), %xmm7 */
    "660f74d1",  /* pcmpeqb %xmm1, %xmm2 */
    "66440f747d00",  /* pcmpeqb (%rbp), %xmm15 */
    "66410f75f8",  /* pcmpeqw %xmm8, %xmm7 */
    "660f753c25ffffff7f",  /* pcmpeqw 0x7fffffff, %xmm7 */
    "660f76f3",  /* pcmpeqd %xmm3, %xmm6 */
    "66440f768988a9cbed",  /* pcmpeqd -305419896(%rcx), %xmm9 */
    "66440fd4c3",  /* paddq %xmm3, %xmm8 */
    "6766410fd410",  /* paddq 0(%r8d), %xmm2 */
    "66440fd5e7",  /* pmullw %xmm7, %xmm12 */
    "660fd50cd510000000",  /* pmullw 0x10(,%rdx,8), %xmm1 */
    "66410fd8d5",  /* psubusb %xmm13, %xmm2 */
    "66470fd84cb004",  /* psubusb 4(%r8,%r14,4), %xmm9 */
    "660fd9ec",  /* psubusw %xmm4, %xmm5 */
    "66440fd93449",  /* psubusw (%rcx,%rcx,2), %xmm14 */
    "66410fdae7",  /* pminub %xmm15, %xmm4 */
    "6766410fda7500",  /* pminub 0(%r13d), %xmm6 */
    "660fdbfc",  /* pand %xmm4, %xmm7 */
    "65660fdb142528000000",  /* pand %gs:0x28, %xmm2 */
    "66410fdcdc",  /* paddusb %xmm12, %xmm3 */
    "66450fdc2488",  /* paddusb (%r8,%rcx,4), %xmm12 */
    "66450fddf7",  /* paddusw %xmm15, %xmm14 */
    "66440fdd3c3510000000",  /* paddusw 0x10(,%rsi,1), %xmm15 */
    "66440fdedb",  /* pmaxub %xmm3, %xmm11 */
    "6766410fde542410",  /* pmaxub 16(%r12d), %xmm2 */
    "66410fdff4",  /* pandn %xmm12, %xmm6 */
    "66450fdf7880",  /* pandn -128(%r8), %xmm15 */
    "660fe0e7",  /* pavgb %xmm7, %xmm4 */
    "65660fe0042500100000",  /* pavgb %gs:0x1000, %xmm0 */
    "66410fe3ea",  /* pavgw %xmm10, %xmm5 */
    "660fe30472",  /* pavgw (%rdx,%rsi,2), %xmm0 */
    "66410fe4d5",  /* pmulhuw %xmm13, %xmm2 */
    "66440fe40c24",  /* pmulhuw (%rsp), %xmm9 */
    "66450fe5d3",  /* pmulhw %xmm11, %xmm10 */
    "660fe514fd10000000",  /* pmulhw 0x10(,%rdi,8), %xmm2 */
    "66440fe8d3",  /* psubsb %xmm3, %xmm10 */
    "66440fe87101",  /* psubsb 1(%rcx), %xmm14 */
    "660fe9e5",  /* psubsw %xmm5, %xmm4 */
    "64660fe91c2500000000",  /* psubsw %fs:0x0, %xmm3 */
    "660feae6",  /* pminsw %xmm6, %xmm4 */
    "66440fea6d00",  /* pminsw (%rbp), %xmm13 */
    "66410febde",  /* por %xmm14, %xmm3 */
    "66440feb3c25ffffff7f",  /* por 0x7fffffff, %xmm15 */
    "66440fecda",  /* paddsb %xmm2, %xmm11 */
    "660fec1424",  /* paddsb (%rsp), %xmm2 */
    "660fedf8",  /* paddsw %xmm0, %xmm7 */
    "66450fed4500",  /* paddsw (%r13), %xmm8 */
    "66450feec9",  /* pmaxsw %xmm9, %xmm9 */
    "660fee542408",  /* pmaxsw 8(%rsp), %xmm2 */
    "66410feff5",  /* pxor %xmm13, %xmm6 */
    "66450fef00",  /* pxor (%r8), %xmm8 */
    "660ff4e8",  /* pmuludq %xmm0, %xmm5 */
    "660ff4043b",  /* pmuludq (%rbx,%rdi,1), %xmm0 */
    "66440ff5f0",  /* pmaddwd %xmm0, %xmm14 */
    "66440ff534b500000000",  /* pmaddwd 0x0(,%rsi,4), %xmm14 */
    "66410ff6eb",  /* psadbw %xmm11, %xmm5 */
    "66420ff63c1d10000000",  /* psadbw 0x10(,%r11,1), %xmm7 */
    "66450ff8e9",  /* psubb %xmm9, %xmm13 */
    "66410ff817",  /* psubb (%r15), %xmm2 */
    "660ff9cf",  /* psubw %xmm7, %xmm1 */
    "660ff93c2534120000",  /* psubw 0x1234, %xmm7 */
    "660ffada",  /* psubd %xmm2, %xmm3 */
    "66440ffa34ed00100000",  /* psubd 0x1000(,%rbp,8), %xmm14 */
    "66410ffbf7",  /* psubq %xmm15, %xmm6 */
    "66440ffb6208",  /* psubq 8(%rdx), %xmm12 */
    "66410ffcc3",  /* paddb %xmm11, %xmm0 */
    "66430ffc84e5a0860100",  /* paddb 100000(%r13,%r12,8), %xmm0 */
    "660ffdd6",  /* paddw %xmm6, %xmm2 */
    "66450ffd3424",  /* paddw (%r12), %xmm14 */
    "66440ffee5",  /* paddd %xmm5, %xmm12 */
    "66450ffe5500",  /* paddd (%r13), %xmm10 */
    "66440f3800fd",  /* pshufb %xmm5, %xmm15 */
    "66410f380001",  /* pshufb (%r9), %xmm0 */
    "66410f3801f0",  /* phaddw %xmm8, %xmm6 */
    "66440f380108",  /* phaddw (%rax), %xmm9 */
    "66410f3802dc",  /* phaddd %xmm12, %xmm3 */
    "67660f38025110",  /* phaddd 16(%ecx), %xmm2 */
    "66410f3804e4",  /* pmaddubsw %xmm12, %xmm4 */
    "660f38040cab",  /* pmaddubsw (%rbx,%rbp,4), %xmm1 */
    "66440f3808f7",  /* psignb %xmm7, %xmm14 */
    "66450f380837",  /* psignb (%r15), %xmm14 */
    "660f380be5",  /* pmulhrsw %xmm5, %xmm4 */
    "66450f380b0c24",  /* pmulhrsw (%r12), %xmm9 */
    "66410f381ce2",  /* pabsb %xmm10, %xmm4 */
    "66440f381c4500",  /* pabsb (%rbp), %xmm8 */
    "660f381deb",  /* pabsw %xmm3, %xmm5 */
    "6566440f381d1c2500000000",  /* pabsw %gs:0x0, %xmm11 */
    "66410f381ec0",  /* pabsd %xmm8, %xmm0 */
    "66420f381e7432f8",  /* pabsd -8(%rdx,%r14,1), %xmm6 */
    "66440f3820c3",  /* pmovsxbw %xmm3, %xmm8 */
    "660f38201c2510000000",  /* pmovsxbw 0x10, %xmm3 */
    "660f3831e4",  /* pmovzxbd %xmm4, %xmm4 */
    "66440f383154ef04",  /* pmovzxbd 4(%rdi,%rbp,8), %xmm10 */
    "66440f3835cc",  /* pmovzxdq %xmm4, %xmm9 */
    "66440f383524fd10000000",  /* pmovzxdq 0x10(,%rdi,8), %xmm12 */
    "66440f3828f3",  /* pmuldq %xmm3, %xmm14 */
    "64660f38280c2528000000",  /* pmuldq %fs:0x28, %xmm1 */
    "660f3829fd",  /* pcmpeqq %xmm5, %xmm7 */
    "66440f38290424",  /* pcmpeqq (%rsp), %xmm8 */
    "660f382bd1",  /* packusdw %xmm1, %xmm2 */
    "66450f382b4500",  /* packusdw (%r13), %xmm8 */
    "66440f3837ca",  /* pcmpgtq %xmm2, %xmm9 */
    "66410f38377cfd00",  /* pcmpgtq (%r13,%rdi,8), %xmm7 */
    "660f3838c5",  /* pminsb %xmm5, %xmm0 */
    "66410f383884bd00040000",  /* pminsb 1024(%r13,%rdi,4), %xmm0 */
    "66450f3839f9",  /* pminsd %xmm9, %xmm15 */
    "66460f38394c74f8",  /* pminsd -8(%rsp,%r14,2), %xmm9 */
    "66450f383ad0",  /* pminuw %xmm8, %xmm10 */
    "6466440f383a142528000000",  /* pminuw %fs:0x28, %xmm10 */
    "66450f383bc4",  /* pminud %xmm12, %xmm8 */
    "66410f383b6500",  /* pminud (%r13), %xmm4 */
    "660f383cd7",  /* pmaxsb %xmm7, %xmm2 */
    "66440f383c5500",  /* pmaxsb (%rbp), %xmm10 */
    "66440f383dd1",  /* pmaxsd %xmm1, %xmm10 */
    "66440f383da67fffffff",  /* pmaxsd -129(%rsi), %xmm12 */
    "66440f383ee1",  /* pmaxuw %xmm1, %xmm12 */
    "66440f383e4d00",  /* pmaxuw (%rbp), %xmm9 */
    "66410f383fc5",  /* pmaxud %xmm13, %xmm0 */
    "66410f383f0c24",  /* pmaxud (%r12), %xmm1 */
    "660f3840c3",  /* pmulld %xmm3, %xmm0 */
    "66410f38402c7b",  /* pmulld (%r11,%rdi,2), %xmm5 */
    "660f3817ee",  /* ptest %xmm6, %xmm5 */
    "66440f38171c24",  /* ptest (%rsp), %xmm11 */
    "66450f3841de",  /* phminposuw %xmm14, %xmm11 */
    "66430f384124b3",  /* phminposuw (%r11,%r14,4), %xmm4 */
    "66410f38dcc3",  /* aesenc %xmm11, %xmm0 */
    "6766440f38dc07",  /* aesenc 0(%edi), %xmm8 */
    "66450f38dde2",  /* aesenclast %xmm10, %xmm12 */
    "66450f38dd5d00",  /* aesenclast (%r13), %xmm11 */
    "66410f38ded7",  /* aesdec %xmm15, %xmm2 */
    "6766410f38de6d10",  /* aesdec 16(%r13d), %xmm5 */
    "660f38dfe5",  /* aesdeclast %xmm5, %xmm4 */
    "66470f38df0428",  /* aesdeclast (%r8,%r13,1), %xmm8 */
    "66450f38dbd2",  /* aesimc %xmm10, %xmm10 */
    "66410f38dbb4ee00040000",  /* aesimc 1024(%r14,%rbp,8), %xmm6 */
    "f2450f7cc4",  /* haddps %xmm12, %xmm8 */
    "f2440f7c7d00",  /* haddps (%rbp), %xmm15 */
    "66440f7de2",  /* hsubpd %xmm2, %xmm12 */
    "6466440f7d3c2528000000",  /* hsubpd %fs:0x28, %xmm15 */
    "f20fd0ca",  /* addsubps %xmm2, %xmm1 */
    "f2420fd00c1500100000",  /* addsubps 0x1000(,%r10,1), %xmm1 */
    "660f71d101",  /* psrlw $1, %xmm1 */
    "66450fd1e7",  /* psrlw %xmm15, %xmm12 */
    "660fd14d00",  /* psrlw (%rbp), %xmm1 */
    "66410f72d007",  /* psrld $7, %xmm8 */
    "66410fd2fe",  /* psrld %xmm14, %xmm7 */
    "660fd2bb7fffffff",  /* psrld -129(%rbx), %xmm7 */
    "66410f73d01f",  /* psrlq $31, %xmm8 */
    "66450fd3dc",  /* psrlq %xmm12, %xmm11 */
    "660fd35500",  /* psrlq (%rbp), %xmm2 */
    "66410f71e407",  /* psraw $7, %xmm12 */
    "66440fe1f6",  /* psraw %xmm6, %xmm14 */
    "660fe11424",  /* psraw (%rsp), %xmm2 */
    "66410f72e601",  /* psrad $1, %xmm14 */
    "66410fe2cb",  /* psrad %xmm11, %xmm1 */
    "66440fe27e01",  /* psrad 1(%rsi), %xmm15 */
    "66410f71f701",  /* psllw $1, %xmm15 */
    "66440ff1ed",  /* psllw %xmm5, %xmm13 */
    "66410ff1707f",  /* psllw 127(%r8), %xmm6 */
    "66410f72f007",  /* pslld $7, %xmm8 */
    "66410ff2fb",  /* pslld %xmm11, %xmm7 */
    "660ff21c25ffffff7f",  /* pslld 0x7fffffff, %xmm3 */
    "660f73f61f",  /* psllq $31, %xmm6 */
    "66440ff3f8",  /* psllq %xmm0, %xmm15 */
    "66440ff3ac2fa0860100",  /* psllq 100000(%rdi,%rbp,1), %xmm13 */
    "f3490f2aca",  /* cvtsi2ss %r10, %xmm1 */
    "f3440f2a28",  /* cvtsi2ssl (%rax), %xmm13 */
    "f3480f2a7b08",  /* cvtsi2ssq 8(%rbx), %xmm7 */
    "f2440f2af8",  /* cvtsi2sd %eax, %xmm15 */
    "f24d0f2ad2",  /* cvtsi2sd %r10, %xmm10 */
    "f20f2a08",  /* cvtsi2sdl (%rax), %xmm1 */
    "f2480f2a7308",  /* cvtsi2sdq 8(%rbx), %xmm6 */
    "f30f2cc3",  /* cvttss2si %xmm3, %eax */
    "f34c0f2cd9",  /* cvttss2si %xmm1, %r11 */
    "f20f2cc1",  /* cvttsd2si %xmm1, %eax */
    "f24d0f2cdf",  /* cvttsd2si %xmm15, %r11 */
    "f30f2dc6",  /* cvtss2si %xmm6, %eax */
    "f34c0f2ddb",  /* cvtss2si %xmm3, %r11 */
    "f2410f2dc2",  /* cvtsd2si %xmm10, %eax */
    "f24d0f2ddb",  /* cvtsd2si %xmm11, %r11 */
    "4000fc",  /* add %dil, %spl */
    "67440018",  /* addb %r11b, 0(%eax) */
    "4180c7ff",  /* add $255, %r15b */
    "4280045f80",  /* addb $-128, (%rdi,%r11,2) */
    "04ff",  /* add $255, %al */
    "664401e0",  /* add %r12w, %ax */
    "66011429",  /* addw %dx, (%rcx,%rbp,1) */
    "6646032cd500100000",  /* add 0x1000(,%r10,8), %r13w */
    "664181c68000",  /* add $128, %r14w */
    "64668304252800000001",  /* addw $1, %fs:0x28 */
    "4501c5",  /* add %r8d, %r13d */
    "44010c2534120000",  /* addl %r9d, 0x1234 */
    "44031424",  /* add (%rsp), %r10d */
    "678343107f",  /* addl $127, 16(%ebx) */
    "4901f0",  /* add %rsi, %r8 */
    "4c018a7fffffff",  /* addq %r9, -129(%rdx) */
    "4d036bff",  /* add -1(%r11), %r13 */
    "4883c001",  /* add $1, %rax */
    "48830425100000007f",  /* addq $127, 0x10 */
    "4108cc",  /* or %cl, %r12b */
    "44081424",  /* orb %r10b, (%rsp) */
    "410a11",  /* or (%r9), %dl */
    "80c900",  /* or $0, %cl */
    "800c253412000001",  /* orb $1, 0x1234 */
    "0c01",  /* or $1, %al */
    "664109ca",  /* or %cx, %r10w */
    "65664409142528000000",  /* orw %r10w, %gs:0x28 */
    "66410b5d00",  /* or (%r13), %bx */
    "664181c8ff7f",  /* or $32767, %r8w */
    "6641818cb9000400008000",  /* orw $128, 1024(%r9,%rdi,4) */
    "660dff7f",  /* or $32767, %ax */
    "4409d0",  /* or %r10d, %eax */
    "6744096c2410",  /* orl %r13d, 16(%esp) */
    "450b1a",  /* or (%r10), %r11d */
    "4183cbff",  /* or $-1, %r11d */
    "810c050010000078563412",  /* orl $305419896, 0x1000(,%rax,1) */
    "4909c3",  /* or %rax, %r11 */
    "4d094500",  /* orq %r8, (%r13) */
    "4d0b1c93",  /* or (%r11,%rdx,4), %r11 */
    "4981c800000080",  /* or $-2147483648, %r8 */
    "48830c25100000007f",  /* orq $127, 0x10 */
    "480d78563412",  /* or $305419896, %rax */
    "4110e1",  /* adc %spl, %r9b */
    "40103cf500000000",  /* adcb %dil, 0x0(,%rsi,8) */
    "4412537f",  /* adc 127(%rbx), %r10b */
    "4180d601",  /* adc $1, %r14b */
    "4180528001",  /* adcb $1, -128(%r10) */
    "664511f0",  /* adc %r14w, %r8w */
    "66117980",  /* adcw %di, -128(%rcx) */
    "656613142528000000",  /* adc %gs:0x28, %dx */
    "664181d3ff7f",  /* adc $32767, %r11w */
    "66418154ad00ff7f",  /* adcw $32767, (%r13,%rbp,4) */
    "66158000",  /* adc $128, %ax */
    "4511dd",  /* adc %r11d, %r13d */
    "44110c2534120000",  /* adcl %r9d, 0x1234 */
    "44132cc6",  /* adc (%rsi,%rax,8), %r13d */
    "81d778563412",  /* adc $305419896, %edi */
    "83550000",  /* adcl $0, (%rbp) */
    "83d001",  /* adc $1, %eax */
    "4911d1",  /* adc %rdx, %r9 */
    "48115cc1f8",  /* adcq %rbx, -8(%rcx,%rax,8) */
    "6748136b10",  /* adc 16(%ebx), %rbp */
    "4981d178563412",  /* adc $305419896, %r9 */
    "4983550001",  /* adcq $1, (%r13) */
    "481578563412",  /* adc $305419896, %rax */
    "4118da",  /* sbb %bl, %r10b */
    "421824ad00000000",  /* sbbb %spl, 0x0(,%r13,4) */
    "401a7c4e04",  /* sbb 4(%rsi,%rcx,2), %dil */
    "4080dc7f",  /* sbb $127, %spl */
    "41805d0000",  /* sbbb $0, (%r13) */
    "1c01",  /* sbb $1, %al */
    "664519c5",  /* sbb %r8w, %r13w */
    "6641194d00",  /* sbbw %cx, (%r13) */
    "661b1c24",  /* sbb (%rsp), %bx */
    "6683da00",  /* sbb $0, %dx */
    "66811a8000",  /* sbbw $128, (%rdx) */
    "4519fa",  /* sbb %r15d, %r10d */
    "65192c2500100000",  /* sbbl %ebp, %gs:0x1000 */
    "1b342534120000",  /* sbb 0x1234, %esi */
    "4183dd7f",  /* sbb $127, %r13d */
    "42831ce000",  /* sbbl $0, (%rax,%r12,8) */
    "4819e9",  /* sbb %rbp, %rcx */
    "4d195d00",  /* sbbq %r11, (%r13) */
    "674d1b5710",  /* sbb 16(%r15d), %r10 */
    "4981dd80000000",  /* sbb $128, %r13 */
    "48811f80000000",  /* sbbq $128, (%rdi) */
    "481d78563412",  /* sbb $305419896, %rax */
    "4420c2",  /* and %r8b, %dl */
    "45206c9504",  /* andb %r13b, 4(%r13,%rdx,4) */
    "65221c2500100000",  /* and %gs:0x1000, %bl */
    "4180e4ff",  /* and $255, %r12b */
    "648024252800000000",  /* andb $0, %fs:0x28 */
    "2401",  /* and $1, %al */
    "664121fc",  /* and %di, %r12w */
    "66210c24",  /* andw %cx, (%rsp) */
    "664423142510000000",  /* and 0x10, %r10w */
    "6683650000",  /* andw $0, (%rbp) */
    "21cf",  /* and %ecx, %edi */
    "41211c24",  /* andl %ebx, (%r12) */
    "41231424",  /* and (%r12), %edx */
    "83e501",  /* and $1, %ebp */
    "4383a496a0860100ff",  /* andl $-1, 100000(%r14,%r10,4) */
    "4d21e3",  /* and %r12, %r11 */
    "4d216c2480",  /* andq %r13, -128(%r12) */
    "4c233c25ffffff7f",  /* and 0x7fffffff, %r15 */
    "4a8324c301",  /* andq $1, (%rbx,%r8,8) */
    "4883e0ff",  /* and $-1, %rax */
    "4428f5",  /* sub %r14b, %bpl */
    "4028249500100000",  /* subb %spl, 0x1000(,%rdx,4) */
    "432ab499a0860100",  /* sub 100000(%r9,%r11,4), %sil */
    "2c7f",  /* sub $127, %al */
    "806d007f",  /* subb $127, (%rbp) */
    "664129fa",  /* sub %di, %r10w */
    "6647297ce500",  /* subw %r15w, (%r13,%r12,8) */
    "6766412b17",  /* sub 0(%r15d), %dx */
    "6681e98000",  /* sub $128, %cx */
    "6642816c5f048000",  /* subw $128, 4(%rdi,%r11,2) */
    "6683e87f",  /* sub $127, %ax */
    "29fe",  /* sub %edi, %esi */
    "4629441500",  /* subl %r8d, (%rbp,%r10,1) */
    "422b2cb500000000",  /* sub 0x0(,%r14,4), %ebp */
    "81ea80000000",  /* sub $128, %edx */
    "836b7fff",  /* subl $-1, 127(%rbx) */
    "83e87f",  /* sub $127, %eax */
    "4929cb",  /* sub %rcx, %r11 */
    "674d2932",  /* subq %r14, 0(%r10d) */
    "65482b342500000000",  /* sub %gs:0x0, %rsi */
    "4983efff",  /* sub $-1, %r15 */
    "49812c8a80000000",  /* subq $128, (%r10,%rcx,4) */
    "4883e800",  /* sub $0, %rax */
    "4430eb",  /* xor %r13b, %bl */
    "45303424",  /* xorb %r14b, (%r12) */
    "654432042528000000",  /* xor %gs:0x28, %r8b */
    "4180f200",  /* xor $0, %r10b */
    "803424ff",  /* xorb $255, (%rsp) */
    "347f",  /* xor $127, %al */
    "664131c9",  /* xor %cx, %r9w */
    "66316cf500",  /* xorw %bp, (%rbp,%rsi,8) */
    "6633a334120000",  /* xor 4660(%rbx), %sp */
    "664183f57f",  /* xor $127, %r13w */
    "664281348510000000ff7f",  /* xorw $32767, 0x10(,%r8,4) */
    "6683f001",  /* xor $1, %ax */
    "31d3",  /* xor %edx, %ebx */
    "312c24",  /* xorl %ebp, (%rsp) */
    "44333c0500000000",  /* xor 0x0(,%rax,1), %r15d */
    "4181f078563412",  /* xor $305419896, %r8d */
    "83342401",  /* xorl $1, (%rsp) */
    "4c31f2",  /* xor %r14, %rdx */
    "4c315d00",  /* xorq %r11, (%rbp) */
    "48336bff",  /* xor -1(%rbx), %rbp */
    "4883f6ff",  /* xor $-1, %rsi */
    "48813425ffffff7f78563412",  /* xorq $305419896, 0x7fffffff */
    "4883f001",  /* xor $1, %rax */
    "4138e4",  /* cmp %spl, %r12b */
    "40383424",  /* cmpb %sil, (%rsp) */
    "463a342d10000000",  /* cmp 0x10(,%r13,1), %r14b */
    "4180fc80",  /* cmp $-128, %r12b */
    "803c247f",  /* cmpb $127, (%rsp) */
    "664539cf",  /* cmp %r9w, %r15w */
    "67663903",  /* cmpw %ax, 0(%ebx) */
    "66413bb457a0860100",  /* cmp 100000(%r15,%rdx,2), %si */
    "6681feff7f",  /* cmp $32767, %si */
    "6641833c2400",  /* cmpw $0, (%r12) */
    "39e4",  /* cmp %esp, %esp */
    "4439717f",  /* cmpl %r14d, 127(%rcx) */
    "67413b07",  /* cmp 0(%r15d), %eax */
    "4183fa7f",  /* cmp $127, %r10d */
    "64833c25001000007f",  /* cmpl $127, %fs:0x1000 */
    "4c39d6",  /* cmp %r10, %rsi */
    "4b3954e5f8",  /* cmpq %rdx, -8(%r13,%r12,8) */
    "64483b142528000000",  /* cmp %fs:0x28, %rdx */
    "4881fe00000080",  /* cmp $-2147483648, %rsi */
    "4b833cc4ff",  /* cmpq $-1, (%r12,%r8,8) */
    "4883f8ff",  /* cmp $-1, %rax */
    "4184d5",  /* test %dl, %r13b */
    "40847500",  /* test %sil, (%rbp) */
    "41f6c680",  /* test $-128, %r14b */
    "f6450001",  /* testb $1, (%rbp) */
    "4588de",  /* mov %r11b, %r14b */
    "6744881e",  /* mov %r11b, 0(%esi) */
    "408a2c5510000000",  /* mov 0x10(,%rdx,2), %bpl */
    "b001",  /* mov $1, %al */
    "67c60001",  /* movb $1, 0(%eax) */
    "40f6d4",  /* not %spl */
    "f6957fffffff",  /* notb -129(%rbp) */
    "40f6dd",  /* neg %bpl */
    "f618",  /* negb (%rax) */
    "41f6e6",  /* mul %r14b */
    "64f6242500000000",  /* mulb %fs:0x0 */
    "41f62c24",  /* imulb (%r12) */
    "f6f0",  /* div %al */
    "f6b4cfa0860100",  /* divb 100000(%rdi,%rcx,8) */
    "43f63c12",  /* idivb (%r10,%r10,1) */
    "41fec3",  /* inc %r11b */
    "fe417f",  /* incb 127(%rcx) */
    "41fec8",  /* dec %r8b */
    "fe4d00",  /* decb (%rbp) */
    "41d0c0",  /* rol %r8b */
    "41d0c2",  /* rol $1, %r10b */
    "41d2c3",  /* rol %cl, %r11b */
    "c0818000000002",  /* rolb $2, 128(%rcx) */
    "d0ca",  /* ror %dl */
    "c0c807",  /* ror $7, %al */
    "d08834120000",  /* rorb $1, 4660(%rax) */
    "d0d1",  /* rcl %cl */
    "c0d103",  /* rcl $3, %cl */
    "40d2d6",  /* rcl %cl, %sil */
    "41c0142405",  /* rclb $5, (%r12) */
    "41d0df",  /* rcr %r15b */
    "41c0df03",  /* rcr $3, %r15b */
    "41d2db",  /* rcr %cl, %r11b */
    "41c01c2405",  /* rcrb $5, (%r12) */
    "41d0e4",  /* shl %r12b */
    "41d0e1",  /* shl $1, %r9b */
    "40d2e5",  /* shl %cl, %bpl */
    "41c064247f02",  /* shlb $2, 127(%r12) */
    "41d0ec",  /* shr %r12b */
    "41d0ec",  /* shr $1, %r12b */
    "41d2eb",  /* shr %cl, %r11b */
    "64d02c2528000000",  /* shrb $1, %fs:0x28 */
    "41d0ff",  /* sar %r15b */
    "41c0f803",  /* sar $3, %r8b */
    "40d2fd",  /* sar %cl, %bpl */
    "67c03c2405",  /* sarb $5, 0(%esp) */
    "40d0e5",  /* sal %bpl */
    "c0e007",  /* sal $7, %al */
    "41d2e6",  /* sal %cl, %r14b */
    "41d0a288a9cbed",  /* salb $1, -305419896(%r10) */
    "6685c9",  /* test %cx, %cx */
    "6644857500",  /* test %r14w, (%rbp) */
    "6641f7c40100",  /* test $1, %r12w */
    "6566f70425000000000000",  /* testw $0, %gs:0x0 */
    "6642890c14",  /* mov %cx, (%rsp,%r10,1) */
    "668b7f80",  /* mov -128(%rdi), %di */
    "6641bdff7f",  /* mov $32767, %r13w */
    "676641c706ff7f",  /* movw $32767, 0(%r14d) */
    "66f7d5",  /* not %bp */
    "6766f710",  /* notw 0(%eax) */
    "66f7db",  /* neg %bx */
    "6641f75cc804",  /* negw 4(%r8,%rcx,8) */
    "6642f7a49ba0860100",  /* mulw 100000(%rbx,%r11,4) */
    "6641f7ec",  /* imul %r12w */
    "676641f729",  /* imulw 0(%r9d) */
    "6641f7f6",  /* div %r14w */
    "66f7f8",  /* idiv %ax */
    "6641f77d00",  /* idivw (%r13) */
    "6641ffc2",  /* inc %r10w */
    "6466ff042500100000",  /* incw %fs:0x1000 */
    "6641ffcb",  /* dec %r11w */
    "66ff0c25ffffff7f",  /* decw 0x7fffffff */
    "66d1c6",  /* rol %si */
    "6641d1c3",  /* rol $1, %r11w */
    "66d3c1",  /* rol %cl, %cx */
    "6643c144850402",  /* rolw $2, 4(%r13,%r8,4) */
    "66d1ce",  /* ror %si */
    "66c1cd03",  /* ror $3, %bp */
    "66d3ce",  /* ror %cl, %si */
    "6643c18cbea086010005",  /* rorw $5, 100000(%r14,%r15,4) */
    "66d1d6",  /* rcl %si */
    "6641c1d507",  /* rcl $7, %r13w */
    "6641d3d7",  /* rcl %cl, %r15w */
    "6641c1550002",  /* rclw $2, (%r13) */
    "66d1d8",  /* rcr %ax */
    "66d1db",  /* rcr $1, %bx */
    "66d3d8",  /* rcr %cl, %ax */
    "6643c11c0905",  /* rcrw $5, (%r9,%r9,1) */
    "66d1e6",  /* shl $1, %si */
    "66d3e6",  /* shl %cl, %si */
    "66c1650002",  /* shlw $2, (%rbp) */
    "66d1ee",  /* shr %si */
    "6641c1e907",  /* shr $7, %r9w */
    "66d3ec",  /* shr %cl, %sp */
    "66d16d00",  /* shrw $1, (%rbp) */
    "66d1fa",  /* sar %dx */
    "66d1fc",  /* sar $1, %sp */
    "6641d3ff",  /* sar %cl, %r15w */
    "6642c13cad0010000005",  /* sarw $5, 0x1000(,%r13,4) */
    "66d1e5",  /* sal %bp */
    "6641c1e103",  /* sal $3, %r9w */
    "6641d3e6",  /* sal %cl, %r14w */
    "6641c1242402",  /* salw $2, (%r12) */
    "4585c9",  /* test %r9d, %r9d */
    "854500",  /* test %eax, (%rbp) */
    "f7c77f000000",  /* test $127, %edi */
    "f70078563412",  /* testl $305419896, (%rax) */
    "89e0",  /* mov %esp, %eax */
    "8928",  /* mov %ebp, (%rax) */
    "8b4d00",  /* mov (%rbp), %ecx */
    "bd80000000",  /* mov $128, %ebp */
    "41c70001000000",  /* movl $1, (%r8) */
    "41f7d2",  /* not %r10d */
    "f713",  /* notl (%rbx) */
    "41f7d9",  /* neg %r9d */
    "42f71cdd10000000",  /* negl 0x10(,%r11,8) */
    "41f7e0",  /* mul %r8d */
    "65f7242500000000",  /* mull %gs:0x0 */
    "f7ed",  /* imul %ebp */
    "6741f76a10",  /* imull 16(%r10d) */
    "f7f4",  /* div %esp */
    "f7341500000000",  /* divl 0x0(,%rdx,1) */
    "41f7ff",  /* idiv %r15d */
    "41f73e",  /* idivl (%r14) */
    "41ffc5",  /* inc %r13d */
    "64ff042500000000",  /* incl %fs:0x0 */
    "ffcc",  /* dec %esp */
    "41ff0c24",  /* decl (%r12) */
    "41d1c6",  /* rol %r14d */
    "41c1c003",  /* rol $3, %r8d */
    "41d3c0",  /* rol %cl, %r8d */
    "42c104551000000002",  /* roll $2, 0x10(,%r10,2) */
    "d1c9",  /* ror %ecx */
    "41c1cf03",  /* ror $3, %r15d */
    "d3cf",  /* ror %cl, %edi */
    "67c14b1002",  /* rorl $2, 16(%ebx) */
    "41d1d7",  /* rcl %r15d */
    "41d1d0",  /* rcl $1, %r8d */
    "41d3d0",  /* rcl %cl, %r8d */
    "d1142510000000",  /* rcll $1, 0x10 */
    "41c1de07",  /* rcr $7, %r14d */
    "d3de",  /* rcr %cl, %esi */
    "65c11c252800000002",  /* rcrl $2, %gs:0x28 */
    "41d1e2",  /* shl %r10d */
    "d3e6",  /* shl %cl, %esi */
    "65d1242500100000",  /* shll $1, %gs:0x1000 */
    "41d1e9",  /* shr %r9d */
    "d1ec",  /* shr $1, %esp */
    "41d3ed",  /* shr %cl, %r13d */
    "c12c251000000002",  /* shrl $2, 0x10 */
    "d1fe",  /* sar %esi */
    "c1fa07",  /* sar $7, %edx */
    "d3fb",  /* sar %cl, %ebx */
    "c178ff05",  /* sarl $5, -1(%rax) */
    "41d1e5",  /* sal %r13d */
    "41c1e703",  /* sal $3, %r15d */
    "41d3e6",  /* sal %cl, %r14d */
    "c124d50000000002",  /* sall $2, 0x0(,%rdx,8) */
    "4885ec",  /* test %rbp, %rsp */
    "4b8524de",  /* test %rsp, (%r14,%r11,8) */
    "48a9ffffffff",  /* test $-1, %rax */
    "48f70425ffffff7f01000000",  /* testq $1, 0x7fffffff */
    "4989e9",  /* mov %rbp, %r9 */
    "48893444",  /* mov %rsi, (%rsp,%rax,2) */
    "4d8b2c24",  /* mov (%r12), %r13 */
    "48c7c580000000",  /* mov $128, %rbp */
    "48c704550000000000000000",  /* movq $0, 0x0(,%rdx,2) */
    "49f7d1",  /* not %r9 */
    "49f71434",  /* notq (%r12,%rsi,1) */
    "49f71b",  /* negq (%r11) */
    "49f7e7",  /* mul %r15 */
    "48f7a432a0860100",  /* mulq 100000(%rdx,%rsi,1) */
    "49f7ea",  /* imul %r10 */
    "48f76d00",  /* imulq (%rbp) */
    "49f7f6",  /* div %r14 */
    "6749f77610",  /* divq 16(%r14d) */
    "48f73c24",  /* idivq (%rsp) */
    "49ffc1",  /* inc %r9 */
    "4bff0471",  /* incq (%r9,%r14,2) */
    "48ff0c1500000000",  /* decq 0x0(,%rdx,1) */
    "49d1c5",  /* rol %r13 */
    "48c1c507",  /* rol $7, %rbp */
    "48d3c3",  /* rol %cl, %rbx */
    "48c1450005",  /* rolq $5, (%rbp) */
    "48d1cb",  /* ror %rbx */
    "49c1cb03",  /* ror $3, %r11 */
    "49d3c8",  /* ror %cl, %r8 */
    "48d14d00",  /* rorq $1, (%rbp) */
    "48d1d6",  /* rcl %rsi */
    "49c1d603",  /* rcl $3, %r14 */
    "49d3d3",  /* rcl %cl, %r11 */
    "6548c114250010000005",  /* rclq $5, %gs:0x1000 */
    "49d1da",  /* rcr %r10 */
    "49c1d903",  /* rcr $3, %r9 */
    "48d15f7f",  /* rcrq $1, 127(%rdi) */
    "48d1e7",  /* shl %rdi */
    "49c1e503",  /* shl $3, %r13 */
    "49d3e0",  /* shl %cl, %r8 */
    "49c1247702",  /* shlq $2, (%r15,%rsi,2) */
    "49d1ee",  /* shr %r14 */
    "48c1e907",  /* shr $7, %rcx */
    "48d3ea",  /* shr %cl, %rdx */
    "48c12c251000000002",  /* shrq $2, 0x10 */
    "49d1fc",  /* sar %r12 */
    "48c1fe03",  /* sar $3, %rsi */
    "48d3fa",  /* sar %cl, %rdx */
    "49c13c2402",  /* sarq $2, (%r12) */
    "48d1e3",  /* sal %rbx */
    "49c1e403",  /* sal $3, %r12 */
    "49d3e3",  /* sal %cl, %r11 */
    "4ad124cd10000000",  /* salq $1, 0x10(,%r9,8) */
    "66450fafde",  /* imul %r14w, %r11w */
    "66410faf7f7f",  /* imul 127(%r15), %di */
    "66416bd300",  /* imul $0, %r11w, %dx */
    "6766446916e803",  /* imul $1000, 0(%esi), %r10w */
    "668d042510000000",  /* lea 0x10, %ax */
    "66450f43f2",  /* cmovnc %r10w, %r14w */
    "65660f441c2500100000",  /* cmove %gs:0x1000, %bx */
    "66440fa3e7",  /* bt %r12w, %di */
    "66450fa33c69",  /* bt %r15w, (%r9,%rbp,2) */
    "660fbae501",  /* bt $1, %bp */
    "6766410fba2303",  /* btw $3, 0(%r11d) */
    "66440fabe0",  /* bts %r12w, %ax */
    "66450fab3c6f",  /* bts %r15w, (%r15,%rbp,2) */
    "660fbaee1f",  /* bts $31, %si */
    "66410fba2c2403",  /* btsw $3, (%r12) */
    "66450fb3cd",  /* btr %r9w, %r13w */
    "66440fb38a7fffffff",  /* btr %r9w, -129(%rdx) */
    "660fbaf21f",  /* btr $31, %dx */
    "66410fba3203",  /* btrw $3, (%r10) */
    "66440fbbeb",  /* btc %r13w, %bx */
    "66440fbb142534120000",  /* btc %r10w, 0x1234 */
    "66410fbafc01",  /* btc $1, %r12w */
    "660fbab93412000003",  /* btcw $3, 4660(%rcx) */
    "660fbcf8",  /* bsf %ax, %di */
    "6766450fbc27",  /* bsf 0(%r15d), %r12w */
    "66450fbdd9",  /* bsr %r9w, %r11w */
    "66450fbd5500",  /* bsr (%r13), %r10w */
    "66f3440fbcdc",  /* tzcnt %sp, %r11w */
    "6766f3410fbc0e",  /* tzcnt 0(%r14d), %cx */
    "66f3450fbdcd",  /* lzcnt %r13w, %r9w */
    "66f3430fbd5c77f8",  /* lzcnt -8(%r15,%r14,2), %bx */
    "66f3410fb8de",  /* popcnt %r14w, %bx */
    "66f30fb82c24",  /* popcnt (%rsp), %bp */
    "66410fa4ca03",  /* shld $3, %cx, %r10w */
    "66470fad7c7c04",  /* shrd %cl, %r15w, 4(%r12,%r15,2) */
    "66450fa5e3",  /* shld %r12w, %r11w */
    "66440fc1a4ad00040000",  /* xadd %r12w, 1024(%rbp,%rbp,4) */
    "66450fb102",  /* cmpxchg %r8w, (%r10) */
    "66f00fb19c6fa0860100",  /* lock cmpxchg %bx, 100000(%rdi,%rbp,2) */
    "66f0430fc16c67f8",  /* lock xadd %bp, -8(%r15,%r12,2) */
    "664487df",  /* xchg %r11w, %di */
    "6641872c24",  /* xchg %bp, (%r12) */
    "6766450f38f05e10",  /* movbe 16(%r14d), %r11w */
    "66410f38f14e01",  /* movbe %cx, 1(%r14) */
    "440fafc5",  /* imul %ebp, %r8d */
    "440faf2424",  /* imul (%rsp), %r12d */
    "456bdf80",  /* imul $-128, %r15d, %r11d */
    "691c2510000000e8030000",  /* imul $1000, 0x10, %ebx */
    "8d2425ffffff7f",  /* lea 0x7fffffff, %esp */
    "410f44da",  /* cmovz %r10d, %ebx */
    "650f44242528000000",  /* cmove %gs:0x28, %esp */
    "450fa3f3",  /* bt %r14d, %r11d */
    "450fa33b",  /* bt %r15d, (%r11) */
    "0fbae201",  /* bt $1, %edx */
    "410fba242403",  /* btl $3, (%r12) */
    "450fabca",  /* bts %r9d, %r10d */
    "440fab2cbd00100000",  /* bts %r13d, 0x1000(,%rdi,4) */
    "410fbaef1f",  /* bts $31, %r15d */
    "650fba2c250010000003",  /* btsl $3, %gs:0x1000 */
    "650fb31c2500000000",  /* btr %ebx, %gs:0x0 */
    "410fbaf61f",  /* btr $31, %r14d */
    "430fbab40d0004000003",  /* btrl $3, 1024(%r13,%r9,1) */
    "410fbbcf",  /* btc %ecx, %r15d */
    "0fbb740d00",  /* btc %esi, (%rbp,%rcx,1) */
    "410fbafc01",  /* btc $1, %r12d */
    "0fba3c2403",  /* btcl $3, (%rsp) */
    "450fbcc7",  /* bsf %r15d, %r8d */
    "640fbc1c2500000000",  /* bsf %fs:0x0, %ebx */
    "450fbddf",  /* bsr %r15d, %r11d */
    "0fbd2c2534120000",  /* bsr 0x1234, %ebp */
    "f3410fbcf0",  /* tzcnt %r8d, %esi */
    "f30fbc9caf00040000",  /* tzcnt 1024(%rdi,%rbp,4), %ebx */
    "f30fbde3",  /* lzcnt %ebx, %esp */
    "f30fbd5f7f",  /* lzcnt 127(%rdi), %ebx */
    "f3410fb8e2",  /* popcnt %r10d, %esp */
    "f3460fb804d9",  /* popcnt (%rcx,%r11,8), %r8d */
    "410fa4e303",  /* shld $3, %esp, %r11d */
    "440fad342534120000",  /* shrd %cl, %r14d, 0x1234 */
    "450fa5c1",  /* shld %r8d, %r9d */
    "67440fc101",  /* xadd %r8d, 0(%ecx) */
    "0fb12c2510000000",  /* cmpxchg %ebp, 0x10 */
    "65f00fb1342500000000",  /* lock cmpxchg %esi, %gs:0x0 */
    "f0420fc11c26",  /* lock xadd %ebx, (%rsi,%r12,1) */
    "4587fd",  /* xchg %r15d, %r13d */
    "46870c8b",  /* xchg %r9d, (%rbx,%r9,4) */
    "670f38f012",  /* movbe 0(%edx), %edx */
    "0f38f160ff",  /* movbe %esp, -1(%rax) */
    "4d0faff4",  /* imul %r12, %r14 */
    "480faf0424",  /* imul (%rsp), %rax */
    "4d6be900",  /* imul $0, %r9, %r13 */
    "4c691480e8030000",  /* imul $1000, (%rax,%rax,4), %r10 */
    "64488d042528000000",  /* lea %fs:0x28, %rax */
    "4d0f43d6",  /* cmovnc %r14, %r10 */
    "4f0f445c06f8",  /* cmove -8(%r14,%r8,1), %r11 */
    "480fa3f7",  /* bt %rsi, %rdi */
    "480fa32c2534120000",  /* bt %rbp, 0x1234 */
    "480fbae31f",  /* bt $31, %rbx */
    "480fba242403",  /* btq $3, (%rsp) */
    "4c0fabe0",  /* bts %r12, %rax */
    "4e0fab94c600040000",  /* bts %r10, 1024(%rsi,%r8,8) */
    "490fbaed01",  /* bts $1, %r13 */
    "480fba2c151000000003",  /* btsq $3, 0x10(,%rdx,1) */
    "4c0fb3cc",  /* btr %r9, %rsp */
    "4d0fb33424",  /* btr %r14, (%r12) */
    "480fbaf701",  /* btr $1, %rdi */
    "64480fba34252800000003",  /* btrq $3, %fs:0x28 */
    "490fbbd7",  /* btc %rdx, %r15 */
    "480fbb3c2510000000",  /* btc %rdi, 0x10 */
    "480fbaf81f",  /* btc $31, %rax */
    "490fba3f03",  /* btcq $3, (%r15) */
    "480fbcc8",  /* bsf %rax, %rcx */
    "654c0fbc0c2528000000",  /* bsf %gs:0x28, %r9 */
    "490fbde1",  /* bsr %r9, %rsp */
    "480fbd1c3d00100000",  /* bsr 0x1000(,%rdi,1), %rbx */
    "f3480fbcfe",  /* tzcnt %rsi, %rdi */
    "f3480fbca580000000",  /* tzcnt 128(%rbp), %rsp */
    "f3480fbdd5",  /* lzcnt %rbp, %rdx */
    "f34e0fbd3c8500000000",  /* lzcnt 0x0(,%r8,4), %r15 */
    "f34c0fb8ea",  /* popcnt %rdx, %r13 */
    "65f3480fb8042528000000",  /* popcnt %gs:0x28, %rax */
    "4d0fa4d103",  /* shld $3, %r10, %r9 */
    "4e0fad3c6510000000",  /* shrd %cl, %r15, 0x10(,%r12,2) */
    "4c0fa5db",  /* shld %r11, %rbx */
    "4d0fc14500",  /* xadd %r8, (%r13) */
    "674c0fb111",  /* cmpxchg %r10, 0(%ecx) */
    "f04d0fb1a41400040000",  /* lock cmpxchg %r12, 1024(%r12,%rdx,1) */
    "f0480fc12c24",  /* lock xadd %rbp, (%rsp) */
    "4d87ea",  /* xchg %r13, %r10 */
    "4c872425ffffff7f",  /* xchg %r12, 0x7fffffff */
    "4c0f38f02425ffffff7f",  /* movbe 0x7fffffff, %r12 */
    "4c0f38f10424",  /* movbe %r8, (%rsp) */
    "0fce",  /* bswap %esi */
    "f2410f38f15d00",  /* crc32l (%r13), %ebx */
    "410fc7f4",  /* rdrand %r12d */
    "410fc7ff",  /* rdseed %r15d */
    "66450f38f6e7",  /* adcx %r15d, %r12d */
    "f3440f38f62424",  /* adox (%rsp), %r12d */
    "c4e250f2c1",  /* andn %ecx, %ebp, %eax */
    "c4a260f25cf3f8",  /* andn -8(%rbx,%r14,8), %ebx, %ebx */
    "c46260f7ce",  /* bextr %ebx, %esi, %r9d */
    "c4e258f7342510000000",  /* bextr %esp, 0x10, %esi */
    "c44210f5d7",  /* bzhi %r13d, %r15d, %r10d */
    "c44208f51424",  /* bzhi %r14d, (%r12), %r10d */
    "c4c22af7f4",  /* sarx %r10d, %r12d, %esi */
    "c4e262f73ccd00000000",  /* sarx %ebx, 0x0(,%rcx,8), %edi */
    "c4c241f7fa",  /* shlx %edi, %r10d, %edi */
    "64c4e269f7342500100000",  /* shlx %edx, %fs:0x1000, %esi */
    "c4621bf7c8",  /* shrx %r12d, %eax, %r9d */
    "c40243f70430",  /* shrx %edi, (%r8,%r14,1), %r8d */
    "c4424bf5cb",  /* pdep %r11d, %esi, %r9d */
    "c4e222f5f4",  /* pext %esp, %r11d, %esi */
    "c46263f6f6",  /* mulx %esi, %ebx, %r14d */
    "c4c238f3cd",  /* blsr %r13d, %r8d */
    "c4e218f3d1",  /* blsmsk %ecx, %r12d */
    "c4c230f3d8",  /* blsi %r8d, %r9d */
    "c4637bf0f605",  /* rorx $5, %esi, %r14d */
    "420fc3341d00000000",  /* movnti %esi, 0x0(,%r11,1) */
    "490fcd",  /* bswap %r13 */
    "64f2480f38f11c2500100000",  /* crc32q %fs:0x1000, %rbx */
    "480fc7f0",  /* rdrand %rax */
    "480fc7fd",  /* rdseed %rbp */
    "664d0f38f6da",  /* adcx %r10, %r11 */
    "67f3480f38f65910",  /* adox 16(%ecx), %rbx */
    "c442e8f2ea",  /* andn %r10, %rdx, %r13 */
    "65c4e288f22c2528000000",  /* andn %gs:0x28, %r14, %rbp */
    "c4e280f7d8",  /* bextr %r15, %rax, %rbx */
    "65c4e2d0f7342500100000",  /* bextr %rbp, %gs:0x1000, %rsi */
    "c44288f5fc",  /* bzhi %r14, %r12, %r15 */
    "65c462a0f53c2528000000",  /* bzhi %r11, %gs:0x28, %r15 */
    "c4628af7f8",  /* sarx %r14, %rax, %r15 */
    "c4e2eaf72c1d00100000",  /* sarx %rdx, 0x1000(,%rbx,1), %rbp */
    "c442d1f7d0",  /* shlx %rbp, %r8, %r10 */
    "64c4e2e9f7042528000000",  /* shlx %rdx, %fs:0x28, %rax */
    "c4c2f3f7ec",  /* shrx %rcx, %r12, %rbp */
    "c4c2e3f70424",  /* shrx %rbx, (%r12), %rax */
    "c4c293f5d6",  /* pdep %r14, %r13, %rdx */
    "c4e2eaf5d3",  /* pext %rbx, %rdx, %rdx */
    "c462e3f6c6",  /* mulx %rsi, %rbx, %r8 */
    "c4e2a8f3ca",  /* blsr %rdx, %r10 */
    "c4e2c8f3d0",  /* blsmsk %rax, %rsi */
    "c4e288f3d9",  /* blsi %rcx, %r14 */
    "c4e3fbf0d905",  /* rorx $5, %rcx, %rbx */
    "65480fc3342528000000",  /* movnti %rsi, %gs:0x28 */
    "660fb6f0",  /* movzbw %al, %si */
    "66450fb62c24",  /* movzbw (%r12), %r13w */
    "66440fbedc",  /* movsbw %spl, %r11w */
    "66440fbe4d00",  /* movsbw (%rbp), %r9w */
    "410fb6d1",  /* movzbl %r9b, %edx */
    "460fb6a4cd00040000",  /* movzbl 1024(%rbp,%r9,8), %r12d */
    "440fbed7",  /* movsbl %dil, %r10d */
    "440fbe1cb500000000",  /* movsbl 0x0(,%rsi,4), %r11d */
    "480fb6ea",  /* movzbq %dl, %rbp */
    "4c0fb6242534120000",  /* movzbq 0x1234, %r12 */
    "490fbef5",  /* movsbq %r13b, %rsi */
    "4d0fbe9b88a9cbed",  /* movsbq -305419896(%r11), %r11 */
    "410fb7ce",  /* movzwl %r14w, %ecx */
    "450fb77d00",  /* movzwl (%r13), %r15d */
    "440fbfce",  /* movswl %si, %r9d */
    "440fbfb27fffffff",  /* movswl -129(%rdx), %r14d */
    "490fb7ec",  /* movzwq %r12w, %rbp */
    "4b0fb744a0f8",  /* movzwq -8(%r8,%r12,4), %rax */
    "480fbff5",  /* movswq %bp, %rsi */
    "4f0fbf5c9e04",  /* movswq 4(%r14,%r11,4), %r11 */
    "4863de",  /* movslq %esi, %rbx */
    "4d630a",  /* movslq (%r10), %r9 */
    "410f90c7",  /* seto %r15b */
    "67410f904110",  /* seto 16(%r9d) */
    "400f91c5",  /* setno %bpl */
    "410f914500",  /* setno (%r13) */
    "410f92c7",  /* setb %r15b */
    "640f92042500100000",  /* setb %fs:0x1000 */
    "0f93c1",  /* setae %cl */
    "640f93042500000000",  /* setae %fs:0x0 */
    "0f94c1",  /* sete %cl */
    "0f944500",  /* sete (%rbp) */
    "410f95c3",  /* setne %r11b */
    "670f9500",  /* setne 0(%eax) */
    "650f96042500100000",  /* setbe %gs:0x1000 */
    "650f97042500000000",  /* seta %gs:0x0 */
    "410f98c6",  /* sets %r14b */
    "0f98042534120000",  /* sets 0x1234 */
    "410f99c4",  /* setns %r12b */
    "0f9902",  /* setns (%rdx) */
    "410f9ac0",  /* setp %r8b */
    "0f9a042510000000",  /* setp 0x10 */
    "0f9bc0",  /* setnp %al */
    "0f9b043b",  /* setnp (%rbx,%rdi,1) */
    "410f9cc4",  /* setl %r12b */
    "0f9c042510000000",  /* setl 0x10 */
    "410f9dc5",  /* setge %r13b */
    "410f9d00",  /* setge (%r8) */
    "400f9ec5",  /* setle %bpl */
    "0f9fc0",  /* setg %al */
    "430f9f44cbf8",  /* setg -8(%r11,%r9,8) */
    "450f58db",  /* addps %xmm11, %xmm11 */
    "0f583c1d10000000",  /* addps 0x10(,%rbx,1), %xmm7 */
    "66440f58fd",  /* addpd %xmm5, %xmm15 */
    "66440f5834d500100000",  /* addpd 0x1000(,%rdx,8), %xmm14 */
    "f3440f58e5",  /* addss %xmm5, %xmm12 */
    "f3420f5814ed00100000",  /* addss 0x1000(,%r13,8), %xmm2 */
    "f2440f58e6",  /* addsd %xmm6, %xmm12 */
    "f2440f589e88a9cbed",  /* addsd -305419896(%rsi), %xmm11 */
    "450f59c3",  /* mulps %xmm11, %xmm8 */
    "420f5924dd00100000",  /* mulps 0x1000(,%r11,8), %xmm4 */
    "66410f59d3",  /* mulpd %xmm11, %xmm2 */
    "66420f5934bd00100000",  /* mulpd 0x1000(,%r15,4), %xmm6 */
    "f3440f59e6",  /* mulss %xmm6, %xmm12 */
    "f3460f59448ef8",  /* mulss -8(%rsi,%r9,4), %xmm8 */
    "f2410f59fe",  /* mulsd %xmm14, %xmm7 */
    "f20f598388a9cbed",  /* mulsd -305419896(%rbx), %xmm0 */
    "410f5ccd",  /* subps %xmm13, %xmm1 */
    "670f5c642410",  /* subps 16(%esp), %xmm4 */
    "66410f5ce8",  /* subpd %xmm8, %xmm5 */
    "66410f5c6d00",  /* subpd (%r13), %xmm5 */
    "f3410f5cd7",  /* subss %xmm15, %xmm2 */
    "f3450f5cac03a0860100",  /* subss 100000(%r11,%rax,1), %xmm13 */
    "f2410f5cdd",  /* subsd %xmm13, %xmm3 */
    "67f2450f5c5910",  /* subsd 16(%r9d), %xmm11 */
    "410f5dd4",  /* minps %xmm12, %xmm2 */
    "450f5d7500",  /* minps (%r13), %xmm14 */
    "66440f5dc3",  /* minpd %xmm3, %xmm8 */
    "66440f5d1c24",  /* minpd (%rsp), %xmm11 */
    "f3410f5df8",  /* minss %xmm8, %xmm7 */
    "67f30f5d5110",  /* minss 16(%ecx), %xmm2 */
    "f20f5de0",  /* minsd %xmm0, %xmm4 */
    "f2460f5d5c5f04",  /* minsd 4(%rdi,%r11,2), %xmm11 */
    "0f5ed5",  /* divps %xmm5, %xmm2 */
    "430f5e04cc",  /* divps (%r12,%r9,8), %xmm0 */
    "66440f5ed3",  /* divpd %xmm3, %xmm10 */
    "66430f5e9c6ca0860100",  /* divpd 100000(%r12,%r13,2), %xmm3 */
    "f30f5ed1",  /* divss %xmm1, %xmm2 */
    "f3410f5e6980",  /* divss -128(%r9), %xmm5 */
    "f2410f5eee",  /* divsd %xmm14, %xmm5 */
    "f2410f5ea37fffffff",  /* divsd -129(%r11), %xmm4 */
    "0f5ffc",  /* maxps %xmm4, %xmm7 */
    "410f5fbc2fa0860100",  /* maxps 100000(%r15,%rbp,1), %xmm7 */
    "66450f5fd9",  /* maxpd %xmm9, %xmm11 */
    "660f5f8588a9cbed",  /* maxpd -305419896(%rbp), %xmm0 */
    "f3410f5fde",  /* maxss %xmm14, %xmm3 */
    "f3460f5f5473f8",  /* maxss -8(%rbx,%r14,2), %xmm10 */
    "f2440f5fee",  /* maxsd %xmm6, %xmm13 */
    "f2450f5f31",  /* maxsd (%r9), %xmm14 */
    "440f51f7",  /* sqrtps %xmm7, %xmm14 */
    "440f51447bf8",  /* sqrtps -8(%rbx,%rdi,2), %xmm8 */
    "660f51e3",  /* sqrtpd %xmm3, %xmm4 */
    "66410f515d00",  /* sqrtpd (%r13), %xmm3 */
    "f3440f51ef",  /* sqrtss %xmm7, %xmm13 */
    "f30f5104ca",  /* sqrtss (%rdx,%rcx,8), %xmm0 */
    "f20f51fd",  /* sqrtsd %xmm5, %xmm7 */
    "65f20f51142528000000",  /* sqrtsd %gs:0x28, %xmm2 */
    "440f28db",  /* movaps %xmm3, %xmm11 */
    "67410f284a10",  /* movaps 16(%r10d), %xmm1 */
    "410f291408",  /* movaps %xmm2, (%r8,%rcx,1) */
    "66450f28f9",  /* movapd %xmm9, %xmm15 */
    "66450f286500",  /* movapd (%r13), %xmm12 */
    "660f290c9500100000",  /* movapd %xmm1, 0x1000(,%rdx,4) */
    "0f10d6",  /* movups %xmm6, %xmm2 */
    "65440f10242500000000",  /* movups %gs:0x0, %xmm12 */
    "440f118634120000",  /* movups %xmm8, 4660(%rsi) */
    "66440f10e2",  /* movupd %xmm2, %xmm12 */
    "66420f103c9500100000",  /* movupd 0x1000(,%r10,4), %xmm7 */
    "66430f11b44400040000",  /* movupd %xmm6, 1024(%r12,%r8,2) */
    "f30f10c4",  /* movss %xmm4, %xmm0 */
    "f3460f10ac8da0860100",  /* movss 100000(%rbp,%r9,4), %xmm13 */
    "f3450f116500",  /* movss %xmm12, (%r13) */
    "f2410f10d1",  /* movsd %xmm9, %xmm2 */
    "67f2440f1022",  /* movsd 0(%edx), %xmm12 */
    "f2440f112c6d00000000",  /* movsd %xmm13, 0x0(,%rbp,2) */
    "66450f6ffe",  /* movdqa %xmm14, %xmm15 */
    "66460f6f3cd3",  /* movdqa (%rbx,%r10,8), %xmm15 */
    "66410f7f7500",  /* movdqa %xmm6, (%r13) */
    "f3450f6fc9",  /* movdqu %xmm9, %xmm9 */
    "f30f6f4c2480",  /* movdqu -128(%rsp), %xmm1 */
    "f30f7f4d00",  /* movdqu %xmm1, (%rbp) */
    "0f16242510000000",  /* movhps 0x10, %xmm4 */
    "0f17042510000000",  /* movhps %xmm0, 0x10 */
    "420f125cb5f8",  /* movlps -8(%rbp,%r14,4), %xmm3 */
    "65440f131c2500100000",  /* movlps %xmm11, %gs:0x1000 */
    "66450f1610",  /* movhpd (%r8), %xmm10 */
    "66430f1744a404",  /* movhpd %xmm0, 4(%r12,%r12,4) */
    "66440f1212",  /* movlpd (%rdx), %xmm10 */
    "66450f13bf80000000",  /* movlpd %xmm15, 128(%r15) */
    "440f12e0",  /* movhlps %xmm0, %xmm12 */
    "440f16c8",  /* movlhps %xmm0, %xmm9 */
    "410f14e6",  /* unpcklps %xmm14, %xmm4 */
    "0f141c2534120000",  /* unpcklps 0x1234, %xmm3 */
    "0f15d2",  /* unpckhps %xmm2, %xmm2 */
    "0f157f01",  /* unpckhps 1(%rdi), %xmm7 */
    "66450f14d2",  /* unpcklpd %xmm10, %xmm10 */
    "660f145500",  /* unpcklpd (%rbp), %xmm2 */
    "66410f15da",  /* unpckhpd %xmm10, %xmm3 */
    "66440f155500",  /* unpckhpd (%rbp), %xmm10 */
    "440f54dd",  /* andps %xmm5, %xmm11 */
    "64440f54342500000000",  /* andps %fs:0x0, %xmm14 */
    "66440f54d2",  /* andpd %xmm2, %xmm10 */
    "66410f54241b",  /* andpd (%r11,%rbx,1), %xmm4 */
    "450f55cf",  /* andnps %xmm15, %xmm9 */
    "67440f556310",  /* andnps 16(%ebx), %xmm12 */
    "660f55f2",  /* andnpd %xmm2, %xmm6 */
    "660f55ba88a9cbed",  /* andnpd -305419896(%rdx), %xmm7 */
    "450f56d0",  /* orps %xmm8, %xmm10 */
    "0f56745504",  /* orps 4(%rbp,%rdx,2), %xmm6 */
    "66450f56d2",  /* orpd %xmm10, %xmm10 */
    "66420f5644daf8",  /* orpd -8(%rdx,%r11,8), %xmm0 */
    "440f57c7",  /* xorps %xmm7, %xmm8 */
    "440f5714b510000000",  /* xorps 0x10(,%rsi,4), %xmm10 */
    "66450f57d1",  /* xorpd %xmm9, %xmm10 */
    "6766410f5732",  /* xorpd 0(%r10d), %xmm6 */
    "450f2ff8",  /* comiss %xmm8, %xmm15 */
    "410f2f5caef8",  /* comiss -8(%r14,%rbp,4), %xmm3 */
    "66410f2fdf",  /* comisd %xmm15, %xmm3 */
    "660f2f2424",  /* comisd (%rsp), %xmm4 */
    "440f2ed1",  /* ucomiss %xmm1, %xmm10 */
    "410f2e38",  /* ucomiss (%r8), %xmm7 */
    "66450f2ed9",  /* ucomisd %xmm9, %xmm11 */
    "66420f2e245510000000",  /* ucomisd 0x10(,%r10,2), %xmm4 */
    "450f5ada",  /* cvtps2pd %xmm10, %xmm11 */
    "67450f5a6a10",  /* cvtps2pd 16(%r10d), %xmm13 */
    "66440f5aea",  /* cvtpd2ps %xmm2, %xmm13 */
    "660f5a446b04",  /* cvtpd2ps 4(%rbx,%rbp,2), %xmm0 */
    "f3450f5af7",  /* cvtss2sd %xmm15, %xmm14 */
    "f3440f5a24ad00000000",  /* cvtss2sd 0x0(,%rbp,4), %xmm12 */
    "f20f5ac2",  /* cvtsd2ss %xmm2, %xmm0 */
    "f2410f5aa534120000",  /* cvtsd2ss 4660(%r13), %xmm4 */
    "440f5bf5",  /* cvtdq2ps %xmm5, %xmm14 */
    "450f5ba430a0860100",  /* cvtdq2ps 100000(%r8,%rsi,1), %xmm12 */
    "660f5bc7",  /* cvtps2dq %xmm7, %xmm0 */
    "66440f5b0425ffffff7f",  /* cvtps2dq 0x7fffffff, %xmm8 */
    "f3440f5bdd",  /* cvttps2dq %xmm5, %xmm11 */
    "f30f5b17",  /* cvttps2dq (%rdi), %xmm2 */
    "f30fe6e5",  /* cvtdq2pd %xmm5, %xmm4 */
    "f3440fe66cfa04",  /* cvtdq2pd 4(%rdx,%rdi,8), %xmm13 */
    "f2410fe6ff",  /* cvtpd2dq %xmm15, %xmm7 */
    "f2450fe68234120000",  /* cvtpd2dq 4660(%r10), %xmm8 */
    "66410fe6c3",  /* cvttpd2dq %xmm11, %xmm0 */
    "66420fe684cfa0860100",  /* cvttpd2dq 100000(%rdi,%r9,8), %xmm0 */
    "450f53f7",  /* rcpps %xmm15, %xmm14 */
    "640f53042528000000",  /* rcpps %fs:0x28, %xmm0 */
    "f3450f53e8",  /* rcpss %xmm8, %xmm13 */
    "f30f531c24",  /* rcpss (%rsp), %xmm3 */
    "440f52c0",  /* rsqrtps %xmm0, %xmm8 */
    "440f520ceb",  /* rsqrtps (%rbx,%rbp,8), %xmm9 */
    "f3450f52fe",  /* rsqrtss %xmm14, %xmm15 */
    "f3450f525801",  /* rsqrtss 1(%r8), %xmm11 */
    "f2450f12c5",  /* movddup %xmm13, %xmm8 */
    "f2410f1274d2f8",  /* movddup -8(%r10,%rdx,8), %xmm6 */
    "f3410f16d3",  /* movshdup %xmm11, %xmm2 */
    "f3440f1638",  /* movshdup (%rax), %xmm15 */
    "f3450f12cd",  /* movsldup %xmm13, %xmm9 */
    "67f3450f125810",  /* movsldup 16(%r8d), %xmm11 */
    "440fc2e91b",  /* cmpps $27, %xmm1, %xmm13 */
    "410fc2242403",  /* cmpps $3, (%r12), %xmm4 */
    "66410fc2d304",  /* cmppd $4, %xmm11, %xmm2 */
    "66470fc29c89a086010003",  /* cmppd $3, 100000(%r9,%r9,4), %xmm11 */
    "f30fc2c604",  /* cmpss $4, %xmm6, %xmm0 */
    "f3440fc2142403",  /* cmpss $3, (%rsp), %xmm10 */
    "f2410fc2dd04",  /* cmpsd $4, %xmm13, %xmm3 */
    "f20fc22c3303",  /* cmpsd $3, (%rbx,%rsi,1), %xmm5 */
    "0fc6f601",  /* shufps $1, %xmm6, %xmm6 */
    "450fc6550003",  /* shufps $3, (%r13), %xmm10 */
    "66450fc6db04",  /* shufpd $4, %xmm11, %xmm11 */
    "6766440fc64c241003",  /* shufpd $3, 16(%esp), %xmm9 */
    "66440f70ee04",  /* pshufd $4, %xmm6, %xmm13 */
    "67660f70531003",  /* pshufd $3, 16(%ebx), %xmm2 */
    "f3450f70f500",  /* pshufhw $0, %xmm13, %xmm14 */
    "f3440f70750003",  /* pshufhw $3, (%rbp), %xmm14 */
    "f2440f70c104",  /* pshuflw $4, %xmm1, %xmm8 */
    "f20f700c0303",  /* pshuflw $3, (%rbx,%rax,1), %xmm1 */
    "66450f3a08f304",  /* roundps $4, %xmm11, %xmm14 */
    "66450f3a08142403",  /* roundps $3, (%r12), %xmm10 */
    "660f3a09f000",  /* roundpd $0, %xmm0, %xmm6 */
    "660f3a093ced0010000003",  /* roundpd $3, 0x1000(,%rbp,8), %xmm7 */
    "66450f3a0adc04",  /* roundss $4, %xmm12, %xmm11 */
    "66430f3a0a745ff803",  /* roundss $3, -8(%r15,%r11,2), %xmm6 */
    "66440f3a0bc201",  /* roundsd $1, %xmm2, %xmm8 */
    "66450f3a0b7d0003",  /* roundsd $3, (%r13), %xmm15 */
    "66450f3a0ce500",  /* blendps $0, %xmm13, %xmm12 */
    "660f3a0c550003",  /* blendps $3, (%rbp), %xmm2 */
    "660f3a0dfa04",  /* blendpd $4, %xmm2, %xmm7 */
    "660f3a0d24253412000003",  /* blendpd $3, 0x1234, %xmm4 */
    "660f3a0ed91b",  /* pblendw $27, %xmm1, %xmm3 */
    "66450f3a0e550003",  /* pblendw $3, (%r13), %xmm10 */
    "66410f3a0fd900",  /* palignr $0, %xmm9, %xmm3 */
    "66450f3a0f1803",  /* palignr $3, (%r8), %xmm11 */
    "660f3a40df04",  /* dpps $4, %xmm7, %xmm3 */
    "66410f3a40142403",  /* dpps $3, (%r12), %xmm2 */
    "66410f3a44d01b",  /* pclmulqdq $27, %xmm8, %xmm2 */
    "660f3a4404251000000003",  /* pclmulqdq $3, 0x10, %xmm0 */
    "66450f3a21d204",  /* insertps $4, %xmm10, %xmm10 */
    "6466440f3a211c252800000003",  /* insertps $3, %fs:0x28, %xmm11 */
    "410fc2f800",  /* cmpeqps %xmm8, %xmm7 */
    "66410fc2de00",  /* cmpeqpd %xmm14, %xmm3 */
    "f3440fc2dc00",  /* cmpeqss %xmm4, %xmm11 */
    "f2440fc2df00",  /* cmpeqsd %xmm7, %xmm11 */
    "440fc2da01",  /* cmpltps %xmm2, %xmm11 */
    "66440fc2ce01",  /* cmpltpd %xmm6, %xmm9 */
    "f30fc2e601",  /* cmpltss %xmm6, %xmm4 */
    "f20fc2d001",  /* cmpltsd %xmm0, %xmm2 */
    "410fc2dc02",  /* cmpleps %xmm12, %xmm3 */
    "660fc2f102",  /* cmplepd %xmm1, %xmm6 */
    "f3450fc2ef02",  /* cmpless %xmm15, %xmm13 */
    "f2450fc2db02",  /* cmplesd %xmm11, %xmm11 */
    "440fc2e703",  /* cmpunordps %xmm7, %xmm12 */
    "66440fc2ea03",  /* cmpunordpd %xmm2, %xmm13 */
    "f3410fc2f403",  /* cmpunordss %xmm12, %xmm6 */
    "f2440fc2e603",  /* cmpunordsd %xmm6, %xmm12 */
    "410fc2f704",  /* cmpneqps %xmm15, %xmm6 */
    "66440fc2d804",  /* cmpneqpd %xmm0, %xmm11 */
    "f3410fc2cb04",  /* cmpneqss %xmm11, %xmm1 */
    "f2440fc2d104",  /* cmpneqsd %xmm1, %xmm10 */
    "410fc2da05",  /* cmpnltps %xmm10, %xmm3 */
    "66440fc2f605",  /* cmpnltpd %xmm6, %xmm14 */
    "f3440fc2c605",  /* cmpnltss %xmm6, %xmm8 */
    "f2450fc2da05",  /* cmpnltsd %xmm10, %xmm11 */
    "450fc2eb06",  /* cmpnleps %xmm11, %xmm13 */
    "66450fc2e506",  /* cmpnlepd %xmm13, %xmm12 */
    "f3440fc2e506",  /* cmpnless %xmm5, %xmm12 */
    "f2450fc2e006",  /* cmpnlesd %xmm8, %xmm12 */
    "450fc2cf07",  /* cmpordps %xmm15, %xmm9 */
    "660fc2d007",  /* cmpordpd %xmm0, %xmm2 */
    "f3410fc2de07",  /* cmpordss %xmm14, %xmm3 */
    "f20fc2db07",  /* cmpordsd %xmm3, %xmm3 */
    "66450f60fb",  /* punpcklbw %xmm11, %xmm15 */
    "66460f603c2d00000000",  /* punpcklbw 0x0(,%r13,1), %xmm15 */
    "66450f61cf",  /* punpcklwd %xmm15, %xmm9 */
    "66410f617d00",  /* punpcklwd (%r13), %xmm7 */
    "66450f62dd",  /* punpckldq %xmm13, %xmm11 */
    "66450f625d00",  /* punpckldq (%r13), %xmm11 */
    "660f63ee",  /* packsswb %xmm6, %xmm5 */
    "66420f63140d10000000",  /* packsswb 0x10(,%r9,1), %xmm2 */
    "66440f64e5",  /* pcmpgtb %xmm5, %xmm12 */
    "65660f640c2500000000",  /* pcmpgtb %gs:0x0, %xmm1 */
    "66410f65f8",  /* pcmpgtw %xmm8, %xmm7 */
    "67660f655310",  /* pcmpgtw 16(%ebx), %xmm2 */
    "66450f66d1",  /* pcmpgtd %xmm9, %xmm10 */
    "66470f6644f704",  /* pcmpgtd 4(%r15,%r14,8), %xmm8 */
    "660f67f2",  /* packuswb %xmm2, %xmm6 */
    "66440f676500",  /* packuswb (%rbp), %xmm12 */
    "660f68da",  /* punpckhbw %xmm2, %xmm3 */
    "66410f680424",  /* punpckhbw (%r12), %xmm0 */
    "66450f69f8",  /* punpckhwd %xmm8, %xmm15 */
    "66460f6994dc00040000",  /* punpckhwd 1024(%rsp,%r11,8), %xmm10 */
    "66450f6ac6",  /* punpckhdq %xmm14, %xmm8 */
    "66410f6a0c24",  /* punpckhdq (%r12), %xmm1 */
    "66450f6bf4",  /* packssdw %xmm12, %xmm14 */
    "67660f6b442410",  /* packssdw 16(%esp), %xmm0 */
    "66410f6cf0",  /* punpcklqdq %xmm8, %xmm6 */
    "66420f6c24ba",  /* punpcklqdq (%rdx,%r15,4), %xmm4 */
    "66410f6dff",  /* punpckhqdq %xmm15, %xmm7 */
    "66410f6d2c24",  /* punpckhqdq (%r12), %xmm5 */
    "66410f74e9",  /* pcmpeqb %xmm9, %xmm5 */
    "660f743424",  /* pcmpeqb (%rsp), %xmm6 */
    "66410f75c4",  /* pcmpeqw %xmm12, %xmm0 */
    "66440f757500",  /* pcmpeqw (%rbp), %xmm14 */
    "66450f76cc",  /* pcmpeqd %xmm12, %xmm9 */
    "6466440f761c2500100000",  /* pcmpeqd %fs:0x1000, %xmm11 */
    "660fd4fb",  /* paddq %xmm3, %xmm7 */
    "660fd472ff",  /* paddq -1(%rdx), %xmm6 */
    "66410fd5dd",  /* pmullw %xmm13, %xmm3 */
    "66460fd54c57f8",  /* pmullw -8(%rdi,%r10,2), %xmm9 */
    "660fd8ea",  /* psubusb %xmm2, %xmm5 */
    "660fd80c9510000000",  /* psubusb 0x10(,%rdx,4), %xmm1 */
    "660fd9f1",  /* psubusw %xmm1, %xmm6 */
    "6766450fd94a10",  /* psubusw 16(%r10d), %xmm9 */
    "66410fdacc",  /* pminub %xmm12, %xmm1 */
    "66450fda2424",  /* pminub (%r12), %xmm12 */
    "66410fdbd4",  /* pand %xmm12, %xmm2 */
    "66420fdb645d00",  /* pand (%rbp,%r11,2), %xmm4 */
    "66450fdcc3",  /* paddusb %xmm11, %xmm8 */
    "660fdcbb7fffffff",  /* paddusb -129(%rbx), %xmm7 */
    "66410fdddc",  /* paddusw %xmm12, %xmm3 */
    "66450fdd30",  /* paddusw (%r8), %xmm14 */
    "66410fdec9",  /* pmaxub %xmm9, %xmm1 */
    "66430fdeacd8a0860100",  /* pmaxub 100000(%r8,%r11,8), %xmm5 */
    "66410fdfea",  /* pandn %xmm10, %xmm5 */
    "6766440fdf6d10",  /* pandn 16(%ebp), %xmm13 */
    "66440fe0f3",  /* pavgb %xmm3, %xmm14 */
    "66460fe03c7500000000",  /* pavgb 0x0(,%r14,2), %xmm15 */
    "660fe3ce",  /* pavgw %xmm6, %xmm1 */
    "6766450fe35010",  /* pavgw 16(%r8d), %xmm10 */
    "66450fe4c9",  /* pmulhuw %xmm9, %xmm9 */
    "66440fe45ca8f8",  /* pmulhuw -8(%rax,%rbp,4), %xmm11 */
    "66450fe5e5",  /* pmulhw %xmm13, %xmm12 */
    "66440fe53424",  /* pmulhw (%rsp), %xmm14 */
    "66410fe8f4",  /* psubsb %xmm12, %xmm6 */
    "6466440fe80c2500100000",  /* psubsb %fs:0x1000, %xmm9 */
    "66440fe9c2",  /* psubsw %xmm2, %xmm8 */
    "66410fe97500",  /* psubsw (%r13), %xmm6 */
    "66440fead4",  /* pminsw %xmm4, %xmm10 */
    "66460fea4ca604",  /* pminsw 4(%rsi,%r12,4), %xmm9 */
    "660febc6",  /* por %xmm6, %xmm0 */
    "66440feb0c25ffffff7f",  /* por 0x7fffffff, %xmm9 */
    "66450fecd2",  /* paddsb %xmm10, %xmm10 */
    "66410fec3cb6",  /* paddsb (%r14,%rsi,4), %xmm7 */
    "66440fedc5",  /* paddsw %xmm5, %xmm8 */
    "66440fed5d7f",  /* paddsw 127(%rbp), %xmm11 */
    "66450feef5",  /* pmaxsw %xmm13, %xmm14 */
    "660fee0c2510000000",  /* pmaxsw 0x10, %xmm1 */
    "66450feffc",  /* pxor %xmm12, %xmm15 */
    "66420fef2c0d00100000",  /* pxor 0x1000(,%r9,1), %xmm5 */
    "66410ff4ce",  /* pmuludq %xmm14, %xmm1 */
    "660ff47d7f",  /* pmuludq 127(%rbp), %xmm7 */
    "66440ff5cc",  /* pmaddwd %xmm4, %xmm9 */
    "66470ff50c79",  /* pmaddwd (%r9,%r15,2), %xmm9 */
    "66450ff6db",  /* psadbw %xmm11, %xmm11 */
    "66450ff69280000000",  /* psadbw 128(%r10), %xmm10 */
    "66410ff8c8",  /* psubb %xmm8, %xmm1 */
    "66450ff87500",  /* psubb (%r13), %xmm14 */
    "66410ff9d9",  /* psubw %xmm9, %xmm3 */
    "66420ff92ce1",  /* psubw (%rcx,%r12,8), %xmm5 */
    "66450ffae7",  /* psubd %xmm15, %xmm12 */
    "66440ffa145d00000000",  /* psubd 0x0(,%rbx,2), %xmm10 */
    "66440ffbe2",  /* psubq %xmm2, %xmm12 */
    "660ffb342510000000",  /* psubq 0x10, %xmm6 */
    "66440ffcc9",  /* paddb %xmm1, %xmm9 */
    "66410ffc4d00",  /* paddb (%r13), %xmm1 */
    "660ffde7",  /* paddw %xmm7, %xmm4 */
    "66450ffd4d01",  /* paddw 1(%r13), %xmm9 */
    "66450ffec8",  /* paddd %xmm8, %xmm9 */
    "66430ffe3c1a",  /* paddd (%r10,%r11,1), %xmm7 */
    "660f3800fc",  /* pshufb %xmm4, %xmm7 */
    "66410f38006500",  /* pshufb (%r13), %xmm4 */
    "66450f3801f3",  /* phaddw %xmm11, %xmm14 */
    "66450f380175ff",  /* phaddw -1(%r13), %xmm14 */
    "660f3802ff",  /* phaddd %xmm7, %xmm7 */
    "66420f38022c0d00000000",  /* phaddd 0x0(,%r9,1), %xmm5 */
    "66410f3804d3",  /* pmaddubsw %xmm11, %xmm2 */
    "66410f38041a",  /* pmaddubsw (%r10), %xmm3 */
    "660f3808e5",  /* psignb %xmm5, %xmm4 */
    "66470f38084cf1f8",  /* psignb -8(%r9,%r14,8), %xmm9 */
    "660f380be6",  /* pmulhrsw %xmm6, %xmm4 */
    "6766440f380b2b",  /* pmulhrsw 0(%ebx), %xmm13 */
    "660f381cd7",  /* pabsb %xmm7, %xmm2 */
    "66440f381c7500",  /* pabsb (%rbp), %xmm14 */
    "660f381ddc",  /* pabsw %xmm4, %xmm3 */
    "660f381d0424",  /* pabsw (%rsp), %xmm0 */
    "660f381ef9",  /* pabsd %xmm1, %xmm7 */
    "65660f381e1c2500000000",  /* pabsd %gs:0x0, %xmm3 */
    "66450f3820f7",  /* pmovsxbw %xmm15, %xmm14 */
    "6766410f38200c24",  /* pmovsxbw 0(%r12d), %xmm1 */
    "66440f3831e5",  /* pmovzxbd %xmm5, %xmm12 */
    "66410f38316f01",  /* pmovzxbd 1(%r15), %xmm5 */
    "66450f3835c0",  /* pmovzxdq %xmm8, %xmm8 */
    "66470f383594a9a0860100",  /* pmovzxdq 100000(%r9,%r13,4), %xmm10 */
    "66410f3828c6",  /* pmuldq %xmm14, %xmm0 */
    "66440f38283425ffffff7f",  /* pmuldq 0x7fffffff, %xmm14 */
    "66450f3829cf",  /* pcmpeqq %xmm15, %xmm9 */
    "66450f38296b08",  /* pcmpeqq 8(%r11), %xmm13 */
    "660f382bee",  /* packusdw %xmm6, %xmm5 */
    "66440f382b5d00",  /* packusdw (%rbp), %xmm11 */
    "66450f3837ef",  /* pcmpgtq %xmm15, %xmm13 */
    "660f383730",  /* pcmpgtq (%rax), %xmm6 */
    "66450f3838e3",  /* pminsb %xmm11, %xmm12 */
    "66420f38382c48",  /* pminsb (%rax,%r9,2), %xmm5 */
    "660f3839de",  /* pminsd %xmm6, %xmm3 */
    "66470f38397c11f8",  /* pminsd -8(%r9,%r10,1), %xmm15 */
    "660f383adf",  /* pminuw %xmm7, %xmm3 */
    "66410f383a3c24",  /* pminuw (%r12), %xmm7 */
    "660f383bef",  /* pminud %xmm7, %xmm5 */
    "66430f383b048a",  /* pminud (%r10,%r9,4), %xmm0 */
    "660f383cda",  /* pmaxsb %xmm2, %xmm3 */
    "660f383c24bd00100000",  /* pmaxsb 0x1000(,%rdi,4), %xmm4 */
    "66450f383df0",  /* pmaxsd %xmm8, %xmm14 */
    "65660f383d3c2500000000",  /* pmaxsd %gs:0x0, %xmm7 */
    "66410f383ec5",  /* pmaxuw %xmm13, %xmm0 */
    "660f383e34d2",  /* pmaxuw (%rdx,%rdx,8), %xmm6 */
    "660f383fdb",  /* pmaxud %xmm3, %xmm3 */
    "66440f383f3c24",  /* pmaxud (%rsp), %xmm15 */
    "66440f3840e9",  /* pmulld %xmm1, %xmm13 */
    "660f38406d01",  /* pmulld 1(%rbp), %xmm5 */
    "66450f3817dd",  /* ptest %xmm13, %xmm11 */
    "660f381714fa",  /* ptest (%rdx,%rdi,8), %xmm2 */
    "66450f3841db",  /* phminposuw %xmm11, %xmm11 */
    "66450f3841acb2a0860100",  /* phminposuw 100000(%r10,%rsi,4), %xmm13 */
    "660f38dcc2",  /* aesenc %xmm2, %xmm0 */
    "66450f38dc74d500",  /* aesenc (%r13,%rdx,8), %xmm14 */
    "66440f38dde3",  /* aesenclast %xmm3, %xmm12 */
    "66470f38dd343b",  /* aesenclast (%r11,%r15,1), %xmm14 */
    "66450f38def3",  /* aesdec %xmm11, %xmm14 */
    "66440f38de0cad00100000",  /* aesdec 0x1000(,%rbp,4), %xmm9 */
    "66450f38dfe8",  /* aesdeclast %xmm8, %xmm13 */
    "64660f38df042528000000",  /* aesdeclast %fs:0x28, %xmm0 */
    "66440f38dbeb",  /* aesimc %xmm3, %xmm13 */
    "6566440f38db042500000000",  /* aesimc %gs:0x0, %xmm8 */
    "f2440f7cee",  /* haddps %xmm6, %xmm13 */
    "f2450f7c32",  /* haddps (%r10), %xmm14 */
    "66410f7dd8",  /* hsubpd %xmm8, %xmm3 */
    "67660f7d5810",  /* hsubpd 16(%eax), %xmm3 */
    "f2410fd0e3",  /* addsubps %xmm11, %xmm4 */
    "64f20fd0342500000000",  /* addsubps %fs:0x0, %xmm6 */
    "66410f71d401",  /* psrlw $1, %xmm12 */
    "660fd1d1",  /* psrlw %xmm1, %xmm2 */
    "6466440fd1042528000000",  /* psrlw %fs:0x28, %xmm8 */
    "66410f72d71f",  /* psrld $31, %xmm15 */
    "66410fd2d6",  /* psrld %xmm14, %xmm2 */
    "66470fd28c40a0860100",  /* psrld 100000(%r8,%r8,2), %xmm9 */
    "660f73d607",  /* psrlq $7, %xmm6 */
    "66440fd3e4",  /* psrlq %xmm4, %xmm12 */
    "66450fd34500",  /* psrlq (%r13), %xmm8 */
    "66410f71e607",  /* psraw $7, %xmm14 */
    "66410fe1de",  /* psraw %xmm14, %xmm3 */
    "66470fe1748500",  /* psraw (%r13,%r8,4), %xmm14 */
    "66410f72e31f",  /* psrad $31, %xmm11 */
    "66450fe2c3",  /* psrad %xmm11, %xmm8 */
    "67660fe236",  /* psrad 0(%esi), %xmm6 */
    "66410f71f71f",  /* psllw $31, %xmm15 */
    "66440ff1d8",  /* psllw %xmm0, %xmm11 */
    "66410ff13c24",  /* psllw (%r12), %xmm7 */
    "660f72f001",  /* pslld $1, %xmm0 */
    "66440ff2ea",  /* pslld %xmm2, %xmm13 */
    "6466440ff20c2528000000",  /* pslld %fs:0x28, %xmm9 */
    "66410f73f407",  /* psllq $7, %xmm12 */
    "660ff3ec",  /* psllq %xmm4, %xmm5 */
    "66410ff33c24",  /* psllq (%r12), %xmm7 */
    "f3440f2ae8",  /* cvtsi2ss %eax, %xmm13 */
    "f34d0f2aca",  /* cvtsi2ss %r10, %xmm9 */
    "f30f2a00",  /* cvtsi2ssl (%rax), %xmm0 */
    "f34c0f2a7b08",  /* cvtsi2ssq 8(%rbx), %xmm15 */
    "f2440f2af0",  /* cvtsi2sd %eax, %xmm14 */
    "f24d0f2aca",  /* cvtsi2sd %r10, %xmm9 */
    "f20f2a10",  /* cvtsi2sdl (%rax), %xmm2 */
    "f24c0f2a6308",  /* cvtsi2sdq 8(%rbx), %xmm12 */
    "f3410f2cc5",  /* cvttss2si %xmm13, %eax */
    "f34c0f2cdd",  /* cvttss2si %xmm5, %r11 */
    "f24d0f2cde",  /* cvttsd2si %xmm14, %r11 */
    "f30f2dc4",  /* cvtss2si %xmm4, %eax */
    "f34d0f2ddf",  /* cvtss2si %xmm15, %r11 */
    "f2410f2dc7",  /* cvtsd2si %xmm15, %eax */
    "f24d0f2dda",  /* cvtsd2si %xmm10, %r11 */
    "e2fe",  /* 1: loop 1b */
    "e916010000",  /* jmp 0f */
    "0f8110010000",  /* jno 4f */
    "e96bb8ffff",  /* jmp 4b */
    "0f8165b8ffff",  /* jno 0b */
    "488d055eb8ffff",  /* lea 3b(%rip), %rax */
    "0f8ef8000000",  /* jle 2f */
    "0f8852b8ffff",  /* js 2b */
    "0f834cb8ffff",  /* jae 3b */
    "0f8046b8ffff",  /* jo 0b */
    "0f86e0000000",  /* jbe 2f */
    "0f893ab8ffff",  /* jns 2b */
    "74bb",  /* je 1b */
    "0f8c32b8ffff",  /* jl 3b */
    "71b3",  /* jno 1b */
    "488d05c9000000",  /* lea 2f(%rip), %rax */
    "0f8223b8ffff",  /* jb 0b */
    "e9be000000",  /* jmp 4f */
    "e919b8ffff",  /* jmp 3b */
    "e914b8ffff",  /* jmp 2b */
    "0f8d0eb8ffff",  /* jge 0b */
    "e909b8ffff",  /* jmp 0b */
    "788a",  /* js 1b */
    "e9a2000000",  /* jmp 2f */
    "e99d000000",  /* jmp 3f */
    "488d05f6b7ffff",  /* lea 2b(%rip), %rax */
    "0f8f90000000",  /* jg 0f */
    "488d056cffffff",  /* lea 1b(%rip), %rax */
    "0f8e83000000",  /* jle 4f */
    "488d057c000000",  /* lea 3f(%rip), %rax */
    "0f80d6b7ffff",  /* jo 2b */
    "7e74",  /* jle 3f */
    "488d056d000000",  /* lea 4f(%rip), %rax */
    "488d05c6b7ffff",  /* lea 0b(%rip), %rax */
    "488d05bfb7ffff",  /* lea 4b(%rip), %rax */
    "765d",  /* jbe 1f */
    "795b",  /* jns 4f */
    "7159",  /* jno 3f */
    "488d0552000000",  /* lea 1f(%rip), %rax */
    "7750",  /* ja 3f */
    "704e",  /* jo 4f */
    "0f8fa8b7ffff",  /* jg 0b */
    "0f8fa2b7ffff",  /* jg 4b */
    "eb40",  /* jmp 1f */
    "e91effffff",  /* jmp 1b */
    "0f8895b7ffff",  /* js 3b */
    "0f8612ffffff",  /* jbe 1b */
    "712d",  /* jno 1f */
    "732b",  /* jae 2f */
    "488d0524000000",  /* lea 0f(%rip), %rax */
    "0f857eb7ffff",  /* jne 3b */
    "771c",  /* ja 2f */
    "0f8e76b7ffff",  /* jle 0b */
    "0f8ef3feffff",  /* jle 1b */
    "0f8b6ab7ffff",  /* jnp 3b */
    "0f8564b7ffff",  /* jne 4b */
    "7202",  /* jb 3f */
    "7900",  /* jns 3f */
};

static int hexval(char c) { return c <= '9' ? c - '0' : c - 'a' + 10; }

int main(void) {
    const unsigned char *p = axiom_asm_table;
    int bad = 0, n = (int)(sizeof want / sizeof want[0]);
    for (int i = 0; i < n; i++) {
        const char *w = want[i];
        int ok = 1;
        for (int k = 0; w[k] != 0; k += 2) {
            if (p >= axiom_asm_table_end || *p != hexval(w[k]) * 16 + hexval(w[k + 1])) ok = 0;
            p++;
        }
        if (!ok && bad++ < 5) {
            print_str("mismatch at line ", 17);
            print_i64(i);
        }
    }
    print_i64(bad);
    print_i64(p == axiom_asm_table_end);
    return 0;
}
#else
int main(void) {
    print_i64(0);
    print_i64(1);
    return 0;
}
#endif
/* expect: 0 */
/* expect: 1 */
