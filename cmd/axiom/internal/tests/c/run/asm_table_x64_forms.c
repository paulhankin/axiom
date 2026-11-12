/* flags: --c-std=gnu17 */
/* native-only */
/* Every form of the x86-64 assembler's instruction table (asm/x64tab.ax: x87, MMX, SSE, AVX, AVX2, FMA, F16C, AVX-512 F/VL/BW/DQ/CD/VNNI/IFMA/VBMI/BITALG/VPOPCNTDQ, GFNI, VAES, VPCLMULQDQ, SHA, gathers and scatters, the system instructions), twice with random operands, against a byte table checked against the system assembler in development. */
#include "host.h"

#ifdef __AXIOM_ASM_X86_64__
__asm__(".section .rodata.axiom_asm_table, \"a\"\n"
        ".globl axiom_asm_table\n"
        "axiom_asm_table:\n"
        "f2xm1\n"
        "fabs\n"
        "fchs\n"
        "fclex\n"
        "fnclex\n"
        "fcompp\n"
        "fcos\n"
        "fdecstp\n"
        "fincstp\n"
        "finit\n"
        "fninit\n"
        "fld1\n"
        "fldl2t\n"
        "fldl2e\n"
        "fldpi\n"
        "fldlg2\n"
        "fldln2\n"
        "fldz\n"
        "fnop\n"
        "fpatan\n"
        "fprem\n"
        "fprem1\n"
        "fptan\n"
        "frndint\n"
        "fscale\n"
        "fsin\n"
        "fsincos\n"
        "fsqrt\n"
        "ftst\n"
        "fucompp\n"
        "fxam\n"
        "fxtract\n"
        "fyl2x\n"
        "fyl2xp1\n"
        "faddp\n"
        "fmulp\n"
        "fsubp\n"
        "fsubrp\n"
        "fdivp\n"
        "fdivrp\n"
        "fcom\n"
        "fcomp\n"
        "fucom\n"
        "fucomp\n"
        "fxch\n"
        "fadds 0x1000(,%rbx,8)\n"
        "fadds 0x1000(,%r8,8)\n"
        "faddl 64(%rsi)\n"
        "faddl 0x12345678(%rax)\n"
        "fiadds (%rbp,%r12,8)\n"
        "fiadds 0x1000(,%rdx,2)\n"
        "fiaddl 128(%rcx)\n"
        "fiaddl 0x100(%rip)\n"
        "fiadd 0x10(,%r14,8)\n"
        "fiadd 0(%rip)\n"
        "fadd %st(6), %st\n"
        "fadd %st(7), %st\n"
        "fadd %st(6)\n"
        "fadd %st(3)\n"
        "fadd %st, %st(3)\n"
        "fadd %st, %st(7)\n"
        "faddp %st, %st(2)\n"
        "faddp %st, %st(7)\n"
        "faddp %st(7)\n"
        "faddp %st(5)\n"
        "fmuls (%r8,%r10,2)\n"
        "fmuls 128(%r13)\n"
        "fmull 4096(%r14)\n"
        "fmull -0x1000(%rsi,%rbx,4)\n"
        "fimuls 128(%rcx,%rcx,4)\n"
        "fimuls 0x10(,%rdi,2)\n"
        "fimull 0(%rip)\n"
        "fimull (%r10,%rbp,1)\n"
        "fimul 0x1000(,%r12,8)\n"
        "fimul 0x1000(,%r9,8)\n"
        "fmul %st(2), %st\n"
        "fmul %st(1), %st\n"
        "fmul %st(4)\n"
        "fmul %st(4)\n"
        "fmul %st, %st(0)\n"
        "fmul %st, %st(0)\n"
        "fmulp %st, %st(1)\n"
        "fmulp %st, %st(7)\n"
        "fmulp %st(7)\n"
        "fmulp %st(6)\n"
        "fsubs -128(%rcx,%rdx,4)\n"
        "fsubs 0x1000(,%r11,8)\n"
        "fsubl (%rbx)\n"
        "fsubl 0x12345678(%rcx,%r15,4)\n"
        "fisubs -128(%rdi)\n"
        "fisubs 4096(%r14,%r10,1)\n"
        "fisubl 256(%rbp,%rbx,2)\n"
        "fisubl 256(%r10)\n"
        "fisub 128(%rsi)\n"
        "fisub 0(%rip)\n"
        "fsub %st(1), %st\n"
        "fsub %st(6), %st\n"
        "fsub %st(7)\n"
        "fsub %st(2)\n"
        "fsub %st, %st(6)\n"
        "fsub %st, %st(5)\n"
        "fsubp %st, %st(0)\n"
        "fsubp %st, %st(5)\n"
        "fsubp %st(6)\n"
        "fsubp %st(5)\n"
        "fsubrs 0x1000(,%rdx,4)\n"
        "fsubrs 0(%rip)\n"
        "fsubrl 0(%rip)\n"
        "fsubrl -8(%rip)\n"
        "fisubrs 0x12345678(%rdi)\n"
        "fisubrs 0x10(,%r13,4)\n"
        "fisubrl 0x100(%rip)\n"
        "fisubrl -64(%r14)\n"
        "fisubr -64(%r9,%r11,2)\n"
        "fisubr 0(%rip)\n"
        "fsubr %st(5), %st\n"
        "fsubr %st(6), %st\n"
        "fsubr %st(3)\n"
        "fsubr %st(7)\n"
        "fsubr %st, %st(7)\n"
        "fsubr %st, %st(5)\n"
        "fsubrp %st, %st(0)\n"
        "fsubrp %st, %st(5)\n"
        "fsubrp %st(4)\n"
        "fsubrp %st(6)\n"
        "fdivs (%rdx,%r8,4)\n"
        "fdivs 0x10(,%rdi,8)\n"
        "fdivl 128(%r9,%r10,1)\n"
        "fdivl 0x100(%rip)\n"
        "fidivs 4096(%rcx)\n"
        "fidivs 0x10(,%rcx,8)\n"
        "fidivl 0x10(,%rbp,8)\n"
        "fidivl -8(%rip)\n"
        "fidiv 0x10(,%r10,4)\n"
        "fidiv 0x10(,%r15,8)\n"
        "fdiv %st(6), %st\n"
        "fdiv %st(4), %st\n"
        "fdiv %st(0)\n"
        "fdiv %st(0)\n"
        "fdiv %st, %st(4)\n"
        "fdiv %st, %st(2)\n"
        "fdivp %st, %st(4)\n"
        "fdivp %st, %st(7)\n"
        "fdivp %st(6)\n"
        "fdivp %st(1)\n"
        "fdivrs -8(%rip)\n"
        "fdivrs 0x100(%rip)\n"
        "fdivrl 0x1000(,%rbx,4)\n"
        "fdivrl 0x10(,%r12,8)\n"
        "fidivrs -8(%rip)\n"
        "fidivrs 0x1000(,%r14,8)\n"
        "fidivrl 0x12345678(%r10,%r11,4)\n"
        "fidivrl -8(%rip)\n"
        "fidivr 128(%r12)\n"
        "fidivr 0(%rip)\n"
        "fdivr %st(4), %st\n"
        "fdivr %st(7), %st\n"
        "fdivr %st(2)\n"
        "fdivr %st(4)\n"
        "fdivr %st, %st(1)\n"
        "fdivr %st, %st(0)\n"
        "fdivrp %st, %st(6)\n"
        "fdivrp %st, %st(3)\n"
        "fdivrp %st(1)\n"
        "fdivrp %st(7)\n"
        "fcoms 0x100(%rip)\n"
        "fcoms 0x1000(,%rbp,8)\n"
        "fcoml 64(%rsi)\n"
        "fcoml 0(%rip)\n"
        "ficoms 4096(%r14,%r10,4)\n"
        "ficoms 0x1000(,%r13,4)\n"
        "ficoml 0(%rip)\n"
        "ficoml 4096(%rbp)\n"
        "ficom -0x1000(%rax)\n"
        "ficom 0x1000(,%r12,8)\n"
        "fcom %st(2)\n"
        "fcom %st(6)\n"
        "fcomps -64(%r8,%rbp,2)\n"
        "fcomps 256(%r9,%r10,8)\n"
        "fcompl 8(%r8,%r10,1)\n"
        "fcompl 0x100(%rip)\n"
        "ficomps -64(%rbp)\n"
        "ficomps 0x12345678(%rcx)\n"
        "ficompl 0(%rip)\n"
        "ficompl -64(%r9)\n"
        "ficomp 64(%rbp)\n"
        "ficomp 0x1000(,%rsi,4)\n"
        "fcomp %st(3)\n"
        "fcomp %st(2)\n"
        "fucom %st(0)\n"
        "fucom %st(3)\n"
        "fucomp %st(4)\n"
        "fucomp %st(2)\n"
        "fcomi %st(3), %st\n"
        "fcomi %st(1), %st\n"
        "fcomi %st(1)\n"
        "fcomi %st(2)\n"
        "fcomip %st(6), %st\n"
        "fcomip %st(1), %st\n"
        "fcomip %st(1)\n"
        "fcomip %st(6)\n"
        "fcompi %st(2), %st\n"
        "fcompi %st(4), %st\n"
        "fcompi %st(3)\n"
        "fcompi %st(5)\n"
        "fucomi %st(2), %st\n"
        "fucomi %st(2), %st\n"
        "fucomi %st(6)\n"
        "fucomi %st(6)\n"
        "fucomip %st(0), %st\n"
        "fucomip %st(4), %st\n"
        "fucomip %st(4)\n"
        "fucomip %st(1)\n"
        "fucompi %st(5), %st\n"
        "fucompi %st(7), %st\n"
        "fucompi %st(6)\n"
        "fucompi %st(2)\n"
        "fcmovb %st(3), %st\n"
        "fcmovb %st(6), %st\n"
        "fcmove %st(2), %st\n"
        "fcmove %st(3), %st\n"
        "fcmovbe %st(6), %st\n"
        "fcmovbe %st(1), %st\n"
        "fcmovu %st(1), %st\n"
        "fcmovu %st(6), %st\n"
        "fcmovnb %st(4), %st\n"
        "fcmovnb %st(3), %st\n"
        "fcmovae %st(3), %st\n"
        "fcmovae %st(6), %st\n"
        "fcmovne %st(7), %st\n"
        "fcmovne %st(0), %st\n"
        "fcmovnbe %st(3), %st\n"
        "fcmovnbe %st(4), %st\n"
        "fcmova %st(6), %st\n"
        "fcmova %st(2), %st\n"
        "fcmovnu %st(4), %st\n"
        "fcmovnu %st(5), %st\n"
        "fcmovnae %st(0), %st\n"
        "fcmovnae %st(6), %st\n"
        "fcmovna %st(5), %st\n"
        "fcmovna %st(7), %st\n"
        "fld %st(5)\n"
        "fld %st(2)\n"
        "fst %st(4)\n"
        "fst %st(4)\n"
        "fstp %st(4)\n"
        "fstp %st(6)\n"
        "fxch %st(3)\n"
        "fxch %st(6)\n"
        "ffree %st(2)\n"
        "ffree %st(3)\n"
        "ffreep %st(4)\n"
        "ffreep %st(1)\n"
        "flds 0x10(,%rax,8)\n"
        "flds 0x10(,%rax,4)\n"
        "fldl (%r8,%r13,1)\n"
        "fldl 0x1000(,%r8,2)\n"
        "fldt -0x1000(%rcx,%rbp,2)\n"
        "fldt 0x100(%rip)\n"
        "filds 8(%r14)\n"
        "filds 0(%rip)\n"
        "fildl 0x10(,%r12,2)\n"
        "fildl -0x1000(%rbp)\n"
        "fild 0x100(%rip)\n"
        "fild 0x10(,%rdi,4)\n"
        "fildll 0x10(,%r15,2)\n"
        "fildll -8(%rip)\n"
        "fildq -64(%rdi,%rbx,4)\n"
        "fildq (%r11,%r9,1)\n"
        "fsts 0x1000(,%r10,2)\n"
        "fsts 64(%r14)\n"
        "fstl -8(%rip)\n"
        "fstl -128(%rbx,%rdi,1)\n"
        "fstps 0(%rip)\n"
        "fstps -8(%rip)\n"
        "fstpl 0x1000(,%rbx,8)\n"
        "fstpl 128(%r11)\n"
        "fstpt 0x100(%rip)\n"
        "fstpt 0x1000(,%rbp,2)\n"
        "fists 0x12345678(%rax)\n"
        "fists -8(%rip)\n"
        "fistl 0x100(%rip)\n"
        "fistl -128(%r12)\n"
        "fist 4096(%r10,%rsi,2)\n"
        "fist 0x1000(,%r15,4)\n"
        "fistps 0x1000(,%r14,2)\n"
        "fistps -0x1000(%rsi)\n"
        "fistpl 0x12345678(%rbp)\n"
        "fistpl 0x1000(,%rbp,4)\n"
        "fistp (%r10)\n"
        "fistp (%r11)\n"
        "fistpll -8(%rip)\n"
        "fistpll 64(%r14,%r12,1)\n"
        "fistpq 0x100(%rip)\n"
        "fistpq 0x1000(,%r11,4)\n"
        "fisttps 0x10(,%r14,8)\n"
        "fisttps 8(%rdi,%r11,8)\n"
        "fisttpl 4096(%r10,%rax,8)\n"
        "fisttpl 8(%r12,%rdx,4)\n"
        "fisttp 0x1000(,%r11,4)\n"
        "fisttp 0x12345678(%r8)\n"
        "fisttpll -64(%r14)\n"
        "fisttpll (%rbx)\n"
        "fisttpq 0(%rip)\n"
        "fisttpq 0x10(,%rax,8)\n"
        "fbld 0x10(,%r13,8)\n"
        "fbld 8(%r11,%rdx,2)\n"
        "fbstp 256(%r9,%rdx,1)\n"
        "fbstp 4096(%rdx)\n"
        "fldcw 0x10(,%r9,8)\n"
        "fldcw 0x1000(,%r14,8)\n"
        "fnstcw -8(%rip)\n"
        "fnstcw 0x10(,%rcx,4)\n"
        "fldenv 0(%rip)\n"
        "fldenv 0x100(%rip)\n"
        "fnstenv 0x10(,%rsi,2)\n"
        "fnstenv 8(%rdx,%rdx,8)\n"
        "fnsave 256(%r9,%r8,1)\n"
        "fnsave (%r13)\n"
        "frstor -64(%rbp,%r11,1)\n"
        "frstor 0x1000(,%rsi,8)\n"
        "fnstsw 0x100(%rip)\n"
        "fnstsw 128(%r9,%r14,8)\n"
        "fldenvl -0x1000(%r13)\n"
        "fldenvl (%rsi)\n"
        "fnstenvl 8(%r13)\n"
        "fnstenvl 0x10(,%rdx,8)\n"
        "fnsavel 0x12345678(%r11,%r15,4)\n"
        "fnsavel 0x1000(,%rdx,8)\n"
        "frstorl -64(%rax,%r11,8)\n"
        "frstorl 4096(%rbp)\n"
        "fstcw 4096(%r9)\n"
        "fstcw 0(%rip)\n"
        "fstenv 0x1000(,%r12,2)\n"
        "fstenv 0(%rip)\n"
        "fsave -0x1000(%r11)\n"
        "fsave -0x1000(%r8,%rax,2)\n"
        "fstsw 0x12345678(%rax)\n"
        "fstsw 0x100(%rip)\n"
        "fstenvl 0x10(,%r15,4)\n"
        "fstenvl 0(%rip)\n"
        "fsavel (%rbp)\n"
        "fsavel 0x100(%rip)\n"
        "fnstsw %ax\n"
        "fnstsw %ax\n"
        "fstsw %ax\n"
        "fstsw %ax\n"
        "fxsave -64(%rdi,%rdx,1)\n"
        "fxsave 0x100(%rip)\n"
        "fxrstor 8(%rax,%r12,4)\n"
        "fxrstor -0x1000(%r10)\n"
        "fxsave64 0x10(,%r15,2)\n"
        "fxsave64 -8(%rip)\n"
        "fxrstor64 128(%r12,%r9,1)\n"
        "fxrstor64 0x12345678(%r15,%rsi,1)\n"
        "xsave (%r13)\n"
        "xsave 0x12345678(%r12)\n"
        "xrstor 64(%r15)\n"
        "xrstor 4096(%r12,%rsi,1)\n"
        "xsave64 0(%rip)\n"
        "xsave64 -8(%rip)\n"
        "xrstor64 8(%rdx,%r13,1)\n"
        "xrstor64 0x1000(,%rsi,4)\n"
        "xsaveopt -64(%rdx,%r10,4)\n"
        "xsaveopt 4096(%rdi)\n"
        "xsaveopt64 0x100(%rip)\n"
        "xsaveopt64 0x1000(,%r12,8)\n"
        "xsavec -8(%rip)\n"
        "xsavec (%rsi)\n"
        "xsavec64 0x10(,%rdi,8)\n"
        "xsavec64 0x1000(,%r11,2)\n"
        "xsaves -8(%rip)\n"
        "xsaves 0x100(%rip)\n"
        "xsaves64 -128(%r12)\n"
        "xsaves64 0x1000(,%r12,2)\n"
        "xrstors 0x100(%rip)\n"
        "xrstors 0x10(,%r15,2)\n"
        "xrstors64 -0x1000(%r10,%rax,2)\n"
        "xrstors64 -0x1000(%rbp,%rdx,4)\n"
        "punpcklbw -64(%r8), %mm4\n"
        "punpcklbw -8(%rip), %mm5\n"
        "punpcklwd 4096(%r13), %mm0\n"
        "punpcklwd (%r13), %mm6\n"
        "punpckldq %mm1, %mm4\n"
        "punpckldq (%r12), %mm0\n"
        "packsswb (%rdi), %mm0\n"
        "packsswb 0(%rip), %mm3\n"
        "pcmpgtb %mm3, %mm6\n"
        "pcmpgtb (%rbp,%rbx,1), %mm5\n"
        "pcmpgtw %mm4, %mm3\n"
        "pcmpgtw %mm1, %mm5\n"
        "pcmpgtd %mm4, %mm3\n"
        "pcmpgtd 64(%rdx,%r13,1), %mm1\n"
        "packuswb %mm1, %mm0\n"
        "packuswb 0(%rip), %mm4\n"
        "punpckhbw 0x1000(,%r14,2), %mm5\n"
        "punpckhbw -0x1000(%r15), %mm7\n"
        "punpckhwd 0x100(%rip), %mm6\n"
        "punpckhwd 0x12345678(%rbp), %mm5\n"
        "punpckhdq %mm4, %mm4\n"
        "punpckhdq -64(%r11,%r11,2), %mm2\n"
        "packssdw 0x100(%rip), %mm3\n"
        "packssdw %mm5, %mm6\n"
        "pcmpeqb 0x10(,%r11,2), %mm2\n"
        "pcmpeqb 0x100(%rip), %mm5\n"
        "pcmpeqw %mm4, %mm6\n"
        "pcmpeqw 256(%rdi,%r10,4), %mm3\n"
        "pcmpeqd %mm3, %mm4\n"
        "pcmpeqd 128(%r9,%r15,2), %mm3\n"
        "psrlw 0x10(,%r14,8), %mm4\n"
        "psrlw %mm4, %mm6\n"
        "psrld %mm3, %mm2\n"
        "psrld -128(%rcx), %mm7\n"
        "psrlq 0(%rip), %mm1\n"
        "psrlq 0x100(%rip), %mm5\n"
        "paddq %mm5, %mm7\n"
        "paddq 0(%rip), %mm1\n"
        "pmullw 0x100(%rip), %mm2\n"
        "pmullw 128(%rcx), %mm0\n"
        "psubusb %mm4, %mm6\n"
        "psubusb -128(%r11,%rcx,4), %mm4\n"
        "psubusw %mm6, %mm5\n"
        "psubusw %mm0, %mm7\n"
        "pminub %mm7, %mm1\n"
        "pminub 0x10(,%rax,8), %mm2\n"
        "pand 0(%rip), %mm5\n"
        "pand %mm2, %mm3\n"
        "paddusb 0x1000(,%rcx,2), %mm5\n"
        "paddusb %mm3, %mm7\n"
        "paddusw 0(%rip), %mm1\n"
        "paddusw 0(%rip), %mm3\n"
        "pmaxub %mm0, %mm4\n"
        "pmaxub 0x12345678(%r12,%r11,8), %mm6\n"
        "pandn %mm0, %mm5\n"
        "pandn 256(%rcx,%r8,8), %mm1\n"
        "pavgb 0(%rip), %mm3\n"
        "pavgb %mm1, %mm0\n"
        "psraw 0x10(,%r15,8), %mm5\n"
        "psraw 8(%rdi,%r9,8), %mm6\n"
        "psrad -64(%rcx,%rdi,2), %mm2\n"
        "psrad %mm1, %mm2\n"
        "pavgw %mm5, %mm7\n"
        "pavgw %mm6, %mm6\n"
        "pmulhuw 0x100(%rip), %mm4\n"
        "pmulhuw %mm7, %mm7\n"
        "pmulhw %mm5, %mm1\n"
        "pmulhw 0x100(%rip), %mm5\n"
        "psubsb %mm3, %mm2\n"
        "psubsb %mm5, %mm5\n"
        "psubsw 0x10(,%r10,4), %mm5\n"
        "psubsw -64(%r11), %mm6\n"
        "pminsw %mm1, %mm3\n"
        "pminsw 0(%rip), %mm3\n"
        "por %mm5, %mm1\n"
        "por 0(%rip), %mm6\n"
        "paddsb %mm2, %mm2\n"
        "paddsb -8(%rip), %mm2\n"
        "paddsw %mm3, %mm5\n"
        "paddsw %mm1, %mm7\n"
        "pmaxsw %mm5, %mm2\n"
        "pmaxsw %mm6, %mm5\n"
        "pxor %mm3, %mm7\n"
        "pxor %mm4, %mm4\n"
        "psllw %mm6, %mm1\n"
        "psllw 0x1000(,%rsi,8), %mm7\n"
        "pslld 64(%rdi,%r9,1), %mm6\n"
        "pslld %mm3, %mm3\n"
        "psllq %mm0, %mm7\n"
        "psllq 0x10(,%r13,2), %mm0\n"
        "pmuludq %mm7, %mm5\n"
        "pmuludq 64(%rbx), %mm0\n"
        "pmaddwd 128(%rcx), %mm0\n"
        "pmaddwd %mm6, %mm7\n"
        "psadbw %mm6, %mm1\n"
        "psadbw 0x10(,%r12,2), %mm6\n"
        "psubb %mm5, %mm1\n"
        "psubb %mm0, %mm1\n"
        "psubw 0x1000(,%r10,4), %mm1\n"
        "psubw 0x100(%rip), %mm4\n"
        "psubd %mm7, %mm4\n"
        "psubd 0x1000(,%r11,4), %mm2\n"
        "psubq %mm2, %mm0\n"
        "psubq 0(%rip), %mm5\n"
        "paddb %mm7, %mm7\n"
        "paddb %mm0, %mm5\n"
        "paddw 128(%rsi), %mm1\n"
        "paddw 0x10(,%r10,8), %mm1\n"
        "paddd %mm2, %mm0\n"
        "paddd %mm4, %mm0\n"
        "pshufb %mm3, %mm2\n"
        "pshufb 128(%r10,%rcx,1), %mm6\n"
        "phaddw 0x1000(,%rsi,2), %mm0\n"
        "phaddw -8(%rip), %mm0\n"
        "phaddd %mm0, %mm3\n"
        "phaddd 0(%rip), %mm7\n"
        "phaddsw %mm7, %mm7\n"
        "phaddsw %mm2, %mm4\n"
        "pmaddubsw (%r9,%r14,2), %mm6\n"
        "pmaddubsw -128(%rdx), %mm4\n"
        "phsubw %mm2, %mm7\n"
        "phsubw 0x10(,%rbp,4), %mm6\n"
        "phsubd 8(%r10,%rbp,4), %mm4\n"
        "phsubd %mm5, %mm4\n"
        "phsubsw %mm5, %mm0\n"
        "phsubsw 0x1000(,%r13,2), %mm1\n"
        "psignb 64(%r13), %mm6\n"
        "psignb %mm3, %mm6\n"
        "psignw -128(%r10), %mm1\n"
        "psignw 0x1000(,%rsi,4), %mm0\n"
        "psignd 64(%r10,%rdx,4), %mm3\n"
        "psignd %mm7, %mm4\n"
        "pmulhrsw %mm2, %mm2\n"
        "pmulhrsw %mm2, %mm6\n"
        "pabsb 64(%r15,%r12,1), %mm5\n"
        "pabsb %mm4, %mm6\n"
        "pabsw 8(%rbx,%rbx,1), %mm7\n"
        "pabsw %mm7, %mm0\n"
        "pabsd %mm3, %mm3\n"
        "pabsd 0x100(%rip), %mm4\n"
        "palignr $58, 0x100(%rip), %mm0\n"
        "palignr $13, %mm3, %mm3\n"
        "psrlw $88, %mm3\n"
        "psrlw $8, %mm2\n"
        "psraw $99, %mm1\n"
        "psraw $132, %mm4\n"
        "psllw $45, %mm7\n"
        "psllw $106, %mm6\n"
        "psrld $112, %mm5\n"
        "psrld $243, %mm0\n"
        "psrad $199, %mm2\n"
        "psrad $155, %mm3\n"
        "pslld $97, %mm0\n"
        "pslld $245, %mm1\n"
        "psrlq $7, %mm6\n"
        "psrlq $243, %mm0\n"
        "psllq $207, %mm7\n"
        "psllq $243, %mm2\n"
        "pshufw $206, %mm7, %mm6\n"
        "pshufw $13, 8(%r15,%r10,4), %mm3\n"
        "pinsrw $171, 0x1000(,%rbx,8), %mm0\n"
        "pinsrw $220, %r11d, %mm5\n"
        "pextrw $14, %mm7, %r12\n"
        "pextrw $166, %mm0, %esi\n"
        "pmovmskb %mm2, %esi\n"
        "pmovmskb %mm7, %rbp\n"
        "movntq %mm4, (%r10)\n"
        "movntq %mm5, 0x1000(,%rcx,2)\n"
        "maskmovq %mm0, %mm5\n"
        "maskmovq %mm2, %mm6\n"
        "movd %ecx, %mm6\n"
        "movd %eax, %mm7\n"
        "movd %mm3, %esi\n"
        "movd %mm0, 0x1000(,%rbp,4)\n"
        "movq %mm6, %mm6\n"
        "movq -8(%rip), %mm4\n"
        "movq %mm4, 128(%r9,%rbp,4)\n"
        "movq %mm7, 256(%rsi,%r12,4)\n"
        "movq %rdx, %mm7\n"
        "movq %rcx, %mm6\n"
        "movq %mm0, %r8\n"
        "movq %mm3, %rax\n"
        "movd %r15, %mm1\n"
        "movd %rbx, %mm6\n"
        "movd %mm6, %r9\n"
        "movd %mm2, %r10\n"
        "cvtpi2ps -8(%rip), %xmm10\n"
        "cvtpi2ps %mm7, %xmm3\n"
        "cvtps2pi %xmm9, %mm2\n"
        "cvtps2pi (%r8), %mm6\n"
        "cvttps2pi %xmm6, %mm3\n"
        "cvttps2pi 0x1000(,%r14,2), %mm4\n"
        "cvtpi2pd %mm1, %xmm15\n"
        "cvtpi2pd %mm4, %xmm0\n"
        "cvtpd2pi 0x10(,%r14,8), %mm6\n"
        "cvtpd2pi %xmm10, %mm3\n"
        "cvttpd2pi %xmm9, %mm6\n"
        "cvttpd2pi -8(%rip), %mm7\n"
        "movdq2q %xmm13, %mm7\n"
        "movdq2q %xmm13, %mm0\n"
        "movq2dq %mm2, %xmm12\n"
        "movq2dq %mm4, %xmm5\n"
        "femms\n"
        "paddb -128(%r15,%rcx,8), %xmm12\n"
        "paddb 4096(%rsi), %xmm2\n"
        "vpaddb %ymm8, %ymm8, %ymm8\n"
        "vpaddb 64(%rdx), %xmm4, %xmm3\n"
        "vpaddb %xmm2, %xmm2, %xmm6{%k2}\n"
        "vpaddb -8(%rip), %xmm10, %xmm24{%k5}\n"
        "paddw %xmm0, %xmm2\n"
        "paddw %xmm0, %xmm14\n"
        "vpaddw -8(%rip), %xmm14, %xmm9\n"
        "vpaddw 0(%rip), %xmm3, %xmm11\n"
        "vpaddw %ymm0, %ymm9, %ymm13{%k7}{z}\n"
        "vpaddw -8(%rip), %zmm27, %zmm26\n"
        "paddsb %xmm12, %xmm7\n"
        "paddsb 0x1000(,%r9,2), %xmm0\n"
        "vpaddsb 4096(%rcx,%rbx,2), %ymm13, %ymm7\n"
        "vpaddsb -64(%r10), %xmm11, %xmm11\n"
        "vpaddsb %xmm11, %xmm21, %xmm30\n"
        "vpaddsb 64(%r13), %ymm23, %ymm4\n"
        "paddsw 0x1000(,%r10,4), %xmm10\n"
        "paddsw 128(%r15), %xmm7\n"
        "vpaddsw -0x1000(%rbp), %xmm14, %xmm5\n"
        "vpaddsw 256(%r8,%rbx,8), %xmm15, %xmm4\n"
        "vpaddsw %xmm16, %xmm18, %xmm2\n"
        "vpaddsw 0(%rip), %zmm27, %zmm30{%k7}\n"
        "paddusb %xmm9, %xmm8\n"
        "paddusb 0x10(,%r15,4), %xmm4\n"
        "vpaddusb 0(%rip), %xmm1, %xmm6\n"
        "vpaddusb %xmm2, %xmm13, %xmm7\n"
        "vpaddusb %xmm17, %xmm1, %xmm1\n"
        "vpaddusb %zmm2, %zmm10, %zmm27{%k7}\n"
        "paddusw 0x100(%rip), %xmm9\n"
        "paddusw 256(%r13,%r11,2), %xmm2\n"
        "vpaddusw 128(%r13), %ymm6, %ymm13\n"
        "vpaddusw 128(%rsi), %ymm14, %ymm5\n"
        "vpaddusw %ymm25, %ymm8, %ymm16\n"
        "vpaddusw %zmm16, %zmm21, %zmm0{%k6}{z}\n"
        "psubb %xmm12, %xmm0\n"
        "psubb %xmm1, %xmm13\n"
        "vpsubb %ymm14, %ymm7, %ymm6\n"
        "vpsubb %ymm9, %ymm12, %ymm6\n"
        "vpsubb 64(%r9,%r11,1), %zmm23, %zmm9{%k3}{z}\n"
        "vpsubb 0x100(%rip), %ymm14, %ymm10{%k1}\n"
        "psubw -128(%rbp,%r13,4), %xmm9\n"
        "psubw %xmm11, %xmm15\n"
        "vpsubw %xmm15, %xmm8, %xmm1\n"
        "vpsubw 0x1000(,%rax,4), %xmm8, %xmm9\n"
        "vpsubw 8(%rsi), %ymm27, %ymm31\n"
        "vpsubw %xmm24, %xmm24, %xmm23{%k5}{z}\n"
        "psubsb %xmm4, %xmm12\n"
        "psubsb 0x1000(,%r8,8), %xmm11\n"
        "vpsubsb %xmm8, %xmm4, %xmm6\n"
        "vpsubsb %xmm9, %xmm6, %xmm11\n"
        "vpsubsb -64(%rsi), %zmm10, %zmm8\n"
        "vpsubsb %xmm4, %xmm31, %xmm13\n"
        "psubsw %xmm7, %xmm2\n"
        "psubsw %xmm11, %xmm8\n"
        "vpsubsw -0x1000(%r13,%r9,2), %xmm15, %xmm8\n"
        "vpsubsw -8(%rip), %xmm7, %xmm13\n"
        "vpsubsw 0x100(%rip), %zmm23, %zmm3{%k2}\n"
        "vpsubsw 8(%rbx,%r14,8), %zmm14, %zmm13\n"
        "psubusb -8(%rip), %xmm1\n"
        "psubusb %xmm14, %xmm13\n"
        "vpsubusb %ymm12, %ymm5, %ymm7\n"
        "vpsubusb 0x1000(,%r15,2), %xmm11, %xmm15\n"
        "vpsubusb %zmm8, %zmm20, %zmm2{%k6}{z}\n"
        "vpsubusb %xmm4, %xmm5, %xmm16\n"
        "psubusw %xmm1, %xmm11\n"
        "psubusw %xmm5, %xmm15\n"
        "vpsubusw %xmm6, %xmm15, %xmm10\n"
        "vpsubusw %ymm5, %ymm10, %ymm0\n"
        "vpsubusw %ymm20, %ymm2, %ymm26\n"
        "vpsubusw 0x1000(,%r13,2), %zmm25, %zmm28{%k5}{z}\n"
        "pcmpeqb -8(%rip), %xmm14\n"
        "pcmpeqb %xmm1, %xmm10\n"
        "vpcmpeqb %xmm8, %xmm10, %xmm5\n"
        "vpcmpeqb 4096(%rdi), %ymm4, %ymm7\n"
        "pcmpeqw 0x1000(,%rdx,2), %xmm9\n"
        "pcmpeqw %xmm11, %xmm15\n"
        "vpcmpeqw %xmm10, %xmm12, %xmm4\n"
        "vpcmpeqw %xmm8, %xmm4, %xmm4\n"
        "pcmpgtb %xmm0, %xmm0\n"
        "pcmpgtb 0x1000(,%r11,4), %xmm13\n"
        "vpcmpgtb 64(%rax), %ymm5, %ymm9\n"
        "vpcmpgtb %ymm7, %ymm14, %ymm0\n"
        "pcmpgtw %xmm8, %xmm12\n"
        "pcmpgtw %xmm10, %xmm5\n"
        "vpcmpgtw 0x10(,%r9,4), %ymm6, %ymm3\n"
        "vpcmpgtw %ymm15, %ymm11, %ymm5\n"
        "pmaxsw %xmm2, %xmm13\n"
        "pmaxsw 0x100(%rip), %xmm10\n"
        "vpmaxsw 0(%rip), %xmm15, %xmm13\n"
        "vpmaxsw -0x1000(%r12), %xmm6, %xmm13\n"
        "vpmaxsw %xmm11, %xmm17, %xmm3\n"
        "vpmaxsw %zmm9, %zmm27, %zmm15{%k3}\n"
        "pmaxub %xmm7, %xmm13\n"
        "pmaxub -64(%r9), %xmm2\n"
        "vpmaxub %xmm13, %xmm7, %xmm0\n"
        "vpmaxub %ymm4, %ymm7, %ymm7\n"
        "vpmaxub %zmm23, %zmm17, %zmm1{%k6}{z}\n"
        "vpmaxub 0x100(%rip), %ymm3, %ymm22\n"
        "pminsw 8(%rsi), %xmm12\n"
        "pminsw %xmm3, %xmm2\n"
        "vpminsw 0x100(%rip), %xmm6, %xmm15\n"
        "vpminsw %ymm2, %ymm2, %ymm14\n"
        "vpminsw %ymm25, %ymm26, %ymm13{%k7}\n"
        "vpminsw -128(%r8), %xmm18, %xmm31\n"
        "pminub (%rdx,%rbp,8), %xmm14\n"
        "pminub %xmm5, %xmm4\n"
        "vpminub 0x1000(,%rax,4), %xmm9, %xmm14\n"
        "vpminub 0x1000(,%rsi,4), %ymm14, %ymm8\n"
        "vpminub 0(%rip), %ymm28, %ymm7{%k3}\n"
        "vpminub %xmm8, %xmm25, %xmm1{%k2}{z}\n"
        "pavgb 128(%rsi,%rax,1), %xmm9\n"
        "pavgb -64(%r9), %xmm9\n"
        "vpavgb 128(%r11), %xmm1, %xmm13\n"
        "vpavgb %ymm8, %ymm9, %ymm1\n"
        "vpavgb %zmm2, %zmm21, %zmm10{%k2}{z}\n"
        "vpavgb -0x1000(%rax), %ymm1, %ymm10{%k7}{z}\n"
        "pavgw %xmm0, %xmm3\n"
        "pavgw %xmm11, %xmm15\n"
        "vpavgw %xmm4, %xmm6, %xmm15\n"
        "vpavgw 0(%rip), %ymm3, %ymm8\n"
        "vpavgw %xmm29, %xmm12, %xmm9{%k5}\n"
        "vpavgw %zmm21, %zmm10, %zmm13\n"
        "pmullw 0x1000(,%r8,4), %xmm2\n"
        "pmullw %xmm5, %xmm4\n"
        "vpmullw %ymm14, %ymm4, %ymm4\n"
        "vpmullw 0x1000(,%r11,4), %xmm12, %xmm15\n"
        "vpmullw 0x10(,%r13,2), %ymm2, %ymm12\n"
        "vpmullw 0x1000(,%r13,2), %ymm9, %ymm22\n"
        "pmulhw %xmm0, %xmm2\n"
        "pmulhw %xmm8, %xmm2\n"
        "vpmulhw 0x10(,%r14,4), %xmm8, %xmm13\n"
        "vpmulhw %xmm1, %xmm3, %xmm14\n"
        "vpmulhw %xmm29, %xmm4, %xmm13\n"
        "vpmulhw %zmm28, %zmm19, %zmm20{%k3}{z}\n"
        "pmulhuw %xmm8, %xmm0\n"
        "pmulhuw 8(%r8,%rdx,4), %xmm8\n"
        "vpmulhuw (%r15,%rbx,4), %xmm7, %xmm5\n"
        "vpmulhuw %xmm12, %xmm9, %xmm3\n"
        "vpmulhuw %ymm19, %ymm19, %ymm19\n"
        "vpmulhuw -8(%rip), %xmm1, %xmm25{%k3}{z}\n"
        "pmaddwd %xmm13, %xmm7\n"
        "pmaddwd %xmm1, %xmm15\n"
        "vpmaddwd %xmm13, %xmm1, %xmm11\n"
        "vpmaddwd 0(%rip), %ymm14, %ymm14\n"
        "vpmaddwd %zmm0, %zmm20, %zmm26\n"
        "vpmaddwd -128(%r14,%rsi,1), %zmm12, %zmm12\n"
        "psadbw %xmm3, %xmm9\n"
        "psadbw 4096(%r14), %xmm4\n"
        "vpsadbw %xmm6, %xmm7, %xmm1\n"
        "vpsadbw %ymm1, %ymm11, %ymm12\n"
        "punpckhbw 0(%rip), %xmm9\n"
        "punpckhbw 0x1000(,%r11,2), %xmm9\n"
        "vpunpckhbw %ymm1, %ymm7, %ymm13\n"
        "vpunpckhbw %ymm5, %ymm6, %ymm6\n"
        "vpunpckhbw 128(%rdx,%r12,8), %ymm5, %ymm16\n"
        "vpunpckhbw 0x10(,%r8,4), %xmm9, %xmm19{%k7}\n"
        "punpckhwd 256(%r13), %xmm3\n"
        "punpckhwd (%rdi,%r9,1), %xmm9\n"
        "vpunpckhwd -64(%r9), %xmm9, %xmm3\n"
        "vpunpckhwd 0x100(%rip), %ymm4, %ymm13\n"
        "vpunpckhwd %zmm23, %zmm5, %zmm8{%k2}\n"
        "vpunpckhwd -8(%rip), %xmm4, %xmm27{%k3}{z}\n"
        "punpcklbw 256(%r15,%r10,2), %xmm11\n"
        "punpcklbw -0x1000(%r10), %xmm8\n"
        "vpunpcklbw 4096(%rcx,%r15,2), %ymm4, %ymm11\n"
        "vpunpcklbw 64(%rsi), %ymm13, %ymm11\n"
        "vpunpcklbw 4096(%r12), %zmm16, %zmm22{%k4}\n"
        "vpunpcklbw %ymm20, %ymm7, %ymm13{%k6}\n"
        "punpcklwd %xmm1, %xmm15\n"
        "punpcklwd 0x10(,%rsi,2), %xmm10\n"
        "vpunpcklwd 0x1000(,%rbp,8), %xmm7, %xmm12\n"
        "vpunpcklwd %ymm3, %ymm9, %ymm11\n"
        "vpunpcklwd %zmm25, %zmm21, %zmm1\n"
        "vpunpcklwd %ymm6, %ymm31, %ymm23{%k4}\n"
        "packsswb %xmm10, %xmm7\n"
        "packsswb %xmm9, %xmm15\n"
        "vpacksswb 0x1000(,%r11,4), %ymm14, %ymm6\n"
        "vpacksswb %ymm10, %ymm11, %ymm2\n"
        "vpacksswb %xmm4, %xmm9, %xmm31\n"
        "vpacksswb %ymm31, %ymm29, %ymm29\n"
        "packuswb %xmm4, %xmm13\n"
        "packuswb %xmm4, %xmm13\n"
        "vpackuswb %xmm11, %xmm10, %xmm3\n"
        "vpackuswb 0x1000(,%r13,8), %ymm14, %ymm2\n"
        "vpackuswb %zmm21, %zmm24, %zmm29{%k7}{z}\n"
        "vpackuswb %zmm20, %zmm2, %zmm6{%k2}{z}\n"
        "packssdw %xmm12, %xmm12\n"
        "packssdw %xmm15, %xmm4\n"
        "vpackssdw %ymm4, %ymm4, %ymm4\n"
        "vpackssdw 0x1000(,%r8,8), %xmm6, %xmm6\n"
        "vpackssdw %xmm20, %xmm17, %xmm15{%k5}\n"
        "vpackssdw 8(%rsi), %xmm29, %xmm16\n"
        "vpsadbw (%rbp), %ymm28, %ymm23\n"
        "vpsadbw 0x10(,%r12,2), %ymm1, %ymm14\n"
        "paddd %xmm8, %xmm2\n"
        "paddd %xmm1, %xmm5\n"
        "vpaddd %ymm9, %ymm7, %ymm14\n"
        "vpaddd 0x1000(,%rsi,8), %ymm5, %ymm6\n"
        "vpaddd 0x12345678(%rdx,%rax,8), %xmm22, %xmm25\n"
        "vpaddd %ymm9, %ymm16, %ymm21{%k2}\n"
        "psubd 0x12345678(%rsi), %xmm1\n"
        "psubd %xmm0, %xmm6\n"
        "vpsubd %ymm14, %ymm10, %ymm5\n"
        "vpsubd %ymm14, %ymm7, %ymm4\n"
        "vpsubd %ymm17, %ymm19, %ymm20\n"
        "vpsubd %ymm3, %ymm23, %ymm1{%k1}{z}\n"
        "pcmpeqd 0x1000(,%r12,8), %xmm0\n"
        "pcmpeqd 0(%rip), %xmm5\n"
        "vpcmpeqd 8(%r10,%r10,2), %ymm14, %ymm14\n"
        "vpcmpeqd %xmm3, %xmm11, %xmm2\n"
        "pcmpgtd 8(%r13,%r15,8), %xmm9\n"
        "pcmpgtd %xmm2, %xmm4\n"
        "vpcmpgtd 0x1000(,%r10,2), %ymm7, %ymm12\n"
        "vpcmpgtd %xmm10, %xmm12, %xmm14\n"
        "pmaxsd %xmm1, %xmm10\n"
        "pmaxsd %xmm4, %xmm10\n"
        "vpmaxsd %xmm14, %xmm1, %xmm15\n"
        "vpmaxsd 128(%r10), %xmm8, %xmm15\n"
        "vpmaxsd %ymm26, %ymm12, %ymm21{%k7}{z}\n"
        "vpmaxsd %zmm25, %zmm24, %zmm24{%k7}\n"
        "pminsd %xmm15, %xmm7\n"
        "pminsd 0(%rip), %xmm10\n"
        "vpminsd %ymm13, %ymm8, %ymm4\n"
        "vpminsd %xmm2, %xmm15, %xmm12\n"
        "vpminsd -64(%rdx,%rbp,8), %xmm22, %xmm24\n"
        "vpminsd %xmm13, %xmm11, %xmm9{%k5}{z}\n"
        "pmaxud %xmm11, %xmm13\n"
        "pmaxud -128(%rcx), %xmm7\n"
        "vpmaxud -8(%rip), %ymm2, %ymm12\n"
        "vpmaxud 4096(%r13), %xmm11, %xmm13\n"
        "vpmaxud -0x1000(%rdx,%r14,8), %xmm17, %xmm6\n"
        "vpmaxud 0x10(,%r9,2){1to4}, %xmm22, %xmm24{%k3}\n"
        "pminud %xmm8, %xmm6\n"
        "pminud 4096(%rsi), %xmm12\n"
        "vpminud 0x12345678(%r14), %ymm3, %ymm6\n"
        "vpminud %xmm6, %xmm14, %xmm10\n"
        "vpminud %xmm3, %xmm30, %xmm8{%k2}\n"
        "vpminud %ymm5, %ymm16, %ymm18\n"
        "pmulld %xmm0, %xmm7\n"
        "pmulld 0x12345678(%r11), %xmm4\n"
        "vpmulld 128(%rbx), %ymm13, %ymm10\n"
        "vpmulld -0x1000(%rbx), %xmm0, %xmm7\n"
        "vpmulld %xmm9, %xmm19, %xmm26{%k5}{z}\n"
        "vpmulld 0x10(,%r11,2), %zmm0, %zmm13{%k7}\n"
        "pmuludq %xmm0, %xmm12\n"
        "pmuludq 8(%rbp), %xmm7\n"
        "vpmuludq %xmm2, %xmm6, %xmm10\n"
        "vpmuludq 64(%rbp), %xmm10, %xmm9\n"
        "vpmuludq %zmm11, %zmm24, %zmm8{%k3}\n"
        "vpmuludq %xmm26, %xmm26, %xmm12\n"
        "punpckhdq %xmm3, %xmm0\n"
        "punpckhdq -64(%r10,%rdi,2), %xmm4\n"
        "vpunpckhdq 4096(%rsi,%rdi,1), %xmm5, %xmm2\n"
        "vpunpckhdq 0x100(%rip), %xmm13, %xmm8\n"
        "vpunpckhdq %ymm14, %ymm2, %ymm29\n"
        "vpunpckhdq 8(%r10,%r8,1), %ymm27, %ymm4\n"
        "punpckldq -8(%rip), %xmm15\n"
        "punpckldq 0x12345678(%r11,%r13,2), %xmm5\n"
        "vpunpckldq %xmm15, %xmm13, %xmm2\n"
        "vpunpckldq 0(%rip), %ymm12, %ymm12\n"
        "vpunpckldq 0x100(%rip){1to4}, %xmm28, %xmm21{%k6}{z}\n"
        "vpunpckldq 0x1000(,%rcx,4), %zmm15, %zmm6{%k7}{z}\n"
        "punpckhqdq %xmm12, %xmm12\n"
        "punpckhqdq %xmm4, %xmm6\n"
        "vpunpckhqdq -64(%r9), %ymm14, %ymm12\n"
        "vpunpckhqdq 64(%rdx), %ymm15, %ymm14\n"
        "vpunpckhqdq 0x10(,%r12,2){1to4}, %ymm11, %ymm30\n"
        "vpunpckhqdq 0(%rip){1to4}, %ymm24, %ymm11{%k5}{z}\n"
        "punpcklqdq %xmm1, %xmm6\n"
        "punpcklqdq 0x10(,%rbp,2), %xmm10\n"
        "vpunpcklqdq %ymm15, %ymm5, %ymm11\n"
        "vpunpcklqdq %ymm7, %ymm6, %ymm11\n"
        "vpunpcklqdq 4096(%r15), %ymm19, %ymm15{%k2}{z}\n"
        "vpunpcklqdq (%r13,%r12,8), %ymm12, %ymm18{%k3}{z}\n"
        "vpcmpeqb -0x1000(%r8), %xmm3, %k2\n"
        "vpcmpeqb %xmm25, %xmm27, %k7{%k2}\n"
        "vpcmpeqw %ymm27, %ymm6, %k3\n"
        "vpcmpeqw -8(%rip), %ymm7, %k4\n"
        "vpcmpgtb 0x12345678(%r9,%r12,1), %ymm16, %k4{%k4}\n"
        "vpcmpgtb %xmm8, %xmm16, %k6\n"
        "vpcmpgtw %zmm3, %zmm25, %k5{%k3}\n"
        "vpcmpgtw 0x10(,%rsi,8), %xmm27, %k7\n"
        "vpcmpeqd %ymm15, %ymm30, %k1{%k5}\n"
        "vpcmpeqd -0x1000(%r9){1to8}, %ymm22, %k7{%k7}\n"
        "vpcmpgtd 0x12345678(%r14){1to16}, %zmm4, %k4\n"
        "vpcmpgtd 0x12345678(%rsi,%rbx,2){1to16}, %zmm3, %k1\n"
        "vpcmpeqq %zmm12, %zmm11, %k3{%k1}\n"
        "vpcmpeqq %ymm27, %ymm10, %k6{%k1}\n"
        "vpcmpgtq 0x100(%rip), %ymm8, %k5{%k4}\n"
        "vpcmpgtq 0x1000(,%rsi,4), %zmm1, %k6\n"
        "pcmpeqq %xmm10, %xmm5\n"
        "pcmpeqq 0x100(%rip), %xmm15\n"
        "vpcmpeqq %ymm1, %ymm1, %ymm12\n"
        "vpcmpeqq %ymm4, %ymm2, %ymm10\n"
        "pcmpgtq %xmm14, %xmm6\n"
        "pcmpgtq %xmm9, %xmm11\n"
        "vpcmpgtq %xmm4, %xmm3, %xmm0\n"
        "vpcmpgtq 0x1000(,%rsi,8), %ymm2, %ymm6\n"
        "paddq (%rdx,%r13,1), %xmm3\n"
        "paddq -0x1000(%r11,%rcx,2), %xmm12\n"
        "vpaddq 0x1000(,%rax,2), %ymm14, %ymm3\n"
        "vpaddq -8(%rip), %xmm2, %xmm5\n"
        "vpaddq %xmm11, %xmm9, %xmm20\n"
        "vpaddq 0x12345678(%rax), %xmm23, %xmm3\n"
        "psubq 8(%rbx,%rdi,2), %xmm11\n"
        "psubq %xmm4, %xmm6\n"
        "vpsubq %ymm9, %ymm3, %ymm11\n"
        "vpsubq 0x100(%rip), %xmm10, %xmm2\n"
        "vpsubq 0(%rip), %ymm19, %ymm29{%k2}{z}\n"
        "vpsubq 256(%rdx), %zmm18, %zmm16{%k1}\n"
        "pmuldq -64(%rbp,%rbx,8), %xmm2\n"
        "pmuldq %xmm14, %xmm13\n"
        "vpmuldq %xmm4, %xmm8, %xmm6\n"
        "vpmuldq %ymm11, %ymm14, %ymm0\n"
        "vpmuldq 0x1000(,%rdx,8), %zmm13, %zmm20{%k3}\n"
        "vpmuldq 0x12345678(%rbx,%r8,1), %zmm6, %zmm30\n"
        "pand 0x1000(,%rbx,8), %xmm3\n"
        "pand 0x10(,%rbp,4), %xmm10\n"
        "vpand %ymm8, %ymm12, %ymm2\n"
        "vpand 4096(%r11,%r8,8), %ymm2, %ymm2\n"
        "vpandd 64(%rax,%r12,1), %xmm24, %xmm25{%k7}\n"
        "vpandd 128(%rax), %zmm25, %zmm12{%k4}{z}\n"
        "vpandq %zmm2, %zmm18, %zmm12{%k2}{z}\n"
        "vpandq %xmm20, %xmm18, %xmm2{%k4}{z}\n"
        "pandn 0x10(,%r8,4), %xmm15\n"
        "pandn -64(%rcx), %xmm6\n"
        "vpandn 0x10(,%r10,4), %xmm10, %xmm4\n"
        "vpandn 0x100(%rip), %ymm13, %ymm4\n"
        "vpandnd %ymm7, %ymm11, %ymm8\n"
        "vpandnd %ymm17, %ymm31, %ymm22\n"
        "vpandnq %xmm16, %xmm15, %xmm4\n"
        "vpandnq %xmm15, %xmm22, %xmm3{%k1}{z}\n"
        "por 0x100(%rip), %xmm5\n"
        "por 256(%r14,%rbx,4), %xmm11\n"
        "vpor -8(%rip), %xmm11, %xmm7\n"
        "vpor %xmm14, %xmm0, %xmm3\n"
        "vpord -0x1000(%rcx), %xmm8, %xmm23{%k4}\n"
        "vpord 0(%rip){1to8}, %ymm4, %ymm7\n"
        "vporq -8(%rip), %xmm12, %xmm0\n"
        "vporq -64(%r12), %zmm27, %zmm21{%k7}{z}\n"
        "pxor %xmm11, %xmm7\n"
        "pxor %xmm3, %xmm14\n"
        "vpxor %xmm1, %xmm7, %xmm4\n"
        "vpxor -0x1000(%rdi,%r13,4), %ymm7, %ymm5\n"
        "vpxord 0x1000(,%rcx,8){1to16}, %zmm21, %zmm6\n"
        "vpxord %xmm13, %xmm30, %xmm1{%k7}{z}\n"
        "vpxorq %zmm12, %zmm28, %zmm31\n"
        "vpxorq %zmm24, %zmm0, %zmm16{%k2}\n"
        "vaddps %ymm12, %ymm14, %ymm15\n"
        "vaddps 0x1000(,%r12,4), %ymm9, %ymm1\n"
        "vaddps 64(%r12,%r13,4), %xmm19, %xmm7\n"
        "vaddps 64(%r14){1to8}, %ymm27, %ymm23\n"
        "vaddpd 0x1000(,%rsi,8), %ymm12, %ymm3\n"
        "vaddpd 128(%rsi), %ymm15, %ymm0\n"
        "vaddpd -8(%rip){1to2}, %xmm13, %xmm6\n"
        "vaddpd %xmm20, %xmm9, %xmm18\n"
        "vaddss %xmm10, %xmm6, %xmm9\n"
        "vaddss 0x100(%rip), %xmm10, %xmm13\n"
        "vaddss %xmm16, %xmm21, %xmm4\n"
        "vaddss %xmm28, %xmm30, %xmm12\n"
        "vaddsd %xmm11, %xmm9, %xmm13\n"
        "vaddsd 0x12345678(%r13,%r9,2), %xmm6, %xmm3\n"
        "vaddsd {ru-sae}, %xmm3, %xmm16, %xmm3\n"
        "vaddsd %xmm0, %xmm12, %xmm11\n"
        "vsubps 0x1000(,%rax,4), %xmm10, %xmm7\n"
        "vsubps 0x10(,%rdi,8), %xmm14, %xmm10\n"
        "vsubps 0x1000(,%r8,2){1to16}, %zmm1, %zmm16{%k5}\n"
        "vsubps %zmm25, %zmm25, %zmm12{%k2}\n"
        "vsubpd 8(%rax,%r10,4), %ymm12, %ymm6\n"
        "vsubpd -64(%r11,%r9,2), %ymm13, %ymm13\n"
        "vsubpd {rz-sae}, %zmm28, %zmm13, %zmm0{%k4}{z}\n"
        "vsubpd 8(%rbx), %zmm18, %zmm25\n"
        "vsubss 0x100(%rip), %xmm1, %xmm14\n"
        "vsubss %xmm13, %xmm3, %xmm9\n"
        "vsubss 0x1000(,%r12,4), %xmm10, %xmm27{%k2}\n"
        "vsubss -8(%rip), %xmm28, %xmm17{%k1}\n"
        "vsubsd %xmm13, %xmm5, %xmm5\n"
        "vsubsd (%r12,%r13,2), %xmm13, %xmm4\n"
        "vsubsd 0x12345678(%r10), %xmm1, %xmm7{%k7}\n"
        "vsubsd %xmm31, %xmm25, %xmm9\n"
        "vmulps %ymm9, %ymm9, %ymm9\n"
        "vmulps (%rsi), %xmm3, %xmm11\n"
        "vmulps 128(%r13){1to16}, %zmm15, %zmm21{%k2}\n"
        "vmulps -0x1000(%rdx,%rdx,1), %ymm25, %ymm29{%k3}{z}\n"
        "vmulpd %xmm8, %xmm9, %xmm7\n"
        "vmulpd %ymm8, %ymm2, %ymm8\n"
        "vmulpd %ymm15, %ymm24, %ymm14\n"
        "vmulpd %xmm2, %xmm12, %xmm12{%k6}{z}\n"
        "vmulss %xmm1, %xmm1, %xmm6\n"
        "vmulss 0x100(%rip), %xmm6, %xmm2\n"
        "vmulss -8(%rip), %xmm7, %xmm14\n"
        "vmulss 0x100(%rip), %xmm21, %xmm5\n"
        "vmulsd 0x100(%rip), %xmm9, %xmm4\n"
        "vmulsd %xmm3, %xmm13, %xmm1\n"
        "vmulsd 128(%r8,%rdi,4), %xmm15, %xmm4\n"
        "vmulsd {rz-sae}, %xmm3, %xmm22, %xmm21\n"
        "vdivps 0x10(,%r11,8), %xmm2, %xmm1\n"
        "vdivps %xmm5, %xmm4, %xmm4\n"
        "vdivps 4096(%rbx,%rax,8), %ymm31, %ymm23{%k1}{z}\n"
        "vdivps %xmm2, %xmm19, %xmm18\n"
        "vdivpd %ymm7, %ymm4, %ymm2\n"
        "vdivpd -64(%r10), %ymm2, %ymm11\n"
        "vdivpd -8(%rip), %zmm0, %zmm18{%k4}\n"
        "vdivpd %xmm2, %xmm20, %xmm6\n"
        "vdivss %xmm1, %xmm12, %xmm7\n"
        "vdivss 0x1000(,%rcx,2), %xmm3, %xmm11\n"
        "vdivss {rn-sae}, %xmm12, %xmm1, %xmm30\n"
        "vdivss %xmm25, %xmm16, %xmm12\n"
        "vdivsd %xmm6, %xmm6, %xmm3\n"
        "vdivsd 0x10(,%r11,4), %xmm14, %xmm13\n"
        "vdivsd {ru-sae}, %xmm17, %xmm0, %xmm5{%k1}{z}\n"
        "vdivsd %xmm21, %xmm2, %xmm29{%k3}\n"
        "vminps 0(%rip), %xmm4, %xmm3\n"
        "vminps %ymm6, %ymm0, %ymm5\n"
        "vminps 0x1000(,%r10,4), %xmm26, %xmm5\n"
        "vminps -64(%r12){1to16}, %zmm22, %zmm6\n"
        "vminpd %xmm12, %xmm5, %xmm4\n"
        "vminpd 128(%r10,%r15,4), %xmm5, %xmm11\n"
        "vminpd %zmm13, %zmm17, %zmm18\n"
        "vminpd 0x10(,%r8,8){1to4}, %ymm4, %ymm5{%k3}\n"
        "vminss %xmm12, %xmm4, %xmm5\n"
        "vminss %xmm10, %xmm10, %xmm11\n"
        "vminss %xmm5, %xmm29, %xmm3\n"
        "vminss {sae}, %xmm16, %xmm26, %xmm12\n"
        "vminsd 0x10(,%rdi,8), %xmm9, %xmm1\n"
        "vminsd 0x10(,%rax,8), %xmm5, %xmm6\n"
        "vminsd %xmm9, %xmm0, %xmm29{%k5}{z}\n"
        "vminsd 64(%rcx,%r12,1), %xmm23, %xmm6\n"
        "vmaxps %xmm2, %xmm6, %xmm2\n"
        "vmaxps %ymm7, %ymm4, %ymm11\n"
        "vmaxps %ymm17, %ymm3, %ymm23\n"
        "vmaxps 0(%rip), %zmm7, %zmm26{%k7}\n"
        "vmaxpd %ymm9, %ymm2, %ymm3\n"
        "vmaxpd %ymm14, %ymm2, %ymm11\n"
        "vmaxpd -64(%r11){1to8}, %zmm26, %zmm14\n"
        "vmaxpd 0x10(,%rbx,4), %zmm7, %zmm22{%k1}\n"
        "vmaxss %xmm1, %xmm1, %xmm15\n"
        "vmaxss 8(%rcx,%r11,1), %xmm7, %xmm10\n"
        "vmaxss {sae}, %xmm13, %xmm13, %xmm27{%k7}\n"
        "vmaxss 0(%rip), %xmm10, %xmm18\n"
        "vmaxsd 4096(%rax,%rdi,8), %xmm0, %xmm4\n"
        "vmaxsd -8(%rip), %xmm2, %xmm11\n"
        "vmaxsd {sae}, %xmm1, %xmm26, %xmm3{%k7}{z}\n"
        "vmaxsd {sae}, %xmm14, %xmm2, %xmm13\n"
        "vsqrtps -64(%r9), %xmm6\n"
        "vsqrtps 4096(%rax), %ymm14\n"
        "vsqrtps -64(%rdi){1to4}, %xmm20\n"
        "vsqrtps 4096(%rax), %ymm4\n"
        "vsqrtpd -0x1000(%r8,%rax,1), %ymm1\n"
        "vsqrtpd %ymm3, %ymm1\n"
        "vsqrtpd (%rax,%r14,8){1to8}, %zmm1{%k1}{z}\n"
        "vsqrtpd %ymm8, %ymm28\n"
        "vsqrtss %xmm15, %xmm3, %xmm3\n"
        "vsqrtss %xmm1, %xmm8, %xmm3\n"
        "vsqrtss 0x1000(,%r9,4), %xmm21, %xmm4\n"
        "vsqrtss {rn-sae}, %xmm15, %xmm1, %xmm11{%k4}{z}\n"
        "vsqrtsd 0x12345678(%rbx), %xmm3, %xmm5\n"
        "vsqrtsd 0x1000(,%rbp,4), %xmm14, %xmm2\n"
        "vsqrtsd %xmm31, %xmm4, %xmm21{%k1}{z}\n"
        "vsqrtsd %xmm28, %xmm28, %xmm1{%k4}{z}\n"
        "vandps (%r11), %xmm13, %xmm10\n"
        "vandps -128(%r12), %xmm6, %xmm4\n"
        "vandps %xmm17, %xmm2, %xmm1{%k2}{z}\n"
        "vandps %ymm13, %ymm16, %ymm18\n"
        "vandpd %xmm4, %xmm6, %xmm8\n"
        "vandpd -64(%r10), %ymm13, %ymm7\n"
        "vandpd 64(%r14), %ymm9, %ymm20\n"
        "vandpd 0x10(,%rsi,2), %ymm15, %ymm20\n"
        "vandnps %xmm3, %xmm5, %xmm4\n"
        "vandnps %xmm15, %xmm14, %xmm1\n"
        "vandnps %ymm5, %ymm30, %ymm17{%k6}{z}\n"
        "vandnps 0x100(%rip){1to8}, %ymm4, %ymm15{%k2}\n"
        "vandnpd %xmm1, %xmm7, %xmm6\n"
        "vandnpd -8(%rip), %ymm2, %ymm6\n"
        "vandnpd %zmm1, %zmm10, %zmm5{%k7}\n"
        "vandnpd 0x12345678(%r15), %zmm6, %zmm19{%k2}{z}\n"
        "vorps 0x10(,%rdi,8), %xmm11, %xmm7\n"
        "vorps %xmm10, %xmm12, %xmm4\n"
        "vorps %ymm29, %ymm25, %ymm31{%k7}\n"
        "vorps 128(%r8), %ymm25, %ymm3{%k6}\n"
        "vorpd (%r12,%rbp,8), %xmm5, %xmm6\n"
        "vorpd 0x12345678(%rax), %xmm2, %xmm8\n"
        "vorpd -8(%rip), %xmm20, %xmm13\n"
        "vorpd 0x10(,%r8,2), %ymm15, %ymm31\n"
        "vxorps %xmm5, %xmm6, %xmm1\n"
        "vxorps 8(%r15,%r9,4), %ymm6, %ymm8\n"
        "vxorps %xmm20, %xmm13, %xmm0{%k2}\n"
        "vxorps %zmm5, %zmm1, %zmm0\n"
        "vxorpd %ymm13, %ymm8, %ymm2\n"
        "vxorpd 128(%r14,%r12,1), %ymm3, %ymm5\n"
        "vxorpd %xmm31, %xmm15, %xmm13{%k7}\n"
        "vxorpd (%r8), %zmm29, %zmm23\n"
        "vunpcklps %xmm1, %xmm3, %xmm1\n"
        "vunpcklps -64(%r12,%r15,2), %ymm8, %ymm3\n"
        "vunpcklps %ymm21, %ymm1, %ymm26\n"
        "vunpcklps 0x100(%rip){1to8}, %ymm20, %ymm3{%k6}{z}\n"
        "vunpckhps %ymm13, %ymm4, %ymm11\n"
        "vunpckhps %xmm2, %xmm0, %xmm2\n"
        "vunpckhps %ymm26, %ymm11, %ymm11{%k1}\n"
        "vunpckhps %ymm15, %ymm23, %ymm25{%k2}\n"
        "vunpcklpd %xmm8, %xmm14, %xmm14\n"
        "vunpcklpd %xmm12, %xmm6, %xmm6\n"
        "vunpcklpd %zmm5, %zmm14, %zmm30{%k2}\n"
        "vunpcklpd %zmm4, %zmm25, %zmm31{%k2}{z}\n"
        "vunpckhpd (%rbp,%rax,4), %ymm7, %ymm13\n"
        "vunpckhpd -8(%rip), %ymm14, %ymm13\n"
        "vunpckhpd -128(%r13), %zmm2, %zmm23\n"
        "vunpckhpd -128(%rdi){1to4}, %ymm11, %ymm18{%k3}\n"
        "vcmpps $224, %ymm13, %ymm10, %ymm6\n"
        "vcmpps $122, (%rbx), %ymm6, %ymm8\n"
        "vcmppd $143, %xmm0, %xmm6, %xmm10\n"
        "vcmppd $113, 0(%rip), %xmm13, %xmm11\n"
        "vcmpss $219, 128(%rbx,%r9,2), %xmm9, %xmm9\n"
        "vcmpss $76, 0(%rip), %xmm5, %xmm12\n"
        "vcmpsd $63, 0x1000(,%rax,8), %xmm3, %xmm2\n"
        "vcmpsd $119, %xmm5, %xmm9, %xmm14\n"
        "vcmpps $4, 0x12345678(%rcx,%r11,1){1to16}, %zmm29, %k4\n"
        "vcmpps $94, %zmm14, %zmm30, %k0\n"
        "vcmppd $126, %zmm2, %zmm8, %k7\n"
        "vcmpss $185, 0x100(%rip), %xmm13, %k5\n"
        "vcmpss $203, %xmm0, %xmm29, %k2\n"
        "vcmpsd $208, %xmm3, %xmm16, %k7\n"
        "vcmpsd $243, 0x10(,%rbp,4), %xmm5, %k4\n"
        "vcomiss %xmm10, %xmm7\n"
        "vcomiss %xmm12, %xmm10\n"
        "vcomiss {sae}, %xmm27, %xmm29\n"
        "vcomiss 0x10(,%r14,2), %xmm1\n"
        "vucomiss %xmm7, %xmm11\n"
        "vucomiss 0x1000(,%r12,4), %xmm4\n"
        "vucomiss 128(%r13), %xmm4\n"
        "vucomiss 0x100(%rip), %xmm22\n"
        "vcomisd 4096(%rdx,%r12,2), %xmm2\n"
        "vcomisd %xmm1, %xmm14\n"
        "vcomisd %xmm18, %xmm24\n"
        "vcomisd -0x1000(%r8,%r15,4), %xmm20\n"
        "vucomisd -128(%r8), %xmm1\n"
        "vucomisd %xmm1, %xmm15\n"
        "vucomisd %xmm27, %xmm6\n"
        "vucomisd 4096(%rbp,%rbx,2), %xmm28\n"
        "vshufps $119, 0(%rip), %xmm5, %xmm11\n"
        "vshufps $219, 0(%rip), %ymm6, %ymm2\n"
        "vshufpd $231, -0x1000(%r12,%r14,4), %xmm7, %xmm5\n"
        "vshufpd $236, %xmm9, %xmm9, %xmm3\n"
        "vshufps $204, 4096(%r8,%r8,8){1to16}, %zmm4, %zmm1\n"
        "vshufps $105, 4096(%rbp,%r8,2), %ymm22, %ymm6{%k7}{z}\n"
        "vshufpd $49, 64(%rbx,%r14,4), %zmm16, %zmm31\n"
        "vshufpd $202, -128(%r9,%rdx,4){1to4}, %ymm9, %ymm20\n"
        "vmovaps %ymm13, %ymm0\n"
        "vmovaps 0x1000(,%r9,8), %ymm14\n"
        "vmovaps %ymm11, (%r8,%rbx,4)\n"
        "vmovaps %xmm6, %xmm12\n"
        "vmovaps 64(%rdi,%rax,4), %xmm14{%k3}{z}\n"
        "vmovaps %ymm29, %ymm16{%k5}{z}\n"
        "vmovaps %zmm10, %zmm23\n"
        "vmovaps %zmm7, -0x1000(%r9){%k1}\n"
        "vmovups %ymm2, %ymm10\n"
        "vmovups -128(%rcx,%rdx,4), %ymm2\n"
        "vmovups %xmm15, -8(%rip)\n"
        "vmovups %xmm11, 128(%r11)\n"
        "vmovups %ymm27, %ymm16\n"
        "vmovups -128(%rsi,%r8,4), %xmm4{%k7}\n"
        "vmovups %xmm21, 0x12345678(%rdi,%rsi,2)\n"
        "vmovups %xmm8, %xmm8\n"
        "vmovapd %xmm10, %xmm4\n"
        "vmovapd %ymm12, %ymm3\n"
        "vmovapd %xmm9, -0x1000(%rbx,%r12,1)\n"
        "vmovapd %xmm6, 0x10(,%r11,8)\n"
        "vmovapd %zmm12, %zmm28{%k7}{z}\n"
        "vmovapd %zmm26, %zmm19\n"
        "vmovapd %zmm2, %zmm16{%k2}{z}\n"
        "vmovapd %zmm19, %zmm23{%k2}{z}\n"
        "vmovupd -64(%r10), %xmm5\n"
        "vmovupd %ymm15, %ymm1\n"
        "vmovupd %ymm13, -8(%rip)\n"
        "vmovupd %xmm10, 4096(%r8)\n"
        "vmovupd -8(%rip), %xmm21\n"
        "vmovupd %ymm2, %ymm8\n"
        "vmovupd %ymm15, 0x100(%rip)\n"
        "vmovupd %xmm16, -8(%rip){%k5}\n"
        "vmovdqa %ymm2, %ymm11\n"
        "vmovdqa 0x12345678(%rbx,%r14,2), %ymm11\n"
        "vmovdqa %xmm6, 0x12345678(%rdi,%r9,2)\n"
        "vmovdqa %xmm14, %xmm13\n"
        "vmovdqu -64(%r14,%rcx,8), %ymm0\n"
        "vmovdqu %ymm3, %ymm3\n"
        "vmovdqu %xmm0, 8(%rcx)\n"
        "vmovdqu %ymm14, 0x1000(,%rdi,8)\n"
        "vmovdqu32 0x1000(,%rbx,4), %zmm6{%k3}{z}\n"
        "vmovdqu32 %zmm11, %zmm9\n"
        "vmovdqu32 %xmm18, %xmm25{%k5}{z}\n"
        "vmovdqu32 %xmm24, (%rdi){%k5}\n"
        "vmovdqu64 %ymm13, %ymm8{%k2}\n"
        "vmovdqu64 %ymm25, %ymm30\n"
        "vmovdqu64 %xmm14, %xmm11\n"
        "vmovdqu64 %ymm8, %ymm26\n"
        "vmovdqa32 %xmm21, %xmm22{%k2}{z}\n"
        "vmovdqa32 -0x1000(%rax,%r13,2), %ymm23\n"
        "vmovdqa32 %zmm12, -8(%rip){%k5}\n"
        "vmovdqa32 %xmm10, %xmm3\n"
        "vmovdqa64 %xmm18, %xmm15\n"
        "vmovdqa64 %zmm24, %zmm14\n"
        "vmovdqa64 %zmm24, %zmm11\n"
        "vmovdqa64 %zmm10, %zmm8{%k4}{z}\n"
        "vmovdqu8 %ymm28, %ymm31{%k7}\n"
        "vmovdqu8 %zmm1, %zmm25{%k2}\n"
        "vmovdqu8 %xmm30, 128(%r10){%k6}\n"
        "vmovdqu8 %ymm1, %ymm27{%k6}{z}\n"
        "vmovdqu16 0x12345678(%r11,%rcx,4), %xmm30{%k5}\n"
        "vmovdqu16 8(%rax,%rsi,8), %ymm27{%k6}{z}\n"
        "vmovdqu16 %zmm24, %zmm0\n"
        "vmovdqu16 %xmm25, %xmm16{%k3}\n"
        "vbroadcastss %xmm5, %ymm8\n"
        "vbroadcastss 0x10(,%rbx,4), %ymm6\n"
        "vbroadcastss %xmm9, %xmm3{%k2}\n"
        "vbroadcastss 0x1000(,%r13,8), %zmm20{%k1}{z}\n"
        "vbroadcastsd -128(%rsi,%rbx,1), %ymm12\n"
        "vbroadcastsd 0x10(,%rbp,8), %ymm5\n"
        "vbroadcastsd 0x100(%rip), %zmm12\n"
        "vbroadcastsd 64(%r13), %ymm25{%k6}{z}\n"
        "vpbroadcastb %xmm1, %xmm6\n"
        "vpbroadcastb %xmm0, %xmm2\n"
        "vpbroadcastw %xmm1, %xmm5\n"
        "vpbroadcastw %xmm14, %xmm6\n"
        "vpbroadcastd %xmm11, %xmm7\n"
        "vpbroadcastd %xmm11, %xmm13\n"
        "vpbroadcastq 0(%rip), %xmm15\n"
        "vpbroadcastq %xmm12, %ymm3\n"
        "vpbroadcastb %xmm10, %zmm7{%k3}{z}\n"
        "vpbroadcastb %xmm29, %ymm24\n"
        "vpbroadcastw %xmm8, %xmm3{%k2}\n"
        "vpbroadcastw -0x1000(%r10), %xmm8{%k1}\n"
        "vpbroadcastd %xmm3, %xmm3{%k5}{z}\n"
        "vpbroadcastd %xmm27, %zmm29{%k2}{z}\n"
        "vpbroadcastq 4096(%r13), %xmm30\n"
        "vpbroadcastq 0x10(,%r9,4), %zmm14\n"
        "vfmadd132ps %ymm5, %ymm4, %ymm6\n"
        "vfmadd132ps %xmm10, %xmm0, %xmm6\n"
        "vfmadd132ps {rz-sae}, %zmm22, %zmm8, %zmm20{%k7}\n"
        "vfmadd132ps -128(%rsi), %ymm23, %ymm25\n"
        "vfmadd132pd %xmm14, %xmm7, %xmm5\n"
        "vfmadd132pd %xmm11, %xmm10, %xmm8\n"
        "vfmadd132pd -64(%r12,%rdi,1){1to4}, %ymm26, %ymm0\n"
        "vfmadd132pd %xmm23, %xmm3, %xmm14{%k7}\n"
        "vfmadd132ss 0x100(%rip), %xmm2, %xmm14\n"
        "vfmadd132ss -8(%rip), %xmm15, %xmm13\n"
        "vfmadd132ss {rz-sae}, %xmm6, %xmm2, %xmm8{%k5}\n"
        "vfmadd132ss %xmm2, %xmm21, %xmm26\n"
        "vfmadd132sd %xmm10, %xmm14, %xmm1\n"
        "vfmadd132sd 0x12345678(%r11), %xmm9, %xmm3\n"
        "vfmadd132sd 256(%rax), %xmm11, %xmm30\n"
        "vfmadd132sd 0x1000(,%rbx,4), %xmm16, %xmm6{%k5}\n"
        "vfmadd213ps %ymm9, %ymm12, %ymm3\n"
        "vfmadd213ps 0x100(%rip), %ymm14, %ymm11\n"
        "vfmadd213ps %ymm25, %ymm5, %ymm4\n"
        "vfmadd213ps 0x12345678(%r10), %xmm14, %xmm27{%k7}{z}\n"
        "vfmadd213pd 0x100(%rip), %xmm15, %xmm15\n"
        "vfmadd213pd 0x1000(,%rax,4), %xmm15, %xmm10\n"
        "vfmadd213pd %xmm19, %xmm4, %xmm12\n"
        "vfmadd213pd %ymm30, %ymm14, %ymm7\n"
        "vfmadd213ss 4096(%rdi,%r13,8), %xmm5, %xmm9\n"
        "vfmadd213ss %xmm9, %xmm10, %xmm8\n"
        "vfmadd213ss 0x10(,%rsi,2), %xmm22, %xmm13{%k4}{z}\n"
        "vfmadd213ss %xmm2, %xmm14, %xmm24{%k5}\n"
        "vfmadd213sd 0x10(,%r8,8), %xmm1, %xmm10\n"
        "vfmadd213sd 0x1000(,%rdx,4), %xmm1, %xmm7\n"
        "vfmadd213sd 256(%rax), %xmm28, %xmm12{%k2}{z}\n"
        "vfmadd213sd 0(%rip), %xmm5, %xmm14\n"
        "vfmadd231ps %xmm10, %xmm3, %xmm3\n"
        "vfmadd231ps 0x1000(,%rdi,8), %xmm0, %xmm12\n"
        "vfmadd231ps 0x1000(,%rcx,2), %zmm2, %zmm25{%k3}{z}\n"
        "vfmadd231ps %zmm2, %zmm2, %zmm18\n"
        "vfmadd231pd (%r10), %xmm11, %xmm12\n"
        "vfmadd231pd %xmm3, %xmm1, %xmm2\n"
        "vfmadd231pd %xmm29, %xmm15, %xmm20{%k4}{z}\n"
        "vfmadd231pd 0(%rip){1to4}, %ymm17, %ymm18{%k4}{z}\n"
        "vfmadd231ss %xmm13, %xmm12, %xmm0\n"
        "vfmadd231ss %xmm9, %xmm2, %xmm7\n"
        "vfmadd231ss %xmm18, %xmm24, %xmm22{%k3}{z}\n"
        "vfmadd231ss %xmm15, %xmm11, %xmm12{%k7}\n"
        "vfmadd231sd %xmm4, %xmm6, %xmm6\n"
        "vfmadd231sd %xmm15, %xmm6, %xmm2\n"
        "vfmadd231sd %xmm9, %xmm2, %xmm21{%k6}\n"
        "vfmadd231sd %xmm5, %xmm14, %xmm20{%k1}{z}\n"
        "vfmsub132ps -8(%rip), %xmm15, %xmm9\n"
        "vfmsub132ps 8(%r13,%rsi,2), %xmm7, %xmm2\n"
        "vfmsub132ps -128(%rdi,%rbx,1){1to8}, %ymm0, %ymm22{%k7}\n"
        "vfmsub132ps 4096(%rdi), %zmm21, %zmm22\n"
        "vfmsub132pd 8(%rbp,%rbp,4), %xmm3, %xmm3\n"
        "vfmsub132pd -64(%rbp), %ymm12, %ymm12\n"
        "vfmsub132pd 0x10(,%rcx,4), %zmm3, %zmm14{%k5}\n"
        "vfmsub132pd 64(%rdi), %ymm14, %ymm13\n"
        "vfmsub132ss 0x1000(,%rbp,4), %xmm6, %xmm13\n"
        "vfmsub132ss 0x10(,%r13,8), %xmm0, %xmm3\n"
        "vfmsub132ss 0x10(,%r15,8), %xmm28, %xmm21\n"
        "vfmsub132ss %xmm21, %xmm3, %xmm25\n"
        "vfmsub132sd %xmm8, %xmm3, %xmm6\n"
        "vfmsub132sd 64(%rsi), %xmm4, %xmm9\n"
        "vfmsub132sd 0(%rip), %xmm6, %xmm21\n"
        "vfmsub132sd -64(%rdx,%rcx,8), %xmm11, %xmm19\n"
        "vfmsub213ps 0x10(,%rdx,4), %ymm1, %ymm10\n"
        "vfmsub213ps 0(%rip), %ymm1, %ymm2\n"
        "vfmsub213ps %xmm31, %xmm31, %xmm23\n"
        "vfmsub213ps 8(%r9){1to16}, %zmm25, %zmm12{%k1}{z}\n"
        "vfmsub213pd 4096(%r15,%r12,1), %ymm15, %ymm7\n"
        "vfmsub213pd %xmm1, %xmm7, %xmm14\n"
        "vfmsub213pd %ymm9, %ymm8, %ymm13\n"
        "vfmsub213pd %xmm2, %xmm23, %xmm16\n"
        "vfmsub213ss %xmm7, %xmm15, %xmm10\n"
        "vfmsub213ss %xmm10, %xmm10, %xmm11\n"
        "vfmsub213ss 0x1000(,%rbp,4), %xmm22, %xmm9\n"
        "vfmsub213ss %xmm16, %xmm3, %xmm14{%k1}{z}\n"
        "vfmsub213sd 64(%rbx,%rdx,1), %xmm11, %xmm6\n"
        "vfmsub213sd %xmm11, %xmm11, %xmm7\n"
        "vfmsub213sd 4096(%rax), %xmm12, %xmm5\n"
        "vfmsub213sd -0x1000(%r8), %xmm11, %xmm26\n"
        "vfmsub231ps 0x10(,%r12,8), %ymm6, %ymm11\n"
        "vfmsub231ps %xmm11, %xmm11, %xmm4\n"
        "vfmsub231ps 0x10(,%rsi,4), %ymm21, %ymm13{%k5}\n"
        "vfmsub231ps %ymm2, %ymm24, %ymm17\n"
        "vfmsub231pd -8(%rip), %xmm9, %xmm10\n"
        "vfmsub231pd %xmm4, %xmm11, %xmm0\n"
        "vfmsub231pd %ymm0, %ymm26, %ymm7{%k4}{z}\n"
        "vfmsub231pd %ymm3, %ymm7, %ymm10{%k1}\n"
        "vfmsub231ss %xmm0, %xmm7, %xmm5\n"
        "vfmsub231ss %xmm11, %xmm5, %xmm3\n"
        "vfmsub231ss %xmm27, %xmm1, %xmm22{%k4}{z}\n"
        "vfmsub231ss -64(%rsi,%r15,1), %xmm20, %xmm0{%k6}\n"
        "vfmsub231sd %xmm2, %xmm9, %xmm1\n"
        "vfmsub231sd %xmm0, %xmm2, %xmm10\n"
        "vfmsub231sd -8(%rip), %xmm7, %xmm11{%k2}{z}\n"
        "vfmsub231sd 0(%rip), %xmm29, %xmm17{%k4}{z}\n"
        "vfnmadd132ps %xmm14, %xmm1, %xmm14\n"
        "vfnmadd132ps 0x1000(,%rbp,2), %ymm1, %ymm11\n"
        "vfnmadd132ps 256(%rax), %zmm21, %zmm3\n"
        "vfnmadd132ps %zmm3, %zmm4, %zmm12\n"
        "vfnmadd132pd %ymm0, %ymm2, %ymm2\n"
        "vfnmadd132pd %xmm6, %xmm10, %xmm1\n"
        "vfnmadd132pd 0x10(,%r10,2){1to2}, %xmm25, %xmm26\n"
        "vfnmadd132pd 0x10(,%r9,8){1to8}, %zmm10, %zmm14\n"
        "vfnmadd132ss 0x100(%rip), %xmm1, %xmm3\n"
        "vfnmadd132ss %xmm2, %xmm10, %xmm3\n"
        "vfnmadd132ss 0x10(,%r8,2), %xmm15, %xmm10{%k5}\n"
        "vfnmadd132ss -64(%r8,%rax,2), %xmm23, %xmm22{%k3}\n"
        "vfnmadd132sd %xmm2, %xmm10, %xmm11\n"
        "vfnmadd132sd 0x1000(,%r11,2), %xmm14, %xmm11\n"
        "vfnmadd132sd {rz-sae}, %xmm25, %xmm20, %xmm18\n"
        "vfnmadd132sd 8(%r10,%rsi,1), %xmm13, %xmm19{%k4}{z}\n"
        "vfnmadd213ps 0x100(%rip), %xmm1, %xmm0\n"
        "vfnmadd213ps 0x100(%rip), %ymm6, %ymm11\n"
        "vfnmadd213ps (%rcx,%r15,1), %xmm7, %xmm6{%k6}{z}\n"
        "vfnmadd213ps 0x10(,%rsi,4), %ymm11, %ymm8\n"
        "vfnmadd213pd %xmm8, %xmm5, %xmm5\n"
        "vfnmadd213pd %xmm6, %xmm4, %xmm10\n"
        "vfnmadd213pd %ymm4, %ymm28, %ymm13{%k4}{z}\n"
        "vfnmadd213pd %zmm12, %zmm16, %zmm11{%k6}\n"
        "vfnmadd213ss %xmm14, %xmm3, %xmm6\n"
        "vfnmadd213ss -8(%rip), %xmm13, %xmm6\n"
        "vfnmadd213ss 0(%rip), %xmm23, %xmm2\n"
        "vfnmadd213ss %xmm1, %xmm13, %xmm25\n"
        "vfnmadd213sd %xmm1, %xmm0, %xmm10\n"
        "vfnmadd213sd 0(%rip), %xmm10, %xmm6\n"
        "vfnmadd213sd -128(%rbx), %xmm8, %xmm16{%k1}{z}\n"
        "vfnmadd213sd %xmm24, %xmm15, %xmm28{%k7}{z}\n"
        "vfnmadd231ps 0x12345678(%r13), %xmm15, %xmm4\n"
        "vfnmadd231ps -64(%r13), %ymm0, %ymm6\n"
        "vfnmadd231ps %xmm8, %xmm14, %xmm20{%k3}\n"
        "vfnmadd231ps %ymm25, %ymm20, %ymm29\n"
        "vfnmadd231pd %ymm5, %ymm5, %ymm14\n"
        "vfnmadd231pd 0x12345678(%r14), %ymm4, %ymm10\n"
        "vfnmadd231pd 256(%rax), %ymm6, %ymm11\n"
        "vfnmadd231pd %zmm11, %zmm23, %zmm29{%k3}\n"
        "vfnmadd231ss %xmm8, %xmm4, %xmm3\n"
        "vfnmadd231ss 256(%r12,%r8,4), %xmm13, %xmm2\n"
        "vfnmadd231ss %xmm20, %xmm13, %xmm15{%k6}{z}\n"
        "vfnmadd231ss 8(%r10), %xmm20, %xmm13{%k7}\n"
        "vfnmadd231sd %xmm12, %xmm3, %xmm8\n"
        "vfnmadd231sd 64(%r9), %xmm9, %xmm6\n"
        "vfnmadd231sd 0x12345678(%r8), %xmm21, %xmm25\n"
        "vfnmadd231sd %xmm7, %xmm6, %xmm10\n"
        "vfnmsub132ps %ymm7, %ymm9, %ymm12\n"
        "vfnmsub132ps 0x1000(,%rsi,4), %ymm8, %ymm3\n"
        "vfnmsub132ps 0x1000(,%r12,4), %ymm26, %ymm25\n"
        "vfnmsub132ps %ymm21, %ymm15, %ymm7{%k5}\n"
        "vfnmsub132pd %ymm9, %ymm1, %ymm8\n"
        "vfnmsub132pd 8(%r14,%rax,4), %ymm13, %ymm1\n"
        "vfnmsub132pd 64(%rax,%r15,1), %xmm30, %xmm17{%k7}{z}\n"
        "vfnmsub132pd %ymm15, %ymm8, %ymm2\n"
        "vfnmsub132ss %xmm6, %xmm13, %xmm6\n"
        "vfnmsub132ss 0x10(,%r14,4), %xmm0, %xmm7\n"
        "vfnmsub132ss 8(%r13,%r10,4), %xmm25, %xmm28\n"
        "vfnmsub132ss 0(%rip), %xmm4, %xmm29\n"
        "vfnmsub132sd %xmm6, %xmm12, %xmm5\n"
        "vfnmsub132sd 256(%r8,%rax,8), %xmm5, %xmm11\n"
        "vfnmsub132sd %xmm6, %xmm27, %xmm7{%k1}{z}\n"
        "vfnmsub132sd %xmm20, %xmm26, %xmm1{%k5}{z}\n"
        "vfnmsub213ps %xmm0, %xmm3, %xmm15\n"
        "vfnmsub213ps %xmm1, %xmm11, %xmm14\n"
        "vfnmsub213ps -8(%rip), %ymm11, %ymm5\n"
        "vfnmsub213ps 8(%rdx,%r9,4), %zmm20, %zmm27\n"
        "vfnmsub213pd 0x1000(,%r15,8), %ymm12, %ymm3\n"
        "vfnmsub213pd %xmm10, %xmm2, %xmm10\n"
        "vfnmsub213pd 0x100(%rip), %ymm0, %ymm31\n"
        "vfnmsub213pd %xmm5, %xmm7, %xmm7{%k3}{z}\n"
        "vfnmsub213ss 64(%rax,%rcx,2), %xmm4, %xmm3\n"
        "vfnmsub213ss %xmm4, %xmm7, %xmm11\n"
        "vfnmsub213ss %xmm22, %xmm9, %xmm15{%k6}{z}\n"
        "vfnmsub213ss %xmm31, %xmm2, %xmm1{%k6}{z}\n"
        "vfnmsub213sd %xmm0, %xmm1, %xmm14\n"
        "vfnmsub213sd 0x100(%rip), %xmm5, %xmm8\n"
        "vfnmsub213sd (%rbx), %xmm5, %xmm5\n"
        "vfnmsub213sd 0x12345678(%r11), %xmm14, %xmm26\n"
        "vfnmsub231ps %ymm0, %ymm13, %ymm8\n"
        "vfnmsub231ps (%rcx), %ymm4, %ymm5\n"
        "vfnmsub231ps %ymm0, %ymm18, %ymm20{%k6}{z}\n"
        "vfnmsub231ps 0(%rip), %zmm5, %zmm12\n"
        "vfnmsub231pd 0x10(,%r9,8), %xmm1, %xmm15\n"
        "vfnmsub231pd 0x1000(,%r12,8), %ymm0, %ymm11\n"
        "vfnmsub231pd %xmm28, %xmm10, %xmm28{%k4}\n"
        "vfnmsub231pd %ymm29, %ymm12, %ymm24\n"
        "vfnmsub231ss %xmm3, %xmm9, %xmm12\n"
        "vfnmsub231ss 8(%rsi,%r15,8), %xmm14, %xmm6\n"
        "vfnmsub231ss 0x100(%rip), %xmm16, %xmm28\n"
        "vfnmsub231ss 0x1000(,%rax,2), %xmm20, %xmm7\n"
        "vfnmsub231sd %xmm12, %xmm7, %xmm10\n"
        "vfnmsub231sd %xmm11, %xmm15, %xmm10\n"
        "vfnmsub231sd %xmm7, %xmm30, %xmm26{%k4}{z}\n"
        "vfnmsub231sd 8(%r9,%rax,1), %xmm0, %xmm8\n"
        "vfmaddsub132ps 64(%rbp,%rax,1), %xmm12, %xmm14\n"
        "vfmaddsub132ps %xmm15, %xmm7, %xmm6\n"
        "vfmaddsub132ps -8(%rip), %ymm24, %ymm8{%k2}\n"
        "vfmaddsub132ps 0x10(,%r15,2), %xmm6, %xmm17{%k6}\n"
        "vfmaddsub132pd %xmm1, %xmm10, %xmm8\n"
        "vfmaddsub132pd 4096(%rax), %ymm0, %ymm3\n"
        "vfmaddsub132pd -8(%rip), %zmm25, %zmm7\n"
        "vfmaddsub132pd %ymm4, %ymm27, %ymm10{%k4}\n"
        "vfmaddsub213ps %ymm10, %ymm5, %ymm1\n"
        "vfmaddsub213ps %ymm4, %ymm12, %ymm14\n"
        "vfmaddsub213ps %zmm30, %zmm24, %zmm13\n"
        "vfmaddsub213ps (%r15), %ymm21, %ymm13{%k5}\n"
        "vfmaddsub213pd (%r14), %xmm10, %xmm7\n"
        "vfmaddsub213pd -8(%rip), %ymm7, %ymm10\n"
        "vfmaddsub213pd 4096(%rax,%rbp,2), %ymm8, %ymm18{%k4}{z}\n"
        "vfmaddsub213pd 0x10(,%r13,8), %ymm13, %ymm13{%k4}\n"
        "vfmaddsub231ps -8(%rip), %ymm10, %ymm2\n"
        "vfmaddsub231ps -64(%rcx,%rcx,1), %xmm4, %xmm7\n"
        "vfmaddsub231ps 0x1000(,%r13,4), %ymm23, %ymm21\n"
        "vfmaddsub231ps {rd-sae}, %zmm21, %zmm14, %zmm12{%k6}{z}\n"
        "vfmaddsub231pd -0x1000(%r11,%rbx,8), %xmm6, %xmm12\n"
        "vfmaddsub231pd %ymm14, %ymm5, %ymm12\n"
        "vfmaddsub231pd {rz-sae}, %zmm16, %zmm17, %zmm12{%k1}\n"
        "vfmsubadd132ps %ymm0, %ymm9, %ymm9\n"
        "vfmsubadd132ps %xmm1, %xmm8, %xmm4\n"
        "vfmsubadd132ps %ymm9, %ymm25, %ymm26\n"
        "vfmsubadd132ps %xmm13, %xmm14, %xmm21\n"
        "vfmsubadd132pd %ymm13, %ymm5, %ymm5\n"
        "vfmsubadd132pd -8(%rip), %ymm6, %ymm13\n"
        "vfmsubadd132pd 0x10(,%rdx,2), %zmm0, %zmm6\n"
        "vfmsubadd132pd 256(%r11,%rax,8), %xmm27, %xmm18{%k3}{z}\n"
        "vfmsubadd213ps %ymm9, %ymm5, %ymm10\n"
        "vfmsubadd213ps %xmm15, %xmm13, %xmm14\n"
        "vfmsubadd213ps 0x100(%rip), %ymm19, %ymm23{%k2}\n"
        "vfmsubadd213ps 4096(%rbp), %zmm2, %zmm19{%k4}{z}\n"
        "vfmsubadd213pd 0(%rip), %ymm2, %ymm15\n"
        "vfmsubadd213pd %ymm9, %ymm15, %ymm6\n"
        "vfmsubadd213pd (%rax){1to4}, %ymm9, %ymm7\n"
        "vfmsubadd213pd %xmm29, %xmm18, %xmm1{%k6}\n"
        "vfmsubadd231ps %xmm4, %xmm2, %xmm12\n"
        "vfmsubadd231ps %ymm5, %ymm8, %ymm8\n"
        "vfmsubadd231ps -64(%rdx), %xmm13, %xmm28{%k7}{z}\n"
        "vfmsubadd231pd %ymm0, %ymm10, %ymm9\n"
        "vfmsubadd231pd %ymm6, %ymm6, %ymm5\n"
        "vfmsubadd231pd 4096(%rbp,%rcx,2), %zmm23, %zmm31\n"
        "vfmsubadd231pd %ymm23, %ymm16, %ymm13\n"
        "vcvtph2ps 0(%rip), %ymm11\n"
        "vcvtph2ps 0x100(%rip), %ymm15\n"
        "vcvtps2ph $89, %xmm13, 0x10(,%rbx,8)\n"
        "vcvtps2ph $172, %xmm3, -8(%rip)\n"
        "vcvtph2ps {sae}, %ymm14, %zmm3{%k5}{z}\n"
        "vcvtph2ps %xmm13, %ymm5\n"
        "vcvtps2ph $68, %zmm27, 0x12345678(%rcx){%k3}\n"
        "vcvtps2ph $203, %ymm20, -128(%rax){%k7}\n"
        "vcvtdq2ps %ymm14, %ymm1\n"
        "vcvtdq2ps %ymm10, %ymm7\n"
        "vcvtdq2ps %xmm23, %xmm8{%k6}{z}\n"
        "vcvtdq2ps 0(%rip), %ymm17{%k6}\n"
        "vcvtps2dq %xmm0, %xmm15\n"
        "vcvtps2dq 4096(%rdx,%r10,4), %xmm14\n"
        "vcvtps2dq %zmm7, %zmm22{%k6}\n"
        "vcvtps2dq {rn-sae}, %zmm6, %zmm29\n"
        "vcvttps2dq %xmm1, %xmm4\n"
        "vcvttps2dq -8(%rip), %ymm9\n"
        "vcvttps2dq %ymm0, %ymm15{%k5}{z}\n"
        "vcvttps2dq %zmm2, %zmm10\n"
        "vcvtpd2ps %xmm9, %xmm12\n"
        "vcvtpd2ps %xmm3, %xmm7\n"
        "vcvtpd2ps %ymm11, %xmm12\n"
        "vcvtpd2ps %ymm11, %xmm13\n"
        "vcvtpd2psx %xmm2, %xmm5\n"
        "vcvtpd2psx 8(%r15), %xmm6\n"
        "vcvtpd2psy 0x100(%rip), %xmm15\n"
        "vcvtpd2psy 0x1000(,%r13,4), %xmm5\n"
        "vcvtpd2ps %zmm4, %ymm9\n"
        "vcvtpd2ps 0x100(%rip), %ymm24{%k6}\n"
        "vcvtpd2psx %xmm3, %xmm14\n"
        "vcvtpd2psx %xmm0, %xmm13\n"
        "vcvtpd2psy 0(%rip), %xmm18{%k2}\n"
        "vcvtpd2psy 128(%r9), %xmm14{%k1}{z}\n"
        "vcvtpd2ps %xmm25, %xmm3\n"
        "vcvtpd2ps %xmm18, %xmm2\n"
        "vcvtpd2ps %ymm18, %xmm0\n"
        "vcvtpd2ps %ymm19, %xmm18\n"
        "vcvtps2pd %xmm10, %ymm2\n"
        "vcvtps2pd %xmm7, %ymm4\n"
        "vcvtps2pd %ymm6, %zmm11\n"
        "vcvtps2pd 128(%rcx,%rdx,4), %zmm10\n"
        "vcvtsd2ss -128(%rsi,%rbp,8), %xmm6, %xmm2\n"
        "vcvtsd2ss -8(%rip), %xmm3, %xmm11\n"
        "vcvtss2sd 0x10(,%rax,4), %xmm15, %xmm5\n"
        "vcvtss2sd -64(%rbx,%r11,4), %xmm7, %xmm5\n"
        "vcvtsd2ss -64(%r9), %xmm8, %xmm29{%k5}{z}\n"
        "vcvtsd2ss {rd-sae}, %xmm8, %xmm25, %xmm18{%k6}\n"
        "vcvtss2sd 128(%rbx,%rax,2), %xmm1, %xmm9\n"
        "vcvtss2sd 0(%rip), %xmm20, %xmm11{%k7}{z}\n"
        "vcvtsi2ss %eax, %xmm10, %xmm10\n"
        "vcvtsi2ss 64(%r12), %xmm3, %xmm10\n"
        "vcvtsi2sd -8(%rip), %xmm1, %xmm1\n"
        "vcvtsi2sd %r11, %xmm13, %xmm7\n"
        "vcvtss2si %xmm2, %r8\n"
        "vcvtss2si 0(%rip), %r8\n"
        "vcvtsd2si -0x1000(%r10), %r15\n"
        "vcvtsd2si 0x1000(,%r10,4), %ebx\n"
        "vcvttss2si -0x1000(%r10), %r15\n"
        "vcvttss2si %xmm6, %r13\n"
        "vcvttsd2si %xmm0, %rbx\n"
        "vcvttsd2si %xmm14, %r11d\n"
        "vmovss %xmm3, %xmm3, %xmm12\n"
        "vmovss %xmm10, %xmm14, %xmm6\n"
        "vmovss %xmm4, %xmm3, %xmm3\n"
        "vmovss %xmm3, %xmm2, %xmm1\n"
        "vmovss 0(%rip), %xmm2\n"
        "vmovss 0(%rip), %xmm7\n"
        "vmovss %xmm15, -8(%rip)\n"
        "vmovss %xmm5, 0x10(,%rdi,8)\n"
        "vmovsd %xmm7, %xmm14, %xmm4\n"
        "vmovsd %xmm12, %xmm10, %xmm4\n"
        "vmovsd %xmm6, %xmm8, %xmm10\n"
        "vmovsd %xmm4, %xmm13, %xmm12\n"
        "vmovsd 0x1000(,%r8,4), %xmm11\n"
        "vmovsd -8(%rip), %xmm6\n"
        "vmovsd %xmm14, 0x10(,%r12,8)\n"
        "vmovsd %xmm6, -8(%rip)\n"
        "vmovq %xmm5, %xmm2\n"
        "vmovq %xmm2, %xmm8\n"
        "vmovq %xmm4, %xmm13\n"
        "vmovq %xmm10, %xmm10\n"
        "vmovd %r11d, %xmm15\n"
        "vmovd %r14d, %xmm14\n"
        "vmovd %xmm9, %ebp\n"
        "vmovd %xmm9, %ebp\n"
        "vmovq 0x1000(,%rbp,2), %xmm1\n"
        "vmovq %rbx, %xmm2\n"
        "vmovq %xmm14, %rdx\n"
        "vmovq %xmm13, 0x1000(,%r14,4)\n"
        "vmovq (%r8), %xmm12\n"
        "vmovq 4096(%rcx), %xmm4\n"
        "vmovq %xmm7, %xmm3\n"
        "vmovq %xmm12, %xmm2\n"
        "vmovd -8(%rip), %xmm19\n"
        "vmovd 0x12345678(%r15,%rbx,1), %xmm20\n"
        "vmovd %xmm18, %ebp\n"
        "vmovd %xmm31, 0x1000(,%rbp,4)\n"
        "vmovq 0x1000(,%r14,2), %xmm23\n"
        "vmovq -8(%rip), %xmm16\n"
        "vmovq %xmm31, -128(%r12)\n"
        "vmovq %xmm20, %r12\n"
        "vpmovmskb %ymm10, %rbx\n"
        "vpmovmskb %ymm3, %r10\n"
        "vmovmskps %xmm6, %r14d\n"
        "vmovmskps %ymm9, %eax\n"
        "vmovmskpd %xmm0, %r11d\n"
        "vmovmskpd %xmm7, %r11\n"
        "vpshufd $235, %xmm9, %xmm2\n"
        "vpshufd $12, %xmm3, %xmm12\n"
        "vpshufd $247, 4096(%r10), %xmm4\n"
        "vpshufd $243, %ymm14, %ymm4\n"
        "vpshuflw $97, %ymm11, %ymm11\n"
        "vpshuflw $221, 128(%r10,%rax,1), %ymm6\n"
        "vpshufhw $75, %xmm14, %xmm11\n"
        "vpshufhw $47, %ymm13, %ymm14\n"
        "vpshufb %ymm2, %ymm4, %ymm0\n"
        "vpshufb 0x10(,%r11,2), %ymm11, %ymm0\n"
        "vpshufb 0x12345678(%r13,%r15,2), %zmm8, %zmm4\n"
        "vpshufb -8(%rip), %zmm17, %zmm0{%k3}{z}\n"
        "vpsrlw $213, %xmm6, %xmm12\n"
        "vpsrlw $61, %ymm11, %ymm13\n"
        "vpsrlw $109, 0x10(,%r15,2), %xmm17\n"
        "vpsrlw $57, %ymm25, %ymm10{%k4}{z}\n"
        "vpsraw $31, %ymm4, %ymm12\n"
        "vpsraw $186, %ymm5, %ymm11\n"
        "vpsraw $107, %xmm18, %xmm2{%k7}\n"
        "vpsraw $61, 0(%rip), %ymm21{%k3}\n"
        "vpsllw $137, %ymm5, %ymm7\n"
        "vpsllw $179, %ymm4, %ymm7\n"
        "vpsllw $48, 0x100(%rip), %xmm29\n"
        "vpsllw $169, -8(%rip), %ymm5{%k4}{z}\n"
        "vpsrld $59, %ymm4, %ymm12\n"
        "vpsrld $34, %ymm15, %ymm12\n"
        "vpsrld $149, -8(%rip){1to16}, %zmm28\n"
        "vpsrld $60, 0(%rip), %ymm18{%k1}{z}\n"
        "vpsrad $82, %xmm11, %xmm9\n"
        "vpsrad $138, %ymm15, %ymm11\n"
        "vpsrad $56, -128(%rcx), %ymm27{%k7}{z}\n"
        "vpsrad $90, -0x1000(%r12,%r12,4){1to16}, %zmm14{%k4}\n"
        "vpslld $244, %xmm2, %xmm3\n"
        "vpslld $192, %ymm5, %ymm13\n"
        "vpslld $180, -8(%rip), %ymm31\n"
        "vpslld $70, 0x10(,%r13,8), %zmm7\n"
        "vpsrlq $193, %xmm1, %xmm15\n"
        "vpsrlq $61, %xmm12, %xmm15\n"
        "vpsrlq $110, %ymm29, %ymm3\n"
        "vpsrlq $149, 0x1000(,%r12,8){1to2}, %xmm19{%k6}{z}\n"
        "vpsllq $145, %xmm12, %xmm11\n"
        "vpsllq $132, %ymm14, %ymm15\n"
        "vpsllq $249, %zmm14, %zmm14\n"
        "vpsllq $12, %ymm18, %ymm23{%k3}{z}\n"
        "vpsrldq $29, %ymm9, %ymm8\n"
        "vpsrldq $182, %xmm15, %xmm2\n"
        "vpsrldq $20, %xmm16, %xmm8\n"
        "vpsrldq $86, -0x1000(%rax), %xmm11\n"
        "vpslldq $121, %xmm4, %xmm8\n"
        "vpslldq $50, %ymm9, %ymm1\n"
        "vpslldq $185, %xmm25, %xmm7\n"
        "vpslldq $91, %xmm23, %xmm11\n"
        "vpsraq $192, 0x10(,%r14,4){1to4}, %ymm20{%k2}{z}\n"
        "vpsraq $110, %zmm19, %zmm12\n"
        "vpsrlw -64(%r9,%r8,4), %ymm12, %ymm1\n"
        "vpsrlw %xmm15, %ymm12, %ymm1\n"
        "vpsrld %xmm0, %xmm0, %xmm6\n"
        "vpsrld %xmm12, %ymm3, %ymm8\n"
        "vpsrlq %xmm9, %xmm15, %xmm0\n"
        "vpsrlq 0(%rip), %ymm7, %ymm15\n"
        "vpsraw %xmm6, %ymm15, %ymm8\n"
        "vpsraw 0(%rip), %ymm0, %ymm10\n"
        "vpsrad 64(%r15), %ymm5, %ymm14\n"
        "vpsrad %xmm6, %xmm13, %xmm5\n"
        "vpsllw %xmm3, %xmm9, %xmm11\n"
        "vpsllw %xmm10, %xmm14, %xmm5\n"
        "vpslld 0x10(,%r10,4), %xmm10, %xmm1\n"
        "vpslld 0x100(%rip), %ymm15, %ymm4\n"
        "vpsllq %xmm0, %ymm2, %ymm4\n"
        "vpsllq %xmm0, %xmm6, %xmm8\n"
        "vpinsrb $80, %esi, %xmm14, %xmm14\n"
        "vpinsrb $177, %r8d, %xmm4, %xmm3\n"
        "vpinsrw $254, %r11d, %xmm15, %xmm0\n"
        "vpinsrw $0, %r9d, %xmm11, %xmm5\n"
        "vpinsrd $82, 0x100(%rip), %xmm3, %xmm8\n"
        "vpinsrd $11, %r14d, %xmm11, %xmm12\n"
        "vpinsrq $218, 4096(%rdx,%r8,8), %xmm8, %xmm2\n"
        "vpinsrq $149, 0x100(%rip), %xmm6, %xmm1\n"
        "vpextrb $53, %xmm5, (%rcx)\n"
        "vpextrb $185, %xmm3, -128(%rbp)\n"
        "vpextrw $114, %xmm1, %edx\n"
        "vpextrw $71, %xmm3, %ebx\n"
        "vpextrd $98, %xmm0, -128(%rsi)\n"
        "vpextrd $75, %xmm3, %ebx\n"
        "vpextrq $13, %xmm7, 128(%r14)\n"
        "vpextrq $90, %xmm5, %r15\n"
        "vinsertps $185, (%rbx), %xmm4, %xmm14\n"
        "vinsertps $68, %xmm1, %xmm11, %xmm10\n"
        "vextractps $218, %xmm7, 8(%rsi,%r9,4)\n"
        "vextractps $10, %xmm12, %esi\n"
        "vblendps $130, %xmm0, %xmm1, %xmm13\n"
        "vblendps $98, %ymm15, %ymm13, %ymm11\n"
        "vblendpd $11, -128(%rsi), %xmm3, %xmm11\n"
        "vblendpd $215, %xmm5, %xmm8, %xmm1\n"
        "vpblendw $188, 8(%r9), %xmm0, %xmm10\n"
        "vpblendw $199, %xmm12, %xmm0, %xmm11\n"
        "vpblendd $45, -64(%rax), %xmm3, %xmm0\n"
        "vpblendd $34, 128(%rcx,%rbp,2), %xmm7, %xmm15\n"
        "vblendvps %ymm14, %ymm7, %ymm13, %ymm3\n"
        "vblendvps %xmm11, %xmm2, %xmm15, %xmm12\n"
        "vblendvpd %xmm10, %xmm1, %xmm3, %xmm6\n"
        "vblendvpd %ymm3, 0(%rip), %ymm11, %ymm0\n"
        "vpblendvb %ymm3, %ymm14, %ymm12, %ymm0\n"
        "vpblendvb %xmm3, %xmm4, %xmm5, %xmm8\n"
        "vdpps $202, 0(%rip), %xmm13, %xmm6\n"
        "vdpps $233, 0(%rip), %xmm5, %xmm7\n"
        "vdppd $222, 0x1000(,%rax,8), %xmm5, %xmm7\n"
        "vdppd $50, %xmm0, %xmm2, %xmm0\n"
        "vpermq $27, %ymm13, %ymm7\n"
        "vpermq $78, 0x12345678(%r13,%rdi,8), %ymm5\n"
        "vpermpd $105, 4096(%r8,%rsi,1), %ymm11\n"
        "vpermpd $187, %ymm3, %ymm9\n"
        "vperm2f128 $102, 0x10(,%rdi,8), %ymm8, %ymm6\n"
        "vperm2f128 $237, %ymm6, %ymm8, %ymm9\n"
        "vperm2i128 $61, %ymm0, %ymm10, %ymm8\n"
        "vperm2i128 $206, %ymm0, %ymm9, %ymm1\n"
        "vpermd %ymm3, %ymm6, %ymm13\n"
        "vpermd %ymm1, %ymm6, %ymm2\n"
        "vpermps -8(%rip), %ymm8, %ymm15\n"
        "vpermps %ymm10, %ymm8, %ymm11\n"
        "vpermilps 256(%rsi,%r8,4), %ymm12, %ymm0\n"
        "vpermilps %xmm14, %xmm5, %xmm5\n"
        "vpermilpd %xmm9, %xmm6, %xmm0\n"
        "vpermilpd 0x1000(,%rax,4), %xmm10, %xmm1\n"
        "vpermilps $121, %xmm10, %xmm8\n"
        "vpermilps $114, -128(%r14,%r8,8), %xmm3\n"
        "vpermilpd $185, %xmm1, %xmm14\n"
        "vpermilpd $40, %xmm0, %xmm4\n"
        "vinsertf128 $159, 0x10(,%r9,4), %ymm2, %ymm0\n"
        "vinsertf128 $34, -64(%r12), %ymm14, %ymm13\n"
        "vextractf128 $97, %ymm8, %xmm13\n"
        "vextractf128 $188, %ymm3, -64(%rbx)\n"
        "vinserti128 $196, %xmm11, %ymm6, %ymm10\n"
        "vinserti128 $232, (%r8,%r8,1), %ymm1, %ymm15\n"
        "vextracti128 $169, %ymm3, %xmm15\n"
        "vextracti128 $197, %ymm7, 0x10(,%r14,8)\n"
        "vinsertf32x4 $94, %xmm18, %ymm28, %ymm10\n"
        "vinsertf32x4 $214, (%rsi,%r13,4), %zmm1, %zmm10\n"
        "vinsertf64x2 $236, %xmm28, %ymm27, %ymm3\n"
        "vinsertf64x2 $13, -64(%rdx,%r12,1), %zmm10, %zmm2\n"
        "vinserti32x4 $109, %xmm16, %ymm6, %ymm10\n"
        "vinserti32x4 $22, 256(%r15), %ymm18, %ymm24\n"
        "vinserti64x2 $27, 0x10(,%rbx,8), %ymm24, %ymm13{%k7}{z}\n"
        "vinserti64x2 $196, %xmm9, %zmm9, %zmm10\n"
        "vinsertf32x8 $247, 0x10(,%r14,8), %zmm3, %zmm23\n"
        "vinsertf32x8 $106, %ymm30, %zmm11, %zmm6\n"
        "vinsertf64x4 $154, 0x100(%rip), %zmm18, %zmm21\n"
        "vinsertf64x4 $69, 0x10(,%r14,8), %zmm13, %zmm31{%k6}\n"
        "vinserti32x8 $115, %ymm0, %zmm27, %zmm29{%k1}{z}\n"
        "vinserti32x8 $135, %ymm4, %zmm9, %zmm4{%k2}{z}\n"
        "vinserti64x4 $215, 0x10(,%r11,4), %zmm0, %zmm29\n"
        "vinserti64x4 $246, %ymm8, %zmm26, %zmm25\n"
        "vextractf32x4 $36, %zmm25, %xmm28\n"
        "vextractf32x4 $215, %zmm0, 0x100(%rip)\n"
        "vextractf64x2 $100, %ymm29, 8(%rdi)\n"
        "vextractf64x2 $196, %zmm1, %xmm1{%k3}{z}\n"
        "vextracti32x4 $187, %zmm19, %xmm29{%k5}{z}\n"
        "vextracti32x4 $249, %zmm23, %xmm9\n"
        "vextracti64x2 $240, %ymm0, (%rax,%r12,4){%k6}\n"
        "vextracti64x2 $127, %ymm22, 0x100(%rip)\n"
        "vextractf32x8 $91, %zmm25, %ymm15{%k1}\n"
        "vextractf32x8 $152, %zmm10, 8(%r9,%rsi,2)\n"
        "vextractf64x4 $27, %zmm14, 64(%r13)\n"
        "vextractf64x4 $229, %zmm5, %ymm15\n"
        "vextracti32x8 $53, %zmm19, 0x10(,%r12,8){%k6}\n"
        "vextracti32x8 $88, %zmm11, %ymm27{%k7}{z}\n"
        "vextracti64x4 $89, %zmm3, 128(%rbx)\n"
        "vextracti64x4 $168, %zmm23, %ymm31\n"
        "vpternlogd $1, 0x10(,%rax,4){1to16}, %zmm7, %zmm11{%k4}\n"
        "vpternlogd $41, 8(%r14,%r15,8), %zmm31, %zmm15{%k3}{z}\n"
        "vpternlogq $235, %ymm6, %ymm26, %ymm12\n"
        "vpternlogq $38, 0(%rip), %xmm13, %xmm21{%k2}\n"
        "vpcmpd $151, (%r8,%rbp,2){1to8}, %ymm21, %k7\n"
        "vpcmpd $232, %ymm7, %ymm12, %k3\n"
        "vpcmpud $56, %zmm0, %zmm3, %k1\n"
        "vpcmpud $43, -64(%r10), %xmm23, %k1\n"
        "vpcmpq $13, 4096(%r11), %zmm3, %k1\n"
        "vpcmpq $65, 4096(%r15), %xmm17, %k2\n"
        "vpcmpuq $241, 0x10(,%r14,4){1to2}, %xmm25, %k2\n"
        "vpcmpuq $172, %ymm4, %ymm22, %k5\n"
        "vpcmpb $1, 0x10(,%rbx,8), %zmm0, %k2\n"
        "vpcmpb $23, 0x1000(,%rcx,2), %xmm31, %k2\n"
        "vpcmpw $118, (%rax), %zmm17, %k6\n"
        "vpcmpw $134, -8(%rip), %zmm9, %k2\n"
        "vpcmpub $216, %xmm0, %xmm2, %k6\n"
        "vpcmpub $30, 0(%rip), %zmm1, %k2\n"
        "vpcmpuw $172, 0x100(%rip), %xmm9, %k7\n"
        "vpcmpuw $28, 64(%r12,%r14,1), %xmm31, %k6\n"
        "kmovb %k6, %k3\n"
        "kmovb 0x100(%rip), %k3\n"
        "kmovb %k5, %k4\n"
        "kmovb %k4, 0x100(%rip)\n"
        "kmovw 64(%rcx,%rax,2), %k7\n"
        "kmovw -64(%rsi), %k1\n"
        "kmovw %k5, %k7\n"
        "kmovw %k3, %k0\n"
        "kmovd %k5, %k1\n"
        "kmovd -64(%rdx,%rax,2), %k3\n"
        "kmovd %k4, %k5\n"
        "kmovd %k0, 0x1000(,%r8,2)\n"
        "kmovq 128(%rdi,%r15,2), %k2\n"
        "kmovq %k2, %k0\n"
        "kmovq %k4, %k6\n"
        "kmovq %k7, %k6\n"
        "kmovb %r11d, %k3\n"
        "kmovb %r11d, %k1\n"
        "kmovw %r14d, %k3\n"
        "kmovw %eax, %k1\n"
        "kmovd %r8d, %k2\n"
        "kmovd %r9d, %k2\n"
        "kmovq %r8, %k1\n"
        "kmovq %r13, %k4\n"
        "kmovb %k5, %ecx\n"
        "kmovb %k6, %ecx\n"
        "kmovw %k2, %ebp\n"
        "kmovw %k5, %esi\n"
        "kmovd %k7, %esi\n"
        "kmovd %k6, %esi\n"
        "kmovq %k4, %rcx\n"
        "kmovq %k1, %r13\n"
        "kandb %k2, %k0, %k6\n"
        "kandb %k1, %k7, %k0\n"
        "kandnb %k5, %k3, %k7\n"
        "kandnb %k6, %k0, %k5\n"
        "korb %k6, %k6, %k2\n"
        "korb %k5, %k5, %k6\n"
        "kxnorb %k5, %k7, %k0\n"
        "kxnorb %k7, %k5, %k0\n"
        "kxorb %k7, %k5, %k3\n"
        "kxorb %k5, %k5, %k7\n"
        "knotb %k0, %k7\n"
        "knotb %k3, %k6\n"
        "kaddb %k3, %k3, %k2\n"
        "kaddb %k7, %k0, %k7\n"
        "kortestb %k7, %k6\n"
        "kortestb %k2, %k6\n"
        "ktestb %k7, %k7\n"
        "ktestb %k2, %k7\n"
        "kandw %k5, %k6, %k1\n"
        "kandw %k7, %k3, %k0\n"
        "kandnw %k5, %k1, %k7\n"
        "kandnw %k4, %k7, %k5\n"
        "korw %k2, %k3, %k2\n"
        "korw %k2, %k5, %k6\n"
        "kxnorw %k6, %k4, %k1\n"
        "kxnorw %k0, %k5, %k0\n"
        "kxorw %k1, %k5, %k0\n"
        "kxorw %k1, %k3, %k3\n"
        "knotw %k4, %k2\n"
        "knotw %k5, %k2\n"
        "kaddw %k6, %k0, %k0\n"
        "kaddw %k3, %k5, %k4\n"
        "kortestw %k2, %k3\n"
        "kortestw %k1, %k0\n"
        "ktestw %k7, %k6\n"
        "ktestw %k5, %k2\n"
        "kandd %k3, %k0, %k2\n"
        "kandd %k3, %k0, %k4\n"
        "kandnd %k3, %k0, %k4\n"
        "kandnd %k6, %k7, %k2\n"
        "kord %k4, %k5, %k1\n"
        "kord %k3, %k6, %k7\n"
        "kxnord %k3, %k4, %k7\n"
        "kxnord %k7, %k2, %k6\n"
        "kxord %k6, %k7, %k4\n"
        "kxord %k2, %k1, %k0\n"
        "knotd %k7, %k3\n"
        "knotd %k3, %k4\n"
        "kaddd %k2, %k0, %k1\n"
        "kaddd %k7, %k4, %k3\n"
        "kortestd %k6, %k2\n"
        "kortestd %k6, %k7\n"
        "ktestd %k4, %k4\n"
        "ktestd %k4, %k6\n"
        "kandq %k7, %k4, %k6\n"
        "kandq %k4, %k2, %k6\n"
        "kandnq %k6, %k3, %k4\n"
        "kandnq %k6, %k0, %k4\n"
        "korq %k0, %k4, %k0\n"
        "korq %k6, %k5, %k6\n"
        "kxnorq %k2, %k6, %k5\n"
        "kxnorq %k2, %k5, %k5\n"
        "kxorq %k1, %k1, %k2\n"
        "kxorq %k1, %k3, %k0\n"
        "knotq %k0, %k1\n"
        "knotq %k4, %k6\n"
        "kaddq %k5, %k2, %k1\n"
        "kaddq %k7, %k6, %k2\n"
        "kortestq %k2, %k7\n"
        "kortestq %k4, %k6\n"
        "ktestq %k2, %k0\n"
        "ktestq %k2, %k7\n"
        "cli\n"
        "sti\n"
        "hlt\n"
        "swapgs\n"
        "iret\n"
        "iretq\n"
        "iretl\n"
        "iretw\n"
        "rsm\n"
        "wrmsr\n"
        "rdmsr\n"
        "wbinvd\n"
        "invd\n"
        "monitor\n"
        "mwait\n"
        "clts\n"
        "sysret\n"
        "sysretq\n"
        "sysenter\n"
        "sysexit\n"
        "invlpg 0x100(%rip)\n"
        "invlpg 4096(%rdi,%r14,4)\n"
        "lgdt 0x12345678(%r10,%rbp,4)\n"
        "lgdt 0x10(,%r12,8)\n"
        "lidt 0x1000(,%r8,4)\n"
        "lidt 128(%rax,%r14,4)\n"
        "sgdt 0x10(,%rdx,4)\n"
        "sgdt 0x1000(,%rdi,4)\n"
        "sidt 0x10(,%rdx,8)\n"
        "sidt 0(%rip)\n"
        "lmsw 0x1000(,%r9,8)\n"
        "lmsw 0x1000(,%rdx,2)\n"
        "smsw 0x10(,%rdi,8)\n"
        "smsw 256(%rdx)\n"
        "lldt %dx\n"
        "lldt %r8w\n"
        "ltr %edi\n"
        "ltr %rsi\n"
        "verr -128(%r11,%r13,2)\n"
        "verr %di\n"
        "verw %r11d\n"
        "verw %r15\n"
        "sldt 0(%rip)\n"
        "sldt %r11d\n"
        "str %edi\n"
        "str %edi\n"
        "smsw %r12\n"
        "smsw %edi\n"
        "lmsw %r8w\n"
        "lmsw %dx\n"
        "mov %cr8, %r12\n"
        "mov %cr4, %r14\n"
        "mov %rbp, %cr0\n"
        "mov %rdx, %cr0\n"
        "mov %db6, %r12\n"
        "mov %db6, %rdx\n"
        "mov %rax, %db7\n"
        "mov %r8, %db3\n"
        "endbr32\n"
        "cldemote 0x10(,%rax,2)\n"
        "cldemote 8(%r9)\n"
        "clrssbsy 0x10(,%rsi,4)\n"
        "clrssbsy 0x10(,%r15,8)\n"
        "wrssd %esi, 256(%r10)\n"
        "wrssd %r14d, 0(%rip)\n"
        "wrssq %r15, 4096(%rbp)\n"
        "wrssq %rbx, 0x100(%rip)\n"
        "wrussd %edx, 256(%r11,%r9,8)\n"
        "wrussd %ecx, 0x1000(,%r12,4)\n"
        "wrussq %r13, 256(%rcx,%rax,4)\n"
        "wrussq %r11, -128(%r10)\n"
        "rstorssp 4096(%r11)\n"
        "rstorssp -8(%rip)\n"
        "setssbsy\n"
        "saveprevssp\n"
        "pcmpestrm $45, %xmm13, %xmm8\n"
        "pcmpestrm $12, 0x12345678(%rcx,%rdx,2), %xmm12\n"
        "vpcmpestrm $135, %xmm0, %xmm15\n"
        "vpcmpestrm $80, 4096(%rcx,%rdx,1), %xmm0\n"
        "pcmpestri $147, %xmm11, %xmm4\n"
        "pcmpestri $63, 0x1000(,%rax,2), %xmm2\n"
        "vpcmpestri $221, %xmm6, %xmm1\n"
        "vpcmpestri $220, 0x1000(,%r9,4), %xmm13\n"
        "pcmpistrm $89, %xmm4, %xmm4\n"
        "pcmpistrm $106, 128(%rdi), %xmm13\n"
        "vpcmpistrm $23, %xmm4, %xmm11\n"
        "vpcmpistrm $0, -0x1000(%r13,%r10,1), %xmm12\n"
        "pcmpistri $73, %xmm11, %xmm9\n"
        "pcmpistri $133, %xmm5, %xmm10\n"
        "vpcmpistri $51, 0x1000(,%r14,4), %xmm2\n"
        "vpcmpistri $205, %xmm9, %xmm4\n"
        "sha1nexte (%rbp), %xmm12\n"
        "sha1nexte 0x1000(,%rsi,8), %xmm9\n"
        "sha1msg1 %xmm2, %xmm2\n"
        "sha1msg1 256(%r8,%r9,1), %xmm13\n"
        "sha1msg2 %xmm12, %xmm11\n"
        "sha1msg2 -128(%rbx,%rbp,2), %xmm6\n"
        "sha256msg1 0(%rip), %xmm8\n"
        "sha256msg1 0x12345678(%rbx,%r15,2), %xmm0\n"
        "sha256msg2 -8(%rip), %xmm5\n"
        "sha256msg2 0x10(,%r9,8), %xmm6\n"
        "sha1rnds4 $58, 256(%rbp,%rbx,4), %xmm9\n"
        "sha1rnds4 $45, %xmm5, %xmm10\n"
        "sha256rnds2 %xmm0, 4096(%rsi,%r11,8), %xmm8\n"
        "sha256rnds2 %xmm0, %xmm3, %xmm14\n"
        "sha256rnds2 %xmm4, %xmm5\n"
        "sha256rnds2 %xmm5, %xmm6\n"
        "gf2p8mulb %xmm13, %xmm13\n"
        "gf2p8mulb 64(%rbp), %xmm8\n"
        "gf2p8affineqb $100, -128(%rax,%r12,1), %xmm5\n"
        "gf2p8affineqb $163, %xmm0, %xmm3\n"
        "gf2p8affineinvqb $232, 0(%rip), %xmm11\n"
        "gf2p8affineinvqb $56, %xmm5, %xmm12\n"
        "vgf2p8mulb 0x10(,%r14,8), %ymm9, %ymm0\n"
        "vgf2p8mulb 8(%rdx), %xmm1, %xmm14\n"
        "vgf2p8mulb %zmm16, %zmm23, %zmm27{%k2}{z}\n"
        "vgf2p8mulb %xmm23, %xmm11, %xmm15{%k3}\n"
        "vgf2p8affineqb $5, -8(%rip), %ymm5, %ymm0\n"
        "vgf2p8affineqb $114, %xmm4, %xmm6, %xmm2\n"
        "vgf2p8affineqb $92, 256(%r15,%r10,1){1to4}, %ymm2, %ymm11{%k3}\n"
        "vgf2p8affineqb $76, %zmm20, %zmm11, %zmm6{%k6}\n"
        "vgf2p8affineinvqb $184, %ymm12, %ymm7, %ymm2\n"
        "vgf2p8affineinvqb $46, 0x100(%rip), %xmm13, %xmm15\n"
        "vgf2p8affineinvqb $131, 0x1000(,%rbp,4), %xmm3, %xmm0{%k2}{z}\n"
        "vgf2p8affineinvqb $72, 0x12345678(%rcx), %ymm4, %ymm31{%k7}\n"
        "vaesenc -8(%rip), %ymm15, %ymm11\n"
        "vaesenc %xmm1, %xmm2, %xmm11\n"
        "vaesenc 0x1000(,%rcx,2), %xmm29, %xmm21\n"
        "vaesenc 0x12345678(%r15), %xmm27, %xmm11\n"
        "vaesenclast %xmm14, %xmm6, %xmm8\n"
        "vaesenclast %ymm4, %ymm7, %ymm7\n"
        "vaesenclast 8(%rsi), %ymm11, %ymm21\n"
        "vaesenclast 128(%r12,%r13,8), %zmm16, %zmm1\n"
        "vaesdec -8(%rip), %ymm10, %ymm5\n"
        "vaesdec %xmm4, %xmm2, %xmm4\n"
        "vaesdec %zmm5, %zmm25, %zmm6\n"
        "vaesdec %xmm13, %xmm27, %xmm28\n"
        "vaesdeclast (%rbp), %ymm9, %ymm2\n"
        "vaesdeclast 0x10(,%r13,4), %xmm3, %xmm1\n"
        "vaesdeclast %xmm16, %xmm7, %xmm14\n"
        "vaesdeclast %zmm12, %zmm0, %zmm17\n"
        "vaesimc 128(%r11), %xmm12\n"
        "vaesimc -0x1000(%r8), %xmm8\n"
        "vaeskeygenassist $249, %xmm10, %xmm11\n"
        "vaeskeygenassist $45, 4096(%rdx,%rax,4), %xmm11\n"
        "vpclmulqdq $117, %ymm2, %ymm3, %ymm9\n"
        "vpclmulqdq $224, %xmm1, %xmm12, %xmm0\n"
        "vpclmulqdq $44, %ymm9, %ymm16, %ymm19\n"
        "vpclmulqdq $57, %ymm3, %ymm0, %ymm9\n"
        "pclmullqlqdq %xmm3, %xmm11\n"
        "pclmullqlqdq %xmm7, %xmm2\n"
        "vpclmullqlqdq 8(%rbp,%r12,8), %xmm4, %xmm9\n"
        "vpclmullqlqdq %xmm8, %xmm3, %xmm2\n"
        "vpclmullqlqdq 0x12345678(%r10,%rbx,1), %xmm15, %xmm13\n"
        "vpclmullqlqdq %ymm1, %ymm5, %ymm6\n"
        "pclmulhqlqdq 0x100(%rip), %xmm13\n"
        "pclmulhqlqdq 256(%r12,%rsi,2), %xmm7\n"
        "vpclmulhqlqdq 0x1000(,%rbx,4), %xmm2, %xmm8\n"
        "vpclmulhqlqdq %ymm12, %ymm8, %ymm4\n"
        "vpclmulhqlqdq %xmm10, %xmm20, %xmm22\n"
        "vpclmulhqlqdq 0x10(,%r13,8), %zmm18, %zmm26\n"
        "pclmullqhqdq %xmm8, %xmm3\n"
        "pclmullqhqdq 0x1000(,%rsi,2), %xmm12\n"
        "vpclmullqhqdq 64(%rdx), %ymm11, %ymm6\n"
        "vpclmullqhqdq -8(%rip), %ymm2, %ymm13\n"
        "vpclmullqhqdq 128(%rbp), %ymm2, %ymm29\n"
        "vpclmullqhqdq 0x1000(,%rbx,2), %zmm17, %zmm15\n"
        "pclmulhqhqdq (%r13), %xmm2\n"
        "pclmulhqhqdq -128(%r9,%r15,8), %xmm14\n"
        "vpclmulhqhqdq %ymm2, %ymm7, %ymm2\n"
        "vpclmulhqhqdq %ymm3, %ymm3, %ymm5\n"
        "vpclmulhqhqdq 0x1000(,%rax,4), %ymm12, %ymm29\n"
        "vpclmulhqhqdq %ymm29, %ymm14, %ymm20\n"
        "vpconflictd %xmm0, %xmm20\n"
        "vpconflictd 0x100(%rip){1to16}, %zmm15{%k4}{z}\n"
        "vpconflictq 0x100(%rip), %zmm6\n"
        "vpconflictq 0x100(%rip), %zmm6\n"
        "vplzcntd -64(%rdi), %xmm5{%k3}{z}\n"
        "vplzcntd 0x10(,%r9,4), %xmm12{%k6}\n"
        "vplzcntq 0x12345678(%rax), %xmm5{%k5}\n"
        "vplzcntq %zmm2, %zmm15\n"
        "vpbroadcastmb2q %k6, %zmm10\n"
        "vpbroadcastmb2q %k1, %ymm26\n"
        "vpbroadcastmw2d %k2, %xmm22\n"
        "vpbroadcastmw2d %k3, %ymm29\n"
        "vpdpbusd %ymm22, %ymm5, %ymm26{%k2}{z}\n"
        "vpdpbusd %ymm29, %ymm10, %ymm25{%k5}{z}\n"
        "vpdpbusd 256(%r10,%r15,2), %xmm10, %xmm11\n"
        "vpdpbusd %xmm0, %xmm1, %xmm0\n"
        "vpdpbusds 0x10(,%rcx,4), %ymm24, %ymm2{%k7}{z}\n"
        "vpdpbusds %xmm31, %xmm11, %xmm24{%k3}{z}\n"
        "vpdpbusds 128(%r11), %xmm12, %xmm12\n"
        "vpdpbusds %ymm8, %ymm7, %ymm15\n"
        "vpdpwssd -8(%rip), %xmm25, %xmm2\n"
        "vpdpwssd 0x1000(,%r10,2){1to4}, %xmm13, %xmm3{%k3}\n"
        "vpdpwssd %xmm14, %xmm5, %xmm6\n"
        "vpdpwssd (%rcx), %ymm8, %ymm8\n"
        "vpdpwssds 0(%rip), %xmm22, %xmm1{%k2}{z}\n"
        "vpdpwssds %xmm8, %xmm0, %xmm1{%k4}{z}\n"
        "vpdpwssds %ymm12, %ymm7, %ymm1\n"
        "vpdpwssds %ymm15, %ymm2, %ymm11\n"
        "vpmadd52luq %xmm31, %xmm20, %xmm13\n"
        "vpmadd52luq 0(%rip), %ymm16, %ymm0{%k4}\n"
        "vpmadd52huq %zmm30, %zmm8, %zmm22\n"
        "vpermb %zmm8, %zmm10, %zmm15{%k7}\n"
        "vpermb %zmm3, %zmm4, %zmm23\n"
        "vpermi2b %xmm9, %xmm4, %xmm27\n"
        "vpermi2b %ymm9, %ymm3, %ymm6{%k2}{z}\n"
        "vpermt2b %zmm5, %zmm28, %zmm4\n"
        "vpermt2b %zmm7, %zmm19, %zmm28\n"
        "vpmultishiftqb (%rcx), %zmm20, %zmm8{%k3}{z}\n"
        "vpmultishiftqb 0x1000(,%rdx,4), %ymm10, %ymm20\n"
        "vpopcntb 0x1000(,%rdi,8), %zmm1\n"
        "vpopcntb %zmm30, %zmm1\n"
        "vpopcntw -128(%rdx,%r14,4), %xmm28\n"
        "vpopcntw %zmm17, %zmm22{%k5}{z}\n"
        "vpopcntd 4096(%r13,%rbp,2), %ymm20\n"
        "vpopcntd 128(%rbp), %xmm19\n"
        "vpopcntq 0x1000(,%rcx,8), %zmm22{%k4}{z}\n"
        "vpopcntq %ymm22, %ymm21\n"
        "vpshufbitqmb 0x10(,%rsi,8), %zmm19, %k0{%k2}\n"
        "vpshufbitqmb %ymm11, %ymm0, %k1{%k1}\n"
        "vpermi2d %zmm23, %zmm15, %zmm23\n"
        "vpermi2d 0(%rip){1to16}, %zmm22, %zmm1{%k3}{z}\n"
        "vpermi2q 256(%rbp), %zmm19, %zmm5{%k4}\n"
        "vpermi2q 0x1000(,%rbx,4), %zmm31, %zmm13\n"
        "vpermi2ps %zmm2, %zmm26, %zmm6\n"
        "vpermi2ps -8(%rip){1to16}, %zmm12, %zmm6{%k2}{z}\n"
        "vpermi2pd %zmm7, %zmm28, %zmm30{%k3}\n"
        "vpermi2pd %zmm2, %zmm7, %zmm7{%k3}{z}\n"
        "vpermt2d %ymm29, %ymm6, %ymm21\n"
        "vpermt2d -8(%rip), %xmm14, %xmm2{%k2}\n"
        "vpermt2q 0x1000(,%rdx,8){1to4}, %ymm19, %ymm14{%k2}{z}\n"
        "vpermt2q %xmm31, %xmm10, %xmm30{%k2}\n"
        "vpermt2ps -8(%rip), %ymm12, %ymm4\n"
        "vpermt2ps 0x1000(,%r12,4), %ymm27, %ymm19\n"
        "vpermt2pd %xmm28, %xmm22, %xmm11{%k7}\n"
        "vpermt2pd %zmm29, %zmm14, %zmm26\n"
        "vpermi2w -8(%rip), %zmm13, %zmm18\n"
        "vpermi2w %ymm24, %ymm3, %ymm16\n"
        "vpermt2w %xmm1, %xmm24, %xmm17\n"
        "vpermt2w 0(%rip), %xmm25, %xmm29\n"
        "vpermw 0x100(%rip), %ymm29, %ymm13\n"
        "vpermw %xmm24, %xmm28, %xmm23\n"
        "vpermd %zmm25, %zmm3, %zmm28\n"
        "vpermd %zmm0, %zmm14, %zmm0\n"
        "vpermq %ymm30, %ymm2, %ymm25\n"
        "vpermq %ymm13, %ymm28, %ymm15\n"
        "vpermps %ymm19, %ymm6, %ymm20{%k4}\n"
        "vpermps 0x1000(,%rcx,8){1to8}, %ymm3, %ymm30\n"
        "vpermpd %ymm8, %ymm16, %ymm22\n"
        "vpermpd %zmm6, %zmm17, %zmm5\n"
        "vpermq $138, -8(%rip), %ymm24{%k4}\n"
        "vpermq $136, %zmm3, %zmm19{%k7}\n"
        "vpermpd $213, %zmm9, %zmm18{%k2}{z}\n"
        "vpermpd $133, 64(%r8){1to8}, %zmm1\n"
        "vpcompressd %zmm31, %zmm4\n"
        "vpcompressd %zmm3, %zmm2{%k2}{z}\n"
        "vpcompressq %zmm15, %zmm12\n"
        "vpcompressq %xmm30, %xmm14\n"
        "vcompressps %xmm14, 64(%rsi,%rcx,1){%k2}\n"
        "vcompressps %zmm3, 0x10(,%rsi,8){%k7}\n"
        "vcompresspd %zmm25, 0x10(,%rsi,2)\n"
        "vcompresspd %zmm30, (%r12,%r10,2)\n"
        "vpexpandd %ymm31, %ymm23{%k1}{z}\n"
        "vpexpandd -0x1000(%rsi,%r11,2), %ymm30\n"
        "vpexpandq %xmm2, %xmm13{%k2}{z}\n"
        "vpexpandq %xmm13, %xmm27{%k5}{z}\n"
        "vexpandps 0(%rip), %xmm6{%k6}{z}\n"
        "vexpandps %ymm17, %ymm21\n"
        "vexpandpd 0x10(,%r15,8), %ymm0\n"
        "vexpandpd %zmm25, %zmm23\n"
        "vpbroadcastb %ecx, %xmm20{%k7}\n"
        "vpbroadcastb %ebx, %xmm23{%k4}\n"
        "vpbroadcastw %r11d, %ymm31\n"
        "vpbroadcastw %edi, %xmm13\n"
        "vpbroadcastd %r14d, %zmm12{%k7}{z}\n"
        "vpbroadcastd %edi, %zmm2{%k7}{z}\n"
        "vpbroadcastq %r9, %ymm6\n"
        "vpbroadcastq %rsi, %zmm24\n"
        "vpsllvd 0(%rip), %xmm5, %xmm5\n"
        "vpsllvd 4096(%r8), %ymm11, %ymm3\n"
        "vpsllvd %ymm31, %ymm3, %ymm1{%k3}{z}\n"
        "vpsllvd 0x1000(,%rdi,8){1to16}, %zmm16, %zmm6{%k5}{z}\n"
        "vpsllvq %xmm12, %xmm8, %xmm2\n"
        "vpsllvq 64(%rcx), %xmm14, %xmm15\n"
        "vpsllvq 0x10(,%rbp,8){1to2}, %xmm23, %xmm27{%k4}{z}\n"
        "vpsllvq %xmm7, %xmm7, %xmm18\n"
        "vpsrlvd 0(%rip), %xmm0, %xmm7\n"
        "vpsrlvd %ymm8, %ymm0, %ymm6\n"
        "vpsrlvd %zmm6, %zmm30, %zmm1\n"
        "vpsrlvd %xmm11, %xmm5, %xmm6\n"
        "vpsrlvq %ymm5, %ymm5, %ymm1\n"
        "vpsrlvq -8(%rip), %xmm0, %xmm4\n"
        "vpsrlvq %xmm17, %xmm29, %xmm28\n"
        "vpsrlvq 0x1000(,%rcx,8), %ymm26, %ymm18\n"
        "vpsravd %xmm14, %xmm3, %xmm6\n"
        "vpsravd -0x1000(%rax), %ymm12, %ymm0\n"
        "vpsravd %ymm24, %ymm5, %ymm10{%k6}\n"
        "vpsravd 128(%rsi), %xmm11, %xmm6{%k7}{z}\n"
        "vpsravq -128(%rbp), %xmm29, %xmm14\n"
        "vpsravq %ymm25, %ymm20, %ymm21\n"
        "vpgatherdd %xmm4, 128(%rdi,%xmm1,1), %xmm7\n"
        "vpgatherdd %xmm11, 64(%rbp,%xmm15,8), %xmm9\n"
        "vpgatherdd (%r8,%ymm2,8), %ymm25{%k5}\n"
        "vpgatherdd -128(%r12,%zmm26,2), %zmm19{%k7}\n"
        "vpgatherdq %xmm8, 128(%rsi,%xmm9,1), %xmm9\n"
        "vpgatherdq %xmm3, -128(%r15,%xmm14,8), %xmm0\n"
        "vpgatherdq (%r8,%xmm16,1), %ymm19{%k1}\n"
        "vpgatherdq -0x1000(%rbp,%ymm4,1), %zmm3{%k6}\n"
        "vpgatherqd %xmm4, -128(%r15,%ymm7,8), %xmm15\n"
        "vpgatherqd %xmm1, 128(%r15,%ymm2,1), %xmm4\n"
        "vpgatherqd 128(%r10,%xmm16,8), %xmm9{%k1}\n"
        "vpgatherqd -64(%rcx,%zmm31,8), %ymm12{%k5}\n"
        "vpgatherqq %xmm1, -128(%r15,%xmm11,4), %xmm8\n"
        "vpgatherqq %ymm2, -64(%r11,%ymm0,4), %ymm4\n"
        "vpgatherqq 4096(%rcx,%zmm15,4), %zmm31{%k2}\n"
        "vpgatherqq -0x1000(%r15,%zmm11,8), %zmm11{%k7}\n"
        "vgatherdps %ymm7, 128(%rdi,%ymm13,1), %ymm5\n"
        "vgatherdps %xmm0, -128(%rdx,%xmm15,4), %xmm6\n"
        "vgatherdps 256(%r12,%xmm17,8), %xmm6{%k4}\n"
        "vgatherdps (%r10,%zmm6,2), %zmm11{%k4}\n"
        "vgatherdpd %ymm7, 128(%r13,%xmm9,1), %ymm8\n"
        "vgatherdpd %xmm7, (%rax,%xmm11,1), %xmm8\n"
        "vgatherdpd 256(%r10,%ymm16,1), %zmm19{%k2}\n"
        "vgatherdpd 8(%r10,%xmm17,1), %ymm0{%k2}\n"
        "vgatherqps %xmm1, 0x12345678(%r9,%xmm2,1), %xmm8\n"
        "vgatherqps %xmm12, -0x1000(%rsi,%ymm3,4), %xmm0\n"
        "vgatherqps 128(%rdi,%zmm12,8), %ymm30{%k4}\n"
        "vgatherqps 128(%rdi,%zmm24,8), %ymm29{%k5}\n"
        "vgatherqpd %xmm1, -0x1000(%r11,%xmm15,8), %xmm9\n"
        "vgatherqpd %ymm8, 4096(%rax,%ymm2,2), %ymm3\n"
        "vgatherqpd 64(%r15,%xmm16,4), %xmm6{%k4}\n"
        "vgatherqpd -0x1000(%rax,%zmm5,8), %zmm31{%k5}\n"
        "vpscatterdd %zmm29, -64(%rdx,%zmm28,4){%k3}\n"
        "vpscatterdd %xmm14, -128(%r15,%xmm15,2){%k4}\n"
        "vpscatterdq %xmm31, 8(%rbx,%xmm26,1){%k3}\n"
        "vpscatterdq %xmm11, 128(%rdx,%xmm19,2){%k5}\n"
        "vpscatterqd %ymm19, 8(%rsi,%zmm28,8){%k2}\n"
        "vpscatterqd %xmm31, -0x1000(%r14,%ymm4,8){%k4}\n"
        "vpscatterqq %xmm20, -0x1000(%r10,%xmm15,4){%k3}\n"
        "vpscatterqq %xmm30, 4096(%rdx,%xmm16,8){%k6}\n"
        "vscatterdps %xmm18, 8(%rax,%xmm23,2){%k2}\n"
        "vscatterdps %zmm3, 128(%r14,%zmm15,1){%k7}\n"
        "vscatterdpd %zmm7, 64(%rdx,%ymm30,2){%k4}\n"
        "vscatterdpd %zmm22, -128(%r10,%ymm22,4){%k2}\n"
        "vscatterqps %ymm30, 0x12345678(%r14,%zmm27,1){%k2}\n"
        "vscatterqps %ymm20, 8(%r14,%zmm20,2){%k4}\n"
        "vscatterqpd %xmm7, -64(%rcx,%xmm19,8){%k4}\n"
        "vscatterqpd %xmm11, (%r15,%xmm3,1){%k1}\n"
        "vpmovqb %zmm2, 256(%r9){%k6}\n"
        "vpmovqb %xmm15, -64(%r10){%k3}\n"
        "vpmovsqb %xmm18, %xmm10\n"
        "vpmovsqb %xmm27, %xmm30{%k4}{z}\n"
        "vpmovusqb %xmm12, %xmm20\n"
        "vpmovusqb %xmm25, %xmm8\n"
        "vpmovqw %zmm1, %xmm30\n"
        "vpmovqw %zmm1, 0x100(%rip)\n"
        "vpmovsqw %zmm28, %xmm29{%k7}{z}\n"
        "vpmovsqw %zmm26, -8(%rip){%k3}\n"
        "vpmovusqw %zmm2, %xmm22\n"
        "vpmovusqw %xmm25, 128(%r12)\n"
        "vpmovqd %ymm11, %xmm14{%k3}\n"
        "vpmovqd %xmm26, %xmm6\n"
        "vpmovsqd %xmm11, 0(%rip){%k4}\n"
        "vpmovsqd %xmm23, 0x1000(,%r12,2)\n"
        "vpmovusqd %ymm21, %xmm30\n"
        "vpmovusqd %xmm8, %xmm1{%k2}{z}\n"
        "vpmovdb %xmm0, %xmm24{%k1}{z}\n"
        "vpmovdb %ymm19, 0x10(,%r14,8){%k1}\n"
        "vpmovsdb %xmm24, %xmm4\n"
        "vpmovsdb %ymm23, 0(%rip){%k2}\n"
        "vpmovusdb %zmm11, %xmm31\n"
        "vpmovusdb %xmm23, %xmm23{%k1}{z}\n"
        "vpmovdw %ymm10, %xmm31{%k1}\n"
        "vpmovdw %xmm5, 0x12345678(%r13)\n"
        "vpmovsdw %ymm30, %xmm29{%k3}{z}\n"
        "vpmovsdw %zmm28, %ymm9\n"
        "vpmovusdw %ymm28, %xmm2\n"
        "vpmovusdw %zmm2, 128(%rbp){%k5}\n"
        "vpmovwb %zmm30, -8(%rip)\n"
        "vpmovwb %xmm31, 0x10(,%r9,2){%k1}\n"
        "vpmovswb %xmm5, -0x1000(%r15)\n"
        "vpmovswb %zmm18, %ymm3{%k4}{z}\n"
        "vpmovuswb %xmm0, 0x1000(,%r13,4)\n"
        "vpmovuswb %zmm5, 0(%rip)\n"
        "inb $187, %al\n"
        "inb $73, %al\n"
        "in $51, %al\n"
        "in $153, %al\n"
        "inb %dx, %al\n"
        "inb %dx, %al\n"
        "in %dx, %al\n"
        "in %dx, %al\n"
        "inw $62, %ax\n"
        "inw $138, %ax\n"
        "in $36, %ax\n"
        "in $131, %ax\n"
        "inw %dx, %ax\n"
        "inw %dx, %ax\n"
        "in %dx, %ax\n"
        "in %dx, %ax\n"
        "inl $110, %eax\n"
        "inl $202, %eax\n"
        "in $25, %eax\n"
        "in $250, %eax\n"
        "inl %dx, %eax\n"
        "inl %dx, %eax\n"
        "in %dx, %eax\n"
        "in %dx, %eax\n"
        "outb %al, $38\n"
        "outb %al, $130\n"
        "out %al, $36\n"
        "out %al, $182\n"
        "outb %al, %dx\n"
        "outb %al, %dx\n"
        "out %al, %dx\n"
        "out %al, %dx\n"
        "outw %ax, $164\n"
        "outw %ax, $124\n"
        "out %ax, $177\n"
        "out %ax, $88\n"
        "outw %ax, %dx\n"
        "outw %ax, %dx\n"
        "out %ax, %dx\n"
        "out %ax, %dx\n"
        "outl %eax, $39\n"
        "outl %eax, $13\n"
        "out %eax, $108\n"
        "out %eax, $174\n"
        "outl %eax, %dx\n"
        "outl %eax, %dx\n"
        "out %eax, %dx\n"
        "out %eax, %dx\n"
        "insb\n"
        "insw\n"
        "insl\n"
        "outsb\n"
        "outsw\n"
        "outsl\n"
        ".globl axiom_asm_table_end\n"
        "axiom_asm_table_end:\n"
        ".text\n");

extern const unsigned char axiom_asm_table[], axiom_asm_table_end[];
static const char *const want[] = {
    "d9f0",  /* f2xm1 */
    "d9e1",  /* fabs */
    "d9e0",  /* fchs */
    "9bdbe2",  /* fclex */
    "dbe2",  /* fnclex */
    "ded9",  /* fcompp */
    "d9ff",  /* fcos */
    "d9f6",  /* fdecstp */
    "d9f7",  /* fincstp */
    "9bdbe3",  /* finit */
    "dbe3",  /* fninit */
    "d9e8",  /* fld1 */
    "d9e9",  /* fldl2t */
    "d9ea",  /* fldl2e */
    "d9eb",  /* fldpi */
    "d9ec",  /* fldlg2 */
    "d9ed",  /* fldln2 */
    "d9ee",  /* fldz */
    "d9d0",  /* fnop */
    "d9f3",  /* fpatan */
    "d9f8",  /* fprem */
    "d9f5",  /* fprem1 */
    "d9f2",  /* fptan */
    "d9fc",  /* frndint */
    "d9fd",  /* fscale */
    "d9fe",  /* fsin */
    "d9fb",  /* fsincos */
    "d9fa",  /* fsqrt */
    "d9e4",  /* ftst */
    "dae9",  /* fucompp */
    "d9e5",  /* fxam */
    "d9f4",  /* fxtract */
    "d9f1",  /* fyl2x */
    "d9f9",  /* fyl2xp1 */
    "dec1",  /* faddp */
    "dec9",  /* fmulp */
    "dee1",  /* fsubp */
    "dee9",  /* fsubrp */
    "def1",  /* fdivp */
    "def9",  /* fdivrp */
    "d8d1",  /* fcom */
    "d8d9",  /* fcomp */
    "dde1",  /* fucom */
    "dde9",  /* fucomp */
    "d9c9",  /* fxch */
    "d804dd00100000",  /* fadds 0x1000(,%rbx,8) */
    "42d804c500100000",  /* fadds 0x1000(,%r8,8) */
    "dc4640",  /* faddl 64(%rsi) */
    "dc8078563412",  /* faddl 0x12345678(%rax) */
    "42de44e500",  /* fiadds (%rbp,%r12,8) */
    "de045500100000",  /* fiadds 0x1000(,%rdx,2) */
    "da8180000000",  /* fiaddl 128(%rcx) */
    "da0500010000",  /* fiaddl 0x100(%rip) */
    "42de04f510000000",  /* fiadd 0x10(,%r14,8) */
    "de0500000000",  /* fiadd 0(%rip) */
    "d8c6",  /* fadd %st(6), %st */
    "d8c7",  /* fadd %st(7), %st */
    "d8c6",  /* fadd %st(6) */
    "d8c3",  /* fadd %st(3) */
    "dcc3",  /* fadd %st, %st(3) */
    "dcc7",  /* fadd %st, %st(7) */
    "dec2",  /* faddp %st, %st(2) */
    "dec7",  /* faddp %st, %st(7) */
    "dec7",  /* faddp %st(7) */
    "dec5",  /* faddp %st(5) */
    "43d80c50",  /* fmuls (%r8,%r10,2) */
    "41d88d80000000",  /* fmuls 128(%r13) */
    "41dc8e00100000",  /* fmull 4096(%r14) */
    "dc8c9e00f0ffff",  /* fmull -0x1000(%rsi,%rbx,4) */
    "de8c8980000000",  /* fimuls 128(%rcx,%rcx,4) */
    "de0c7d10000000",  /* fimuls 0x10(,%rdi,2) */
    "da0d00000000",  /* fimull 0(%rip) */
    "41da0c2a",  /* fimull (%r10,%rbp,1) */
    "42de0ce500100000",  /* fimul 0x1000(,%r12,8) */
    "42de0ccd00100000",  /* fimul 0x1000(,%r9,8) */
    "d8ca",  /* fmul %st(2), %st */
    "d8c9",  /* fmul %st(1), %st */
    "d8cc",  /* fmul %st(4) */
    "d8cc",  /* fmul %st(4) */
    "d8c8",  /* fmul %st, %st(0) */
    "d8c8",  /* fmul %st, %st(0) */
    "dec9",  /* fmulp %st, %st(1) */
    "decf",  /* fmulp %st, %st(7) */
    "decf",  /* fmulp %st(7) */
    "dece",  /* fmulp %st(6) */
    "d8649180",  /* fsubs -128(%rcx,%rdx,4) */
    "42d824dd00100000",  /* fsubs 0x1000(,%r11,8) */
    "dc23",  /* fsubl (%rbx) */
    "42dca4b978563412",  /* fsubl 0x12345678(%rcx,%r15,4) */
    "de6780",  /* fisubs -128(%rdi) */
    "43dea41600100000",  /* fisubs 4096(%r14,%r10,1) */
    "daa45d00010000",  /* fisubl 256(%rbp,%rbx,2) */
    "41daa200010000",  /* fisubl 256(%r10) */
    "dea680000000",  /* fisub 128(%rsi) */
    "de2500000000",  /* fisub 0(%rip) */
    "d8e1",  /* fsub %st(1), %st */
    "d8e6",  /* fsub %st(6), %st */
    "d8e7",  /* fsub %st(7) */
    "d8e2",  /* fsub %st(2) */
    "dce6",  /* fsub %st, %st(6) */
    "dce5",  /* fsub %st, %st(5) */
    "dee0",  /* fsubp %st, %st(0) */
    "dee5",  /* fsubp %st, %st(5) */
    "dee6",  /* fsubp %st(6) */
    "dee5",  /* fsubp %st(5) */
    "d82c9500100000",  /* fsubrs 0x1000(,%rdx,4) */
    "d82d00000000",  /* fsubrs 0(%rip) */
    "dc2d00000000",  /* fsubrl 0(%rip) */
    "dc2df8ffffff",  /* fsubrl -8(%rip) */
    "deaf78563412",  /* fisubrs 0x12345678(%rdi) */
    "42de2cad10000000",  /* fisubrs 0x10(,%r13,4) */
    "da2d00010000",  /* fisubrl 0x100(%rip) */
    "41da6ec0",  /* fisubrl -64(%r14) */
    "43de6c59c0",  /* fisubr -64(%r9,%r11,2) */
    "de2d00000000",  /* fisubr 0(%rip) */
    "d8ed",  /* fsubr %st(5), %st */
    "d8ee",  /* fsubr %st(6), %st */
    "d8eb",  /* fsubr %st(3) */
    "d8ef",  /* fsubr %st(7) */
    "dcef",  /* fsubr %st, %st(7) */
    "dced",  /* fsubr %st, %st(5) */
    "dee8",  /* fsubrp %st, %st(0) */
    "deed",  /* fsubrp %st, %st(5) */
    "deec",  /* fsubrp %st(4) */
    "deee",  /* fsubrp %st(6) */
    "42d83482",  /* fdivs (%rdx,%r8,4) */
    "d834fd10000000",  /* fdivs 0x10(,%rdi,8) */
    "43dcb41180000000",  /* fdivl 128(%r9,%r10,1) */
    "dc3500010000",  /* fdivl 0x100(%rip) */
    "deb100100000",  /* fidivs 4096(%rcx) */
    "de34cd10000000",  /* fidivs 0x10(,%rcx,8) */
    "da34ed10000000",  /* fidivl 0x10(,%rbp,8) */
    "da35f8ffffff",  /* fidivl -8(%rip) */
    "42de349510000000",  /* fidiv 0x10(,%r10,4) */
    "42de34fd10000000",  /* fidiv 0x10(,%r15,8) */
    "d8f6",  /* fdiv %st(6), %st */
    "d8f4",  /* fdiv %st(4), %st */
    "d8f0",  /* fdiv %st(0) */
    "d8f0",  /* fdiv %st(0) */
    "dcf4",  /* fdiv %st, %st(4) */
    "dcf2",  /* fdiv %st, %st(2) */
    "def4",  /* fdivp %st, %st(4) */
    "def7",  /* fdivp %st, %st(7) */
    "def6",  /* fdivp %st(6) */
    "def1",  /* fdivp %st(1) */
    "d83df8ffffff",  /* fdivrs -8(%rip) */
    "d83d00010000",  /* fdivrs 0x100(%rip) */
    "dc3c9d00100000",  /* fdivrl 0x1000(,%rbx,4) */
    "42dc3ce510000000",  /* fdivrl 0x10(,%r12,8) */
    "de3df8ffffff",  /* fidivrs -8(%rip) */
    "42de3cf500100000",  /* fidivrs 0x1000(,%r14,8) */
    "43dabc9a78563412",  /* fidivrl 0x12345678(%r10,%r11,4) */
    "da3df8ffffff",  /* fidivrl -8(%rip) */
    "41debc2480000000",  /* fidivr 128(%r12) */
    "de3d00000000",  /* fidivr 0(%rip) */
    "d8fc",  /* fdivr %st(4), %st */
    "d8ff",  /* fdivr %st(7), %st */
    "d8fa",  /* fdivr %st(2) */
    "d8fc",  /* fdivr %st(4) */
    "dcf9",  /* fdivr %st, %st(1) */
    "d8f8",  /* fdivr %st, %st(0) */
    "defe",  /* fdivrp %st, %st(6) */
    "defb",  /* fdivrp %st, %st(3) */
    "def9",  /* fdivrp %st(1) */
    "deff",  /* fdivrp %st(7) */
    "d81500010000",  /* fcoms 0x100(%rip) */
    "d814ed00100000",  /* fcoms 0x1000(,%rbp,8) */
    "dc5640",  /* fcoml 64(%rsi) */
    "dc1500000000",  /* fcoml 0(%rip) */
    "43de949600100000",  /* ficoms 4096(%r14,%r10,4) */
    "42de14ad00100000",  /* ficoms 0x1000(,%r13,4) */
    "da1500000000",  /* ficoml 0(%rip) */
    "da9500100000",  /* ficoml 4096(%rbp) */
    "de9000f0ffff",  /* ficom -0x1000(%rax) */
    "42de14e500100000",  /* ficom 0x1000(,%r12,8) */
    "d8d2",  /* fcom %st(2) */
    "d8d6",  /* fcom %st(6) */
    "41d85c68c0",  /* fcomps -64(%r8,%rbp,2) */
    "43d89cd100010000",  /* fcomps 256(%r9,%r10,8) */
    "43dc5c1008",  /* fcompl 8(%r8,%r10,1) */
    "dc1d00010000",  /* fcompl 0x100(%rip) */
    "de5dc0",  /* ficomps -64(%rbp) */
    "de9978563412",  /* ficomps 0x12345678(%rcx) */
    "da1d00000000",  /* ficompl 0(%rip) */
    "41da59c0",  /* ficompl -64(%r9) */
    "de5d40",  /* ficomp 64(%rbp) */
    "de1cb500100000",  /* ficomp 0x1000(,%rsi,4) */
    "d8db",  /* fcomp %st(3) */
    "d8da",  /* fcomp %st(2) */
    "dde0",  /* fucom %st(0) */
    "dde3",  /* fucom %st(3) */
    "ddec",  /* fucomp %st(4) */
    "ddea",  /* fucomp %st(2) */
    "dbf3",  /* fcomi %st(3), %st */
    "dbf1",  /* fcomi %st(1), %st */
    "dbf1",  /* fcomi %st(1) */
    "dbf2",  /* fcomi %st(2) */
    "dff6",  /* fcomip %st(6), %st */
    "dff1",  /* fcomip %st(1), %st */
    "dff1",  /* fcomip %st(1) */
    "dff6",  /* fcomip %st(6) */
    "dff2",  /* fcompi %st(2), %st */
    "dff4",  /* fcompi %st(4), %st */
    "dff3",  /* fcompi %st(3) */
    "dff5",  /* fcompi %st(5) */
    "dbea",  /* fucomi %st(2), %st */
    "dbea",  /* fucomi %st(2), %st */
    "dbee",  /* fucomi %st(6) */
    "dbee",  /* fucomi %st(6) */
    "dfe8",  /* fucomip %st(0), %st */
    "dfec",  /* fucomip %st(4), %st */
    "dfec",  /* fucomip %st(4) */
    "dfe9",  /* fucomip %st(1) */
    "dfed",  /* fucompi %st(5), %st */
    "dfef",  /* fucompi %st(7), %st */
    "dfee",  /* fucompi %st(6) */
    "dfea",  /* fucompi %st(2) */
    "dac3",  /* fcmovb %st(3), %st */
    "dac6",  /* fcmovb %st(6), %st */
    "daca",  /* fcmove %st(2), %st */
    "dacb",  /* fcmove %st(3), %st */
    "dad6",  /* fcmovbe %st(6), %st */
    "dad1",  /* fcmovbe %st(1), %st */
    "dad9",  /* fcmovu %st(1), %st */
    "dade",  /* fcmovu %st(6), %st */
    "dbc4",  /* fcmovnb %st(4), %st */
    "dbc3",  /* fcmovnb %st(3), %st */
    "dbc3",  /* fcmovae %st(3), %st */
    "dbc6",  /* fcmovae %st(6), %st */
    "dbcf",  /* fcmovne %st(7), %st */
    "dbc8",  /* fcmovne %st(0), %st */
    "dbd3",  /* fcmovnbe %st(3), %st */
    "dbd4",  /* fcmovnbe %st(4), %st */
    "dbd6",  /* fcmova %st(6), %st */
    "dbd2",  /* fcmova %st(2), %st */
    "dbdc",  /* fcmovnu %st(4), %st */
    "dbdd",  /* fcmovnu %st(5), %st */
    "dac0",  /* fcmovnae %st(0), %st */
    "dac6",  /* fcmovnae %st(6), %st */
    "dad5",  /* fcmovna %st(5), %st */
    "dad7",  /* fcmovna %st(7), %st */
    "d9c5",  /* fld %st(5) */
    "d9c2",  /* fld %st(2) */
    "ddd4",  /* fst %st(4) */
    "ddd4",  /* fst %st(4) */
    "dddc",  /* fstp %st(4) */
    "ddde",  /* fstp %st(6) */
    "d9cb",  /* fxch %st(3) */
    "d9ce",  /* fxch %st(6) */
    "ddc2",  /* ffree %st(2) */
    "ddc3",  /* ffree %st(3) */
    "dfc4",  /* ffreep %st(4) */
    "dfc1",  /* ffreep %st(1) */
    "d904c510000000",  /* flds 0x10(,%rax,8) */
    "d9048510000000",  /* flds 0x10(,%rax,4) */
    "43dd0428",  /* fldl (%r8,%r13,1) */
    "42dd044500100000",  /* fldl 0x1000(,%r8,2) */
    "dbac6900f0ffff",  /* fldt -0x1000(%rcx,%rbp,2) */
    "db2d00010000",  /* fldt 0x100(%rip) */
    "41df4608",  /* filds 8(%r14) */
    "df0500000000",  /* filds 0(%rip) */
    "42db046510000000",  /* fildl 0x10(,%r12,2) */
    "db8500f0ffff",  /* fildl -0x1000(%rbp) */
    "df0500010000",  /* fild 0x100(%rip) */
    "df04bd10000000",  /* fild 0x10(,%rdi,4) */
    "42df2c7d10000000",  /* fildll 0x10(,%r15,2) */
    "df2df8ffffff",  /* fildll -8(%rip) */
    "df6c9fc0",  /* fildq -64(%rdi,%rbx,4) */
    "43df2c0b",  /* fildq (%r11,%r9,1) */
    "42d9145500100000",  /* fsts 0x1000(,%r10,2) */
    "41d95640",  /* fsts 64(%r14) */
    "dd15f8ffffff",  /* fstl -8(%rip) */
    "dd543b80",  /* fstl -128(%rbx,%rdi,1) */
    "d91d00000000",  /* fstps 0(%rip) */
    "d91df8ffffff",  /* fstps -8(%rip) */
    "dd1cdd00100000",  /* fstpl 0x1000(,%rbx,8) */
    "41dd9b80000000",  /* fstpl 128(%r11) */
    "db3d00010000",  /* fstpt 0x100(%rip) */
    "db3c6d00100000",  /* fstpt 0x1000(,%rbp,2) */
    "df9078563412",  /* fists 0x12345678(%rax) */
    "df15f8ffffff",  /* fists -8(%rip) */
    "db1500010000",  /* fistl 0x100(%rip) */
    "41db542480",  /* fistl -128(%r12) */
    "41df947200100000",  /* fist 4096(%r10,%rsi,2) */
    "42df14bd00100000",  /* fist 0x1000(,%r15,4) */
    "42df1c7500100000",  /* fistps 0x1000(,%r14,2) */
    "df9e00f0ffff",  /* fistps -0x1000(%rsi) */
    "db9d78563412",  /* fistpl 0x12345678(%rbp) */
    "db1cad00100000",  /* fistpl 0x1000(,%rbp,4) */
    "41df1a",  /* fistp (%r10) */
    "41df1b",  /* fistp (%r11) */
    "df3df8ffffff",  /* fistpll -8(%rip) */
    "43df7c2640",  /* fistpll 64(%r14,%r12,1) */
    "df3d00010000",  /* fistpq 0x100(%rip) */
    "42df3c9d00100000",  /* fistpq 0x1000(,%r11,4) */
    "42df0cf510000000",  /* fisttps 0x10(,%r14,8) */
    "42df4cdf08",  /* fisttps 8(%rdi,%r11,8) */
    "41db8cc200100000",  /* fisttpl 4096(%r10,%rax,8) */
    "41db4c9408",  /* fisttpl 8(%r12,%rdx,4) */
    "42df0c9d00100000",  /* fisttp 0x1000(,%r11,4) */
    "41df8878563412",  /* fisttp 0x12345678(%r8) */
    "41dd4ec0",  /* fisttpll -64(%r14) */
    "dd0b",  /* fisttpll (%rbx) */
    "dd0d00000000",  /* fisttpq 0(%rip) */
    "dd0cc510000000",  /* fisttpq 0x10(,%rax,8) */
    "42df24ed10000000",  /* fbld 0x10(,%r13,8) */
    "41df645308",  /* fbld 8(%r11,%rdx,2) */
    "41dfb41100010000",  /* fbstp 256(%r9,%rdx,1) */
    "dfb200100000",  /* fbstp 4096(%rdx) */
    "42d92ccd10000000",  /* fldcw 0x10(,%r9,8) */
    "42d92cf500100000",  /* fldcw 0x1000(,%r14,8) */
    "d93df8ffffff",  /* fnstcw -8(%rip) */
    "d93c8d10000000",  /* fnstcw 0x10(,%rcx,4) */
    "d92500000000",  /* fldenv 0(%rip) */
    "d92500010000",  /* fldenv 0x100(%rip) */
    "d9347510000000",  /* fnstenv 0x10(,%rsi,2) */
    "d974d208",  /* fnstenv 8(%rdx,%rdx,8) */
    "43ddb40100010000",  /* fnsave 256(%r9,%r8,1) */
    "41dd7500",  /* fnsave (%r13) */
    "42dd641dc0",  /* frstor -64(%rbp,%r11,1) */
    "dd24f500100000",  /* frstor 0x1000(,%rsi,8) */
    "dd3d00010000",  /* fnstsw 0x100(%rip) */
    "43ddbcf180000000",  /* fnstsw 128(%r9,%r14,8) */
    "41d9a500f0ffff",  /* fldenvl -0x1000(%r13) */
    "d926",  /* fldenvl (%rsi) */
    "41d97508",  /* fnstenvl 8(%r13) */
    "d934d510000000",  /* fnstenvl 0x10(,%rdx,8) */
    "43ddb4bb78563412",  /* fnsavel 0x12345678(%r11,%r15,4) */
    "dd34d500100000",  /* fnsavel 0x1000(,%rdx,8) */
    "42dd64d8c0",  /* frstorl -64(%rax,%r11,8) */
    "dda500100000",  /* frstorl 4096(%rbp) */
    "9b41d9b900100000",  /* fstcw 4096(%r9) */
    "9bd93d00000000",  /* fstcw 0(%rip) */
    "9b42d9346500100000",  /* fstenv 0x1000(,%r12,2) */
    "9bd93500000000",  /* fstenv 0(%rip) */
    "9b41ddb300f0ffff",  /* fsave -0x1000(%r11) */
    "9b41ddb44000f0ffff",  /* fsave -0x1000(%r8,%rax,2) */
    "9bddb878563412",  /* fstsw 0x12345678(%rax) */
    "9bdd3d00010000",  /* fstsw 0x100(%rip) */
    "9b42d934bd10000000",  /* fstenvl 0x10(,%r15,4) */
    "9bd93500000000",  /* fstenvl 0(%rip) */
    "9bdd7500",  /* fsavel (%rbp) */
    "9bdd3500010000",  /* fsavel 0x100(%rip) */
    "dfe0",  /* fnstsw %ax */
    "dfe0",  /* fnstsw %ax */
    "9bdfe0",  /* fstsw %ax */
    "9bdfe0",  /* fstsw %ax */
    "0fae4417c0",  /* fxsave -64(%rdi,%rdx,1) */
    "0fae0500010000",  /* fxsave 0x100(%rip) */
    "420fae4ca008",  /* fxrstor 8(%rax,%r12,4) */
    "410fae8a00f0ffff",  /* fxrstor -0x1000(%r10) */
    "4a0fae047d10000000",  /* fxsave64 0x10(,%r15,2) */
    "480fae05f8ffffff",  /* fxsave64 -8(%rip) */
    "4b0fae8c0c80000000",  /* fxrstor64 128(%r12,%r9,1) */
    "490fae8c3778563412",  /* fxrstor64 0x12345678(%r15,%rsi,1) */
    "410fae6500",  /* xsave (%r13) */
    "410faea42478563412",  /* xsave 0x12345678(%r12) */
    "410fae6f40",  /* xrstor 64(%r15) */
    "410faeac3400100000",  /* xrstor 4096(%r12,%rsi,1) */
    "480fae2500000000",  /* xsave64 0(%rip) */
    "480fae25f8ffffff",  /* xsave64 -8(%rip) */
    "4a0fae6c2a08",  /* xrstor64 8(%rdx,%r13,1) */
    "480fae2cb500100000",  /* xrstor64 0x1000(,%rsi,4) */
    "420fae7492c0",  /* xsaveopt -64(%rdx,%r10,4) */
    "0faeb700100000",  /* xsaveopt 4096(%rdi) */
    "480fae3500010000",  /* xsaveopt64 0x100(%rip) */
    "4a0fae34e500100000",  /* xsaveopt64 0x1000(,%r12,8) */
    "0fc725f8ffffff",  /* xsavec -8(%rip) */
    "0fc726",  /* xsavec (%rsi) */
    "480fc724fd10000000",  /* xsavec64 0x10(,%rdi,8) */
    "4a0fc7245d00100000",  /* xsavec64 0x1000(,%r11,2) */
    "0fc72df8ffffff",  /* xsaves -8(%rip) */
    "0fc72d00010000",  /* xsaves 0x100(%rip) */
    "490fc76c2480",  /* xsaves64 -128(%r12) */
    "4a0fc72c6500100000",  /* xsaves64 0x1000(,%r12,2) */
    "0fc71d00010000",  /* xrstors 0x100(%rip) */
    "420fc71c7d10000000",  /* xrstors 0x10(,%r15,2) */
    "490fc79c4200f0ffff",  /* xrstors64 -0x1000(%r10,%rax,2) */
    "480fc79c9500f0ffff",  /* xrstors64 -0x1000(%rbp,%rdx,4) */
    "410f6060c0",  /* punpcklbw -64(%r8), %mm4 */
    "0f602df8ffffff",  /* punpcklbw -8(%rip), %mm5 */
    "410f618500100000",  /* punpcklwd 4096(%r13), %mm0 */
    "410f617500",  /* punpcklwd (%r13), %mm6 */
    "0f62e1",  /* punpckldq %mm1, %mm4 */
    "410f620424",  /* punpckldq (%r12), %mm0 */
    "0f6307",  /* packsswb (%rdi), %mm0 */
    "0f631d00000000",  /* packsswb 0(%rip), %mm3 */
    "0f64f3",  /* pcmpgtb %mm3, %mm6 */
    "0f646c1d00",  /* pcmpgtb (%rbp,%rbx,1), %mm5 */
    "0f65dc",  /* pcmpgtw %mm4, %mm3 */
    "0f65e9",  /* pcmpgtw %mm1, %mm5 */
    "0f66dc",  /* pcmpgtd %mm4, %mm3 */
    "420f664c2a40",  /* pcmpgtd 64(%rdx,%r13,1), %mm1 */
    "0f67c1",  /* packuswb %mm1, %mm0 */
    "0f672500000000",  /* packuswb 0(%rip), %mm4 */
    "420f682c7500100000",  /* punpckhbw 0x1000(,%r14,2), %mm5 */
    "410f68bf00f0ffff",  /* punpckhbw -0x1000(%r15), %mm7 */
    "0f693500010000",  /* punpckhwd 0x100(%rip), %mm6 */
    "0f69ad78563412",  /* punpckhwd 0x12345678(%rbp), %mm5 */
    "0f6ae4",  /* punpckhdq %mm4, %mm4 */
    "430f6a545bc0",  /* punpckhdq -64(%r11,%r11,2), %mm2 */
    "0f6b1d00010000",  /* packssdw 0x100(%rip), %mm3 */
    "0f6bf5",  /* packssdw %mm5, %mm6 */
    "420f74145d10000000",  /* pcmpeqb 0x10(,%r11,2), %mm2 */
    "0f742d00010000",  /* pcmpeqb 0x100(%rip), %mm5 */
    "0f75f4",  /* pcmpeqw %mm4, %mm6 */
    "420f759c9700010000",  /* pcmpeqw 256(%rdi,%r10,4), %mm3 */
    "0f76e3",  /* pcmpeqd %mm3, %mm4 */
    "430f769c7980000000",  /* pcmpeqd 128(%r9,%r15,2), %mm3 */
    "420fd124f510000000",  /* psrlw 0x10(,%r14,8), %mm4 */
    "0fd1f4",  /* psrlw %mm4, %mm6 */
    "0fd2d3",  /* psrld %mm3, %mm2 */
    "0fd27980",  /* psrld -128(%rcx), %mm7 */
    "0fd30d00000000",  /* psrlq 0(%rip), %mm1 */
    "0fd32d00010000",  /* psrlq 0x100(%rip), %mm5 */
    "0fd4fd",  /* paddq %mm5, %mm7 */
    "0fd40d00000000",  /* paddq 0(%rip), %mm1 */
    "0fd51500010000",  /* pmullw 0x100(%rip), %mm2 */
    "0fd58180000000",  /* pmullw 128(%rcx), %mm0 */
    "0fd8f4",  /* psubusb %mm4, %mm6 */
    "410fd8648b80",  /* psubusb -128(%r11,%rcx,4), %mm4 */
    "0fd9ee",  /* psubusw %mm6, %mm5 */
    "0fd9f8",  /* psubusw %mm0, %mm7 */
    "0fdacf",  /* pminub %mm7, %mm1 */
    "0fda14c510000000",  /* pminub 0x10(,%rax,8), %mm2 */
    "0fdb2d00000000",  /* pand 0(%rip), %mm5 */
    "0fdbda",  /* pand %mm2, %mm3 */
    "0fdc2c4d00100000",  /* paddusb 0x1000(,%rcx,2), %mm5 */
    "0fdcfb",  /* paddusb %mm3, %mm7 */
    "0fdd0d00000000",  /* paddusw 0(%rip), %mm1 */
    "0fdd1d00000000",  /* paddusw 0(%rip), %mm3 */
    "0fdee0",  /* pmaxub %mm0, %mm4 */
    "430fdeb4dc78563412",  /* pmaxub 0x12345678(%r12,%r11,8), %mm6 */
    "0fdfe8",  /* pandn %mm0, %mm5 */
    "420fdf8cc100010000",  /* pandn 256(%rcx,%r8,8), %mm1 */
    "0fe01d00000000",  /* pavgb 0(%rip), %mm3 */
    "0fe0c1",  /* pavgb %mm1, %mm0 */
    "420fe12cfd10000000",  /* psraw 0x10(,%r15,8), %mm5 */
    "420fe174cf08",  /* psraw 8(%rdi,%r9,8), %mm6 */
    "0fe25479c0",  /* psrad -64(%rcx,%rdi,2), %mm2 */
    "0fe2d1",  /* psrad %mm1, %mm2 */
    "0fe3fd",  /* pavgw %mm5, %mm7 */
    "0fe3f6",  /* pavgw %mm6, %mm6 */
    "0fe42500010000",  /* pmulhuw 0x100(%rip), %mm4 */
    "0fe4ff",  /* pmulhuw %mm7, %mm7 */
    "0fe5cd",  /* pmulhw %mm5, %mm1 */
    "0fe52d00010000",  /* pmulhw 0x100(%rip), %mm5 */
    "0fe8d3",  /* psubsb %mm3, %mm2 */
    "0fe8ed",  /* psubsb %mm5, %mm5 */
    "420fe92c9510000000",  /* psubsw 0x10(,%r10,4), %mm5 */
    "410fe973c0",  /* psubsw -64(%r11), %mm6 */
    "0fead9",  /* pminsw %mm1, %mm3 */
    "0fea1d00000000",  /* pminsw 0(%rip), %mm3 */
    "0febcd",  /* por %mm5, %mm1 */
    "0feb3500000000",  /* por 0(%rip), %mm6 */
    "0fecd2",  /* paddsb %mm2, %mm2 */
    "0fec15f8ffffff",  /* paddsb -8(%rip), %mm2 */
    "0fedeb",  /* paddsw %mm3, %mm5 */
    "0fedf9",  /* paddsw %mm1, %mm7 */
    "0feed5",  /* pmaxsw %mm5, %mm2 */
    "0feeee",  /* pmaxsw %mm6, %mm5 */
    "0feffb",  /* pxor %mm3, %mm7 */
    "0fefe4",  /* pxor %mm4, %mm4 */
    "0ff1ce",  /* psllw %mm6, %mm1 */
    "0ff13cf500100000",  /* psllw 0x1000(,%rsi,8), %mm7 */
    "420ff2740f40",  /* pslld 64(%rdi,%r9,1), %mm6 */
    "0ff2db",  /* pslld %mm3, %mm3 */
    "0ff3f8",  /* psllq %mm0, %mm7 */
    "420ff3046d10000000",  /* psllq 0x10(,%r13,2), %mm0 */
    "0ff4ef",  /* pmuludq %mm7, %mm5 */
    "0ff44340",  /* pmuludq 64(%rbx), %mm0 */
    "0ff58180000000",  /* pmaddwd 128(%rcx), %mm0 */
    "0ff5fe",  /* pmaddwd %mm6, %mm7 */
    "0ff6ce",  /* psadbw %mm6, %mm1 */
    "420ff6346510000000",  /* psadbw 0x10(,%r12,2), %mm6 */
    "0ff8cd",  /* psubb %mm5, %mm1 */
    "0ff8c8",  /* psubb %mm0, %mm1 */
    "420ff90c9500100000",  /* psubw 0x1000(,%r10,4), %mm1 */
    "0ff92500010000",  /* psubw 0x100(%rip), %mm4 */
    "0ffae7",  /* psubd %mm7, %mm4 */
    "420ffa149d00100000",  /* psubd 0x1000(,%r11,4), %mm2 */
    "0ffbc2",  /* psubq %mm2, %mm0 */
    "0ffb2d00000000",  /* psubq 0(%rip), %mm5 */
    "0ffcff",  /* paddb %mm7, %mm7 */
    "0ffce8",  /* paddb %mm0, %mm5 */
    "0ffd8e80000000",  /* paddw 128(%rsi), %mm1 */
    "420ffd0cd510000000",  /* paddw 0x10(,%r10,8), %mm1 */
    "0ffec2",  /* paddd %mm2, %mm0 */
    "0ffec4",  /* paddd %mm4, %mm0 */
    "0f3800d3",  /* pshufb %mm3, %mm2 */
    "410f3800b40a80000000",  /* pshufb 128(%r10,%rcx,1), %mm6 */
    "0f3801047500100000",  /* phaddw 0x1000(,%rsi,2), %mm0 */
    "0f380105f8ffffff",  /* phaddw -8(%rip), %mm0 */
    "0f3802d8",  /* phaddd %mm0, %mm3 */
    "0f38023d00000000",  /* phaddd 0(%rip), %mm7 */
    "0f3803ff",  /* phaddsw %mm7, %mm7 */
    "0f3803e2",  /* phaddsw %mm2, %mm4 */
    "430f38043471",  /* pmaddubsw (%r9,%r14,2), %mm6 */
    "0f38046280",  /* pmaddubsw -128(%rdx), %mm4 */
    "0f3805fa",  /* phsubw %mm2, %mm7 */
    "0f380534ad10000000",  /* phsubw 0x10(,%rbp,4), %mm6 */
    "410f380664aa08",  /* phsubd 8(%r10,%rbp,4), %mm4 */
    "0f3806e5",  /* phsubd %mm5, %mm4 */
    "0f3807c5",  /* phsubsw %mm5, %mm0 */
    "420f38070c6d00100000",  /* phsubsw 0x1000(,%r13,2), %mm1 */
    "410f38087540",  /* psignb 64(%r13), %mm6 */
    "0f3808f3",  /* psignb %mm3, %mm6 */
    "410f38094a80",  /* psignw -128(%r10), %mm1 */
    "0f380904b500100000",  /* psignw 0x1000(,%rsi,4), %mm0 */
    "410f380a5c9240",  /* psignd 64(%r10,%rdx,4), %mm3 */
    "0f380ae7",  /* psignd %mm7, %mm4 */
    "0f380bd2",  /* pmulhrsw %mm2, %mm2 */
    "0f380bf2",  /* pmulhrsw %mm2, %mm6 */
    "430f381c6c2740",  /* pabsb 64(%r15,%r12,1), %mm5 */
    "0f381cf4",  /* pabsb %mm4, %mm6 */
    "0f381d7c1b08",  /* pabsw 8(%rbx,%rbx,1), %mm7 */
    "0f381dc7",  /* pabsw %mm7, %mm0 */
    "0f381edb",  /* pabsd %mm3, %mm3 */
    "0f381e2500010000",  /* pabsd 0x100(%rip), %mm4 */
    "0f3a0f05000100003a",  /* palignr $58, 0x100(%rip), %mm0 */
    "0f3a0fdb0d",  /* palignr $13, %mm3, %mm3 */
    "0f71d358",  /* psrlw $88, %mm3 */
    "0f71d208",  /* psrlw $8, %mm2 */
    "0f71e163",  /* psraw $99, %mm1 */
    "0f71e484",  /* psraw $132, %mm4 */
    "0f71f72d",  /* psllw $45, %mm7 */
    "0f71f66a",  /* psllw $106, %mm6 */
    "0f72d570",  /* psrld $112, %mm5 */
    "0f72d0f3",  /* psrld $243, %mm0 */
    "0f72e2c7",  /* psrad $199, %mm2 */
    "0f72e39b",  /* psrad $155, %mm3 */
    "0f72f061",  /* pslld $97, %mm0 */
    "0f72f1f5",  /* pslld $245, %mm1 */
    "0f73d607",  /* psrlq $7, %mm6 */
    "0f73d0f3",  /* psrlq $243, %mm0 */
    "0f73f7cf",  /* psllq $207, %mm7 */
    "0f73f2f3",  /* psllq $243, %mm2 */
    "0f70f7ce",  /* pshufw $206, %mm7, %mm6 */
    "430f705c97080d",  /* pshufw $13, 8(%r15,%r10,4), %mm3 */
    "0fc404dd00100000ab",  /* pinsrw $171, 0x1000(,%rbx,8), %mm0 */
    "410fc4ebdc",  /* pinsrw $220, %r11d, %mm5 */
    "440fc5e70e",  /* pextrw $14, %mm7, %r12 */
    "0fc5f0a6",  /* pextrw $166, %mm0, %esi */
    "0fd7f2",  /* pmovmskb %mm2, %esi */
    "0fd7ef",  /* pmovmskb %mm7, %rbp */
    "410fe722",  /* movntq %mm4, (%r10) */
    "0fe72c4d00100000",  /* movntq %mm5, 0x1000(,%rcx,2) */
    "0ff7e8",  /* maskmovq %mm0, %mm5 */
    "0ff7f2",  /* maskmovq %mm2, %mm6 */
    "0f6ef1",  /* movd %ecx, %mm6 */
    "0f6ef8",  /* movd %eax, %mm7 */
    "0f7ede",  /* movd %mm3, %esi */
    "0f7e04ad00100000",  /* movd %mm0, 0x1000(,%rbp,4) */
    "0f6ff6",  /* movq %mm6, %mm6 */
    "0f6f25f8ffffff",  /* movq -8(%rip), %mm4 */
    "410f7fa4a980000000",  /* movq %mm4, 128(%r9,%rbp,4) */
    "420f7fbca600010000",  /* movq %mm7, 256(%rsi,%r12,4) */
    "480f6efa",  /* movq %rdx, %mm7 */
    "480f6ef1",  /* movq %rcx, %mm6 */
    "490f7ec0",  /* movq %mm0, %r8 */
    "480f7ed8",  /* movq %mm3, %rax */
    "490f6ecf",  /* movd %r15, %mm1 */
    "480f6ef3",  /* movd %rbx, %mm6 */
    "490f7ef1",  /* movd %mm6, %r9 */
    "490f7ed2",  /* movd %mm2, %r10 */
    "440f2a15f8ffffff",  /* cvtpi2ps -8(%rip), %xmm10 */
    "0f2adf",  /* cvtpi2ps %mm7, %xmm3 */
    "410f2dd1",  /* cvtps2pi %xmm9, %mm2 */
    "410f2d30",  /* cvtps2pi (%r8), %mm6 */
    "0f2cde",  /* cvttps2pi %xmm6, %mm3 */
    "420f2c247500100000",  /* cvttps2pi 0x1000(,%r14,2), %mm4 */
    "66440f2af9",  /* cvtpi2pd %mm1, %xmm15 */
    "660f2ac4",  /* cvtpi2pd %mm4, %xmm0 */
    "66420f2d34f510000000",  /* cvtpd2pi 0x10(,%r14,8), %mm6 */
    "66410f2dda",  /* cvtpd2pi %xmm10, %mm3 */
    "66410f2cf1",  /* cvttpd2pi %xmm9, %mm6 */
    "660f2c3df8ffffff",  /* cvttpd2pi -8(%rip), %mm7 */
    "f2410fd6fd",  /* movdq2q %xmm13, %mm7 */
    "f2410fd6c5",  /* movdq2q %xmm13, %mm0 */
    "f3440fd6e2",  /* movq2dq %mm2, %xmm12 */
    "f30fd6ec",  /* movq2dq %mm4, %xmm5 */
    "0f0e",  /* femms */
    "66450ffc64cf80",  /* paddb -128(%r15,%rcx,8), %xmm12 */
    "660ffc9600100000",  /* paddb 4096(%rsi), %xmm2 */
    "c4413dfcc0",  /* vpaddb %ymm8, %ymm8, %ymm8 */
    "c5d9fc5a40",  /* vpaddb 64(%rdx), %xmm4, %xmm3 */
    "62f16d0afcf2",  /* vpaddb %xmm2, %xmm2, %xmm6{%k2} */
    "62612d0dfc05f8ffffff",  /* vpaddb -8(%rip), %xmm10, %xmm24{%k5} */
    "660ffdd0",  /* paddw %xmm0, %xmm2 */
    "66440ffdf0",  /* paddw %xmm0, %xmm14 */
    "c509fd0df8ffffff",  /* vpaddw -8(%rip), %xmm14, %xmm9 */
    "c561fd1d00000000",  /* vpaddw 0(%rip), %xmm3, %xmm11 */
    "627135affde8",  /* vpaddw %ymm0, %ymm9, %ymm13{%k7}{z} */
    "62612540fd15f8ffffff",  /* vpaddw -8(%rip), %zmm27, %zmm26 */
    "66410fecfc",  /* paddsb %xmm12, %xmm7 */
    "66420fec044d00100000",  /* paddsb 0x1000(,%r9,2), %xmm0 */
    "c595ecbc5900100000",  /* vpaddsb 4096(%rcx,%rbx,2), %ymm13, %ymm7 */
    "c44121ec5ac0",  /* vpaddsb -64(%r10), %xmm11, %xmm11 */
    "62415500ecf3",  /* vpaddsb %xmm11, %xmm21, %xmm30 */
    "62d14520ec6502",  /* vpaddsb 64(%r13), %ymm23, %ymm4 */
    "66460fed149500100000",  /* paddsw 0x1000(,%r10,4), %xmm10 */
    "66410fedbf80000000",  /* paddsw 128(%r15), %xmm7 */
    "c589edad00f0ffff",  /* vpaddsw -0x1000(%rbp), %xmm14, %xmm5 */
    "c4c101eda4d800010000",  /* vpaddsw 256(%r8,%rbx,8), %xmm15, %xmm4 */
    "62b16d00edd0",  /* vpaddsw %xmm16, %xmm18, %xmm2 */
    "62612547ed3500000000",  /* vpaddsw 0(%rip), %zmm27, %zmm30{%k7} */
    "66450fdcc1",  /* paddusb %xmm9, %xmm8 */
    "66420fdc24bd10000000",  /* paddusb 0x10(,%r15,4), %xmm4 */
    "c5f1dc3500000000",  /* vpaddusb 0(%rip), %xmm1, %xmm6 */
    "c591dcfa",  /* vpaddusb %xmm2, %xmm13, %xmm7 */
    "62b17508dcc9",  /* vpaddusb %xmm17, %xmm1, %xmm1 */
    "62612d4fdcda",  /* vpaddusb %zmm2, %zmm10, %zmm27{%k7} */
    "66440fdd0d00010000",  /* paddusw 0x100(%rip), %xmm9 */
    "66430fdd945d00010000",  /* paddusw 256(%r13,%r11,2), %xmm2 */
    "c4414dddad80000000",  /* vpaddusw 128(%r13), %ymm6, %ymm13 */
    "c58dddae80000000",  /* vpaddusw 128(%rsi), %ymm14, %ymm5 */
    "62813d28ddc1",  /* vpaddusw %ymm25, %ymm8, %ymm16 */
    "62b155c6ddc0",  /* vpaddusw %zmm16, %zmm21, %zmm0{%k6}{z} */
    "66410ff8c4",  /* psubb %xmm12, %xmm0 */
    "66440ff8e9",  /* psubb %xmm1, %xmm13 */
    "c4c145f8f6",  /* vpsubb %ymm14, %ymm7, %ymm6 */
    "c4c11df8f1",  /* vpsubb %ymm9, %ymm12, %ymm6 */
    "621145c3f84c1901",  /* vpsubb 64(%r9,%r11,1), %zmm23, %zmm9{%k3}{z} */
    "62710d29f81500010000",  /* vpsubb 0x100(%rip), %ymm14, %ymm10{%k1} */
    "66460ff94cad80",  /* psubw -128(%rbp,%r13,4), %xmm9 */
    "66450ff9fb",  /* psubw %xmm11, %xmm15 */
    "c4c139f9cf",  /* vpsubw %xmm15, %xmm8, %xmm1 */
    "c539f90c8500100000",  /* vpsubw 0x1000(,%rax,4), %xmm8, %xmm9 */
    "62612520f9be08000000",  /* vpsubw 8(%rsi), %ymm27, %ymm31 */
    "62813d85f9f8",  /* vpsubw %xmm24, %xmm24, %xmm23{%k5}{z} */
    "66440fe8e4",  /* psubsb %xmm4, %xmm12 */
    "66460fe81cc500100000",  /* psubsb 0x1000(,%r8,8), %xmm11 */
    "c4c159e8f0",  /* vpsubsb %xmm8, %xmm4, %xmm6 */
    "c44149e8d9",  /* vpsubsb %xmm9, %xmm6, %xmm11 */
    "62712d48e846ff",  /* vpsubsb -64(%rsi), %zmm10, %zmm8 */
    "62710500e8ec",  /* vpsubsb %xmm4, %xmm31, %xmm13 */
    "660fe9d7",  /* psubsw %xmm7, %xmm2 */
    "66450fe9c3",  /* psubsw %xmm11, %xmm8 */
    "c40101e9844d00f0ffff",  /* vpsubsw -0x1000(%r13,%r9,2), %xmm15, %xmm8 */
    "c541e92df8ffffff",  /* vpsubsw -8(%rip), %xmm7, %xmm13 */
    "62f14542e91d00010000",  /* vpsubsw 0x100(%rip), %zmm23, %zmm3{%k2} */
    "62310d48e9acf308000000",  /* vpsubsw 8(%rbx,%r14,8), %zmm14, %zmm13 */
    "660fd80df8ffffff",  /* psubusb -8(%rip), %xmm1 */
    "66450fd8ee",  /* psubusb %xmm14, %xmm13 */
    "c4c155d8fc",  /* vpsubusb %ymm12, %ymm5, %ymm7 */
    "c42121d83c7d00100000",  /* vpsubusb 0x1000(,%r15,2), %xmm11, %xmm15 */
    "62d15dc6d8d0",  /* vpsubusb %zmm8, %zmm20, %zmm2{%k6}{z} */
    "62e15508d8c4",  /* vpsubusb %xmm4, %xmm5, %xmm16 */
    "66440fd9d9",  /* psubusw %xmm1, %xmm11 */
    "66440fd9fd",  /* psubusw %xmm5, %xmm15 */
    "c501d9d6",  /* vpsubusw %xmm6, %xmm15, %xmm10 */
    "c5add9c5",  /* vpsubusw %ymm5, %ymm10, %ymm0 */
    "62216d28d9d4",  /* vpsubusw %ymm20, %ymm2, %ymm26 */
    "622135c5d9246d00100000",  /* vpsubusw 0x1000(,%r13,2), %zmm25, %zmm28{%k5}{z} */
    "66440f7435f8ffffff",  /* pcmpeqb -8(%rip), %xmm14 */
    "66440f74d1",  /* pcmpeqb %xmm1, %xmm10 */
    "c4c12974e8",  /* vpcmpeqb %xmm8, %xmm10, %xmm5 */
    "c5dd74bf00100000",  /* vpcmpeqb 4096(%rdi), %ymm4, %ymm7 */
    "66440f750c5500100000",  /* pcmpeqw 0x1000(,%rdx,2), %xmm9 */
    "66450f75fb",  /* pcmpeqw %xmm11, %xmm15 */
    "c4c11975e2",  /* vpcmpeqw %xmm10, %xmm12, %xmm4 */
    "c4c15975e0",  /* vpcmpeqw %xmm8, %xmm4, %xmm4 */
    "660f64c0",  /* pcmpgtb %xmm0, %xmm0 */
    "66460f642c9d00100000",  /* pcmpgtb 0x1000(,%r11,4), %xmm13 */
    "c555644840",  /* vpcmpgtb 64(%rax), %ymm5, %ymm9 */
    "c58d64c7",  /* vpcmpgtb %ymm7, %ymm14, %ymm0 */
    "66450f65e0",  /* pcmpgtw %xmm8, %xmm12 */
    "66410f65ea",  /* pcmpgtw %xmm10, %xmm5 */
    "c4a14d651c8d10000000",  /* vpcmpgtw 0x10(,%r9,4), %ymm6, %ymm3 */
    "c4c12565ef",  /* vpcmpgtw %ymm15, %ymm11, %ymm5 */
    "66440feeea",  /* pmaxsw %xmm2, %xmm13 */
    "66440fee1500010000",  /* pmaxsw 0x100(%rip), %xmm10 */
    "c501ee2d00000000",  /* vpmaxsw 0(%rip), %xmm15, %xmm13 */
    "c44149eeac2400f0ffff",  /* vpmaxsw -0x1000(%r12), %xmm6, %xmm13 */
    "62d17500eedb",  /* vpmaxsw %xmm11, %xmm17, %xmm3 */
    "62512543eef9",  /* vpmaxsw %zmm9, %zmm27, %zmm15{%k3} */
    "66440fdeef",  /* pmaxub %xmm7, %xmm13 */
    "66410fde51c0",  /* pmaxub -64(%r9), %xmm2 */
    "c4c141dec5",  /* vpmaxub %xmm13, %xmm7, %xmm0 */
    "c5c5defc",  /* vpmaxub %ymm4, %ymm7, %ymm7 */
    "62b175c6decf",  /* vpmaxub %zmm23, %zmm17, %zmm1{%k6}{z} */
    "62e16528de3500010000",  /* vpmaxub 0x100(%rip), %ymm3, %ymm22 */
    "66440fea6608",  /* pminsw 8(%rsi), %xmm12 */
    "660fead3",  /* pminsw %xmm3, %xmm2 */
    "c549ea3d00010000",  /* vpminsw 0x100(%rip), %xmm6, %xmm15 */
    "c56deaf2",  /* vpminsw %ymm2, %ymm2, %ymm14 */
    "62112d27eae9",  /* vpminsw %ymm25, %ymm26, %ymm13{%k7} */
    "62416d00ea78f8",  /* vpminsw -128(%r8), %xmm18, %xmm31 */
    "66440fda34ea",  /* pminub (%rdx,%rbp,8), %xmm14 */
    "660fdae5",  /* pminub %xmm5, %xmm4 */
    "c531da348500100000",  /* vpminub 0x1000(,%rax,4), %xmm9, %xmm14 */
    "c50dda04b500100000",  /* vpminub 0x1000(,%rsi,4), %ymm14, %ymm8 */
    "62f11d23da3d00000000",  /* vpminub 0(%rip), %ymm28, %ymm7{%k3} */
    "62d13582dac8",  /* vpminub %xmm8, %xmm25, %xmm1{%k2}{z} */
    "66440fe08c0680000000",  /* pavgb 128(%rsi,%rax,1), %xmm9 */
    "66450fe049c0",  /* pavgb -64(%r9), %xmm9 */
    "c44171e0ab80000000",  /* vpavgb 128(%r11), %xmm1, %xmm13 */
    "c4c135e0c8",  /* vpavgb %ymm8, %ymm9, %ymm1 */
    "627155c2e0d2",  /* vpavgb %zmm2, %zmm21, %zmm10{%k2}{z} */
    "627175afe05080",  /* vpavgb -0x1000(%rax), %ymm1, %ymm10{%k7}{z} */
    "660fe3d8",  /* pavgw %xmm0, %xmm3 */
    "66450fe3fb",  /* pavgw %xmm11, %xmm15 */
    "c549e3fc",  /* vpavgw %xmm4, %xmm6, %xmm15 */
    "c565e30500000000",  /* vpavgw 0(%rip), %ymm3, %ymm8 */
    "62111d0de3cd",  /* vpavgw %xmm29, %xmm12, %xmm9{%k5} */
    "62312d48e3ed",  /* vpavgw %zmm21, %zmm10, %zmm13 */
    "66420fd5148500100000",  /* pmullw 0x1000(,%r8,4), %xmm2 */
    "660fd5e5",  /* pmullw %xmm5, %xmm4 */
    "c4c15dd5e6",  /* vpmullw %ymm14, %ymm4, %ymm4 */
    "c42119d53c9d00100000",  /* vpmullw 0x1000(,%r11,4), %xmm12, %xmm15 */
    "c4216dd5246d10000000",  /* vpmullw 0x10(,%r13,2), %ymm2, %ymm12 */
    "62a13528d5346d00100000",  /* vpmullw 0x1000(,%r13,2), %ymm9, %ymm22 */
    "660fe5d0",  /* pmulhw %xmm0, %xmm2 */
    "66410fe5d0",  /* pmulhw %xmm8, %xmm2 */
    "c42139e52cb510000000",  /* vpmulhw 0x10(,%r14,4), %xmm8, %xmm13 */
    "c561e5f1",  /* vpmulhw %xmm1, %xmm3, %xmm14 */
    "62115d08e5ed",  /* vpmulhw %xmm29, %xmm4, %xmm13 */
    "628165c3e5e4",  /* vpmulhw %zmm28, %zmm19, %zmm20{%k3}{z} */
    "66410fe4c0",  /* pmulhuw %xmm8, %xmm0 */
    "66450fe4449008",  /* pmulhuw 8(%r8,%rdx,4), %xmm8 */
    "c4c141e42c9f",  /* vpmulhuw (%r15,%rbx,4), %xmm7, %xmm5 */
    "c4c131e4dc",  /* vpmulhuw %xmm12, %xmm9, %xmm3 */
    "62a16520e4db",  /* vpmulhuw %ymm19, %ymm19, %ymm19 */
    "6261758be40df8ffffff",  /* vpmulhuw -8(%rip), %xmm1, %xmm25{%k3}{z} */
    "66410ff5fd",  /* pmaddwd %xmm13, %xmm7 */
    "66440ff5f9",  /* pmaddwd %xmm1, %xmm15 */
    "c44171f5dd",  /* vpmaddwd %xmm13, %xmm1, %xmm11 */
    "c50df53500000000",  /* vpmaddwd 0(%rip), %ymm14, %ymm14 */
    "62615d40f5d0",  /* vpmaddwd %zmm0, %zmm20, %zmm26 */
    "62511d48f56436fe",  /* vpmaddwd -128(%r14,%rsi,1), %zmm12, %zmm12 */
    "66440ff6cb",  /* psadbw %xmm3, %xmm9 */
    "66410ff6a600100000",  /* psadbw 4096(%r14), %xmm4 */
    "c5c1f6ce",  /* vpsadbw %xmm6, %xmm7, %xmm1 */
    "c525f6e1",  /* vpsadbw %ymm1, %ymm11, %ymm12 */
    "66440f680d00000000",  /* punpckhbw 0(%rip), %xmm9 */
    "66460f680c5d00100000",  /* punpckhbw 0x1000(,%r11,2), %xmm9 */
    "c54568e9",  /* vpunpckhbw %ymm1, %ymm7, %ymm13 */
    "c5cd68f5",  /* vpunpckhbw %ymm5, %ymm6, %ymm6 */
    "62a155286844e204",  /* vpunpckhbw 128(%rdx,%r12,8), %ymm5, %ymm16 */
    "62a1350f681c8510000000",  /* vpunpckhbw 0x10(,%r8,4), %xmm9, %xmm19{%k7} */
    "66410f699d00010000",  /* punpckhwd 256(%r13), %xmm3 */
    "66460f690c0f",  /* punpckhwd (%rdi,%r9,1), %xmm9 */
    "c4c1316959c0",  /* vpunpckhwd -64(%r9), %xmm9, %xmm3 */
    "c55d692d00010000",  /* vpunpckhwd 0x100(%rip), %ymm4, %ymm13 */
    "6231554a69c7",  /* vpunpckhwd %zmm23, %zmm5, %zmm8{%k2} */
    "62615d8b691df8ffffff",  /* vpunpckhwd -8(%rip), %xmm4, %xmm27{%k3}{z} */
    "66470f609c5700010000",  /* punpcklbw 256(%r15,%r10,2), %xmm11 */
    "66450f608200f0ffff",  /* punpcklbw -0x1000(%r10), %xmm8 */
    "c4215d609c7900100000",  /* vpunpcklbw 4096(%rcx,%r15,2), %ymm4, %ymm11 */
    "c515605e40",  /* vpunpcklbw 64(%rsi), %ymm13, %ymm11 */
    "62c17d4460742440",  /* vpunpcklbw 4096(%r12), %zmm16, %zmm22{%k4} */
    "6231452e60ec",  /* vpunpcklbw %ymm20, %ymm7, %ymm13{%k6} */
    "66440f61f9",  /* punpcklwd %xmm1, %xmm15 */
    "66440f61147510000000",  /* punpcklwd 0x10(,%rsi,2), %xmm10 */
    "c5416124ed00100000",  /* vpunpcklwd 0x1000(,%rbp,8), %xmm7, %xmm12 */
    "c53561db",  /* vpunpcklwd %ymm3, %ymm9, %ymm11 */
    "6291554061c9",  /* vpunpcklwd %zmm25, %zmm21, %zmm1 */
    "62e1052461fe",  /* vpunpcklwd %ymm6, %ymm31, %ymm23{%k4} */
    "66410f63fa",  /* packsswb %xmm10, %xmm7 */
    "66450f63f9",  /* packsswb %xmm9, %xmm15 */
    "c4a10d63349d00100000",  /* vpacksswb 0x1000(,%r11,4), %ymm14, %ymm6 */
    "c4c12563d2",  /* vpacksswb %ymm10, %ymm11, %ymm2 */
    "6261350863fc",  /* vpacksswb %xmm4, %xmm9, %xmm31 */
    "6201152063ef",  /* vpacksswb %ymm31, %ymm29, %ymm29 */
    "66440f67ec",  /* packuswb %xmm4, %xmm13 */
    "66440f67ec",  /* packuswb %xmm4, %xmm13 */
    "c4c12967db",  /* vpackuswb %xmm11, %xmm10, %xmm3 */
    "c4a10d6714ed00100000",  /* vpackuswb 0x1000(,%r13,8), %ymm14, %ymm2 */
    "62213dc767ed",  /* vpackuswb %zmm21, %zmm24, %zmm29{%k7}{z} */
    "62b16dca67f4",  /* vpackuswb %zmm20, %zmm2, %zmm6{%k2}{z} */
    "66450f6be4",  /* packssdw %xmm12, %xmm12 */
    "66410f6be7",  /* packssdw %xmm15, %xmm4 */
    "c5dd6be4",  /* vpackssdw %ymm4, %ymm4, %ymm4 */
    "c4a1496b34c500100000",  /* vpackssdw 0x1000(,%r8,8), %xmm6, %xmm6 */
    "623175056bfc",  /* vpackssdw %xmm20, %xmm17, %xmm15{%k5} */
    "62e115006b8608000000",  /* vpackssdw 8(%rsi), %xmm29, %xmm16 */
    "62e11d20f67d00",  /* vpsadbw (%rbp), %ymm28, %ymm23 */
    "c42175f6346510000000",  /* vpsadbw 0x10(,%r12,2), %ymm1, %ymm14 */
    "66410ffed0",  /* paddd %xmm8, %xmm2 */
    "660ffee9",  /* paddd %xmm1, %xmm5 */
    "c44145fef1",  /* vpaddd %ymm9, %ymm7, %ymm14 */
    "c5d5fe34f500100000",  /* vpaddd 0x1000(,%rsi,8), %ymm5, %ymm6 */
    "62614d00fe8cc278563412",  /* vpaddd 0x12345678(%rdx,%rax,8), %xmm22, %xmm25 */
    "62c17d22fee9",  /* vpaddd %ymm9, %ymm16, %ymm21{%k2} */
    "660ffa8e78563412",  /* psubd 0x12345678(%rsi), %xmm1 */
    "660ffaf0",  /* psubd %xmm0, %xmm6 */
    "c4c12dfaee",  /* vpsubd %ymm14, %ymm10, %ymm5 */
    "c4c145fae6",  /* vpsubd %ymm14, %ymm7, %ymm4 */
    "62a16520fae1",  /* vpsubd %ymm17, %ymm19, %ymm20 */
    "62f145a1facb",  /* vpsubd %ymm3, %ymm23, %ymm1{%k1}{z} */
    "66420f7604e500100000",  /* pcmpeqd 0x1000(,%r12,8), %xmm0 */
    "660f762d00000000",  /* pcmpeqd 0(%rip), %xmm5 */
    "c4010d76745208",  /* vpcmpeqd 8(%r10,%r10,2), %ymm14, %ymm14 */
    "c5a176d3",  /* vpcmpeqd %xmm3, %xmm11, %xmm2 */
    "66470f664cfd08",  /* pcmpgtd 8(%r13,%r15,8), %xmm9 */
    "660f66e2",  /* pcmpgtd %xmm2, %xmm4 */
    "c4214566245500100000",  /* vpcmpgtd 0x1000(,%r10,2), %ymm7, %ymm12 */
    "c4411966f2",  /* vpcmpgtd %xmm10, %xmm12, %xmm14 */
    "66440f383dd1",  /* pmaxsd %xmm1, %xmm10 */
    "66440f383dd4",  /* pmaxsd %xmm4, %xmm10 */
    "c442713dfe",  /* vpmaxsd %xmm14, %xmm1, %xmm15 */
    "c442393dba80000000",  /* vpmaxsd 128(%r10), %xmm8, %xmm15 */
    "62821daf3dea",  /* vpmaxsd %ymm26, %ymm12, %ymm21{%k7}{z} */
    "62023d473dc1",  /* vpmaxsd %zmm25, %zmm24, %zmm24{%k7} */
    "66410f3839ff",  /* pminsd %xmm15, %xmm7 */
    "66440f38391500000000",  /* pminsd 0(%rip), %xmm10 */
    "c4c23d39e5",  /* vpminsd %ymm13, %ymm8, %ymm4 */
    "c4620139e2",  /* vpminsd %xmm2, %xmm15, %xmm12 */
    "62624d003944eafc",  /* vpminsd -64(%rdx,%rbp,8), %xmm22, %xmm24 */
    "6252258d39cd",  /* vpminsd %xmm13, %xmm11, %xmm9{%k5}{z} */
    "66450f383feb",  /* pmaxud %xmm11, %xmm13 */
    "660f383f7980",  /* pmaxud -128(%rcx), %xmm7 */
    "c4626d3f25f8ffffff",  /* vpmaxud -8(%rip), %ymm2, %ymm12 */
    "c442213fad00100000",  /* vpmaxud 4096(%r13), %xmm11, %xmm13 */
    "62b275003fb4f200f0ffff",  /* vpmaxud -0x1000(%rdx,%r14,8), %xmm17, %xmm6 */
    "62224d133f044d10000000",  /* vpmaxud 0x10(,%r9,2){1to4}, %xmm22, %xmm24{%k3} */
    "66410f383bf0",  /* pminud %xmm8, %xmm6 */
    "66440f383ba600100000",  /* pminud 4096(%rsi), %xmm12 */
    "c4c2653bb678563412",  /* vpminud 0x12345678(%r14), %ymm3, %ymm6 */
    "c462093bd6",  /* vpminud %xmm6, %xmm14, %xmm10 */
    "62720d023bc3",  /* vpminud %xmm3, %xmm30, %xmm8{%k2} */
    "62e27d203bd5",  /* vpminud %ymm5, %ymm16, %ymm18 */
    "660f3840f8",  /* pmulld %xmm0, %xmm7 */
    "66410f3840a378563412",  /* pmulld 0x12345678(%r11), %xmm4 */
    "c46215409380000000",  /* vpmulld 128(%rbx), %ymm13, %ymm10 */
    "c4e27940bb00f0ffff",  /* vpmulld -0x1000(%rbx), %xmm0, %xmm7 */
    "6242658540d1",  /* vpmulld %xmm9, %xmm19, %xmm26{%k5}{z} */
    "62327d4f402c5d10000000",  /* vpmulld 0x10(,%r11,2), %zmm0, %zmm13{%k7} */
    "66440ff4e0",  /* pmuludq %xmm0, %xmm12 */
    "660ff47d08",  /* pmuludq 8(%rbp), %xmm7 */
    "c549f4d2",  /* vpmuludq %xmm2, %xmm6, %xmm10 */
    "c529f44d40",  /* vpmuludq 64(%rbp), %xmm10, %xmm9 */
    "6251bd43f4c3",  /* vpmuludq %zmm11, %zmm24, %zmm8{%k3} */
    "6211ad00f4e2",  /* vpmuludq %xmm26, %xmm26, %xmm12 */
    "660f6ac3",  /* punpckhdq %xmm3, %xmm0 */
    "66410f6a647ac0",  /* punpckhdq -64(%r10,%rdi,2), %xmm4 */
    "c5d16a943e00100000",  /* vpunpckhdq 4096(%rsi,%rdi,1), %xmm5, %xmm2 */
    "c5116a0500010000",  /* vpunpckhdq 0x100(%rip), %xmm13, %xmm8 */
    "62416d286aee",  /* vpunpckhdq %ymm14, %ymm2, %ymm29 */
    "629125206aa40208000000",  /* vpunpckhdq 8(%r10,%r8,1), %ymm27, %ymm4 */
    "66440f623df8ffffff",  /* punpckldq -8(%rip), %xmm15 */
    "66430f62ac6b78563412",  /* punpckldq 0x12345678(%r11,%r13,2), %xmm5 */
    "c4c11162d7",  /* vpunpckldq %xmm15, %xmm13, %xmm2 */
    "c51d622500000000",  /* vpunpckldq 0(%rip), %ymm12, %ymm12 */
    "62e11d96622d00010000",  /* vpunpckldq 0x100(%rip){1to4}, %xmm28, %xmm21{%k6}{z} */
    "62f105cf62348d00100000",  /* vpunpckldq 0x1000(,%rcx,4), %zmm15, %zmm6{%k7}{z} */
    "66450f6de4",  /* punpckhqdq %xmm12, %xmm12 */
    "660f6df4",  /* punpckhqdq %xmm4, %xmm6 */
    "c4410d6d61c0",  /* vpunpckhqdq -64(%r9), %ymm14, %ymm12 */
    "c5056d7240",  /* vpunpckhqdq 64(%rdx), %ymm15, %ymm14 */
    "6221a5386d346510000000",  /* vpunpckhqdq 0x10(,%r12,2){1to4}, %ymm11, %ymm30 */
    "6271bdb56d1d00000000",  /* vpunpckhqdq 0(%rip){1to4}, %ymm24, %ymm11{%k5}{z} */
    "660f6cf1",  /* punpcklqdq %xmm1, %xmm6 */
    "66440f6c146d10000000",  /* punpcklqdq 0x10(,%rbp,2), %xmm10 */
    "c441556cdf",  /* vpunpcklqdq %ymm15, %ymm5, %ymm11 */
    "c54d6cdf",  /* vpunpcklqdq %ymm7, %ymm6, %ymm11 */
    "6251e5a26cbf00100000",  /* vpunpcklqdq 4096(%r15), %ymm19, %ymm15{%k2}{z} */
    "62819dab6c54e500",  /* vpunpcklqdq (%r13,%r12,8), %ymm12, %ymm18{%k3}{z} */
    "62d16508749000f0ffff",  /* vpcmpeqb -0x1000(%r8), %xmm3, %k2 */
    "6291250274f9",  /* vpcmpeqb %xmm25, %xmm27, %k7{%k2} */
    "62914d2875db",  /* vpcmpeqw %ymm27, %ymm6, %k3 */
    "62f145287525f8ffffff",  /* vpcmpeqw -8(%rip), %ymm7, %k4 */
    "62917d2464a42178563412",  /* vpcmpgtb 0x12345678(%r9,%r12,1), %ymm16, %k4{%k4} */
    "62d17d0064f0",  /* vpcmpgtb %xmm8, %xmm16, %k6 */
    "62f1354365eb",  /* vpcmpgtw %zmm3, %zmm25, %k5{%k3} */
    "62f12500653cf510000000",  /* vpcmpgtw 0x10(,%rsi,8), %xmm27, %k7 */
    "62d10d2576cf",  /* vpcmpeqd %ymm15, %ymm30, %k1{%k5} */
    "62d14d3776b900f0ffff",  /* vpcmpeqd -0x1000(%r9){1to8}, %ymm22, %k7{%k7} */
    "62d15d5866a678563412",  /* vpcmpgtd 0x12345678(%r14){1to16}, %zmm4, %k4 */
    "62f16558668c5e78563412",  /* vpcmpgtd 0x12345678(%rsi,%rbx,2){1to16}, %zmm3, %k1 */
    "62d2a54929dc",  /* vpcmpeqq %zmm12, %zmm11, %k3{%k1} */
    "6292ad2929f3",  /* vpcmpeqq %ymm27, %ymm10, %k6{%k1} */
    "62f2bd2c372d00010000",  /* vpcmpgtq 0x100(%rip), %ymm8, %k5{%k4} */
    "62f2f5483734b500100000",  /* vpcmpgtq 0x1000(,%rsi,4), %zmm1, %k6 */
    "66410f3829ea",  /* pcmpeqq %xmm10, %xmm5 */
    "66440f38293d00010000",  /* pcmpeqq 0x100(%rip), %xmm15 */
    "c4627529e1",  /* vpcmpeqq %ymm1, %ymm1, %ymm12 */
    "c4626d29d4",  /* vpcmpeqq %ymm4, %ymm2, %ymm10 */
    "66410f3837f6",  /* pcmpgtq %xmm14, %xmm6 */
    "66450f3837d9",  /* pcmpgtq %xmm9, %xmm11 */
    "c4e26137c4",  /* vpcmpgtq %xmm4, %xmm3, %xmm0 */
    "c4e26d3734f500100000",  /* vpcmpgtq 0x1000(,%rsi,8), %ymm2, %ymm6 */
    "66420fd41c2a",  /* paddq (%rdx,%r13,1), %xmm3 */
    "66450fd4a44b00f0ffff",  /* paddq -0x1000(%r11,%rcx,2), %xmm12 */
    "c58dd41c4500100000",  /* vpaddq 0x1000(,%rax,2), %ymm14, %ymm3 */
    "c5e9d42df8ffffff",  /* vpaddq -8(%rip), %xmm2, %xmm5 */
    "62c1b508d4e3",  /* vpaddq %xmm11, %xmm9, %xmm20 */
    "62f1c500d49878563412",  /* vpaddq 0x12345678(%rax), %xmm23, %xmm3 */
    "66440ffb5c7b08",  /* psubq 8(%rbx,%rdi,2), %xmm11 */
    "660ffbf4",  /* psubq %xmm4, %xmm6 */
    "c44165fbd9",  /* vpsubq %ymm9, %ymm3, %ymm11 */
    "c5a9fb1500010000",  /* vpsubq 0x100(%rip), %xmm10, %xmm2 */
    "6261e5a2fb2d00000000",  /* vpsubq 0(%rip), %ymm19, %ymm29{%k2}{z} */
    "62e1ed41fb4204",  /* vpsubq 256(%rdx), %zmm18, %zmm16{%k1} */
    "660f382854ddc0",  /* pmuldq -64(%rbp,%rbx,8), %xmm2 */
    "66450f3828ee",  /* pmuldq %xmm14, %xmm13 */
    "c4e23928f4",  /* vpmuldq %xmm4, %xmm8, %xmm6 */
    "c4c20d28c3",  /* vpmuldq %ymm11, %ymm14, %ymm0 */
    "62e2954b2824d500100000",  /* vpmuldq 0x1000(,%rdx,8), %zmm13, %zmm20{%k3} */
    "6222cd4828b40378563412",  /* vpmuldq 0x12345678(%rbx,%r8,1), %zmm6, %zmm30 */
    "660fdb1cdd00100000",  /* pand 0x1000(,%rbx,8), %xmm3 */
    "66440fdb14ad10000000",  /* pand 0x10(,%rbp,4), %xmm10 */
    "c4c11ddbd0",  /* vpand %ymm8, %ymm12, %ymm2 */
    "c4816ddb94c300100000",  /* vpand 4096(%r11,%r8,8), %ymm2, %ymm2 */
    "62213d07db4c2004",  /* vpandd 64(%rax,%r12,1), %xmm24, %xmm25{%k7} */
    "627135c4db6002",  /* vpandd 128(%rax), %zmm25, %zmm12{%k4}{z} */
    "6271edc2dbe2",  /* vpandq %zmm2, %zmm18, %zmm12{%k2}{z} */
    "62b1ed84dbd4",  /* vpandq %xmm20, %xmm18, %xmm2{%k4}{z} */
    "66460fdf3c8510000000",  /* pandn 0x10(,%r8,4), %xmm15 */
    "660fdf71c0",  /* pandn -64(%rcx), %xmm6 */
    "c4a129df249510000000",  /* vpandn 0x10(,%r10,4), %xmm10, %xmm4 */
    "c595df2500010000",  /* vpandn 0x100(%rip), %ymm13, %ymm4 */
    "62712528dfc7",  /* vpandnd %ymm7, %ymm11, %ymm8 */
    "62a10520dff1",  /* vpandnd %ymm17, %ymm31, %ymm22 */
    "62b18508dfe0",  /* vpandnq %xmm16, %xmm15, %xmm4 */
    "62d1cd81dfdf",  /* vpandnq %xmm15, %xmm22, %xmm3{%k1}{z} */
    "660feb2d00010000",  /* por 0x100(%rip), %xmm5 */
    "66450feb9c9e00010000",  /* por 256(%r14,%rbx,4), %xmm11 */
    "c5a1eb3df8ffffff",  /* vpor -8(%rip), %xmm11, %xmm7 */
    "c4c179ebde",  /* vpor %xmm14, %xmm0, %xmm3 */
    "62e13d0cebb900f0ffff",  /* vpord -0x1000(%rcx), %xmm8, %xmm23{%k4} */
    "62f15d38eb3d00000000",  /* vpord 0(%rip){1to8}, %ymm4, %ymm7 */
    "62f19d08eb05f8ffffff",  /* vporq -8(%rip), %xmm12, %xmm0 */
    "62c1a5c7eb6c24ff",  /* vporq -64(%r12), %zmm27, %zmm21{%k7}{z} */
    "66410feffb",  /* pxor %xmm11, %xmm7 */
    "66440feff3",  /* pxor %xmm3, %xmm14 */
    "c5c1efe1",  /* vpxor %xmm1, %xmm7, %xmm4 */
    "c4a145efacaf00f0ffff",  /* vpxor -0x1000(%rdi,%r13,4), %ymm7, %ymm5 */
    "62f15550ef34cd00100000",  /* vpxord 0x1000(,%rcx,8){1to16}, %zmm21, %zmm6 */
    "62d10d87efcd",  /* vpxord %xmm13, %xmm30, %xmm1{%k7}{z} */
    "62419d40effc",  /* vpxorq %zmm12, %zmm28, %zmm31 */
    "6281fd4aefc0",  /* vpxorq %zmm24, %zmm0, %zmm16{%k2} */
    "c4410c58fc",  /* vaddps %ymm12, %ymm14, %ymm15 */
    "c4a134580ca500100000",  /* vaddps 0x1000(,%r12,4), %ymm9, %ymm1 */
    "62916400587cac04",  /* vaddps 64(%r12,%r13,4), %xmm19, %xmm7 */
    "62c12430587e10",  /* vaddps 64(%r14){1to8}, %ymm27, %ymm23 */
    "c59d581cf500100000",  /* vaddpd 0x1000(,%rsi,8), %ymm12, %ymm3 */
    "c585588680000000",  /* vaddpd 128(%rsi), %ymm15, %ymm0 */
    "62f195185835f8ffffff",  /* vaddpd -8(%rip){1to2}, %xmm13, %xmm6 */
    "62a1b50858d4",  /* vaddpd %xmm20, %xmm9, %xmm18 */
    "c4414a58ca",  /* vaddss %xmm10, %xmm6, %xmm9 */
    "c52a582d00010000",  /* vaddss 0x100(%rip), %xmm10, %xmm13 */
    "62b1560058e0",  /* vaddss %xmm16, %xmm21, %xmm4 */
    "62110e0058e4",  /* vaddss %xmm28, %xmm30, %xmm12 */
    "c4413358eb",  /* vaddsd %xmm11, %xmm9, %xmm13 */
    "c4814b589c4d78563412",  /* vaddsd 0x12345678(%r13,%r9,2), %xmm6, %xmm3 */
    "62f1ff5058db",  /* vaddsd {ru-sae}, %xmm3, %xmm16, %xmm3 */
    "c51b58d8",  /* vaddsd %xmm0, %xmm12, %xmm11 */
    "c5a85c3c8500100000",  /* vsubps 0x1000(,%rax,4), %xmm10, %xmm7 */
    "c5085c14fd10000000",  /* vsubps 0x10(,%rdi,8), %xmm14, %xmm10 */
    "62a1745d5c044500100000",  /* vsubps 0x1000(,%r8,2){1to16}, %zmm1, %zmm16{%k5} */
    "621134425ce1",  /* vsubps %zmm25, %zmm25, %zmm12{%k2} */
    "c4a11d5c749008",  /* vsubpd 8(%rax,%r10,4), %ymm12, %ymm6 */
    "c401155c6c4bc0",  /* vsubpd -64(%r11,%r9,2), %ymm13, %ymm13 */
    "629195fc5cc4",  /* vsubpd {rz-sae}, %zmm28, %zmm13, %zmm0{%k4}{z} */
    "6261ed405c8b08000000",  /* vsubpd 8(%rbx), %zmm18, %zmm25 */
    "c5725c3500010000",  /* vsubss 0x100(%rip), %xmm1, %xmm14 */
    "c441625ccd",  /* vsubss %xmm13, %xmm3, %xmm9 */
    "62212e0a5c1ca500100000",  /* vsubss 0x1000(,%r12,4), %xmm10, %xmm27{%k2} */
    "62e11e015c0df8ffffff",  /* vsubss -8(%rip), %xmm28, %xmm17{%k1} */
    "c4c1535ced",  /* vsubsd %xmm13, %xmm5, %xmm5 */
    "c481135c246c",  /* vsubsd (%r12,%r13,2), %xmm13, %xmm4 */
    "62d1f70f5cba78563412",  /* vsubsd 0x12345678(%r10), %xmm1, %xmm7{%k7} */
    "6211b7005ccf",  /* vsubsd %xmm31, %xmm25, %xmm9 */
    "c4413459c9",  /* vmulps %ymm9, %ymm9, %ymm9 */
    "c560591e",  /* vmulps (%rsi), %xmm3, %xmm11 */
    "62c1045a596d20",  /* vmulps 128(%r13){1to16}, %zmm15, %zmm21{%k2} */
    "626134a3596c1280",  /* vmulps -0x1000(%rdx,%rdx,1), %ymm25, %ymm29{%k3}{z} */
    "c4c13159f8",  /* vmulpd %xmm8, %xmm9, %xmm7 */
    "c4416d59c0",  /* vmulpd %ymm8, %ymm2, %ymm8 */
    "6251bd2059f7",  /* vmulpd %ymm15, %ymm24, %ymm14 */
    "62719d8e59e2",  /* vmulpd %xmm2, %xmm12, %xmm12{%k6}{z} */
    "c5f259f1",  /* vmulss %xmm1, %xmm1, %xmm6 */
    "c5ca591500010000",  /* vmulss 0x100(%rip), %xmm6, %xmm2 */
    "c5425935f8ffffff",  /* vmulss -8(%rip), %xmm7, %xmm14 */
    "62f15600592d00010000",  /* vmulss 0x100(%rip), %xmm21, %xmm5 */
    "c5b3592500010000",  /* vmulsd 0x100(%rip), %xmm9, %xmm4 */
    "c59359cb",  /* vmulsd %xmm3, %xmm13, %xmm1 */
    "c4c10359a4b880000000",  /* vmulsd 128(%r8,%rdi,4), %xmm15, %xmm4 */
    "62e1cf7059eb",  /* vmulsd {rz-sae}, %xmm3, %xmm22, %xmm21 */
    "c4a1685e0cdd10000000",  /* vdivps 0x10(,%r11,8), %xmm2, %xmm1 */
    "c5d85ee5",  /* vdivps %xmm5, %xmm4, %xmm4 */
    "62e104a15ebcc300100000",  /* vdivps 4096(%rbx,%rax,8), %ymm31, %ymm23{%k1}{z} */
    "62e164005ed2",  /* vdivps %xmm2, %xmm19, %xmm18 */
    "c5dd5ed7",  /* vdivpd %ymm7, %ymm4, %ymm2 */
    "c4416d5e5ac0",  /* vdivpd -64(%r10), %ymm2, %ymm11 */
    "62e1fd4c5e15f8ffffff",  /* vdivpd -8(%rip), %zmm0, %zmm18{%k4} */
    "62f1dd005ef2",  /* vdivpd %xmm2, %xmm20, %xmm6 */
    "c59a5ef9",  /* vdivss %xmm1, %xmm12, %xmm7 */
    "c5625e1c4d00100000",  /* vdivss 0x1000(,%rcx,2), %xmm3, %xmm11 */
    "624176185ef4",  /* vdivss {rn-sae}, %xmm12, %xmm1, %xmm30 */
    "62117e005ee1",  /* vdivss %xmm25, %xmm16, %xmm12 */
    "c5cb5ede",  /* vdivsd %xmm6, %xmm6, %xmm3 */
    "c4210b5e2c9d10000000",  /* vdivsd 0x10(,%r11,4), %xmm14, %xmm13 */
    "62b1ffd95ee9",  /* vdivsd {ru-sae}, %xmm17, %xmm0, %xmm5{%k1}{z} */
    "6221ef0b5eed",  /* vdivsd %xmm21, %xmm2, %xmm29{%k3} */
    "c5d85d1d00000000",  /* vminps 0(%rip), %xmm4, %xmm3 */
    "c5fc5dee",  /* vminps %ymm6, %ymm0, %ymm5 */
    "62b12c005d2c9500100000",  /* vminps 0x1000(,%r10,4), %xmm26, %xmm5 */
    "62d14c505d7424f0",  /* vminps -64(%r12){1to16}, %zmm22, %zmm6 */
    "c4c1515de4",  /* vminpd %xmm12, %xmm5, %xmm4 */
    "c401515d9cba80000000",  /* vminpd 128(%r10,%r15,4), %xmm5, %xmm11 */
    "62c1f5405dd5",  /* vminpd %zmm13, %zmm17, %zmm18 */
    "62b1dd3b5d2cc510000000",  /* vminpd 0x10(,%r8,8){1to4}, %ymm4, %ymm5{%k3} */
    "c4c15a5dec",  /* vminss %xmm12, %xmm4, %xmm5 */
    "c4412a5dda",  /* vminss %xmm10, %xmm10, %xmm11 */
    "62f116005ddd",  /* vminss %xmm5, %xmm29, %xmm3 */
    "62312e105de0",  /* vminss {sae}, %xmm16, %xmm26, %xmm12 */
    "c5b35d0cfd10000000",  /* vminsd 0x10(,%rdi,8), %xmm9, %xmm1 */
    "c5d35d34c510000000",  /* vminsd 0x10(,%rax,8), %xmm5, %xmm6 */
    "6241ff8d5de9",  /* vminsd %xmm9, %xmm0, %xmm29{%k5}{z} */
    "62b1c7005d742108",  /* vminsd 64(%rcx,%r12,1), %xmm23, %xmm6 */
    "c5c85fd2",  /* vmaxps %xmm2, %xmm6, %xmm2 */
    "c55c5fdf",  /* vmaxps %ymm7, %ymm4, %ymm11 */
    "62a164285ff9",  /* vmaxps %ymm17, %ymm3, %ymm23 */
    "6261444f5f1500000000",  /* vmaxps 0(%rip), %zmm7, %zmm26{%k7} */
    "c4c16d5fd9",  /* vmaxpd %ymm9, %ymm2, %ymm3 */
    "c4416d5fde",  /* vmaxpd %ymm14, %ymm2, %ymm11 */
    "6251ad505f73f8",  /* vmaxpd -64(%r11){1to8}, %zmm26, %zmm14 */
    "62e1c5495f349d10000000",  /* vmaxpd 0x10(,%rbx,4), %zmm7, %zmm22{%k1} */
    "c5725ff9",  /* vmaxss %xmm1, %xmm1, %xmm15 */
    "c421425f541908",  /* vmaxss 8(%rcx,%r11,1), %xmm7, %xmm10 */
    "6241161f5fdd",  /* vmaxss {sae}, %xmm13, %xmm13, %xmm27{%k7} */
    "62e12e085f1500000000",  /* vmaxss 0(%rip), %xmm10, %xmm18 */
    "c5fb5fa4f800100000",  /* vmaxsd 4096(%rax,%rdi,8), %xmm0, %xmm4 */
    "c56b5f1df8ffffff",  /* vmaxsd -8(%rip), %xmm2, %xmm11 */
    "62f1af975fd9",  /* vmaxsd {sae}, %xmm1, %xmm26, %xmm3{%k7}{z} */
    "6251ef185fee",  /* vmaxsd {sae}, %xmm14, %xmm2, %xmm13 */
    "c4c1785171c0",  /* vsqrtps -64(%r9), %xmm6 */
    "c57c51b000100000",  /* vsqrtps 4096(%rax), %ymm14 */
    "62e17c185167f0",  /* vsqrtps -64(%rdi){1to4}, %xmm20 */
    "c5fc51a000100000",  /* vsqrtps 4096(%rax), %ymm4 */
    "c4c17d518c0000f0ffff",  /* vsqrtpd -0x1000(%r8,%rax,1), %ymm1 */
    "c5fd51cb",  /* vsqrtpd %ymm3, %ymm1 */
    "62b1fdd9510cf0",  /* vsqrtpd (%rax,%r14,8){1to8}, %zmm1{%k1}{z} */
    "6241fd2851e0",  /* vsqrtpd %ymm8, %ymm28 */
    "c4c16251df",  /* vsqrtss %xmm15, %xmm3, %xmm3 */
    "c5ba51d9",  /* vsqrtss %xmm1, %xmm8, %xmm3 */
    "62b1560051248d00100000",  /* vsqrtss 0x1000(,%r9,4), %xmm21, %xmm4 */
    "6251769c51df",  /* vsqrtss {rn-sae}, %xmm15, %xmm1, %xmm11{%k4}{z} */
    "c5e351ab78563412",  /* vsqrtsd 0x12345678(%rbx), %xmm3, %xmm5 */
    "c58b5114ad00100000",  /* vsqrtsd 0x1000(,%rbp,4), %xmm14, %xmm2 */
    "6281df8951ef",  /* vsqrtsd %xmm31, %xmm4, %xmm21{%k1}{z} */
    "62919f8451cc",  /* vsqrtsd %xmm28, %xmm28, %xmm1{%k4}{z} */
    "c441105413",  /* vandps (%r11), %xmm13, %xmm10 */
    "c4c14854642480",  /* vandps -128(%r12), %xmm6, %xmm4 */
    "62b16c8a54c9",  /* vandps %xmm17, %xmm2, %xmm1{%k2}{z} */
    "62c17c2054d5",  /* vandps %ymm13, %ymm16, %ymm18 */
    "c54954c4",  /* vandpd %xmm4, %xmm6, %xmm8 */
    "c4c115547ac0",  /* vandpd -64(%r10), %ymm13, %ymm7 */
    "62c1b528546602",  /* vandpd 64(%r14), %ymm9, %ymm20 */
    "62e1852854247510000000",  /* vandpd 0x10(,%rsi,2), %ymm15, %ymm20 */
    "c5d055e3",  /* vandnps %xmm3, %xmm5, %xmm4 */
    "c4c10855cf",  /* vandnps %xmm15, %xmm14, %xmm1 */
    "62e10ca655cd",  /* vandnps %ymm5, %ymm30, %ymm17{%k6}{z} */
    "62715c3a553d00010000",  /* vandnps 0x100(%rip){1to8}, %ymm4, %ymm15{%k2} */
    "c5c155f1",  /* vandnpd %xmm1, %xmm7, %xmm6 */
    "c5ed5535f8ffffff",  /* vandnpd -8(%rip), %ymm2, %ymm6 */
    "62f1ad4f55e9",  /* vandnpd %zmm1, %zmm10, %zmm5{%k7} */
    "62c1cdca559f78563412",  /* vandnpd 0x12345678(%r15), %zmm6, %zmm19{%k2}{z} */
    "c5a0563cfd10000000",  /* vorps 0x10(,%rdi,8), %xmm11, %xmm7 */
    "c4c11856e2",  /* vorps %xmm10, %xmm12, %xmm4 */
    "6201342756fd",  /* vorps %ymm29, %ymm25, %ymm31{%k7} */
    "62d13426565804",  /* vorps 128(%r8), %ymm25, %ymm3{%k6} */
    "c4c1515634ec",  /* vorpd (%r12,%rbp,8), %xmm5, %xmm6 */
    "c569568078563412",  /* vorpd 0x12345678(%rax), %xmm2, %xmm8 */
    "6271dd00562df8ffffff",  /* vorpd -8(%rip), %xmm20, %xmm13 */
    "62218528563c4510000000",  /* vorpd 0x10(,%r8,2), %ymm15, %ymm31 */
    "c5c857cd",  /* vxorps %xmm5, %xmm6, %xmm1 */
    "c4014c57448f08",  /* vxorps 8(%r15,%r9,4), %ymm6, %ymm8 */
    "62b1140a57c4",  /* vxorps %xmm20, %xmm13, %xmm0{%k2} */
    "62f1744857c5",  /* vxorps %zmm5, %zmm1, %zmm0 */
    "c4c13d57d5",  /* vxorpd %ymm13, %ymm8, %ymm2 */
    "c4816557ac2680000000",  /* vxorpd 128(%r14,%r12,1), %ymm3, %ymm5 */
    "6211850f57ef",  /* vxorpd %xmm31, %xmm15, %xmm13{%k7} */
    "62c195405738",  /* vxorpd (%r8), %zmm29, %zmm23 */
    "c5e014c9",  /* vunpcklps %xmm1, %xmm3, %xmm1 */
    "c4813c145c7cc0",  /* vunpcklps -64(%r12,%r15,2), %ymm8, %ymm3 */
    "6221742814d5",  /* vunpcklps %ymm21, %ymm1, %ymm26 */
    "62f15cb6141d00010000",  /* vunpcklps 0x100(%rip){1to8}, %ymm20, %ymm3{%k6}{z} */
    "c4415c15dd",  /* vunpckhps %ymm13, %ymm4, %ymm11 */
    "c5f815d2",  /* vunpckhps %xmm2, %xmm0, %xmm2 */
    "6211242915da",  /* vunpckhps %ymm26, %ymm11, %ymm11{%k1} */
    "6241442215cf",  /* vunpckhps %ymm15, %ymm23, %ymm25{%k2} */
    "c4410914f0",  /* vunpcklpd %xmm8, %xmm14, %xmm14 */
    "c4c14914f4",  /* vunpcklpd %xmm12, %xmm6, %xmm6 */
    "62618d4a14f5",  /* vunpcklpd %zmm5, %zmm14, %zmm30{%k2} */
    "6261b5c214fc",  /* vunpcklpd %zmm4, %zmm25, %zmm31{%k2}{z} */
    "c545156c8500",  /* vunpckhpd (%rbp,%rax,4), %ymm7, %ymm13 */
    "c50d152df8ffffff",  /* vunpckhpd -8(%rip), %ymm14, %ymm13 */
    "62c1ed48157dfe",  /* vunpckhpd -128(%r13), %zmm2, %zmm23 */
    "62e1a53b1557f0",  /* vunpckhpd -128(%rdi){1to4}, %ymm11, %ymm18{%k3} */
    "c4c12cc2f5e0",  /* vcmpps $224, %ymm13, %ymm10, %ymm6 */
    "c54cc2037a",  /* vcmpps $122, (%rbx), %ymm6, %ymm8 */
    "c549c2d08f",  /* vcmppd $143, %xmm0, %xmm6, %xmm10 */
    "c511c21d0000000071",  /* vcmppd $113, 0(%rip), %xmm13, %xmm11 */
    "c42132c28c4b80000000db",  /* vcmpss $219, 128(%rbx,%r9,2), %xmm9, %xmm9 */
    "c552c225000000004c",  /* vcmpss $76, 0(%rip), %xmm5, %xmm12 */
    "c5e3c214c5001000003f",  /* vcmpsd $63, 0x1000(,%rax,8), %xmm3, %xmm2 */
    "c533c2f577",  /* vcmpsd $119, %xmm5, %xmm9, %xmm14 */
    "62b11450c2a4197856341204",  /* vcmpps $4, 0x12345678(%rcx,%r11,1){1to16}, %zmm29, %k4 */
    "62d10c40c2c65e",  /* vcmpps $94, %zmm14, %zmm30, %k0 */
    "62f1bd48c2fa7e",  /* vcmppd $126, %zmm2, %zmm8, %k7 */
    "62f11608c22d00010000b9",  /* vcmpss $185, 0x100(%rip), %xmm13, %k5 */
    "62f11600c2d0cb",  /* vcmpss $203, %xmm0, %xmm29, %k2 */
    "62f1ff00c2fbd0",  /* vcmpsd $208, %xmm3, %xmm16, %k7 */
    "62f1d708c224ad10000000f3",  /* vcmpsd $243, 0x10(,%rbp,4), %xmm5, %k4 */
    "c4c1782ffa",  /* vcomiss %xmm10, %xmm7 */
    "c441782fd4",  /* vcomiss %xmm12, %xmm10 */
    "62017c182feb",  /* vcomiss {sae}, %xmm27, %xmm29 */
    "c4a1782f0c7510000000",  /* vcomiss 0x10(,%r14,2), %xmm1 */
    "c5782edf",  /* vucomiss %xmm7, %xmm11 */
    "c4a1782e24a500100000",  /* vucomiss 0x1000(,%r12,4), %xmm4 */
    "c4c1782ea580000000",  /* vucomiss 128(%r13), %xmm4 */
    "62e17c082e3500010000",  /* vucomiss 0x100(%rip), %xmm22 */
    "c4a1792f946200100000",  /* vcomisd 4096(%rdx,%r12,2), %xmm2 */
    "c5792ff1",  /* vcomisd %xmm1, %xmm14 */
    "6221fd082fc2",  /* vcomisd %xmm18, %xmm24 */
    "6281fd082fa4b800f0ffff",  /* vcomisd -0x1000(%r8,%r15,4), %xmm20 */
    "c4c1792e4880",  /* vucomisd -128(%r8), %xmm1 */
    "c5792ef9",  /* vucomisd %xmm1, %xmm15 */
    "6291fd082ef3",  /* vucomisd %xmm27, %xmm6 */
    "6261fd082ea45d00100000",  /* vucomisd 4096(%rbp,%rbx,2), %xmm28 */
    "c550c61d0000000077",  /* vshufps $119, 0(%rip), %xmm5, %xmm11 */
    "c5ccc61500000000db",  /* vshufps $219, 0(%rip), %ymm6, %ymm2 */
    "c48141c6acb400f0ffffe7",  /* vshufpd $231, -0x1000(%r12,%r14,4), %xmm7, %xmm5 */
    "c4c131c6d9ec",  /* vshufpd $236, %xmm9, %xmm9, %xmm3 */
    "62915c58c68cc000100000cc",  /* vshufps $204, 4096(%r8,%r8,8){1to16}, %zmm4, %zmm1 */
    "62b14ca7c6b4450010000069",  /* vshufps $105, 4096(%rbp,%r8,2), %ymm22, %ymm6{%k7}{z} */
    "6221fd40c67cb30131",  /* vshufpd $49, 64(%rbx,%r14,4), %zmm16, %zmm31 */
    "62c1b538c66491f0ca",  /* vshufpd $202, -128(%r9,%rdx,4){1to4}, %ymm9, %ymm20 */
    "c57c29e8",  /* vmovaps %ymm13, %ymm0 */
    "c4217c2834cd00100000",  /* vmovaps 0x1000(,%r9,8), %ymm14 */
    "c4417c291c98",  /* vmovaps %ymm11, (%r8,%rbx,4) */
    "c57828e6",  /* vmovaps %xmm6, %xmm12 */
    "62717c8b28748704",  /* vmovaps 64(%rdi,%rax,4), %xmm14{%k3}{z} */
    "62817cad28c5",  /* vmovaps %ymm29, %ymm16{%k5}{z} */
    "62c17c4828fa",  /* vmovaps %zmm10, %zmm23 */
    "62d17c492979c0",  /* vmovaps %zmm7, -0x1000(%r9){%k1} */
    "c57c10d2",  /* vmovups %ymm2, %ymm10 */
    "c5fc10549180",  /* vmovups -128(%rcx,%rdx,4), %ymm2 */
    "c578113df8ffffff",  /* vmovups %xmm15, -8(%rip) */
    "c44178119b80000000",  /* vmovups %xmm11, 128(%r11) */
    "62817c2810c3",  /* vmovups %ymm27, %ymm16 */
    "62b17c0f106486f8",  /* vmovups -128(%rsi,%r8,4), %xmm4{%k7} */
    "62e17c0811ac7778563412",  /* vmovups %xmm21, 0x12345678(%rdi,%rsi,2) */
    "c4417810c0",  /* vmovups %xmm8, %xmm8 */
    "c57929d4",  /* vmovapd %xmm10, %xmm4 */
    "c57d29e3",  /* vmovapd %ymm12, %ymm3 */
    "c42179298c2300f0ffff",  /* vmovapd %xmm9, -0x1000(%rbx,%r12,1) */
    "c4a1792934dd10000000",  /* vmovapd %xmm6, 0x10(,%r11,8) */
    "6241fdcf28e4",  /* vmovapd %zmm12, %zmm28{%k7}{z} */
    "6281fd4828da",  /* vmovapd %zmm26, %zmm19 */
    "62e1fdca28c2",  /* vmovapd %zmm2, %zmm16{%k2}{z} */
    "62a1fdca28fb",  /* vmovapd %zmm19, %zmm23{%k2}{z} */
    "c4c179106ac0",  /* vmovupd -64(%r10), %xmm5 */
    "c57d11f9",  /* vmovupd %ymm15, %ymm1 */
    "c57d112df8ffffff",  /* vmovupd %ymm13, -8(%rip) */
    "c44179119000100000",  /* vmovupd %xmm10, 4096(%r8) */
    "62e1fd08102df8ffffff",  /* vmovupd -8(%rip), %xmm21 */
    "c57d10c2",  /* vmovupd %ymm2, %ymm8 */
    "c57d113d00010000",  /* vmovupd %ymm15, 0x100(%rip) */
    "62e1fd0d1105f8ffffff",  /* vmovupd %xmm16, -8(%rip){%k5} */
    "c57d6fda",  /* vmovdqa %ymm2, %ymm11 */
    "c4217d6f9c7378563412",  /* vmovdqa 0x12345678(%rbx,%r14,2), %ymm11 */
    "c4a1797fb44f78563412",  /* vmovdqa %xmm6, 0x12345678(%rdi,%r9,2) */
    "c441796fee",  /* vmovdqa %xmm14, %xmm13 */
    "c4c17e6f44cec0",  /* vmovdqu -64(%r14,%rcx,8), %ymm0 */
    "c5fe6fdb",  /* vmovdqu %ymm3, %ymm3 */
    "c5fa7f4108",  /* vmovdqu %xmm0, 8(%rcx) */
    "c57e7f34fd00100000",  /* vmovdqu %ymm14, 0x1000(,%rdi,8) */
    "62f17ecb6f349d00100000",  /* vmovdqu32 0x1000(,%rbx,4), %zmm6{%k3}{z} */
    "62517e486fcb",  /* vmovdqu32 %zmm11, %zmm9 */
    "62217e8d6fca",  /* vmovdqu32 %xmm18, %xmm25{%k5}{z} */
    "62617e0d7f07",  /* vmovdqu32 %xmm24, (%rdi){%k5} */
    "6251fe2a6fc5",  /* vmovdqu64 %ymm13, %ymm8{%k2} */
    "6201fe286ff1",  /* vmovdqu64 %ymm25, %ymm30 */
    "6251fe086fde",  /* vmovdqu64 %xmm14, %xmm11 */
    "6241fe286fd0",  /* vmovdqu64 %ymm8, %ymm26 */
    "62a17d8a6ff5",  /* vmovdqa32 %xmm21, %xmm22{%k2}{z} */
    "62a17d286f7c6880",  /* vmovdqa32 -0x1000(%rax,%r13,2), %ymm23 */
    "62717d4d7f25f8ffffff",  /* vmovdqa32 %zmm12, -8(%rip){%k5} */
    "62d17d086fda",  /* vmovdqa32 %xmm10, %xmm3 */
    "6231fd086ffa",  /* vmovdqa64 %xmm18, %xmm15 */
    "6211fd486ff0",  /* vmovdqa64 %zmm24, %zmm14 */
    "6211fd486fd8",  /* vmovdqa64 %zmm24, %zmm11 */
    "6251fdcc6fc2",  /* vmovdqa64 %zmm10, %zmm8{%k4}{z} */
    "62017f2f6ffc",  /* vmovdqu8 %ymm28, %ymm31{%k7} */
    "62617f4a6fc9",  /* vmovdqu8 %zmm1, %zmm25{%k2} */
    "62417f0e7f7208",  /* vmovdqu8 %xmm30, 128(%r10){%k6} */
    "62617fae6fd9",  /* vmovdqu8 %ymm1, %ymm27{%k6}{z} */
    "6241ff0d6fb48b78563412",  /* vmovdqu16 0x12345678(%r11,%rcx,4), %xmm30{%k5} */
    "6261ffae6f9cf008000000",  /* vmovdqu16 8(%rax,%rsi,8), %ymm27{%k6}{z} */
    "6291ff486fc0",  /* vmovdqu16 %zmm24, %zmm0 */
    "6281ff0b6fc1",  /* vmovdqu16 %xmm25, %xmm16{%k3} */
    "c4627d18c5",  /* vbroadcastss %xmm5, %ymm8 */
    "c4e27d18349d10000000",  /* vbroadcastss 0x10(,%rbx,4), %ymm6 */
    "62d27d0a18d9",  /* vbroadcastss %xmm9, %xmm3{%k2} */
    "62a27dc91824ed00100000",  /* vbroadcastss 0x1000(,%r13,8), %zmm20{%k1}{z} */
    "c4627d19641e80",  /* vbroadcastsd -128(%rsi,%rbx,1), %ymm12 */
    "c4e27d192ced10000000",  /* vbroadcastsd 0x10(,%rbp,8), %ymm5 */
    "6272fd48192500010000",  /* vbroadcastsd 0x100(%rip), %zmm12 */
    "6242fdae194d08",  /* vbroadcastsd 64(%r13), %ymm25{%k6}{z} */
    "c4e27978f1",  /* vpbroadcastb %xmm1, %xmm6 */
    "c4e27978d0",  /* vpbroadcastb %xmm0, %xmm2 */
    "c4e27979e9",  /* vpbroadcastw %xmm1, %xmm5 */
    "c4c27979f6",  /* vpbroadcastw %xmm14, %xmm6 */
    "c4c27958fb",  /* vpbroadcastd %xmm11, %xmm7 */
    "c4427958eb",  /* vpbroadcastd %xmm11, %xmm13 */
    "c46279593d00000000",  /* vpbroadcastq 0(%rip), %xmm15 */
    "c4c27d59dc",  /* vpbroadcastq %xmm12, %ymm3 */
    "62d27dcb78fa",  /* vpbroadcastb %xmm10, %zmm7{%k3}{z} */
    "62027d2878c5",  /* vpbroadcastb %xmm29, %ymm24 */
    "62d27d0a79d8",  /* vpbroadcastw %xmm8, %xmm3{%k2} */
    "62527d09798200f0ffff",  /* vpbroadcastw -0x1000(%r10), %xmm8{%k1} */
    "62f27d8d58db",  /* vpbroadcastd %xmm3, %xmm3{%k5}{z} */
    "62027dca58eb",  /* vpbroadcastd %xmm27, %zmm29{%k2}{z} */
    "6242fd0859b500100000",  /* vpbroadcastq 4096(%r13), %xmm30 */
    "6232fd4859348d10000000",  /* vpbroadcastq 0x10(,%r9,4), %zmm14 */
    "c4e25d98f5",  /* vfmadd132ps %ymm5, %ymm4, %ymm6 */
    "c4c27998f2",  /* vfmadd132ps %xmm10, %xmm0, %xmm6 */
    "62a23d7f98e6",  /* vfmadd132ps {rz-sae}, %zmm22, %zmm8, %zmm20{%k7} */
    "62624520984efc",  /* vfmadd132ps -128(%rsi), %ymm23, %ymm25 */
    "c4c2c198ee",  /* vfmadd132pd %xmm14, %xmm7, %xmm5 */
    "c442a998c3",  /* vfmadd132pd %xmm11, %xmm10, %xmm8 */
    "62d2ad3098443cf8",  /* vfmadd132pd -64(%r12,%rdi,1){1to4}, %ymm26, %ymm0 */
    "6232e50f98f7",  /* vfmadd132pd %xmm23, %xmm3, %xmm14{%k7} */
    "c46269993500010000",  /* vfmadd132ss 0x100(%rip), %xmm2, %xmm14 */
    "c46201992df8ffffff",  /* vfmadd132ss -8(%rip), %xmm15, %xmm13 */
    "62726d7d99c6",  /* vfmadd132ss {rz-sae}, %xmm6, %xmm2, %xmm8{%k5} */
    "6262550099d2",  /* vfmadd132ss %xmm2, %xmm21, %xmm26 */
    "c4c28999ca",  /* vfmadd132sd %xmm10, %xmm14, %xmm1 */
    "c4c2b1999b78563412",  /* vfmadd132sd 0x12345678(%r11), %xmm9, %xmm3 */
    "6262a508997020",  /* vfmadd132sd 256(%rax), %xmm11, %xmm30 */
    "62f2fd0599349d00100000",  /* vfmadd132sd 0x1000(,%rbx,4), %xmm16, %xmm6{%k5} */
    "c4c21da8d9",  /* vfmadd213ps %ymm9, %ymm12, %ymm3 */
    "c4620da81d00010000",  /* vfmadd213ps 0x100(%rip), %ymm14, %ymm11 */
    "62925528a8e1",  /* vfmadd213ps %ymm25, %ymm5, %ymm4 */
    "62420d8fa89a78563412",  /* vfmadd213ps 0x12345678(%r10), %xmm14, %xmm27{%k7}{z} */
    "c46281a83d00010000",  /* vfmadd213pd 0x100(%rip), %xmm15, %xmm15 */
    "c46281a8148500100000",  /* vfmadd213pd 0x1000(,%rax,4), %xmm15, %xmm10 */
    "6232dd08a8e3",  /* vfmadd213pd %xmm19, %xmm4, %xmm12 */
    "62928d28a8fe",  /* vfmadd213pd %ymm30, %ymm14, %ymm7 */
    "c42251a98cef00100000",  /* vfmadd213ss 4096(%rdi,%r13,8), %xmm5, %xmm9 */
    "c44229a9c1",  /* vfmadd213ss %xmm9, %xmm10, %xmm8 */
    "62724d84a92c7510000000",  /* vfmadd213ss 0x10(,%rsi,2), %xmm22, %xmm13{%k4}{z} */
    "62620d0da9c2",  /* vfmadd213ss %xmm2, %xmm14, %xmm24{%k5} */
    "c422f1a914c510000000",  /* vfmadd213sd 0x10(,%r8,8), %xmm1, %xmm10 */
    "c4e2f1a93c9500100000",  /* vfmadd213sd 0x1000(,%rdx,4), %xmm1, %xmm7 */
    "62729d82a96020",  /* vfmadd213sd 256(%rax), %xmm28, %xmm12{%k2}{z} */
    "c462d1a93500000000",  /* vfmadd213sd 0(%rip), %xmm5, %xmm14 */
    "c4c261b8da",  /* vfmadd231ps %xmm10, %xmm3, %xmm3 */
    "c46279b824fd00100000",  /* vfmadd231ps 0x1000(,%rdi,8), %xmm0, %xmm12 */
    "62626dcbb80c4d00100000",  /* vfmadd231ps 0x1000(,%rcx,2), %zmm2, %zmm25{%k3}{z} */
    "62e26d48b8d2",  /* vfmadd231ps %zmm2, %zmm2, %zmm18 */
    "c442a1b822",  /* vfmadd231pd (%r10), %xmm11, %xmm12 */
    "c4e2f1b8d3",  /* vfmadd231pd %xmm3, %xmm1, %xmm2 */
    "6282858cb8e5",  /* vfmadd231pd %xmm29, %xmm15, %xmm20{%k4}{z} */
    "62e2f5b4b81500000000",  /* vfmadd231pd 0(%rip){1to4}, %ymm17, %ymm18{%k4}{z} */
    "c4c219b9c5",  /* vfmadd231ss %xmm13, %xmm12, %xmm0 */
    "c4c269b9f9",  /* vfmadd231ss %xmm9, %xmm2, %xmm7 */
    "62a23d83b9f2",  /* vfmadd231ss %xmm18, %xmm24, %xmm22{%k3}{z} */
    "6252250fb9e7",  /* vfmadd231ss %xmm15, %xmm11, %xmm12{%k7} */
    "c4e2c9b9f4",  /* vfmadd231sd %xmm4, %xmm6, %xmm6 */
    "c4c2c9b9d7",  /* vfmadd231sd %xmm15, %xmm6, %xmm2 */
    "62c2ed0eb9e9",  /* vfmadd231sd %xmm9, %xmm2, %xmm21{%k6} */
    "62e28d89b9e5",  /* vfmadd231sd %xmm5, %xmm14, %xmm20{%k1}{z} */
    "c462019a0df8ffffff",  /* vfmsub132ps -8(%rip), %xmm15, %xmm9 */
    "c4c2419a547508",  /* vfmsub132ps 8(%r13,%rsi,2), %xmm7, %xmm2 */
    "62e27d3f9a741fe0",  /* vfmsub132ps -128(%rdi,%rbx,1){1to8}, %ymm0, %ymm22{%k7} */
    "62e255409a7740",  /* vfmsub132ps 4096(%rdi), %zmm21, %zmm22 */
    "c4e2e19a5cad08",  /* vfmsub132pd 8(%rbp,%rbp,4), %xmm3, %xmm3 */
    "c4629d9a65c0",  /* vfmsub132pd -64(%rbp), %ymm12, %ymm12 */
    "6272e54d9a348d10000000",  /* vfmsub132pd 0x10(,%rcx,4), %zmm3, %zmm14{%k5} */
    "c4628d9a6f40",  /* vfmsub132pd 64(%rdi), %ymm14, %ymm13 */
    "c462499b2cad00100000",  /* vfmsub132ss 0x1000(,%rbp,4), %xmm6, %xmm13 */
    "c4a2799b1ced10000000",  /* vfmsub132ss 0x10(,%r13,8), %xmm0, %xmm3 */
    "62a21d009b2cfd10000000",  /* vfmsub132ss 0x10(,%r15,8), %xmm28, %xmm21 */
    "622265089bcd",  /* vfmsub132ss %xmm21, %xmm3, %xmm25 */
    "c4c2e19bf0",  /* vfmsub132sd %xmm8, %xmm3, %xmm6 */
    "c462d99b4e40",  /* vfmsub132sd 64(%rsi), %xmm4, %xmm9 */
    "62e2cd089b2d00000000",  /* vfmsub132sd 0(%rip), %xmm6, %xmm21 */
    "62e2a5089b5ccaf8",  /* vfmsub132sd -64(%rdx,%rcx,8), %xmm11, %xmm19 */
    "c46275aa149510000000",  /* vfmsub213ps 0x10(,%rdx,4), %ymm1, %ymm10 */
    "c4e275aa1500000000",  /* vfmsub213ps 0(%rip), %ymm1, %ymm2 */
    "62820500aaff",  /* vfmsub213ps %xmm31, %xmm31, %xmm23 */
    "625235d1aa6102",  /* vfmsub213ps 8(%r9){1to16}, %zmm25, %zmm12{%k1}{z} */
    "c48285aabc2700100000",  /* vfmsub213pd 4096(%r15,%r12,1), %ymm15, %ymm7 */
    "c462c1aaf1",  /* vfmsub213pd %xmm1, %xmm7, %xmm14 */
    "c442bdaae9",  /* vfmsub213pd %ymm9, %ymm8, %ymm13 */
    "62e2c500aac2",  /* vfmsub213pd %xmm2, %xmm23, %xmm16 */
    "c46201abd7",  /* vfmsub213ss %xmm7, %xmm15, %xmm10 */
    "c44229abda",  /* vfmsub213ss %xmm10, %xmm10, %xmm11 */
    "62724d00ab0cad00100000",  /* vfmsub213ss 0x1000(,%rbp,4), %xmm22, %xmm9 */
    "62326589abf0",  /* vfmsub213ss %xmm16, %xmm3, %xmm14{%k1}{z} */
    "c4e2a1ab741340",  /* vfmsub213sd 64(%rbx,%rdx,1), %xmm11, %xmm6 */
    "c4c2a1abfb",  /* vfmsub213sd %xmm11, %xmm11, %xmm7 */
    "c4e299aba800100000",  /* vfmsub213sd 4096(%rax), %xmm12, %xmm5 */
    "6242a508ab9000f0ffff",  /* vfmsub213sd -0x1000(%r8), %xmm11, %xmm26 */
    "c4224dba1ce510000000",  /* vfmsub231ps 0x10(,%r12,8), %ymm6, %ymm11 */
    "c4c221bae3",  /* vfmsub231ps %xmm11, %xmm11, %xmm4 */
    "62725525ba2cb510000000",  /* vfmsub231ps 0x10(,%rsi,4), %ymm21, %ymm13{%k5} */
    "62e23d20baca",  /* vfmsub231ps %ymm2, %ymm24, %ymm17 */
    "c462b1ba15f8ffffff",  /* vfmsub231pd -8(%rip), %xmm9, %xmm10 */
    "c4e2a1bac4",  /* vfmsub231pd %xmm4, %xmm11, %xmm0 */
    "62f2ada4baf8",  /* vfmsub231pd %ymm0, %ymm26, %ymm7{%k4}{z} */
    "6272c529bad3",  /* vfmsub231pd %ymm3, %ymm7, %ymm10{%k1} */
    "c4e241bbe8",  /* vfmsub231ss %xmm0, %xmm7, %xmm5 */
    "c4c251bbdb",  /* vfmsub231ss %xmm11, %xmm5, %xmm3 */
    "6282758cbbf3",  /* vfmsub231ss %xmm27, %xmm1, %xmm22{%k4}{z} */
    "62b25d06bb443ef0",  /* vfmsub231ss -64(%rsi,%r15,1), %xmm20, %xmm0{%k6} */
    "c4e2b1bbca",  /* vfmsub231sd %xmm2, %xmm9, %xmm1 */
    "c462e9bbd0",  /* vfmsub231sd %xmm0, %xmm2, %xmm10 */
    "6272c58abb1df8ffffff",  /* vfmsub231sd -8(%rip), %xmm7, %xmm11{%k2}{z} */
    "62e29584bb0d00000000",  /* vfmsub231sd 0(%rip), %xmm29, %xmm17{%k4}{z} */
    "c442719cf6",  /* vfnmadd132ps %xmm14, %xmm1, %xmm14 */
    "c462759c1c6d00100000",  /* vfnmadd132ps 0x1000(,%rbp,2), %ymm1, %ymm11 */
    "62f255409c5804",  /* vfnmadd132ps 256(%rax), %zmm21, %zmm3 */
    "62725d489ce3",  /* vfnmadd132ps %zmm3, %zmm4, %zmm12 */
    "c4e2ed9cd0",  /* vfnmadd132pd %ymm0, %ymm2, %ymm2 */
    "c4e2a99cce",  /* vfnmadd132pd %xmm6, %xmm10, %xmm1 */
    "6222b5109c145510000000",  /* vfnmadd132pd 0x10(,%r10,2){1to2}, %xmm25, %xmm26 */
    "6232ad589c34cd10000000",  /* vfnmadd132pd 0x10(,%r9,8){1to8}, %zmm10, %zmm14 */
    "c4e2719d1d00010000",  /* vfnmadd132ss 0x100(%rip), %xmm1, %xmm3 */
    "c4e2299dda",  /* vfnmadd132ss %xmm2, %xmm10, %xmm3 */
    "6232050d9d144510000000",  /* vfnmadd132ss 0x10(,%r8,2), %xmm15, %xmm10{%k5} */
    "62c245039d7440f0",  /* vfnmadd132ss -64(%r8,%rax,2), %xmm23, %xmm22{%k3} */
    "c462a99dda",  /* vfnmadd132sd %xmm2, %xmm10, %xmm11 */
    "c422899d1c5d00100000",  /* vfnmadd132sd 0x1000(,%r11,2), %xmm14, %xmm11 */
    "6282dd709dd1",  /* vfnmadd132sd {rz-sae}, %xmm25, %xmm20, %xmm18 */
    "62c2958c9d5c3201",  /* vfnmadd132sd 8(%r10,%rsi,1), %xmm13, %xmm19{%k4}{z} */
    "c4e271ac0500010000",  /* vfnmadd213ps 0x100(%rip), %xmm1, %xmm0 */
    "c4624dac1d00010000",  /* vfnmadd213ps 0x100(%rip), %ymm6, %ymm11 */
    "62b2458eac3439",  /* vfnmadd213ps (%rcx,%r15,1), %xmm7, %xmm6{%k6}{z} */
    "c46225ac04b510000000",  /* vfnmadd213ps 0x10(,%rsi,4), %ymm11, %ymm8 */
    "c4c2d1ace8",  /* vfnmadd213pd %xmm8, %xmm5, %xmm5 */
    "c462d9acd6",  /* vfnmadd213pd %xmm6, %xmm4, %xmm10 */
    "62729da4acec",  /* vfnmadd213pd %ymm4, %ymm28, %ymm13{%k4}{z} */
    "6252fd46acdc",  /* vfnmadd213pd %zmm12, %zmm16, %zmm11{%k6} */
    "c4c261adf6",  /* vfnmadd213ss %xmm14, %xmm3, %xmm6 */
    "c4e211ad35f8ffffff",  /* vfnmadd213ss -8(%rip), %xmm13, %xmm6 */
    "62f24500ad1500000000",  /* vfnmadd213ss 0(%rip), %xmm23, %xmm2 */
    "62621508adc9",  /* vfnmadd213ss %xmm1, %xmm13, %xmm25 */
    "c462f9add1",  /* vfnmadd213sd %xmm1, %xmm0, %xmm10 */
    "c4e2a9ad3500000000",  /* vfnmadd213sd 0(%rip), %xmm10, %xmm6 */
    "62e2bd89ad43f0",  /* vfnmadd213sd -128(%rbx), %xmm8, %xmm16{%k1}{z} */
    "6202858fade0",  /* vfnmadd213sd %xmm24, %xmm15, %xmm28{%k7}{z} */
    "c4c201bca578563412",  /* vfnmadd231ps 0x12345678(%r13), %xmm15, %xmm4 */
    "c4c27dbc75c0",  /* vfnmadd231ps -64(%r13), %ymm0, %ymm6 */
    "62c20d0bbce0",  /* vfnmadd231ps %xmm8, %xmm14, %xmm20{%k3} */
    "62025d20bce9",  /* vfnmadd231ps %ymm25, %ymm20, %ymm29 */
    "c462d5bcf5",  /* vfnmadd231pd %ymm5, %ymm5, %ymm14 */
    "c442ddbc9678563412",  /* vfnmadd231pd 0x12345678(%r14), %ymm4, %ymm10 */
    "c462cdbc9800010000",  /* vfnmadd231pd 256(%rax), %ymm6, %ymm11 */
    "6242c543bceb",  /* vfnmadd231pd %zmm11, %zmm23, %zmm29{%k3} */
    "c4c259bdd8",  /* vfnmadd231ss %xmm8, %xmm4, %xmm3 */
    "c48211bd948400010000",  /* vfnmadd231ss 256(%r12,%r8,4), %xmm13, %xmm2 */
    "6232158ebdfc",  /* vfnmadd231ss %xmm20, %xmm13, %xmm15{%k6}{z} */
    "62525d07bd6a02",  /* vfnmadd231ss 8(%r10), %xmm20, %xmm13{%k7} */
    "c442e1bdc4",  /* vfnmadd231sd %xmm12, %xmm3, %xmm8 */
    "c4c2b1bd7140",  /* vfnmadd231sd 64(%r9), %xmm9, %xmm6 */
    "6242d500bd8878563412",  /* vfnmadd231sd 0x12345678(%r8), %xmm21, %xmm25 */
    "c462c9bdd7",  /* vfnmadd231sd %xmm7, %xmm6, %xmm10 */
    "c462359ee7",  /* vfnmsub132ps %ymm7, %ymm9, %ymm12 */
    "c4e23d9e1cb500100000",  /* vfnmsub132ps 0x1000(,%rsi,4), %ymm8, %ymm3 */
    "62222d209e0ca500100000",  /* vfnmsub132ps 0x1000(,%r12,4), %ymm26, %ymm25 */
    "62b2052d9efd",  /* vfnmsub132ps %ymm21, %ymm15, %ymm7{%k5} */
    "c442f59ec1",  /* vfnmsub132pd %ymm9, %ymm1, %ymm8 */
    "c4c2959e4c8608",  /* vfnmsub132pd 8(%r14,%rax,4), %ymm13, %ymm1 */
    "62a28d879e4c3804",  /* vfnmsub132pd 64(%rax,%r15,1), %xmm30, %xmm17{%k7}{z} */
    "c4c2bd9ed7",  /* vfnmsub132pd %ymm15, %ymm8, %ymm2 */
    "c4e2119ff6",  /* vfnmsub132ss %xmm6, %xmm13, %xmm6 */
    "c4a2799f3cb510000000",  /* vfnmsub132ss 0x10(,%r14,4), %xmm0, %xmm7 */
    "620235009f649502",  /* vfnmsub132ss 8(%r13,%r10,4), %xmm25, %xmm28 */
    "62625d089f2d00000000",  /* vfnmsub132ss 0(%rip), %xmm4, %xmm29 */
    "c4e2999fee",  /* vfnmsub132sd %xmm6, %xmm12, %xmm5 */
    "c442d19f9cc000010000",  /* vfnmsub132sd 256(%r8,%rax,8), %xmm5, %xmm11 */
    "62f2a5819ffe",  /* vfnmsub132sd %xmm6, %xmm27, %xmm7{%k1}{z} */
    "62b2ad859fcc",  /* vfnmsub132sd %xmm20, %xmm26, %xmm1{%k5}{z} */
    "c46261aef8",  /* vfnmsub213ps %xmm0, %xmm3, %xmm15 */
    "c46221aef1",  /* vfnmsub213ps %xmm1, %xmm11, %xmm14 */
    "c4e225ae2df8ffffff",  /* vfnmsub213ps -8(%rip), %ymm11, %ymm5 */
    "62225d40ae9c8a08000000",  /* vfnmsub213ps 8(%rdx,%r9,4), %zmm20, %zmm27 */
    "c4a29dae1cfd00100000",  /* vfnmsub213pd 0x1000(,%r15,8), %ymm12, %ymm3 */
    "c442e9aed2",  /* vfnmsub213pd %xmm10, %xmm2, %xmm10 */
    "6262fd28ae3d00010000",  /* vfnmsub213pd 0x100(%rip), %ymm0, %ymm31 */
    "62f2c58baefd",  /* vfnmsub213pd %xmm5, %xmm7, %xmm7{%k3}{z} */
    "c4e259af5c4840",  /* vfnmsub213ss 64(%rax,%rcx,2), %xmm4, %xmm3 */
    "c46241afdc",  /* vfnmsub213ss %xmm4, %xmm7, %xmm11 */
    "6232358eaffe",  /* vfnmsub213ss %xmm22, %xmm9, %xmm15{%k6}{z} */
    "62926d8eafcf",  /* vfnmsub213ss %xmm31, %xmm2, %xmm1{%k6}{z} */
    "c462f1aff0",  /* vfnmsub213sd %xmm0, %xmm1, %xmm14 */
    "c462d1af0500010000",  /* vfnmsub213sd 0x100(%rip), %xmm5, %xmm8 */
    "c4e2d1af2b",  /* vfnmsub213sd (%rbx), %xmm5, %xmm5 */
    "62428d08af9378563412",  /* vfnmsub213sd 0x12345678(%r11), %xmm14, %xmm26 */
    "c46215bec0",  /* vfnmsub231ps %ymm0, %ymm13, %ymm8 */
    "c4e25dbe29",  /* vfnmsub231ps (%rcx), %ymm4, %ymm5 */
    "62e26da6bee0",  /* vfnmsub231ps %ymm0, %ymm18, %ymm20{%k6}{z} */
    "62725548be2500000000",  /* vfnmsub231ps 0(%rip), %zmm5, %zmm12 */
    "c422f1be3ccd10000000",  /* vfnmsub231pd 0x10(,%r9,8), %xmm1, %xmm15 */
    "c422fdbe1ce500100000",  /* vfnmsub231pd 0x1000(,%r12,8), %ymm0, %ymm11 */
    "6202ad0cbee4",  /* vfnmsub231pd %xmm28, %xmm10, %xmm28{%k4} */
    "62029d28bec5",  /* vfnmsub231pd %ymm29, %ymm12, %ymm24 */
    "c46231bfe3",  /* vfnmsub231ss %xmm3, %xmm9, %xmm12 */
    "c4a209bf74fe08",  /* vfnmsub231ss 8(%rsi,%r15,8), %xmm14, %xmm6 */
    "62627d00bf2500010000",  /* vfnmsub231ss 0x100(%rip), %xmm16, %xmm28 */
    "62f25d00bf3c4500100000",  /* vfnmsub231ss 0x1000(,%rax,2), %xmm20, %xmm7 */
    "c442c1bfd4",  /* vfnmsub231sd %xmm12, %xmm7, %xmm10 */
    "c44281bfd3",  /* vfnmsub231sd %xmm11, %xmm15, %xmm10 */
    "62628d84bfd7",  /* vfnmsub231sd %xmm7, %xmm30, %xmm26{%k4}{z} */
    "c442f9bf440108",  /* vfnmsub231sd 8(%r9,%rax,1), %xmm0, %xmm8 */
    "c4621996740540",  /* vfmaddsub132ps 64(%rbp,%rax,1), %xmm12, %xmm14 */
    "c4c24196f7",  /* vfmaddsub132ps %xmm15, %xmm7, %xmm6 */
    "62723d229605f8ffffff",  /* vfmaddsub132ps -8(%rip), %ymm24, %ymm8{%k2} */
    "62a24d0e960c7d10000000",  /* vfmaddsub132ps 0x10(,%r15,2), %xmm6, %xmm17{%k6} */
    "c462a996c1",  /* vfmaddsub132pd %xmm1, %xmm10, %xmm8 */
    "c4e2fd969800100000",  /* vfmaddsub132pd 4096(%rax), %ymm0, %ymm3 */
    "62f2b540963df8ffffff",  /* vfmaddsub132pd -8(%rip), %zmm25, %zmm7 */
    "6272a52496d4",  /* vfmaddsub132pd %ymm4, %ymm27, %ymm10{%k4} */
    "c4c255a6ca",  /* vfmaddsub213ps %ymm10, %ymm5, %ymm1 */
    "c4621da6f4",  /* vfmaddsub213ps %ymm4, %ymm12, %ymm14 */
    "62123d40a6ee",  /* vfmaddsub213ps %zmm30, %zmm24, %zmm13 */
    "62525525a62f",  /* vfmaddsub213ps (%r15), %ymm21, %ymm13{%k5} */
    "c4c2a9a63e",  /* vfmaddsub213pd (%r14), %xmm10, %xmm7 */
    "c462c5a615f8ffffff",  /* vfmaddsub213pd -8(%rip), %ymm7, %ymm10 */
    "62e2bdaca6946800100000",  /* vfmaddsub213pd 4096(%rax,%rbp,2), %ymm8, %ymm18{%k4}{z} */
    "6232952ca62ced10000000",  /* vfmaddsub213pd 0x10(,%r13,8), %ymm13, %ymm13{%k4} */
    "c4e22db615f8ffffff",  /* vfmaddsub231ps -8(%rip), %ymm10, %ymm2 */
    "c4e259b67c09c0",  /* vfmaddsub231ps -64(%rcx,%rcx,1), %xmm4, %xmm7 */
    "62a24520b62cad00100000",  /* vfmaddsub231ps 0x1000(,%r13,4), %ymm23, %ymm21 */
    "62320dbeb6e5",  /* vfmaddsub231ps {rd-sae}, %zmm21, %zmm14, %zmm12{%k6}{z} */
    "c442c9b6a4db00f0ffff",  /* vfmaddsub231pd -0x1000(%r11,%rbx,8), %xmm6, %xmm12 */
    "c442d5b6e6",  /* vfmaddsub231pd %ymm14, %ymm5, %ymm12 */
    "6232f571b6e0",  /* vfmaddsub231pd {rz-sae}, %zmm16, %zmm17, %zmm12{%k1} */
    "c4623597c8",  /* vfmsubadd132ps %ymm0, %ymm9, %ymm9 */
    "c4e23997e1",  /* vfmsubadd132ps %xmm1, %xmm8, %xmm4 */
    "6242352097d1",  /* vfmsubadd132ps %ymm9, %ymm25, %ymm26 */
    "62c20d0897ed",  /* vfmsubadd132ps %xmm13, %xmm14, %xmm21 */
    "c4c2d597ed",  /* vfmsubadd132pd %ymm13, %ymm5, %ymm5 */
    "c462cd972df8ffffff",  /* vfmsubadd132pd -8(%rip), %ymm6, %ymm13 */
    "62f2fd4897345510000000",  /* vfmsubadd132pd 0x10(,%rdx,2), %zmm0, %zmm6 */
    "62c2a5839754c310",  /* vfmsubadd132pd 256(%r11,%rax,8), %xmm27, %xmm18{%k3}{z} */
    "c44255a7d1",  /* vfmsubadd213ps %ymm9, %ymm5, %ymm10 */
    "c44211a7f7",  /* vfmsubadd213ps %xmm15, %xmm13, %xmm14 */
    "62e26522a73d00010000",  /* vfmsubadd213ps 0x100(%rip), %ymm19, %ymm23{%k2} */
    "62e26dcca75d40",  /* vfmsubadd213ps 4096(%rbp), %zmm2, %zmm19{%k4}{z} */
    "c462eda73d00000000",  /* vfmsubadd213pd 0(%rip), %ymm2, %ymm15 */
    "c4c285a7f1",  /* vfmsubadd213pd %ymm9, %ymm15, %ymm6 */
    "62f2b538a738",  /* vfmsubadd213pd (%rax){1to4}, %ymm9, %ymm7 */
    "6292ed06a7cd",  /* vfmsubadd213pd %xmm29, %xmm18, %xmm1{%k6} */
    "c46269b7e4",  /* vfmsubadd231ps %xmm4, %xmm2, %xmm12 */
    "c4623db7c5",  /* vfmsubadd231ps %ymm5, %ymm8, %ymm8 */
    "6262158fb762fc",  /* vfmsubadd231ps -64(%rdx), %xmm13, %xmm28{%k7}{z} */
    "c462adb7c8",  /* vfmsubadd231pd %ymm0, %ymm10, %ymm9 */
    "c4e2cdb7ee",  /* vfmsubadd231pd %ymm6, %ymm6, %ymm5 */
    "6262c540b77c4d40",  /* vfmsubadd231pd 4096(%rbp,%rcx,2), %zmm23, %zmm31 */
    "6232fd20b7ef",  /* vfmsubadd231pd %ymm23, %ymm16, %ymm13 */
    "c4627d131d00000000",  /* vcvtph2ps 0(%rip), %ymm11 */
    "c4627d133d00010000",  /* vcvtph2ps 0x100(%rip), %ymm15 */
    "c463791d2cdd1000000059",  /* vcvtps2ph $89, %xmm13, 0x10(,%rbx,8) */
    "c4e3791d1df8ffffffac",  /* vcvtps2ph $172, %xmm3, -8(%rip) */
    "62d27d9d13de",  /* vcvtph2ps {sae}, %ymm14, %zmm3{%k5}{z} */
    "c4c27d13ed",  /* vcvtph2ps %xmm13, %ymm5 */
    "62637d4b1d997856341244",  /* vcvtps2ph $68, %zmm27, 0x12345678(%rcx){%k3} */
    "62e37d2f1d60f8cb",  /* vcvtps2ph $203, %ymm20, -128(%rax){%k7} */
    "c4c17c5bce",  /* vcvtdq2ps %ymm14, %ymm1 */
    "c4c17c5bfa",  /* vcvtdq2ps %ymm10, %ymm7 */
    "62317c8e5bc7",  /* vcvtdq2ps %xmm23, %xmm8{%k6}{z} */
    "62e17c2e5b0d00000000",  /* vcvtdq2ps 0(%rip), %ymm17{%k6} */
    "c5795bf8",  /* vcvtps2dq %xmm0, %xmm15 */
    "c421795bb49200100000",  /* vcvtps2dq 4096(%rdx,%r10,4), %xmm14 */
    "62e17d4e5bf7",  /* vcvtps2dq %zmm7, %zmm22{%k6} */
    "62617d185bee",  /* vcvtps2dq {rn-sae}, %zmm6, %zmm29 */
    "c5fa5be1",  /* vcvttps2dq %xmm1, %xmm4 */
    "c57e5b0df8ffffff",  /* vcvttps2dq -8(%rip), %ymm9 */
    "62717ead5bf8",  /* vcvttps2dq %ymm0, %ymm15{%k5}{z} */
    "62717e485bd2",  /* vcvttps2dq %zmm2, %zmm10 */
    "c441795ae1",  /* vcvtpd2ps %xmm9, %xmm12 */
    "c5f95afb",  /* vcvtpd2ps %xmm3, %xmm7 */
    "c4417d5ae3",  /* vcvtpd2ps %ymm11, %xmm12 */
    "c4417d5aeb",  /* vcvtpd2ps %ymm11, %xmm13 */
    "c5f95aea",  /* vcvtpd2psx %xmm2, %xmm5 */
    "c4c1795a7708",  /* vcvtpd2psx 8(%r15), %xmm6 */
    "c57d5a3d00010000",  /* vcvtpd2psy 0x100(%rip), %xmm15 */
    "c4a17d5a2cad00100000",  /* vcvtpd2psy 0x1000(,%r13,4), %xmm5 */
    "6271fd485acc",  /* vcvtpd2ps %zmm4, %ymm9 */
    "6261fd4e5a0500010000",  /* vcvtpd2ps 0x100(%rip), %ymm24{%k6} */
    "c5795af3",  /* vcvtpd2psx %xmm3, %xmm14 */
    "c5795ae8",  /* vcvtpd2psx %xmm0, %xmm13 */
    "62e1fd2a5a1500000000",  /* vcvtpd2psy 0(%rip), %xmm18{%k2} */
    "6251fda95a7104",  /* vcvtpd2psy 128(%r9), %xmm14{%k1}{z} */
    "6291fd085ad9",  /* vcvtpd2ps %xmm25, %xmm3 */
    "62b1fd085ad2",  /* vcvtpd2ps %xmm18, %xmm2 */
    "62b1fd285ac2",  /* vcvtpd2ps %ymm18, %xmm0 */
    "62a1fd285ad3",  /* vcvtpd2ps %ymm19, %xmm18 */
    "c4c17c5ad2",  /* vcvtps2pd %xmm10, %ymm2 */
    "c5fc5ae7",  /* vcvtps2pd %xmm7, %ymm4 */
    "62717c485ade",  /* vcvtps2pd %ymm6, %zmm11 */
    "62717c485a549104",  /* vcvtps2pd 128(%rcx,%rdx,4), %zmm10 */
    "c5cb5a54ee80",  /* vcvtsd2ss -128(%rsi,%rbp,8), %xmm6, %xmm2 */
    "c5635a1df8ffffff",  /* vcvtsd2ss -8(%rip), %xmm3, %xmm11 */
    "c5825a2c8510000000",  /* vcvtss2sd 0x10(,%rax,4), %xmm15, %xmm5 */
    "c4a1425a6c9bc0",  /* vcvtss2sd -64(%rbx,%r11,4), %xmm7, %xmm5 */
    "6241bf8d5a69f8",  /* vcvtsd2ss -64(%r9), %xmm8, %xmm29{%k5}{z} */
    "62c1b7365ad0",  /* vcvtsd2ss {rd-sae}, %xmm8, %xmm25, %xmm18{%k6} */
    "c5725a8c4380000000",  /* vcvtss2sd 128(%rbx,%rax,2), %xmm1, %xmm9 */
    "62715e875a1d00000000",  /* vcvtss2sd 0(%rip), %xmm20, %xmm11{%k7}{z} */
    "c52a2ad0",  /* vcvtsi2ss %eax, %xmm10, %xmm10 */
    "c441622a542440",  /* vcvtsi2ss 64(%r12), %xmm3, %xmm10 */
    "c5f32a0df8ffffff",  /* vcvtsi2sd -8(%rip), %xmm1, %xmm1 */
    "c4c1932afb",  /* vcvtsi2sd %r11, %xmm13, %xmm7 */
    "c461fa2dc2",  /* vcvtss2si %xmm2, %r8 */
    "c461fa2d0500000000",  /* vcvtss2si 0(%rip), %r8 */
    "c441fb2dba00f0ffff",  /* vcvtsd2si -0x1000(%r10), %r15 */
    "c4a17b2d1c9500100000",  /* vcvtsd2si 0x1000(,%r10,4), %ebx */
    "c441fa2cba00f0ffff",  /* vcvttss2si -0x1000(%r10), %r15 */
    "c461fa2cee",  /* vcvttss2si %xmm6, %r13 */
    "c4e1fb2cd8",  /* vcvttsd2si %xmm0, %rbx */
    "c4417b2cde",  /* vcvttsd2si %xmm14, %r11d */
    "c56210e3",  /* vmovss %xmm3, %xmm3, %xmm12 */
    "c50a11d6",  /* vmovss %xmm10, %xmm14, %xmm6 */
    "c5e210dc",  /* vmovss %xmm4, %xmm3, %xmm3 */
    "c5ea10cb",  /* vmovss %xmm3, %xmm2, %xmm1 */
    "c5fa101500000000",  /* vmovss 0(%rip), %xmm2 */
    "c5fa103d00000000",  /* vmovss 0(%rip), %xmm7 */
    "c57a113df8ffffff",  /* vmovss %xmm15, -8(%rip) */
    "c5fa112cfd10000000",  /* vmovss %xmm5, 0x10(,%rdi,8) */
    "c58b10e7",  /* vmovsd %xmm7, %xmm14, %xmm4 */
    "c52b11e4",  /* vmovsd %xmm12, %xmm10, %xmm4 */
    "c53b10d6",  /* vmovsd %xmm6, %xmm8, %xmm10 */
    "c51310e4",  /* vmovsd %xmm4, %xmm13, %xmm12 */
    "c4217b101c8500100000",  /* vmovsd 0x1000(,%r8,4), %xmm11 */
    "c5fb1035f8ffffff",  /* vmovsd -8(%rip), %xmm6 */
    "c4217b1134e510000000",  /* vmovsd %xmm14, 0x10(,%r12,8) */
    "c5fb1135f8ffffff",  /* vmovsd %xmm6, -8(%rip) */
    "c5fa7ed5",  /* vmovq %xmm5, %xmm2 */
    "c57a7ec2",  /* vmovq %xmm2, %xmm8 */
    "c57a7eec",  /* vmovq %xmm4, %xmm13 */
    "c4417a7ed2",  /* vmovq %xmm10, %xmm10 */
    "c441796efb",  /* vmovd %r11d, %xmm15 */
    "c441796ef6",  /* vmovd %r14d, %xmm14 */
    "c5797ecd",  /* vmovd %xmm9, %ebp */
    "c5797ecd",  /* vmovd %xmm9, %ebp */
    "c5fa7e0c6d00100000",  /* vmovq 0x1000(,%rbp,2), %xmm1 */
    "c4e1f96ed3",  /* vmovq %rbx, %xmm2 */
    "c461f97ef2",  /* vmovq %xmm14, %rdx */
    "c42179d62cb500100000",  /* vmovq %xmm13, 0x1000(,%r14,4) */
    "c4417a7e20",  /* vmovq (%r8), %xmm12 */
    "c5fa7ea100100000",  /* vmovq 4096(%rcx), %xmm4 */
    "c5fa7edf",  /* vmovq %xmm7, %xmm3 */
    "c579d6e2",  /* vmovq %xmm12, %xmm2 */
    "62e17d086e1df8ffffff",  /* vmovd -8(%rip), %xmm19 */
    "62c17d086ea41f78563412",  /* vmovd 0x12345678(%r15,%rbx,1), %xmm20 */
    "62e17d087ed5",  /* vmovd %xmm18, %ebp */
    "62617d087e3cad00100000",  /* vmovd %xmm31, 0x1000(,%rbp,4) */
    "62a1fd086e3c7500100000",  /* vmovq 0x1000(,%r14,2), %xmm23 */
    "62e1fd086e05f8ffffff",  /* vmovq -8(%rip), %xmm16 */
    "6241fd087e7c24f0",  /* vmovq %xmm31, -128(%r12) */
    "62c1fd087ee4",  /* vmovq %xmm20, %r12 */
    "c4c17dd7da",  /* vpmovmskb %ymm10, %rbx */
    "c57dd7d3",  /* vpmovmskb %ymm3, %r10 */
    "c57850f6",  /* vmovmskps %xmm6, %r14d */
    "c4c17c50c1",  /* vmovmskps %ymm9, %eax */
    "c57950d8",  /* vmovmskpd %xmm0, %r11d */
    "c57950df",  /* vmovmskpd %xmm7, %r11 */
    "c4c17970d1eb",  /* vpshufd $235, %xmm9, %xmm2 */
    "c57970e30c",  /* vpshufd $12, %xmm3, %xmm12 */
    "c4c17970a200100000f7",  /* vpshufd $247, 4096(%r10), %xmm4 */
    "c4c17d70e6f3",  /* vpshufd $243, %ymm14, %ymm4 */
    "c4417f70db61",  /* vpshuflw $97, %ymm11, %ymm11 */
    "c4c17f70b40280000000dd",  /* vpshuflw $221, 128(%r10,%rax,1), %ymm6 */
    "c4417a70de4b",  /* vpshufhw $75, %xmm14, %xmm11 */
    "c4417e70f52f",  /* vpshufhw $47, %ymm13, %ymm14 */
    "c4e25d00c2",  /* vpshufb %ymm2, %ymm4, %ymm0 */
    "c4a22500045d10000000",  /* vpshufb 0x10(,%r11,2), %ymm11, %ymm0 */
    "62923d4800a47d78563412",  /* vpshufb 0x12345678(%r13,%r15,2), %zmm8, %zmm4 */
    "62f275c30005f8ffffff",  /* vpshufb -8(%rip), %zmm17, %zmm0{%k3}{z} */
    "c59971d6d5",  /* vpsrlw $213, %xmm6, %xmm12 */
    "c4c11571d33d",  /* vpsrlw $61, %ymm11, %ymm13 */
    "62b1750071147d100000006d",  /* vpsrlw $109, 0x10(,%r15,2), %xmm17 */
    "62912dac71d139",  /* vpsrlw $57, %ymm25, %ymm10{%k4}{z} */
    "c59d71e41f",  /* vpsraw $31, %ymm4, %ymm12 */
    "c5a571e5ba",  /* vpsraw $186, %ymm5, %ymm11 */
    "62b16d0f71e26b",  /* vpsraw $107, %xmm18, %xmm2{%k7} */
    "62f155237125000000003d",  /* vpsraw $61, 0(%rip), %ymm21{%k3} */
    "c5c571f589",  /* vpsllw $137, %ymm5, %ymm7 */
    "c5c571f4b3",  /* vpsllw $179, %ymm4, %ymm7 */
    "62f1150071350001000030",  /* vpsllw $48, 0x100(%rip), %xmm29 */
    "62f155ac7135f8ffffffa9",  /* vpsllw $169, -8(%rip), %ymm5{%k4}{z} */
    "c59d72d43b",  /* vpsrld $59, %ymm4, %ymm12 */
    "c4c11d72d722",  /* vpsrld $34, %ymm15, %ymm12 */
    "62f11d507215f8ffffff95",  /* vpsrld $149, -8(%rip){1to16}, %zmm28 */
    "62f16da17215000000003c",  /* vpsrld $60, 0(%rip), %ymm18{%k1}{z} */
    "c4c13172e352",  /* vpsrad $82, %xmm11, %xmm9 */
    "c4c12572e78a",  /* vpsrad $138, %ymm15, %ymm11 */
    "62f125a77261fc38",  /* vpsrad $56, -128(%rcx), %ymm27{%k7}{z} */
    "62910d5c72a4a400f0ffff5a",  /* vpsrad $90, -0x1000(%r12,%r12,4){1to16}, %zmm14{%k4} */
    "c5e172f2f4",  /* vpslld $244, %xmm2, %xmm3 */
    "c59572f5c0",  /* vpslld $192, %ymm5, %ymm13 */
    "62f105207235f8ffffffb4",  /* vpslld $180, -8(%rip), %ymm31 */
    "62b145487234ed1000000046",  /* vpslld $70, 0x10(,%r13,8), %zmm7 */
    "c58173d1c1",  /* vpsrlq $193, %xmm1, %xmm15 */
    "c4c10173d43d",  /* vpsrlq $61, %xmm12, %xmm15 */
    "6291e52873d56e",  /* vpsrlq $110, %ymm29, %ymm3 */
    "62b1e5967314e50010000095",  /* vpsrlq $149, 0x1000(,%r12,8){1to2}, %xmm19{%k6}{z} */
    "c4c12173f491",  /* vpsllq $145, %xmm12, %xmm11 */
    "c4c10573f684",  /* vpsllq $132, %ymm14, %ymm15 */
    "62d18d4873f6f9",  /* vpsllq $249, %zmm14, %zmm14 */
    "62b1c5a373f20c",  /* vpsllq $12, %ymm18, %ymm23{%k3}{z} */
    "c4c13d73d91d",  /* vpsrldq $29, %ymm9, %ymm8 */
    "c4c16973dfb6",  /* vpsrldq $182, %xmm15, %xmm2 */
    "62b13d0873d814",  /* vpsrldq $20, %xmm16, %xmm8 */
    "62f12508739800f0ffff56",  /* vpsrldq $86, -0x1000(%rax), %xmm11 */
    "c5b973fc79",  /* vpslldq $121, %xmm4, %xmm8 */
    "c4c17573f932",  /* vpslldq $50, %ymm9, %ymm1 */
    "6291450873f9b9",  /* vpslldq $185, %xmm25, %xmm7 */
    "62b1250873ff5b",  /* vpslldq $91, %xmm23, %xmm11 */
    "62b1ddb27224b510000000c0",  /* vpsraq $192, 0x10(,%r14,4){1to4}, %ymm20{%k2}{z} */
    "62b19d4872e36e",  /* vpsraq $110, %zmm19, %zmm12 */
    "c4811dd14c81c0",  /* vpsrlw -64(%r9,%r8,4), %ymm12, %ymm1 */
    "c4c11dd1cf",  /* vpsrlw %xmm15, %ymm12, %ymm1 */
    "c5f9d2f0",  /* vpsrld %xmm0, %xmm0, %xmm6 */
    "c44165d2c4",  /* vpsrld %xmm12, %ymm3, %ymm8 */
    "c4c101d3c1",  /* vpsrlq %xmm9, %xmm15, %xmm0 */
    "c545d33d00000000",  /* vpsrlq 0(%rip), %ymm7, %ymm15 */
    "c505e1c6",  /* vpsraw %xmm6, %ymm15, %ymm8 */
    "c57de11500000000",  /* vpsraw 0(%rip), %ymm0, %ymm10 */
    "c44155e27740",  /* vpsrad 64(%r15), %ymm5, %ymm14 */
    "c591e2ee",  /* vpsrad %xmm6, %xmm13, %xmm5 */
    "c531f1db",  /* vpsllw %xmm3, %xmm9, %xmm11 */
    "c4c109f1ea",  /* vpsllw %xmm10, %xmm14, %xmm5 */
    "c4a129f20c9510000000",  /* vpslld 0x10(,%r10,4), %xmm10, %xmm1 */
    "c585f22500010000",  /* vpslld 0x100(%rip), %ymm15, %ymm4 */
    "c5edf3e0",  /* vpsllq %xmm0, %ymm2, %ymm4 */
    "c549f3c0",  /* vpsllq %xmm0, %xmm6, %xmm8 */
    "c4630920f650",  /* vpinsrb $80, %esi, %xmm14, %xmm14 */
    "c4c35920d8b1",  /* vpinsrb $177, %r8d, %xmm4, %xmm3 */
    "c4c101c4c3fe",  /* vpinsrw $254, %r11d, %xmm15, %xmm0 */
    "c4c121c4e900",  /* vpinsrw $0, %r9d, %xmm11, %xmm5 */
    "c4636122050001000052",  /* vpinsrd $82, 0x100(%rip), %xmm3, %xmm8 */
    "c4432122e60b",  /* vpinsrd $11, %r14d, %xmm11, %xmm12 */
    "c4a3b92294c200100000da",  /* vpinsrq $218, 4096(%rdx,%r8,8), %xmm8, %xmm2 */
    "c4e3c9220d0001000095",  /* vpinsrq $149, 0x100(%rip), %xmm6, %xmm1 */
    "c4e379142935",  /* vpextrb $53, %xmm5, (%rcx) */
    "c4e379145d80b9",  /* vpextrb $185, %xmm3, -128(%rbp) */
    "c5f9c5d172",  /* vpextrw $114, %xmm1, %edx */
    "c5f9c5db47",  /* vpextrw $71, %xmm3, %ebx */
    "c4e37916468062",  /* vpextrd $98, %xmm0, -128(%rsi) */
    "c4e37916db4b",  /* vpextrd $75, %xmm3, %ebx */
    "c4c3f916be800000000d",  /* vpextrq $13, %xmm7, 128(%r14) */
    "c4c3f916ef5a",  /* vpextrq $90, %xmm5, %r15 */
    "c463592133b9",  /* vinsertps $185, (%rbx), %xmm4, %xmm14 */
    "c4632121d144",  /* vinsertps $68, %xmm1, %xmm11, %xmm10 */
    "c4a379177c8e08da",  /* vextractps $218, %xmm7, 8(%rsi,%r9,4) */
    "c4637917e60a",  /* vextractps $10, %xmm12, %esi */
    "c463710ce882",  /* vblendps $130, %xmm0, %xmm1, %xmm13 */
    "c443150cdf62",  /* vblendps $98, %ymm15, %ymm13, %ymm11 */
    "c463610d5e800b",  /* vblendpd $11, -128(%rsi), %xmm3, %xmm11 */
    "c4e3390dcdd7",  /* vblendpd $215, %xmm5, %xmm8, %xmm1 */
    "c443790e5108bc",  /* vpblendw $188, 8(%r9), %xmm0, %xmm10 */
    "c443790edcc7",  /* vpblendw $199, %xmm12, %xmm0, %xmm11 */
    "c4e3610240c02d",  /* vpblendd $45, -64(%rax), %xmm3, %xmm0 */
    "c4634102bc698000000022",  /* vpblendd $34, 128(%rcx,%rbp,2), %xmm7, %xmm15 */
    "c4e3154adfe0",  /* vblendvps %ymm14, %ymm7, %ymm13, %ymm3 */
    "c463014ae2b0",  /* vblendvps %xmm11, %xmm2, %xmm15, %xmm12 */
    "c4e3614bf1a0",  /* vblendvpd %xmm10, %xmm1, %xmm3, %xmm6 */
    "c4e3254b050000000030",  /* vblendvpd %ymm3, 0(%rip), %ymm11, %ymm0 */
    "c4c31d4cc630",  /* vpblendvb %ymm3, %ymm14, %ymm12, %ymm0 */
    "c463514cc430",  /* vpblendvb %xmm3, %xmm4, %xmm5, %xmm8 */
    "c4e311403500000000ca",  /* vdpps $202, 0(%rip), %xmm13, %xmm6 */
    "c4e351403d00000000e9",  /* vdpps $233, 0(%rip), %xmm5, %xmm7 */
    "c4e351413cc500100000de",  /* vdppd $222, 0x1000(,%rax,8), %xmm5, %xmm7 */
    "c4e36941c032",  /* vdppd $50, %xmm0, %xmm2, %xmm0 */
    "c4c3fd00fd1b",  /* vpermq $27, %ymm13, %ymm7 */
    "c4c3fd00acfd785634124e",  /* vpermq $78, 0x12345678(%r13,%rdi,8), %ymm5 */
    "c443fd019c300010000069",  /* vpermpd $105, 4096(%r8,%rsi,1), %ymm11 */
    "c463fd01cbbb",  /* vpermpd $187, %ymm3, %ymm9 */
    "c4e33d0634fd1000000066",  /* vperm2f128 $102, 0x10(,%rdi,8), %ymm8, %ymm6 */
    "c4633d06ceed",  /* vperm2f128 $237, %ymm6, %ymm8, %ymm9 */
    "c4632d46c03d",  /* vperm2i128 $61, %ymm0, %ymm10, %ymm8 */
    "c4e33546c8ce",  /* vperm2i128 $206, %ymm0, %ymm9, %ymm1 */
    "c4624d36eb",  /* vpermd %ymm3, %ymm6, %ymm13 */
    "c4e24d36d1",  /* vpermd %ymm1, %ymm6, %ymm2 */
    "c4623d163df8ffffff",  /* vpermps -8(%rip), %ymm8, %ymm15 */
    "c4423d16da",  /* vpermps %ymm10, %ymm8, %ymm11 */
    "c4a21d0c848600010000",  /* vpermilps 256(%rsi,%r8,4), %ymm12, %ymm0 */
    "c4c2510cee",  /* vpermilps %xmm14, %xmm5, %xmm5 */
    "c4c2490dc1",  /* vpermilpd %xmm9, %xmm6, %xmm0 */
    "c4e2290d0c8500100000",  /* vpermilpd 0x1000(,%rax,4), %xmm10, %xmm1 */
    "c4437904c279",  /* vpermilps $121, %xmm10, %xmm8 */
    "c48379045cc68072",  /* vpermilps $114, -128(%r14,%r8,8), %xmm3 */
    "c4637905f1b9",  /* vpermilpd $185, %xmm1, %xmm14 */
    "c4e37905e028",  /* vpermilpd $40, %xmm0, %xmm4 */
    "c4a36d18048d100000009f",  /* vinsertf128 $159, 0x10(,%r9,4), %ymm2, %ymm0 */
    "c4430d186c24c022",  /* vinsertf128 $34, -64(%r12), %ymm14, %ymm13 */
    "c4437d19c561",  /* vextractf128 $97, %ymm8, %xmm13 */
    "c4e37d195bc0bc",  /* vextractf128 $188, %ymm3, -64(%rbx) */
    "c4434d38d3c4",  /* vinserti128 $196, %xmm11, %ymm6, %ymm10 */
    "c40375383c00e8",  /* vinserti128 $232, (%r8,%r8,1), %ymm1, %ymm15 */
    "c4c37d39dfa9",  /* vextracti128 $169, %ymm3, %xmm15 */
    "c4a37d393cf510000000c5",  /* vextracti128 $197, %ymm7, 0x10(,%r14,8) */
    "62331d2018d25e",  /* vinsertf32x4 $94, %xmm18, %ymm28, %ymm10 */
    "623375481814aed6",  /* vinsertf32x4 $214, (%rsi,%r13,4), %zmm1, %zmm10 */
    "6293a52018dcec",  /* vinsertf64x2 $236, %xmm28, %ymm27, %ymm3 */
    "62b3ad48185422fc0d",  /* vinsertf64x2 $13, -64(%rdx,%r12,1), %zmm10, %zmm2 */
    "62334d2838d06d",  /* vinserti32x4 $109, %xmm16, %ymm6, %ymm10 */
    "62436d2038471016",  /* vinserti32x4 $22, 256(%r15), %ymm18, %ymm24 */
    "6273bda7382cdd100000001b",  /* vinserti64x2 $27, 0x10(,%rbx,8), %ymm24, %ymm13{%k7}{z} */
    "6253b54838d1c4",  /* vinserti64x2 $196, %xmm9, %zmm9, %zmm10 */
    "62a365481a3cf510000000f7",  /* vinsertf32x8 $247, 0x10(,%r14,8), %zmm3, %zmm23 */
    "629325481af66a",  /* vinsertf32x8 $106, %ymm30, %zmm11, %zmm6 */
    "62e3ed401a2d000100009a",  /* vinsertf64x4 $154, 0x100(%rip), %zmm18, %zmm21 */
    "6223954e1a3cf51000000045",  /* vinsertf64x4 $69, 0x10(,%r14,8), %zmm13, %zmm31{%k6} */
    "626325c13ae873",  /* vinserti32x8 $115, %ymm0, %zmm27, %zmm29{%k1}{z} */
    "62f335ca3ae487",  /* vinserti32x8 $135, %ymm4, %zmm9, %zmm4{%k2}{z} */
    "6223fd483a2c9d10000000d7",  /* vinserti64x4 $215, 0x10(,%r11,4), %zmm0, %zmm29 */
    "6243ad403ac8f6",  /* vinserti64x4 $246, %ymm8, %zmm26, %zmm25 */
    "62037d4819cc24",  /* vextractf32x4 $36, %zmm25, %xmm28 */
    "62f37d48190500010000d7",  /* vextractf32x4 $215, %zmm0, 0x100(%rip) */
    "6263fd2819af0800000064",  /* vextractf64x2 $100, %ymm29, 8(%rdi) */
    "62f3fdcb19c9c4",  /* vextractf64x2 $196, %zmm1, %xmm1{%k3}{z} */
    "62837dcd39ddbb",  /* vextracti32x4 $187, %zmm19, %xmm29{%k5}{z} */
    "62c37d4839f9f9",  /* vextracti32x4 $249, %zmm23, %xmm9 */
    "62b3fd2e3904a0f0",  /* vextracti64x2 $240, %ymm0, (%rax,%r12,4){%k6} */
    "62e3fd283935000100007f",  /* vextracti64x2 $127, %ymm22, 0x100(%rip) */
    "62437d491bcf5b",  /* vextractf32x8 $91, %zmm25, %ymm15{%k1} */
    "62537d481b94710800000098",  /* vextractf32x8 $152, %zmm10, 8(%r9,%rsi,2) */
    "6253fd481b75021b",  /* vextractf64x4 $27, %zmm14, 64(%r13) */
    "62d3fd481befe5",  /* vextractf64x4 $229, %zmm5, %ymm15 */
    "62a37d4e3b1ce51000000035",  /* vextracti32x8 $53, %zmm19, 0x10(,%r12,8){%k6} */
    "62137dcf3bdb58",  /* vextracti32x8 $88, %zmm11, %ymm27{%k7}{z} */
    "62f3fd483b5b0459",  /* vextracti64x4 $89, %zmm3, 128(%rbx) */
    "6283fd483bffa8",  /* vextracti64x4 $168, %zmm23, %ymm31 */
    "6273455c251c851000000001",  /* vpternlogd $1, 0x10(,%rax,4){1to16}, %zmm7, %zmm11{%k4} */
    "621305c325bcfe0800000029",  /* vpternlogd $41, 8(%r14,%r15,8), %zmm31, %zmm15{%k3}{z} */
    "6273ad2025e6eb",  /* vpternlogq $235, %ymm6, %ymm26, %ymm12 */
    "62e3950a252d0000000026",  /* vpternlogq $38, 0(%rip), %xmm13, %xmm21{%k2} */
    "62d355301f3c6897",  /* vpcmpd $151, (%r8,%rbp,2){1to8}, %ymm21, %k7 */
    "62f31d281fdfe8",  /* vpcmpd $232, %ymm7, %ymm12, %k3 */
    "62f365481ec838",  /* vpcmpud $56, %zmm0, %zmm3, %k1 */
    "62d345001e4afc2b",  /* vpcmpud $43, -64(%r10), %xmm23, %k1 */
    "62d3e5481f4b400d",  /* vpcmpq $13, 4096(%r11), %zmm3, %k1 */
    "62d3f5001f970010000041",  /* vpcmpq $65, 4096(%r15), %xmm17, %k2 */
    "62b3b5101e14b510000000f1",  /* vpcmpuq $241, 0x10(,%r14,4){1to2}, %xmm25, %k2 */
    "62f3cd201eecac",  /* vpcmpuq $172, %ymm4, %ymm22, %k5 */
    "62f37d483f14dd1000000001",  /* vpcmpb $1, 0x10(,%rbx,8), %zmm0, %k2 */
    "62f305003f144d0010000017",  /* vpcmpb $23, 0x1000(,%rcx,2), %xmm31, %k2 */
    "62f3f5403f3076",  /* vpcmpw $118, (%rax), %zmm17, %k6 */
    "62f3b5483f15f8ffffff86",  /* vpcmpw $134, -8(%rip), %zmm9, %k2 */
    "62f36d083ef0d8",  /* vpcmpub $216, %xmm0, %xmm2, %k6 */
    "62f375483e15000000001e",  /* vpcmpub $30, 0(%rip), %zmm1, %k2 */
    "62f3b5083e3d00010000ac",  /* vpcmpuw $172, 0x100(%rip), %xmm9, %k7 */
    "629385003e7434041c",  /* vpcmpuw $28, 64(%r12,%r14,1), %xmm31, %k6 */
    "c5f990de",  /* kmovb %k6, %k3 */
    "c5f9901d00010000",  /* kmovb 0x100(%rip), %k3 */
    "c5f990e5",  /* kmovb %k5, %k4 */
    "c5f9912500010000",  /* kmovb %k4, 0x100(%rip) */
    "c5f8907c4140",  /* kmovw 64(%rcx,%rax,2), %k7 */
    "c5f8904ec0",  /* kmovw -64(%rsi), %k1 */
    "c5f890fd",  /* kmovw %k5, %k7 */
    "c5f890c3",  /* kmovw %k3, %k0 */
    "c4e1f990cd",  /* kmovd %k5, %k1 */
    "c4e1f9905c42c0",  /* kmovd -64(%rdx,%rax,2), %k3 */
    "c4e1f990ec",  /* kmovd %k4, %k5 */
    "c4a1f991044500100000",  /* kmovd %k0, 0x1000(,%r8,2) */
    "c4a1f890947f80000000",  /* kmovq 128(%rdi,%r15,2), %k2 */
    "c4e1f890c2",  /* kmovq %k2, %k0 */
    "c4e1f890f4",  /* kmovq %k4, %k6 */
    "c4e1f890f7",  /* kmovq %k7, %k6 */
    "c4c17992db",  /* kmovb %r11d, %k3 */
    "c4c17992cb",  /* kmovb %r11d, %k1 */
    "c4c17892de",  /* kmovw %r14d, %k3 */
    "c5f892c8",  /* kmovw %eax, %k1 */
    "c4c17b92d0",  /* kmovd %r8d, %k2 */
    "c4c17b92d1",  /* kmovd %r9d, %k2 */
    "c4c1fb92c8",  /* kmovq %r8, %k1 */
    "c4c1fb92e5",  /* kmovq %r13, %k4 */
    "c5f993cd",  /* kmovb %k5, %ecx */
    "c5f993ce",  /* kmovb %k6, %ecx */
    "c5f893ea",  /* kmovw %k2, %ebp */
    "c5f893f5",  /* kmovw %k5, %esi */
    "c5fb93f7",  /* kmovd %k7, %esi */
    "c5fb93f6",  /* kmovd %k6, %esi */
    "c4e1fb93cc",  /* kmovq %k4, %rcx */
    "c461fb93e9",  /* kmovq %k1, %r13 */
    "c5fd41f2",  /* kandb %k2, %k0, %k6 */
    "c5c541c1",  /* kandb %k1, %k7, %k0 */
    "c5e542fd",  /* kandnb %k5, %k3, %k7 */
    "c5fd42ee",  /* kandnb %k6, %k0, %k5 */
    "c5cd45d6",  /* korb %k6, %k6, %k2 */
    "c5d545f5",  /* korb %k5, %k5, %k6 */
    "c5c546c5",  /* kxnorb %k5, %k7, %k0 */
    "c5d546c7",  /* kxnorb %k7, %k5, %k0 */
    "c5d547df",  /* kxorb %k7, %k5, %k3 */
    "c5d547fd",  /* kxorb %k5, %k5, %k7 */
    "c5f944f8",  /* knotb %k0, %k7 */
    "c5f944f3",  /* knotb %k3, %k6 */
    "c5e54ad3",  /* kaddb %k3, %k3, %k2 */
    "c5fd4aff",  /* kaddb %k7, %k0, %k7 */
    "c5f998f7",  /* kortestb %k7, %k6 */
    "c5f998f2",  /* kortestb %k2, %k6 */
    "c5f999ff",  /* ktestb %k7, %k7 */
    "c5f999fa",  /* ktestb %k2, %k7 */
    "c5cc41cd",  /* kandw %k5, %k6, %k1 */
    "c5e441c7",  /* kandw %k7, %k3, %k0 */
    "c5f442fd",  /* kandnw %k5, %k1, %k7 */
    "c5c442ec",  /* kandnw %k4, %k7, %k5 */
    "c5e445d2",  /* korw %k2, %k3, %k2 */
    "c5d445f2",  /* korw %k2, %k5, %k6 */
    "c5dc46ce",  /* kxnorw %k6, %k4, %k1 */
    "c5d446c0",  /* kxnorw %k0, %k5, %k0 */
    "c5d447c1",  /* kxorw %k1, %k5, %k0 */
    "c5e447d9",  /* kxorw %k1, %k3, %k3 */
    "c5f844d4",  /* knotw %k4, %k2 */
    "c5f844d5",  /* knotw %k5, %k2 */
    "c5fc4ac6",  /* kaddw %k6, %k0, %k0 */
    "c5d44ae3",  /* kaddw %k3, %k5, %k4 */
    "c5f898da",  /* kortestw %k2, %k3 */
    "c5f898c1",  /* kortestw %k1, %k0 */
    "c5f899f7",  /* ktestw %k7, %k6 */
    "c5f899d5",  /* ktestw %k5, %k2 */
    "c4e1fd41d3",  /* kandd %k3, %k0, %k2 */
    "c4e1fd41e3",  /* kandd %k3, %k0, %k4 */
    "c4e1fd42e3",  /* kandnd %k3, %k0, %k4 */
    "c4e1c542d6",  /* kandnd %k6, %k7, %k2 */
    "c4e1d545cc",  /* kord %k4, %k5, %k1 */
    "c4e1cd45fb",  /* kord %k3, %k6, %k7 */
    "c4e1dd46fb",  /* kxnord %k3, %k4, %k7 */
    "c4e1ed46f7",  /* kxnord %k7, %k2, %k6 */
    "c4e1c547e6",  /* kxord %k6, %k7, %k4 */
    "c4e1f547c2",  /* kxord %k2, %k1, %k0 */
    "c4e1f944df",  /* knotd %k7, %k3 */
    "c4e1f944e3",  /* knotd %k3, %k4 */
    "c4e1fd4aca",  /* kaddd %k2, %k0, %k1 */
    "c4e1dd4adf",  /* kaddd %k7, %k4, %k3 */
    "c4e1f998d6",  /* kortestd %k6, %k2 */
    "c4e1f998fe",  /* kortestd %k6, %k7 */
    "c4e1f999e4",  /* ktestd %k4, %k4 */
    "c4e1f999f4",  /* ktestd %k4, %k6 */
    "c4e1dc41f7",  /* kandq %k7, %k4, %k6 */
    "c4e1ec41f4",  /* kandq %k4, %k2, %k6 */
    "c4e1e442e6",  /* kandnq %k6, %k3, %k4 */
    "c4e1fc42e6",  /* kandnq %k6, %k0, %k4 */
    "c4e1dc45c0",  /* korq %k0, %k4, %k0 */
    "c4e1d445f6",  /* korq %k6, %k5, %k6 */
    "c4e1cc46ea",  /* kxnorq %k2, %k6, %k5 */
    "c4e1d446ea",  /* kxnorq %k2, %k5, %k5 */
    "c4e1f447d1",  /* kxorq %k1, %k1, %k2 */
    "c4e1e447c1",  /* kxorq %k1, %k3, %k0 */
    "c4e1f844c8",  /* knotq %k0, %k1 */
    "c4e1f844f4",  /* knotq %k4, %k6 */
    "c4e1ec4acd",  /* kaddq %k5, %k2, %k1 */
    "c4e1cc4ad7",  /* kaddq %k7, %k6, %k2 */
    "c4e1f898fa",  /* kortestq %k2, %k7 */
    "c4e1f898f4",  /* kortestq %k4, %k6 */
    "c4e1f899c2",  /* ktestq %k2, %k0 */
    "c4e1f899fa",  /* ktestq %k2, %k7 */
    "fa",  /* cli */
    "fb",  /* sti */
    "f4",  /* hlt */
    "0f01f8",  /* swapgs */
    "cf",  /* iret */
    "48cf",  /* iretq */
    "cf",  /* iretl */
    "66cf",  /* iretw */
    "0faa",  /* rsm */
    "0f30",  /* wrmsr */
    "0f32",  /* rdmsr */
    "0f09",  /* wbinvd */
    "0f08",  /* invd */
    "0f01c8",  /* monitor */
    "0f01c9",  /* mwait */
    "0f06",  /* clts */
    "0f07",  /* sysret */
    "480f07",  /* sysretq */
    "0f34",  /* sysenter */
    "0f35",  /* sysexit */
    "0f013d00010000",  /* invlpg 0x100(%rip) */
    "420f01bcb700100000",  /* invlpg 4096(%rdi,%r14,4) */
    "410f0194aa78563412",  /* lgdt 0x12345678(%r10,%rbp,4) */
    "420f0114e510000000",  /* lgdt 0x10(,%r12,8) */
    "420f011c8500100000",  /* lidt 0x1000(,%r8,4) */
    "420f019cb080000000",  /* lidt 128(%rax,%r14,4) */
    "0f01049510000000",  /* sgdt 0x10(,%rdx,4) */
    "0f0104bd00100000",  /* sgdt 0x1000(,%rdi,4) */
    "0f010cd510000000",  /* sidt 0x10(,%rdx,8) */
    "0f010d00000000",  /* sidt 0(%rip) */
    "420f0134cd00100000",  /* lmsw 0x1000(,%r9,8) */
    "0f01345500100000",  /* lmsw 0x1000(,%rdx,2) */
    "0f0124fd10000000",  /* smsw 0x10(,%rdi,8) */
    "0f01a200010000",  /* smsw 256(%rdx) */
    "0f00d2",  /* lldt %dx */
    "410f00d0",  /* lldt %r8w */
    "0f00df",  /* ltr %edi */
    "0f00de",  /* ltr %rsi */
    "430f00646b80",  /* verr -128(%r11,%r13,2) */
    "0f00e7",  /* verr %di */
    "410f00eb",  /* verw %r11d */
    "410f00ef",  /* verw %r15 */
    "0f000500000000",  /* sldt 0(%rip) */
    "410f00c3",  /* sldt %r11d */
    "0f00cf",  /* str %edi */
    "0f00cf",  /* str %edi */
    "490f01e4",  /* smsw %r12 */
    "0f01e7",  /* smsw %edi */
    "410f01f0",  /* lmsw %r8w */
    "0f01f2",  /* lmsw %dx */
    "450f20c4",  /* mov %cr8, %r12 */
    "410f20e6",  /* mov %cr4, %r14 */
    "0f22c5",  /* mov %rbp, %cr0 */
    "0f22c2",  /* mov %rdx, %cr0 */
    "410f21f4",  /* mov %db6, %r12 */
    "0f21f2",  /* mov %db6, %rdx */
    "0f23f8",  /* mov %rax, %db7 */
    "410f23d8",  /* mov %r8, %db3 */
    "f30f1efb",  /* endbr32 */
    "0f1c044510000000",  /* cldemote 0x10(,%rax,2) */
    "410f1c4108",  /* cldemote 8(%r9) */
    "f30fae34b510000000",  /* clrssbsy 0x10(,%rsi,4) */
    "f3420fae34fd10000000",  /* clrssbsy 0x10(,%r15,8) */
    "410f38f6b200010000",  /* wrssd %esi, 256(%r10) */
    "440f38f63500000000",  /* wrssd %r14d, 0(%rip) */
    "4c0f38f6bd00100000",  /* wrssq %r15, 4096(%rbp) */
    "480f38f61d00010000",  /* wrssq %rbx, 0x100(%rip) */
    "66430f38f594cb00010000",  /* wrussd %edx, 256(%r11,%r9,8) */
    "66420f38f50ca500100000",  /* wrussd %ecx, 0x1000(,%r12,4) */
    "664c0f38f5ac8100010000",  /* wrussq %r13, 256(%rcx,%rax,4) */
    "664d0f38f55a80",  /* wrussq %r11, -128(%r10) */
    "f3410f01ab00100000",  /* rstorssp 4096(%r11) */
    "f30f012df8ffffff",  /* rstorssp -8(%rip) */
    "f30f01e8",  /* setssbsy */
    "f30f01ea",  /* saveprevssp */
    "66450f3a60c52d",  /* pcmpestrm $45, %xmm13, %xmm8 */
    "66440f3a60a451785634120c",  /* pcmpestrm $12, 0x12345678(%rcx,%rdx,2), %xmm12 */
    "c4637960f887",  /* vpcmpestrm $135, %xmm0, %xmm15 */
    "c4e3796084110010000050",  /* vpcmpestrm $80, 4096(%rcx,%rdx,1), %xmm0 */
    "66410f3a61e393",  /* pcmpestri $147, %xmm11, %xmm4 */
    "660f3a611445001000003f",  /* pcmpestri $63, 0x1000(,%rax,2), %xmm2 */
    "c4e37961cedd",  /* vpcmpestri $221, %xmm6, %xmm1 */
    "c42379612c8d00100000dc",  /* vpcmpestri $220, 0x1000(,%r9,4), %xmm13 */
    "660f3a62e459",  /* pcmpistrm $89, %xmm4, %xmm4 */
    "66440f3a62af800000006a",  /* pcmpistrm $106, 128(%rdi), %xmm13 */
    "c4637962dc17",  /* vpcmpistrm $23, %xmm4, %xmm11 */
    "c4037962a41500f0ffff00",  /* vpcmpistrm $0, -0x1000(%r13,%r10,1), %xmm12 */
    "66450f3a63cb49",  /* pcmpistri $73, %xmm11, %xmm9 */
    "66440f3a63d585",  /* pcmpistri $133, %xmm5, %xmm10 */
    "c4a3796314b50010000033",  /* vpcmpistri $51, 0x1000(,%r14,4), %xmm2 */
    "c4c37963e1cd",  /* vpcmpistri $205, %xmm9, %xmm4 */
    "440f38c86500",  /* sha1nexte (%rbp), %xmm12 */
    "440f38c80cf500100000",  /* sha1nexte 0x1000(,%rsi,8), %xmm9 */
    "0f38c9d2",  /* sha1msg1 %xmm2, %xmm2 */
    "470f38c9ac0800010000",  /* sha1msg1 256(%r8,%r9,1), %xmm13 */
    "450f38cadc",  /* sha1msg2 %xmm12, %xmm11 */
    "0f38ca746b80",  /* sha1msg2 -128(%rbx,%rbp,2), %xmm6 */
    "440f38cc0500000000",  /* sha256msg1 0(%rip), %xmm8 */
    "420f38cc847b78563412",  /* sha256msg1 0x12345678(%rbx,%r15,2), %xmm0 */
    "0f38cd2df8ffffff",  /* sha256msg2 -8(%rip), %xmm5 */
    "420f38cd34cd10000000",  /* sha256msg2 0x10(,%r9,8), %xmm6 */
    "440f3acc8c9d000100003a",  /* sha1rnds4 $58, 256(%rbp,%rbx,4), %xmm9 */
    "440f3accd52d",  /* sha1rnds4 $45, %xmm5, %xmm10 */
    "460f38cb84de00100000",  /* sha256rnds2 %xmm0, 4096(%rsi,%r11,8), %xmm8 */
    "440f38cbf3",  /* sha256rnds2 %xmm0, %xmm3, %xmm14 */
    "0f38cbec",  /* sha256rnds2 %xmm4, %xmm5 */
    "0f38cbf5",  /* sha256rnds2 %xmm5, %xmm6 */
    "66450f38cfed",  /* gf2p8mulb %xmm13, %xmm13 */
    "66440f38cf4540",  /* gf2p8mulb 64(%rbp), %xmm8 */
    "66420f3ace6c208064",  /* gf2p8affineqb $100, -128(%rax,%r12,1), %xmm5 */
    "660f3aced8a3",  /* gf2p8affineqb $163, %xmm0, %xmm3 */
    "66440f3acf1d00000000e8",  /* gf2p8affineinvqb $232, 0(%rip), %xmm11 */
    "66440f3acfe538",  /* gf2p8affineinvqb $56, %xmm5, %xmm12 */
    "c4a235cf04f510000000",  /* vgf2p8mulb 0x10(,%r14,8), %ymm9, %ymm0 */
    "c46271cf7208",  /* vgf2p8mulb 8(%rdx), %xmm1, %xmm14 */
    "622245c2cfd8",  /* vgf2p8mulb %zmm16, %zmm23, %zmm27{%k2}{z} */
    "6232250bcfff",  /* vgf2p8mulb %xmm23, %xmm11, %xmm15{%k3} */
    "c4e3d5ce05f8ffffff05",  /* vgf2p8affineqb $5, -8(%rip), %ymm5, %ymm0 */
    "c4e3c9ced472",  /* vgf2p8affineqb $114, %xmm4, %xmm6, %xmm2 */
    "6213ed3bce5c17205c",  /* vgf2p8affineqb $92, 256(%r15,%r10,1){1to4}, %ymm2, %ymm11{%k3} */
    "62b3a54ecef44c",  /* vgf2p8affineqb $76, %zmm20, %zmm11, %zmm6{%k6} */
    "c4c3c5cfd4b8",  /* vgf2p8affineinvqb $184, %ymm12, %ymm7, %ymm2 */
    "c46391cf3d000100002e",  /* vgf2p8affineinvqb $46, 0x100(%rip), %xmm13, %xmm15 */
    "62f3e58acf04ad0010000083",  /* vgf2p8affineinvqb $131, 0x1000(,%rbp,4), %xmm3, %xmm0{%k2}{z} */
    "6263dd2fcfb97856341248",  /* vgf2p8affineinvqb $72, 0x12345678(%rcx), %ymm4, %ymm31{%k7} */
    "c46205dc1df8ffffff",  /* vaesenc -8(%rip), %ymm15, %ymm11 */
    "c46269dcd9",  /* vaesenc %xmm1, %xmm2, %xmm11 */
    "62e21500dc2c4d00100000",  /* vaesenc 0x1000(,%rcx,2), %xmm29, %xmm21 */
    "62522500dc9f78563412",  /* vaesenc 0x12345678(%r15), %xmm27, %xmm11 */
    "c44249ddc6",  /* vaesenclast %xmm14, %xmm6, %xmm8 */
    "c4e245ddfc",  /* vaesenclast %ymm4, %ymm7, %ymm7 */
    "62e22528ddae08000000",  /* vaesenclast 8(%rsi), %ymm11, %ymm21 */
    "62927d40dd4cec02",  /* vaesenclast 128(%r12,%r13,8), %zmm16, %zmm1 */
    "c4e22dde2df8ffffff",  /* vaesdec -8(%rip), %ymm10, %ymm5 */
    "c4e269dee4",  /* vaesdec %xmm4, %xmm2, %xmm4 */
    "62f23540def5",  /* vaesdec %zmm5, %zmm25, %zmm6 */
    "62422500dee5",  /* vaesdec %xmm13, %xmm27, %xmm28 */
    "c4e235df5500",  /* vaesdeclast (%rbp), %ymm9, %ymm2 */
    "c4a261df0cad10000000",  /* vaesdeclast 0x10(,%r13,4), %xmm3, %xmm1 */
    "62324508dff0",  /* vaesdeclast %xmm16, %xmm7, %xmm14 */
    "62c27d48dfcc",  /* vaesdeclast %zmm12, %zmm0, %zmm17 */
    "c44279dba380000000",  /* vaesimc 128(%r11), %xmm12 */
    "c44279db8000f0ffff",  /* vaesimc -0x1000(%r8), %xmm8 */
    "c44379dfdaf9",  /* vaeskeygenassist $249, %xmm10, %xmm11 */
    "c46379df9c82001000002d",  /* vaeskeygenassist $45, 4096(%rdx,%rax,4), %xmm11 */
    "c4636544ca75",  /* vpclmulqdq $117, %ymm2, %ymm3, %ymm9 */
    "c4e31944c1e0",  /* vpclmulqdq $224, %xmm1, %xmm12, %xmm0 */
    "62c37d2044d92c",  /* vpclmulqdq $44, %ymm9, %ymm16, %ymm19 */
    "c4637d44cb39",  /* vpclmulqdq $57, %ymm3, %ymm0, %ymm9 */
    "66440f3a44db00",  /* pclmullqlqdq %xmm3, %xmm11 */
    "660f3a44d700",  /* pclmullqlqdq %xmm7, %xmm2 */
    "c42359444ce50800",  /* vpclmullqlqdq 8(%rbp,%r12,8), %xmm4, %xmm9 */
    "c4c36144d000",  /* vpclmullqlqdq %xmm8, %xmm3, %xmm2 */
    "c4430144ac1a7856341200",  /* vpclmullqlqdq 0x12345678(%r10,%rbx,1), %xmm15, %xmm13 */
    "c4e35544f100",  /* vpclmullqlqdq %ymm1, %ymm5, %ymm6 */
    "66440f3a442d0001000001",  /* pclmulhqlqdq 0x100(%rip), %xmm13 */
    "66410f3a44bc740001000001",  /* pclmulhqlqdq 256(%r12,%rsi,2), %xmm7 */
    "c4636944049d0010000001",  /* vpclmulhqlqdq 0x1000(,%rbx,4), %xmm2, %xmm8 */
    "c4c33d44e401",  /* vpclmulhqlqdq %ymm12, %ymm8, %ymm4 */
    "62c35d0044f201",  /* vpclmulhqlqdq %xmm10, %xmm20, %xmm22 */
    "62236d404414ed1000000001",  /* vpclmulhqlqdq 0x10(,%r13,8), %zmm18, %zmm26 */
    "66410f3a44d810",  /* pclmullqhqdq %xmm8, %xmm3 */
    "66440f3a4424750010000010",  /* pclmullqhqdq 0x1000(,%rsi,2), %xmm12 */
    "c4e32544724010",  /* vpclmullqhqdq 64(%rdx), %ymm11, %ymm6 */
    "c4636d442df8ffffff10",  /* vpclmullqhqdq -8(%rip), %ymm2, %ymm13 */
    "62636d28446d0410",  /* vpclmullqhqdq 128(%rbp), %ymm2, %ymm29 */
    "62737540443c5d0010000010",  /* vpclmullqhqdq 0x1000(,%rbx,2), %zmm17, %zmm15 */
    "66410f3a44550011",  /* pclmulhqhqdq (%r13), %xmm2 */
    "66470f3a4474f98011",  /* pclmulhqhqdq -128(%r9,%r15,8), %xmm14 */
    "c4e34544d211",  /* vpclmulhqhqdq %ymm2, %ymm7, %ymm2 */
    "c4e36544eb11",  /* vpclmulhqhqdq %ymm3, %ymm3, %ymm5 */
    "62631d28442c850010000011",  /* vpclmulhqhqdq 0x1000(,%rax,4), %ymm12, %ymm29 */
    "62830d2844e511",  /* vpclmulhqhqdq %ymm29, %ymm14, %ymm20 */
    "62e27d08c4e0",  /* vpconflictd %xmm0, %xmm20 */
    "62727ddcc43d00010000",  /* vpconflictd 0x100(%rip){1to16}, %zmm15{%k4}{z} */
    "62f2fd48c43500010000",  /* vpconflictq 0x100(%rip), %zmm6 */
    "62f2fd48c43500010000",  /* vpconflictq 0x100(%rip), %zmm6 */
    "62f27d8b446ffc",  /* vplzcntd -64(%rdi), %xmm5{%k3}{z} */
    "62327d0e44248d10000000",  /* vplzcntd 0x10(,%r9,4), %xmm12{%k6} */
    "62f2fd0d44a878563412",  /* vplzcntq 0x12345678(%rax), %xmm5{%k5} */
    "6272fd4844fa",  /* vplzcntq %zmm2, %zmm15 */
    "6272fe482ad6",  /* vpbroadcastmb2q %k6, %zmm10 */
    "6262fe282ad1",  /* vpbroadcastmb2q %k1, %ymm26 */
    "62e27e083af2",  /* vpbroadcastmw2d %k2, %xmm22 */
    "62627e283aeb",  /* vpbroadcastmw2d %k3, %ymm29 */
    "622255aa50d6",  /* vpdpbusd %ymm22, %ymm5, %ymm26{%k2}{z} */
    "62022dad50cd",  /* vpdpbusd %ymm29, %ymm10, %ymm25{%k5}{z} */
    "62122d08505c7a10",  /* vpdpbusd 256(%r10,%r15,2), %xmm10, %xmm11 */
    "62f2750850c0",  /* vpdpbusd %xmm0, %xmm1, %xmm0 */
    "62f23da751148d10000000",  /* vpdpbusds 0x10(,%rcx,4), %ymm24, %ymm2{%k7}{z} */
    "6202258b51c7",  /* vpdpbusds %xmm31, %xmm11, %xmm24{%k3}{z} */
    "62521d08516308",  /* vpdpbusds 128(%r11), %xmm12, %xmm12 */
    "6252452851f8",  /* vpdpbusds %ymm8, %ymm7, %ymm15 */
    "62f235005215f8ffffff",  /* vpdpwssd -8(%rip), %xmm25, %xmm2 */
    "62b2151b521c5500100000",  /* vpdpwssd 0x1000(,%r10,2){1to4}, %xmm13, %xmm3{%k3} */
    "62d2550852f6",  /* vpdpwssd %xmm14, %xmm5, %xmm6 */
    "62723d285201",  /* vpdpwssd (%rcx), %ymm8, %ymm8 */
    "62f24d82530d00000000",  /* vpdpwssds 0(%rip), %xmm22, %xmm1{%k2}{z} */
    "62d27d8c53c8",  /* vpdpwssds %xmm8, %xmm0, %xmm1{%k4}{z} */
    "62d2452853cc",  /* vpdpwssds %ymm12, %ymm7, %ymm1 */
    "62526d2853df",  /* vpdpwssds %ymm15, %ymm2, %ymm11 */
    "6212dd00b4ef",  /* vpmadd52luq %xmm31, %xmm20, %xmm13 */
    "62f2fd24b40500000000",  /* vpmadd52luq 0(%rip), %ymm16, %ymm0{%k4} */
    "6282bd48b5f6",  /* vpmadd52huq %zmm30, %zmm8, %zmm22 */
    "62522d4f8df8",  /* vpermb %zmm8, %zmm10, %zmm15{%k7} */
    "62e25d488dfb",  /* vpermb %zmm3, %zmm4, %zmm23 */
    "62425d0875d9",  /* vpermi2b %xmm9, %xmm4, %xmm27 */
    "62d265aa75f1",  /* vpermi2b %ymm9, %ymm3, %ymm6{%k2}{z} */
    "62f21d407de5",  /* vpermt2b %zmm5, %zmm28, %zmm4 */
    "626265407de7",  /* vpermt2b %zmm7, %zmm19, %zmm28 */
    "6272ddc38301",  /* vpmultishiftqb (%rcx), %zmm20, %zmm8{%k3}{z} */
    "62e2ad2883249500100000",  /* vpmultishiftqb 0x1000(,%rdx,4), %ymm10, %ymm20 */
    "62f27d48540cfd00100000",  /* vpopcntb 0x1000(,%rdi,8), %zmm1 */
    "62927d4854ce",  /* vpopcntb %zmm30, %zmm1 */
    "6222fd085464b2f8",  /* vpopcntw -128(%rdx,%r14,4), %xmm28 */
    "62a2fdcd54f1",  /* vpopcntw %zmm17, %zmm22{%k5}{z} */
    "62c27d2855a46d00100000",  /* vpopcntd 4096(%r13,%rbp,2), %ymm20 */
    "62e27d08555d08",  /* vpopcntd 128(%rbp), %xmm19 */
    "62e2fdcc5534cd00100000",  /* vpopcntq 0x1000(,%rcx,8), %zmm22{%k4}{z} */
    "62a2fd2855ee",  /* vpopcntq %ymm22, %ymm21 */
    "62f265428f04f510000000",  /* vpshufbitqmb 0x10(,%rsi,8), %zmm19, %k0{%k2} */
    "62d27d298fcb",  /* vpshufbitqmb %ymm11, %ymm0, %k1{%k1} */
    "62a2054876ff",  /* vpermi2d %zmm23, %zmm15, %zmm23 */
    "62f24dd3760d00000000",  /* vpermi2d 0(%rip){1to16}, %zmm22, %zmm1{%k3}{z} */
    "62f2e544766d04",  /* vpermi2q 256(%rbp), %zmm19, %zmm5{%k4} */
    "62728540762c9d00100000",  /* vpermi2q 0x1000(,%rbx,4), %zmm31, %zmm13 */
    "62f22d4077f2",  /* vpermi2ps %zmm2, %zmm26, %zmm6 */
    "62f21dda7735f8ffffff",  /* vpermi2ps -8(%rip){1to16}, %zmm12, %zmm6{%k2}{z} */
    "62629d4377f7",  /* vpermi2pd %zmm7, %zmm28, %zmm30{%k3} */
    "62f2c5cb77fa",  /* vpermi2pd %zmm2, %zmm7, %zmm7{%k3}{z} */
    "62824d287eed",  /* vpermt2d %ymm29, %ymm6, %ymm21 */
    "62f20d0a7e15f8ffffff",  /* vpermt2d -8(%rip), %xmm14, %xmm2{%k2} */
    "6272e5b27e34d500100000",  /* vpermt2q 0x1000(,%rdx,8){1to4}, %ymm19, %ymm14{%k2}{z} */
    "6202ad0a7ef7",  /* vpermt2q %xmm31, %xmm10, %xmm30{%k2} */
    "62f21d287f25f8ffffff",  /* vpermt2ps -8(%rip), %ymm12, %ymm4 */
    "62a225207f1ca500100000",  /* vpermt2ps 0x1000(,%r12,4), %ymm27, %ymm19 */
    "6212cd077fdc",  /* vpermt2pd %xmm28, %xmm22, %xmm11{%k7} */
    "62028d487fd5",  /* vpermt2pd %zmm29, %zmm14, %zmm26 */
    "62e295487515f8ffffff",  /* vpermi2w -8(%rip), %zmm13, %zmm18 */
    "6282e52875c0",  /* vpermi2w %ymm24, %ymm3, %ymm16 */
    "62e2bd007dc9",  /* vpermt2w %xmm1, %xmm24, %xmm17 */
    "6262b5007d2d00000000",  /* vpermt2w 0(%rip), %xmm25, %xmm29 */
    "627295208d2d00010000",  /* vpermw 0x100(%rip), %ymm29, %ymm13 */
    "62829d008df8",  /* vpermw %xmm24, %xmm28, %xmm23 */
    "6202654836e1",  /* vpermd %zmm25, %zmm3, %zmm28 */
    "62f20d4836c0",  /* vpermd %zmm0, %zmm14, %zmm0 */
    "6202ed2836ce",  /* vpermq %ymm30, %ymm2, %ymm25 */
    "62529d2036fd",  /* vpermq %ymm13, %ymm28, %ymm15 */
    "62a24d2c16e3",  /* vpermps %ymm19, %ymm6, %ymm20{%k4} */
    "626265381634cd00100000",  /* vpermps 0x1000(,%rcx,8){1to8}, %ymm3, %ymm30 */
    "62c2fd2016f0",  /* vpermpd %ymm8, %ymm16, %ymm22 */
    "62f2f54016ee",  /* vpermpd %zmm6, %zmm17, %zmm5 */
    "6263fd2c0005f8ffffff8a",  /* vpermq $138, -8(%rip), %ymm24{%k4} */
    "62e3fd4f00db88",  /* vpermq $136, %zmm3, %zmm19{%k7} */
    "62c3fdca01d1d5",  /* vpermpd $213, %zmm9, %zmm18{%k2}{z} */
    "62d3fd5801480885",  /* vpermpd $133, 64(%r8){1to8}, %zmm1 */
    "62627d488bfc",  /* vpcompressd %zmm31, %zmm4 */
    "62f27dca8bda",  /* vpcompressd %zmm3, %zmm2{%k2}{z} */
    "6252fd488bfc",  /* vpcompressq %zmm15, %zmm12 */
    "6242fd088bf6",  /* vpcompressq %xmm30, %xmm14 */
    "62727d0a8a740e10",  /* vcompressps %xmm14, 64(%rsi,%rcx,1){%k2} */
    "62f27d4f8a1cf510000000",  /* vcompressps %zmm3, 0x10(,%rsi,8){%k7} */
    "6262fd488a0c7510000000",  /* vcompresspd %zmm25, 0x10(,%rsi,2) */
    "6202fd488a3454",  /* vcompresspd %zmm30, (%r12,%r10,2) */
    "62827da989ff",  /* vpexpandd %ymm31, %ymm23{%k1}{z} */
    "62227d2889b45e00f0ffff",  /* vpexpandd -0x1000(%rsi,%r11,2), %ymm30 */
    "6272fd8a89ea",  /* vpexpandq %xmm2, %xmm13{%k2}{z} */
    "6242fd8d89dd",  /* vpexpandq %xmm13, %xmm27{%k5}{z} */
    "62f27d8e883500000000",  /* vexpandps 0(%rip), %xmm6{%k6}{z} */
    "62a27d2888e9",  /* vexpandps %ymm17, %ymm21 */
    "62b2fd288804fd10000000",  /* vexpandpd 0x10(,%r15,8), %ymm0 */
    "6282fd4888f9",  /* vexpandpd %zmm25, %zmm23 */
    "62e27d0f7ae1",  /* vpbroadcastb %ecx, %xmm20{%k7} */
    "62e27d0c7afb",  /* vpbroadcastb %ebx, %xmm23{%k4} */
    "62427d287bfb",  /* vpbroadcastw %r11d, %ymm31 */
    "62727d087bef",  /* vpbroadcastw %edi, %xmm13 */
    "62527dcf7ce6",  /* vpbroadcastd %r14d, %zmm12{%k7}{z} */
    "62f27dcf7cd7",  /* vpbroadcastd %edi, %zmm2{%k7}{z} */
    "62d2fd287cf1",  /* vpbroadcastq %r9, %ymm6 */
    "6262fd487cc6",  /* vpbroadcastq %rsi, %zmm24 */
    "c4e251472d00000000",  /* vpsllvd 0(%rip), %xmm5, %xmm5 */
    "c4c225479800100000",  /* vpsllvd 4096(%r8), %ymm11, %ymm3 */
    "629265ab47cf",  /* vpsllvd %ymm31, %ymm3, %ymm1{%k3}{z} */
    "62f27dd54734fd00100000",  /* vpsllvd 0x1000(,%rdi,8){1to16}, %zmm16, %zmm6{%k5}{z} */
    "c4c2b947d4",  /* vpsllvq %xmm12, %xmm8, %xmm2 */
    "c46289477940",  /* vpsllvq 64(%rcx), %xmm14, %xmm15 */
    "6262c594471ced10000000",  /* vpsllvq 0x10(,%rbp,8){1to2}, %xmm23, %xmm27{%k4}{z} */
    "62e2c50847d7",  /* vpsllvq %xmm7, %xmm7, %xmm18 */
    "c4e279453d00000000",  /* vpsrlvd 0(%rip), %xmm0, %xmm7 */
    "c4c27d45f0",  /* vpsrlvd %ymm8, %ymm0, %ymm6 */
    "62f20d4045ce",  /* vpsrlvd %zmm6, %zmm30, %zmm1 */
    "c4c25145f3",  /* vpsrlvd %xmm11, %xmm5, %xmm6 */
    "c4e2d545cd",  /* vpsrlvq %ymm5, %ymm5, %ymm1 */
    "c4e2f94525f8ffffff",  /* vpsrlvq -8(%rip), %xmm0, %xmm4 */
    "6222950045e1",  /* vpsrlvq %xmm17, %xmm29, %xmm28 */
    "62e2ad204514cd00100000",  /* vpsrlvq 0x1000(,%rcx,8), %ymm26, %ymm18 */
    "c4c26146f6",  /* vpsravd %xmm14, %xmm3, %xmm6 */
    "c4e21d468000f0ffff",  /* vpsravd -0x1000(%rax), %ymm12, %ymm0 */
    "6212552e46d0",  /* vpsravd %ymm24, %ymm5, %ymm10{%k6} */
    "62f2258f467608",  /* vpsravd 128(%rsi), %xmm11, %xmm6{%k7}{z} */
    "627295004675f8",  /* vpsravq -128(%rbp), %xmm29, %xmm14 */
    "6282dd2046e9",  /* vpsravq %ymm25, %ymm20, %ymm21 */
    "c4e25990bc0f80000000",  /* vpgatherdd %xmm4, 128(%rdi,%xmm1,1), %xmm7 */
    "c42221904cfd40",  /* vpgatherdd %xmm11, 64(%rbp,%xmm15,8), %xmm9 */
    "62427d2d900cd0",  /* vpgatherdd (%r8,%ymm2,8), %ymm25{%k5} */
    "62827d47905c54e0",  /* vpgatherdd -128(%r12,%zmm26,2), %zmm19{%k7} */
    "c422b9908c0e80000000",  /* vpgatherdq %xmm8, 128(%rsi,%xmm9,1), %xmm9 */
    "c482e19044f780",  /* vpgatherdq %xmm3, -128(%r15,%xmm14,8), %xmm0 */
    "62c2fd21901c00",  /* vpgatherdq (%r8,%xmm16,1), %ymm19{%k1} */
    "62f2fd4e909c2500f0ffff",  /* vpgatherdq -0x1000(%rbp,%ymm4,1), %zmm3{%k6} */
    "c4425d917cff80",  /* vpgatherqd %xmm4, -128(%r15,%ymm7,8), %xmm15 */
    "c4c27591a41780000000",  /* vpgatherqd %xmm1, 128(%r15,%ymm2,1), %xmm4 */
    "62527d01914cc220",  /* vpgatherqd 128(%r10,%xmm16,8), %xmm9{%k1} */
    "62327d459164f9f0",  /* vpgatherqd -64(%rcx,%zmm31,8), %ymm12{%k5} */
    "c402f191449f80",  /* vpgatherqq %xmm1, -128(%r15,%xmm11,4), %xmm8 */
    "c4c2ed916483c0",  /* vpgatherqq %ymm2, -64(%r11,%ymm0,4), %ymm4 */
    "6222fd4a91bcb900100000",  /* vpgatherqq 4096(%rcx,%zmm15,4), %zmm31{%k2} */
    "6212fd4f919cdf00f0ffff",  /* vpgatherqq -0x1000(%r15,%zmm11,8), %zmm11{%k7} */
    "c4a24592ac2f80000000",  /* vgatherdps %ymm7, 128(%rdi,%ymm13,1), %ymm5 */
    "c4a2799274ba80",  /* vgatherdps %xmm0, -128(%rdx,%xmm15,4), %xmm6 */
    "62d27d049274cc40",  /* vgatherdps 256(%r12,%xmm17,8), %xmm6{%k4} */
    "62527d4c921c72",  /* vgatherdps (%r10,%zmm6,2), %zmm11{%k4} */
    "c402c592840d80000000",  /* vgatherdpd %ymm7, 128(%r13,%xmm9,1), %ymm8 */
    "c422c1920418",  /* vgatherdpd %xmm7, (%rax,%xmm11,1), %xmm8 */
    "62c2fd42925c0220",  /* vgatherdpd 256(%r10,%ymm16,1), %zmm19{%k2} */
    "62d2fd2292440a01",  /* vgatherdpd 8(%r10,%xmm17,1), %ymm0{%k2} */
    "c4427193841178563412",  /* vgatherqps %xmm1, 0x12345678(%r9,%xmm2,1), %xmm8 */
    "c4e21d93849e00f0ffff",  /* vgatherqps %xmm12, -0x1000(%rsi,%ymm3,4), %xmm0 */
    "62227d4c9374e720",  /* vgatherqps 128(%rdi,%zmm12,8), %ymm30{%k4} */
    "62227d45936cc720",  /* vgatherqps 128(%rdi,%zmm24,8), %ymm29{%k5} */
    "c402f1938cfb00f0ffff",  /* vgatherqpd %xmm1, -0x1000(%r11,%xmm15,8), %xmm9 */
    "c4e2bd939c5000100000",  /* vgatherqpd %ymm8, 4096(%rax,%ymm2,2), %ymm3 */
    "62d2fd0493748708",  /* vgatherqpd 64(%r15,%xmm16,4), %xmm6{%k4} */
    "6262fd4d93bce800f0ffff",  /* vgatherqpd -0x1000(%rax,%zmm5,8), %zmm31{%k5} */
    "62227d43a06ca2f0",  /* vpscatterdd %zmm29, -64(%rdx,%zmm28,4){%k3} */
    "62127d0ca0747fe0",  /* vpscatterdd %xmm14, -128(%r15,%xmm15,2){%k4} */
    "6222fd03a07c1301",  /* vpscatterdq %xmm31, 8(%rbx,%xmm26,1){%k3} */
    "6272fd05a05c5a10",  /* vpscatterdq %xmm11, 128(%rdx,%xmm19,2){%k5} */
    "62a27d42a15ce602",  /* vpscatterqd %ymm19, 8(%rsi,%zmm28,8){%k2} */
    "62427d2ca1bce600f0ffff",  /* vpscatterqd %xmm31, -0x1000(%r14,%ymm4,8){%k4} */
    "6282fd0ba1a4ba00f0ffff",  /* vpscatterqq %xmm20, -0x1000(%r10,%xmm15,4){%k3} */
    "6262fd06a1b4c200100000",  /* vpscatterqq %xmm30, 4096(%rdx,%xmm16,8){%k6} */
    "62e27d02a2547802",  /* vscatterdps %xmm18, 8(%rax,%xmm23,2){%k2} */
    "62927d4fa25c3e20",  /* vscatterdps %zmm3, 128(%r14,%zmm15,1){%k7} */
    "62b2fd44a27c7208",  /* vscatterdpd %zmm7, 64(%rdx,%ymm30,2){%k4} */
    "62c2fd42a274b2f0",  /* vscatterdpd %zmm22, -128(%r10,%ymm22,4){%k2} */
    "62027d42a3b41e78563412",  /* vscatterqps %ymm30, 0x12345678(%r14,%zmm27,1){%k2} */
    "62c27d44a3646602",  /* vscatterqps %ymm20, 8(%r14,%zmm20,2){%k4} */
    "62f2fd04a37cd9f8",  /* vscatterqpd %xmm7, -64(%rcx,%xmm19,8){%k4} */
    "6252fd09a31c1f",  /* vscatterqpd %xmm11, (%r15,%xmm3,1){%k1} */
    "62d27e4e325120",  /* vpmovqb %zmm2, 256(%r9){%k6} */
    "62527e0b327ae0",  /* vpmovqb %xmm15, -64(%r10){%k3} */
    "62c27e0822d2",  /* vpmovsqb %xmm18, %xmm10 */
    "62027e8c22de",  /* vpmovsqb %xmm27, %xmm30{%k4}{z} */
    "62327e0812e4",  /* vpmovusqb %xmm12, %xmm20 */
    "62427e0812c8",  /* vpmovusqb %xmm25, %xmm8 */
    "62927e4834ce",  /* vpmovqw %zmm1, %xmm30 */
    "62f27e48340d00010000",  /* vpmovqw %zmm1, 0x100(%rip) */
    "62027ecf24e5",  /* vpmovsqw %zmm28, %xmm29{%k7}{z} */
    "62627e4b2415f8ffffff",  /* vpmovsqw %zmm26, -8(%rip){%k3} */
    "62b27e4814d6",  /* vpmovusqw %zmm2, %xmm22 */
    "62427e08144c2420",  /* vpmovusqw %xmm25, 128(%r12) */
    "62527e2b35de",  /* vpmovqd %ymm11, %xmm14{%k3} */
    "62627e0835d6",  /* vpmovqd %xmm26, %xmm6 */
    "62727e0c251d00000000",  /* vpmovsqd %xmm11, 0(%rip){%k4} */
    "62a27e08253c6500100000",  /* vpmovsqd %xmm23, 0x1000(,%r12,2) */
    "62827e2815ee",  /* vpmovusqd %ymm21, %xmm30 */
    "62727e8a15c1",  /* vpmovusqd %xmm8, %xmm1{%k2}{z} */
    "62927e8931c0",  /* vpmovdb %xmm0, %xmm24{%k1}{z} */
    "62a27e29311cf510000000",  /* vpmovdb %ymm19, 0x10(,%r14,8){%k1} */
    "62627e0821c4",  /* vpmovsdb %xmm24, %xmm4 */
    "62e27e2a213d00000000",  /* vpmovsdb %ymm23, 0(%rip){%k2} */
    "62127e4811df",  /* vpmovusdb %zmm11, %xmm31 */
    "62a27e8911ff",  /* vpmovusdb %xmm23, %xmm23{%k1}{z} */
    "62127e2933d7",  /* vpmovdw %ymm10, %xmm31{%k1} */
    "62d27e0833ad78563412",  /* vpmovdw %xmm5, 0x12345678(%r13) */
    "62027eab23f5",  /* vpmovsdw %ymm30, %xmm29{%k3}{z} */
    "62427e4823e1",  /* vpmovsdw %zmm28, %ymm9 */
    "62627e2813e2",  /* vpmovusdw %ymm28, %xmm2 */
    "62f27e4d135504",  /* vpmovusdw %zmm2, 128(%rbp){%k5} */
    "62627e483035f8ffffff",  /* vpmovwb %zmm30, -8(%rip) */
    "62227e09303c4d10000000",  /* vpmovwb %xmm31, 0x10(,%r9,2){%k1} */
    "62d27e0820af00f0ffff",  /* vpmovswb %xmm5, -0x1000(%r15) */
    "62e27ecc20d3",  /* vpmovswb %zmm18, %ymm3{%k4}{z} */
    "62b27e081004ad00100000",  /* vpmovuswb %xmm0, 0x1000(,%r13,4) */
    "62f27e48102d00000000",  /* vpmovuswb %zmm5, 0(%rip) */
    "e4bb",  /* inb $187, %al */
    "e449",  /* inb $73, %al */
    "e433",  /* in $51, %al */
    "e499",  /* in $153, %al */
    "ec",  /* inb %dx, %al */
    "ec",  /* inb %dx, %al */
    "ec",  /* in %dx, %al */
    "ec",  /* in %dx, %al */
    "66e53e",  /* inw $62, %ax */
    "66e58a",  /* inw $138, %ax */
    "66e524",  /* in $36, %ax */
    "66e583",  /* in $131, %ax */
    "66ed",  /* inw %dx, %ax */
    "66ed",  /* inw %dx, %ax */
    "66ed",  /* in %dx, %ax */
    "66ed",  /* in %dx, %ax */
    "e56e",  /* inl $110, %eax */
    "e5ca",  /* inl $202, %eax */
    "e519",  /* in $25, %eax */
    "e5fa",  /* in $250, %eax */
    "ed",  /* inl %dx, %eax */
    "ed",  /* inl %dx, %eax */
    "ed",  /* in %dx, %eax */
    "ed",  /* in %dx, %eax */
    "e626",  /* outb %al, $38 */
    "e682",  /* outb %al, $130 */
    "e624",  /* out %al, $36 */
    "e6b6",  /* out %al, $182 */
    "ee",  /* outb %al, %dx */
    "ee",  /* outb %al, %dx */
    "ee",  /* out %al, %dx */
    "ee",  /* out %al, %dx */
    "66e7a4",  /* outw %ax, $164 */
    "66e77c",  /* outw %ax, $124 */
    "66e7b1",  /* out %ax, $177 */
    "66e758",  /* out %ax, $88 */
    "66ef",  /* outw %ax, %dx */
    "66ef",  /* outw %ax, %dx */
    "66ef",  /* out %ax, %dx */
    "66ef",  /* out %ax, %dx */
    "e727",  /* outl %eax, $39 */
    "e70d",  /* outl %eax, $13 */
    "e76c",  /* out %eax, $108 */
    "e7ae",  /* out %eax, $174 */
    "ef",  /* outl %eax, %dx */
    "ef",  /* outl %eax, %dx */
    "ef",  /* out %eax, %dx */
    "ef",  /* out %eax, %dx */
    "6c",  /* insb */
    "666d",  /* insw */
    "6d",  /* insl */
    "6e",  /* outsb */
    "666f",  /* outsw */
    "6f",  /* outsl */
};

static int hexval(char c) { return c <= '9' ? c - '0' : c - 'a' + 10; }

int main(void) {
    const unsigned char *p = axiom_asm_table;
    int bad = 0, n = (int)(sizeof want / sizeof want[0]);
    for (int i = 0; i < n; i++) {
        const char *w = want[i];
        for (int k = 0; w[k] != 0; k += 2) {
            if (p >= axiom_asm_table_end || *p != hexval(w[k]) * 16 + hexval(w[k + 1])) {
                if (bad++ < 5) { print_str("mismatch at line ", 17); print_i64(i); }
            }
            p++;
        }
    }
    print_i64(bad);
    print_i64(p == axiom_asm_table_end);
    return 0;
}
#else
int main(void) { print_i64(0); print_i64(1); return 0; }
#endif
/* expect: 0 */
/* expect: 1 */
