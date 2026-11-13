/* flags: --c-std=gnu17 */
/* native-only */
/* The x86-64 assembler in the Intel syntax (.intel_syntax noprefix: operands destination first, registers without %, SIZE PTR memory operands, OFFSET, movzx/movsx, BCST, the comparison pseudo-ops ...): a sample of each instruction of the tables and of the general-purpose ones, in the form the disassembler writes them, against a byte table checked against the system assembler in Intel mode in development. */
#include "host.h"

#ifdef __AXIOM_ASM_X86_64__
__asm__(".section .rodata.axiom_asm_table, \"a\"\n"
        ".globl axiom_asm_table\n"
        "axiom_asm_table:\n"
        ".intel_syntax noprefix\n"
        "adc BYTE PTR [rbx-0x40],bpl\n"
        "adc DWORD PTR [esi+0x0],0x10\n"
        "adc r12,QWORD PTR [r12+rcx*4]\n"
        "adcx r9,rcx\n"
        "add DWORD PTR [ebx],0xff\n"
        "add WORD PTR fs:[rsp+0x8],0xff\n"
        "add rsp,QWORD PTR fs:[r9-0x40]\n"
        "addsd xmm11,QWORD PTR [r8+rax*8+0x0]\n"
        "addsd xmm5,QWORD PTR [rax*2+0x0]\n"
        "addsd xmm8,QWORD PTR ds:0x0\n"
        "addss xmm5,DWORD PTR [rax-0x40]\n"
        "addss xmm13,DWORD PTR fs:[r9-0x80]\n"
        "addss xmm9,DWORD PTR ds:0x0\n"
        "and rdi,QWORD PTR [rdi+r13*1+0x0]\n"
        "and BYTE PTR gs:[rsp+0x0],0x80\n"
        "and esp,DWORD PTR fs:[r12-0x80]\n"
        "andn rbp,r8,rsp\n"
        "andpd xmm12,XMMWORD PTR [rax*8+0x0]\n"
        "andpd xmm14,XMMWORD PTR [r15+0x0]\n"
        "andpd xmm4,XMMWORD PTR [rbp+rdi*1-0x80]\n"
        "bextr esp,DWORD PTR [r9*2+0x10],ebx\n"
        "bextr ebx,DWORD PTR [rip+0x0],edx\n"
        "bextr r8d,DWORD PTR fs:0x28,r12d\n"
        "bsf esp,DWORD PTR [rip+0x0]\n"
        "bsf dx,WORD PTR [rbp*2+0x10]\n"
        "bsf si,WORD PTR [esi+0x100]\n"
        "bsr r15w,WORD PTR [rax+0x0]\n"
        "bsr rax,QWORD PTR gs:[rsi+0x7f]\n"
        "bsr dx,WORD PTR fs:[rcx+0x0]\n"
        "bswap eax\n"
        "bt QWORD PTR [rip+0x0],0x21\n"
        "bt DWORD PTR ds:0x1234,0x2e\n"
        "bt QWORD PTR [r9+rbp*2-0x80],0x8\n"
        "btc WORD PTR [ebx+0x0],0x23\n"
        "btc WORD PTR fs:[r8],0x3a\n"
        "btc r12,0x2d\n"
        "btr QWORD PTR [rax+r12*8+0x0],0x0\n"
        "btr WORD PTR [esi-0x80],0xf\n"
        "btr WORD PTR gs:[rsp-0x40],0x35\n"
        "bts rsp,0x28\n"
        "bts DWORD PTR fs:[r12+0x100],0x1f\n"
        "bts DWORD PTR [r9*8+0x10],0x16\n"
        "cdqe\n"
        "cldemote BYTE PTR [rip+0x0]\n"
        "cldemote BYTE PTR [rdi+r15*4+0x80]\n"
        "cldemote BYTE PTR [rbx+rbp*8]\n"
        "clflush BYTE PTR [r9+rbx*2]\n"
        "clflush BYTE PTR [ebx-0x40]\n"
        "clflush BYTE PTR [rdi+0x12345678]\n"
        "cli\n"
        "clrssbsy QWORD PTR [r8*4+0x10]\n"
        "clrssbsy QWORD PTR [rsi+r13*1+0x8]\n"
        "clrssbsy QWORD PTR [rbx-0x40]\n"
        "clts\n"
        "cmova dx,WORD PTR gs:[rsp-0x80]\n"
        "cmova bp,WORD PTR [rax+0x14]\n"
        "cmova esp,DWORD PTR [rip+0x0]\n"
        "cmove rcx,QWORD PTR fs:[rcx-0x40]\n"
        "cmove rsi,QWORD PTR fs:0x28\n"
        "cmove dx,WORD PTR [r9]\n"
        "cmovl r15d,DWORD PTR fs:0x28\n"
        "cmovl rax,QWORD PTR [rcx+0x8]\n"
        "cmovl bx,WORD PTR gs:[r15+0x0]\n"
        "cmovns r8w,WORD PTR ds:0x1234\n"
        "cmovns dx,WORD PTR [rip+0x0]\n"
        "cmovns di,WORD PTR fs:0x28\n"
        "cmp r15b,BYTE PTR fs:[rbp+0x12345678]\n"
        "cmp WORD PTR [r12*8+0x1000],0xff\n"
        "cmp WORD PTR [rbx],0xff80\n"
        "cmps WORD PTR ds:[rsi],WORD PTR es:[rdi]\n"
        "cmpxchg cx,di\n"
        "cmpxchg16b OWORD PTR [rax+rdi*4-0x80]\n"
        "cmpxchg16b OWORD PTR [eax+0x100]\n"
        "cmpxchg16b OWORD PTR gs:[rbp+0x0]\n"
        "cmpxchg8b QWORD PTR [rdx*2+0x10]\n"
        "cmpxchg8b QWORD PTR fs:[rsi-0x40]\n"
        "cmpxchg8b QWORD PTR [rsi+r13*4-0x40]\n"
        "comiss xmm1,DWORD PTR [rip+0x0]\n"
        "comiss xmm11,DWORD PTR ds:0x1234\n"
        "comiss xmm1,xmm7\n"
        "cqo\n"
        "crc32 edx,BYTE PTR fs:0x28\n"
        "crc32 ebx,DWORD PTR [rip+0x0]\n"
        "crc32 esp,BYTE PTR [ebx+0x0]\n"
        "cvtpd2pi mm2,xmm2\n"
        "cvtpd2pi mm1,XMMWORD PTR [r11+rdx*2+0x8]\n"
        "cvtpd2pi mm0,XMMWORD PTR [r8-0x80]\n"
        "cvtpi2pd xmm8,mm3\n"
        "cvtpi2pd xmm14,QWORD PTR [rip+0x0]\n"
        "cvtpi2pd xmm12,mm4\n"
        "cvtpi2ps xmm11,mm5\n"
        "cvtpi2ps xmm7,QWORD PTR [rdx+0x12345678]\n"
        "cvtpi2ps xmm2,mm1\n"
        "cvtps2pi mm1,xmm0\n"
        "cvtps2pi mm1,QWORD PTR [r8+0x80]\n"
        "cvtps2pi mm4,QWORD PTR [r8*2+0x10]\n"
        "cvtsd2ss xmm3,QWORD PTR [ebx+0x14]\n"
        "cvtsd2ss xmm12,QWORD PTR [rdx+rbx*2+0x12345678]\n"
        "cvtsd2ss xmm13,QWORD PTR [r12+r15*2]\n"
        "cvtsi2sd xmm3,QWORD PTR [r9*8+0x10]\n"
        "cvtsi2sd xmm11,QWORD PTR [rsi+0x8]\n"
        "cvtsi2sd xmm5,DWORD PTR [rsp+r9*8]\n"
        "cvtsi2ss xmm9,QWORD PTR [rsi*4+0x0]\n"
        "cvtsi2ss xmm1,QWORD PTR [ebx+0x14]\n"
        "cvtsi2ss xmm4,rsp\n"
        "cvtss2sd xmm15,DWORD PTR gs:[rbp+0x14]\n"
        "cvtss2sd xmm15,DWORD PTR [rax]\n"
        "cvtss2sd xmm0,DWORD PTR [esi-0x80]\n"
        "cvttpd2pi mm2,XMMWORD PTR [rip+0x100]\n"
        "cvttpd2pi mm0,xmm3\n"
        "cvttpd2pi mm6,XMMWORD PTR [rsi*8+0x1000]\n"
        "cvttps2pi mm5,QWORD PTR [r10+rdi*4+0x8]\n"
        "cvttps2pi mm0,xmm0\n"
        "cvttps2pi mm3,QWORD PTR [r14+r8*4-0x40]\n"
        "cvttsd2si r15,QWORD PTR [rip+0x0]\n"
        "cvttsd2si r9,QWORD PTR [rbp+rdi*1+0x0]\n"
        "cvttsd2si r8,QWORD PTR [rsi-0x40]\n"
        "cwde\n"
        "dec WORD PTR gs:[r13-0x80]\n"
        "dec QWORD PTR [r12+rsi*2-0x40]\n"
        "dec DWORD PTR gs:[rdi+0x8]\n"
        "div DWORD PTR fs:0x28\n"
        "div QWORD PTR [eax+0x0]\n"
        "div QWORD PTR ds:0x1234\n"
        "endbr32\n"
        "endbr64\n"
        "enter 0x10,0x0\n"
        "f2xm1\n"
        "fabs\n"
        "fadd QWORD PTR gs:[r9+0x0]\n"
        "fadd DWORD PTR [rbp-0x8]\n"
        "fadd QWORD PTR [r12-0x8]\n"
        "faddp st(1),st\n"
        "fbld TBYTE PTR [rip+0x100]\n"
        "fbld TBYTE PTR [rcx*4+0x10]\n"
        "fbld TBYTE PTR [rsi+r8*8]\n"
        "fbstp TBYTE PTR fs:0x28\n"
        "fbstp TBYTE PTR fs:[r8+0x14]\n"
        "fbstp TBYTE PTR [r12+rsi*8+0x100]\n"
        "fchs\n"
        "fclex\n"
        "fcmovb st,st(0)\n"
        "fcmovbe st,st(2)\n"
        "fcmove st,st(1)\n"
        "fcmovnb st,st(3)\n"
        "fcmovnbe st,st(2)\n"
        "fcmovne st,st(4)\n"
        "fcmovnu st,st(0)\n"
        "fcmovu st,st(3)\n"
        "fcom QWORD PTR [r11+r11*2-0x40]\n"
        "fcom DWORD PTR [rdx+rbp*8-0x1000]\n"
        "fcom QWORD PTR fs:[rsp+0x0]\n"
        "fcomi st,st(3)\n"
        "fcomip st,st(4)\n"
        "fcomp DWORD PTR [rbp+r15*4-0x1000]\n"
        "fcomp DWORD PTR fs:0x28\n"
        "fcomp QWORD PTR [rip+0x0]\n"
        "fcompp\n"
        "fcos\n"
        "fdecstp\n"
        "fdiv DWORD PTR gs:[r9+0x7f]\n"
        "fdiv DWORD PTR [rbx+rbp*8-0x2f]\n"
        "fdiv QWORD PTR ds:0x0\n"
        "fdivp st(1),st\n"
        "fdivr DWORD PTR gs:[rsp+0x0]\n"
        "fdivr QWORD PTR [rcx+0x8]\n"
        "fdivr QWORD PTR fs:0x28\n"
        "fdivrp st(1),st\n"
        "femms\n"
        "ffree st(7)\n"
        "ffreep st(4)\n"
        "fiadd WORD PTR [rcx-0x80]\n"
        "fiadd DWORD PTR fs:[rbp-0x80]\n"
        "fiadd DWORD PTR [r13-0x40]\n"
        "ficom DWORD PTR [rax]\n"
        "ficom DWORD PTR [r9+r14*8-0x80]\n"
        "ficom DWORD PTR [rip+0x0]\n"
        "ficomp WORD PTR [rcx+r8*4]\n"
        "ficomp DWORD PTR [r14*8+0x1000]\n"
        "ficomp DWORD PTR [r15]\n"
        "fidiv WORD PTR [eax]\n"
        "fidiv DWORD PTR [r11+rcx*1-0x1000]\n"
        "fidiv DWORD PTR [r10+rsi*2+0x8]\n"
        "fidivr DWORD PTR [r15-0x40]\n"
        "fidivr WORD PTR [r12+rsi*1+0x1000]\n"
        "fidivr DWORD PTR ds:0x1234\n"
        "fild DWORD PTR [rdx+rdi*4-0x67]\n"
        "fild WORD PTR gs:[rbp+0x12345678]\n"
        "fild DWORD PTR [r15+0x100]\n"
        "fimul DWORD PTR fs:[rbp+0x100]\n"
        "fimul DWORD PTR ds:0x0\n"
        "fimul DWORD PTR [r9+r10*1+0x80]\n"
        "fincstp\n"
        "finit\n"
        "fist DWORD PTR [rax*4+0x1000]\n"
        "fist WORD PTR [rdi+r10*2+0x80]\n"
        "fist WORD PTR [rsi*8+0x10]\n"
        "fistp QWORD PTR [r12*2+0x10]\n"
        "fistp DWORD PTR [r8*8+0x1000]\n"
        "fistp DWORD PTR [r11-0x40]\n"
        "fisttp DWORD PTR [r12+rbp*8+0x80]\n"
        "fisttp QWORD PTR [rax+rdi*8+0x40]\n"
        "fisttp DWORD PTR [r11+rcx*2-0x40]\n"
        "fisub DWORD PTR [rbp+0x40]\n"
        "fisub DWORD PTR [r10+r10*2-0x40]\n"
        "fisub WORD PTR [rcx+0x12345678]\n"
        "fisubr DWORD PTR [r13+rax*4-0x40]\n"
        "fisubr WORD PTR [r8*2+0x10]\n"
        "fisubr DWORD PTR fs:0x28\n"
        "fld DWORD PTR gs:[r15-0x40]\n"
        "fld DWORD PTR [r9]\n"
        "fld DWORD PTR [r14-0x80]\n"
        "fld1\n"
        "fldcw WORD PTR [rdi]\n"
        "fldcw WORD PTR [rsi*4+0x1000]\n"
        "fldcw WORD PTR [r8+r15*4+0x80]\n"
        "fldenv [rdx*2+0x1000]\n"
        "fldenv [rax+rdi*4+0x80]\n"
        "fldenv [rip+0xfffffffffffffff8]\n"
        "fldl2e\n"
        "fldl2t\n"
        "fldlg2\n"
        "fldln2\n"
        "fldpi\n"
        "fldz\n"
        "fmul DWORD PTR [rip+0x0]\n"
        "fmul DWORD PTR gs:[rdi+0x14]\n"
        "fmul QWORD PTR ds:0x1234\n"
        "fmulp st(1),st\n"
        "fnclex\n"
        "fninit\n"
        "fnop\n"
        "fnsave [r14-0x40]\n"
        "fnsave [r10+rdi*4-0x40]\n"
        "fnsave [r12+0x80]\n"
        "fnstcw WORD PTR [rip+0x0]\n"
        "fnstcw WORD PTR [rsi*8+0x10]\n"
        "fnstcw WORD PTR [r10+r12*4+0x100]\n"
        "fnstenv [r10+r14*1+0x12345678]\n"
        "fnstenv [r8*4+0x10]\n"
        "fnstenv [rbx-0x1000]\n"
        "fnstsw WORD PTR [r9+rax*1+0x0]\n"
        "fnstsw WORD PTR [rax]\n"
        "fnstsw WORD PTR gs:[r12+0x12345678]\n"
        "fpatan\n"
        "fprem\n"
        "fprem1\n"
        "fptan\n"
        "frndint\n"
        "frstor [r15+rsi*8-0x1000]\n"
        "frstor [rcx*2+0x1000]\n"
        "frstor [r10]\n"
        "fsave [rdi+0x40]\n"
        "fsave [rsi+rcx*4+0x8]\n"
        "fsave [rcx+rdi*8-0x40]\n"
        "fscale\n"
        "fsin\n"
        "fsincos\n"
        "fsqrt\n"
        "fst DWORD PTR [r15-0x80]\n"
        "fst QWORD PTR [r8+rsi*8+0xd1]\n"
        "fst DWORD PTR [rbp*8+0x10]\n"
        "fstcw WORD PTR [r8+0x1000]\n"
        "fstcw WORD PTR [r13*2+0x1000]\n"
        "fstcw WORD PTR [rbp+r10*1+0x80]\n"
        "fstenv [rax+0x80]\n"
        "fstenv [r8+r9*1+0x8]\n"
        "fstenv [rdi+r9*1]\n"
        "fstp DWORD PTR [rbx+rcx*2]\n"
        "fstp QWORD PTR [r14+rdi*8+0x39]\n"
        "fstp QWORD PTR [rcx]\n"
        "fstsw WORD PTR [rip+0x0]\n"
        "fstsw WORD PTR [r13-0x80]\n"
        "fstsw WORD PTR [rcx+r15*2+0x1000]\n"
        "fsub DWORD PTR [rdi+rdx*8]\n"
        "fsub DWORD PTR [rax+rdi*2+0x35]\n"
        "fsub QWORD PTR ds:0x1234\n"
        "fsubp st(1),st\n"
        "fsubr DWORD PTR [r8]\n"
        "fsubr QWORD PTR [r12]\n"
        "fsubr DWORD PTR [rcx+0x100]\n"
        "fsubrp st(1),st\n"
        "ftst\n"
        "fucom st(1)\n"
        "fucomi st,st(3)\n"
        "fucomip st,st(5)\n"
        "fucomp st(1)\n"
        "fucompp\n"
        "fxam\n"
        "fxch st(4)\n"
        "fxrstor [rcx-0x80]\n"
        "fxrstor [r10+rcx*2-0x1000]\n"
        "fxrstor [rip+0xfffffffffffffff8]\n"
        "fxrstor64 [rip+0xfffffffffffffff8]\n"
        "fxrstor64 [r15*4+0x1000]\n"
        "fxrstor64 [r13+0x8]\n"
        "fxsave [rcx+0x12345678]\n"
        "fxsave [rbp+r10*8+0x12345678]\n"
        "fxsave ds:0x0\n"
        "fxsave64 [rsi]\n"
        "fxsave64 [rip+0xfffffffffffffff8]\n"
        "fxsave64 [rbp+0x0]\n"
        "fxtract\n"
        "fyl2x\n"
        "fyl2xp1\n"
        "gf2p8affineinvqb xmm7,xmm7,0x96\n"
        "gf2p8affineinvqb xmm6,XMMWORD PTR [r14+rsi*8-0x80],0x37\n"
        "gf2p8affineinvqb xmm10,XMMWORD PTR [rbx+0x80],0x77\n"
        "gf2p8affineqb xmm1,XMMWORD PTR [rdx+r8*1+0x1000],0xd1\n"
        "gf2p8affineqb xmm6,xmm12,0x81\n"
        "gf2p8affineqb xmm8,XMMWORD PTR [rbx+rax*4],0x44\n"
        "gf2p8mulb xmm8,XMMWORD PTR [rbx*4+0x10]\n"
        "gf2p8mulb xmm14,XMMWORD PTR [rax+0x100]\n"
        "gf2p8mulb xmm12,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "hlt\n"
        "idiv DWORD PTR fs:[rax-0x80]\n"
        "idiv BYTE PTR [rdi+0x7f]\n"
        "idiv DWORD PTR [r12*2+0x0]\n"
        "imul bx,WORD PTR ds:0x1234\n"
        "imul WORD PTR [esi+0x8]\n"
        "imul BYTE PTR [rip+0x0]\n"
        "in al,dx\n"
        "in al,0xd0\n"
        "inc WORD PTR [r8+r13*4+0x14]\n"
        "inc BYTE PTR gs:[r9+0x100]\n"
        "inc QWORD PTR ds:0x0\n"
        "ins WORD PTR es:[rdi],dx\n"
        "ins BYTE PTR es:[rdi],dx\n"
        "ins DWORD PTR es:[rdi],dx\n"
        "int 0x80\n"
        "int3\n"
        "invd\n"
        "invlpg BYTE PTR [rip+0x100]\n"
        "invlpg BYTE PTR [r15+rbp*8+0x40]\n"
        "invlpg BYTE PTR [r8*4+0x10]\n"
        "iretq\n"
        "iretw\n"
        "kaddb k1,k4,k0\n"
        "kaddb k6,k2,k0\n"
        "kaddb k6,k2,k1\n"
        "kaddd k5,k2,k0\n"
        "kaddd k1,k5,k1\n"
        "kaddd k5,k6,k1\n"
        "kaddq k0,k6,k6\n"
        "kaddq k6,k3,k4\n"
        "kaddq k0,k0,k0\n"
        "kaddw k4,k4,k5\n"
        "kaddw k0,k1,k4\n"
        "kaddw k7,k5,k0\n"
        "kandb k2,k1,k7\n"
        "kandb k4,k5,k4\n"
        "kandb k7,k4,k3\n"
        "kandd k2,k5,k2\n"
        "kandd k1,k6,k3\n"
        "kandd k3,k5,k5\n"
        "kandnb k0,k3,k7\n"
        "kandnb k2,k6,k6\n"
        "kandnb k6,k7,k4\n"
        "kandnd k2,k4,k0\n"
        "kandnd k3,k2,k0\n"
        "kandnd k1,k6,k0\n"
        "kandnq k4,k6,k1\n"
        "kandnq k0,k2,k4\n"
        "kandnq k2,k0,k5\n"
        "kandnw k1,k1,k0\n"
        "kandnw k0,k1,k7\n"
        "kandnw k3,k4,k7\n"
        "kandq k1,k2,k1\n"
        "kandq k1,k6,k3\n"
        "kandq k5,k2,k6\n"
        "kandw k0,k5,k3\n"
        "kandw k3,k3,k1\n"
        "kandw k6,k5,k2\n"
        "kmovb k6,k6\n"
        "kmovb k4,BYTE PTR [rip+0xfffffffffffffff8]\n"
        "kmovb k6,k2\n"
        "kmovd k7,k5\n"
        "kmovd k3,DWORD PTR [rip+0x100]\n"
        "kmovd DWORD PTR [rsi+0x12345678],k7\n"
        "kmovq k0,rsi\n"
        "kmovq k0,k5\n"
        "kmovq rbp,k7\n"
        "kmovw k7,k3\n"
        "kmovw k5,edi\n"
        "kmovw k4,k1\n"
        "knotb k1,k7\n"
        "knotb k2,k1\n"
        "knotb k2,k7\n"
        "knotd k3,k6\n"
        "knotd k2,k6\n"
        "knotd k5,k6\n"
        "knotq k5,k1\n"
        "knotq k0,k7\n"
        "knotq k7,k4\n"
        "knotw k3,k0\n"
        "knotw k4,k0\n"
        "knotw k4,k1\n"
        "korb k4,k1,k2\n"
        "korb k1,k0,k4\n"
        "korb k2,k5,k3\n"
        "kord k2,k3,k5\n"
        "kord k6,k0,k0\n"
        "kord k0,k6,k1\n"
        "korq k4,k3,k2\n"
        "korq k2,k2,k1\n"
        "korq k7,k1,k2\n"
        "kortestb k5,k7\n"
        "kortestb k0,k6\n"
        "kortestb k4,k3\n"
        "kortestd k4,k2\n"
        "kortestd k3,k6\n"
        "kortestd k3,k7\n"
        "kortestq k6,k5\n"
        "kortestq k0,k5\n"
        "kortestq k4,k4\n"
        "kortestw k3,k3\n"
        "kortestw k5,k3\n"
        "kortestw k1,k4\n"
        "korw k2,k7,k7\n"
        "korw k5,k7,k6\n"
        "korw k7,k6,k7\n"
        "ktestb k0,k6\n"
        "ktestb k1,k4\n"
        "ktestb k6,k5\n"
        "ktestd k5,k7\n"
        "ktestd k3,k0\n"
        "ktestd k1,k0\n"
        "ktestq k6,k1\n"
        "ktestq k3,k0\n"
        "ktestq k6,k6\n"
        "ktestw k1,k2\n"
        "ktestw k5,k7\n"
        "ktestw k2,k0\n"
        "kxnorb k0,k2,k0\n"
        "kxnorb k4,k5,k5\n"
        "kxnorb k1,k5,k4\n"
        "kxnord k6,k4,k5\n"
        "kxnord k0,k5,k3\n"
        "kxnord k1,k5,k0\n"
        "kxnorq k7,k2,k7\n"
        "kxnorq k6,k1,k4\n"
        "kxnorq k0,k5,k0\n"
        "kxnorw k3,k3,k3\n"
        "kxnorw k0,k7,k4\n"
        "kxnorw k2,k6,k3\n"
        "kxorb k1,k7,k3\n"
        "kxorb k2,k4,k5\n"
        "kxorb k5,k0,k2\n"
        "kxord k6,k4,k0\n"
        "kxord k3,k6,k2\n"
        "kxord k7,k6,k4\n"
        "kxorq k7,k3,k5\n"
        "kxorq k4,k4,k7\n"
        "kxorq k5,k0,k0\n"
        "kxorw k1,k0,k2\n"
        "kxorw k3,k3,k3\n"
        "kxorw k3,k4,k5\n"
        "ldmxcsr DWORD PTR fs:0x28\n"
        "ldmxcsr DWORD PTR gs:[rsp-0x40]\n"
        "ldmxcsr DWORD PTR ds:0x0\n"
        "lea esp,[eax]\n"
        "lea r12,[rbp+rbx*1-0x40]\n"
        "lea rsp,[rax+0x7f]\n"
        "leave\n"
        "lgdt [rdi-0x1000]\n"
        "lgdt [rbx]\n"
        "lgdt [r15+rbx*1+0x12345678]\n"
        "lidt [rbp*4+0x1000]\n"
        "lidt [r10+0x8]\n"
        "lidt [rip+0x0]\n"
        "lldt WORD PTR [r8+0x1000]\n"
        "lldt WORD PTR [r12*2+0x1000]\n"
        "lldt WORD PTR [r9+r9*1+0x12345678]\n"
        "lmsw WORD PTR [rsi]\n"
        "lmsw WORD PTR [rip+0x0]\n"
        "lmsw r8w\n"
        "lock add DWORD PTR fs:[rbx],0x1\n"
        "lock add DWORD PTR [rbx+0x8],0x1\n"
        "lock add DWORD PTR [r15+rsi*2],0x1\n"
        "lods eax,DWORD PTR ds:[rsi]\n"
        "ltr WORD PTR [r14*2+0x10]\n"
        "ltr WORD PTR [rsi+rdx*2+0x100]\n"
        "ltr WORD PTR [rip+0x100]\n"
        "lzcnt r12,QWORD PTR [r9*4+0x1000]\n"
        "lzcnt cx,WORD PTR fs:[rsi-0x80]\n"
        "lzcnt edx,DWORD PTR gs:[r12+0x0]\n"
        "maskmovq mm2,mm3\n"
        "maskmovq mm6,mm5\n"
        "maskmovq mm6,mm2\n"
        "monitor\n"
        "mov dr7,rdx\n"
        "mov ebx,DWORD PTR [rdi*4+0x1000]\n"
        "mov BYTE PTR [r13+0x0],r8b\n"
        "movabs rax,0x1122334455667788\n"
        "movaps xmm3,XMMWORD PTR [rdx+0x8]\n"
        "movaps xmm0,XMMWORD PTR ds:0x1234\n"
        "movaps xmm1,XMMWORD PTR [rcx+rbx*8+0x7f]\n"
        "movbe ebp,DWORD PTR gs:[rdi+0x12345678]\n"
        "movbe edx,DWORD PTR [rip+0x0]\n"
        "movbe ecx,DWORD PTR [r9*8+0x10]\n"
        "movd xmm6,DWORD PTR [esi]\n"
        "movd mm3,DWORD PTR [rax-0x40]\n"
        "movd xmm4,DWORD PTR [rbp+0x0]\n"
        "movdq2q mm7,xmm12\n"
        "movdq2q mm5,xmm2\n"
        "movdq2q mm4,xmm12\n"
        "movdqa xmm13,XMMWORD PTR [rip+0x0]\n"
        "movdqa xmm1,XMMWORD PTR [ebx-0x40]\n"
        "movdqa xmm3,XMMWORD PTR ds:0x1234\n"
        "movdqu xmm2,XMMWORD PTR [rdx+rdx*8-0x40]\n"
        "movdqu xmm4,XMMWORD PTR ds:0x0\n"
        "movdqu xmm0,XMMWORD PTR [ebx-0x40]\n"
        "movnti QWORD PTR [rcx],rsi\n"
        "movnti QWORD PTR fs:[rbp-0x80],r15\n"
        "movnti QWORD PTR [esi-0x40],rsp\n"
        "movntq QWORD PTR [rax+r13*2-0x1000],mm4\n"
        "movntq QWORD PTR [rdx+r8*8+0x40],mm7\n"
        "movntq QWORD PTR [r15*2+0x1000],mm4\n"
        "movq r13,mm7\n"
        "movq mm0,mm1\n"
        "movq mm4,mm6\n"
        "movq2dq xmm8,mm0\n"
        "movq2dq xmm1,mm6\n"
        "movq2dq xmm2,mm1\n"
        "movs QWORD PTR es:[rdi],QWORD PTR ds:[rsi]\n"
        "movsd xmm3,QWORD PTR fs:[r9+0x0]\n"
        "movsd xmm13,QWORD PTR gs:[r12+0x0]\n"
        "movsd xmm0,xmm15\n"
        "movss xmm6,DWORD PTR [r9*8+0x1000]\n"
        "movss xmm6,DWORD PTR [esi-0x40]\n"
        "movss xmm6,DWORD PTR [rbx]\n"
        "movsx esp,ax\n"
        "movsx ax,BYTE PTR gs:[r9-0x80]\n"
        "movsx edx,WORD PTR ds:0x0\n"
        "movsxd rax,DWORD PTR [eax]\n"
        "movsxd rbp,DWORD PTR fs:0x28\n"
        "movsxd r8,DWORD PTR [eax+0x14]\n"
        "movups xmm4,XMMWORD PTR [rax*4+0x0]\n"
        "movups xmm15,XMMWORD PTR [eax]\n"
        "movups xmm10,XMMWORD PTR [rax+rbp*2+0x0]\n"
        "movzx rax,WORD PTR gs:[rdi+0x0]\n"
        "movzx esp,BYTE PTR fs:[rdi+0x7f]\n"
        "movzx ecx,BYTE PTR fs:0x28\n"
        "mul BYTE PTR [esi]\n"
        "mul QWORD PTR ds:0x1234\n"
        "mul BYTE PTR gs:[rsp-0x40]\n"
        "mulps xmm10,XMMWORD PTR [rbx*4+0x0]\n"
        "mulps xmm7,XMMWORD PTR [esi-0x80]\n"
        "mulps xmm8,XMMWORD PTR ds:0x0\n"
        "mwait\n"
        "neg WORD PTR [r12+rax*1]\n"
        "neg QWORD PTR [rcx+rbp*2+0x7f]\n"
        "neg DWORD PTR [rbx+rbp*8-0x40]\n"
        "nop\n"
        "nop WORD PTR [rax+rax*1]\n"
        "nop DWORD PTR [rax]\n"
        "not WORD PTR [esi-0x40]\n"
        "not QWORD PTR [eax+0x8]\n"
        "not DWORD PTR [rip+0x0]\n"
        "or edx,DWORD PTR fs:[rax+0x0]\n"
        "or BYTE PTR fs:[r15-0x40],0x80\n"
        "or r15w,WORD PTR fs:[rcx]\n"
        "out dx,al\n"
        "out 0xab,al\n"
        "outs dx,WORD PTR ds:[rsi]\n"
        "outs dx,BYTE PTR ds:[rsi]\n"
        "outs dx,DWORD PTR ds:[rsi]\n"
        "pabsb mm3,mm5\n"
        "pabsb mm0,QWORD PTR [rip+0xfffffffffffffff8]\n"
        "pabsb mm2,QWORD PTR [r11+0x40]\n"
        "pabsd mm1,mm4\n"
        "pabsd mm6,QWORD PTR [r14-0x80]\n"
        "pabsd mm4,QWORD PTR [rip+0xfffffffffffffff8]\n"
        "pabsw mm2,QWORD PTR [r15+r15*1+0x40]\n"
        "pabsw mm2,mm4\n"
        "pabsw mm6,QWORD PTR [r11+0x1000]\n"
        "packssdw mm4,QWORD PTR [r9+0x1000]\n"
        "packssdw mm1,QWORD PTR [r12-0x1000]\n"
        "packssdw mm7,mm0\n"
        "packsswb mm2,QWORD PTR [rbp+0x40]\n"
        "packsswb mm7,mm2\n"
        "packsswb mm7,QWORD PTR [rcx*2+0x1000]\n"
        "packuswb mm7,QWORD PTR [rip+0xfffffffffffffff8]\n"
        "packuswb mm2,mm7\n"
        "packuswb mm2,QWORD PTR [r8-0x40]\n"
        "paddb mm2,mm3\n"
        "paddb mm3,QWORD PTR [rbp-0x8]\n"
        "paddb mm5,mm0\n"
        "paddd mm2,mm0\n"
        "paddd xmm4,XMMWORD PTR [rbx+0x12345678]\n"
        "paddd mm7,mm0\n"
        "paddq mm1,mm4\n"
        "paddq mm3,QWORD PTR [r8+rbp*2-0xfb]\n"
        "paddq mm1,mm5\n"
        "paddsb mm7,mm0\n"
        "paddsb mm1,QWORD PTR [rcx*2+0x1000]\n"
        "paddsb mm3,mm0\n"
        "paddsw mm1,mm4\n"
        "paddsw mm0,mm6\n"
        "paddsw xmm4,XMMWORD PTR [r15+rdi*8-0x1000]\n"
        "paddusb mm7,QWORD PTR [rbx*4+0x1000]\n"
        "paddusb mm3,mm5\n"
        "paddusb mm1,QWORD PTR [r9*2+0x1000]\n"
        "paddusw mm1,mm1\n"
        "paddusw xmm2,XMMWORD PTR [r11+r8*4]\n"
        "paddusw mm2,QWORD PTR [r11+r9*8+0x40]\n"
        "paddw mm5,mm2\n"
        "paddw mm4,mm6\n"
        "paddw mm3,QWORD PTR [rax+0x100]\n"
        "palignr mm7,QWORD PTR [rdx+0x12345678],0xa9\n"
        "palignr mm0,QWORD PTR [r8-0x80],0xf3\n"
        "palignr mm7,QWORD PTR [rbp+rbx*2+0x1000],0x33\n"
        "pand mm4,QWORD PTR [rax+0x100]\n"
        "pand xmm2,XMMWORD PTR [rdi+rbx*8+0x40]\n"
        "pand mm3,mm0\n"
        "pandn mm5,mm7\n"
        "pandn mm0,QWORD PTR [rip+0x100]\n"
        "pandn mm5,QWORD PTR [r12-0x1000]\n"
        "pavgb mm6,QWORD PTR [r12+r10*1]\n"
        "pavgb xmm14,xmm9\n"
        "pavgb xmm13,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "pavgw mm2,QWORD PTR [r11+0x100]\n"
        "pavgw mm0,QWORD PTR [r8+r11*4-0x40]\n"
        "pavgw mm4,QWORD PTR [r14+0x100]\n"
        "pblendw xmm14,XMMWORD PTR [rip+0x0],0x5\n"
        "pblendw xmm13,XMMWORD PTR [r15+rax*2-0x80],0x5\n"
        "pblendw xmm4,XMMWORD PTR ds:0x1234,0x5\n"
        "pclmulhqhqdq xmm8,xmm10\n"
        "pclmulhqhqdq xmm15,XMMWORD PTR [r12*2+0x1000]\n"
        "pclmulhqhqdq xmm5,XMMWORD PTR [r11+0x12345678]\n"
        "pclmulhqlqdq xmm5,xmm13\n"
        "pclmulhqlqdq xmm7,XMMWORD PTR [r12-0x1000]\n"
        "pclmulhqlqdq xmm15,XMMWORD PTR [r9+rbx*1+0x80]\n"
        "pclmullqhqdq xmm6,XMMWORD PTR [rdi+0x1000]\n"
        "pclmullqhqdq xmm7,xmm4\n"
        "pclmullqhqdq xmm9,XMMWORD PTR [r12*4+0x1000]\n"
        "pclmullqlqdq xmm7,XMMWORD PTR [rcx+r14*1+0x40]\n"
        "pclmullqlqdq xmm5,xmm15\n"
        "pclmullqlqdq xmm0,XMMWORD PTR [r13+rdx*4-0x80]\n"
        "pcmpeqb mm3,mm0\n"
        "pcmpeqb mm1,mm0\n"
        "pcmpeqb xmm12,XMMWORD PTR [rbx+rdx*2]\n"
        "pcmpeqd xmm5,xmm6\n"
        "pcmpeqd mm6,mm2\n"
        "pcmpeqd mm0,mm7\n"
        "pcmpeqq xmm8,XMMWORD PTR [r15-0x1000]\n"
        "pcmpeqq xmm5,xmm4\n"
        "pcmpeqq xmm15,XMMWORD PTR [rdi*4+0x10]\n"
        "pcmpeqw mm7,mm2\n"
        "pcmpeqw mm0,QWORD PTR [r13+r8*1+0x12345678]\n"
        "pcmpeqw mm4,mm1\n"
        "pcmpestri xmm8,XMMWORD PTR [r15+rax*2+0x1000],0x56\n"
        "pcmpestri xmm15,XMMWORD PTR [rip+0x0],0x9c\n"
        "pcmpestri xmm14,XMMWORD PTR [r13*8+0x10],0xa1\n"
        "pcmpestrm xmm12,xmm8,0xf7\n"
        "pcmpestrm xmm13,XMMWORD PTR [rip+0x0],0x8d\n"
        "pcmpestrm xmm14,XMMWORD PTR [rsi+0x12345678],0x83\n"
        "pcmpgtb mm2,QWORD PTR [r15-0x40]\n"
        "pcmpgtb mm4,mm7\n"
        "pcmpgtb xmm3,XMMWORD PTR [rbp*4+0x1000]\n"
        "pcmpgtd mm1,mm6\n"
        "pcmpgtd mm4,QWORD PTR [r9+r12*1+0x12345678]\n"
        "pcmpgtd mm1,mm4\n"
        "pcmpgtq xmm7,XMMWORD PTR [r13*2+0x10]\n"
        "pcmpgtq xmm11,xmm12\n"
        "pcmpgtq xmm10,XMMWORD PTR [rdx+r15*1+0x8]\n"
        "pcmpgtw mm2,QWORD PTR [r9+r12*8+0x100]\n"
        "pcmpgtw mm3,mm0\n"
        "pcmpgtw mm0,mm7\n"
        "pcmpistri xmm15,XMMWORD PTR [rdx+rsi*8-0x1000],0xe1\n"
        "pcmpistri xmm8,XMMWORD PTR [rdx+r11*4+0x80],0x4c\n"
        "pcmpistri xmm14,xmm8,0x0\n"
        "pcmpistrm xmm1,XMMWORD PTR [r15*2+0x10],0xa2\n"
        "pcmpistrm xmm5,XMMWORD PTR [r10+r10*4-0x80],0x70\n"
        "pcmpistrm xmm15,XMMWORD PTR [rbx+0x100],0xee\n"
        "pdep ebx,ecx,eax\n"
        "pextrw edi,mm0,0x2e\n"
        "pextrw r14d,mm2,0xd6\n"
        "pextrw edi,xmm10,0x1\n"
        "phaddd mm2,QWORD PTR [rip+0xfffffffffffffff8]\n"
        "phaddd mm7,QWORD PTR [rax+r9*1+0x1000]\n"
        "phaddd mm6,mm3\n"
        "phaddsw mm1,QWORD PTR [rip+0x0]\n"
        "phaddsw mm5,QWORD PTR [rip+0xfffffffffffffff8]\n"
        "phaddsw mm1,QWORD PTR [rcx*4+0x1000]\n"
        "phaddw mm3,mm5\n"
        "phaddw mm1,mm3\n"
        "phaddw mm5,mm4\n"
        "phsubd mm0,mm4\n"
        "phsubd mm1,mm5\n"
        "phsubd mm5,QWORD PTR [r13+r8*8-0x40]\n"
        "phsubsw mm4,mm3\n"
        "phsubsw mm6,QWORD PTR [rdx+0x12345678]\n"
        "phsubsw mm0,QWORD PTR [rdi*2+0x10]\n"
        "phsubw mm6,mm5\n"
        "phsubw mm2,QWORD PTR [rdi+rbp*2+0x80]\n"
        "phsubw mm0,QWORD PTR [r10+0x8]\n"
        "pinsrw mm5,WORD PTR [rsi-0x1000],0x54\n"
        "pinsrw xmm3,WORD PTR ds:0x0,0x2\n"
        "pinsrw mm4,WORD PTR [rsi*8+0x10],0xaf\n"
        "pmaddubsw mm5,mm4\n"
        "pmaddubsw mm2,mm6\n"
        "pmaddubsw mm7,mm6\n"
        "pmaddwd mm4,mm7\n"
        "pmaddwd mm2,QWORD PTR [rdx+rsi*2]\n"
        "pmaddwd mm5,mm3\n"
        "pmaxsd xmm14,XMMWORD PTR [r13+rcx*8-0x80]\n"
        "pmaxsd xmm5,XMMWORD PTR [rdx+rbp*4+0x12345678]\n"
        "pmaxsd xmm7,xmm5\n"
        "pmaxsw mm5,mm2\n"
        "pmaxsw mm1,mm7\n"
        "pmaxsw mm4,mm7\n"
        "pmaxub mm5,mm0\n"
        "pmaxub mm2,QWORD PTR [rip+0xfffffffffffffff8]\n"
        "pmaxub xmm12,XMMWORD PTR [r10+0x100]\n"
        "pmaxud xmm1,XMMWORD PTR [rip+0x100]\n"
        "pmaxud xmm5,xmm14\n"
        "pmaxud xmm9,XMMWORD PTR [r15-0x80]\n"
        "pminsd xmm6,XMMWORD PTR [r13+0x40]\n"
        "pminsd xmm14,XMMWORD PTR [rbx*4+0x1000]\n"
        "pminsd xmm11,xmm5\n"
        "pminsw mm3,mm6\n"
        "pminsw mm5,mm0\n"
        "pminsw mm3,mm7\n"
        "pminub xmm0,xmm14\n"
        "pminub mm2,mm6\n"
        "pminub mm3,mm2\n"
        "pminud xmm2,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "pminud xmm5,xmm12\n"
        "pminud xmm3,XMMWORD PTR [rdi+r9*2-0x40]\n"
        "pmovmskb edi,mm0\n"
        "pmovmskb r9d,mm7\n"
        "pmovmskb r13d,mm1\n"
        "pmovzxbw xmm15,QWORD PTR [rdx]\n"
        "pmovzxbw xmm9,QWORD PTR [rbx+0x0]\n"
        "pmovzxbw xmm0,QWORD PTR [rdi+r8*8+0x0]\n"
        "pmuldq xmm1,XMMWORD PTR [r15+r9*2]\n"
        "pmuldq xmm14,XMMWORD PTR [rdx+0x8]\n"
        "pmuldq xmm8,xmm5\n"
        "pmulhrsw mm7,QWORD PTR [r15+r14*2+0x80]\n"
        "pmulhrsw mm3,QWORD PTR [r13*2+0x1000]\n"
        "pmulhrsw mm3,mm7\n"
        "pmulhuw mm0,mm0\n"
        "pmulhuw xmm0,xmm3\n"
        "pmulhuw mm6,QWORD PTR [r14*4+0x1000]\n"
        "pmulhw mm4,QWORD PTR [rip+0xfffffffffffffff8]\n"
        "pmulhw mm1,mm3\n"
        "pmulhw mm0,QWORD PTR [rip+0xfffffffffffffff8]\n"
        "pmulld xmm15,XMMWORD PTR [rip+0x0]\n"
        "pmulld xmm8,xmm1\n"
        "pmulld xmm13,XMMWORD PTR [rax-0x1000]\n"
        "pmullw mm2,mm6\n"
        "pmullw mm7,mm3\n"
        "pmullw mm2,mm5\n"
        "pmuludq mm4,mm4\n"
        "pmuludq mm7,QWORD PTR [rip+0xfffffffffffffff8]\n"
        "pmuludq mm0,QWORD PTR [r15+0x40]\n"
        "pop QWORD PTR gs:[r13+0x100]\n"
        "pop QWORD PTR fs:0x28\n"
        "pop QWORD PTR [r8+rdi*8-0x80]\n"
        "popcnt r15,QWORD PTR gs:[r15]\n"
        "popcnt rcx,QWORD PTR [esi+0x8]\n"
        "popcnt rdx,QWORD PTR [eax]\n"
        "por mm4,mm5\n"
        "por mm0,QWORD PTR [r15+r10*1+0x80]\n"
        "por mm1,mm7\n"
        "prefetcht0 BYTE PTR [esi-0x40]\n"
        "prefetcht0 BYTE PTR fs:[r15-0x80]\n"
        "prefetcht0 BYTE PTR [rdx+r8*4-0x80]\n"
        "psadbw xmm11,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "psadbw mm6,mm0\n"
        "psadbw mm6,QWORD PTR [rcx*8+0x1000]\n"
        "pshufb mm0,QWORD PTR [rcx*4+0x1000]\n"
        "pshufb mm3,QWORD PTR [r10*4+0x1000]\n"
        "pshufb mm3,mm0\n"
        "pshufd xmm4,xmm10,0x1b\n"
        "pshufd xmm2,XMMWORD PTR [rcx+rdx*8+0x7f],0x1b\n"
        "pshufd xmm15,XMMWORD PTR fs:0x28,0x1b\n"
        "pshufw mm4,QWORD PTR [rdx+0x40],0x69\n"
        "pshufw mm2,QWORD PTR [r10-0x40],0xfb\n"
        "pshufw mm4,mm6,0x85\n"
        "psignb mm5,mm4\n"
        "psignb mm2,QWORD PTR [r12*4+0x1000]\n"
        "psignb mm7,mm5\n"
        "psignd mm0,mm4\n"
        "psignd mm1,QWORD PTR [r13*2+0x10]\n"
        "psignd mm5,mm4\n"
        "psignw mm3,mm5\n"
        "psignw mm7,mm2\n"
        "psignw mm7,QWORD PTR [rdx+0x8]\n"
        "pslld mm6,mm2\n"
        "pslld mm3,0xd\n"
        "pslld mm6,mm5\n"
        "psllq mm0,0xd2\n"
        "psllq mm1,mm7\n"
        "psllq mm6,0x42\n"
        "psllw mm0,0xf\n"
        "psllw mm2,QWORD PTR [r8+r10*1+0x100]\n"
        "psllw mm2,mm6\n"
        "psrad mm4,QWORD PTR [rip+0x100]\n"
        "psrad mm3,mm2\n"
        "psrad mm6,mm1\n"
        "psraw mm3,0x4\n"
        "psraw mm2,0x1\n"
        "psraw mm5,0x2\n"
        "psrld mm5,0xb8\n"
        "psrld mm6,QWORD PTR [rax+0x80]\n"
        "psrld mm6,QWORD PTR [r8*4+0x10]\n"
        "psrlq mm5,0x9\n"
        "psrlq mm0,mm3\n"
        "psrlq mm6,mm2\n"
        "psrlw mm1,QWORD PTR [rip+0x0]\n"
        "psrlw mm4,QWORD PTR [r8+rbx*4+0x80]\n"
        "psrlw mm2,mm3\n"
        "psubb mm1,mm4\n"
        "psubb mm7,QWORD PTR [r9]\n"
        "psubb mm4,QWORD PTR [r8]\n"
        "psubd xmm3,XMMWORD PTR [r13+0x8]\n"
        "psubd mm2,mm3\n"
        "psubd xmm6,XMMWORD PTR [rip+0x100]\n"
        "psubq mm6,QWORD PTR [r12*2+0x1000]\n"
        "psubq xmm8,XMMWORD PTR [r11+0x80]\n"
        "psubq mm4,QWORD PTR [rcx+0x80]\n"
        "psubsb mm5,mm6\n"
        "psubsb mm1,mm0\n"
        "psubsb xmm15,XMMWORD PTR [r10+r14*2+0x12345678]\n"
        "psubsw mm3,QWORD PTR [rip+0x100]\n"
        "psubsw xmm3,xmm3\n"
        "psubsw xmm5,XMMWORD PTR [rbp+r14*8+0x8]\n"
        "psubusb mm2,mm6\n"
        "psubusb mm3,mm6\n"
        "psubusb mm1,QWORD PTR [r8+r9*4-0x40]\n"
        "psubusw xmm11,xmm11\n"
        "psubusw mm5,mm4\n"
        "psubusw mm2,mm0\n"
        "psubw mm6,mm1\n"
        "psubw mm3,mm6\n"
        "psubw xmm0,xmm14\n"
        "ptest xmm14,XMMWORD PTR ds:0x0\n"
        "ptest xmm9,XMMWORD PTR [r13+r13*8+0x12345678]\n"
        "ptest xmm12,XMMWORD PTR [rip+0x0]\n"
        "punpckhbw mm1,mm0\n"
        "punpckhbw mm2,mm2\n"
        "punpckhbw xmm1,XMMWORD PTR [r15-0x80]\n"
        "punpckhdq xmm7,XMMWORD PTR [rax+0x100]\n"
        "punpckhdq xmm7,XMMWORD PTR [r12*8+0x10]\n"
        "punpckhdq mm4,QWORD PTR [rip+0x100]\n"
        "punpckhqdq xmm5,XMMWORD PTR [rip+0x100]\n"
        "punpckhqdq xmm6,XMMWORD PTR [r12-0x80]\n"
        "punpckhqdq xmm3,xmm8\n"
        "punpckhwd mm0,QWORD PTR [r11*8+0x1000]\n"
        "punpckhwd mm0,mm1\n"
        "punpckhwd xmm8,XMMWORD PTR [r15*4+0x1000]\n"
        "punpcklbw mm5,mm4\n"
        "punpcklbw xmm11,XMMWORD PTR [r13*4+0x10]\n"
        "punpcklbw mm7,DWORD PTR [rax+rcx*4]\n"
        "punpckldq xmm9,XMMWORD PTR [rip+0x0]\n"
        "punpckldq mm5,DWORD PTR [r10*4+0x1000]\n"
        "punpckldq mm6,mm6\n"
        "punpcklqdq xmm5,XMMWORD PTR [rax+rbx*2]\n"
        "punpcklqdq xmm3,xmm7\n"
        "punpcklqdq xmm1,XMMWORD PTR [r12+r12*1+0x40]\n"
        "punpcklwd mm1,mm0\n"
        "punpcklwd xmm6,XMMWORD PTR [rbp*4+0x1000]\n"
        "punpcklwd mm2,DWORD PTR [r9*8+0x1000]\n"
        "push r15\n"
        "push QWORD PTR [rdi-0x40]\n"
        "push QWORD PTR gs:[r12]\n"
        "pxor mm5,mm6\n"
        "pxor mm3,mm0\n"
        "pxor mm4,mm5\n"
        "rcl QWORD PTR fs:[rbx-0x80],cl\n"
        "rcl QWORD PTR [r12*4+0x1000],1\n"
        "rcl WORD PTR [r8*8+0x1000],cl\n"
        "rcr QWORD PTR [rsi+rax*1+0x8],0x0\n"
        "rcr QWORD PTR [esi+0x8],cl\n"
        "rcr DWORD PTR [r8+0x100],1\n"
        "rdmsr\n"
        "rdpid rax\n"
        "rdrand r13d\n"
        "rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]\n"
        "rep stos QWORD PTR es:[rdi],rax\n"
        "repnz scas al,BYTE PTR es:[rdi]\n"
        "ret 0x8\n"
        "ret\n"
        "rol QWORD PTR [rdx+0x100],cl\n"
        "rol QWORD PTR [r13+0x8],0x16\n"
        "rol DWORD PTR [rdi+rsi*4+0x0],cl\n"
        "ror WORD PTR gs:[r13+0x14],1\n"
        "ror BYTE PTR [r12+r12*8+0x100],1\n"
        "ror QWORD PTR fs:[r8],cl\n"
        "rorx eax,DWORD PTR fs:[r13+0x14],0x5\n"
        "rorx r13d,DWORD PTR [r12*2+0x1000],0x5\n"
        "rorx r8d,DWORD PTR [rip+0x0],0x5\n"
        "roundsd xmm7,QWORD PTR ds:0x0,0x1\n"
        "roundsd xmm5,QWORD PTR gs:[r15+0x0],0x1\n"
        "roundsd xmm14,QWORD PTR [r13+r9*4+0x7f],0x1\n"
        "rsm\n"
        "rstorssp QWORD PTR [rip+0x100]\n"
        "rstorssp QWORD PTR [rdi+rbx*1+0x8]\n"
        "rstorssp QWORD PTR [r11+rsi*4]\n"
        "sar DWORD PTR [r9+rbp*4+0x0],1\n"
        "sar BYTE PTR [r8*2+0x10],0x9\n"
        "sar QWORD PTR [r8+r9*1],1\n"
        "saveprevssp\n"
        "sbb DWORD PTR fs:[rcx-0x80],esp\n"
        "sbb BYTE PTR [rdi+rcx*2+0x14],dl\n"
        "sbb QWORD PTR gs:[rcx+0x14],0x0\n"
        "seta BYTE PTR fs:[rcx+0x8]\n"
        "seta BYTE PTR fs:0x28\n"
        "seta sil\n"
        "setb r15b\n"
        "setb BYTE PTR ds:0x1234\n"
        "setb BYTE PTR [rip+0x0]\n"
        "sete dil\n"
        "sete BYTE PTR [rax+r15*8+0x0]\n"
        "setg bpl\n"
        "setg BYTE PTR [rip+0x0]\n"
        "setl sil\n"
        "setl BYTE PTR gs:[rcx]\n"
        "setl BYTE PTR [esi+0x100]\n"
        "setne BYTE PTR ds:0x1234\n"
        "setne BYTE PTR [rsi+rbp*8+0x14]\n"
        "setne al\n"
        "seto BYTE PTR ds:0x1234\n"
        "seto BYTE PTR [ebx-0x80]\n"
        "seto dil\n"
        "setp BYTE PTR [rsi+r15*2-0x40]\n"
        "setp bh\n"
        "setp BYTE PTR [rbp+0x12345678]\n"
        "setssbsy\n"
        "sgdt [r15*4+0x10]\n"
        "sgdt [r10+rdx*1+0x1000]\n"
        "sgdt [rdi]\n"
        "sha1msg1 xmm12,xmm8\n"
        "sha1msg1 xmm4,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "sha1msg1 xmm9,XMMWORD PTR [rax+r14*1]\n"
        "sha1msg2 xmm4,XMMWORD PTR [r12+0x12345678]\n"
        "sha1msg2 xmm12,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "sha1msg2 xmm2,XMMWORD PTR [r8*4+0x1000]\n"
        "sha1nexte xmm0,XMMWORD PTR [rip+0x100]\n"
        "sha1nexte xmm4,XMMWORD PTR [r9+rdi*8+0x100]\n"
        "sha1nexte xmm2,XMMWORD PTR [r11+0x12345678]\n"
        "sha1rnds4 xmm12,XMMWORD PTR [rip+0xfffffffffffffff8],0x58\n"
        "sha1rnds4 xmm0,XMMWORD PTR [r10-0x80],0x1e\n"
        "sha1rnds4 xmm1,xmm8,0x0\n"
        "sha256msg1 xmm13,XMMWORD PTR [r9+r13*4+0x80]\n"
        "sha256msg1 xmm1,XMMWORD PTR [rip+0x0]\n"
        "sha256msg1 xmm1,xmm3\n"
        "sha256msg2 xmm15,XMMWORD PTR [r10*8+0x1000]\n"
        "sha256msg2 xmm2,XMMWORD PTR [rip+0x100]\n"
        "sha256msg2 xmm6,xmm7\n"
        "sha256rnds2 xmm12,XMMWORD PTR [rax+r14*8-0x40],xmm0\n"
        "sha256rnds2 xmm8,XMMWORD PTR [rdx-0x40],xmm0\n"
        "sha256rnds2 xmm15,XMMWORD PTR [rcx],xmm0\n"
        "shl QWORD PTR fs:0x28,0xb\n"
        "shl WORD PTR fs:0x28,cl\n"
        "shl QWORD PTR [esi],1\n"
        "shld DWORD PTR [eax],ecx,0x3\n"
        "shld DWORD PTR [ebx-0x80],ebp,0x3\n"
        "shld DWORD PTR [rip+0x0],esi,0x3\n"
        "shlx r9,rax,r13\n"
        "shr WORD PTR fs:[rax],0x6\n"
        "shr DWORD PTR fs:[r15+0x0],0x10\n"
        "shr QWORD PTR [r15*4+0x10],1\n"
        "shrd rdi,r15,cl\n"
        "shufps xmm3,XMMWORD PTR [ebx+0x0],0x3\n"
        "shufps xmm2,XMMWORD PTR gs:[rbp+0x12345678],0x3\n"
        "shufps xmm1,XMMWORD PTR fs:0x28,0x3\n"
        "sidt [rsp+0x8]\n"
        "sidt [r12*2+0x1000]\n"
        "sidt [rbx-0x80]\n"
        "sldt WORD PTR [rcx*4+0x1000]\n"
        "sldt r13d\n"
        "sldt WORD PTR [rip+0xfffffffffffffff8]\n"
        "smsw WORD PTR [rip+0x100]\n"
        "smsw r14\n"
        "smsw WORD PTR [rdx-0x80]\n"
        "sqrtsd xmm6,QWORD PTR [rip+0x0]\n"
        "sqrtsd xmm5,QWORD PTR gs:[r9+0x7f]\n"
        "sqrtsd xmm11,QWORD PTR [r9-0x80]\n"
        "sti\n"
        "stmxcsr DWORD PTR [rbp+rcx*8+0x0]\n"
        "stmxcsr DWORD PTR [esi]\n"
        "stmxcsr DWORD PTR fs:[rax+0x8]\n"
        "stos BYTE PTR es:[rdi],al\n"
        "str r15d\n"
        "str WORD PTR [r14+rax*8+0x80]\n"
        "str WORD PTR [r13*2+0x10]\n"
        "sub WORD PTR fs:[r8+0x0],r15w\n"
        "sub BYTE PTR [eax],0xff\n"
        "sub QWORD PTR fs:[rbp+0x0],r8\n"
        "subpd xmm5,XMMWORD PTR [rsp+rax*8+0x0]\n"
        "subpd xmm2,XMMWORD PTR [rsp]\n"
        "subpd xmm4,XMMWORD PTR fs:[rsp+0x100]\n"
        "swapgs\n"
        "sysenter\n"
        "sysretq\n"
        "test WORD PTR [rdi+rax*8+0x100],0x7fff\n"
        "test DWORD PTR ds:0x0,0x80\n"
        "test WORD PTR [r12+r13*4-0x80],bp\n"
        "tzcnt ax,WORD PTR [rax+r15*2]\n"
        "tzcnt r8w,WORD PTR [rsi+rdi*2+0x7f]\n"
        "tzcnt r8d,DWORD PTR [r15*8+0x10]\n"
        "ucomisd xmm12,QWORD PTR gs:[r12+0x0]\n"
        "ucomisd xmm1,QWORD PTR [r15*8+0x0]\n"
        "ucomisd xmm1,QWORD PTR [eax]\n"
        "unpcklps xmm14,XMMWORD PTR [rax-0x80]\n"
        "unpcklps xmm2,XMMWORD PTR [rip+0x0]\n"
        "unpcklps xmm12,XMMWORD PTR [r15+r15*8-0x80]\n"
        "vaddpd ymm3,ymm15,ymm6\n"
        "vaddpd ymm20{k6},ymm6,YMMWORD PTR [r8+0x1000]\n"
        "vaddpd zmm11{k3},zmm6,ZMMWORD PTR [r14+r9*8-0x40]\n"
        "vaddps zmm3,zmm2,DWORD BCST [rax]\n"
        "vaddps xmm13,xmm2,XMMWORD PTR [rcx+rcx*8-0x80]\n"
        "vaddps xmm2,xmm4,XMMWORD PTR [r12+r11*8]\n"
        "vaddsd xmm3,xmm1,QWORD PTR [rdi+r8*8+0x80]\n"
        "vaddsd xmm14{k5},xmm18,xmm2\n"
        "vaddsd xmm5{k4},xmm26,xmm26\n"
        "vaddss xmm12,xmm7,DWORD PTR [rsi+0x80]\n"
        "vaddss xmm10,xmm5,xmm21{rd-sae}\n"
        "vaddss xmm1,xmm21,DWORD PTR [rip+0x100]\n"
        "vaesdec ymm23,ymm30,YMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vaesdec xmm12,xmm1,XMMWORD PTR [rdi+0x12345678]\n"
        "vaesdec xmm1,xmm8,XMMWORD PTR [rip+0x100]\n"
        "vaesdeclast zmm0,zmm9,ZMMWORD PTR [r12+r9*2-0x80]\n"
        "vaesdeclast xmm13,xmm1,XMMWORD PTR [rax+0x8]\n"
        "vaesdeclast ymm12,ymm11,YMMWORD PTR [rcx-0x80]\n"
        "vaesenc ymm10,ymm1,ymm8\n"
        "vaesenc xmm8,xmm9,XMMWORD PTR [r15*4+0x10]\n"
        "vaesenc zmm17,zmm8,ZMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vaesenclast zmm26,zmm30,ZMMWORD PTR [rax+rbx*8]\n"
        "vaesenclast xmm15,xmm1,XMMWORD PTR [rbp*2+0x10]\n"
        "vaesenclast xmm2,xmm27,XMMWORD PTR [r8-0x40]\n"
        "vaesimc xmm11,xmm1\n"
        "vaesimc xmm3,XMMWORD PTR [rdi*8+0x1000]\n"
        "vaesimc xmm15,XMMWORD PTR [r9+0x100]\n"
        "vaeskeygenassist xmm11,xmm8,0xf8\n"
        "vaeskeygenassist xmm10,XMMWORD PTR [r8*4+0x10],0xa9\n"
        "vaeskeygenassist xmm7,XMMWORD PTR [rdi-0x80],0x36\n"
        "vandnpd ymm12,ymm11,ymm1\n"
        "vandnpd xmm30{k3},xmm15,xmm22\n"
        "vandnpd ymm28{k2}{z},ymm30,ymm13\n"
        "vandnps xmm31{k1}{z},xmm12,xmm18\n"
        "vandnps zmm17{k5},zmm15,zmm20\n"
        "vandnps xmm6,xmm6,XMMWORD PTR [r14*2+0x1000]\n"
        "vandpd zmm14{k2},zmm30,QWORD BCST [rip+0xfffffffffffffff8]\n"
        "vandpd xmm4,xmm11,xmm13\n"
        "vandpd xmm0,xmm0,XMMWORD PTR [rdx+r11*4+0x8]\n"
        "vandps xmm7,xmm3,XMMWORD PTR [r9*2+0x10]\n"
        "vandps ymm2,ymm4,YMMWORD PTR [rcx+r10*4+0x100]\n"
        "vandps xmm26{k4},xmm21,DWORD BCST [r11*2+0x10]\n"
        "vblendpd xmm6,xmm4,XMMWORD PTR [rdi+r11*8+0x40],0x6e\n"
        "vblendpd xmm2,xmm10,xmm3,0x94\n"
        "vblendpd ymm7,ymm12,YMMWORD PTR [rcx+0x12345678],0xaa\n"
        "vblendps xmm3,xmm9,XMMWORD PTR [r9],0x17\n"
        "vblendps xmm9,xmm11,XMMWORD PTR [rip+0xfffffffffffffff8],0xd8\n"
        "vblendps ymm4,ymm15,YMMWORD PTR [rbp+0x0],0x52\n"
        "vblendvpd ymm8,ymm1,ymm11,ymm7\n"
        "vblendvpd ymm9,ymm9,YMMWORD PTR [rip+0xfffffffffffffff8],ymm5\n"
        "vblendvpd xmm3,xmm7,XMMWORD PTR [rsi-0x1000],xmm6\n"
        "vblendvps xmm2,xmm5,XMMWORD PTR [rbx+0x8],xmm15\n"
        "vblendvps ymm8,ymm2,YMMWORD PTR [r14+0x8],ymm10\n"
        "vblendvps xmm9,xmm15,xmm8,xmm7\n"
        "vbroadcastsd ymm11,QWORD PTR [r13+rcx*2-0x1000]\n"
        "vbroadcastsd ymm1{k1},QWORD PTR [rdx+0x8]\n"
        "vbroadcastsd ymm6,QWORD PTR [r13*2+0x10]\n"
        "vbroadcastss ymm5,DWORD PTR [rdi*4+0x1000]\n"
        "vbroadcastss zmm17{k2}{z},xmm4\n"
        "vbroadcastss ymm3,DWORD PTR [r15+0x1000]\n"
        "vcmpeq_uqpd xmm14,xmm4,xmm5\n"
        "vcmpeq_uqps xmm14,xmm0,XMMWORD PTR [rsi+r12*4+0x58]\n"
        "vcmpeq_uqss xmm6,xmm13,DWORD PTR [rip+0x0]\n"
        "vcmpeq_uspd ymm8,ymm8,ymm13\n"
        "vcmpeq_usps xmm12,xmm3,xmm2\n"
        "vcmpeqpd xmm10,xmm6,xmm11\n"
        "vcmpeqps xmm1,xmm3,xmm15\n"
        "vcmpeqss xmm14,xmm0,xmm15\n"
        "vcmpfalse_ospd ymm0,ymm4,ymm11\n"
        "vcmpfalse_ospd ymm10,ymm3,YMMWORD PTR [rdx]\n"
        "vcmpfalseps ymm10,ymm6,YMMWORD PTR [rdx+rdi*2]\n"
        "vcmpfalseps xmm7,xmm12,xmm9\n"
        "vcmpge_oqps ymm13,ymm5,ymm5\n"
        "vcmpgepd ymm8,ymm11,ymm15\n"
        "vcmpgeps xmm4,xmm15,xmm6\n"
        "vcmpgt_oqpd ymm6,ymm7,ymm1\n"
        "vcmpgt_oqps ymm0,ymm1,ymm0\n"
        "vcmpgtpd ymm12,ymm14,YMMWORD PTR [rsi+r15*4-0x15]\n"
        "vcmpgtps xmm4,xmm11,xmm6\n"
        "vcmple_oqpd xmm5,xmm10,xmm12\n"
        "vcmple_oqps ymm2,ymm3,ymm15\n"
        "vcmplepd xmm11,xmm1,xmm7\n"
        "vcmpleps k1,xmm15,xmm1\n"
        "vcmpless xmm11,xmm7,xmm6\n"
        "vcmplt_oqpd xmm14,xmm6,xmm2\n"
        "vcmplt_oqps ymm7,ymm9,ymm5\n"
        "vcmpltpd xmm15,xmm3,xmm0\n"
        "vcmpltps ymm13,ymm4,ymm2\n"
        "vcmpltss xmm3,xmm0,xmm7\n"
        "vcmpneq_oqpd xmm1,xmm15,xmm0\n"
        "vcmpneq_oqps ymm15,ymm3,ymm4\n"
        "vcmpneq_ospd xmm6,xmm1,xmm7\n"
        "vcmpneq_usss xmm15,xmm0,DWORD PTR [rax*4+0x10]\n"
        "vcmpneqps k4,xmm17,xmm23\n"
        "vcmpneqsd k7,xmm8,QWORD PTR [r9*4+0x10]\n"
        "vcmpneqss xmm1,xmm3,xmm3\n"
        "vcmpneqss xmm9,xmm8,DWORD PTR [rdx-0x8]\n"
        "vcmpneqss xmm0,xmm14,DWORD PTR [rax]\n"
        "vcmpnge_uqpd xmm9,xmm13,xmm3\n"
        "vcmpnge_uqps ymm3,ymm14,ymm10\n"
        "vcmpngesd k2,xmm29,xmm19\n"
        "vcmpngt_uqpd ymm13,ymm1,YMMWORD PTR [r9]\n"
        "vcmpngt_uqpd ymm3,ymm7,ymm2\n"
        "vcmpngtpd xmm13,xmm2,xmm6\n"
        "vcmpngtps ymm8,ymm6,ymm2\n"
        "vcmpnle_uqpd ymm2,ymm13,ymm15\n"
        "vcmpnle_uqss k1,xmm17,DWORD PTR [rip+0x0]\n"
        "vcmpnlepd ymm0,ymm11,ymm11\n"
        "vcmpnleps ymm11,ymm15,ymm10\n"
        "vcmpnless xmm14,xmm5,xmm5\n"
        "vcmpnlt_uqpd xmm4,xmm3,xmm9\n"
        "vcmpnlt_uqpd xmm7,xmm2,XMMWORD PTR [r15]\n"
        "vcmpnlt_uqps ymm3,ymm3,ymm8\n"
        "vcmpnltpd xmm2,xmm0,xmm7\n"
        "vcmpnltps xmm12,xmm9,xmm4\n"
        "vcmpnltss xmm6,xmm4,DWORD PTR [r12+0x8]\n"
        "vcmpnltss xmm7,xmm2,xmm12\n"
        "vcmpord_spd ymm0,ymm1,ymm4\n"
        "vcmpord_sps xmm11,xmm0,xmm7\n"
        "vcmpordps xmm0,xmm9,XMMWORD PTR [rdx]\n"
        "vcmpordss xmm12,xmm14,xmm14\n"
        "vcmpordss k5,xmm20,DWORD PTR [rax*8+0x10]\n"
        "vcmpordss k0,xmm9,DWORD PTR [rip+0xfffffffffffffff8]\n"
        "vcmppd k4,zmm5,ZMMWORD PTR [r15+r8*1-0x40],0xff\n"
        "vcmppd k5,zmm27,ZMMWORD PTR [rip+0xfffffffffffffff8],0x3d\n"
        "vcmppd k7,xmm2,XMMWORD PTR [rcx+0x100],0x3f\n"
        "vcmpps k7,zmm3,zmm20,0x51\n"
        "vcmpps k5,ymm16,YMMWORD PTR [r8+0x40],0xdb\n"
        "vcmpps xmm9,xmm15,XMMWORD PTR [rdx-0x40],0x2f\n"
        "vcmpsd k0,xmm3,QWORD PTR [r14+rcx*8+0x8],0x75\n"
        "vcmpsd xmm9,xmm1,QWORD PTR [r14+r8*8+0x100],0xd3\n"
        "vcmpsd k4,xmm7,xmm9,0x64\n"
        "vcmpss xmm5,xmm7,DWORD PTR [rsi+r15*1-0x80],0xd0\n"
        "vcmpss k3,xmm2,DWORD PTR [r12+r13*4+0x100],0x4b\n"
        "vcmpss k7,xmm1,xmm25,0xa5\n"
        "vcmptrue_uspd ymm0,ymm10,ymm9\n"
        "vcmptrue_usps xmm11,xmm12,xmm13\n"
        "vcmptruepd ymm10,ymm13,ymm7\n"
        "vcmpunord_spd ymm12,ymm4,YMMWORD PTR [rcx]\n"
        "vcmpunord_spd ymm0,ymm9,ymm6\n"
        "vcmpunordps xmm11,xmm3,xmm14\n"
        "vcmpunordsd xmm12,xmm15,xmm8\n"
        "vcmpunordss xmm1,xmm13,DWORD PTR [r15+r9*1]\n"
        "vcmpunordss xmm4,xmm2,xmm4\n"
        "vcomisd xmm11,QWORD PTR [rdi+0x1000]\n"
        "vcomisd xmm4,xmm8\n"
        "vcomisd xmm5,QWORD PTR [r8-0x1000]\n"
        "vcomiss xmm2,xmm0\n"
        "vcomiss xmm2,DWORD PTR [r8+0x40]\n"
        "vcomiss xmm9,xmm18{sae}\n"
        "vcompresspd ymm8,ymm27\n"
        "vcompresspd ZMMWORD PTR [r15*8+0x10],zmm12\n"
        "vcompresspd zmm7{k1},zmm9\n"
        "vcompressps XMMWORD PTR [rip+0x0]{k3},xmm4\n"
        "vcompressps YMMWORD PTR [r9*2+0x10],ymm30\n"
        "vcompressps XMMWORD PTR [rdi*4+0x1000],xmm29\n"
        "vcvtdq2ps xmm3{k4},XMMWORD PTR [rbx*8+0x1000]\n"
        "vcvtdq2ps xmm21,XMMWORD PTR [rax-0x40]\n"
        "vcvtdq2ps xmm10,XMMWORD PTR [rcx+r15*4+0x8]\n"
        "vcvtpd2ps ymm26,zmm26{rz-sae}\n"
        "vcvtpd2ps ymm27{k3},ZMMWORD PTR [rax+0x80]\n"
        "vcvtpd2ps xmm6,YMMWORD PTR [rbp+rsi*1-0x1000]\n"
        "vcvtph2ps zmm8{k2}{z},YMMWORD PTR [r11]\n"
        "vcvtph2ps ymm15,XMMWORD PTR [r12]\n"
        "vcvtph2ps zmm19,YMMWORD PTR [r8+0x1000]\n"
        "vcvtps2dq xmm9,XMMWORD PTR [rip+0x0]\n"
        "vcvtps2dq zmm2,DWORD BCST [rax*4+0x10]\n"
        "vcvtps2dq xmm0,XMMWORD PTR [rdi*4+0x1000]\n"
        "vcvtps2pd xmm14,QWORD PTR [rip+0x100]\n"
        "vcvtps2pd xmm2,QWORD PTR [r9]\n"
        "vcvtps2pd ymm11,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vcvtps2ph xmm2,ymm10,0x0\n"
        "vcvtps2ph XMMWORD PTR [rcx-0x80],ymm1,0x0\n"
        "vcvtps2ph XMMWORD PTR [rbp+rbx*1-0xed],ymm12,0x0\n"
        "vcvtsd2si edi,QWORD PTR [r9*2+0x10]\n"
        "vcvtsd2si rbx,QWORD PTR [r10+0x40]\n"
        "vcvtsd2si r8,QWORD PTR [rax+r12*2-0x40]\n"
        "vcvtsd2ss xmm15,xmm0,QWORD PTR [r14]\n"
        "vcvtsd2ss xmm8,xmm14,QWORD PTR [rdi+0x80]\n"
        "vcvtsd2ss xmm12,xmm3,QWORD PTR [rbx+rdi*8]\n"
        "vcvtsi2sd xmm13,xmm11,DWORD PTR [rip+0xfffffffffffffff8]\n"
        "vcvtsi2sd xmm15,xmm3,DWORD PTR [rax+r15*4+0x40]\n"
        "vcvtsi2sd xmm11,xmm8,rsi\n"
        "vcvtsi2ss xmm7,xmm10,DWORD PTR [rip+0xfffffffffffffff8]\n"
        "vcvtsi2ss xmm15,xmm4,DWORD PTR [rdx+rdx*1+0x80]\n"
        "vcvtsi2ss xmm2,xmm2,DWORD PTR [rbp*2+0x1000]\n"
        "vcvtss2sd xmm13,xmm13,DWORD PTR [rsi+0x100]\n"
        "vcvtss2sd xmm0,xmm21,DWORD PTR [rdx+r15*8]\n"
        "vcvtss2sd xmm25,xmm16,DWORD PTR [r12*4+0x1000]\n"
        "vcvtss2si rbx,xmm12\n"
        "vcvtss2si r15,DWORD PTR [rbx*4+0x10]\n"
        "vcvtss2si ebp,DWORD PTR [rip+0xfffffffffffffff8]\n"
        "vcvttps2dq xmm13,XMMWORD PTR [rbp+rax*8+0x1000]\n"
        "vcvttps2dq ymm14{k2}{z},ymm28\n"
        "vcvttps2dq ymm25{k2}{z},YMMWORD PTR [r12*8+0x10]\n"
        "vcvttsd2si edx,QWORD PTR [r13+r9*2-0x1000]\n"
        "vcvttsd2si rbp,QWORD PTR [r12+0x1000]\n"
        "vcvttsd2si r14,QWORD PTR [rax+rax*4+0x40]\n"
        "vcvttss2si r9,xmm6\n"
        "vcvttss2si r14,DWORD PTR [rcx+rdi*1+0x80]\n"
        "vcvttss2si r14d,DWORD PTR [rip+0x100]\n"
        "vdivpd xmm14,xmm12,xmm15\n"
        "vdivpd ymm15,ymm3,YMMWORD PTR [rsi+r8*4-0x80]\n"
        "vdivpd zmm6,zmm31,zmm15{rz-sae}\n"
        "vdivps ymm10,ymm13,YMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vdivps zmm10{k6}{z},zmm14,zmm29{rd-sae}\n"
        "vdivps zmm4{k3},zmm1,ZMMWORD PTR [r10+0x80]\n"
        "vdivsd xmm18{k4}{z},xmm24,QWORD PTR [r9-0x80]\n"
        "vdivsd xmm28{k7}{z},xmm20,QWORD PTR [rip+0x0]\n"
        "vdivsd xmm8,xmm13,QWORD PTR [r14]\n"
        "vdivss xmm2,xmm5,DWORD PTR [rip+0x0]\n"
        "vdivss xmm27,xmm22,DWORD PTR [rbx+r13*4+0x100]\n"
        "vdivss xmm8{k7}{z},xmm29,xmm9{rn-sae}\n"
        "vdppd xmm0,xmm5,xmm12,0xb0\n"
        "vdppd xmm3,xmm0,XMMWORD PTR [r14+rbp*1-0x40],0x5d\n"
        "vdppd xmm1,xmm10,XMMWORD PTR [r10*8+0x10],0xbe\n"
        "vdpps ymm15,ymm2,YMMWORD PTR [rsi+r9*4],0x6e\n"
        "vdpps ymm0,ymm2,YMMWORD PTR [rcx*8+0x10],0xf7\n"
        "vdpps xmm6,xmm10,xmm1,0x69\n"
        "verr WORD PTR [rcx+r8*4+0x100]\n"
        "verr r9d\n"
        "verr WORD PTR [r9-0x40]\n"
        "verw WORD PTR [r14*8+0x10]\n"
        "verw WORD PTR [rbp+0x0]\n"
        "verw WORD PTR [r10+r8*1-0x1000]\n"
        "vexpandpd zmm4,ZMMWORD PTR [rdi*8+0x1000]\n"
        "vexpandpd zmm28,zmm25\n"
        "vexpandpd xmm4{k1}{z},XMMWORD PTR [rsi+r8*8+0x80]\n"
        "vexpandps ymm22,YMMWORD PTR [r14*2+0x10]\n"
        "vexpandps ymm20{k5},YMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vexpandps xmm20{k7},xmm23\n"
        "vextractf128 xmm6,ymm14,0x87\n"
        "vextractf128 XMMWORD PTR [r8+rax*8+0x12345678],ymm12,0x43\n"
        "vextractf128 XMMWORD PTR [r9+0x40],ymm9,0xe6\n"
        "vextractf32x4 xmm13{k3}{z},ymm26,0xd1\n"
        "vextractf32x4 XMMWORD PTR [rip+0xfffffffffffffff8]{k7},zmm30,0x59\n"
        "vextractf32x4 XMMWORD PTR [rip+0x100]{k5},ymm28,0x9c\n"
        "vextractf32x8 YMMWORD PTR [r9*8+0x10],zmm0,0x53\n"
        "vextractf32x8 ymm22,zmm2,0xd9\n"
        "vextractf32x8 ymm0{k4}{z},zmm1,0xe1\n"
        "vextractf64x2 XMMWORD PTR [rsi*8+0x10]{k1},zmm5,0x72\n"
        "vextractf64x2 XMMWORD PTR [r8+0x100],ymm5,0xff\n"
        "vextractf64x2 xmm3{k6}{z},zmm2,0xa3\n"
        "vextractf64x4 YMMWORD PTR [rdi+rdx*1+0x100],zmm9,0x5c\n"
        "vextractf64x4 YMMWORD PTR [rip+0x0],zmm8,0xc6\n"
        "vextractf64x4 ymm8{k2},zmm17,0x10\n"
        "vextracti128 XMMWORD PTR [rsi],ymm2,0x1\n"
        "vextracti128 XMMWORD PTR [r12+rsi*2-0x86],ymm3,0x1\n"
        "vextracti128 XMMWORD PTR [rip+0x100],ymm2,0xfd\n"
        "vextracti32x4 xmm29{k3}{z},ymm22,0x25\n"
        "vextracti32x4 xmm29{k7},ymm23,0xa2\n"
        "vextracti32x4 XMMWORD PTR [rip+0x100]{k7},ymm7,0x46\n"
        "vextracti32x8 ymm27{k2}{z},zmm17,0x45\n"
        "vextracti32x8 YMMWORD PTR [r10*8+0x10],zmm24,0xd6\n"
        "vextracti32x8 YMMWORD PTR [r15+rcx*4-0x80]{k1},zmm30,0x3c\n"
        "vextracti64x2 XMMWORD PTR [r12+rbx*1+0x12345678]{k6},ymm27,0x2f\n"
        "vextracti64x2 XMMWORD PTR [rip+0x0]{k4},zmm27,0x36\n"
        "vextracti64x2 xmm10{k5}{z},zmm25,0x77\n"
        "vextracti64x4 ymm10{k1}{z},zmm4,0x87\n"
        "vextracti64x4 ymm6{k6}{z},zmm24,0xba\n"
        "vextracti64x4 YMMWORD PTR [r9*2+0x10],zmm6,0x59\n"
        "vextractps DWORD PTR [r9+rsi*2+0x1000],xmm2,0x27\n"
        "vextractps DWORD PTR [r10*4+0x1000],xmm5,0xfa\n"
        "vextractps DWORD PTR [r10+r11*1-0x40],xmm3,0xfa\n"
        "vfmadd132pd xmm0,xmm0,XMMWORD PTR [rdx+0x8]\n"
        "vfmadd132pd ymm23,ymm5,YMMWORD PTR [r15+rbx*1+0x8]\n"
        "vfmadd132pd xmm0,xmm11,XMMWORD PTR [r12+rsi*1+0x100]\n"
        "vfmadd132ps xmm25{k7}{z},xmm19,xmm17\n"
        "vfmadd132ps xmm23{k4}{z},xmm22,xmm27\n"
        "vfmadd132ps zmm14,zmm27,zmm18{rz-sae}\n"
        "vfmadd132sd xmm21,xmm13,xmm8{ru-sae}\n"
        "vfmadd132sd xmm17{k6},xmm3,QWORD PTR [rdx+0x8]\n"
        "vfmadd132sd xmm11,xmm1,QWORD PTR [rsi+r9*8+0x40]\n"
        "vfmadd132ss xmm19{k2}{z},xmm1,DWORD PTR [r14*8+0x10]\n"
        "vfmadd132ss xmm8,xmm5,DWORD PTR [rdi+0x100]\n"
        "vfmadd132ss xmm0,xmm12,DWORD PTR [rsi+r11*2+0x8]\n"
        "vfmadd213pd xmm8,xmm2,XMMWORD PTR [rip+0x100]\n"
        "vfmadd213pd xmm11,xmm0,xmm2\n"
        "vfmadd213pd ymm13,ymm11,YMMWORD PTR [r13+rbx*4+0x100]\n"
        "vfmadd213ps ymm12,ymm30,DWORD BCST [r14+r15*4+0x8]\n"
        "vfmadd213ps xmm15,xmm13,XMMWORD PTR [rcx*2+0x10]\n"
        "vfmadd213ps ymm6,ymm10,YMMWORD PTR [r12+r12*2+0x40]\n"
        "vfmadd213sd xmm0{k7},xmm5,QWORD PTR [rax+rax*1+0x40]\n"
        "vfmadd213sd xmm21,xmm8,xmm16{rn-sae}\n"
        "vfmadd213sd xmm19{k5}{z},xmm31,QWORD PTR [rsi+0x40]\n"
        "vfmadd213ss xmm6,xmm12,DWORD PTR [rbx+r15*2-0x80]\n"
        "vfmadd213ss xmm12{k5}{z},xmm2,DWORD PTR [r9*4+0x10]\n"
        "vfmadd213ss xmm13,xmm15,DWORD PTR [rip+0xfffffffffffffff8]\n"
        "vfmadd231pd xmm20,xmm23,XMMWORD PTR [r12+0x40]\n"
        "vfmadd231pd xmm10{k7}{z},xmm22,XMMWORD PTR [rip+0x0]\n"
        "vfmadd231pd ymm1{k4},ymm3,ymm6\n"
        "vfmadd231ps ymm25{k7}{z},ymm26,ymm14\n"
        "vfmadd231ps zmm9,zmm1,ZMMWORD PTR [r14-0x1000]\n"
        "vfmadd231ps ymm3,ymm8,YMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vfmadd231sd xmm7,xmm0,QWORD PTR [r14+r11*2-0x80]\n"
        "vfmadd231sd xmm1{k5}{z},xmm28,xmm31\n"
        "vfmadd231sd xmm18,xmm24,xmm13{rn-sae}\n"
        "vfmadd231ss xmm18{k4},xmm25,DWORD PTR [rip+0x100]\n"
        "vfmadd231ss xmm19,xmm25,xmm19\n"
        "vfmadd231ss xmm4{k3},xmm12,DWORD PTR [rbp+rdx*1-0x80]\n"
        "vfmaddsub132pd zmm30{k6},zmm23,zmm31\n"
        "vfmaddsub132pd xmm19,xmm0,QWORD BCST [rbx+rax*8+0x8]\n"
        "vfmaddsub132pd ymm1{k6}{z},ymm15,QWORD BCST [r11+0x1000]\n"
        "vfmaddsub132ps xmm27{k6},xmm29,XMMWORD PTR [rdx+rdi*8+0x12345678]\n"
        "vfmaddsub132ps ymm9,ymm13,YMMWORD PTR [rcx+0x40]\n"
        "vfmaddsub132ps ymm13,ymm0,YMMWORD PTR [r10-0x80]\n"
        "vfmaddsub213pd ymm4,ymm8,ymm2\n"
        "vfmaddsub213pd ymm7,ymm7,YMMWORD PTR [rip+0x100]\n"
        "vfmaddsub213pd ymm6{k2}{z},ymm7,YMMWORD PTR [rip+0x100]\n"
        "vfmaddsub213ps ymm14{k4},ymm0,ymm29\n"
        "vfmaddsub213ps zmm25,zmm23,ZMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vfmaddsub213ps xmm6{k1}{z},xmm23,DWORD BCST [r11]\n"
        "vfmaddsub231pd xmm0{k1}{z},xmm11,xmm0\n"
        "vfmaddsub231pd xmm4,xmm8,XMMWORD PTR [r10-0x40]\n"
        "vfmaddsub231pd ymm3,ymm6,YMMWORD PTR [r12]\n"
        "vfmaddsub231ps xmm0,xmm7,xmm7\n"
        "vfmaddsub231ps xmm8{k7}{z},xmm1,XMMWORD PTR [r8*8+0x10]\n"
        "vfmaddsub231ps ymm3{k7}{z},ymm0,YMMWORD PTR [r10+r10*4-0x80]\n"
        "vfmsub132pd ymm12{k7}{z},ymm25,YMMWORD PTR [r12-0x80]\n"
        "vfmsub132pd xmm6,xmm7,xmm5\n"
        "vfmsub132pd ymm28{k7},ymm18,ymm24\n"
        "vfmsub132ps ymm30{k5},ymm8,YMMWORD PTR [rsi+0x40]\n"
        "vfmsub132ps xmm19,xmm9,DWORD BCST [r13-0x80]\n"
        "vfmsub132ps zmm22{k5},zmm26,zmm0\n"
        "vfmsub132sd xmm29,xmm1,QWORD PTR [rip+0xfffffffffffffff8]\n"
        "vfmsub132sd xmm5{k1}{z},xmm24,xmm4\n"
        "vfmsub132sd xmm2{k4},xmm23,xmm1{ru-sae}\n"
        "vfmsub132ss xmm13,xmm11,DWORD PTR [rcx*2+0x1000]\n"
        "vfmsub132ss xmm31{k3}{z},xmm27,xmm9\n"
        "vfmsub132ss xmm11,xmm11,DWORD PTR [rdi+r9*2+0x80]\n"
        "vfmsub213pd zmm28{k5}{z},zmm10,ZMMWORD PTR [r13*4+0x1000]\n"
        "vfmsub213pd zmm17{k4}{z},zmm8,zmm6{rd-sae}\n"
        "vfmsub213pd ymm8,ymm11,YMMWORD PTR [rbx*2+0x1000]\n"
        "vfmsub213ps xmm7,xmm4,DWORD BCST [r13+0x1000]\n"
        "vfmsub213ps ymm27{k7},ymm25,YMMWORD PTR [rsi*2+0x10]\n"
        "vfmsub213ps zmm25,zmm17,ZMMWORD PTR [r12+0x12345678]\n"
        "vfmsub213sd xmm15,xmm19,QWORD PTR [rip+0xfffffffffffffff8]\n"
        "vfmsub213sd xmm8,xmm2,QWORD PTR [r9+r12*1]\n"
        "vfmsub213sd xmm12,xmm2,QWORD PTR [r10+rbp*2+0x8]\n"
        "vfmsub213ss xmm9{k6}{z},xmm26,DWORD PTR [rax*2+0x10]\n"
        "vfmsub213ss xmm14,xmm3,xmm7\n"
        "vfmsub213ss xmm7,xmm3,DWORD PTR [r14+0x8]\n"
        "vfmsub231pd ymm21,ymm10,YMMWORD PTR [rip+0x100]\n"
        "vfmsub231pd ymm13,ymm1,YMMWORD PTR [r15+r14*1+0x12345678]\n"
        "vfmsub231pd ymm10{k7}{z},ymm12,YMMWORD PTR [rsi+r9*4]\n"
        "vfmsub231ps xmm27{k7},xmm16,xmm16\n"
        "vfmsub231ps ymm0,ymm7,YMMWORD PTR [rax]\n"
        "vfmsub231ps ymm4,ymm0,YMMWORD PTR [r15-0x80]\n"
        "vfmsub231sd xmm28{k3}{z},xmm21,QWORD PTR [rdx-0x40]\n"
        "vfmsub231sd xmm2,xmm1,QWORD PTR [rax*2+0x1000]\n"
        "vfmsub231sd xmm21{k4}{z},xmm1,QWORD PTR [r14-0x1000]\n"
        "vfmsub231ss xmm0{k2}{z},xmm8,xmm22\n"
        "vfmsub231ss xmm25{k4}{z},xmm23,xmm24\n"
        "vfmsub231ss xmm5,xmm7,DWORD PTR [r15+0x100]\n"
        "vfmsubadd132pd zmm22{k3}{z},zmm9,ZMMWORD PTR [rip+0x0]\n"
        "vfmsubadd132pd zmm20{k4},zmm18,zmm18\n"
        "vfmsubadd132pd xmm16{k5},xmm19,xmm10\n"
        "vfmsubadd132ps xmm3,xmm11,XMMWORD PTR [r14+r11*4-0x1000]\n"
        "vfmsubadd132ps ymm20{k6},ymm13,ymm26\n"
        "vfmsubadd132ps xmm11,xmm5,xmm8\n"
        "vfmsubadd213pd ymm0,ymm2,YMMWORD PTR [rcx+r9*1+0x12345678]\n"
        "vfmsubadd213pd zmm26,zmm28,ZMMWORD PTR [rbx-0x80]\n"
        "vfmsubadd213pd zmm14{k3}{z},zmm15,zmm26{ru-sae}\n"
        "vfmsubadd213ps ymm3,ymm6,YMMWORD PTR [rbx*2+0x10]\n"
        "vfmsubadd213ps xmm5,xmm12,XMMWORD PTR [r13+r15*8+0x100]\n"
        "vfmsubadd213ps zmm24{k1}{z},zmm20,ZMMWORD PTR [rbp*2+0x1000]\n"
        "vfmsubadd231pd ymm16,ymm16,QWORD BCST [rdi+r10*8]\n"
        "vfmsubadd231pd xmm3,xmm4,xmm14\n"
        "vfmsubadd231pd xmm23,xmm13,QWORD BCST [r12*8+0x1000]\n"
        "vfmsubadd231ps zmm11,zmm25,zmm17{rz-sae}\n"
        "vfmsubadd231ps ymm9,ymm3,YMMWORD PTR [rbp+0x0]\n"
        "vfmsubadd231ps xmm9,xmm3,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vfnmadd132pd xmm10,xmm8,XMMWORD PTR [r15-0x40]\n"
        "vfnmadd132pd ymm5,ymm14,YMMWORD PTR [r14+rcx*2+0x80]\n"
        "vfnmadd132pd xmm23{k3}{z},xmm21,XMMWORD PTR [r8*2+0x10]\n"
        "vfnmadd132ps zmm30,zmm31,ZMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vfnmadd132ps xmm17,xmm30,XMMWORD PTR [r8*2+0x1000]\n"
        "vfnmadd132ps ymm7,ymm11,YMMWORD PTR [rdi*4+0x1000]\n"
        "vfnmadd132sd xmm6{k4}{z},xmm17,xmm8\n"
        "vfnmadd132sd xmm7,xmm1,QWORD PTR [rax-0x1000]\n"
        "vfnmadd132sd xmm26{k5},xmm25,xmm27{ru-sae}\n"
        "vfnmadd132ss xmm16{k5},xmm31,DWORD PTR [rsi+r8*2+0x1000]\n"
        "vfnmadd132ss xmm9{k2},xmm12,xmm31\n"
        "vfnmadd132ss xmm13,xmm5,DWORD PTR [rip+0xfffffffffffffff8]\n"
        "vfnmadd213pd ymm4{k3},ymm12,ymm11\n"
        "vfnmadd213pd ymm9{k6}{z},ymm26,ymm9\n"
        "vfnmadd213pd xmm2,xmm15,XMMWORD PTR [rbp*2+0x10]\n"
        "vfnmadd213ps xmm5,xmm8,XMMWORD PTR [rbp+0x8]\n"
        "vfnmadd213ps ymm11,ymm0,YMMWORD PTR [rbx-0x80]\n"
        "vfnmadd213ps xmm3,xmm1,XMMWORD PTR [rsi+r15*1+0x1000]\n"
        "vfnmadd213sd xmm0{k6}{z},xmm11,QWORD PTR [rsi+0x100]\n"
        "vfnmadd213sd xmm30{k2}{z},xmm13,xmm6{rz-sae}\n"
        "vfnmadd213sd xmm6,xmm8,QWORD PTR [r14+rbp*8+0x1000]\n"
        "vfnmadd213ss xmm16{k4}{z},xmm9,DWORD PTR [r9*8+0x1000]\n"
        "vfnmadd213ss xmm9{k2},xmm9,xmm28{rn-sae}\n"
        "vfnmadd213ss xmm3,xmm11,DWORD PTR [r12+r8*4+0x100]\n"
        "vfnmadd231pd xmm6,xmm6,XMMWORD PTR [r14]\n"
        "vfnmadd231pd ymm4,ymm6,YMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vfnmadd231pd xmm22{k6}{z},xmm6,xmm8\n"
        "vfnmadd231ps ymm0{k1}{z},ymm5,ymm5\n"
        "vfnmadd231ps xmm13,xmm4,XMMWORD PTR [rdi*4+0x10]\n"
        "vfnmadd231ps ymm9,ymm3,YMMWORD PTR [r9+0x8]\n"
        "vfnmadd231sd xmm31{k6},xmm0,xmm9{rd-sae}\n"
        "vfnmadd231sd xmm10,xmm3,QWORD PTR [r8*8+0x10]\n"
        "vfnmadd231sd xmm0,xmm10,QWORD PTR [r15-0x80]\n"
        "vfnmadd231ss xmm10,xmm0,DWORD PTR [rip+0x0]\n"
        "vfnmadd231ss xmm10{k3},xmm3,DWORD PTR [r10+rsi*8-0x40]\n"
        "vfnmadd231ss xmm4,xmm0,DWORD PTR [rdx+rbp*1-0x80]\n"
        "vfnmsub132pd xmm8,xmm11,XMMWORD PTR [r11+rax*2-0x40]\n"
        "vfnmsub132pd zmm11{k6}{z},zmm30,ZMMWORD PTR [r9+0x100]\n"
        "vfnmsub132pd xmm15,xmm4,XMMWORD PTR [r10+0x12345678]\n"
        "vfnmsub132ps zmm9{k6}{z},zmm18,zmm14{rz-sae}\n"
        "vfnmsub132ps xmm9{k4},xmm31,xmm29\n"
        "vfnmsub132ps ymm1,ymm5,YMMWORD PTR [rip+0x0]\n"
        "vfnmsub132sd xmm8,xmm13,QWORD PTR [r8+rcx*4-0x1000]\n"
        "vfnmsub132sd xmm31{k4}{z},xmm5,xmm29\n"
        "vfnmsub132sd xmm14,xmm1,QWORD PTR [r8]\n"
        "vfnmsub132ss xmm8,xmm22,DWORD PTR [r12+r11*8+0x40]\n"
        "vfnmsub132ss xmm27,xmm23,DWORD PTR [r10+0x1000]\n"
        "vfnmsub132ss xmm9,xmm13,DWORD PTR [rbx+rcx*2-0x80]\n"
        "vfnmsub213pd zmm0{k4},zmm29,zmm10\n"
        "vfnmsub213pd zmm24{k5},zmm22,QWORD BCST [rbx*2+0x1000]\n"
        "vfnmsub213pd zmm5,zmm5,ZMMWORD PTR [r13*4+0x10]\n"
        "vfnmsub213ps xmm14,xmm15,XMMWORD PTR [rdi*4+0x10]\n"
        "vfnmsub213ps xmm9,xmm15,XMMWORD PTR [rax+r10*4+0x8]\n"
        "vfnmsub213ps xmm6{k3}{z},xmm19,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vfnmsub213sd xmm25{k1}{z},xmm13,QWORD PTR [rax*8+0x1000]\n"
        "vfnmsub213sd xmm17{k3}{z},xmm27,xmm2{ru-sae}\n"
        "vfnmsub213sd xmm8,xmm6,QWORD PTR [r12+r8*2-0x1000]\n"
        "vfnmsub213ss xmm6,xmm11,DWORD PTR [r8+r11*8-0x40]\n"
        "vfnmsub213ss xmm6{k7},xmm27,DWORD PTR [r12+r13*2+0x100]\n"
        "vfnmsub213ss xmm10{k4},xmm2,DWORD PTR [rax+0x1000]\n"
        "vfnmsub231pd ymm6,ymm2,YMMWORD PTR [r8*8+0x10]\n"
        "vfnmsub231pd zmm24{k5}{z},zmm26,ZMMWORD PTR [rdi*4+0x10]\n"
        "vfnmsub231pd ymm23{k2},ymm1,YMMWORD PTR [rip+0x100]\n"
        "vfnmsub231ps xmm1,xmm15,XMMWORD PTR [r9+0x12345678]\n"
        "vfnmsub231ps ymm18,ymm11,YMMWORD PTR [r13+r9*8+0x0]\n"
        "vfnmsub231ps xmm1,xmm2,XMMWORD PTR [rax*8+0x1000]\n"
        "vfnmsub231sd xmm0,xmm5,QWORD PTR [rax-0x1000]\n"
        "vfnmsub231sd xmm12,xmm23,QWORD PTR [r10+r9*8-0x80]\n"
        "vfnmsub231sd xmm18{k1},xmm27,xmm11{ru-sae}\n"
        "vfnmsub231ss xmm12{k7},xmm19,DWORD PTR [rip+0x100]\n"
        "vfnmsub231ss xmm2,xmm1,xmm1\n"
        "vfnmsub231ss xmm2,xmm2,DWORD PTR [rbx*2+0x10]\n"
        "vgatherdpd xmm15,QWORD PTR [rbx+xmm7*4+0x8],xmm0\n"
        "vgatherdpd ymm5{k6},QWORD PTR [r9+xmm31*8-0x80]\n"
        "vgatherdpd xmm19{k2},QWORD PTR [rdx+xmm14*1+0x12345678]\n"
        "vgatherdps xmm1,DWORD PTR [rbx+xmm10*1],xmm9\n"
        "vgatherdps ymm6{k5},DWORD PTR [r13+ymm25*8+0x100]\n"
        "vgatherdps zmm9{k2},DWORD PTR [rdi+zmm10*4+0x80]\n"
        "vgatherqpd xmm9{k4},QWORD PTR [rsi+xmm3*1+0x40]\n"
        "vgatherqpd ymm2,QWORD PTR [rdi+ymm3*8],ymm4\n"
        "vgatherqpd ymm25{k7},QWORD PTR [rbx+ymm17*2+0x8]\n"
        "vgatherqps ymm6{k5},DWORD PTR [r8+zmm13*8]\n"
        "vgatherqps ymm24{k7},DWORD PTR [rax+zmm10*8-0x40]\n"
        "vgatherqps ymm15{k2},DWORD PTR [r12+zmm9*8-0x1000]\n"
        "vgf2p8affineinvqb ymm21{k7},ymm13,YMMWORD PTR [r12+0x100],0xd9\n"
        "vgf2p8affineinvqb ymm0,ymm26,YMMWORD PTR [r9*2+0x1000],0xd0\n"
        "vgf2p8affineinvqb xmm22,xmm30,XMMWORD PTR [r15+rbp*8-0x1000],0x90\n"
        "vgf2p8affineqb ymm0,ymm13,YMMWORD PTR [rbx*4+0x1000],0xf5\n"
        "vgf2p8affineqb xmm8,xmm10,XMMWORD PTR [r13*8+0x1000],0x23\n"
        "vgf2p8affineqb ymm3,ymm9,YMMWORD PTR [rip+0x0],0x5b\n"
        "vgf2p8mulb ymm3,ymm0,YMMWORD PTR [rdi+rbx*2+0x80]\n"
        "vgf2p8mulb ymm14,ymm30,YMMWORD PTR [rip+0x0]\n"
        "vgf2p8mulb ymm31{k7}{z},ymm20,YMMWORD PTR [rip+0x0]\n"
        "vinsertf128 ymm2,ymm6,xmm8,0x1\n"
        "vinsertf128 ymm2,ymm2,XMMWORD PTR [r8+r11*2+0x1000],0x28\n"
        "vinsertf128 ymm7,ymm0,XMMWORD PTR [rax*4+0x1000],0x23\n"
        "vinsertf32x4 zmm20,zmm10,XMMWORD PTR [r15],0xc7\n"
        "vinsertf32x4 zmm16,zmm25,XMMWORD PTR [r14*2+0x10],0x52\n"
        "vinsertf32x4 ymm7{k4},ymm17,xmm30,0xca\n"
        "vinsertf32x8 zmm5{k5}{z},zmm1,YMMWORD PTR [r9+0x40],0xde\n"
        "vinsertf32x8 zmm11,zmm16,YMMWORD PTR [r12*2+0x1000],0xee\n"
        "vinsertf32x8 zmm11,zmm31,ymm27,0xde\n"
        "vinsertf64x2 zmm20{k3},zmm13,XMMWORD PTR [r14*2+0x1000],0x90\n"
        "vinsertf64x2 ymm10,ymm8,XMMWORD PTR [rdx+rsi*4-0x40],0x9f\n"
        "vinsertf64x2 ymm16,ymm8,XMMWORD PTR [r11],0xc2\n"
        "vinsertf64x4 zmm31{k1}{z},zmm17,YMMWORD PTR [rdi+rdx*1+0x12345678],0x66\n"
        "vinsertf64x4 zmm11,zmm4,YMMWORD PTR [rip+0x100],0x49\n"
        "vinsertf64x4 zmm27{k4}{z},zmm8,ymm23,0x21\n"
        "vinserti128 ymm14,ymm5,XMMWORD PTR [rdx-0x40],0xa0\n"
        "vinserti128 ymm8,ymm7,XMMWORD PTR [rcx+0x12345678],0x7b\n"
        "vinserti128 ymm13,ymm9,XMMWORD PTR [r10*2+0x1000],0x4f\n"
        "vinserti32x4 ymm28,ymm17,XMMWORD PTR [rip+0x0],0x2d\n"
        "vinserti32x4 zmm16{k7}{z},zmm7,xmm15,0xfc\n"
        "vinserti32x4 zmm28,zmm2,XMMWORD PTR [rsi*4+0x1000],0xcf\n"
        "vinserti32x8 zmm21,zmm3,ymm15,0x22\n"
        "vinserti32x8 zmm12{k1}{z},zmm8,YMMWORD PTR [rcx+r8*1-0x40],0x29\n"
        "vinserti32x8 zmm21{k3}{z},zmm22,YMMWORD PTR [rbp-0x40],0xd2\n"
        "vinserti64x2 ymm27{k1}{z},ymm13,XMMWORD PTR [r11*2+0x1000],0x3c\n"
        "vinserti64x2 ymm4,ymm8,XMMWORD PTR [rsi+r9*2+0x12345678],0x7b\n"
        "vinserti64x2 ymm2{k4}{z},ymm0,xmm16,0x46\n"
        "vinserti64x4 zmm15{k1},zmm31,YMMWORD PTR [rbp-0x40],0x6a\n"
        "vinserti64x4 zmm20{k3}{z},zmm24,ymm0,0x93\n"
        "vinserti64x4 zmm20{k7}{z},zmm26,ymm2,0xd5\n"
        "vinsertps xmm12,xmm3,DWORD PTR [rip+0x0],0x3f\n"
        "vinsertps xmm12,xmm12,DWORD PTR [r13*2+0x1000],0x67\n"
        "vinsertps xmm7,xmm4,DWORD PTR [r10+rsi*8],0x1d\n"
        "vmaxpd ymm16{k7},ymm16,QWORD BCST [rbx+0x100]\n"
        "vmaxpd ymm10,ymm4,YMMWORD PTR [rbp+rbp*4-0x1000]\n"
        "vmaxpd zmm17,zmm16,QWORD BCST [r15+rcx*4+0x8]\n"
        "vmaxps ymm7{k1}{z},ymm1,DWORD BCST [r8+rbx*2-0x40]\n"
        "vmaxps ymm11,ymm5,YMMWORD PTR [rbx+0x1000]\n"
        "vmaxps xmm8,xmm10,XMMWORD PTR [r13+rsi*1+0x80]\n"
        "vmaxsd xmm2{k1}{z},xmm28,QWORD PTR [rbx*4+0x1000]\n"
        "vmaxsd xmm28{k5}{z},xmm25,QWORD PTR [rdi-0x40]\n"
        "vmaxsd xmm7,xmm5,xmm14\n"
        "vmaxss xmm0,xmm3,xmm15\n"
        "vmaxss xmm2{k6}{z},xmm21,xmm1\n"
        "vmaxss xmm0,xmm21,xmm14{sae}\n"
        "vminpd xmm13,xmm15,XMMWORD PTR [rcx+0x12345678]\n"
        "vminpd xmm4,xmm3,XMMWORD PTR [rip+0x0]\n"
        "vminpd ymm8,ymm2,YMMWORD PTR [rax+r15*8+0x8]\n"
        "vminps ymm28,ymm27,YMMWORD PTR [r11*4+0x1000]\n"
        "vminps xmm27{k3},xmm19,xmm15\n"
        "vminps ymm17,ymm8,DWORD BCST [rip+0x0]\n"
        "vminsd xmm18{k5},xmm7,QWORD PTR [r8*2+0x10]\n"
        "vminsd xmm16{k3}{z},xmm25,xmm19\n"
        "vminsd xmm4,xmm7,xmm3\n"
        "vminss xmm13{k4}{z},xmm28,xmm12\n"
        "vminss xmm13,xmm4,DWORD PTR [r15]\n"
        "vminss xmm27{k7},xmm18,xmm0{sae}\n"
        "vmovapd XMMWORD PTR [r10+r13*4+0x40]{k5},xmm6\n"
        "vmovapd ZMMWORD PTR [rip+0x0]{k2},zmm12\n"
        "vmovapd XMMWORD PTR [rcx+0x8],xmm0\n"
        "vmovaps ymm3,YMMWORD PTR [rdi]\n"
        "vmovaps xmm13,XMMWORD PTR [rip+0x100]\n"
        "vmovaps ZMMWORD PTR [r10*2+0x1000]{k6},zmm15\n"
        "vmovd xmm4,DWORD PTR [r9+r14*8+0x1000]\n"
        "vmovd ebp,xmm11\n"
        "vmovd DWORD PTR [r15+rdx*8],xmm8\n"
        "vmovdqa xmm2,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vmovdqa ymm3,ymm15\n"
        "vmovdqa xmm8,XMMWORD PTR [rbx*8+0x1000]\n"
        "vmovdqa32 XMMWORD PTR [rdi+0x100],xmm2\n"
        "vmovdqa32 zmm15{k2}{z},ZMMWORD PTR [r11+0x100]\n"
        "vmovdqa32 YMMWORD PTR [r15*2+0x1000],ymm16\n"
        "vmovdqa64 ymm16{k1},YMMWORD PTR [rax-0x80]\n"
        "vmovdqa64 YMMWORD PTR [rdi]{k5},ymm26\n"
        "vmovdqa64 xmm26{k1},xmm0\n"
        "vmovdqu YMMWORD PTR [r15],ymm3\n"
        "vmovdqu YMMWORD PTR [rdx-0x80],ymm1\n"
        "vmovdqu ymm10,YMMWORD PTR [rbx*4+0x1000]\n"
        "vmovdqu16 xmm1{k3}{z},xmm15\n"
        "vmovdqu16 zmm12,ZMMWORD PTR [rbp+0x8]\n"
        "vmovdqu16 xmm13,XMMWORD PTR [r13+0x100]\n"
        "vmovdqu32 zmm20{k1}{z},ZMMWORD PTR [r14*2+0x10]\n"
        "vmovdqu32 zmm1,zmm1\n"
        "vmovdqu32 ymm14,YMMWORD PTR [rcx+0x40]\n"
        "vmovdqu64 zmm18{k3}{z},ZMMWORD PTR [rsi+0x100]\n"
        "vmovdqu64 ZMMWORD PTR [rip+0x100],zmm6\n"
        "vmovdqu64 ZMMWORD PTR [rip+0x0]{k3},zmm27\n"
        "vmovdqu8 YMMWORD PTR [r9]{k2},ymm13\n"
        "vmovdqu8 XMMWORD PTR [rax+0x12345678]{k5},xmm3\n"
        "vmovdqu8 ymm26{k7}{z},ymm5\n"
        "vmovmskpd r9d,xmm12\n"
        "vmovmskps edi,ymm11\n"
        "vmovq xmm10,QWORD PTR [r12+0x100]\n"
        "vmovq xmm28,QWORD PTR [r11-0x40]\n"
        "vmovq QWORD PTR [r14*4+0x1000],xmm6\n"
        "vmovsd QWORD PTR [rbx+rdi*1],xmm10\n"
        "vmovsd xmm12,xmm0,xmm11\n"
        "vmovsd xmm10,QWORD PTR [r8+r8*1+0x8]\n"
        "vmovss xmm1,DWORD PTR [r15+rdx*1+0x3d]\n"
        "vmovss xmm6,DWORD PTR [r8]\n"
        "vmovss xmm6,DWORD PTR [r8+r9*1-0xad]\n"
        "vmovupd YMMWORD PTR [r13*4+0x10]{k1},ymm26\n"
        "vmovupd xmm5{k1},XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vmovupd XMMWORD PTR [r15+0x80],xmm9\n"
        "vmovups xmm4,XMMWORD PTR [rbp*2+0x10]\n"
        "vmovups ymm18{k4}{z},ymm15\n"
        "vmovups YMMWORD PTR [rdi+rbx*2+0xd5],ymm3\n"
        "vmulpd xmm0,xmm11,XMMWORD PTR [r13*8+0x10]\n"
        "vmulpd zmm26,zmm20,ZMMWORD PTR [rax+rbx*2+0x75]\n"
        "vmulpd ymm11,ymm9,YMMWORD PTR [rdi+0x8]\n"
        "vmulps ymm2,ymm4,YMMWORD PTR [rdi+r13*4+0x8]\n"
        "vmulps xmm16{k5},xmm11,XMMWORD PTR [rbp+0x80]\n"
        "vmulps xmm7,xmm21,XMMWORD PTR [rsi+r8*8+0x80]\n"
        "vmulsd xmm17,xmm14,QWORD PTR [r11+rbp*4+0x40]\n"
        "vmulsd xmm13,xmm7,QWORD PTR [r9]\n"
        "vmulsd xmm16{k1},xmm0,QWORD PTR [rcx*4+0x10]\n"
        "vmulss xmm12{k6},xmm3,xmm16\n"
        "vmulss xmm20{k2},xmm5,xmm15{rd-sae}\n"
        "vmulss xmm1,xmm10,DWORD PTR [rdi+0x8]\n"
        "vorpd zmm9,zmm9,ZMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vorpd xmm30{k2}{z},xmm24,xmm16\n"
        "vorpd ymm8,ymm0,YMMWORD PTR [rsi]\n"
        "vorps xmm9,xmm8,XMMWORD PTR [rsi+0x40]\n"
        "vorps ymm5,ymm11,YMMWORD PTR [rdi+0x40]\n"
        "vorps xmm5,xmm0,XMMWORD PTR [r8+rcx*1+0x40]\n"
        "vpackssdw ymm4,ymm2,YMMWORD PTR [r9*4+0x10]\n"
        "vpackssdw ymm16{k2}{z},ymm22,YMMWORD PTR [rcx+r11*8+0x40]\n"
        "vpackssdw xmm23{k2},xmm4,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpacksswb ymm15,ymm3,YMMWORD PTR [rax+r12*4]\n"
        "vpacksswb ymm14{k7},ymm18,ymm30\n"
        "vpacksswb ymm21{k6},ymm8,ymm8\n"
        "vpackuswb xmm0,xmm7,XMMWORD PTR [rdi+0x100]\n"
        "vpackuswb xmm5,xmm18,XMMWORD PTR [r14*2+0x10]\n"
        "vpackuswb zmm27{k5},zmm19,zmm5\n"
        "vpaddb ymm6,ymm4,YMMWORD PTR [r15+rdi*2+0x48]\n"
        "vpaddb ymm8,ymm9,YMMWORD PTR [r9*4+0x10]\n"
        "vpaddb xmm10,xmm3,XMMWORD PTR [r10+0x1000]\n"
        "vpaddd zmm23,zmm0,ZMMWORD PTR [rdx+r9*2+0x7]\n"
        "vpaddd ymm9,ymm5,YMMWORD PTR [rdi*4+0x10]\n"
        "vpaddd zmm7{k2},zmm31,zmm21\n"
        "vpaddq zmm29{k2},zmm23,zmm9\n"
        "vpaddq xmm9,xmm1,XMMWORD PTR [rdi+rbp*4]\n"
        "vpaddq zmm10{k3},zmm4,zmm19\n"
        "vpaddsb ymm11,ymm10,YMMWORD PTR [rax-0x80]\n"
        "vpaddsb ymm2,ymm6,YMMWORD PTR [r10+rbx*8]\n"
        "vpaddsb zmm17{k1}{z},zmm16,ZMMWORD PTR [rdi-0x40]\n"
        "vpaddsw ymm10{k6},ymm19,YMMWORD PTR [rbx+rax*1-0x1000]\n"
        "vpaddsw xmm22,xmm13,XMMWORD PTR [r9+rsi*1]\n"
        "vpaddsw zmm22{k1}{z},zmm8,ZMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpaddusb zmm26,zmm10,ZMMWORD PTR [rcx+rbx*1+0x40]\n"
        "vpaddusb zmm23{k2},zmm18,zmm21\n"
        "vpaddusb ymm11,ymm1,YMMWORD PTR [rbp*2+0x1000]\n"
        "vpaddusw ymm11,ymm8,YMMWORD PTR [rcx-0x80]\n"
        "vpaddusw xmm14,xmm8,XMMWORD PTR [rcx-0x40]\n"
        "vpaddusw zmm5{k1},zmm2,ZMMWORD PTR [r14-0x80]\n"
        "vpaddw ymm12{k7}{z},ymm7,ymm17\n"
        "vpaddw ymm21,ymm28,YMMWORD PTR [r15*8+0x10]\n"
        "vpaddw xmm5{k3},xmm15,xmm15\n"
        "vpand xmm6,xmm2,XMMWORD PTR [r12-0x80]\n"
        "vpand ymm0,ymm15,ymm7\n"
        "vpand ymm3,ymm5,YMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpandd ymm6,ymm10,ymm18\n"
        "vpandd xmm15,xmm7,XMMWORD PTR [rax-0x1000]\n"
        "vpandd ymm12{k6}{z},ymm27,ymm30\n"
        "vpandn ymm7,ymm6,YMMWORD PTR [rbx-0x1000]\n"
        "vpandn ymm1,ymm12,YMMWORD PTR [rdi+r12*8]\n"
        "vpandn xmm9,xmm7,xmm3\n"
        "vpandnd zmm19{k2}{z},zmm0,zmm9\n"
        "vpandnd ymm6,ymm31,YMMWORD PTR [r9*2+0x1000]\n"
        "vpandnd ymm24{k2},ymm7,ymm16\n"
        "vpandnq xmm4{k4}{z},xmm17,xmm29\n"
        "vpandnq zmm28,zmm12,ZMMWORD PTR [r10+0x1000]\n"
        "vpandnq ymm3{k7},ymm30,QWORD BCST [rcx+0x8]\n"
        "vpandq ymm9,ymm8,YMMWORD PTR [r10+0x1000]\n"
        "vpandq zmm30{k7}{z},zmm31,zmm5\n"
        "vpandq zmm4{k4}{z},zmm30,zmm17\n"
        "vpavgb zmm12{k5}{z},zmm12,ZMMWORD PTR [r14+rbx*4]\n"
        "vpavgb xmm1,xmm12,XMMWORD PTR [r8]\n"
        "vpavgb ymm12,ymm29,YMMWORD PTR [r10+0x12345678]\n"
        "vpavgw ymm11{k3},ymm23,ymm3\n"
        "vpavgw xmm23{k3}{z},xmm19,xmm27\n"
        "vpavgw ymm4,ymm12,YMMWORD PTR [r11*8+0x10]\n"
        "vpblendd ymm13,ymm15,ymm12,0xe1\n"
        "vpblendd ymm15,ymm12,YMMWORD PTR [rsi-0x40],0xf2\n"
        "vpblendd ymm2,ymm4,YMMWORD PTR [rip+0x100],0x40\n"
        "vpblendvb ymm7,ymm2,YMMWORD PTR [rcx*8+0x10],ymm1\n"
        "vpblendvb xmm12,xmm9,xmm14,xmm14\n"
        "vpblendvb ymm8,ymm15,YMMWORD PTR [r15],ymm8\n"
        "vpblendw ymm15,ymm15,YMMWORD PTR [rsi+r9*2+0x12345678],0x34\n"
        "vpblendw ymm9,ymm8,YMMWORD PTR [r8+r12*8-0x1000],0x25\n"
        "vpblendw ymm1,ymm15,YMMWORD PTR [rdi],0xcb\n"
        "vpbroadcastb ymm20{k7},xmm2\n"
        "vpbroadcastb zmm26,BYTE PTR [r13+r12*2-0x80]\n"
        "vpbroadcastb ymm13,BYTE PTR [rip+0x100]\n"
        "vpbroadcastd ymm7{k7}{z},ebx\n"
        "vpbroadcastd xmm19{k3},DWORD PTR [r14+rax*4+0x8]\n"
        "vpbroadcastd ymm8,xmm15\n"
        "vpbroadcastmb2q xmm10,k7\n"
        "vpbroadcastmb2q xmm14,k1\n"
        "vpbroadcastmb2q ymm3,k6\n"
        "vpbroadcastmw2d zmm27,k5\n"
        "vpbroadcastmw2d zmm25,k6\n"
        "vpbroadcastmw2d zmm10,k4\n"
        "vpbroadcastq zmm31{k1},r12\n"
        "vpbroadcastq ymm5,QWORD PTR [rsi*2+0x10]\n"
        "vpbroadcastq zmm23{k1},xmm31\n"
        "vpbroadcastw ymm10{k6}{z},r8d\n"
        "vpbroadcastw zmm26,esi\n"
        "vpbroadcastw ymm15{k3}{z},WORD PTR [rbp+r15*4-0x1000]\n"
        "vpclmulhqhqdq ymm4,ymm4,YMMWORD PTR [r14+0x8]\n"
        "vpclmulhqhqdq xmm11,xmm0,XMMWORD PTR [rbx]\n"
        "vpclmulhqhqdq ymm1,ymm9,YMMWORD PTR [r10*4+0x10]\n"
        "vpclmulhqlqdq ymm13,ymm15,YMMWORD PTR [r13+rbx*8-0x1000]\n"
        "vpclmulhqlqdq xmm0,xmm8,XMMWORD PTR [r8]\n"
        "vpclmulhqlqdq zmm29,zmm27,ZMMWORD PTR [rip+0x100]\n"
        "vpclmullqhqdq xmm7,xmm11,XMMWORD PTR [r14+r14*1-0x40]\n"
        "vpclmullqhqdq zmm28,zmm18,ZMMWORD PTR [r15*2+0x10]\n"
        "vpclmullqhqdq xmm9,xmm4,XMMWORD PTR [rcx-0x1000]\n"
        "vpclmullqlqdq xmm8,xmm2,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpclmullqlqdq ymm14,ymm13,YMMWORD PTR [r9-0x1000]\n"
        "vpclmullqlqdq xmm0,xmm29,XMMWORD PTR [r10*8+0x1000]\n"
        "vpclmulqdq ymm8,ymm4,ymm11,0x13\n"
        "vpclmulqdq xmm16,xmm18,XMMWORD PTR [r8+r15*2-0x40],0x2b\n"
        "vpclmulqdq xmm4,xmm8,XMMWORD PTR [r9+0x12345678],0x3f\n"
        "vpcmpb k5,zmm2,ZMMWORD PTR [rbx+0x80],0xe4\n"
        "vpcmpb k0,zmm28,ZMMWORD PTR [r15+r13*1+0x8],0x39\n"
        "vpcmpb k3,xmm15,xmm19,0x59\n"
        "vpcmpd k7,zmm18,ZMMWORD PTR [r11*2+0x1000],0x75\n"
        "vpcmpd k1,xmm27,DWORD BCST [rip+0x100],0xbd\n"
        "vpcmpd k2,zmm27,zmm11,0x7\n"
        "vpcmpeqb k0{k3},xmm15,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpcmpeqb k3{k3},ymm27,ymm13\n"
        "vpcmpeqb k5{k4},zmm17,ZMMWORD PTR [rip+0x100]\n"
        "vpcmpeqd xmm5,xmm4,XMMWORD PTR [rax+r8*2+0x8]\n"
        "vpcmpeqd k2,xmm12,DWORD BCST [r15+0x1000]\n"
        "vpcmpeqd k5,xmm0,XMMWORD PTR [rbp+rbp*4+0x100]\n"
        "vpcmpeqq k0,ymm25,YMMWORD PTR [r12+0x80]\n"
        "vpcmpeqq xmm9,xmm8,XMMWORD PTR [rcx]\n"
        "vpcmpeqq k2,xmm12,XMMWORD PTR [r10+r15*8]\n"
        "vpcmpequd k2,zmm24,zmm18\n"
        "vpcmpequd k6,zmm6,zmm22\n"
        "vpcmpequd k7,zmm17,zmm7\n"
        "vpcmpeqw k1,zmm2,zmm13\n"
        "vpcmpeqw k4,zmm7,ZMMWORD PTR [r8*4+0x10]\n"
        "vpcmpeqw k6,xmm19,xmm18\n"
        "vpcmpestri xmm5,XMMWORD PTR [rdx*4+0x10],0x1f\n"
        "vpcmpestri xmm14,XMMWORD PTR [rip+0x100],0x69\n"
        "vpcmpestri xmm8,xmm15,0x87\n"
        "vpcmpestrm xmm2,XMMWORD PTR [rdx+rbp*1],0x3e\n"
        "vpcmpestrm xmm14,XMMWORD PTR [r12+rcx*2-0x1000],0xf1\n"
        "vpcmpestrm xmm4,XMMWORD PTR [rdx-0x40],0x9f\n"
        "vpcmpgtb ymm2,ymm4,YMMWORD PTR [r15+0x80]\n"
        "vpcmpgtb ymm2,ymm6,YMMWORD PTR [r14*8+0x10]\n"
        "vpcmpgtb ymm15,ymm15,YMMWORD PTR [r8+r11*4+0x100]\n"
        "vpcmpgtd k5{k3},zmm29,zmm12\n"
        "vpcmpgtd xmm4,xmm2,XMMWORD PTR [rbp+0x0]\n"
        "vpcmpgtd ymm8,ymm9,YMMWORD PTR [r15+0x8]\n"
        "vpcmpgtq k7,ymm31,ymm28\n"
        "vpcmpgtq k1{k5},zmm2,ZMMWORD PTR [r14+rcx*8-0x40]\n"
        "vpcmpgtq k7{k2},zmm2,zmm3\n"
        "vpcmpgtw k4,ymm21,YMMWORD PTR [r12*2+0x1000]\n"
        "vpcmpgtw k7{k5},xmm19,xmm14\n"
        "vpcmpgtw k5,zmm21,zmm23\n"
        "vpcmpistri xmm10,xmm14,0xed\n"
        "vpcmpistri xmm1,XMMWORD PTR [rip+0x100],0xeb\n"
        "vpcmpistri xmm15,XMMWORD PTR [r12*4+0x1000],0xb4\n"
        "vpcmpistrm xmm4,XMMWORD PTR [rip+0x100],0x66\n"
        "vpcmpistrm xmm3,xmm11,0xe3\n"
        "vpcmpistrm xmm0,XMMWORD PTR [rcx+0x80],0x29\n"
        "vpcmpled k2,zmm27,zmm1\n"
        "vpcmpled k5,zmm3,zmm9\n"
        "vpcmpled k3,zmm19,zmm18\n"
        "vpcmpleq k1,zmm26,zmm3\n"
        "vpcmpleud k4,zmm10,zmm15\n"
        "vpcmpleud k5,zmm15,zmm8\n"
        "vpcmpleud k2,zmm6,zmm12\n"
        "vpcmpltd k3,zmm28,zmm10\n"
        "vpcmpltd k7,zmm5,zmm1\n"
        "vpcmpltd k1,zmm9,zmm1\n"
        "vpcmpltq k7,zmm0,zmm12\n"
        "vpcmpltq k5,zmm20,zmm30\n"
        "vpcmpltq k1,zmm16,zmm26\n"
        "vpcmpltud k1,zmm23,zmm30\n"
        "vpcmpltud k5,zmm7,zmm2\n"
        "vpcmpltud k4,zmm4,zmm8\n"
        "vpcmpneqd k3,zmm25,zmm12\n"
        "vpcmpneqd k2,zmm19,zmm24\n"
        "vpcmpneqd k6,zmm10,zmm12\n"
        "vpcmpneqq k1,zmm4,zmm15\n"
        "vpcmpneqq k4,zmm17,zmm14\n"
        "vpcmpneqq k2,zmm28,zmm14\n"
        "vpcmpnequd k1,zmm3,zmm17\n"
        "vpcmpnequd k4,zmm23,zmm8\n"
        "vpcmpnequd k7,zmm29,zmm22\n"
        "vpcmpnled k3,zmm7,zmm3\n"
        "vpcmpnled k4,zmm29,zmm7\n"
        "vpcmpnled k6,zmm3,zmm2\n"
        "vpcmpnleq k4,zmm8,zmm22\n"
        "vpcmpnleq k1,zmm9,zmm10\n"
        "vpcmpnleq k2,zmm15,zmm15\n"
        "vpcmpnleud k3,zmm24,zmm30\n"
        "vpcmpnleud k2,zmm25,zmm24\n"
        "vpcmpnleud k7,zmm27,zmm9\n"
        "vpcmpnltd k1,zmm7,zmm21\n"
        "vpcmpnltd k3,zmm0,zmm24\n"
        "vpcmpnltq k2,zmm22,zmm6\n"
        "vpcmpnltq k5,zmm8,zmm2\n"
        "vpcmpnltq k1,zmm16,zmm19\n"
        "vpcmpnltud k4,zmm27,zmm21\n"
        "vpcmpnltud k3,zmm29,zmm20\n"
        "vpcmpnltud k2,zmm0,zmm26\n"
        "vpcmpq k2,zmm22,zmm3,0x3\n"
        "vpcmpq k6,zmm18,ZMMWORD PTR [rsi+r8*4+0x40],0x71\n"
        "vpcmpq k4,ymm12,QWORD BCST [rip+0x0],0x49\n"
        "vpcmpub k3,zmm20,ZMMWORD PTR [r8*8+0x1000],0xf2\n"
        "vpcmpub k3,xmm23,xmm4,0x7a\n"
        "vpcmpub k2,zmm0,zmm8,0x8b\n"
        "vpcmpud k5,zmm14,ZMMWORD PTR [rbp+0x80],0xbf\n"
        "vpcmpud k0,xmm28,DWORD BCST [rax+0x12345678],0x55\n"
        "vpcmpud k6,zmm28,zmm20,0x3\n"
        "vpcmpuq k1,zmm4,ZMMWORD PTR [r12*8+0x1000],0x9d\n"
        "vpcmpuq k6,zmm23,ZMMWORD PTR [rip+0x100],0xac\n"
        "vpcmpuq k7,xmm1,QWORD BCST [r11*2+0x1000],0xab\n"
        "vpcmpuw k0,zmm14,ZMMWORD PTR [rip+0x100],0x68\n"
        "vpcmpuw k5,zmm18,zmm27,0x7f\n"
        "vpcmpuw k1,ymm3,YMMWORD PTR [r15+0x40],0x9f\n"
        "vpcmpw k1,xmm2,xmm25,0x3e\n"
        "vpcmpw k5,xmm8,xmm3,0x4f\n"
        "vpcmpw k5,ymm15,YMMWORD PTR [rdi*8+0x10],0xcb\n"
        "vpcompressd xmm30{k5}{z},xmm0\n"
        "vpcompressd ymm23{k2},ymm10\n"
        "vpcompressd ymm13{k3}{z},ymm17\n"
        "vpcompressq ymm9,ymm24\n"
        "vpcompressq XMMWORD PTR [rcx+0x8]{k5},xmm30\n"
        "vpcompressq YMMWORD PTR [rip+0xfffffffffffffff8]{k4},ymm21\n"
        "vpconflictd xmm13,xmm24\n"
        "vpconflictd zmm27{k7}{z},ZMMWORD PTR [rip+0x0]\n"
        "vpconflictd ymm12,YMMWORD PTR [rsi+r13*2-0x80]\n"
        "vpconflictq zmm13{k6}{z},zmm25\n"
        "vpconflictq ymm27{k1},YMMWORD PTR [rip+0x100]\n"
        "vpconflictq ymm30{k4}{z},ymm4\n"
        "vpdpbusd ymm1,ymm12,YMMWORD PTR [rip+0x0]\n"
        "vpdpbusd zmm12,zmm5,ZMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpdpbusd xmm5,xmm0,XMMWORD PTR [rdi+r13*4+0x100]\n"
        "vpdpbusds ymm16,ymm26,ymm26\n"
        "vpdpbusds xmm27{k7},xmm13,DWORD BCST [r8+rax*8-0x40]\n"
        "vpdpbusds ymm5,ymm18,DWORD BCST [r14-0x40]\n"
        "vpdpwssd zmm18,zmm3,DWORD BCST [r8+0x80]\n"
        "vpdpwssd ymm6{k4}{z},ymm9,YMMWORD PTR [r10+r10*2-0x40]\n"
        "vpdpwssd xmm25{k4},xmm5,XMMWORD PTR [r8+rsi*8]\n"
        "vpdpwssds ymm7,ymm8,YMMWORD PTR [r11+rbp*2]\n"
        "vpdpwssds zmm26{k4},zmm15,zmm27\n"
        "vpdpwssds xmm10,xmm8,XMMWORD PTR [rsi*8+0x1000]\n"
        "vperm2f128 ymm7,ymm13,ymm10,0xce\n"
        "vperm2f128 ymm0,ymm9,YMMWORD PTR [rdi*4+0x10],0xe9\n"
        "vperm2f128 ymm14,ymm5,YMMWORD PTR [r12+0x80],0x87\n"
        "vperm2i128 ymm2,ymm0,ymm9,0x74\n"
        "vperm2i128 ymm2,ymm2,YMMWORD PTR [rip+0x0],0x7f\n"
        "vperm2i128 ymm11,ymm14,YMMWORD PTR [r12+rsi*8+0x40],0x99\n"
        "vpermb zmm21{k3}{z},zmm29,zmm20\n"
        "vpermb ymm6{k2},ymm23,YMMWORD PTR [rip+0x100]\n"
        "vpermb ymm10,ymm1,ymm17\n"
        "vpermd ymm29,ymm14,DWORD BCST [r13*2+0x1000]\n"
        "vpermd zmm21{k3}{z},zmm31,zmm15\n"
        "vpermd ymm5,ymm15,YMMWORD PTR [r11+0x1000]\n"
        "vpermi2b zmm20{k3},zmm31,ZMMWORD PTR [r14-0x1000]\n"
        "vpermi2b ymm17{k6}{z},ymm28,YMMWORD PTR [r10+rdi*8+0x40]\n"
        "vpermi2b xmm26{k3},xmm30,xmm14\n"
        "vpermi2d zmm25,zmm30,DWORD BCST [rip+0xfffffffffffffff8]\n"
        "vpermi2d xmm22,xmm23,DWORD BCST [r15+0x40]\n"
        "vpermi2d xmm25,xmm1,XMMWORD PTR [rbx*8+0x1000]\n"
        "vpermi2pd zmm30{k5},zmm20,ZMMWORD PTR [r9+0x100]\n"
        "vpermi2pd zmm13,zmm15,ZMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpermi2pd ymm24,ymm25,ymm25\n"
        "vpermi2ps zmm11{k1},zmm29,zmm15\n"
        "vpermi2ps zmm11{k5}{z},zmm8,zmm24\n"
        "vpermi2ps zmm26{k3}{z},zmm4,DWORD BCST [rdi+0x100]\n"
        "vpermi2q ymm11,ymm16,YMMWORD PTR [rax-0x40]\n"
        "vpermi2q ymm26{k1}{z},ymm18,ymm23\n"
        "vpermi2q zmm24{k4},zmm12,ZMMWORD PTR [rdx*4+0x10]\n"
        "vpermi2w zmm3{k7},zmm19,ZMMWORD PTR [rip+0x100]\n"
        "vpermi2w zmm30,zmm19,ZMMWORD PTR [rip+0x100]\n"
        "vpermi2w zmm24,zmm0,zmm24\n"
        "vpermilpd ymm13,ymm15,ymm1\n"
        "vpermilpd ymm3,ymm4,0x7f\n"
        "vpermilpd xmm10,xmm2,XMMWORD PTR [r9*4+0x10]\n"
        "vpermilps xmm0,xmm2,XMMWORD PTR [rcx*2+0x1000]\n"
        "vpermilps ymm12,YMMWORD PTR [rbp+0x12345678],0xa1\n"
        "vpermilps ymm7,ymm11,YMMWORD PTR [rip+0x100]\n"
        "vpermpd zmm16{k4}{z},zmm30,zmm13\n"
        "vpermpd ymm9,YMMWORD PTR [r8*4+0x10],0x4a\n"
        "vpermpd ymm4,YMMWORD PTR [r9+r10*4+0x12345678],0xb5\n"
        "vpermps ymm3,ymm6,ymm13\n"
        "vpermps ymm19{k5},ymm14,ymm15\n"
        "vpermps ymm14,ymm15,YMMWORD PTR [r10+0x80]\n"
        "vpermq zmm2,zmm25,zmm3\n"
        "vpermq zmm5{k7}{z},zmm3,ZMMWORD PTR [r8*2+0x10]\n"
        "vpermq zmm3{k6},ZMMWORD PTR [rcx+rbx*4-0x1000],0x48\n"
        "vpermt2b xmm12{k7},xmm6,XMMWORD PTR [rcx+rdx*2+0x40]\n"
        "vpermt2b xmm27,xmm8,XMMWORD PTR [r14+r15*8-0x80]\n"
        "vpermt2b zmm21{k3},zmm9,zmm4\n"
        "vpermt2d zmm1{k3},zmm7,zmm28\n"
        "vpermt2d zmm0{k6}{z},zmm5,zmm0\n"
        "vpermt2d xmm28{k6}{z},xmm19,XMMWORD PTR [rdi*2+0x10]\n"
        "vpermt2pd zmm26,zmm16,ZMMWORD PTR [r14+r8*2-0x40]\n"
        "vpermt2pd xmm6{k5}{z},xmm30,XMMWORD PTR [rbx*8+0x10]\n"
        "vpermt2pd xmm2{k1},xmm17,XMMWORD PTR [r11+0x80]\n"
        "vpermt2ps xmm27{k2}{z},xmm22,XMMWORD PTR [r14+rax*4-0x1000]\n"
        "vpermt2ps zmm14,zmm12,zmm20\n"
        "vpermt2ps ymm24{k7},ymm8,YMMWORD PTR [rax+r9*8+0x12345678]\n"
        "vpermt2q xmm31,xmm3,XMMWORD PTR [rip+0x0]\n"
        "vpermt2q xmm22{k6},xmm8,xmm19\n"
        "vpermt2q zmm29{k5}{z},zmm18,ZMMWORD PTR [rip+0x0]\n"
        "vpermt2w xmm13,xmm0,XMMWORD PTR [r12-0x80]\n"
        "vpermt2w zmm16{k2},zmm13,zmm9\n"
        "vpermt2w zmm17,zmm2,ZMMWORD PTR [rcx+0x40]\n"
        "vpermw ymm23{k6}{z},ymm2,YMMWORD PTR [rbx+rsi*1-0x80]\n"
        "vpermw zmm27,zmm31,zmm7\n"
        "vpermw ymm6{k6}{z},ymm8,ymm9\n"
        "vpexpandd zmm13{k1}{z},zmm23\n"
        "vpexpandd ymm21{k3}{z},YMMWORD PTR [rdi+0x12345678]\n"
        "vpexpandd xmm24{k2}{z},XMMWORD PTR [rbp*4+0x1000]\n"
        "vpexpandq ymm8,ymm14\n"
        "vpexpandq ymm1,YMMWORD PTR [rip+0x0]\n"
        "vpexpandq ymm10{k7},ymm21\n"
        "vpextrb BYTE PTR [r8],xmm15,0xaa\n"
        "vpextrb BYTE PTR [rip+0xfffffffffffffff8],xmm4,0xa3\n"
        "vpextrb BYTE PTR [r11*4+0x10],xmm0,0x91\n"
        "vpextrd DWORD PTR [rip+0x100],xmm12,0xd4\n"
        "vpextrd r10d,xmm4,0x2\n"
        "vpextrd DWORD PTR [r9*8+0x10],xmm5,0x4\n"
        "vpextrq QWORD PTR [rax*4+0x1000],xmm5,0x14\n"
        "vpextrq r11,xmm6,0xb3\n"
        "vpextrq QWORD PTR [rip+0x100],xmm9,0xe8\n"
        "vpextrw eax,xmm2,0x9c\n"
        "vpgatherdd ymm5{k1},DWORD PTR [r12+ymm12*2-0x80]\n"
        "vpgatherdd ymm7{k2},DWORD PTR [r9+ymm0*8+0x100]\n"
        "vpgatherdd ymm11,DWORD PTR [r15+ymm2*2-0x80],ymm1\n"
        "vpgatherdq ymm13,QWORD PTR [rdi+xmm6*8],ymm8\n"
        "vpgatherdq zmm30{k5},QWORD PTR [rdx+ymm26*2-0x1000]\n"
        "vpgatherdq ymm23{k3},QWORD PTR [r9+xmm12*2-0x80]\n"
        "vpgatherqd xmm14{k3},DWORD PTR [rbx+ymm31*8+0x40]\n"
        "vpgatherqd xmm21{k2},DWORD PTR [r8+ymm25*2]\n"
        "vpgatherqd xmm6,DWORD PTR [rbx+xmm13*8],xmm7\n"
        "vpgatherqq ymm4,QWORD PTR [r15+ymm0*8+0x1000],ymm13\n"
        "vpgatherqq xmm2{k6},QWORD PTR [r14+xmm5*4-0x80]\n"
        "vpgatherqq zmm13{k2},QWORD PTR [r8+zmm21*4-0x40]\n"
        "vpinsrb xmm11,xmm7,BYTE PTR [r15+0x8],0x92\n"
        "vpinsrb xmm14,xmm14,BYTE PTR [rdi+r9*4],0xfc\n"
        "vpinsrb xmm4,xmm5,r10d,0x3\n"
        "vpinsrd xmm5,xmm14,edi,0x59\n"
        "vpinsrd xmm13,xmm1,DWORD PTR [rip+0x0],0x21\n"
        "vpinsrd xmm4,xmm10,DWORD PTR [r10+r13*1+0x100],0xc6\n"
        "vpinsrq xmm11,xmm11,QWORD PTR [r11-0x40],0x26\n"
        "vpinsrq xmm4,xmm10,rbx,0x44\n"
        "vpinsrq xmm14,xmm13,QWORD PTR [rdx+r8*8-0x40],0x58\n"
        "vpinsrw xmm2,xmm4,WORD PTR [rip+0x100],0x9d\n"
        "vpinsrw xmm12,xmm7,r14d,0x3d\n"
        "vpinsrw xmm2,xmm9,WORD PTR [rsi+0x12345678],0x8\n"
        "vplzcntd zmm1{k3},zmm30\n"
        "vplzcntd zmm28{k1},ZMMWORD PTR [rdi*8+0x1000]\n"
        "vplzcntd xmm6{k5},xmm23\n"
        "vplzcntq zmm19,ZMMWORD PTR [rbx*8+0x1000]\n"
        "vplzcntq zmm24,ZMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vplzcntq zmm28{k2}{z},QWORD BCST [rsi*2+0x1000]\n"
        "vpmadd52huq ymm17{k6}{z},ymm1,YMMWORD PTR [r15*4+0x10]\n"
        "vpmadd52huq xmm27,xmm1,XMMWORD PTR [rdx+r9*1-0x80]\n"
        "vpmadd52huq xmm27,xmm10,XMMWORD PTR [rip+0x100]\n"
        "vpmadd52luq zmm29{k6},zmm24,ZMMWORD PTR [r9*4+0x1000]\n"
        "vpmadd52luq ymm3,ymm25,YMMWORD PTR [rbx*8+0x1000]\n"
        "vpmadd52luq xmm29,xmm23,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpmaddwd ymm7{k2},ymm7,ymm23\n"
        "vpmaddwd ymm0,ymm0,YMMWORD PTR [r8]\n"
        "vpmaddwd ymm6,ymm2,YMMWORD PTR [r14*2+0x1000]\n"
        "vpmaxsd ymm2,ymm7,YMMWORD PTR [rip+0x0]\n"
        "vpmaxsd zmm13,zmm12,ZMMWORD PTR [r14+0x100]\n"
        "vpmaxsd zmm28{k6},zmm7,ZMMWORD PTR [r8]\n"
        "vpmaxsw xmm4,xmm7,XMMWORD PTR [r15+r11*1-0x1000]\n"
        "vpmaxsw ymm9,ymm6,YMMWORD PTR [r11]\n"
        "vpmaxsw xmm2,xmm6,XMMWORD PTR [rbp+0x8]\n"
        "vpmaxub xmm11,xmm17,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpmaxub xmm11{k2},xmm6,XMMWORD PTR [r12+rcx*2+0x100]\n"
        "vpmaxub ymm7,ymm15,YMMWORD PTR [r15*8+0x1000]\n"
        "vpmaxud zmm23{k6},zmm0,zmm0\n"
        "vpmaxud ymm12,ymm13,YMMWORD PTR [r11*4+0x1000]\n"
        "vpmaxud xmm12,xmm2,XMMWORD PTR [r12+0x100]\n"
        "vpminsd zmm25,zmm0,ZMMWORD PTR [rip+0x100]\n"
        "vpminsd ymm13,ymm13,YMMWORD PTR [r10+0x100]\n"
        "vpminsd xmm2{k4},xmm8,xmm20\n"
        "vpminsw xmm9,xmm16,XMMWORD PTR [rip+0x0]\n"
        "vpminsw ymm14{k3},ymm22,YMMWORD PTR [r10+r10*8+0x80]\n"
        "vpminsw ymm0,ymm6,ymm10\n"
        "vpminub ymm12,ymm2,YMMWORD PTR [rsi+r11*2+0x100]\n"
        "vpminub xmm1,xmm4,XMMWORD PTR [r8+r14*1-0x80]\n"
        "vpminub xmm3,xmm15,XMMWORD PTR [rip+0x0]\n"
        "vpminud zmm7{k3}{z},zmm27,zmm13\n"
        "vpminud ymm6,ymm7,YMMWORD PTR [rip+0x0]\n"
        "vpminud xmm3{k4},xmm2,xmm25\n"
        "vpmovdb DWORD PTR [r12+r9*8-0x40],xmm5\n"
        "vpmovdb DWORD PTR [r13+rsi*8+0x12345678],xmm16\n"
        "vpmovdb xmm29,ymm25\n"
        "vpmovdw ymm10{k4}{z},zmm27\n"
        "vpmovdw XMMWORD PTR [r11*8+0x1000],ymm29\n"
        "vpmovdw xmm29{k5},xmm11\n"
        "vpmovmskb r14d,xmm13\n"
        "vpmovqb DWORD PTR [rcx-0x80],ymm13\n"
        "vpmovqb xmm1{k4},zmm30\n"
        "vpmovqb WORD PTR [r9+r8*1+0x80]{k2},xmm26\n"
        "vpmovqd xmm14{k3},xmm4\n"
        "vpmovqd ymm29{k5},zmm12\n"
        "vpmovqd xmm24{k7}{z},xmm2\n"
        "vpmovqw xmm19,zmm10\n"
        "vpmovqw DWORD PTR [rip+0x100],xmm14\n"
        "vpmovqw QWORD PTR [r15],ymm6\n"
        "vpmovsdb DWORD PTR [rcx+rbx*2-0x80]{k6},xmm12\n"
        "vpmovsdb xmm15{k7},ymm29\n"
        "vpmovsdb XMMWORD PTR [rip+0x100]{k2},zmm24\n"
        "vpmovsdw xmm24,xmm7\n"
        "vpmovsdw QWORD PTR [rcx*4+0x1000],xmm18\n"
        "vpmovsdw ymm7{k4},zmm30\n"
        "vpmovsqb DWORD PTR [rdx+0x100],ymm15\n"
        "vpmovsqb QWORD PTR [rdi*4+0x10],zmm14\n"
        "vpmovsqb WORD PTR [rip+0x100],xmm18\n"
        "vpmovsqd QWORD PTR [r15*2+0x10]{k1},xmm10\n"
        "vpmovsqd QWORD PTR [r15-0x1000],xmm11\n"
        "vpmovsqd XMMWORD PTR [r11-0x40],ymm26\n"
        "vpmovsqw DWORD PTR [r11+r11*4+0x40],xmm18\n"
        "vpmovsqw DWORD PTR [rip+0xfffffffffffffff8],xmm30\n"
        "vpmovsqw xmm30{k1}{z},ymm11\n"
        "vpmovswb QWORD PTR [rip+0xfffffffffffffff8],xmm20\n"
        "vpmovswb QWORD PTR [rip+0xfffffffffffffff8]{k2},xmm16\n"
        "vpmovswb ymm25{k1},zmm26\n"
        "vpmovusdb QWORD PTR [r11+r8*4+0x1000]{k5},ymm24\n"
        "vpmovusdb XMMWORD PTR [rbx+0x40],zmm21\n"
        "vpmovusdb QWORD PTR [rip+0x100],ymm0\n"
        "vpmovusdw YMMWORD PTR [r15*8+0x1000]{k5},zmm12\n"
        "vpmovusdw QWORD PTR [rbp*2+0x1000]{k2},xmm25\n"
        "vpmovusdw XMMWORD PTR [r10+0x1000]{k5},ymm31\n"
        "vpmovusqb xmm17{k4}{z},ymm4\n"
        "vpmovusqb xmm4{k2},xmm7\n"
        "vpmovusqb QWORD PTR [r14-0x1000]{k4},zmm6\n"
        "vpmovusqd xmm7{k2}{z},xmm18\n"
        "vpmovusqd xmm5,xmm4\n"
        "vpmovusqd XMMWORD PTR [r13+r14*2+0x1000]{k2},ymm0\n"
        "vpmovusqw DWORD PTR [r14*2+0x1000],xmm18\n"
        "vpmovusqw XMMWORD PTR [rip+0x100]{k5},zmm29\n"
        "vpmovusqw xmm29{k7}{z},xmm5\n"
        "vpmovuswb QWORD PTR [rdi+rsi*4+0x40]{k1},xmm13\n"
        "vpmovuswb XMMWORD PTR [rip+0xfffffffffffffff8],ymm31\n"
        "vpmovuswb xmm20{k3},ymm9\n"
        "vpmovwb xmm2,ymm7\n"
        "vpmovwb QWORD PTR [rdx+0x40]{k6},xmm9\n"
        "vpmovwb xmm1{k4}{z},ymm4\n"
        "vpmuldq ymm29,ymm24,YMMWORD PTR [r15*4+0x10]\n"
        "vpmuldq zmm15{k4},zmm16,ZMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpmuldq zmm19{k7}{z},zmm31,QWORD BCST [rip+0xfffffffffffffff8]\n"
        "vpmulhuw xmm4{k3}{z},xmm1,xmm14\n"
        "vpmulhuw xmm1,xmm11,XMMWORD PTR [rdx*4+0x10]\n"
        "vpmulhuw ymm25,ymm11,YMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpmulhw xmm14{k5},xmm20,XMMWORD PTR [rsi+0x100]\n"
        "vpmulhw xmm9,xmm4,XMMWORD PTR [rip+0x0]\n"
        "vpmulhw zmm16{k2},zmm8,zmm3\n"
        "vpmulld ymm2,ymm2,YMMWORD PTR [r9+rdx*4-0x40]\n"
        "vpmulld xmm14,xmm2,xmm4\n"
        "vpmulld ymm10,ymm13,YMMWORD PTR [r8+0x12345678]\n"
        "vpmullw ymm5{k1}{z},ymm14,ymm15\n"
        "vpmullw zmm21{k1},zmm24,ZMMWORD PTR [rcx+rbx*8+0x1000]\n"
        "vpmullw ymm9,ymm5,YMMWORD PTR [rsi+r12*2+0x80]\n"
        "vpmultishiftqb zmm13{k6}{z},zmm12,zmm30\n"
        "vpmultishiftqb zmm28,zmm11,ZMMWORD PTR [rbx*8+0x10]\n"
        "vpmultishiftqb zmm23,zmm13,zmm26\n"
        "vpmuludq xmm10,xmm0,XMMWORD PTR [r11-0x1000]\n"
        "vpmuludq ymm13,ymm6,YMMWORD PTR [rip+0x100]\n"
        "vpmuludq zmm7,zmm10,QWORD BCST [r10*8+0x10]\n"
        "vpopcntb ymm31{k7},YMMWORD PTR [rax+r13*8+0x40]\n"
        "vpopcntb zmm24,ZMMWORD PTR [rip+0x0]\n"
        "vpopcntb ymm10,YMMWORD PTR [rip+0x0]\n"
        "vpopcntd zmm1,ZMMWORD PTR [r8+0x8]\n"
        "vpopcntd zmm13,ZMMWORD PTR [rip+0x100]\n"
        "vpopcntd zmm22{k4},DWORD BCST [r10+0x100]\n"
        "vpopcntq xmm29{k2}{z},QWORD BCST [r15+0x40]\n"
        "vpopcntq zmm20{k5}{z},zmm24\n"
        "vpopcntq ymm11,QWORD BCST [rip+0x0]\n"
        "vpopcntw zmm13{k7}{z},zmm19\n"
        "vpopcntw xmm30{k6},xmm21\n"
        "vpopcntw ymm19{k5},YMMWORD PTR [r12+r11*2-0x40]\n"
        "vpor xmm10,xmm7,xmm7\n"
        "vpor xmm10,xmm9,XMMWORD PTR [r12+rdx*1]\n"
        "vpor ymm13,ymm0,YMMWORD PTR [rip+0x0]\n"
        "vpord ymm24,ymm12,ymm22\n"
        "vpord ymm14{k1}{z},ymm24,DWORD BCST [rbp-0x1000]\n"
        "vpord zmm16{k1}{z},zmm23,zmm28\n"
        "vporq xmm8{k4}{z},xmm15,xmm24\n"
        "vporq ymm1,ymm2,YMMWORD PTR [rip+0x100]\n"
        "vporq zmm13{k3}{z},zmm16,zmm24\n"
        "vpsadbw xmm3,xmm11,xmm15\n"
        "vpsadbw xmm10,xmm1,XMMWORD PTR [rip+0x0]\n"
        "vpsadbw ymm1,ymm14,YMMWORD PTR [r11+0x100]\n"
        "vpscatterdd DWORD PTR [r13+ymm16*2+0x80]{k6},ymm3\n"
        "vpscatterdd DWORD PTR [rdi+zmm31*4]{k2},zmm2\n"
        "vpscatterdd DWORD PTR [r14+zmm25*4+0x100]{k5},zmm15\n"
        "vpscatterdq QWORD PTR [r13+xmm7*2+0x40]{k3},xmm10\n"
        "vpscatterdq QWORD PTR [r15+xmm10*1-0x80]{k2},xmm1\n"
        "vpscatterdq QWORD PTR [r14+xmm20*2+0x8]{k5},xmm31\n"
        "vpscatterqd DWORD PTR [rdi+ymm13*1+0x40]{k6},xmm6\n"
        "vpscatterqd DWORD PTR [rax+ymm8*2]{k4},xmm28\n"
        "vpscatterqd DWORD PTR [rdx+zmm19*2+0x100]{k1},ymm7\n"
        "vpscatterqq QWORD PTR [r15+zmm11*2+0x12345678]{k6},zmm26\n"
        "vpscatterqq QWORD PTR [r8+xmm13*2+0x80]{k5},xmm28\n"
        "vpscatterqq QWORD PTR [rsi+zmm14*2-0x80]{k4},zmm7\n"
        "vpshufb xmm6{k7}{z},xmm3,XMMWORD PTR [r10+0x40]\n"
        "vpshufb xmm20{k6}{z},xmm18,XMMWORD PTR [r11+r10*2+0x100]\n"
        "vpshufb ymm2{k5}{z},ymm24,ymm14\n"
        "vpshufbitqmb k3{k6},xmm3,XMMWORD PTR [r8+0x100]\n"
        "vpshufbitqmb k3{k2},ymm0,ymm0\n"
        "vpshufbitqmb k5{k3},ymm10,ymm3\n"
        "vpshufd ymm11,YMMWORD PTR [rbp*4+0x1000],0x4\n"
        "vpshufd xmm2,XMMWORD PTR [r14+rdx*4-0x80],0xce\n"
        "vpshufd xmm12,XMMWORD PTR [rcx],0x6d\n"
        "vpshufhw xmm8,XMMWORD PTR [rbp-0x80],0x14\n"
        "vpshufhw xmm11,XMMWORD PTR [rcx+r12*4-0x1000],0x47\n"
        "vpshufhw xmm3,XMMWORD PTR [rax*2+0x10],0xa2\n"
        "vpshuflw ymm5,ymm10,0x80\n"
        "vpshuflw ymm3,YMMWORD PTR [rip+0xfffffffffffffff8],0x3e\n"
        "vpslld ymm29{k1}{z},DWORD BCST [r11-0x80],0x25\n"
        "vpslld zmm8{k1}{z},DWORD BCST [r14*4+0x1000],0x86\n"
        "vpslld xmm10,xmm12,XMMWORD PTR [rbx+0x100]\n"
        "vpslldq zmm26,ZMMWORD PTR [rbx-0x80],0x8\n"
        "vpslldq ymm6,ymm8,0x58\n"
        "vpslldq zmm16,ZMMWORD PTR [rcx+r14*8+0x8],0xab\n"
        "vpsllq xmm27,XMMWORD PTR [r14*2+0x1000],0x9a\n"
        "vpsllq ymm14,ymm10,XMMWORD PTR [r9+rsi*2-0x40]\n"
        "vpsllq ymm7,ymm9,XMMWORD PTR [rip+0x100]\n"
        "vpsllvd xmm6,xmm10,XMMWORD PTR [rcx+rdi*2+0x1000]\n"
        "vpsllvd xmm3{k5},xmm25,xmm19\n"
        "vpsllvd xmm10{k6}{z},xmm1,xmm21\n"
        "vpsllvq ymm29,ymm31,YMMWORD PTR [r14+rcx*2+0x100]\n"
        "vpsllvq xmm16{k3}{z},xmm17,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpsllvq xmm1{k7},xmm21,XMMWORD PTR [r11+r13*4-0x40]\n"
        "vpsllw zmm16{k5}{z},zmm19,0xe3\n"
        "vpsllw xmm1,xmm8,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpsllw xmm9,xmm1,xmm7\n"
        "vpsrad zmm6,ZMMWORD PTR [rip+0x0],0x9b\n"
        "vpsrad zmm0{k4},zmm30,0x37\n"
        "vpsrad ymm4,ymm2,XMMWORD PTR [rsi*2+0x10]\n"
        "vpsraq ymm17{k4}{z},ymm17,0x33\n"
        "vpsraq ymm2{k5},ymm30,0x5a\n"
        "vpsraq ymm16{k6}{z},QWORD BCST [rsi+0x80],0x1c\n"
        "vpsravd ymm27,ymm28,DWORD BCST [rip+0x0]\n"
        "vpsravd xmm2{k2},xmm7,xmm30\n"
        "vpsravd zmm29,zmm17,ZMMWORD PTR [rbx+0x8]\n"
        "vpsravq ymm15{k6}{z},ymm0,ymm2\n"
        "vpsravq xmm12,xmm31,xmm31\n"
        "vpsravq xmm11,xmm18,QWORD BCST [r13+rsi*8-0x80]\n"
        "vpsraw zmm25{k7},zmm30,0x56\n"
        "vpsraw ymm20,YMMWORD PTR [rdx+rsi*2+0x40],0xf1\n"
        "vpsraw ymm27{k1},ymm26,0x43\n"
        "vpsrld zmm18{k2}{z},ZMMWORD PTR [rbx+0x1000],0xb7\n"
        "vpsrld ymm8,ymm12,XMMWORD PTR [rdx+r14*2-0x40]\n"
        "vpsrld xmm15{k7}{z},xmm8,0x10\n"
        "vpsrldq zmm7,ZMMWORD PTR [rip+0x100],0xe2\n"
        "vpsrldq xmm3,xmm3,0xec\n"
        "vpsrlq xmm25{k2}{z},xmm12,0x41\n"
        "vpsrlq xmm0,xmm6,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpsrlq zmm10,ZMMWORD PTR [rip+0xfffffffffffffff8],0x1d\n"
        "vpsrlvd xmm6,xmm1,XMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpsrlvd xmm10,xmm10,xmm6\n"
        "vpsrlvd ymm14,ymm12,YMMWORD PTR [rdi+r14*2+0x12345678]\n"
        "vpsrlvq xmm0,xmm15,xmm8\n"
        "vpsrlvq xmm22{k5}{z},xmm25,xmm8\n"
        "vpsrlvq zmm11{k2},zmm6,QWORD BCST [rbp+rbp*4-0x40]\n"
        "vpsrlw xmm1,xmm9,XMMWORD PTR [rip+0x0]\n"
        "vpsrlw xmm11,xmm9,XMMWORD PTR [r15+0x40]\n"
        "vpsrlw xmm8,xmm2,xmm12\n"
        "vpsubb ymm11{k7},ymm12,YMMWORD PTR [rdx-0x80]\n"
        "vpsubb zmm15,zmm21,ZMMWORD PTR [rip+0x0]\n"
        "vpsubb ymm7,ymm12,YMMWORD PTR [rcx+0x1000]\n"
        "vpsubd xmm28{k5},xmm7,XMMWORD PTR [rdi+rdx*2+0x12345678]\n"
        "vpsubd ymm4,ymm0,ymm13\n"
        "vpsubd ymm0{k1}{z},ymm7,DWORD BCST [rsi*4+0x1000]\n"
        "vpsubq xmm3,xmm3,XMMWORD PTR [rbx-0x40]\n"
        "vpsubq ymm7,ymm9,YMMWORD PTR [rip+0x0]\n"
        "vpsubq xmm19{k2}{z},xmm6,XMMWORD PTR [rbp+rdx*8+0x12345678]\n"
        "vpsubsb ymm25{k6},ymm9,YMMWORD PTR [rip+0x100]\n"
        "vpsubsb ymm22{k5},ymm11,YMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpsubsb xmm9,xmm3,XMMWORD PTR [r13+r15*1+0x100]\n"
        "vpsubsw zmm5,zmm5,ZMMWORD PTR [rbx]\n"
        "vpsubsw xmm17,xmm7,XMMWORD PTR [rip+0x100]\n"
        "vpsubsw zmm9,zmm26,ZMMWORD PTR [r11+r9*4+0x12345678]\n"
        "vpsubusb zmm25,zmm5,ZMMWORD PTR [rip+0x100]\n"
        "vpsubusb xmm7,xmm3,XMMWORD PTR [r9+0x100]\n"
        "vpsubusb xmm12{k6}{z},xmm0,XMMWORD PTR [rdi+rdx*8-0x1000]\n"
        "vpsubusw ymm20,ymm0,YMMWORD PTR [rdi+r15*2-0x80]\n"
        "vpsubusw zmm11{k2}{z},zmm24,zmm22\n"
        "vpsubusw xmm13,xmm13,XMMWORD PTR [r12-0x80]\n"
        "vpsubw ymm21{k3}{z},ymm0,YMMWORD PTR [r15]\n"
        "vpsubw zmm14{k3}{z},zmm22,ZMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpsubw zmm11{k6},zmm31,ZMMWORD PTR [rbp*4+0x10]\n"
        "vpternlogd zmm9{k3}{z},zmm26,DWORD BCST [r15+rdx*8-0x80],0x8d\n"
        "vpternlogd xmm15{k4}{z},xmm22,XMMWORD PTR [rdi+r9*2+0x12345678],0x19\n"
        "vpternlogd ymm21,ymm2,YMMWORD PTR [rdx+0x8],0x3a\n"
        "vpternlogq ymm21{k5},ymm11,ymm4,0x51\n"
        "vpternlogq xmm20,xmm7,QWORD BCST [rip+0xfffffffffffffff8],0xa1\n"
        "vpternlogq xmm18{k2}{z},xmm9,xmm26,0x84\n"
        "vpunpckhbw xmm11,xmm5,xmm1\n"
        "vpunpckhbw xmm4,xmm1,XMMWORD PTR [rsi+0x12345678]\n"
        "vpunpckhbw ymm7,ymm8,YMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpunpckhdq zmm4,zmm10,ZMMWORD PTR [r9-0x1000]\n"
        "vpunpckhdq ymm3,ymm6,YMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpunpckhdq ymm1,ymm0,YMMWORD PTR [rsi+r9*4+0x40]\n"
        "vpunpckhqdq zmm25{k2}{z},zmm6,ZMMWORD PTR [rbx+rdi*2+0x12345678]\n"
        "vpunpckhqdq xmm7,xmm5,XMMWORD PTR [rbx+r8*1+0x1000]\n"
        "vpunpckhqdq zmm17,zmm11,ZMMWORD PTR [rcx+rcx*2-0x40]\n"
        "vpunpckhwd xmm11,xmm12,XMMWORD PTR [rbx+0x40]\n"
        "vpunpckhwd xmm21{k2},xmm7,XMMWORD PTR [rsi+r13*8-0x40]\n"
        "vpunpckhwd ymm21,ymm21,YMMWORD PTR [rip+0x100]\n"
        "vpunpcklbw xmm0,xmm9,XMMWORD PTR [rcx*4+0x1000]\n"
        "vpunpcklbw ymm10,ymm15,YMMWORD PTR [rsi*2+0x1000]\n"
        "vpunpcklbw zmm17{k7},zmm15,ZMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vpunpckldq xmm20{k7}{z},xmm0,XMMWORD PTR [rbp*8+0x10]\n"
        "vpunpckldq ymm14,ymm6,YMMWORD PTR [rax*4+0x1000]\n"
        "vpunpckldq ymm6,ymm13,ymm15\n"
        "vpunpcklqdq xmm0,xmm27,XMMWORD PTR [r10]\n"
        "vpunpcklqdq xmm13,xmm11,XMMWORD PTR [rip+0x0]\n"
        "vpunpcklqdq ymm0,ymm8,YMMWORD PTR [rip+0x0]\n"
        "vpunpcklwd xmm30{k5},xmm14,xmm19\n"
        "vpunpcklwd ymm28{k1}{z},ymm5,ymm0\n"
        "vpunpcklwd ymm15,ymm15,YMMWORD PTR [rbp+0x12345678]\n"
        "vpxor ymm7,ymm8,ymm10\n"
        "vpxor xmm10,xmm11,XMMWORD PTR [r10*2+0x1000]\n"
        "vpxor ymm11,ymm6,YMMWORD PTR [r11*4+0x10]\n"
        "vpxord zmm3,zmm2,zmm0\n"
        "vpxord zmm14{k2}{z},zmm31,zmm6\n"
        "vpxord zmm16{k7},zmm20,zmm3\n"
        "vpxorq zmm3{k5},zmm26,QWORD BCST [r8*8+0x1000]\n"
        "vpxorq xmm19{k7}{z},xmm28,xmm22\n"
        "vpxorq zmm22{k1},zmm16,QWORD BCST [r13+r8*2+0x12345678]\n"
        "vscatterdpd QWORD PTR [r11+ymm21*4+0x80]{k3},zmm10\n"
        "vscatterdpd QWORD PTR [rcx+ymm2*4+0x100]{k7},zmm12\n"
        "vscatterdpd QWORD PTR [rdi+ymm20*2-0x80]{k3},zmm3\n"
        "vscatterdps DWORD PTR [rbx+xmm29*8+0x80]{k3},xmm25\n"
        "vscatterdps DWORD PTR [r10+zmm23*1+0x40]{k5},zmm30\n"
        "vscatterdps DWORD PTR [r11+zmm12*2-0x80]{k6},zmm15\n"
        "vscatterqpd QWORD PTR [r10+xmm28*4+0x12345678]{k1},xmm31\n"
        "vscatterqpd QWORD PTR [r8+xmm14*1-0x1000]{k2},xmm19\n"
        "vscatterqpd QWORD PTR [r12+zmm8*8]{k5},zmm11\n"
        "vscatterqps DWORD PTR [rsi+ymm25*2+0x8]{k4},xmm18\n"
        "vscatterqps DWORD PTR [rbp+ymm18*4-0x40]{k6},xmm14\n"
        "vscatterqps DWORD PTR [r10+ymm15*2-0x1000]{k3},xmm22\n"
        "vshufpd zmm27{k5}{z},zmm21,zmm12,0x61\n"
        "vshufpd ymm25,ymm30,YMMWORD PTR [r8*4+0x1000],0x36\n"
        "vshufpd ymm31,ymm28,YMMWORD PTR [rip+0x0],0x3b\n"
        "vshufps xmm12,xmm5,XMMWORD PTR [r15+r11*8-0x1000],0x30\n"
        "vshufps zmm13{k7}{z},zmm13,ZMMWORD PTR [rip+0x0],0x1e\n"
        "vshufps zmm2{k6}{z},zmm22,zmm5,0x38\n"
        "vsqrtpd ymm6,YMMWORD PTR [r8*4+0x1000]\n"
        "vsqrtpd xmm7{k3}{z},xmm13\n"
        "vsqrtpd ymm6,YMMWORD PTR [rip+0xfffffffffffffff8]\n"
        "vsqrtps ymm14,YMMWORD PTR [r15+rcx*1-0x54]\n"
        "vsqrtps xmm4,XMMWORD PTR [rax+r14*8+0xd5]\n"
        "vsqrtps zmm8,zmm7{rz-sae}\n"
        "vsqrtsd xmm19{k5},xmm3,xmm29\n"
        "vsqrtsd xmm12,xmm15,xmm0\n"
        "vsqrtsd xmm28,xmm0,xmm11{rz-sae}\n"
        "vsqrtss xmm3,xmm15,DWORD PTR [rbp+0x0]\n"
        "vsqrtss xmm8,xmm30,xmm9{ru-sae}\n"
        "vsqrtss xmm0,xmm15,DWORD PTR [r12-0x40]\n"
        "vsubpd ymm10,ymm6,ymm5\n"
        "vsubpd ymm4{k2},ymm21,YMMWORD PTR [rdx+r12*1+0x80]\n"
        "vsubpd ymm12,ymm0,YMMWORD PTR [r14+0x8]\n"
        "vsubps xmm1{k2}{z},xmm1,DWORD BCST [r9*4+0x10]\n"
        "vsubps xmm0,xmm7,xmm14\n"
        "vsubps ymm9,ymm19,YMMWORD PTR [rdi+r11*4+0x1000]\n"
        "vsubsd xmm5,xmm8,QWORD PTR [r14+0x8]\n"
        "vsubsd xmm31,xmm1,QWORD PTR [r10+rdi*4-0x40]\n"
        "vsubsd xmm3,xmm27,xmm17{rd-sae}\n"
        "vsubss xmm2,xmm6,DWORD PTR [r12+r8*4-0xd7]\n"
        "vsubss xmm3,xmm19,xmm28{rd-sae}\n"
        "vsubss xmm0{k1},xmm0,xmm14{ru-sae}\n"
        "vucomisd xmm21,QWORD PTR [rbp*8+0x10]\n"
        "vucomisd xmm5,QWORD PTR [rcx+0x1000]\n"
        "vucomisd xmm14,QWORD PTR [r11+rdx*4+0x40]\n"
        "vucomiss xmm6,DWORD PTR [r14+rdx*1+0x80]\n"
        "vucomiss xmm1,xmm15{sae}\n"
        "vucomiss xmm14,xmm11\n"
        "vunpckhpd zmm23{k2}{z},zmm14,QWORD BCST [rbx+0x100]\n"
        "vunpckhpd xmm10,xmm11,XMMWORD PTR [rax+0x12345678]\n"
        "vunpckhpd ymm10,ymm15,YMMWORD PTR [rax]\n"
        "vunpckhps ymm4{k3},ymm2,DWORD BCST [rcx+0x100]\n"
        "vunpckhps zmm28,zmm29,ZMMWORD PTR [r9*2+0x10]\n"
        "vunpckhps ymm8,ymm1,YMMWORD PTR [rdi+r8*4+0x8]\n"
        "vunpcklpd xmm6,xmm2,XMMWORD PTR [rbx+rdx*4+0x100]\n"
        "vunpcklpd zmm10{k4},zmm28,zmm17\n"
        "vunpcklpd zmm5{k3},zmm14,ZMMWORD PTR [rsi-0x1000]\n"
        "vunpcklps ymm22{k3},ymm1,ymm18\n"
        "vunpcklps xmm5,xmm14,XMMWORD PTR [r15+0x40]\n"
        "vunpcklps xmm11,xmm6,XMMWORD PTR [rdx]\n"
        "vxorpd ymm8,ymm7,YMMWORD PTR [rbx+rsi*8+0x100]\n"
        "vxorpd xmm14,xmm29,QWORD BCST [rip+0x100]\n"
        "vxorpd xmm13{k7}{z},xmm5,xmm0\n"
        "vxorps xmm1,xmm12,XMMWORD PTR [rax*2+0x1000]\n"
        "vxorps xmm10,xmm8,xmm6\n"
        "vxorps xmm7,xmm3,XMMWORD PTR [rbp+rdx*8+0x0]\n"
        "wbinvd\n"
        "wrmsr\n"
        "wrssd [rip+0x100],r8d\n"
        "wrssd [rcx+0x80],eax\n"
        "wrssd [rax+r14*2+0x1000],ecx\n"
        "wrssq [r11+rdx*2+0x100],rsi\n"
        "wrssq [rsi+0x8],r8\n"
        "wrssq [rip+0x0],r15\n"
        "wrussd [r12+r12*4+0x100],r11d\n"
        "wrussd [r14+rbp*2],r9d\n"
        "wrussd [r12+r14*1-0x1000],r9d\n"
        "wrussq [rdx+r13*1+0x40],r15\n"
        "wrussq [rip+0xfffffffffffffff8],rcx\n"
        "wrussq [r11+0x8],rcx\n"
        "xadd cx,r15w\n"
        "xchg QWORD PTR [ebx-0x80],rdi\n"
        "xchg DWORD PTR fs:[rbp+0x12345678],esp\n"
        "xchg QWORD PTR [rax+rsi*2+0x12345678],r9\n"
        "xlat BYTE PTR ds:[rbx]\n"
        "xor WORD PTR [eax+0x0],r15w\n"
        "xor WORD PTR [rax+r13*1-0x80],dx\n"
        "xor QWORD PTR [r12-0x40],0x80\n"
        "xorps xmm0,XMMWORD PTR fs:[rcx]\n"
        "xorps xmm1,XMMWORD PTR [rip+0x0]\n"
        "xorps xmm12,XMMWORD PTR [eax+0x7f]\n"
        "xrstor [r11+0x100]\n"
        "xrstor [rip+0xfffffffffffffff8]\n"
        "xrstor [r10+rcx*1+0x40]\n"
        "xrstor64 [rip+0xfffffffffffffff8]\n"
        "xrstor64 [rbp+rcx*4-0x1000]\n"
        "xrstor64 [rax*2+0x1000]\n"
        "xrstors [rax+0x8]\n"
        "xrstors [r14*8+0x1000]\n"
        "xrstors [rip+0x0]\n"
        "xrstors64 [r15*2+0x1000]\n"
        "xrstors64 [rsi+r10*1+0x1000]\n"
        "xrstors64 [r9+0x8]\n"
        "xsave [r9-0x80]\n"
        "xsave [r11*2+0x1000]\n"
        "xsave [rip+0xfffffffffffffff8]\n"
        "xsave64 [rip+0x0]\n"
        "xsave64 [rax*4+0x1000]\n"
        "xsave64 [r13+rbx*1+0x8]\n"
        "xsavec [r9*2+0x1000]\n"
        "xsavec [rip+0xfffffffffffffff8]\n"
        "xsavec [r11+r12*4+0x100]\n"
        "xsavec64 [r14+r12*8]\n"
        "xsavec64 [r13+0x40]\n"
        "xsavec64 [rip+0xfffffffffffffff8]\n"
        "xsaveopt [r11-0x1000]\n"
        "xsaveopt [rdi+r13*2+0x8]\n"
        "xsaveopt [rip+0xfffffffffffffff8]\n"
        "xsaveopt64 [rbx-0x40]\n"
        "xsaveopt64 [rbp*2+0x10]\n"
        "xsaveopt64 [rax+0x12345678]\n"
        "xsaves [rdx*8+0x10]\n"
        "xsaves [r13-0x40]\n"
        "xsaves [r15+rax*1+0x1000]\n"
        "xsaves64 [r14-0x40]\n"
        "xsaves64 [rip+0x0]\n"
        "xsaves64 [rdx*4+0x10]\n"
        "{evex} vpslld xmm3,XMMWORD PTR [rdx+r9*1+0x7c],0x1c\n"
        "{evex} vpsrldq ymm13,YMMWORD PTR [rip+0x0],0x32\n"
        "{evex} vpsrlq ymm14,YMMWORD PTR [rdx],0x0\n"
        "mov eax, 5\n"
        "mov rax, QWORD PTR [rbx+rcx*4+8]\n"
        "add BYTE PTR [rax], 1\n"
        "movzx eax, BYTE PTR [rdi]\n"
        "movsx rax, WORD PTR [rdi+2]\n"
        "movsxd rax, DWORD PTR [rdi]\n"
        "lea rax, [rsp+rax]\n"
        "call rax\n"
        "jmp QWORD PTR [rax*8+0x10]\n"
        "fld TBYTE PTR [rax]\n"
        "fild QWORD PTR [rax]\n"
        "fsub st(1), st\n"
        "fsubp st(1), st\n"
        "fsub st, st(1)\n"
        "vaddps zmm0{k1}{z}, zmm1, DWORD PTR [rax]{1to16}\n"
        "vaddps zmm0, zmm1, zmm2, {rn-sae}\n"
        "cvtsi2sd xmm0, DWORD PTR [rax]\n"
        "mov rax, QWORD PTR fs:0x28\n"
        "mov eax, DWORD PTR fs:[rax]\n"
        "movsd\n"
        "rep stosq\n"
        "imul eax, ebx, 5\n"
        "shl DWORD PTR [rax], cl\n"
        "nop WORD PTR cs:[rax+rax*1+0x0]\n"
        "push 5\n"
        "enter 16, 0\n"
        "vpgatherdd ymm0{k1}, [rax+ymm1*4]\n"
        "pinsrw xmm0, WORD PTR [rax], 1\n"
        "movq rax, xmm0\n"
        "movd mm0, DWORD PTR [rax]\n"
        "crc32 eax, BYTE PTR [rax]\n"
        "movabs rax, 0x1122334455667788\n"
        "movs byte ptr es:[rdi], byte ptr fs:[rsi]\n"
        "lods dword ptr gs:[rsi]\n"
        "outs dx, byte ptr [rsi]\n"
        "ins dword ptr es:[rdi], dx\n"
        "xlat byte ptr [rbx]\n"
        "movs qword ptr [rdi], qword ptr [rsi]\n"
        "rep stos byte ptr [rdi], al\n"
        "cmps byte ptr [rsi], byte ptr es:[rdi]\n"
        "scas al, byte ptr es:[rdi]\n"
        "vcmpltps xmm1, xmm2, xmm3\n"
        "vcmpnge_uqpd ymm1, ymm2, YMMWORD PTR [rax]\n"
        "vcmptrue_usss xmm1, xmm2, DWORD PTR [rax+4]\n"
        "vcmpeqps k1{k2}, zmm2, zmm3\n"
        "vpcmpltud k1, zmm2, zmm3\n"
        "vpcmpnleq k3{k1}, zmm4, QWORD PTR [rax]{1to8}\n"
        "vpcmpequb k1, zmm2, zmm3\n"
        "vpcmpneqw k1, ymm2, ymm3\n"
        "mov bh, 1\n"
        "mov ah, 0x80\n"
        "mov ch, BYTE PTR [rax]\n"
        "fsubp st(0), st\n"
        "fdivrp\n"
        "vaddps zmm3, zmm2, zmm1{rn-sae}\n"
        "vaddps zmm3, zmm2, DWORD BCST [rax]\n"
        "mov eax, [rbx + rsp]\n"
        "mov eax, [4*rcx + rbx - 8]\n"
        "mov eax, DWORD PTR [rax + (2+3)*4]\n"
        "mov eax, DWORD PTR -8[rbp]\n"
        "mov eax, DWORD PTR es:[rax]\n"
        "mov %eax, %ebx\n"
        "mov eax, [%rbx]\n"
        "lea eax, [eax + ecx*2]\n"
        "call [rax]\n"
        "jmp QWORD PTR fs:[rax]\n"
        "push QWORD PTR [rip]\n"
        ".att_syntax\n"
        ".globl axiom_asm_table_end\n"
        "axiom_asm_table_end:\n"
        ".text\n");

extern const unsigned char axiom_asm_table[], axiom_asm_table_end[];
static const char *const want[] = {
    "40106bc0",  /* adc BYTE PTR [rbx-0x40],bpl */
    "67831610",  /* adc DWORD PTR [esi+0x0],0x10 */
    "4d13248c",  /* adc r12,QWORD PTR [r12+rcx*4] */
    "664c0f38f6c9",  /* adcx r9,rcx */
    "678103ff000000",  /* add DWORD PTR [ebx],0xff */
    "646681442408ff00",  /* add WORD PTR fs:[rsp+0x8],0xff */
    "64490361c0",  /* add rsp,QWORD PTR fs:[r9-0x40] */
    "f2450f581cc0",  /* addsd xmm11,QWORD PTR [r8+rax*8+0x0] */
    "f20f582c4500000000",  /* addsd xmm5,QWORD PTR [rax*2+0x0] */
    "f2440f58042500000000",  /* addsd xmm8,QWORD PTR ds:0x0 */
    "f30f5868c0",  /* addss xmm5,DWORD PTR [rax-0x40] */
    "64f3450f586980",  /* addss xmm13,DWORD PTR fs:[r9-0x80] */
    "f3440f580c2500000000",  /* addss xmm9,DWORD PTR ds:0x0 */
    "4a233c2f",  /* and rdi,QWORD PTR [rdi+r13*1+0x0] */
    "6580242480",  /* and BYTE PTR gs:[rsp+0x0],0x80 */
    "644123642480",  /* and esp,DWORD PTR fs:[r12-0x80] */
    "c4e2b8f2ec",  /* andn rbp,r8,rsp */
    "66440f5424c500000000",  /* andpd xmm12,XMMWORD PTR [rax*8+0x0] */
    "66450f5437",  /* andpd xmm14,XMMWORD PTR [r15+0x0] */
    "660f54643d80",  /* andpd xmm4,XMMWORD PTR [rbp+rdi*1-0x80] */
    "c4a260f7244d10000000",  /* bextr esp,DWORD PTR [r9*2+0x10],ebx */
    "c4e268f71d00000000",  /* bextr ebx,DWORD PTR [rip+0x0],edx */
    "64c46218f7042528000000",  /* bextr r8d,DWORD PTR fs:0x28,r12d */
    "0fbc2500000000",  /* bsf esp,DWORD PTR [rip+0x0] */
    "660fbc146d10000000",  /* bsf dx,WORD PTR [rbp*2+0x10] */
    "67660fbcb600010000",  /* bsf si,WORD PTR [esi+0x100] */
    "66440fbd38",  /* bsr r15w,WORD PTR [rax+0x0] */
    "65480fbd467f",  /* bsr rax,QWORD PTR gs:[rsi+0x7f] */
    "64660fbd11",  /* bsr dx,WORD PTR fs:[rcx+0x0] */
    "0fc8",  /* bswap eax */
    "480fba250000000021",  /* bt QWORD PTR [rip+0x0],0x21 */
    "0fba2425341200002e",  /* bt DWORD PTR ds:0x1234,0x2e */
    "490fba64698008",  /* bt QWORD PTR [r9+rbp*2-0x80],0x8 */
    "67660fba3b23",  /* btc WORD PTR [ebx+0x0],0x23 */
    "6466410fba383a",  /* btc WORD PTR fs:[r8],0x3a */
    "490fbafc2d",  /* btc r12,0x2d */
    "4a0fba34e000",  /* btr QWORD PTR [rax+r12*8+0x0],0x0 */
    "67660fba76800f",  /* btr WORD PTR [esi-0x80],0xf */
    "65660fba7424c035",  /* btr WORD PTR gs:[rsp-0x40],0x35 */
    "480fbaec28",  /* bts rsp,0x28 */
    "64410fbaac24000100001f",  /* bts DWORD PTR fs:[r12+0x100],0x1f */
    "420fba2ccd1000000016",  /* bts DWORD PTR [r9*8+0x10],0x16 */
    "4898",  /* cdqe */
    "0f1c0500000000",  /* cldemote BYTE PTR [rip+0x0] */
    "420f1c84bf80000000",  /* cldemote BYTE PTR [rdi+r15*4+0x80] */
    "0f1c04eb",  /* cldemote BYTE PTR [rbx+rbp*8] */
    "410fae3c59",  /* clflush BYTE PTR [r9+rbx*2] */
    "670fae7bc0",  /* clflush BYTE PTR [ebx-0x40] */
    "0faebf78563412",  /* clflush BYTE PTR [rdi+0x12345678] */
    "fa",  /* cli */
    "f3420fae348510000000",  /* clrssbsy QWORD PTR [r8*4+0x10] */
    "f3420fae742e08",  /* clrssbsy QWORD PTR [rsi+r13*1+0x8] */
    "f30fae73c0",  /* clrssbsy QWORD PTR [rbx-0x40] */
    "0f06",  /* clts */
    "65660f47542480",  /* cmova dx,WORD PTR gs:[rsp-0x80] */
    "660f476814",  /* cmova bp,WORD PTR [rax+0x14] */
    "0f472500000000",  /* cmova esp,DWORD PTR [rip+0x0] */
    "64480f4449c0",  /* cmove rcx,QWORD PTR fs:[rcx-0x40] */
    "64480f44342528000000",  /* cmove rsi,QWORD PTR fs:0x28 */
    "66410f4411",  /* cmove dx,WORD PTR [r9] */
    "64440f4c3c2528000000",  /* cmovl r15d,DWORD PTR fs:0x28 */
    "480f4c4108",  /* cmovl rax,QWORD PTR [rcx+0x8] */
    "6566410f4c1f",  /* cmovl bx,WORD PTR gs:[r15+0x0] */
    "66440f49042534120000",  /* cmovns r8w,WORD PTR ds:0x1234 */
    "660f491500000000",  /* cmovns dx,WORD PTR [rip+0x0] */
    "64660f493c2528000000",  /* cmovns di,WORD PTR fs:0x28 */
    "64443abd78563412",  /* cmp r15b,BYTE PTR fs:[rbp+0x12345678] */
    "6642813ce500100000ff00",  /* cmp WORD PTR [r12*8+0x1000],0xff */
    "66833b80",  /* cmp WORD PTR [rbx],0xff80 */
    "66a7",  /* cmps WORD PTR ds:[rsi],WORD PTR es:[rdi] */
    "660fb1f9",  /* cmpxchg cx,di */
    "480fc74cb880",  /* cmpxchg16b OWORD PTR [rax+rdi*4-0x80] */
    "67480fc78800010000",  /* cmpxchg16b OWORD PTR [eax+0x100] */
    "65480fc74d00",  /* cmpxchg16b OWORD PTR gs:[rbp+0x0] */
    "0fc70c5510000000",  /* cmpxchg8b QWORD PTR [rdx*2+0x10] */
    "640fc74ec0",  /* cmpxchg8b QWORD PTR fs:[rsi-0x40] */
    "420fc74caec0",  /* cmpxchg8b QWORD PTR [rsi+r13*4-0x40] */
    "0f2f0d00000000",  /* comiss xmm1,DWORD PTR [rip+0x0] */
    "440f2f1c2534120000",  /* comiss xmm11,DWORD PTR ds:0x1234 */
    "0f2fcf",  /* comiss xmm1,xmm7 */
    "4899",  /* cqo */
    "64f20f38f0142528000000",  /* crc32 edx,BYTE PTR fs:0x28 */
    "f20f38f11d00000000",  /* crc32 ebx,DWORD PTR [rip+0x0] */
    "67f20f38f023",  /* crc32 esp,BYTE PTR [ebx+0x0] */
    "660f2dd2",  /* cvtpd2pi mm2,xmm2 */
    "66410f2d4c5308",  /* cvtpd2pi mm1,XMMWORD PTR [r11+rdx*2+0x8] */
    "66410f2d4080",  /* cvtpd2pi mm0,XMMWORD PTR [r8-0x80] */
    "66440f2ac3",  /* cvtpi2pd xmm8,mm3 */
    "66440f2a3500000000",  /* cvtpi2pd xmm14,QWORD PTR [rip+0x0] */
    "66440f2ae4",  /* cvtpi2pd xmm12,mm4 */
    "440f2add",  /* cvtpi2ps xmm11,mm5 */
    "0f2aba78563412",  /* cvtpi2ps xmm7,QWORD PTR [rdx+0x12345678] */
    "0f2ad1",  /* cvtpi2ps xmm2,mm1 */
    "0f2dc8",  /* cvtps2pi mm1,xmm0 */
    "410f2d8880000000",  /* cvtps2pi mm1,QWORD PTR [r8+0x80] */
    "420f2d244510000000",  /* cvtps2pi mm4,QWORD PTR [r8*2+0x10] */
    "67f20f5a5b14",  /* cvtsd2ss xmm3,QWORD PTR [ebx+0x14] */
    "f2440f5aa45a78563412",  /* cvtsd2ss xmm12,QWORD PTR [rdx+rbx*2+0x12345678] */
    "f2470f5a2c7c",  /* cvtsd2ss xmm13,QWORD PTR [r12+r15*2] */
    "f24a0f2a1ccd10000000",  /* cvtsi2sd xmm3,QWORD PTR [r9*8+0x10] */
    "f24c0f2a5e08",  /* cvtsi2sd xmm11,QWORD PTR [rsi+0x8] */
    "f2420f2a2ccc",  /* cvtsi2sd xmm5,DWORD PTR [rsp+r9*8] */
    "f34c0f2a0cb500000000",  /* cvtsi2ss xmm9,QWORD PTR [rsi*4+0x0] */
    "67f3480f2a4b14",  /* cvtsi2ss xmm1,QWORD PTR [ebx+0x14] */
    "f3480f2ae4",  /* cvtsi2ss xmm4,rsp */
    "65f3440f5a7d14",  /* cvtss2sd xmm15,DWORD PTR gs:[rbp+0x14] */
    "f3440f5a38",  /* cvtss2sd xmm15,DWORD PTR [rax] */
    "67f30f5a4680",  /* cvtss2sd xmm0,DWORD PTR [esi-0x80] */
    "660f2c1500010000",  /* cvttpd2pi mm2,XMMWORD PTR [rip+0x100] */
    "660f2cc3",  /* cvttpd2pi mm0,xmm3 */
    "660f2c34f500100000",  /* cvttpd2pi mm6,XMMWORD PTR [rsi*8+0x1000] */
    "410f2c6cba08",  /* cvttps2pi mm5,QWORD PTR [r10+rdi*4+0x8] */
    "0f2cc0",  /* cvttps2pi mm0,xmm0 */
    "430f2c5c86c0",  /* cvttps2pi mm3,QWORD PTR [r14+r8*4-0x40] */
    "f24c0f2c3d00000000",  /* cvttsd2si r15,QWORD PTR [rip+0x0] */
    "f24c0f2c4c3d00",  /* cvttsd2si r9,QWORD PTR [rbp+rdi*1+0x0] */
    "f24c0f2c46c0",  /* cvttsd2si r8,QWORD PTR [rsi-0x40] */
    "98",  /* cwde */
    "656641ff4d80",  /* dec WORD PTR gs:[r13-0x80] */
    "49ff4c74c0",  /* dec QWORD PTR [r12+rsi*2-0x40] */
    "65ff4f08",  /* dec DWORD PTR gs:[rdi+0x8] */
    "64f7342528000000",  /* div DWORD PTR fs:0x28 */
    "6748f730",  /* div QWORD PTR [eax+0x0] */
    "48f7342534120000",  /* div QWORD PTR ds:0x1234 */
    "f30f1efb",  /* endbr32 */
    "f30f1efa",  /* endbr64 */
    "c8100000",  /* enter 0x10,0x0 */
    "d9f0",  /* f2xm1 */
    "d9e1",  /* fabs */
    "6541dc01",  /* fadd QWORD PTR gs:[r9+0x0] */
    "d845f8",  /* fadd DWORD PTR [rbp-0x8] */
    "41dc4424f8",  /* fadd QWORD PTR [r12-0x8] */
    "dec1",  /* faddp st(1),st */
    "df2500010000",  /* fbld TBYTE PTR [rip+0x100] */
    "df248d10000000",  /* fbld TBYTE PTR [rcx*4+0x10] */
    "42df24c6",  /* fbld TBYTE PTR [rsi+r8*8] */
    "64df342528000000",  /* fbstp TBYTE PTR fs:0x28 */
    "6441df7014",  /* fbstp TBYTE PTR fs:[r8+0x14] */
    "41dfb4f400010000",  /* fbstp TBYTE PTR [r12+rsi*8+0x100] */
    "d9e0",  /* fchs */
    "9bdbe2",  /* fclex */
    "dac0",  /* fcmovb st,st(0) */
    "dad2",  /* fcmovbe st,st(2) */
    "dac9",  /* fcmove st,st(1) */
    "dbc3",  /* fcmovnb st,st(3) */
    "dbd2",  /* fcmovnbe st,st(2) */
    "dbcc",  /* fcmovne st,st(4) */
    "dbd8",  /* fcmovnu st,st(0) */
    "dadb",  /* fcmovu st,st(3) */
    "43dc545bc0",  /* fcom QWORD PTR [r11+r11*2-0x40] */
    "d894ea00f0ffff",  /* fcom DWORD PTR [rdx+rbp*8-0x1000] */
    "64dc1424",  /* fcom QWORD PTR fs:[rsp+0x0] */
    "dbf3",  /* fcomi st,st(3) */
    "dff4",  /* fcomip st,st(4) */
    "42d89cbd00f0ffff",  /* fcomp DWORD PTR [rbp+r15*4-0x1000] */
    "64d81c2528000000",  /* fcomp DWORD PTR fs:0x28 */
    "dc1d00000000",  /* fcomp QWORD PTR [rip+0x0] */
    "ded9",  /* fcompp */
    "d9ff",  /* fcos */
    "d9f6",  /* fdecstp */
    "6541d8717f",  /* fdiv DWORD PTR gs:[r9+0x7f] */
    "d874ebd1",  /* fdiv DWORD PTR [rbx+rbp*8-0x2f] */
    "dc342500000000",  /* fdiv QWORD PTR ds:0x0 */
    "def9",  /* fdivp st(1),st */
    "65d83c24",  /* fdivr DWORD PTR gs:[rsp+0x0] */
    "dc7908",  /* fdivr QWORD PTR [rcx+0x8] */
    "64dc3c2528000000",  /* fdivr QWORD PTR fs:0x28 */
    "def1",  /* fdivrp st(1),st */
    "0f0e",  /* femms */
    "ddc7",  /* ffree st(7) */
    "dfc4",  /* ffreep st(4) */
    "de4180",  /* fiadd WORD PTR [rcx-0x80] */
    "64da4580",  /* fiadd DWORD PTR fs:[rbp-0x80] */
    "41da45c0",  /* fiadd DWORD PTR [r13-0x40] */
    "da10",  /* ficom DWORD PTR [rax] */
    "43da54f180",  /* ficom DWORD PTR [r9+r14*8-0x80] */
    "da1500000000",  /* ficom DWORD PTR [rip+0x0] */
    "42de1c81",  /* ficomp WORD PTR [rcx+r8*4] */
    "42da1cf500100000",  /* ficomp DWORD PTR [r14*8+0x1000] */
    "41da1f",  /* ficomp DWORD PTR [r15] */
    "67de30",  /* fidiv WORD PTR [eax] */
    "41dab40b00f0ffff",  /* fidiv DWORD PTR [r11+rcx*1-0x1000] */
    "41da747208",  /* fidiv DWORD PTR [r10+rsi*2+0x8] */
    "41da7fc0",  /* fidivr DWORD PTR [r15-0x40] */
    "41debc3400100000",  /* fidivr WORD PTR [r12+rsi*1+0x1000] */
    "da3c2534120000",  /* fidivr DWORD PTR ds:0x1234 */
    "db44ba99",  /* fild DWORD PTR [rdx+rdi*4-0x67] */
    "65df8578563412",  /* fild WORD PTR gs:[rbp+0x12345678] */
    "41db8700010000",  /* fild DWORD PTR [r15+0x100] */
    "64da8d00010000",  /* fimul DWORD PTR fs:[rbp+0x100] */
    "da0c2500000000",  /* fimul DWORD PTR ds:0x0 */
    "43da8c1180000000",  /* fimul DWORD PTR [r9+r10*1+0x80] */
    "d9f7",  /* fincstp */
    "9bdbe3",  /* finit */
    "db148500100000",  /* fist DWORD PTR [rax*4+0x1000] */
    "42df945780000000",  /* fist WORD PTR [rdi+r10*2+0x80] */
    "df14f510000000",  /* fist WORD PTR [rsi*8+0x10] */
    "42df3c6510000000",  /* fistp QWORD PTR [r12*2+0x10] */
    "42db1cc500100000",  /* fistp DWORD PTR [r8*8+0x1000] */
    "41db5bc0",  /* fistp DWORD PTR [r11-0x40] */
    "41db8cec80000000",  /* fisttp DWORD PTR [r12+rbp*8+0x80] */
    "dd4cf840",  /* fisttp QWORD PTR [rax+rdi*8+0x40] */
    "41db4c4bc0",  /* fisttp DWORD PTR [r11+rcx*2-0x40] */
    "da6540",  /* fisub DWORD PTR [rbp+0x40] */
    "43da6452c0",  /* fisub DWORD PTR [r10+r10*2-0x40] */
    "dea178563412",  /* fisub WORD PTR [rcx+0x12345678] */
    "41da6c85c0",  /* fisubr DWORD PTR [r13+rax*4-0x40] */
    "42de2c4510000000",  /* fisubr WORD PTR [r8*2+0x10] */
    "64da2c2528000000",  /* fisubr DWORD PTR fs:0x28 */
    "6541d947c0",  /* fld DWORD PTR gs:[r15-0x40] */
    "41d901",  /* fld DWORD PTR [r9] */
    "41d94680",  /* fld DWORD PTR [r14-0x80] */
    "d9e8",  /* fld1 */
    "d92f",  /* fldcw WORD PTR [rdi] */
    "d92cb500100000",  /* fldcw WORD PTR [rsi*4+0x1000] */
    "43d9acb880000000",  /* fldcw WORD PTR [r8+r15*4+0x80] */
    "d9245500100000",  /* fldenv [rdx*2+0x1000] */
    "d9a4b880000000",  /* fldenv [rax+rdi*4+0x80] */
    "d925f8ffffff",  /* fldenv [rip+0xfffffffffffffff8] */
    "d9ea",  /* fldl2e */
    "d9e9",  /* fldl2t */
    "d9ec",  /* fldlg2 */
    "d9ed",  /* fldln2 */
    "d9eb",  /* fldpi */
    "d9ee",  /* fldz */
    "d80d00000000",  /* fmul DWORD PTR [rip+0x0] */
    "65d84f14",  /* fmul DWORD PTR gs:[rdi+0x14] */
    "dc0c2534120000",  /* fmul QWORD PTR ds:0x1234 */
    "dec9",  /* fmulp st(1),st */
    "dbe2",  /* fnclex */
    "dbe3",  /* fninit */
    "d9d0",  /* fnop */
    "41dd76c0",  /* fnsave [r14-0x40] */
    "41dd74bac0",  /* fnsave [r10+rdi*4-0x40] */
    "41ddb42480000000",  /* fnsave [r12+0x80] */
    "d93d00000000",  /* fnstcw WORD PTR [rip+0x0] */
    "d93cf510000000",  /* fnstcw WORD PTR [rsi*8+0x10] */
    "43d9bca200010000",  /* fnstcw WORD PTR [r10+r12*4+0x100] */
    "43d9b43278563412",  /* fnstenv [r10+r14*1+0x12345678] */
    "42d9348510000000",  /* fnstenv [r8*4+0x10] */
    "d9b300f0ffff",  /* fnstenv [rbx-0x1000] */
    "41dd3c01",  /* fnstsw WORD PTR [r9+rax*1+0x0] */
    "dd38",  /* fnstsw WORD PTR [rax] */
    "6541ddbc2478563412",  /* fnstsw WORD PTR gs:[r12+0x12345678] */
    "d9f3",  /* fpatan */
    "d9f8",  /* fprem */
    "d9f5",  /* fprem1 */
    "d9f2",  /* fptan */
    "d9fc",  /* frndint */
    "41dda4f700f0ffff",  /* frstor [r15+rsi*8-0x1000] */
    "dd244d00100000",  /* frstor [rcx*2+0x1000] */
    "41dd22",  /* frstor [r10] */
    "9bdd7740",  /* fsave [rdi+0x40] */
    "9bdd748e08",  /* fsave [rsi+rcx*4+0x8] */
    "9bdd74f9c0",  /* fsave [rcx+rdi*8-0x40] */
    "d9fd",  /* fscale */
    "d9fe",  /* fsin */
    "d9fb",  /* fsincos */
    "d9fa",  /* fsqrt */
    "41d95780",  /* fst DWORD PTR [r15-0x80] */
    "41dd94f0d1000000",  /* fst QWORD PTR [r8+rsi*8+0xd1] */
    "d914ed10000000",  /* fst DWORD PTR [rbp*8+0x10] */
    "9b41d9b800100000",  /* fstcw WORD PTR [r8+0x1000] */
    "9b42d93c6d00100000",  /* fstcw WORD PTR [r13*2+0x1000] */
    "9b42d9bc1580000000",  /* fstcw WORD PTR [rbp+r10*1+0x80] */
    "9bd9b080000000",  /* fstenv [rax+0x80] */
    "9b43d9740808",  /* fstenv [r8+r9*1+0x8] */
    "9b42d9340f",  /* fstenv [rdi+r9*1] */
    "d91c4b",  /* fstp DWORD PTR [rbx+rcx*2] */
    "41dd5cfe39",  /* fstp QWORD PTR [r14+rdi*8+0x39] */
    "dd19",  /* fstp QWORD PTR [rcx] */
    "9bdd3d00000000",  /* fstsw WORD PTR [rip+0x0] */
    "9b41dd7d80",  /* fstsw WORD PTR [r13-0x80] */
    "9b42ddbc7900100000",  /* fstsw WORD PTR [rcx+r15*2+0x1000] */
    "d824d7",  /* fsub DWORD PTR [rdi+rdx*8] */
    "d8647835",  /* fsub DWORD PTR [rax+rdi*2+0x35] */
    "dc242534120000",  /* fsub QWORD PTR ds:0x1234 */
    "dee9",  /* fsubp st(1),st */
    "41d828",  /* fsubr DWORD PTR [r8] */
    "41dc2c24",  /* fsubr QWORD PTR [r12] */
    "d8a900010000",  /* fsubr DWORD PTR [rcx+0x100] */
    "dee1",  /* fsubrp st(1),st */
    "d9e4",  /* ftst */
    "dde1",  /* fucom st(1) */
    "dbeb",  /* fucomi st,st(3) */
    "dfed",  /* fucomip st,st(5) */
    "dde9",  /* fucomp st(1) */
    "dae9",  /* fucompp */
    "d9e5",  /* fxam */
    "d9cc",  /* fxch st(4) */
    "0fae4980",  /* fxrstor [rcx-0x80] */
    "410fae8c4a00f0ffff",  /* fxrstor [r10+rcx*2-0x1000] */
    "0fae0df8ffffff",  /* fxrstor [rip+0xfffffffffffffff8] */
    "480fae0df8ffffff",  /* fxrstor64 [rip+0xfffffffffffffff8] */
    "4a0fae0cbd00100000",  /* fxrstor64 [r15*4+0x1000] */
    "490fae4d08",  /* fxrstor64 [r13+0x8] */
    "0fae8178563412",  /* fxsave [rcx+0x12345678] */
    "420fae84d578563412",  /* fxsave [rbp+r10*8+0x12345678] */
    "0fae042500000000",  /* fxsave ds:0x0 */
    "480fae06",  /* fxsave64 [rsi] */
    "480fae05f8ffffff",  /* fxsave64 [rip+0xfffffffffffffff8] */
    "480fae4500",  /* fxsave64 [rbp+0x0] */
    "d9f4",  /* fxtract */
    "d9f1",  /* fyl2x */
    "d9f9",  /* fyl2xp1 */
    "660f3acfff96",  /* gf2p8affineinvqb xmm7,xmm7,0x96 */
    "66410f3acf74f68037",  /* gf2p8affineinvqb xmm6,XMMWORD PTR [r14+rsi*8-0x80],0x37 */
    "66440f3acf938000000077",  /* gf2p8affineinvqb xmm10,XMMWORD PTR [rbx+0x80],0x77 */
    "66420f3ace8c0200100000d1",  /* gf2p8affineqb xmm1,XMMWORD PTR [rdx+r8*1+0x1000],0xd1 */
    "66410f3acef481",  /* gf2p8affineqb xmm6,xmm12,0x81 */
    "66440f3ace048344",  /* gf2p8affineqb xmm8,XMMWORD PTR [rbx+rax*4],0x44 */
    "66440f38cf049d10000000",  /* gf2p8mulb xmm8,XMMWORD PTR [rbx*4+0x10] */
    "66440f38cfb000010000",  /* gf2p8mulb xmm14,XMMWORD PTR [rax+0x100] */
    "66440f38cf25f8ffffff",  /* gf2p8mulb xmm12,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "f4",  /* hlt */
    "64f77880",  /* idiv DWORD PTR fs:[rax-0x80] */
    "f67f7f",  /* idiv BYTE PTR [rdi+0x7f] */
    "42f73c6500000000",  /* idiv DWORD PTR [r12*2+0x0] */
    "660faf1c2534120000",  /* imul bx,WORD PTR ds:0x1234 */
    "6766f76e08",  /* imul WORD PTR [esi+0x8] */
    "f62d00000000",  /* imul BYTE PTR [rip+0x0] */
    "ec",  /* in al,dx */
    "e4d0",  /* in al,0xd0 */
    "6643ff44a814",  /* inc WORD PTR [r8+r13*4+0x14] */
    "6541fe8100010000",  /* inc BYTE PTR gs:[r9+0x100] */
    "48ff042500000000",  /* inc QWORD PTR ds:0x0 */
    "666d",  /* ins WORD PTR es:[rdi],dx */
    "6c",  /* ins BYTE PTR es:[rdi],dx */
    "6d",  /* ins DWORD PTR es:[rdi],dx */
    "cd80",  /* int 0x80 */
    "cc",  /* int3 */
    "0f08",  /* invd */
    "0f013d00010000",  /* invlpg BYTE PTR [rip+0x100] */
    "410f017cef40",  /* invlpg BYTE PTR [r15+rbp*8+0x40] */
    "420f013c8510000000",  /* invlpg BYTE PTR [r8*4+0x10] */
    "48cf",  /* iretq */
    "66cf",  /* iretw */
    "c5dd4ac8",  /* kaddb k1,k4,k0 */
    "c5ed4af0",  /* kaddb k6,k2,k0 */
    "c5ed4af1",  /* kaddb k6,k2,k1 */
    "c4e1ed4ae8",  /* kaddd k5,k2,k0 */
    "c4e1d54ac9",  /* kaddd k1,k5,k1 */
    "c4e1cd4ae9",  /* kaddd k5,k6,k1 */
    "c4e1cc4ac6",  /* kaddq k0,k6,k6 */
    "c4e1e44af4",  /* kaddq k6,k3,k4 */
    "c4e1fc4ac0",  /* kaddq k0,k0,k0 */
    "c5dc4ae5",  /* kaddw k4,k4,k5 */
    "c5f44ac4",  /* kaddw k0,k1,k4 */
    "c5d44af8",  /* kaddw k7,k5,k0 */
    "c5f541d7",  /* kandb k2,k1,k7 */
    "c5d541e4",  /* kandb k4,k5,k4 */
    "c5dd41fb",  /* kandb k7,k4,k3 */
    "c4e1d541d2",  /* kandd k2,k5,k2 */
    "c4e1cd41cb",  /* kandd k1,k6,k3 */
    "c4e1d541dd",  /* kandd k3,k5,k5 */
    "c5e542c7",  /* kandnb k0,k3,k7 */
    "c5cd42d6",  /* kandnb k2,k6,k6 */
    "c5c542f4",  /* kandnb k6,k7,k4 */
    "c4e1dd42d0",  /* kandnd k2,k4,k0 */
    "c4e1ed42d8",  /* kandnd k3,k2,k0 */
    "c4e1cd42c8",  /* kandnd k1,k6,k0 */
    "c4e1cc42e1",  /* kandnq k4,k6,k1 */
    "c4e1ec42c4",  /* kandnq k0,k2,k4 */
    "c4e1fc42d5",  /* kandnq k2,k0,k5 */
    "c5f442c8",  /* kandnw k1,k1,k0 */
    "c5f442c7",  /* kandnw k0,k1,k7 */
    "c5dc42df",  /* kandnw k3,k4,k7 */
    "c4e1ec41c9",  /* kandq k1,k2,k1 */
    "c4e1cc41cb",  /* kandq k1,k6,k3 */
    "c4e1ec41ee",  /* kandq k5,k2,k6 */
    "c5d441c3",  /* kandw k0,k5,k3 */
    "c5e441d9",  /* kandw k3,k3,k1 */
    "c5d441f2",  /* kandw k6,k5,k2 */
    "c5f990f6",  /* kmovb k6,k6 */
    "c5f99025f8ffffff",  /* kmovb k4,BYTE PTR [rip+0xfffffffffffffff8] */
    "c5f990f2",  /* kmovb k6,k2 */
    "c4e1f990fd",  /* kmovd k7,k5 */
    "c4e1f9901d00010000",  /* kmovd k3,DWORD PTR [rip+0x100] */
    "c4e1f991be78563412",  /* kmovd DWORD PTR [rsi+0x12345678],k7 */
    "c4e1fb92c6",  /* kmovq k0,rsi */
    "c4e1f890c5",  /* kmovq k0,k5 */
    "c4e1fb93ef",  /* kmovq rbp,k7 */
    "c5f890fb",  /* kmovw k7,k3 */
    "c5f892ef",  /* kmovw k5,edi */
    "c5f890e1",  /* kmovw k4,k1 */
    "c5f944cf",  /* knotb k1,k7 */
    "c5f944d1",  /* knotb k2,k1 */
    "c5f944d7",  /* knotb k2,k7 */
    "c4e1f944de",  /* knotd k3,k6 */
    "c4e1f944d6",  /* knotd k2,k6 */
    "c4e1f944ee",  /* knotd k5,k6 */
    "c4e1f844e9",  /* knotq k5,k1 */
    "c4e1f844c7",  /* knotq k0,k7 */
    "c4e1f844fc",  /* knotq k7,k4 */
    "c5f844d8",  /* knotw k3,k0 */
    "c5f844e0",  /* knotw k4,k0 */
    "c5f844e1",  /* knotw k4,k1 */
    "c5f545e2",  /* korb k4,k1,k2 */
    "c5fd45cc",  /* korb k1,k0,k4 */
    "c5d545d3",  /* korb k2,k5,k3 */
    "c4e1e545d5",  /* kord k2,k3,k5 */
    "c4e1fd45f0",  /* kord k6,k0,k0 */
    "c4e1cd45c1",  /* kord k0,k6,k1 */
    "c4e1e445e2",  /* korq k4,k3,k2 */
    "c4e1ec45d1",  /* korq k2,k2,k1 */
    "c4e1f445fa",  /* korq k7,k1,k2 */
    "c5f998ef",  /* kortestb k5,k7 */
    "c5f998c6",  /* kortestb k0,k6 */
    "c5f998e3",  /* kortestb k4,k3 */
    "c4e1f998e2",  /* kortestd k4,k2 */
    "c4e1f998de",  /* kortestd k3,k6 */
    "c4e1f998df",  /* kortestd k3,k7 */
    "c4e1f898f5",  /* kortestq k6,k5 */
    "c4e1f898c5",  /* kortestq k0,k5 */
    "c4e1f898e4",  /* kortestq k4,k4 */
    "c5f898db",  /* kortestw k3,k3 */
    "c5f898eb",  /* kortestw k5,k3 */
    "c5f898cc",  /* kortestw k1,k4 */
    "c5c445d7",  /* korw k2,k7,k7 */
    "c5c445ee",  /* korw k5,k7,k6 */
    "c5cc45ff",  /* korw k7,k6,k7 */
    "c5f999c6",  /* ktestb k0,k6 */
    "c5f999cc",  /* ktestb k1,k4 */
    "c5f999f5",  /* ktestb k6,k5 */
    "c4e1f999ef",  /* ktestd k5,k7 */
    "c4e1f999d8",  /* ktestd k3,k0 */
    "c4e1f999c8",  /* ktestd k1,k0 */
    "c4e1f899f1",  /* ktestq k6,k1 */
    "c4e1f899d8",  /* ktestq k3,k0 */
    "c4e1f899f6",  /* ktestq k6,k6 */
    "c5f899ca",  /* ktestw k1,k2 */
    "c5f899ef",  /* ktestw k5,k7 */
    "c5f899d0",  /* ktestw k2,k0 */
    "c5ed46c0",  /* kxnorb k0,k2,k0 */
    "c5d546e5",  /* kxnorb k4,k5,k5 */
    "c5d546cc",  /* kxnorb k1,k5,k4 */
    "c4e1dd46f5",  /* kxnord k6,k4,k5 */
    "c4e1d546c3",  /* kxnord k0,k5,k3 */
    "c4e1d546c8",  /* kxnord k1,k5,k0 */
    "c4e1ec46ff",  /* kxnorq k7,k2,k7 */
    "c4e1f446f4",  /* kxnorq k6,k1,k4 */
    "c4e1d446c0",  /* kxnorq k0,k5,k0 */
    "c5e446db",  /* kxnorw k3,k3,k3 */
    "c5c446c4",  /* kxnorw k0,k7,k4 */
    "c5cc46d3",  /* kxnorw k2,k6,k3 */
    "c5c547cb",  /* kxorb k1,k7,k3 */
    "c5dd47d5",  /* kxorb k2,k4,k5 */
    "c5fd47ea",  /* kxorb k5,k0,k2 */
    "c4e1dd47f0",  /* kxord k6,k4,k0 */
    "c4e1cd47da",  /* kxord k3,k6,k2 */
    "c4e1cd47fc",  /* kxord k7,k6,k4 */
    "c4e1e447fd",  /* kxorq k7,k3,k5 */
    "c4e1dc47e7",  /* kxorq k4,k4,k7 */
    "c4e1fc47e8",  /* kxorq k5,k0,k0 */
    "c5fc47ca",  /* kxorw k1,k0,k2 */
    "c5e447db",  /* kxorw k3,k3,k3 */
    "c5dc47dd",  /* kxorw k3,k4,k5 */
    "640fae142528000000",  /* ldmxcsr DWORD PTR fs:0x28 */
    "650fae5424c0",  /* ldmxcsr DWORD PTR gs:[rsp-0x40] */
    "0fae142500000000",  /* ldmxcsr DWORD PTR ds:0x0 */
    "678d20",  /* lea esp,[eax] */
    "4c8d641dc0",  /* lea r12,[rbp+rbx*1-0x40] */
    "488d607f",  /* lea rsp,[rax+0x7f] */
    "c9",  /* leave */
    "0f019700f0ffff",  /* lgdt [rdi-0x1000] */
    "0f0113",  /* lgdt [rbx] */
    "410f01941f78563412",  /* lgdt [r15+rbx*1+0x12345678] */
    "0f011cad00100000",  /* lidt [rbp*4+0x1000] */
    "410f015a08",  /* lidt [r10+0x8] */
    "0f011d00000000",  /* lidt [rip+0x0] */
    "410f009000100000",  /* lldt WORD PTR [r8+0x1000] */
    "420f00146500100000",  /* lldt WORD PTR [r12*2+0x1000] */
    "430f00940978563412",  /* lldt WORD PTR [r9+r9*1+0x12345678] */
    "0f0136",  /* lmsw WORD PTR [rsi] */
    "0f013500000000",  /* lmsw WORD PTR [rip+0x0] */
    "410f01f0",  /* lmsw r8w */
    "64f0830301",  /* lock add DWORD PTR fs:[rbx],0x1 */
    "f083430801",  /* lock add DWORD PTR [rbx+0x8],0x1 */
    "f04183047701",  /* lock add DWORD PTR [r15+rsi*2],0x1 */
    "ad",  /* lods eax,DWORD PTR ds:[rsi] */
    "420f001c7510000000",  /* ltr WORD PTR [r14*2+0x10] */
    "0f009c5600010000",  /* ltr WORD PTR [rsi+rdx*2+0x100] */
    "0f001d00010000",  /* ltr WORD PTR [rip+0x100] */
    "f34e0fbd248d00100000",  /* lzcnt r12,QWORD PTR [r9*4+0x1000] */
    "6466f30fbd4e80",  /* lzcnt cx,WORD PTR fs:[rsi-0x80] */
    "65f3410fbd1424",  /* lzcnt edx,DWORD PTR gs:[r12+0x0] */
    "0ff7d3",  /* maskmovq mm2,mm3 */
    "0ff7f5",  /* maskmovq mm6,mm5 */
    "0ff7f2",  /* maskmovq mm6,mm2 */
    "0f01c8",  /* monitor */
    "0f23fa",  /* mov dr7,rdx */
    "8b1cbd00100000",  /* mov ebx,DWORD PTR [rdi*4+0x1000] */
    "45884500",  /* mov BYTE PTR [r13+0x0],r8b */
    "48b88877665544332211",  /* movabs rax,0x1122334455667788 */
    "0f285a08",  /* movaps xmm3,XMMWORD PTR [rdx+0x8] */
    "0f28042534120000",  /* movaps xmm0,XMMWORD PTR ds:0x1234 */
    "0f284cd97f",  /* movaps xmm1,XMMWORD PTR [rcx+rbx*8+0x7f] */
    "650f38f0af78563412",  /* movbe ebp,DWORD PTR gs:[rdi+0x12345678] */
    "0f38f01500000000",  /* movbe edx,DWORD PTR [rip+0x0] */
    "420f38f00ccd10000000",  /* movbe ecx,DWORD PTR [r9*8+0x10] */
    "67660f6e36",  /* movd xmm6,DWORD PTR [esi] */
    "0f6e58c0",  /* movd mm3,DWORD PTR [rax-0x40] */
    "660f6e6500",  /* movd xmm4,DWORD PTR [rbp+0x0] */
    "f2410fd6fc",  /* movdq2q mm7,xmm12 */
    "f20fd6ea",  /* movdq2q mm5,xmm2 */
    "f2410fd6e4",  /* movdq2q mm4,xmm12 */
    "66440f6f2d00000000",  /* movdqa xmm13,XMMWORD PTR [rip+0x0] */
    "67660f6f4bc0",  /* movdqa xmm1,XMMWORD PTR [ebx-0x40] */
    "660f6f1c2534120000",  /* movdqa xmm3,XMMWORD PTR ds:0x1234 */
    "f30f6f54d2c0",  /* movdqu xmm2,XMMWORD PTR [rdx+rdx*8-0x40] */
    "f30f6f242500000000",  /* movdqu xmm4,XMMWORD PTR ds:0x0 */
    "67f30f6f43c0",  /* movdqu xmm0,XMMWORD PTR [ebx-0x40] */
    "480fc331",  /* movnti QWORD PTR [rcx],rsi */
    "644c0fc37d80",  /* movnti QWORD PTR fs:[rbp-0x80],r15 */
    "67480fc366c0",  /* movnti QWORD PTR [esi-0x40],rsp */
    "420fe7a46800f0ffff",  /* movntq QWORD PTR [rax+r13*2-0x1000],mm4 */
    "420fe77cc240",  /* movntq QWORD PTR [rdx+r8*8+0x40],mm7 */
    "420fe7247d00100000",  /* movntq QWORD PTR [r15*2+0x1000],mm4 */
    "490f7efd",  /* movq r13,mm7 */
    "0f6fc1",  /* movq mm0,mm1 */
    "0f6fe6",  /* movq mm4,mm6 */
    "f3440fd6c0",  /* movq2dq xmm8,mm0 */
    "f30fd6ce",  /* movq2dq xmm1,mm6 */
    "f30fd6d1",  /* movq2dq xmm2,mm1 */
    "48a5",  /* movs QWORD PTR es:[rdi],QWORD PTR ds:[rsi] */
    "64f2410f1019",  /* movsd xmm3,QWORD PTR fs:[r9+0x0] */
    "65f2450f102c24",  /* movsd xmm13,QWORD PTR gs:[r12+0x0] */
    "f2410f10c7",  /* movsd xmm0,xmm15 */
    "f3420f1034cd00100000",  /* movss xmm6,DWORD PTR [r9*8+0x1000] */
    "67f30f1076c0",  /* movss xmm6,DWORD PTR [esi-0x40] */
    "f30f1033",  /* movss xmm6,DWORD PTR [rbx] */
    "0fbfe0",  /* movsx esp,ax */
    "6566410fbe4180",  /* movsx ax,BYTE PTR gs:[r9-0x80] */
    "0fbf142500000000",  /* movsx edx,WORD PTR ds:0x0 */
    "67486300",  /* movsxd rax,DWORD PTR [eax] */
    "6448632c2528000000",  /* movsxd rbp,DWORD PTR fs:0x28 */
    "674c634014",  /* movsxd r8,DWORD PTR [eax+0x14] */
    "0f10248500000000",  /* movups xmm4,XMMWORD PTR [rax*4+0x0] */
    "67440f1038",  /* movups xmm15,XMMWORD PTR [eax] */
    "440f101468",  /* movups xmm10,XMMWORD PTR [rax+rbp*2+0x0] */
    "65480fb707",  /* movzx rax,WORD PTR gs:[rdi+0x0] */
    "640fb6677f",  /* movzx esp,BYTE PTR fs:[rdi+0x7f] */
    "640fb60c2528000000",  /* movzx ecx,BYTE PTR fs:0x28 */
    "67f626",  /* mul BYTE PTR [esi] */
    "48f7242534120000",  /* mul QWORD PTR ds:0x1234 */
    "65f66424c0",  /* mul BYTE PTR gs:[rsp-0x40] */
    "440f59149d00000000",  /* mulps xmm10,XMMWORD PTR [rbx*4+0x0] */
    "670f597e80",  /* mulps xmm7,XMMWORD PTR [esi-0x80] */
    "440f59042500000000",  /* mulps xmm8,XMMWORD PTR ds:0x0 */
    "0f01c9",  /* mwait */
    "6641f71c04",  /* neg WORD PTR [r12+rax*1] */
    "48f75c697f",  /* neg QWORD PTR [rcx+rbp*2+0x7f] */
    "f75cebc0",  /* neg DWORD PTR [rbx+rbp*8-0x40] */
    "90",  /* nop */
    "660f1f0400",  /* nop WORD PTR [rax+rax*1] */
    "0f1f00",  /* nop DWORD PTR [rax] */
    "6766f756c0",  /* not WORD PTR [esi-0x40] */
    "6748f75008",  /* not QWORD PTR [eax+0x8] */
    "f71500000000",  /* not DWORD PTR [rip+0x0] */
    "640b10",  /* or edx,DWORD PTR fs:[rax+0x0] */
    "6441804fc080",  /* or BYTE PTR fs:[r15-0x40],0x80 */
    "6466440b39",  /* or r15w,WORD PTR fs:[rcx] */
    "ee",  /* out dx,al */
    "e6ab",  /* out 0xab,al */
    "666f",  /* outs dx,WORD PTR ds:[rsi] */
    "6e",  /* outs dx,BYTE PTR ds:[rsi] */
    "6f",  /* outs dx,DWORD PTR ds:[rsi] */
    "0f381cdd",  /* pabsb mm3,mm5 */
    "0f381c05f8ffffff",  /* pabsb mm0,QWORD PTR [rip+0xfffffffffffffff8] */
    "410f381c5340",  /* pabsb mm2,QWORD PTR [r11+0x40] */
    "0f381ecc",  /* pabsd mm1,mm4 */
    "410f381e7680",  /* pabsd mm6,QWORD PTR [r14-0x80] */
    "0f381e25f8ffffff",  /* pabsd mm4,QWORD PTR [rip+0xfffffffffffffff8] */
    "430f381d543f40",  /* pabsw mm2,QWORD PTR [r15+r15*1+0x40] */
    "0f381dd4",  /* pabsw mm2,mm4 */
    "410f381db300100000",  /* pabsw mm6,QWORD PTR [r11+0x1000] */
    "410f6ba100100000",  /* packssdw mm4,QWORD PTR [r9+0x1000] */
    "410f6b8c2400f0ffff",  /* packssdw mm1,QWORD PTR [r12-0x1000] */
    "0f6bf8",  /* packssdw mm7,mm0 */
    "0f635540",  /* packsswb mm2,QWORD PTR [rbp+0x40] */
    "0f63fa",  /* packsswb mm7,mm2 */
    "0f633c4d00100000",  /* packsswb mm7,QWORD PTR [rcx*2+0x1000] */
    "0f673df8ffffff",  /* packuswb mm7,QWORD PTR [rip+0xfffffffffffffff8] */
    "0f67d7",  /* packuswb mm2,mm7 */
    "410f6750c0",  /* packuswb mm2,QWORD PTR [r8-0x40] */
    "0ffcd3",  /* paddb mm2,mm3 */
    "0ffc5df8",  /* paddb mm3,QWORD PTR [rbp-0x8] */
    "0ffce8",  /* paddb mm5,mm0 */
    "0ffed0",  /* paddd mm2,mm0 */
    "660ffea378563412",  /* paddd xmm4,XMMWORD PTR [rbx+0x12345678] */
    "0ffef8",  /* paddd mm7,mm0 */
    "0fd4cc",  /* paddq mm1,mm4 */
    "410fd49c6805ffffff",  /* paddq mm3,QWORD PTR [r8+rbp*2-0xfb] */
    "0fd4cd",  /* paddq mm1,mm5 */
    "0fecf8",  /* paddsb mm7,mm0 */
    "0fec0c4d00100000",  /* paddsb mm1,QWORD PTR [rcx*2+0x1000] */
    "0fecd8",  /* paddsb mm3,mm0 */
    "0fedcc",  /* paddsw mm1,mm4 */
    "0fedc6",  /* paddsw mm0,mm6 */
    "66410feda4ff00f0ffff",  /* paddsw xmm4,XMMWORD PTR [r15+rdi*8-0x1000] */
    "0fdc3c9d00100000",  /* paddusb mm7,QWORD PTR [rbx*4+0x1000] */
    "0fdcdd",  /* paddusb mm3,mm5 */
    "420fdc0c4d00100000",  /* paddusb mm1,QWORD PTR [r9*2+0x1000] */
    "0fddc9",  /* paddusw mm1,mm1 */
    "66430fdd1483",  /* paddusw xmm2,XMMWORD PTR [r11+r8*4] */
    "430fdd54cb40",  /* paddusw mm2,QWORD PTR [r11+r9*8+0x40] */
    "0ffdea",  /* paddw mm5,mm2 */
    "0ffde6",  /* paddw mm4,mm6 */
    "0ffd9800010000",  /* paddw mm3,QWORD PTR [rax+0x100] */
    "0f3a0fba78563412a9",  /* palignr mm7,QWORD PTR [rdx+0x12345678],0xa9 */
    "410f3a0f4080f3",  /* palignr mm0,QWORD PTR [r8-0x80],0xf3 */
    "0f3a0fbc5d0010000033",  /* palignr mm7,QWORD PTR [rbp+rbx*2+0x1000],0x33 */
    "0fdba000010000",  /* pand mm4,QWORD PTR [rax+0x100] */
    "660fdb54df40",  /* pand xmm2,XMMWORD PTR [rdi+rbx*8+0x40] */
    "0fdbd8",  /* pand mm3,mm0 */
    "0fdfef",  /* pandn mm5,mm7 */
    "0fdf0500010000",  /* pandn mm0,QWORD PTR [rip+0x100] */
    "410fdfac2400f0ffff",  /* pandn mm5,QWORD PTR [r12-0x1000] */
    "430fe03414",  /* pavgb mm6,QWORD PTR [r12+r10*1] */
    "66450fe0f1",  /* pavgb xmm14,xmm9 */
    "66440fe02df8ffffff",  /* pavgb xmm13,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "410fe39300010000",  /* pavgw mm2,QWORD PTR [r11+0x100] */
    "430fe34498c0",  /* pavgw mm0,QWORD PTR [r8+r11*4-0x40] */
    "410fe3a600010000",  /* pavgw mm4,QWORD PTR [r14+0x100] */
    "66440f3a0e350000000005",  /* pblendw xmm14,XMMWORD PTR [rip+0x0],0x5 */
    "66450f3a0e6c478005",  /* pblendw xmm13,XMMWORD PTR [r15+rax*2-0x80],0x5 */
    "660f3a0e24253412000005",  /* pblendw xmm4,XMMWORD PTR ds:0x1234,0x5 */
    "66450f3a44c211",  /* pclmulhqhqdq xmm8,xmm10 */
    "66460f3a443c650010000011",  /* pclmulhqhqdq xmm15,XMMWORD PTR [r12*2+0x1000] */
    "66410f3a44ab7856341211",  /* pclmulhqhqdq xmm5,XMMWORD PTR [r11+0x12345678] */
    "66410f3a44ed01",  /* pclmulhqlqdq xmm5,xmm13 */
    "66410f3a44bc2400f0ffff01",  /* pclmulhqlqdq xmm7,XMMWORD PTR [r12-0x1000] */
    "66450f3a44bc198000000001",  /* pclmulhqlqdq xmm15,XMMWORD PTR [r9+rbx*1+0x80] */
    "660f3a44b70010000010",  /* pclmullqhqdq xmm6,XMMWORD PTR [rdi+0x1000] */
    "660f3a44fc10",  /* pclmullqhqdq xmm7,xmm4 */
    "66460f3a440ca50010000010",  /* pclmullqhqdq xmm9,XMMWORD PTR [r12*4+0x1000] */
    "66420f3a447c314000",  /* pclmullqlqdq xmm7,XMMWORD PTR [rcx+r14*1+0x40] */
    "66410f3a44ef00",  /* pclmullqlqdq xmm5,xmm15 */
    "66410f3a4444958000",  /* pclmullqlqdq xmm0,XMMWORD PTR [r13+rdx*4-0x80] */
    "0f74d8",  /* pcmpeqb mm3,mm0 */
    "0f74c8",  /* pcmpeqb mm1,mm0 */
    "66440f742453",  /* pcmpeqb xmm12,XMMWORD PTR [rbx+rdx*2] */
    "660f76ee",  /* pcmpeqd xmm5,xmm6 */
    "0f76f2",  /* pcmpeqd mm6,mm2 */
    "0f76c7",  /* pcmpeqd mm0,mm7 */
    "66450f38298700f0ffff",  /* pcmpeqq xmm8,XMMWORD PTR [r15-0x1000] */
    "660f3829ec",  /* pcmpeqq xmm5,xmm4 */
    "66440f38293cbd10000000",  /* pcmpeqq xmm15,XMMWORD PTR [rdi*4+0x10] */
    "0f75fa",  /* pcmpeqw mm7,mm2 */
    "430f75840578563412",  /* pcmpeqw mm0,QWORD PTR [r13+r8*1+0x12345678] */
    "0f75e1",  /* pcmpeqw mm4,mm1 */
    "66450f3a6184470010000056",  /* pcmpestri xmm8,XMMWORD PTR [r15+rax*2+0x1000],0x56 */
    "66440f3a613d000000009c",  /* pcmpestri xmm15,XMMWORD PTR [rip+0x0],0x9c */
    "66460f3a6134ed10000000a1",  /* pcmpestri xmm14,XMMWORD PTR [r13*8+0x10],0xa1 */
    "66450f3a60e0f7",  /* pcmpestrm xmm12,xmm8,0xf7 */
    "66440f3a602d000000008d",  /* pcmpestrm xmm13,XMMWORD PTR [rip+0x0],0x8d */
    "66440f3a60b67856341283",  /* pcmpestrm xmm14,XMMWORD PTR [rsi+0x12345678],0x83 */
    "410f6457c0",  /* pcmpgtb mm2,QWORD PTR [r15-0x40] */
    "0f64e7",  /* pcmpgtb mm4,mm7 */
    "660f641cad00100000",  /* pcmpgtb xmm3,XMMWORD PTR [rbp*4+0x1000] */
    "0f66ce",  /* pcmpgtd mm1,mm6 */
    "430f66a42178563412",  /* pcmpgtd mm4,QWORD PTR [r9+r12*1+0x12345678] */
    "0f66cc",  /* pcmpgtd mm1,mm4 */
    "66420f38373c6d10000000",  /* pcmpgtq xmm7,XMMWORD PTR [r13*2+0x10] */
    "66450f3837dc",  /* pcmpgtq xmm11,xmm12 */
    "66460f3837543a08",  /* pcmpgtq xmm10,XMMWORD PTR [rdx+r15*1+0x8] */
    "430f6594e100010000",  /* pcmpgtw mm2,QWORD PTR [r9+r12*8+0x100] */
    "0f65d8",  /* pcmpgtw mm3,mm0 */
    "0f65c7",  /* pcmpgtw mm0,mm7 */
    "66440f3a63bcf200f0ffffe1",  /* pcmpistri xmm15,XMMWORD PTR [rdx+rsi*8-0x1000],0xe1 */
    "66460f3a63849a800000004c",  /* pcmpistri xmm8,XMMWORD PTR [rdx+r11*4+0x80],0x4c */
    "66450f3a63f000",  /* pcmpistri xmm14,xmm8,0x0 */
    "66420f3a620c7d10000000a2",  /* pcmpistrm xmm1,XMMWORD PTR [r15*2+0x10],0xa2 */
    "66430f3a626c928070",  /* pcmpistrm xmm5,XMMWORD PTR [r10+r10*4-0x80],0x70 */
    "66440f3a62bb00010000ee",  /* pcmpistrm xmm15,XMMWORD PTR [rbx+0x100],0xee */
    "c4e273f5d8",  /* pdep ebx,ecx,eax */
    "0fc5f82e",  /* pextrw edi,mm0,0x2e */
    "440fc5f2d6",  /* pextrw r14d,mm2,0xd6 */
    "66410fc5fa01",  /* pextrw edi,xmm10,0x1 */
    "0f380215f8ffffff",  /* phaddd mm2,QWORD PTR [rip+0xfffffffffffffff8] */
    "420f3802bc0800100000",  /* phaddd mm7,QWORD PTR [rax+r9*1+0x1000] */
    "0f3802f3",  /* phaddd mm6,mm3 */
    "0f38030d00000000",  /* phaddsw mm1,QWORD PTR [rip+0x0] */
    "0f38032df8ffffff",  /* phaddsw mm5,QWORD PTR [rip+0xfffffffffffffff8] */
    "0f38030c8d00100000",  /* phaddsw mm1,QWORD PTR [rcx*4+0x1000] */
    "0f3801dd",  /* phaddw mm3,mm5 */
    "0f3801cb",  /* phaddw mm1,mm3 */
    "0f3801ec",  /* phaddw mm5,mm4 */
    "0f3806c4",  /* phsubd mm0,mm4 */
    "0f3806cd",  /* phsubd mm1,mm5 */
    "430f38066cc5c0",  /* phsubd mm5,QWORD PTR [r13+r8*8-0x40] */
    "0f3807e3",  /* phsubsw mm4,mm3 */
    "0f3807b278563412",  /* phsubsw mm6,QWORD PTR [rdx+0x12345678] */
    "0f3807047d10000000",  /* phsubsw mm0,QWORD PTR [rdi*2+0x10] */
    "0f3805f5",  /* phsubw mm6,mm5 */
    "0f3805946f80000000",  /* phsubw mm2,QWORD PTR [rdi+rbp*2+0x80] */
    "410f38054208",  /* phsubw mm0,QWORD PTR [r10+0x8] */
    "0fc4ae00f0ffff54",  /* pinsrw mm5,WORD PTR [rsi-0x1000],0x54 */
    "660fc41c250000000002",  /* pinsrw xmm3,WORD PTR ds:0x0,0x2 */
    "0fc424f510000000af",  /* pinsrw mm4,WORD PTR [rsi*8+0x10],0xaf */
    "0f3804ec",  /* pmaddubsw mm5,mm4 */
    "0f3804d6",  /* pmaddubsw mm2,mm6 */
    "0f3804fe",  /* pmaddubsw mm7,mm6 */
    "0ff5e7",  /* pmaddwd mm4,mm7 */
    "0ff51472",  /* pmaddwd mm2,QWORD PTR [rdx+rsi*2] */
    "0ff5eb",  /* pmaddwd mm5,mm3 */
    "66450f383d74cd80",  /* pmaxsd xmm14,XMMWORD PTR [r13+rcx*8-0x80] */
    "660f383dacaa78563412",  /* pmaxsd xmm5,XMMWORD PTR [rdx+rbp*4+0x12345678] */
    "660f383dfd",  /* pmaxsd xmm7,xmm5 */
    "0feeea",  /* pmaxsw mm5,mm2 */
    "0feecf",  /* pmaxsw mm1,mm7 */
    "0feee7",  /* pmaxsw mm4,mm7 */
    "0fdee8",  /* pmaxub mm5,mm0 */
    "0fde15f8ffffff",  /* pmaxub mm2,QWORD PTR [rip+0xfffffffffffffff8] */
    "66450fdea200010000",  /* pmaxub xmm12,XMMWORD PTR [r10+0x100] */
    "660f383f0d00010000",  /* pmaxud xmm1,XMMWORD PTR [rip+0x100] */
    "66410f383fee",  /* pmaxud xmm5,xmm14 */
    "66450f383f4f80",  /* pmaxud xmm9,XMMWORD PTR [r15-0x80] */
    "66410f38397540",  /* pminsd xmm6,XMMWORD PTR [r13+0x40] */
    "66440f3839349d00100000",  /* pminsd xmm14,XMMWORD PTR [rbx*4+0x1000] */
    "66440f3839dd",  /* pminsd xmm11,xmm5 */
    "0feade",  /* pminsw mm3,mm6 */
    "0feae8",  /* pminsw mm5,mm0 */
    "0feadf",  /* pminsw mm3,mm7 */
    "66410fdac6",  /* pminub xmm0,xmm14 */
    "0fdad6",  /* pminub mm2,mm6 */
    "0fdada",  /* pminub mm3,mm2 */
    "660f383b15f8ffffff",  /* pminud xmm2,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "66410f383bec",  /* pminud xmm5,xmm12 */
    "66420f383b5c4fc0",  /* pminud xmm3,XMMWORD PTR [rdi+r9*2-0x40] */
    "0fd7f8",  /* pmovmskb edi,mm0 */
    "440fd7cf",  /* pmovmskb r9d,mm7 */
    "440fd7e9",  /* pmovmskb r13d,mm1 */
    "66440f38303a",  /* pmovzxbw xmm15,QWORD PTR [rdx] */
    "66440f38300b",  /* pmovzxbw xmm9,QWORD PTR [rbx+0x0] */
    "66420f383004c7",  /* pmovzxbw xmm0,QWORD PTR [rdi+r8*8+0x0] */
    "66430f38280c4f",  /* pmuldq xmm1,XMMWORD PTR [r15+r9*2] */
    "66440f38287208",  /* pmuldq xmm14,XMMWORD PTR [rdx+0x8] */
    "66440f3828c5",  /* pmuldq xmm8,xmm5 */
    "430f380bbc7780000000",  /* pmulhrsw mm7,QWORD PTR [r15+r14*2+0x80] */
    "420f380b1c6d00100000",  /* pmulhrsw mm3,QWORD PTR [r13*2+0x1000] */
    "0f380bdf",  /* pmulhrsw mm3,mm7 */
    "0fe4c0",  /* pmulhuw mm0,mm0 */
    "660fe4c3",  /* pmulhuw xmm0,xmm3 */
    "420fe434b500100000",  /* pmulhuw mm6,QWORD PTR [r14*4+0x1000] */
    "0fe525f8ffffff",  /* pmulhw mm4,QWORD PTR [rip+0xfffffffffffffff8] */
    "0fe5cb",  /* pmulhw mm1,mm3 */
    "0fe505f8ffffff",  /* pmulhw mm0,QWORD PTR [rip+0xfffffffffffffff8] */
    "66440f38403d00000000",  /* pmulld xmm15,XMMWORD PTR [rip+0x0] */
    "66440f3840c1",  /* pmulld xmm8,xmm1 */
    "66440f3840a800f0ffff",  /* pmulld xmm13,XMMWORD PTR [rax-0x1000] */
    "0fd5d6",  /* pmullw mm2,mm6 */
    "0fd5fb",  /* pmullw mm7,mm3 */
    "0fd5d5",  /* pmullw mm2,mm5 */
    "0ff4e4",  /* pmuludq mm4,mm4 */
    "0ff43df8ffffff",  /* pmuludq mm7,QWORD PTR [rip+0xfffffffffffffff8] */
    "410ff44740",  /* pmuludq mm0,QWORD PTR [r15+0x40] */
    "65418f8500010000",  /* pop QWORD PTR gs:[r13+0x100] */
    "648f042528000000",  /* pop QWORD PTR fs:0x28 */
    "418f44f880",  /* pop QWORD PTR [r8+rdi*8-0x80] */
    "65f34d0fb83f",  /* popcnt r15,QWORD PTR gs:[r15] */
    "67f3480fb84e08",  /* popcnt rcx,QWORD PTR [esi+0x8] */
    "67f3480fb810",  /* popcnt rdx,QWORD PTR [eax] */
    "0febe5",  /* por mm4,mm5 */
    "430feb841780000000",  /* por mm0,QWORD PTR [r15+r10*1+0x80] */
    "0febcf",  /* por mm1,mm7 */
    "670f184ec0",  /* prefetcht0 BYTE PTR [esi-0x40] */
    "64410f184f80",  /* prefetcht0 BYTE PTR fs:[r15-0x80] */
    "420f184c8280",  /* prefetcht0 BYTE PTR [rdx+r8*4-0x80] */
    "66440ff61df8ffffff",  /* psadbw xmm11,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "0ff6f0",  /* psadbw mm6,mm0 */
    "0ff634cd00100000",  /* psadbw mm6,QWORD PTR [rcx*8+0x1000] */
    "0f3800048d00100000",  /* pshufb mm0,QWORD PTR [rcx*4+0x1000] */
    "420f38001c9500100000",  /* pshufb mm3,QWORD PTR [r10*4+0x1000] */
    "0f3800d8",  /* pshufb mm3,mm0 */
    "66410f70e21b",  /* pshufd xmm4,xmm10,0x1b */
    "660f7054d17f1b",  /* pshufd xmm2,XMMWORD PTR [rcx+rdx*8+0x7f],0x1b */
    "6466440f703c25280000001b",  /* pshufd xmm15,XMMWORD PTR fs:0x28,0x1b */
    "0f70624069",  /* pshufw mm4,QWORD PTR [rdx+0x40],0x69 */
    "410f7052c0fb",  /* pshufw mm2,QWORD PTR [r10-0x40],0xfb */
    "0f70e685",  /* pshufw mm4,mm6,0x85 */
    "0f3808ec",  /* psignb mm5,mm4 */
    "420f380814a500100000",  /* psignb mm2,QWORD PTR [r12*4+0x1000] */
    "0f3808fd",  /* psignb mm7,mm5 */
    "0f380ac4",  /* psignd mm0,mm4 */
    "420f380a0c6d10000000",  /* psignd mm1,QWORD PTR [r13*2+0x10] */
    "0f380aec",  /* psignd mm5,mm4 */
    "0f3809dd",  /* psignw mm3,mm5 */
    "0f3809fa",  /* psignw mm7,mm2 */
    "0f38097a08",  /* psignw mm7,QWORD PTR [rdx+0x8] */
    "0ff2f2",  /* pslld mm6,mm2 */
    "0f72f30d",  /* pslld mm3,0xd */
    "0ff2f5",  /* pslld mm6,mm5 */
    "0f73f0d2",  /* psllq mm0,0xd2 */
    "0ff3cf",  /* psllq mm1,mm7 */
    "0f73f642",  /* psllq mm6,0x42 */
    "0f71f00f",  /* psllw mm0,0xf */
    "430ff1941000010000",  /* psllw mm2,QWORD PTR [r8+r10*1+0x100] */
    "0ff1d6",  /* psllw mm2,mm6 */
    "0fe22500010000",  /* psrad mm4,QWORD PTR [rip+0x100] */
    "0fe2da",  /* psrad mm3,mm2 */
    "0fe2f1",  /* psrad mm6,mm1 */
    "0f71e304",  /* psraw mm3,0x4 */
    "0f71e201",  /* psraw mm2,0x1 */
    "0f71e502",  /* psraw mm5,0x2 */
    "0f72d5b8",  /* psrld mm5,0xb8 */
    "0fd2b080000000",  /* psrld mm6,QWORD PTR [rax+0x80] */
    "420fd2348510000000",  /* psrld mm6,QWORD PTR [r8*4+0x10] */
    "0f73d509",  /* psrlq mm5,0x9 */
    "0fd3c3",  /* psrlq mm0,mm3 */
    "0fd3f2",  /* psrlq mm6,mm2 */
    "0fd10d00000000",  /* psrlw mm1,QWORD PTR [rip+0x0] */
    "410fd1a49880000000",  /* psrlw mm4,QWORD PTR [r8+rbx*4+0x80] */
    "0fd1d3",  /* psrlw mm2,mm3 */
    "0ff8cc",  /* psubb mm1,mm4 */
    "410ff839",  /* psubb mm7,QWORD PTR [r9] */
    "410ff820",  /* psubb mm4,QWORD PTR [r8] */
    "66410ffa5d08",  /* psubd xmm3,XMMWORD PTR [r13+0x8] */
    "0ffad3",  /* psubd mm2,mm3 */
    "660ffa3500010000",  /* psubd xmm6,XMMWORD PTR [rip+0x100] */
    "420ffb346500100000",  /* psubq mm6,QWORD PTR [r12*2+0x1000] */
    "66450ffb8380000000",  /* psubq xmm8,XMMWORD PTR [r11+0x80] */
    "0ffba180000000",  /* psubq mm4,QWORD PTR [rcx+0x80] */
    "0fe8ee",  /* psubsb mm5,mm6 */
    "0fe8c8",  /* psubsb mm1,mm0 */
    "66470fe8bc7278563412",  /* psubsb xmm15,XMMWORD PTR [r10+r14*2+0x12345678] */
    "0fe91d00010000",  /* psubsw mm3,QWORD PTR [rip+0x100] */
    "660fe9db",  /* psubsw xmm3,xmm3 */
    "66420fe96cf508",  /* psubsw xmm5,XMMWORD PTR [rbp+r14*8+0x8] */
    "0fd8d6",  /* psubusb mm2,mm6 */
    "0fd8de",  /* psubusb mm3,mm6 */
    "430fd84c88c0",  /* psubusb mm1,QWORD PTR [r8+r9*4-0x40] */
    "66450fd9db",  /* psubusw xmm11,xmm11 */
    "0fd9ec",  /* psubusw mm5,mm4 */
    "0fd9d0",  /* psubusw mm2,mm0 */
    "0ff9f1",  /* psubw mm6,mm1 */
    "0ff9de",  /* psubw mm3,mm6 */
    "66410ff9c6",  /* psubw xmm0,xmm14 */
    "66440f3817342500000000",  /* ptest xmm14,XMMWORD PTR ds:0x0 */
    "66470f38178ced78563412",  /* ptest xmm9,XMMWORD PTR [r13+r13*8+0x12345678] */
    "66440f38172500000000",  /* ptest xmm12,XMMWORD PTR [rip+0x0] */
    "0f68c8",  /* punpckhbw mm1,mm0 */
    "0f68d2",  /* punpckhbw mm2,mm2 */
    "66410f684f80",  /* punpckhbw xmm1,XMMWORD PTR [r15-0x80] */
    "660f6ab800010000",  /* punpckhdq xmm7,XMMWORD PTR [rax+0x100] */
    "66420f6a3ce510000000",  /* punpckhdq xmm7,XMMWORD PTR [r12*8+0x10] */
    "0f6a2500010000",  /* punpckhdq mm4,QWORD PTR [rip+0x100] */
    "660f6d2d00010000",  /* punpckhqdq xmm5,XMMWORD PTR [rip+0x100] */
    "66410f6d742480",  /* punpckhqdq xmm6,XMMWORD PTR [r12-0x80] */
    "66410f6dd8",  /* punpckhqdq xmm3,xmm8 */
    "420f6904dd00100000",  /* punpckhwd mm0,QWORD PTR [r11*8+0x1000] */
    "0f69c1",  /* punpckhwd mm0,mm1 */
    "66460f6904bd00100000",  /* punpckhwd xmm8,XMMWORD PTR [r15*4+0x1000] */
    "0f60ec",  /* punpcklbw mm5,mm4 */
    "66460f601cad10000000",  /* punpcklbw xmm11,XMMWORD PTR [r13*4+0x10] */
    "0f603c88",  /* punpcklbw mm7,DWORD PTR [rax+rcx*4] */
    "66440f620d00000000",  /* punpckldq xmm9,XMMWORD PTR [rip+0x0] */
    "420f622c9500100000",  /* punpckldq mm5,DWORD PTR [r10*4+0x1000] */
    "0f62f6",  /* punpckldq mm6,mm6 */
    "660f6c2c58",  /* punpcklqdq xmm5,XMMWORD PTR [rax+rbx*2] */
    "660f6cdf",  /* punpcklqdq xmm3,xmm7 */
    "66430f6c4c2440",  /* punpcklqdq xmm1,XMMWORD PTR [r12+r12*1+0x40] */
    "0f61c8",  /* punpcklwd mm1,mm0 */
    "660f6134ad00100000",  /* punpcklwd xmm6,XMMWORD PTR [rbp*4+0x1000] */
    "420f6114cd00100000",  /* punpcklwd mm2,DWORD PTR [r9*8+0x1000] */
    "4157",  /* push r15 */
    "ff77c0",  /* push QWORD PTR [rdi-0x40] */
    "6541ff3424",  /* push QWORD PTR gs:[r12] */
    "0fefee",  /* pxor mm5,mm6 */
    "0fefd8",  /* pxor mm3,mm0 */
    "0fefe5",  /* pxor mm4,mm5 */
    "6448d35380",  /* rcl QWORD PTR fs:[rbx-0x80],cl */
    "4ad114a500100000",  /* rcl QWORD PTR [r12*4+0x1000],1 */
    "6642d314c500100000",  /* rcl WORD PTR [r8*8+0x1000],cl */
    "48c15c060800",  /* rcr QWORD PTR [rsi+rax*1+0x8],0x0 */
    "6748d35e08",  /* rcr QWORD PTR [esi+0x8],cl */
    "41d19800010000",  /* rcr DWORD PTR [r8+0x100],1 */
    "0f32",  /* rdmsr */
    "f30fc7f8",  /* rdpid rax */
    "410fc7f5",  /* rdrand r13d */
    "f3a4",  /* rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi] */
    "f348ab",  /* rep stos QWORD PTR es:[rdi],rax */
    "f2ae",  /* repnz scas al,BYTE PTR es:[rdi] */
    "c20800",  /* ret 0x8 */
    "c3",  /* ret */
    "48d38200010000",  /* rol QWORD PTR [rdx+0x100],cl */
    "49c1450816",  /* rol QWORD PTR [r13+0x8],0x16 */
    "d304b7",  /* rol DWORD PTR [rdi+rsi*4+0x0],cl */
    "656641d14d14",  /* ror WORD PTR gs:[r13+0x14],1 */
    "43d08ce400010000",  /* ror BYTE PTR [r12+r12*8+0x100],1 */
    "6449d308",  /* ror QWORD PTR fs:[r8],cl */
    "64c4c37bf0451405",  /* rorx eax,DWORD PTR fs:[r13+0x14],0x5 */
    "c4237bf02c650010000005",  /* rorx r13d,DWORD PTR [r12*2+0x1000],0x5 */
    "c4637bf0050000000005",  /* rorx r8d,DWORD PTR [rip+0x0],0x5 */
    "660f3a0b3c250000000001",  /* roundsd xmm7,QWORD PTR ds:0x0,0x1 */
    "6566410f3a0b2f01",  /* roundsd xmm5,QWORD PTR gs:[r15+0x0],0x1 */
    "66470f3a0b748d7f01",  /* roundsd xmm14,QWORD PTR [r13+r9*4+0x7f],0x1 */
    "0faa",  /* rsm */
    "f30f012d00010000",  /* rstorssp QWORD PTR [rip+0x100] */
    "f30f016c1f08",  /* rstorssp QWORD PTR [rdi+rbx*1+0x8] */
    "f3410f012cb3",  /* rstorssp QWORD PTR [r11+rsi*4] */
    "41d13ca9",  /* sar DWORD PTR [r9+rbp*4+0x0],1 */
    "42c03c451000000009",  /* sar BYTE PTR [r8*2+0x10],0x9 */
    "4bd13c08",  /* sar QWORD PTR [r8+r9*1],1 */
    "f30f01ea",  /* saveprevssp */
    "64196180",  /* sbb DWORD PTR fs:[rcx-0x80],esp */
    "18544f14",  /* sbb BYTE PTR [rdi+rcx*2+0x14],dl */
    "654883591400",  /* sbb QWORD PTR gs:[rcx+0x14],0x0 */
    "640f974108",  /* seta BYTE PTR fs:[rcx+0x8] */
    "640f97042528000000",  /* seta BYTE PTR fs:0x28 */
    "400f97c6",  /* seta sil */
    "410f92c7",  /* setb r15b */
    "0f92042534120000",  /* setb BYTE PTR ds:0x1234 */
    "0f920500000000",  /* setb BYTE PTR [rip+0x0] */
    "400f94c7",  /* sete dil */
    "420f9404f8",  /* sete BYTE PTR [rax+r15*8+0x0] */
    "400f9fc5",  /* setg bpl */
    "0f9f0500000000",  /* setg BYTE PTR [rip+0x0] */
    "400f9cc6",  /* setl sil */
    "650f9c01",  /* setl BYTE PTR gs:[rcx] */
    "670f9c8600010000",  /* setl BYTE PTR [esi+0x100] */
    "0f95042534120000",  /* setne BYTE PTR ds:0x1234 */
    "0f9544ee14",  /* setne BYTE PTR [rsi+rbp*8+0x14] */
    "0f95c0",  /* setne al */
    "0f90042534120000",  /* seto BYTE PTR ds:0x1234 */
    "670f904380",  /* seto BYTE PTR [ebx-0x80] */
    "400f90c7",  /* seto dil */
    "420f9a447ec0",  /* setp BYTE PTR [rsi+r15*2-0x40] */
    "0f9ac7",  /* setp bh */
    "0f9a8578563412",  /* setp BYTE PTR [rbp+0x12345678] */
    "f30f01e8",  /* setssbsy */
    "420f0104bd10000000",  /* sgdt [r15*4+0x10] */
    "410f01841200100000",  /* sgdt [r10+rdx*1+0x1000] */
    "0f0107",  /* sgdt [rdi] */
    "450f38c9e0",  /* sha1msg1 xmm12,xmm8 */
    "0f38c925f8ffffff",  /* sha1msg1 xmm4,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "460f38c90c30",  /* sha1msg1 xmm9,XMMWORD PTR [rax+r14*1] */
    "410f38caa42478563412",  /* sha1msg2 xmm4,XMMWORD PTR [r12+0x12345678] */
    "440f38ca25f8ffffff",  /* sha1msg2 xmm12,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "420f38ca148500100000",  /* sha1msg2 xmm2,XMMWORD PTR [r8*4+0x1000] */
    "0f38c80500010000",  /* sha1nexte xmm0,XMMWORD PTR [rip+0x100] */
    "410f38c8a4f900010000",  /* sha1nexte xmm4,XMMWORD PTR [r9+rdi*8+0x100] */
    "410f38c89378563412",  /* sha1nexte xmm2,XMMWORD PTR [r11+0x12345678] */
    "440f3acc25f8ffffff58",  /* sha1rnds4 xmm12,XMMWORD PTR [rip+0xfffffffffffffff8],0x58 */
    "410f3acc42801e",  /* sha1rnds4 xmm0,XMMWORD PTR [r10-0x80],0x1e */
    "410f3accc800",  /* sha1rnds4 xmm1,xmm8,0x0 */
    "470f38ccaca980000000",  /* sha256msg1 xmm13,XMMWORD PTR [r9+r13*4+0x80] */
    "0f38cc0d00000000",  /* sha256msg1 xmm1,XMMWORD PTR [rip+0x0] */
    "0f38cccb",  /* sha256msg1 xmm1,xmm3 */
    "460f38cd3cd500100000",  /* sha256msg2 xmm15,XMMWORD PTR [r10*8+0x1000] */
    "0f38cd1500010000",  /* sha256msg2 xmm2,XMMWORD PTR [rip+0x100] */
    "0f38cdf7",  /* sha256msg2 xmm6,xmm7 */
    "460f38cb64f0c0",  /* sha256rnds2 xmm12,XMMWORD PTR [rax+r14*8-0x40],xmm0 */
    "440f38cb42c0",  /* sha256rnds2 xmm8,XMMWORD PTR [rdx-0x40],xmm0 */
    "440f38cb39",  /* sha256rnds2 xmm15,XMMWORD PTR [rcx],xmm0 */
    "6448c12425280000000b",  /* shl QWORD PTR fs:0x28,0xb */
    "6466d3242528000000",  /* shl WORD PTR fs:0x28,cl */
    "6748d126",  /* shl QWORD PTR [esi],1 */
    "670fa40803",  /* shld DWORD PTR [eax],ecx,0x3 */
    "670fa46b8003",  /* shld DWORD PTR [ebx-0x80],ebp,0x3 */
    "0fa4350000000003",  /* shld DWORD PTR [rip+0x0],esi,0x3 */
    "c46291f7c8",  /* shlx r9,rax,r13 */
    "6466c12806",  /* shr WORD PTR fs:[rax],0x6 */
    "6441c12f10",  /* shr DWORD PTR fs:[r15+0x0],0x10 */
    "4ad12cbd10000000",  /* shr QWORD PTR [r15*4+0x10],1 */
    "4c0fadff",  /* shrd rdi,r15,cl */
    "670fc61b03",  /* shufps xmm3,XMMWORD PTR [ebx+0x0],0x3 */
    "650fc6957856341203",  /* shufps xmm2,XMMWORD PTR gs:[rbp+0x12345678],0x3 */
    "640fc60c252800000003",  /* shufps xmm1,XMMWORD PTR fs:0x28,0x3 */
    "0f014c2408",  /* sidt [rsp+0x8] */
    "420f010c6500100000",  /* sidt [r12*2+0x1000] */
    "0f014b80",  /* sidt [rbx-0x80] */
    "0f00048d00100000",  /* sldt WORD PTR [rcx*4+0x1000] */
    "410f00c5",  /* sldt r13d */
    "0f0005f8ffffff",  /* sldt WORD PTR [rip+0xfffffffffffffff8] */
    "0f012500010000",  /* smsw WORD PTR [rip+0x100] */
    "490f01e6",  /* smsw r14 */
    "0f016280",  /* smsw WORD PTR [rdx-0x80] */
    "f20f513500000000",  /* sqrtsd xmm6,QWORD PTR [rip+0x0] */
    "65f2410f51697f",  /* sqrtsd xmm5,QWORD PTR gs:[r9+0x7f] */
    "f2450f515980",  /* sqrtsd xmm11,QWORD PTR [r9-0x80] */
    "fb",  /* sti */
    "0fae5ccd00",  /* stmxcsr DWORD PTR [rbp+rcx*8+0x0] */
    "670fae1e",  /* stmxcsr DWORD PTR [esi] */
    "640fae5808",  /* stmxcsr DWORD PTR fs:[rax+0x8] */
    "aa",  /* stos BYTE PTR es:[rdi],al */
    "410f00cf",  /* str r15d */
    "410f008cc680000000",  /* str WORD PTR [r14+rax*8+0x80] */
    "420f000c6d10000000",  /* str WORD PTR [r13*2+0x10] */
    "6466452938",  /* sub WORD PTR fs:[r8+0x0],r15w */
    "678028ff",  /* sub BYTE PTR [eax],0xff */
    "644c294500",  /* sub QWORD PTR fs:[rbp+0x0],r8 */
    "660f5c2cc4",  /* subpd xmm5,XMMWORD PTR [rsp+rax*8+0x0] */
    "660f5c1424",  /* subpd xmm2,XMMWORD PTR [rsp] */
    "64660f5ca42400010000",  /* subpd xmm4,XMMWORD PTR fs:[rsp+0x100] */
    "0f01f8",  /* swapgs */
    "0f34",  /* sysenter */
    "480f07",  /* sysretq */
    "66f784c700010000ff7f",  /* test WORD PTR [rdi+rax*8+0x100],0x7fff */
    "f704250000000080000000",  /* test DWORD PTR ds:0x0,0x80 */
    "6643856cac80",  /* test WORD PTR [r12+r13*4-0x80],bp */
    "66f3420fbc0478",  /* tzcnt ax,WORD PTR [rax+r15*2] */
    "66f3440fbc447e7f",  /* tzcnt r8w,WORD PTR [rsi+rdi*2+0x7f] */
    "f3460fbc04fd10000000",  /* tzcnt r8d,DWORD PTR [r15*8+0x10] */
    "6566450f2e2424",  /* ucomisd xmm12,QWORD PTR gs:[r12+0x0] */
    "66420f2e0cfd00000000",  /* ucomisd xmm1,QWORD PTR [r15*8+0x0] */
    "67660f2e08",  /* ucomisd xmm1,QWORD PTR [eax] */
    "440f147080",  /* unpcklps xmm14,XMMWORD PTR [rax-0x80] */
    "0f141500000000",  /* unpcklps xmm2,XMMWORD PTR [rip+0x0] */
    "470f1464ff80",  /* unpcklps xmm12,XMMWORD PTR [r15+r15*8-0x80] */
    "c58558de",  /* vaddpd ymm3,ymm15,ymm6 */
    "62c1cd2e58a000100000",  /* vaddpd ymm20{k6},ymm6,YMMWORD PTR [r8+0x1000] */
    "6211cd4b585cceff",  /* vaddpd zmm11{k3},zmm6,ZMMWORD PTR [r14+r9*8-0x40] */
    "62f16c585818",  /* vaddps zmm3,zmm2,DWORD BCST [rax] */
    "c568586cc980",  /* vaddps xmm13,xmm2,XMMWORD PTR [rcx+rcx*8-0x80] */
    "c481585814dc",  /* vaddps xmm2,xmm4,XMMWORD PTR [r12+r11*8] */
    "c4a173589cc780000000",  /* vaddsd xmm3,xmm1,QWORD PTR [rdi+r8*8+0x80] */
    "6271ef0558f2",  /* vaddsd xmm14{k5},xmm18,xmm2 */
    "6291af0458ea",  /* vaddsd xmm5{k4},xmm26,xmm26 */
    "c54258a680000000",  /* vaddss xmm12,xmm7,DWORD PTR [rsi+0x80] */
    "6231563858d5",  /* vaddss xmm10,xmm5,xmm21{rd-sae} */
    "62f15600580d00010000",  /* vaddss xmm1,xmm21,DWORD PTR [rip+0x100] */
    "62e20d20de3df8ffffff",  /* vaesdec ymm23,ymm30,YMMWORD PTR [rip+0xfffffffffffffff8] */
    "c46271dea778563412",  /* vaesdec xmm12,xmm1,XMMWORD PTR [rdi+0x12345678] */
    "c4e239de0d00010000",  /* vaesdec xmm1,xmm8,XMMWORD PTR [rip+0x100] */
    "62923548df444cfe",  /* vaesdeclast zmm0,zmm9,ZMMWORD PTR [r12+r9*2-0x80] */
    "c46271df6808",  /* vaesdeclast xmm13,xmm1,XMMWORD PTR [rax+0x8] */
    "c46225df6180",  /* vaesdeclast ymm12,ymm11,YMMWORD PTR [rcx-0x80] */
    "c44275dcd0",  /* vaesenc ymm10,ymm1,ymm8 */
    "c42231dc04bd10000000",  /* vaesenc xmm8,xmm9,XMMWORD PTR [r15*4+0x10] */
    "62e23d48dc0df8ffffff",  /* vaesenc zmm17,zmm8,ZMMWORD PTR [rip+0xfffffffffffffff8] */
    "62620d40dd14d8",  /* vaesenclast zmm26,zmm30,ZMMWORD PTR [rax+rbx*8] */
    "c46271dd3c6d10000000",  /* vaesenclast xmm15,xmm1,XMMWORD PTR [rbp*2+0x10] */
    "62d22500dd50fc",  /* vaesenclast xmm2,xmm27,XMMWORD PTR [r8-0x40] */
    "c46279dbd9",  /* vaesimc xmm11,xmm1 */
    "c4e279db1cfd00100000",  /* vaesimc xmm3,XMMWORD PTR [rdi*8+0x1000] */
    "c44279dbb900010000",  /* vaesimc xmm15,XMMWORD PTR [r9+0x100] */
    "c44379dfd8f8",  /* vaeskeygenassist xmm11,xmm8,0xf8 */
    "c42379df148510000000a9",  /* vaeskeygenassist xmm10,XMMWORD PTR [r8*4+0x10],0xa9 */
    "c4e379df7f8036",  /* vaeskeygenassist xmm7,XMMWORD PTR [rdi-0x80],0x36 */
    "c52555e1",  /* vandnpd ymm12,ymm11,ymm1 */
    "6221850b55f6",  /* vandnpd xmm30{k3},xmm15,xmm22 */
    "62418da255e5",  /* vandnpd ymm28{k2}{z},ymm30,ymm13 */
    "62211c8955fa",  /* vandnps xmm31{k1}{z},xmm12,xmm18 */
    "62a1044d55cc",  /* vandnps zmm17{k5},zmm15,zmm20 */
    "c4a14855347500100000",  /* vandnps xmm6,xmm6,XMMWORD PTR [r14*2+0x1000] */
    "62718d525435f8ffffff",  /* vandpd zmm14{k2},zmm30,QWORD BCST [rip+0xfffffffffffffff8] */
    "c4c12154e5",  /* vandpd xmm4,xmm11,xmm13 */
    "c4a17954449a08",  /* vandpd xmm0,xmm0,XMMWORD PTR [rdx+r11*4+0x8] */
    "c4a160543c4d10000000",  /* vandps xmm7,xmm3,XMMWORD PTR [r9*2+0x10] */
    "c4a15c54949100010000",  /* vandps ymm2,ymm4,YMMWORD PTR [rcx+r10*4+0x100] */
    "6221541454145d10000000",  /* vandps xmm26{k4},xmm21,DWORD BCST [r11*2+0x10] */
    "c4a3590d74df406e",  /* vblendpd xmm6,xmm4,XMMWORD PTR [rdi+r11*8+0x40],0x6e */
    "c4e3290dd394",  /* vblendpd xmm2,xmm10,xmm3,0x94 */
    "c4e31d0db978563412aa",  /* vblendpd ymm7,ymm12,YMMWORD PTR [rcx+0x12345678],0xaa */
    "c4c3310c1917",  /* vblendps xmm3,xmm9,XMMWORD PTR [r9],0x17 */
    "c463210c0df8ffffffd8",  /* vblendps xmm9,xmm11,XMMWORD PTR [rip+0xfffffffffffffff8],0xd8 */
    "c4e3050c650052",  /* vblendps ymm4,ymm15,YMMWORD PTR [rbp+0x0],0x52 */
    "c443754bc370",  /* vblendvpd ymm8,ymm1,ymm11,ymm7 */
    "c463354b0df8ffffff50",  /* vblendvpd ymm9,ymm9,YMMWORD PTR [rip+0xfffffffffffffff8],ymm5 */
    "c4e3414b9e00f0ffff60",  /* vblendvpd xmm3,xmm7,XMMWORD PTR [rsi-0x1000],xmm6 */
    "c4e3514a5308f0",  /* vblendvps xmm2,xmm5,XMMWORD PTR [rbx+0x8],xmm15 */
    "c4436d4a4608a0",  /* vblendvps ymm8,ymm2,YMMWORD PTR [r14+0x8],ymm10 */
    "c443014ac870",  /* vblendvps xmm9,xmm15,xmm8,xmm7 */
    "c4427d199c4d00f0ffff",  /* vbroadcastsd ymm11,QWORD PTR [r13+rcx*2-0x1000] */
    "62f2fd29194a01",  /* vbroadcastsd ymm1{k1},QWORD PTR [rdx+0x8] */
    "c4a27d19346d10000000",  /* vbroadcastsd ymm6,QWORD PTR [r13*2+0x10] */
    "c4e27d182cbd00100000",  /* vbroadcastss ymm5,DWORD PTR [rdi*4+0x1000] */
    "62e27dca18cc",  /* vbroadcastss zmm17{k2}{z},xmm4 */
    "c4c27d189f00100000",  /* vbroadcastss ymm3,DWORD PTR [r15+0x1000] */
    "c559c2f508",  /* vcmpeq_uqpd xmm14,xmm4,xmm5 */
    "c42178c274a65808",  /* vcmpeq_uqps xmm14,xmm0,XMMWORD PTR [rsi+r12*4+0x58] */
    "c592c2350000000008",  /* vcmpeq_uqss xmm6,xmm13,DWORD PTR [rip+0x0] */
    "c4413dc2c518",  /* vcmpeq_uspd ymm8,ymm8,ymm13 */
    "c560c2e218",  /* vcmpeq_usps xmm12,xmm3,xmm2 */
    "c44149c2d300",  /* vcmpeqpd xmm10,xmm6,xmm11 */
    "c4c160c2cf00",  /* vcmpeqps xmm1,xmm3,xmm15 */
    "c4417ac2f700",  /* vcmpeqss xmm14,xmm0,xmm15 */
    "c4c15dc2c31b",  /* vcmpfalse_ospd ymm0,ymm4,ymm11 */
    "c565c2121b",  /* vcmpfalse_ospd ymm10,ymm3,YMMWORD PTR [rdx] */
    "c54cc2147a0b",  /* vcmpfalseps ymm10,ymm6,YMMWORD PTR [rdx+rdi*2] */
    "c4c118c2f90b",  /* vcmpfalseps xmm7,xmm12,xmm9 */
    "c554c2ed1d",  /* vcmpge_oqps ymm13,ymm5,ymm5 */
    "c44125c2c70d",  /* vcmpgepd ymm8,ymm11,ymm15 */
    "c580c2e60d",  /* vcmpgeps xmm4,xmm15,xmm6 */
    "c5c5c2f11e",  /* vcmpgt_oqpd ymm6,ymm7,ymm1 */
    "c5f4c2c01e",  /* vcmpgt_oqps ymm0,ymm1,ymm0 */
    "c4210dc264beeb0e",  /* vcmpgtpd ymm12,ymm14,YMMWORD PTR [rsi+r15*4-0x15] */
    "c5a0c2e60e",  /* vcmpgtps xmm4,xmm11,xmm6 */
    "c4c129c2ec12",  /* vcmple_oqpd xmm5,xmm10,xmm12 */
    "c4c164c2d712",  /* vcmple_oqps ymm2,ymm3,ymm15 */
    "c571c2df02",  /* vcmplepd xmm11,xmm1,xmm7 */
    "62f10408c2c902",  /* vcmpleps k1,xmm15,xmm1 */
    "c542c2de02",  /* vcmpless xmm11,xmm7,xmm6 */
    "c549c2f211",  /* vcmplt_oqpd xmm14,xmm6,xmm2 */
    "c5b4c2fd11",  /* vcmplt_oqps ymm7,ymm9,ymm5 */
    "c561c2f801",  /* vcmpltpd xmm15,xmm3,xmm0 */
    "c55cc2ea01",  /* vcmpltps ymm13,ymm4,ymm2 */
    "c5fac2df01",  /* vcmpltss xmm3,xmm0,xmm7 */
    "c581c2c80c",  /* vcmpneq_oqpd xmm1,xmm15,xmm0 */
    "c564c2fc0c",  /* vcmpneq_oqps ymm15,ymm3,ymm4 */
    "c5f1c2f71c",  /* vcmpneq_ospd xmm6,xmm1,xmm7 */
    "c57ac23c851000000014",  /* vcmpneq_usss xmm15,xmm0,DWORD PTR [rax*4+0x10] */
    "62b17400c2e704",  /* vcmpneqps k4,xmm17,xmm23 */
    "62b1bf08c23c8d1000000004",  /* vcmpneqsd k7,xmm8,QWORD PTR [r9*4+0x10] */
    "c5e2c2cb04",  /* vcmpneqss xmm1,xmm3,xmm3 */
    "c53ac24af804",  /* vcmpneqss xmm9,xmm8,DWORD PTR [rdx-0x8] */
    "c58ac20004",  /* vcmpneqss xmm0,xmm14,DWORD PTR [rax] */
    "c511c2cb19",  /* vcmpnge_uqpd xmm9,xmm13,xmm3 */
    "c4c10cc2da19",  /* vcmpnge_uqps ymm3,ymm14,ymm10 */
    "62b19700c2d309",  /* vcmpngesd k2,xmm29,xmm19 */
    "c44175c2291a",  /* vcmpngt_uqpd ymm13,ymm1,YMMWORD PTR [r9] */
    "c5c5c2da1a",  /* vcmpngt_uqpd ymm3,ymm7,ymm2 */
    "c569c2ee0a",  /* vcmpngtpd xmm13,xmm2,xmm6 */
    "c54cc2c20a",  /* vcmpngtps ymm8,ymm6,ymm2 */
    "c4c115c2d716",  /* vcmpnle_uqpd ymm2,ymm13,ymm15 */
    "62f17600c20d0000000016",  /* vcmpnle_uqss k1,xmm17,DWORD PTR [rip+0x0] */
    "c4c125c2c306",  /* vcmpnlepd ymm0,ymm11,ymm11 */
    "c44104c2da06",  /* vcmpnleps ymm11,ymm15,ymm10 */
    "c552c2f506",  /* vcmpnless xmm14,xmm5,xmm5 */
    "c4c161c2e115",  /* vcmpnlt_uqpd xmm4,xmm3,xmm9 */
    "c4c169c23f15",  /* vcmpnlt_uqpd xmm7,xmm2,XMMWORD PTR [r15] */
    "c4c164c2d815",  /* vcmpnlt_uqps ymm3,ymm3,ymm8 */
    "c5f9c2d705",  /* vcmpnltpd xmm2,xmm0,xmm7 */
    "c530c2e405",  /* vcmpnltps xmm12,xmm9,xmm4 */
    "c4c15ac274240805",  /* vcmpnltss xmm6,xmm4,DWORD PTR [r12+0x8] */
    "c4c16ac2fc05",  /* vcmpnltss xmm7,xmm2,xmm12 */
    "c5f5c2c417",  /* vcmpord_spd ymm0,ymm1,ymm4 */
    "c578c2df17",  /* vcmpord_sps xmm11,xmm0,xmm7 */
    "c5b0c20207",  /* vcmpordps xmm0,xmm9,XMMWORD PTR [rdx] */
    "c4410ac2e607",  /* vcmpordss xmm12,xmm14,xmm14 */
    "62f15e00c22cc51000000007",  /* vcmpordss k5,xmm20,DWORD PTR [rax*8+0x10] */
    "62f13608c205f8ffffff07",  /* vcmpordss k0,xmm9,DWORD PTR [rip+0xfffffffffffffff8] */
    "6291d548c26407ffff",  /* vcmppd k4,zmm5,ZMMWORD PTR [r15+r8*1-0x40],0xff */
    "62f1a540c22df8ffffff3d",  /* vcmppd k5,zmm27,ZMMWORD PTR [rip+0xfffffffffffffff8],0x3d */
    "62f1ed08c279103f",  /* vcmppd k7,xmm2,XMMWORD PTR [rcx+0x100],0x3f */
    "62b16448c2fc51",  /* vcmpps k7,zmm3,zmm20,0x51 */
    "62d17c20c26802db",  /* vcmpps k5,ymm16,YMMWORD PTR [r8+0x40],0xdb */
    "c500c24ac02f",  /* vcmpps xmm9,xmm15,XMMWORD PTR [rdx-0x40],0x2f */
    "62d1e708c244ce0175",  /* vcmpsd k0,xmm3,QWORD PTR [r14+rcx*8+0x8],0x75 */
    "c40173c28cc600010000d3",  /* vcmpsd xmm9,xmm1,QWORD PTR [r14+r8*8+0x100],0xd3 */
    "62d1c708c2e164",  /* vcmpsd k4,xmm7,xmm9,0x64 */
    "c4a142c26c3e80d0",  /* vcmpss xmm5,xmm7,DWORD PTR [rsi+r15*1-0x80],0xd0 */
    "62916e08c25cac404b",  /* vcmpss k3,xmm2,DWORD PTR [r12+r13*4+0x100],0x4b */
    "62917608c2f9a5",  /* vcmpss k7,xmm1,xmm25,0xa5 */
    "c4c12dc2c11f",  /* vcmptrue_uspd ymm0,ymm10,ymm9 */
    "c44118c2dd1f",  /* vcmptrue_usps xmm11,xmm12,xmm13 */
    "c515c2d70f",  /* vcmptruepd ymm10,ymm13,ymm7 */
    "c55dc22113",  /* vcmpunord_spd ymm12,ymm4,YMMWORD PTR [rcx] */
    "c5b5c2c613",  /* vcmpunord_spd ymm0,ymm9,ymm6 */
    "c44160c2de03",  /* vcmpunordps xmm11,xmm3,xmm14 */
    "c44103c2e003",  /* vcmpunordsd xmm12,xmm15,xmm8 */
    "c48112c20c0f03",  /* vcmpunordss xmm1,xmm13,DWORD PTR [r15+r9*1] */
    "c5eac2e403",  /* vcmpunordss xmm4,xmm2,xmm4 */
    "c5792f9f00100000",  /* vcomisd xmm11,QWORD PTR [rdi+0x1000] */
    "c4c1792fe0",  /* vcomisd xmm4,xmm8 */
    "c4c1792fa800f0ffff",  /* vcomisd xmm5,QWORD PTR [r8-0x1000] */
    "c5f82fd0",  /* vcomiss xmm2,xmm0 */
    "c4c1782f5040",  /* vcomiss xmm2,DWORD PTR [r8+0x40] */
    "62317c182fca",  /* vcomiss xmm9,xmm18{sae} */
    "6242fd288ad8",  /* vcompresspd ymm8,ymm27 */
    "6232fd488a24fd10000000",  /* vcompresspd ZMMWORD PTR [r15*8+0x10],zmm12 */
    "6272fd498acf",  /* vcompresspd zmm7{k1},zmm9 */
    "62f27d0b8a2500000000",  /* vcompressps XMMWORD PTR [rip+0x0]{k3},xmm4 */
    "62227d288a344d10000000",  /* vcompressps YMMWORD PTR [r9*2+0x10],ymm30 */
    "62627d088a2cbd00100000",  /* vcompressps XMMWORD PTR [rdi*4+0x1000],xmm29 */
    "62f17c0c5b1cdd00100000",  /* vcvtdq2ps xmm3{k4},XMMWORD PTR [rbx*8+0x1000] */
    "62e17c085b68fc",  /* vcvtdq2ps xmm21,XMMWORD PTR [rax-0x40] */
    "c421785b54b908",  /* vcvtdq2ps xmm10,XMMWORD PTR [rcx+r15*4+0x8] */
    "6201fd785ad2",  /* vcvtpd2ps ymm26,zmm26{rz-sae} */
    "6261fd4b5a5802",  /* vcvtpd2ps ymm27{k3},ZMMWORD PTR [rax+0x80] */
    "c5fd5ab43500f0ffff",  /* vcvtpd2ps xmm6,YMMWORD PTR [rbp+rsi*1-0x1000] */
    "62527dca1303",  /* vcvtph2ps zmm8{k2}{z},YMMWORD PTR [r11] */
    "c4427d133c24",  /* vcvtph2ps ymm15,XMMWORD PTR [r12] */
    "62c27d48139800100000",  /* vcvtph2ps zmm19,YMMWORD PTR [r8+0x1000] */
    "c5795b0d00000000",  /* vcvtps2dq xmm9,XMMWORD PTR [rip+0x0] */
    "62f17d585b148510000000",  /* vcvtps2dq zmm2,DWORD BCST [rax*4+0x10] */
    "c5f95b04bd00100000",  /* vcvtps2dq xmm0,XMMWORD PTR [rdi*4+0x1000] */
    "c5785a3500010000",  /* vcvtps2pd xmm14,QWORD PTR [rip+0x100] */
    "c4c1785a11",  /* vcvtps2pd xmm2,QWORD PTR [r9] */
    "c57c5a1df8ffffff",  /* vcvtps2pd ymm11,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "c4637d1dd200",  /* vcvtps2ph xmm2,ymm10,0x0 */
    "c4e37d1d498000",  /* vcvtps2ph XMMWORD PTR [rcx-0x80],ymm1,0x0 */
    "c4637d1da41d13ffffff00",  /* vcvtps2ph XMMWORD PTR [rbp+rbx*1-0xed],ymm12,0x0 */
    "c4a17b2d3c4d10000000",  /* vcvtsd2si edi,QWORD PTR [r9*2+0x10] */
    "c4c1fb2d5a40",  /* vcvtsd2si rbx,QWORD PTR [r10+0x40] */
    "c421fb2d4460c0",  /* vcvtsd2si r8,QWORD PTR [rax+r12*2-0x40] */
    "c4417b5a3e",  /* vcvtsd2ss xmm15,xmm0,QWORD PTR [r14] */
    "c50b5a8780000000",  /* vcvtsd2ss xmm8,xmm14,QWORD PTR [rdi+0x80] */
    "c5635a24fb",  /* vcvtsd2ss xmm12,xmm3,QWORD PTR [rbx+rdi*8] */
    "c5232a2df8ffffff",  /* vcvtsi2sd xmm13,xmm11,DWORD PTR [rip+0xfffffffffffffff8] */
    "c421632a7cb840",  /* vcvtsi2sd xmm15,xmm3,DWORD PTR [rax+r15*4+0x40] */
    "c461bb2ade",  /* vcvtsi2sd xmm11,xmm8,rsi */
    "c5aa2a3df8ffffff",  /* vcvtsi2ss xmm7,xmm10,DWORD PTR [rip+0xfffffffffffffff8] */
    "c55a2abc1280000000",  /* vcvtsi2ss xmm15,xmm4,DWORD PTR [rdx+rdx*1+0x80] */
    "c5ea2a146d00100000",  /* vcvtsi2ss xmm2,xmm2,DWORD PTR [rbp*2+0x1000] */
    "c5125aae00010000",  /* vcvtss2sd xmm13,xmm13,DWORD PTR [rsi+0x100] */
    "62b156005a04fa",  /* vcvtss2sd xmm0,xmm21,DWORD PTR [rdx+r15*8] */
    "62217e005a0ca500100000",  /* vcvtss2sd xmm25,xmm16,DWORD PTR [r12*4+0x1000] */
    "c4c1fa2ddc",  /* vcvtss2si rbx,xmm12 */
    "c461fa2d3c9d10000000",  /* vcvtss2si r15,DWORD PTR [rbx*4+0x10] */
    "c5fa2d2df8ffffff",  /* vcvtss2si ebp,DWORD PTR [rip+0xfffffffffffffff8] */
    "c57a5bacc500100000",  /* vcvttps2dq xmm13,XMMWORD PTR [rbp+rax*8+0x1000] */
    "62117eaa5bf4",  /* vcvttps2dq ymm14{k2}{z},ymm28 */
    "62217eaa5b0ce510000000",  /* vcvttps2dq ymm25{k2}{z},YMMWORD PTR [r12*8+0x10] */
    "c4817b2c944d00f0ffff",  /* vcvttsd2si edx,QWORD PTR [r13+r9*2-0x1000] */
    "c4c1fb2cac2400100000",  /* vcvttsd2si rbp,QWORD PTR [r12+0x1000] */
    "c461fb2c748040",  /* vcvttsd2si r14,QWORD PTR [rax+rax*4+0x40] */
    "c461fa2cce",  /* vcvttss2si r9,xmm6 */
    "c461fa2cb43980000000",  /* vcvttss2si r14,DWORD PTR [rcx+rdi*1+0x80] */
    "c57a2c3500010000",  /* vcvttss2si r14d,DWORD PTR [rip+0x100] */
    "c441195ef7",  /* vdivpd xmm14,xmm12,xmm15 */
    "c421655e7c8680",  /* vdivpd ymm15,ymm3,YMMWORD PTR [rsi+r8*4-0x80] */
    "62d185705ef7",  /* vdivpd zmm6,zmm31,zmm15{rz-sae} */
    "c5145e15f8ffffff",  /* vdivps ymm10,ymm13,YMMWORD PTR [rip+0xfffffffffffffff8] */
    "62110cbe5ed5",  /* vdivps zmm10{k6}{z},zmm14,zmm29{rd-sae} */
    "62d1744b5e6202",  /* vdivps zmm4{k3},zmm1,ZMMWORD PTR [r10+0x80] */
    "62c1bf845e51f0",  /* vdivsd xmm18{k4}{z},xmm24,QWORD PTR [r9-0x80] */
    "6261df875e2500000000",  /* vdivsd xmm28{k7}{z},xmm20,QWORD PTR [rip+0x0] */
    "c441135e06",  /* vdivsd xmm8,xmm13,QWORD PTR [r14] */
    "c5d25e1500000000",  /* vdivss xmm2,xmm5,DWORD PTR [rip+0x0] */
    "62214e005e5cab40",  /* vdivss xmm27,xmm22,DWORD PTR [rbx+r13*4+0x100] */
    "625116975ec1",  /* vdivss xmm8{k7}{z},xmm29,xmm9{rn-sae} */
    "c4c35141c4b0",  /* vdppd xmm0,xmm5,xmm12,0xb0 */
    "c4c379415c2ec05d",  /* vdppd xmm3,xmm0,XMMWORD PTR [r14+rbp*1-0x40],0x5d */
    "c4a329410cd510000000be",  /* vdppd xmm1,xmm10,XMMWORD PTR [r10*8+0x10],0xbe */
    "c4236d403c8e6e",  /* vdpps ymm15,ymm2,YMMWORD PTR [rsi+r9*4],0x6e */
    "c4e36d4004cd10000000f7",  /* vdpps ymm0,ymm2,YMMWORD PTR [rcx*8+0x10],0xf7 */
    "c4e32940f169",  /* vdpps xmm6,xmm10,xmm1,0x69 */
    "420f00a48100010000",  /* verr WORD PTR [rcx+r8*4+0x100] */
    "410f00e1",  /* verr r9d */
    "410f0061c0",  /* verr WORD PTR [r9-0x40] */
    "420f002cf510000000",  /* verw WORD PTR [r14*8+0x10] */
    "0f006d00",  /* verw WORD PTR [rbp+0x0] */
    "430f00ac0200f0ffff",  /* verw WORD PTR [r10+r8*1-0x1000] */
    "62f2fd488824fd00100000",  /* vexpandpd zmm4,ZMMWORD PTR [rdi*8+0x1000] */
    "6202fd4888e1",  /* vexpandpd zmm28,zmm25 */
    "62b2fd898864c610",  /* vexpandpd xmm4{k1}{z},XMMWORD PTR [rsi+r8*8+0x80] */
    "62a27d2888347510000000",  /* vexpandps ymm22,YMMWORD PTR [r14*2+0x10] */
    "62e27d2d8825f8ffffff",  /* vexpandps ymm20{k5},YMMWORD PTR [rip+0xfffffffffffffff8] */
    "62a27d0f88e7",  /* vexpandps xmm20{k7},xmm23 */
    "c4637d19f687",  /* vextractf128 xmm6,ymm14,0x87 */
    "c4437d19a4c07856341243",  /* vextractf128 XMMWORD PTR [r8+rax*8+0x12345678],ymm12,0x43 */
    "c4437d194940e6",  /* vextractf128 XMMWORD PTR [r9+0x40],ymm9,0xe6 */
    "62437dab19d5d1",  /* vextractf32x4 xmm13{k3}{z},ymm26,0xd1 */
    "62637d4f1935f8ffffff59",  /* vextractf32x4 XMMWORD PTR [rip+0xfffffffffffffff8]{k7},zmm30,0x59 */
    "62637d2d1925000100009c",  /* vextractf32x4 XMMWORD PTR [rip+0x100]{k5},ymm28,0x9c */
    "62b37d481b04cd1000000053",  /* vextractf32x8 YMMWORD PTR [r9*8+0x10],zmm0,0x53 */
    "62b37d481bd6d9",  /* vextractf32x8 ymm22,zmm2,0xd9 */
    "62f37dcc1bc8e1",  /* vextractf32x8 ymm0{k4}{z},zmm1,0xe1 */
    "62f3fd49192cf51000000072",  /* vextractf64x2 XMMWORD PTR [rsi*8+0x10]{k1},zmm5,0x72 */
    "62d3fd28196810ff",  /* vextractf64x2 XMMWORD PTR [r8+0x100],ymm5,0xff */
    "62f3fdce19d3a3",  /* vextractf64x2 xmm3{k6}{z},zmm2,0xa3 */
    "6273fd481b4c17085c",  /* vextractf64x4 YMMWORD PTR [rdi+rdx*1+0x100],zmm9,0x5c */
    "6273fd481b0500000000c6",  /* vextractf64x4 YMMWORD PTR [rip+0x0],zmm8,0xc6 */
    "62c3fd4a1bc810",  /* vextractf64x4 ymm8{k2},zmm17,0x10 */
    "c4e37d391601",  /* vextracti128 XMMWORD PTR [rsi],ymm2,0x1 */
    "c4c37d399c747affffff01",  /* vextracti128 XMMWORD PTR [r12+rsi*2-0x86],ymm3,0x1 */
    "c4e37d391500010000fd",  /* vextracti128 XMMWORD PTR [rip+0x100],ymm2,0xfd */
    "62837dab39f525",  /* vextracti32x4 xmm29{k3}{z},ymm22,0x25 */
    "62837d2f39fda2",  /* vextracti32x4 xmm29{k7},ymm23,0xa2 */
    "62f37d2f393d0001000046",  /* vextracti32x4 XMMWORD PTR [rip+0x100]{k7},ymm7,0x46 */
    "62837dca3bcb45",  /* vextracti32x8 ymm27{k2}{z},zmm17,0x45 */
    "62237d483b04d510000000d6",  /* vextracti32x8 YMMWORD PTR [r10*8+0x10],zmm24,0xd6 */
    "62437d493b748ffc3c",  /* vextracti32x8 YMMWORD PTR [r15+rcx*4-0x80]{k1},zmm30,0x3c */
    "6243fd2e399c1c785634122f",  /* vextracti64x2 XMMWORD PTR [r12+rbx*1+0x12345678]{k6},ymm27,0x2f */
    "6263fd4c391d0000000036",  /* vextracti64x2 XMMWORD PTR [rip+0x0]{k4},zmm27,0x36 */
    "6243fdcd39ca77",  /* vextracti64x2 xmm10{k5}{z},zmm25,0x77 */
    "62d3fdc93be287",  /* vextracti64x4 ymm10{k1}{z},zmm4,0x87 */
    "6263fdce3bc6ba",  /* vextracti64x4 ymm6{k6}{z},zmm24,0xba */
    "62b3fd483b344d1000000059",  /* vextracti64x4 YMMWORD PTR [r9*2+0x10],zmm6,0x59 */
    "c4c3791794710010000027",  /* vextractps DWORD PTR [r9+rsi*2+0x1000],xmm2,0x27 */
    "c4a379172c9500100000fa",  /* vextractps DWORD PTR [r10*4+0x1000],xmm5,0xfa */
    "c48379175c1ac0fa",  /* vextractps DWORD PTR [r10+r11*1-0x40],xmm3,0xfa */
    "c4e2f9984208",  /* vfmadd132pd xmm0,xmm0,XMMWORD PTR [rdx+0x8] */
    "62c2d52898bc1f08000000",  /* vfmadd132pd ymm23,ymm5,YMMWORD PTR [r15+rbx*1+0x8] */
    "c4c2a198843400010000",  /* vfmadd132pd xmm0,xmm11,XMMWORD PTR [r12+rsi*1+0x100] */
    "6222658798c9",  /* vfmadd132ps xmm25{k7}{z},xmm19,xmm17 */
    "62824d8498fb",  /* vfmadd132ps xmm23{k4}{z},xmm22,xmm27 */
    "6232257098f2",  /* vfmadd132ps zmm14,zmm27,zmm18{rz-sae} */
    "62c2955899e8",  /* vfmadd132sd xmm21,xmm13,xmm8{ru-sae} */
    "62e2e50e994a01",  /* vfmadd132sd xmm17{k6},xmm3,QWORD PTR [rdx+0x8] */
    "c422f1995cce40",  /* vfmadd132sd xmm11,xmm1,QWORD PTR [rsi+r9*8+0x40] */
    "62a2758a991cf510000000",  /* vfmadd132ss xmm19{k2}{z},xmm1,DWORD PTR [r14*8+0x10] */
    "c46251998700010000",  /* vfmadd132ss xmm8,xmm5,DWORD PTR [rdi+0x100] */
    "c4a21999445e08",  /* vfmadd132ss xmm0,xmm12,DWORD PTR [rsi+r11*2+0x8] */
    "c462e9a80500010000",  /* vfmadd213pd xmm8,xmm2,XMMWORD PTR [rip+0x100] */
    "c462f9a8da",  /* vfmadd213pd xmm11,xmm0,xmm2 */
    "c442a5a8ac9d00010000",  /* vfmadd213pd ymm13,ymm11,YMMWORD PTR [r13+rbx*4+0x100] */
    "62120d30a864be02",  /* vfmadd213ps ymm12,ymm30,DWORD BCST [r14+r15*4+0x8] */
    "c46211a83c4d10000000",  /* vfmadd213ps xmm15,xmm13,XMMWORD PTR [rcx*2+0x10] */
    "c4822da8746440",  /* vfmadd213ps ymm6,ymm10,YMMWORD PTR [r12+r12*2+0x40] */
    "62f2d50fa9440008",  /* vfmadd213sd xmm0{k7},xmm5,QWORD PTR [rax+rax*1+0x40] */
    "62a2bd18a9e8",  /* vfmadd213sd xmm21,xmm8,xmm16{rn-sae} */
    "62e28585a95e08",  /* vfmadd213sd xmm19{k5}{z},xmm31,QWORD PTR [rsi+0x40] */
    "c4a219a9747b80",  /* vfmadd213ss xmm6,xmm12,DWORD PTR [rbx+r15*2-0x80] */
    "62326d8da9248d10000000",  /* vfmadd213ss xmm12{k5}{z},xmm2,DWORD PTR [r9*4+0x10] */
    "c46201a92df8ffffff",  /* vfmadd213ss xmm13,xmm15,DWORD PTR [rip+0xfffffffffffffff8] */
    "62c2c500b8642404",  /* vfmadd231pd xmm20,xmm23,XMMWORD PTR [r12+0x40] */
    "6272cd87b81500000000",  /* vfmadd231pd xmm10{k7}{z},xmm22,XMMWORD PTR [rip+0x0] */
    "62f2e52cb8ce",  /* vfmadd231pd ymm1{k4},ymm3,ymm6 */
    "62422da7b8ce",  /* vfmadd231ps ymm25{k7}{z},ymm26,ymm14 */
    "62527548b84ec0",  /* vfmadd231ps zmm9,zmm1,ZMMWORD PTR [r14-0x1000] */
    "c4e23db81df8ffffff",  /* vfmadd231ps ymm3,ymm8,YMMWORD PTR [rip+0xfffffffffffffff8] */
    "c482f9b97c5e80",  /* vfmadd231sd xmm7,xmm0,QWORD PTR [r14+r11*2-0x80] */
    "62929d85b9cf",  /* vfmadd231sd xmm1{k5}{z},xmm28,xmm31 */
    "62c2bd10b9d5",  /* vfmadd231sd xmm18,xmm24,xmm13{rn-sae} */
    "62e23504b91500010000",  /* vfmadd231ss xmm18{k4},xmm25,DWORD PTR [rip+0x100] */
    "62a23500b9db",  /* vfmadd231ss xmm19,xmm25,xmm19 */
    "62f21d0bb96415e0",  /* vfmadd231ss xmm4{k3},xmm12,DWORD PTR [rbp+rdx*1-0x80] */
    "6202c54696f7",  /* vfmaddsub132pd zmm30{k6},zmm23,zmm31 */
    "62e2fd18965cc301",  /* vfmaddsub132pd xmm19,xmm0,QWORD BCST [rbx+rax*8+0x8] */
    "62d285be968b00100000",  /* vfmaddsub132pd ymm1{k6}{z},ymm15,QWORD BCST [r11+0x1000] */
    "62621506969cfa78563412",  /* vfmaddsub132ps xmm27{k6},xmm29,XMMWORD PTR [rdx+rdi*8+0x12345678] */
    "c46215964940",  /* vfmaddsub132ps ymm9,ymm13,YMMWORD PTR [rcx+0x40] */
    "c4427d966a80",  /* vfmaddsub132ps ymm13,ymm0,YMMWORD PTR [r10-0x80] */
    "c4e2bda6e2",  /* vfmaddsub213pd ymm4,ymm8,ymm2 */
    "c4e2c5a63d00010000",  /* vfmaddsub213pd ymm7,ymm7,YMMWORD PTR [rip+0x100] */
    "62f2c5aaa63500010000",  /* vfmaddsub213pd ymm6{k2}{z},ymm7,YMMWORD PTR [rip+0x100] */
    "62127d2ca6f5",  /* vfmaddsub213ps ymm14{k4},ymm0,ymm29 */
    "62624540a60df8ffffff",  /* vfmaddsub213ps zmm25,zmm23,ZMMWORD PTR [rip+0xfffffffffffffff8] */
    "62d24591a633",  /* vfmaddsub213ps xmm6{k1}{z},xmm23,DWORD BCST [r11] */
    "62f2a589b6c0",  /* vfmaddsub231pd xmm0{k1}{z},xmm11,xmm0 */
    "c4c2b9b662c0",  /* vfmaddsub231pd xmm4,xmm8,XMMWORD PTR [r10-0x40] */
    "c4c2cdb61c24",  /* vfmaddsub231pd ymm3,ymm6,YMMWORD PTR [r12] */
    "c4e241b6c7",  /* vfmaddsub231ps xmm0,xmm7,xmm7 */
    "6232758fb604c510000000",  /* vfmaddsub231ps xmm8{k7}{z},xmm1,XMMWORD PTR [r8*8+0x10] */
    "62927dafb65c92fc",  /* vfmaddsub231ps ymm3{k7}{z},ymm0,YMMWORD PTR [r10+r10*4-0x80] */
    "6252b5a79a6424fc",  /* vfmsub132pd ymm12{k7}{z},ymm25,YMMWORD PTR [r12-0x80] */
    "c4e2c19af5",  /* vfmsub132pd xmm6,xmm7,xmm5 */
    "6202ed279ae0",  /* vfmsub132pd ymm28{k7},ymm18,ymm24 */
    "62623d2d9a7602",  /* vfmsub132ps ymm30{k5},ymm8,YMMWORD PTR [rsi+0x40] */
    "62c235189a5de0",  /* vfmsub132ps xmm19,xmm9,DWORD BCST [r13-0x80] */
    "62e22d459af0",  /* vfmsub132ps zmm22{k5},zmm26,zmm0 */
    "6262f5089b2df8ffffff",  /* vfmsub132sd xmm29,xmm1,QWORD PTR [rip+0xfffffffffffffff8] */
    "62f2bd819bec",  /* vfmsub132sd xmm5{k1}{z},xmm24,xmm4 */
    "62f2c5549bd1",  /* vfmsub132sd xmm2{k4},xmm23,xmm1{ru-sae} */
    "c462219b2c4d00100000",  /* vfmsub132ss xmm13,xmm11,DWORD PTR [rcx*2+0x1000] */
    "624225839bf9",  /* vfmsub132ss xmm31{k3}{z},xmm27,xmm9 */
    "c422219b9c4f80000000",  /* vfmsub132ss xmm11,xmm11,DWORD PTR [rdi+r9*2+0x80] */
    "6222adcdaa24ad00100000",  /* vfmsub213pd zmm28{k5}{z},zmm10,ZMMWORD PTR [r13*4+0x1000] */
    "62e2bdbcaace",  /* vfmsub213pd zmm17{k4}{z},zmm8,zmm6{rd-sae} */
    "c462a5aa045d00100000",  /* vfmsub213pd ymm8,ymm11,YMMWORD PTR [rbx*2+0x1000] */
    "62d25d18aabd00100000",  /* vfmsub213ps xmm7,xmm4,DWORD BCST [r13+0x1000] */
    "62623527aa1c7510000000",  /* vfmsub213ps ymm27{k7},ymm25,YMMWORD PTR [rsi*2+0x10] */
    "62427540aa8c2478563412",  /* vfmsub213ps zmm25,zmm17,ZMMWORD PTR [r12+0x12345678] */
    "6272e500ab3df8ffffff",  /* vfmsub213sd xmm15,xmm19,QWORD PTR [rip+0xfffffffffffffff8] */
    "c402e9ab0421",  /* vfmsub213sd xmm8,xmm2,QWORD PTR [r9+r12*1] */
    "c442e9ab646a08",  /* vfmsub213sd xmm12,xmm2,QWORD PTR [r10+rbp*2+0x8] */
    "62722d86ab0c4510000000",  /* vfmsub213ss xmm9{k6}{z},xmm26,DWORD PTR [rax*2+0x10] */
    "c46261abf7",  /* vfmsub213ss xmm14,xmm3,xmm7 */
    "c4c261ab7e08",  /* vfmsub213ss xmm7,xmm3,DWORD PTR [r14+0x8] */
    "62e2ad28ba2d00010000",  /* vfmsub231pd ymm21,ymm10,YMMWORD PTR [rip+0x100] */
    "c402f5baac3778563412",  /* vfmsub231pd ymm13,ymm1,YMMWORD PTR [r15+r14*1+0x12345678] */
    "62329dafba148e",  /* vfmsub231pd ymm10{k7}{z},ymm12,YMMWORD PTR [rsi+r9*4] */
    "62227d07bad8",  /* vfmsub231ps xmm27{k7},xmm16,xmm16 */
    "c4e245ba00",  /* vfmsub231ps ymm0,ymm7,YMMWORD PTR [rax] */
    "c4c27dba6780",  /* vfmsub231ps ymm4,ymm0,YMMWORD PTR [r15-0x80] */
    "6262d583bb62f8",  /* vfmsub231sd xmm28{k3}{z},xmm21,QWORD PTR [rdx-0x40] */
    "c4e2f1bb144500100000",  /* vfmsub231sd xmm2,xmm1,QWORD PTR [rax*2+0x1000] */
    "62c2f58cbbae00f0ffff",  /* vfmsub231sd xmm21{k4}{z},xmm1,QWORD PTR [r14-0x1000] */
    "62b23d8abbc6",  /* vfmsub231ss xmm0{k2}{z},xmm8,xmm22 */
    "62024584bbc8",  /* vfmsub231ss xmm25{k4}{z},xmm23,xmm24 */
    "c4c241bbaf00010000",  /* vfmsub231ss xmm5,xmm7,DWORD PTR [r15+0x100] */
    "62e2b5cb973500000000",  /* vfmsubadd132pd zmm22{k3}{z},zmm9,ZMMWORD PTR [rip+0x0] */
    "62a2ed4497e2",  /* vfmsubadd132pd zmm20{k4},zmm18,zmm18 */
    "62c2e50597c2",  /* vfmsubadd132pd xmm16{k5},xmm19,xmm10 */
    "c48221979c9e00f0ffff",  /* vfmsubadd132ps xmm3,xmm11,XMMWORD PTR [r14+r11*4-0x1000] */
    "6282152e97e2",  /* vfmsubadd132ps ymm20{k6},ymm13,ymm26 */
    "c4425197d8",  /* vfmsubadd132ps xmm11,xmm5,xmm8 */
    "c4a2eda7840978563412",  /* vfmsubadd213pd ymm0,ymm2,YMMWORD PTR [rcx+r9*1+0x12345678] */
    "62629d40a753fe",  /* vfmsubadd213pd zmm26,zmm28,ZMMWORD PTR [rbx-0x80] */
    "621285dba7f2",  /* vfmsubadd213pd zmm14{k3}{z},zmm15,zmm26{ru-sae} */
    "c4e24da71c5d10000000",  /* vfmsubadd213ps ymm3,ymm6,YMMWORD PTR [rbx*2+0x10] */
    "c48219a7acfd00010000",  /* vfmsubadd213ps xmm5,xmm12,XMMWORD PTR [r13+r15*8+0x100] */
    "62625dc1a7046d00100000",  /* vfmsubadd213ps zmm24{k1}{z},zmm20,ZMMWORD PTR [rbp*2+0x1000] */
    "62a2fd30b704d7",  /* vfmsubadd231pd ymm16,ymm16,QWORD BCST [rdi+r10*8] */
    "c4c2d9b7de",  /* vfmsubadd231pd xmm3,xmm4,xmm14 */
    "62a29518b73ce500100000",  /* vfmsubadd231pd xmm23,xmm13,QWORD BCST [r12*8+0x1000] */
    "62323570b7d9",  /* vfmsubadd231ps zmm11,zmm25,zmm17{rz-sae} */
    "c46265b74d00",  /* vfmsubadd231ps ymm9,ymm3,YMMWORD PTR [rbp+0x0] */
    "c46261b70df8ffffff",  /* vfmsubadd231ps xmm9,xmm3,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "c442b99c57c0",  /* vfnmadd132pd xmm10,xmm8,XMMWORD PTR [r15-0x40] */
    "c4c28d9cac4e80000000",  /* vfnmadd132pd ymm5,ymm14,YMMWORD PTR [r14+rcx*2+0x80] */
    "62a2d5839c3c4510000000",  /* vfnmadd132pd xmm23{k3}{z},xmm21,XMMWORD PTR [r8*2+0x10] */
    "626205409c35f8ffffff",  /* vfnmadd132ps zmm30,zmm31,ZMMWORD PTR [rip+0xfffffffffffffff8] */
    "62a20d009c0c4500100000",  /* vfnmadd132ps xmm17,xmm30,XMMWORD PTR [r8*2+0x1000] */
    "c4e2259c3cbd00100000",  /* vfnmadd132ps ymm7,ymm11,YMMWORD PTR [rdi*4+0x1000] */
    "62d2f5849df0",  /* vfnmadd132sd xmm6{k4}{z},xmm17,xmm8 */
    "c4e2f19db800f0ffff",  /* vfnmadd132sd xmm7,xmm1,QWORD PTR [rax-0x1000] */
    "6202b5559dd3",  /* vfnmadd132sd xmm26{k5},xmm25,xmm27{ru-sae} */
    "62a205059d844600100000",  /* vfnmadd132ss xmm16{k5},xmm31,DWORD PTR [rsi+r8*2+0x1000] */
    "62121d0a9dcf",  /* vfnmadd132ss xmm9{k2},xmm12,xmm31 */
    "c462519d2df8ffffff",  /* vfnmadd132ss xmm13,xmm5,DWORD PTR [rip+0xfffffffffffffff8] */
    "62d29d2bace3",  /* vfnmadd213pd ymm4{k3},ymm12,ymm11 */
    "6252ada6acc9",  /* vfnmadd213pd ymm9{k6}{z},ymm26,ymm9 */
    "c4e281ac146d10000000",  /* vfnmadd213pd xmm2,xmm15,XMMWORD PTR [rbp*2+0x10] */
    "c4e239ac6d08",  /* vfnmadd213ps xmm5,xmm8,XMMWORD PTR [rbp+0x8] */
    "c4627dac5b80",  /* vfnmadd213ps ymm11,ymm0,YMMWORD PTR [rbx-0x80] */
    "c4a271ac9c3e00100000",  /* vfnmadd213ps xmm3,xmm1,XMMWORD PTR [rsi+r15*1+0x1000] */
    "62f2a58ead4620",  /* vfnmadd213sd xmm0{k6}{z},xmm11,QWORD PTR [rsi+0x100] */
    "626295faadf6",  /* vfnmadd213sd xmm30{k2}{z},xmm13,xmm6{rz-sae} */
    "c4c2b9adb4ee00100000",  /* vfnmadd213sd xmm6,xmm8,QWORD PTR [r14+rbp*8+0x1000] */
    "62a2358cad04cd00100000",  /* vfnmadd213ss xmm16{k4}{z},xmm9,DWORD PTR [r9*8+0x1000] */
    "6212351aadcc",  /* vfnmadd213ss xmm9{k2},xmm9,xmm28{rn-sae} */
    "c48221ad9c8400010000",  /* vfnmadd213ss xmm3,xmm11,DWORD PTR [r12+r8*4+0x100] */
    "c4c2c9bc36",  /* vfnmadd231pd xmm6,xmm6,XMMWORD PTR [r14] */
    "c4e2cdbc25f8ffffff",  /* vfnmadd231pd ymm4,ymm6,YMMWORD PTR [rip+0xfffffffffffffff8] */
    "62c2cd8ebcf0",  /* vfnmadd231pd xmm22{k6}{z},xmm6,xmm8 */
    "62f255a9bcc5",  /* vfnmadd231ps ymm0{k1}{z},ymm5,ymm5 */
    "c46259bc2cbd10000000",  /* vfnmadd231ps xmm13,xmm4,XMMWORD PTR [rdi*4+0x10] */
    "c44265bc4908",  /* vfnmadd231ps ymm9,ymm3,YMMWORD PTR [r9+0x8] */
    "6242fd3ebdf9",  /* vfnmadd231sd xmm31{k6},xmm0,xmm9{rd-sae} */
    "c422e1bd14c510000000",  /* vfnmadd231sd xmm10,xmm3,QWORD PTR [r8*8+0x10] */
    "c4c2a9bd4780",  /* vfnmadd231sd xmm0,xmm10,QWORD PTR [r15-0x80] */
    "c46279bd1500000000",  /* vfnmadd231ss xmm10,xmm0,DWORD PTR [rip+0x0] */
    "6252650bbd54f2f0",  /* vfnmadd231ss xmm10{k3},xmm3,DWORD PTR [r10+rsi*8-0x40] */
    "c4e279bd642a80",  /* vfnmadd231ss xmm4,xmm0,DWORD PTR [rdx+rbp*1-0x80] */
    "c442a19e4443c0",  /* vfnmsub132pd xmm8,xmm11,XMMWORD PTR [r11+rax*2-0x40] */
    "62528dc69e5904",  /* vfnmsub132pd zmm11{k6}{z},zmm30,ZMMWORD PTR [r9+0x100] */
    "c442d99eba78563412",  /* vfnmsub132pd xmm15,xmm4,XMMWORD PTR [r10+0x12345678] */
    "62526df69ece",  /* vfnmsub132ps zmm9{k6}{z},zmm18,zmm14{rz-sae} */
    "621205049ecd",  /* vfnmsub132ps xmm9{k4},xmm31,xmm29 */
    "c4e2559e0d00000000",  /* vfnmsub132ps ymm1,ymm5,YMMWORD PTR [rip+0x0] */
    "c442919f848800f0ffff",  /* vfnmsub132sd xmm8,xmm13,QWORD PTR [r8+rcx*4-0x1000] */
    "6202d58c9ffd",  /* vfnmsub132sd xmm31{k4}{z},xmm5,xmm29 */
    "c442f19f30",  /* vfnmsub132sd xmm14,xmm1,QWORD PTR [r8] */
    "62124d009f44dc10",  /* vfnmsub132ss xmm8,xmm22,DWORD PTR [r12+r11*8+0x40] */
    "624245009f9a00100000",  /* vfnmsub132ss xmm27,xmm23,DWORD PTR [r10+0x1000] */
    "c462119f4c4b80",  /* vfnmsub132ss xmm9,xmm13,DWORD PTR [rbx+rcx*2-0x80] */
    "62d29544aec2",  /* vfnmsub213pd zmm0{k4},zmm29,zmm10 */
    "6262cd55ae045d00100000",  /* vfnmsub213pd zmm24{k5},zmm22,QWORD BCST [rbx*2+0x1000] */
    "62b2d548ae2cad10000000",  /* vfnmsub213pd zmm5,zmm5,ZMMWORD PTR [r13*4+0x10] */
    "c46201ae34bd10000000",  /* vfnmsub213ps xmm14,xmm15,XMMWORD PTR [rdi*4+0x10] */
    "c42201ae4c9008",  /* vfnmsub213ps xmm9,xmm15,XMMWORD PTR [rax+r10*4+0x8] */
    "62f26583ae35f8ffffff",  /* vfnmsub213ps xmm6{k3}{z},xmm19,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "62629589af0cc500100000",  /* vfnmsub213sd xmm25{k1}{z},xmm13,QWORD PTR [rax*8+0x1000] */
    "62e2a5d3afca",  /* vfnmsub213sd xmm17{k3}{z},xmm27,xmm2{ru-sae} */
    "c402c9af844400f0ffff",  /* vfnmsub213sd xmm8,xmm6,QWORD PTR [r12+r8*2-0x1000] */
    "c48221af74d8c0",  /* vfnmsub213ss xmm6,xmm11,DWORD PTR [r8+r11*8-0x40] */
    "62922507af746c40",  /* vfnmsub213ss xmm6{k7},xmm27,DWORD PTR [r12+r13*2+0x100] */
    "62726d0caf9000100000",  /* vfnmsub213ss xmm10{k4},xmm2,DWORD PTR [rax+0x1000] */
    "c4a2edbe34c510000000",  /* vfnmsub231pd ymm6,ymm2,YMMWORD PTR [r8*8+0x10] */
    "6262adc5be04bd10000000",  /* vfnmsub231pd zmm24{k5}{z},zmm26,ZMMWORD PTR [rdi*4+0x10] */
    "62e2f52abe3d00010000",  /* vfnmsub231pd ymm23{k2},ymm1,YMMWORD PTR [rip+0x100] */
    "c4c201be8978563412",  /* vfnmsub231ps xmm1,xmm15,XMMWORD PTR [r9+0x12345678] */
    "62822528be54cd00",  /* vfnmsub231ps ymm18,ymm11,YMMWORD PTR [r13+r9*8+0x0] */
    "c4e269be0cc500100000",  /* vfnmsub231ps xmm1,xmm2,XMMWORD PTR [rax*8+0x1000] */
    "c4e2d1bf8000f0ffff",  /* vfnmsub231sd xmm0,xmm5,QWORD PTR [rax-0x1000] */
    "6212c500bf64caf0",  /* vfnmsub231sd xmm12,xmm23,QWORD PTR [r10+r9*8-0x80] */
    "62c2a551bfd3",  /* vfnmsub231sd xmm18{k1},xmm27,xmm11{ru-sae} */
    "62726507bf2500010000",  /* vfnmsub231ss xmm12{k7},xmm19,DWORD PTR [rip+0x100] */
    "c4e271bfd1",  /* vfnmsub231ss xmm2,xmm1,xmm1 */
    "c4e269bf145d10000000",  /* vfnmsub231ss xmm2,xmm2,DWORD PTR [rbx*2+0x10] */
    "c462f9927cbb08",  /* vgatherdpd xmm15,QWORD PTR [rbx+xmm7*4+0x8],xmm0 */
    "6292fd26926cf9f0",  /* vgatherdpd ymm5{k6},QWORD PTR [r9+xmm31*8-0x80] */
    "62a2fd0a929c3278563412",  /* vgatherdpd xmm19{k2},QWORD PTR [rdx+xmm14*1+0x12345678] */
    "c4a231920c13",  /* vgatherdps xmm1,DWORD PTR [rbx+xmm10*1],xmm9 */
    "62927d259274cd40",  /* vgatherdps ymm6{k5},DWORD PTR [r13+ymm25*8+0x100] */
    "62327d4a924c9720",  /* vgatherdps zmm9{k2},DWORD PTR [rdi+zmm10*4+0x80] */
    "6272fd0c934c1e08",  /* vgatherqpd xmm9{k4},QWORD PTR [rsi+xmm3*1+0x40] */
    "c4e2dd9314df",  /* vgatherqpd ymm2,QWORD PTR [rdi+ymm3*8],ymm4 */
    "6262fd27934c4b01",  /* vgatherqpd ymm25{k7},QWORD PTR [rbx+ymm17*2+0x8] */
    "62927d4d9334e8",  /* vgatherqps ymm6{k5},DWORD PTR [r8+zmm13*8] */
    "62227d4f9344d0f0",  /* vgatherqps ymm24{k7},DWORD PTR [rax+zmm10*8-0x40] */
    "62127d4a93bccc00f0ffff",  /* vgatherqps ymm15{k2},DWORD PTR [r12+zmm9*8-0x1000] */
    "62c3952fcf6c2408d9",  /* vgf2p8affineinvqb ymm21{k7},ymm13,YMMWORD PTR [r12+0x100],0xd9 */
    "62b3ad20cf044d00100000d0",  /* vgf2p8affineinvqb ymm0,ymm26,YMMWORD PTR [r9*2+0x1000],0xd0 */
    "62c38d00cfb4ef00f0ffff90",  /* vgf2p8affineinvqb xmm22,xmm30,XMMWORD PTR [r15+rbp*8-0x1000],0x90 */
    "c4e395ce049d00100000f5",  /* vgf2p8affineqb ymm0,ymm13,YMMWORD PTR [rbx*4+0x1000],0xf5 */
    "c423a9ce04ed0010000023",  /* vgf2p8affineqb xmm8,xmm10,XMMWORD PTR [r13*8+0x1000],0x23 */
    "c4e3b5ce1d000000005b",  /* vgf2p8affineqb ymm3,ymm9,YMMWORD PTR [rip+0x0],0x5b */
    "c4e27dcf9c5f80000000",  /* vgf2p8mulb ymm3,ymm0,YMMWORD PTR [rdi+rbx*2+0x80] */
    "62720d20cf3500000000",  /* vgf2p8mulb ymm14,ymm30,YMMWORD PTR [rip+0x0] */
    "62625da7cf3d00000000",  /* vgf2p8mulb ymm31{k7}{z},ymm20,YMMWORD PTR [rip+0x0] */
    "c4c34d18d001",  /* vinsertf128 ymm2,ymm6,xmm8,0x1 */
    "c4836d1894580010000028",  /* vinsertf128 ymm2,ymm2,XMMWORD PTR [r8+r11*2+0x1000],0x28 */
    "c4e37d183c850010000023",  /* vinsertf128 ymm7,ymm0,XMMWORD PTR [rax*4+0x1000],0x23 */
    "62c32d481827c7",  /* vinsertf32x4 zmm20,zmm10,XMMWORD PTR [r15],0xc7 */
    "62a335401804751000000052",  /* vinsertf32x4 zmm16,zmm25,XMMWORD PTR [r14*2+0x10],0x52 */
    "6293752418feca",  /* vinsertf32x4 ymm7{k4},ymm17,xmm30,0xca */
    "62d375cd1a6902de",  /* vinsertf32x8 zmm5{k5}{z},zmm1,YMMWORD PTR [r9+0x40],0xde */
    "62337d401a1c6500100000ee",  /* vinsertf32x8 zmm11,zmm16,YMMWORD PTR [r12*2+0x1000],0xee */
    "621305401adbde",  /* vinsertf32x8 zmm11,zmm31,ymm27,0xde */
    "62a3954b1824750010000090",  /* vinsertf64x2 zmm20{k3},zmm13,XMMWORD PTR [r14*2+0x1000],0x90 */
    "6273bd281854b2fc9f",  /* vinsertf64x2 ymm10,ymm8,XMMWORD PTR [rdx+rsi*4-0x40],0x9f */
    "62c3bd281803c2",  /* vinsertf64x2 ymm16,ymm8,XMMWORD PTR [r11],0xc2 */
    "6263f5c11abc177856341266",  /* vinsertf64x4 zmm31{k1}{z},zmm17,YMMWORD PTR [rdi+rdx*1+0x12345678],0x66 */
    "6273dd481a1d0001000049",  /* vinsertf64x4 zmm11,zmm4,YMMWORD PTR [rip+0x100],0x49 */
    "6223bdcc1adf21",  /* vinsertf64x4 zmm27{k4}{z},zmm8,ymm23,0x21 */
    "c463553872c0a0",  /* vinserti128 ymm14,ymm5,XMMWORD PTR [rdx-0x40],0xa0 */
    "c463453881785634127b",  /* vinserti128 ymm8,ymm7,XMMWORD PTR [rcx+0x12345678],0x7b */
    "c42335382c55001000004f",  /* vinserti128 ymm13,ymm9,XMMWORD PTR [r10*2+0x1000],0x4f */
    "626375203825000000002d",  /* vinserti32x4 ymm28,ymm17,XMMWORD PTR [rip+0x0],0x2d */
    "62c345cf38c7fc",  /* vinserti32x4 zmm16{k7}{z},zmm7,xmm15,0xfc */
    "62636d483824b500100000cf",  /* vinserti32x4 zmm28,zmm2,XMMWORD PTR [rsi*4+0x1000],0xcf */
    "62c365483aef22",  /* vinserti32x8 zmm21,zmm3,ymm15,0x22 */
    "62333dc93a6401fe29",  /* vinserti32x8 zmm12{k1}{z},zmm8,YMMWORD PTR [rcx+r8*1-0x40],0x29 */
    "62e34dc33a6dfed2",  /* vinserti32x8 zmm21{k3}{z},zmm22,YMMWORD PTR [rbp-0x40],0xd2 */
    "622395a9381c5d001000003c",  /* vinserti64x2 ymm27{k1}{z},ymm13,XMMWORD PTR [r11*2+0x1000],0x3c */
    "62b3bd2838a44e785634127b",  /* vinserti64x2 ymm4,ymm8,XMMWORD PTR [rsi+r9*2+0x12345678],0x7b */
    "62b3fdac38d046",  /* vinserti64x2 ymm2{k4}{z},ymm0,xmm16,0x46 */
    "627385413a7dfe6a",  /* vinserti64x4 zmm15{k1},zmm31,YMMWORD PTR [rbp-0x40],0x6a */
    "62e3bdc33ae093",  /* vinserti64x4 zmm20{k3}{z},zmm24,ymm0,0x93 */
    "62e3adc73ae2d5",  /* vinserti64x4 zmm20{k7}{z},zmm26,ymm2,0xd5 */
    "c463612125000000003f",  /* vinsertps xmm12,xmm3,DWORD PTR [rip+0x0],0x3f */
    "c4231921246d0010000067",  /* vinsertps xmm12,xmm12,DWORD PTR [r13*2+0x1000],0x67 */
    "c4c359213cf21d",  /* vinsertps xmm7,xmm4,DWORD PTR [r10+rsi*8],0x1d */
    "62e1fd375f4320",  /* vmaxpd ymm16{k7},ymm16,QWORD BCST [rbx+0x100] */
    "c55d5f94ad00f0ffff",  /* vmaxpd ymm10,ymm4,YMMWORD PTR [rbp+rbp*4-0x1000] */
    "62c1fd505f4c8f01",  /* vmaxpd zmm17,zmm16,QWORD BCST [r15+rcx*4+0x8] */
    "62d174b95f7c58f0",  /* vmaxps ymm7{k1}{z},ymm1,DWORD BCST [r8+rbx*2-0x40] */
    "c5545f9b00100000",  /* vmaxps ymm11,ymm5,YMMWORD PTR [rbx+0x1000] */
    "c441285f843580000000",  /* vmaxps xmm8,xmm10,XMMWORD PTR [r13+rsi*1+0x80] */
    "62f19f815f149d00100000",  /* vmaxsd xmm2{k1}{z},xmm28,QWORD PTR [rbx*4+0x1000] */
    "6261b7855f67f8",  /* vmaxsd xmm28{k5}{z},xmm25,QWORD PTR [rdi-0x40] */
    "c4c1535ffe",  /* vmaxsd xmm7,xmm5,xmm14 */
    "c4c1625fc7",  /* vmaxss xmm0,xmm3,xmm15 */
    "62f156865fd1",  /* vmaxss xmm2{k6}{z},xmm21,xmm1 */
    "62d156105fc6",  /* vmaxss xmm0,xmm21,xmm14{sae} */
    "c5015da978563412",  /* vminpd xmm13,xmm15,XMMWORD PTR [rcx+0x12345678] */
    "c5e15d2500000000",  /* vminpd xmm4,xmm3,XMMWORD PTR [rip+0x0] */
    "c4216d5d44f808",  /* vminpd ymm8,ymm2,YMMWORD PTR [rax+r15*8+0x8] */
    "622124205d249d00100000",  /* vminps ymm28,ymm27,YMMWORD PTR [r11*4+0x1000] */
    "624164035ddf",  /* vminps xmm27{k3},xmm19,xmm15 */
    "62e13c385d0d00000000",  /* vminps ymm17,ymm8,DWORD BCST [rip+0x0] */
    "62a1c70d5d144510000000",  /* vminsd xmm18{k5},xmm7,QWORD PTR [r8*2+0x10] */
    "62a1b7835dc3",  /* vminsd xmm16{k3}{z},xmm25,xmm19 */
    "c5c35de3",  /* vminsd xmm4,xmm7,xmm3 */
    "62511e845dec",  /* vminss xmm13{k4}{z},xmm28,xmm12 */
    "c4415a5d2f",  /* vminss xmm13,xmm4,DWORD PTR [r15] */
    "62616e175dd8",  /* vminss xmm27{k7},xmm18,xmm0{sae} */
    "6291fd0d2974aa04",  /* vmovapd XMMWORD PTR [r10+r13*4+0x40]{k5},xmm6 */
    "6271fd4a292500000000",  /* vmovapd ZMMWORD PTR [rip+0x0]{k2},zmm12 */
    "c5f9294108",  /* vmovapd XMMWORD PTR [rcx+0x8],xmm0 */
    "c5fc281f",  /* vmovaps ymm3,YMMWORD PTR [rdi] */
    "c578282d00010000",  /* vmovaps xmm13,XMMWORD PTR [rip+0x100] */
    "62317c4e293c5500100000",  /* vmovaps ZMMWORD PTR [r10*2+0x1000]{k6},zmm15 */
    "c481796ea4f100100000",  /* vmovd xmm4,DWORD PTR [r9+r14*8+0x1000] */
    "c5797edd",  /* vmovd ebp,xmm11 */
    "c441797e04d7",  /* vmovd DWORD PTR [r15+rdx*8],xmm8 */
    "c5f96f15f8ffffff",  /* vmovdqa xmm2,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "c57d7ffb",  /* vmovdqa ymm3,ymm15 */
    "c5796f04dd00100000",  /* vmovdqa xmm8,XMMWORD PTR [rbx*8+0x1000] */
    "62f17d087f5710",  /* vmovdqa32 XMMWORD PTR [rdi+0x100],xmm2 */
    "62517dca6f7b04",  /* vmovdqa32 zmm15{k2}{z},ZMMWORD PTR [r11+0x100] */
    "62a17d287f047d00100000",  /* vmovdqa32 YMMWORD PTR [r15*2+0x1000],ymm16 */
    "62e1fd296f40fc",  /* vmovdqa64 ymm16{k1},YMMWORD PTR [rax-0x80] */
    "6261fd2d7f17",  /* vmovdqa64 YMMWORD PTR [rdi]{k5},ymm26 */
    "6261fd096fd0",  /* vmovdqa64 xmm26{k1},xmm0 */
    "c4c17e7f1f",  /* vmovdqu YMMWORD PTR [r15],ymm3 */
    "c5fe7f4a80",  /* vmovdqu YMMWORD PTR [rdx-0x80],ymm1 */
    "c57e6f149d00100000",  /* vmovdqu ymm10,YMMWORD PTR [rbx*4+0x1000] */
    "62d1ff8b6fcf",  /* vmovdqu16 xmm1{k3}{z},xmm15 */
    "6271ff486fa508000000",  /* vmovdqu16 zmm12,ZMMWORD PTR [rbp+0x8] */
    "6251ff086f6d10",  /* vmovdqu16 xmm13,XMMWORD PTR [r13+0x100] */
    "62a17ec96f247510000000",  /* vmovdqu32 zmm20{k1}{z},ZMMWORD PTR [r14*2+0x10] */
    "62f17e486fc9",  /* vmovdqu32 zmm1,zmm1 */
    "62717e286f7102",  /* vmovdqu32 ymm14,YMMWORD PTR [rcx+0x40] */
    "62e1fecb6f5604",  /* vmovdqu64 zmm18{k3}{z},ZMMWORD PTR [rsi+0x100] */
    "62f1fe487f3500010000",  /* vmovdqu64 ZMMWORD PTR [rip+0x100],zmm6 */
    "6261fe4b7f1d00000000",  /* vmovdqu64 ZMMWORD PTR [rip+0x0]{k3},zmm27 */
    "62517f2a7f29",  /* vmovdqu8 YMMWORD PTR [r9]{k2},ymm13 */
    "62f17f0d7f9878563412",  /* vmovdqu8 XMMWORD PTR [rax+0x12345678]{k5},xmm3 */
    "62617faf6fd5",  /* vmovdqu8 ymm26{k7}{z},ymm5 */
    "c4417950cc",  /* vmovmskpd r9d,xmm12 */
    "c4c17c50fb",  /* vmovmskps edi,ymm11 */
    "c4417a7e942400010000",  /* vmovq xmm10,QWORD PTR [r12+0x100] */
    "6241fd086e63f8",  /* vmovq xmm28,QWORD PTR [r11-0x40] */
    "c4a179d634b500100000",  /* vmovq QWORD PTR [r14*4+0x1000],xmm6 */
    "c57b11143b",  /* vmovsd QWORD PTR [rbx+rdi*1],xmm10 */
    "c4417b10e3",  /* vmovsd xmm12,xmm0,xmm11 */
    "c4017b10540008",  /* vmovsd xmm10,QWORD PTR [r8+r8*1+0x8] */
    "c4c17a104c173d",  /* vmovss xmm1,DWORD PTR [r15+rdx*1+0x3d] */
    "c4c17a1030",  /* vmovss xmm6,DWORD PTR [r8] */
    "c4817a10b40853ffffff",  /* vmovss xmm6,DWORD PTR [r8+r9*1-0xad] */
    "6221fd291114ad10000000",  /* vmovupd YMMWORD PTR [r13*4+0x10]{k1},ymm26 */
    "62f1fd09102df8ffffff",  /* vmovupd xmm5{k1},XMMWORD PTR [rip+0xfffffffffffffff8] */
    "c44179118f80000000",  /* vmovupd XMMWORD PTR [r15+0x80],xmm9 */
    "c5f810246d10000000",  /* vmovups xmm4,XMMWORD PTR [rbp*2+0x10] */
    "62c17cac10d7",  /* vmovups ymm18{k4}{z},ymm15 */
    "c5fc119c5fd5000000",  /* vmovups YMMWORD PTR [rdi+rbx*2+0xd5],ymm3 */
    "c4a1215904ed10000000",  /* vmulpd xmm0,xmm11,XMMWORD PTR [r13*8+0x10] */
    "6261dd4059945875000000",  /* vmulpd zmm26,zmm20,ZMMWORD PTR [rax+rbx*2+0x75] */
    "c535595f08",  /* vmulpd ymm11,ymm9,YMMWORD PTR [rdi+0x8] */
    "c4a15c5954af08",  /* vmulps ymm2,ymm4,YMMWORD PTR [rdi+r13*4+0x8] */
    "62e1240d594508",  /* vmulps xmm16{k5},xmm11,XMMWORD PTR [rbp+0x80] */
    "62b15400597cc608",  /* vmulps xmm7,xmm21,XMMWORD PTR [rsi+r8*8+0x80] */
    "62c18f08594cab08",  /* vmulsd xmm17,xmm14,QWORD PTR [r11+rbp*4+0x40] */
    "c441435929",  /* vmulsd xmm13,xmm7,QWORD PTR [r9] */
    "62e1ff0959048d10000000",  /* vmulsd xmm16{k1},xmm0,QWORD PTR [rcx*4+0x10] */
    "6231660e59e0",  /* vmulss xmm12{k6},xmm3,xmm16 */
    "62c1563a59e7",  /* vmulss xmm20{k2},xmm5,xmm15{rd-sae} */
    "c5aa594f08",  /* vmulss xmm1,xmm10,DWORD PTR [rdi+0x8] */
    "6271b548560df8ffffff",  /* vorpd zmm9,zmm9,ZMMWORD PTR [rip+0xfffffffffffffff8] */
    "6221bd8256f0",  /* vorpd xmm30{k2}{z},xmm24,xmm16 */
    "c57d5606",  /* vorpd ymm8,ymm0,YMMWORD PTR [rsi] */
    "c538564e40",  /* vorps xmm9,xmm8,XMMWORD PTR [rsi+0x40] */
    "c5a4566f40",  /* vorps ymm5,ymm11,YMMWORD PTR [rdi+0x40] */
    "c4c178566c0840",  /* vorps xmm5,xmm0,XMMWORD PTR [r8+rcx*1+0x40] */
    "c4a16d6b248d10000000",  /* vpackssdw ymm4,ymm2,YMMWORD PTR [r9*4+0x10] */
    "62a14da26b44d902",  /* vpackssdw ymm16{k2}{z},ymm22,YMMWORD PTR [rcx+r11*8+0x40] */
    "62e15d0a6b3df8ffffff",  /* vpackssdw xmm23{k2},xmm4,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "c42165633ca0",  /* vpacksswb ymm15,ymm3,YMMWORD PTR [rax+r12*4] */
    "62116d2763f6",  /* vpacksswb ymm14{k7},ymm18,ymm30 */
    "62c13d2e63e8",  /* vpacksswb ymm21{k6},ymm8,ymm8 */
    "c5c1678700010000",  /* vpackuswb xmm0,xmm7,XMMWORD PTR [rdi+0x100] */
    "62b16d00672c7510000000",  /* vpackuswb xmm5,xmm18,XMMWORD PTR [r14*2+0x10] */
    "6261654567dd",  /* vpackuswb zmm27{k5},zmm19,zmm5 */
    "c4c15dfc747f48",  /* vpaddb ymm6,ymm4,YMMWORD PTR [r15+rdi*2+0x48] */
    "c42135fc048d10000000",  /* vpaddb ymm8,ymm9,YMMWORD PTR [r9*4+0x10] */
    "c44161fc9200100000",  /* vpaddb xmm10,xmm3,XMMWORD PTR [r10+0x1000] */
    "62a17d48febc4a07000000",  /* vpaddd zmm23,zmm0,ZMMWORD PTR [rdx+r9*2+0x7] */
    "c555fe0cbd10000000",  /* vpaddd ymm9,ymm5,YMMWORD PTR [rdi*4+0x10] */
    "62b10542fefd",  /* vpaddd zmm7{k2},zmm31,zmm21 */
    "6241c542d4e9",  /* vpaddq zmm29{k2},zmm23,zmm9 */
    "c571d40caf",  /* vpaddq xmm9,xmm1,XMMWORD PTR [rdi+rbp*4] */
    "6231dd4bd4d3",  /* vpaddq zmm10{k3},zmm4,zmm19 */
    "c52dec5880",  /* vpaddsb ymm11,ymm10,YMMWORD PTR [rax-0x80] */
    "c4c14dec14da",  /* vpaddsb ymm2,ymm6,YMMWORD PTR [r10+rbx*8] */
    "62e17dc1ec4fff",  /* vpaddsb zmm17{k1}{z},zmm16,ZMMWORD PTR [rdi-0x40] */
    "62716526ed540380",  /* vpaddsw ymm10{k6},ymm19,YMMWORD PTR [rbx+rax*1-0x1000] */
    "62c11508ed3431",  /* vpaddsw xmm22,xmm13,XMMWORD PTR [r9+rsi*1] */
    "62e13dc9ed35f8ffffff",  /* vpaddsw zmm22{k1}{z},zmm8,ZMMWORD PTR [rip+0xfffffffffffffff8] */
    "62612d48dc541901",  /* vpaddusb zmm26,zmm10,ZMMWORD PTR [rcx+rbx*1+0x40] */
    "62a16d42dcfd",  /* vpaddusb zmm23{k2},zmm18,zmm21 */
    "c575dc1c6d00100000",  /* vpaddusb ymm11,ymm1,YMMWORD PTR [rbp*2+0x1000] */
    "c53ddd5980",  /* vpaddusw ymm11,ymm8,YMMWORD PTR [rcx-0x80] */
    "c539dd71c0",  /* vpaddusw xmm14,xmm8,XMMWORD PTR [rcx-0x40] */
    "62d16d49dd6efe",  /* vpaddusw zmm5{k1},zmm2,ZMMWORD PTR [r14-0x80] */
    "623145affde1",  /* vpaddw ymm12{k7}{z},ymm7,ymm17 */
    "62a11d20fd2cfd10000000",  /* vpaddw ymm21,ymm28,YMMWORD PTR [r15*8+0x10] */
    "62d1050bfdef",  /* vpaddw xmm5{k3},xmm15,xmm15 */
    "c4c169db742480",  /* vpand xmm6,xmm2,XMMWORD PTR [r12-0x80] */
    "c585dbc7",  /* vpand ymm0,ymm15,ymm7 */
    "c5d5db1df8ffffff",  /* vpand ymm3,ymm5,YMMWORD PTR [rip+0xfffffffffffffff8] */
    "62b12d28dbf2",  /* vpandd ymm6,ymm10,ymm18 */
    "62714508dbb800f0ffff",  /* vpandd xmm15,xmm7,XMMWORD PTR [rax-0x1000] */
    "621125a6dbe6",  /* vpandd ymm12{k6}{z},ymm27,ymm30 */
    "c5cddfbb00f0ffff",  /* vpandn ymm7,ymm6,YMMWORD PTR [rbx-0x1000] */
    "c4a11ddf0ce7",  /* vpandn ymm1,ymm12,YMMWORD PTR [rdi+r12*8] */
    "c541dfcb",  /* vpandn xmm9,xmm7,xmm3 */
    "62c17dcadfd9",  /* vpandnd zmm19{k2}{z},zmm0,zmm9 */
    "62b10520df344d00100000",  /* vpandnd ymm6,ymm31,YMMWORD PTR [r9*2+0x1000] */
    "6221452adfc0",  /* vpandnd ymm24{k2},ymm7,ymm16 */
    "6291f584dfe5",  /* vpandnq xmm4{k4}{z},xmm17,xmm29 */
    "62419d48df6240",  /* vpandnq zmm28,zmm12,ZMMWORD PTR [r10+0x1000] */
    "62f18d37df5901",  /* vpandnq ymm3{k7},ymm30,QWORD BCST [rcx+0x8] */
    "6251bd28db8a00100000",  /* vpandq ymm9,ymm8,YMMWORD PTR [r10+0x1000] */
    "626185c7dbf5",  /* vpandq zmm30{k7}{z},zmm31,zmm5 */
    "62b18dc4dbe1",  /* vpandq zmm4{k4}{z},zmm30,zmm17 */
    "62511dcde0249e",  /* vpavgb zmm12{k5}{z},zmm12,ZMMWORD PTR [r14+rbx*4] */
    "c4c119e008",  /* vpavgb xmm1,xmm12,XMMWORD PTR [r8] */
    "62511520e0a278563412",  /* vpavgb ymm12,ymm29,YMMWORD PTR [r10+0x12345678] */
    "62714523e3db",  /* vpavgw ymm11{k3},ymm23,ymm3 */
    "62816583e3fb",  /* vpavgw xmm23{k3}{z},xmm19,xmm27 */
    "c4a11de324dd10000000",  /* vpavgw ymm4,ymm12,YMMWORD PTR [r11*8+0x10] */
    "c4430502ece1",  /* vpblendd ymm13,ymm15,ymm12,0xe1 */
    "c4631d027ec0f2",  /* vpblendd ymm15,ymm12,YMMWORD PTR [rsi-0x40],0xf2 */
    "c4e35d02150001000040",  /* vpblendd ymm2,ymm4,YMMWORD PTR [rip+0x100],0x40 */
    "c4e36d4c3ccd1000000010",  /* vpblendvb ymm7,ymm2,YMMWORD PTR [rcx*8+0x10],ymm1 */
    "c443314ce6e0",  /* vpblendvb xmm12,xmm9,xmm14,xmm14 */
    "c443054c0780",  /* vpblendvb ymm8,ymm15,YMMWORD PTR [r15],ymm8 */
    "c423050ebc4e7856341234",  /* vpblendw ymm15,ymm15,YMMWORD PTR [rsi+r9*2+0x12345678],0x34 */
    "c4033d0e8ce000f0ffff25",  /* vpblendw ymm9,ymm8,YMMWORD PTR [r8+r12*8-0x1000],0x25 */
    "c4e3050e0fcb",  /* vpblendw ymm1,ymm15,YMMWORD PTR [rdi],0xcb */
    "62e27d2f78e2",  /* vpbroadcastb ymm20{k7},xmm2 */
    "62027d4878546580",  /* vpbroadcastb zmm26,BYTE PTR [r13+r12*2-0x80] */
    "c4627d782d00010000",  /* vpbroadcastb ymm13,BYTE PTR [rip+0x100] */
    "62f27daf7cfb",  /* vpbroadcastd ymm7{k7}{z},ebx */
    "62c27d0b585c8602",  /* vpbroadcastd xmm19{k3},DWORD PTR [r14+rax*4+0x8] */
    "c4427d58c7",  /* vpbroadcastd ymm8,xmm15 */
    "6272fe082ad7",  /* vpbroadcastmb2q xmm10,k7 */
    "6272fe082af1",  /* vpbroadcastmb2q xmm14,k1 */
    "62f2fe282ade",  /* vpbroadcastmb2q ymm3,k6 */
    "62627e483add",  /* vpbroadcastmw2d zmm27,k5 */
    "62627e483ace",  /* vpbroadcastmw2d zmm25,k6 */
    "62727e483ad4",  /* vpbroadcastmw2d zmm10,k4 */
    "6242fd497cfc",  /* vpbroadcastq zmm31{k1},r12 */
    "c4e27d592c7510000000",  /* vpbroadcastq ymm5,QWORD PTR [rsi*2+0x10] */
    "6282fd4959ff",  /* vpbroadcastq zmm23{k1},xmm31 */
    "62527dae7bd0",  /* vpbroadcastw ymm10{k6}{z},r8d */
    "62627d487bd6",  /* vpbroadcastw zmm26,esi */
    "62327dab79bcbd00f0ffff",  /* vpbroadcastw ymm15{k3}{z},WORD PTR [rbp+r15*4-0x1000] */
    "c4c35d44660811",  /* vpclmulhqhqdq ymm4,ymm4,YMMWORD PTR [r14+0x8] */
    "c46379441b11",  /* vpclmulhqhqdq xmm11,xmm0,XMMWORD PTR [rbx] */
    "c4a335440c951000000011",  /* vpclmulhqhqdq ymm1,ymm9,YMMWORD PTR [r10*4+0x10] */
    "c4430544acdd00f0ffff01",  /* vpclmulhqlqdq ymm13,ymm15,YMMWORD PTR [r13+rbx*8-0x1000] */
    "c4c339440001",  /* vpclmulhqlqdq xmm0,xmm8,XMMWORD PTR [r8] */
    "62632540442d0001000001",  /* vpclmulhqlqdq zmm29,zmm27,ZMMWORD PTR [rip+0x100] */
    "c48321447c36c010",  /* vpclmullqhqdq xmm7,xmm11,XMMWORD PTR [r14+r14*1-0x40] */
    "62236d4044247d1000000010",  /* vpclmullqhqdq zmm28,zmm18,ZMMWORD PTR [r15*2+0x10] */
    "c46359448900f0ffff10",  /* vpclmullqhqdq xmm9,xmm4,XMMWORD PTR [rcx-0x1000] */
    "c463694405f8ffffff00",  /* vpclmullqlqdq xmm8,xmm2,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "c4431544b100f0ffff00",  /* vpclmullqlqdq ymm14,ymm13,YMMWORD PTR [r9-0x1000] */
    "62b315004404d50010000000",  /* vpclmullqlqdq xmm0,xmm29,XMMWORD PTR [r10*8+0x1000] */
    "c4435d44c313",  /* vpclmulqdq ymm8,ymm4,ymm11,0x13 */
    "62836d00444478fc2b",  /* vpclmulqdq xmm16,xmm18,XMMWORD PTR [r8+r15*2-0x40],0x2b */
    "c4c33944a1785634123f",  /* vpclmulqdq xmm4,xmm8,XMMWORD PTR [r9+0x12345678],0x3f */
    "62f36d483f6b02e4",  /* vpcmpb k5,zmm2,ZMMWORD PTR [rbx+0x80],0xe4 */
    "62931d403f842f0800000039",  /* vpcmpb k0,zmm28,ZMMWORD PTR [r15+r13*1+0x8],0x39 */
    "62b305083fdb59",  /* vpcmpb k3,xmm15,xmm19,0x59 */
    "62b36d401f3c5d0010000075",  /* vpcmpd k7,zmm18,ZMMWORD PTR [r11*2+0x1000],0x75 */
    "62f325101f0d00010000bd",  /* vpcmpd k1,xmm27,DWORD BCST [rip+0x100],0xbd */
    "62d325401fd307",  /* vpcmpd k2,zmm27,zmm11,0x7 */
    "62f1050b7405f8ffffff",  /* vpcmpeqb k0{k3},xmm15,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "62d1252374dd",  /* vpcmpeqb k3{k3},ymm27,ymm13 */
    "62f17544742d00010000",  /* vpcmpeqb k5{k4},zmm17,ZMMWORD PTR [rip+0x100] */
    "c4a159766c4008",  /* vpcmpeqd xmm5,xmm4,XMMWORD PTR [rax+r8*2+0x8] */
    "62d11d18769700100000",  /* vpcmpeqd k2,xmm12,DWORD BCST [r15+0x1000] */
    "62f17d08766cad10",  /* vpcmpeqd k5,xmm0,XMMWORD PTR [rbp+rbp*4+0x100] */
    "62d2b52029442404",  /* vpcmpeqq k0,ymm25,YMMWORD PTR [r12+0x80] */
    "c462392909",  /* vpcmpeqq xmm9,xmm8,XMMWORD PTR [rcx] */
    "62929d082914fa",  /* vpcmpeqq k2,xmm12,XMMWORD PTR [r10+r15*8] */
    "62b33d401ed200",  /* vpcmpequd k2,zmm24,zmm18 */
    "62b34d481ef600",  /* vpcmpequd k6,zmm6,zmm22 */
    "62f375401eff00",  /* vpcmpequd k7,zmm17,zmm7 */
    "62d16d4875cd",  /* vpcmpeqw k1,zmm2,zmm13 */
    "62b1454875248510000000",  /* vpcmpeqw k4,zmm7,ZMMWORD PTR [r8*4+0x10] */
    "62b1650075f2",  /* vpcmpeqw k6,xmm19,xmm18 */
    "c4e379612c95100000001f",  /* vpcmpestri xmm5,XMMWORD PTR [rdx*4+0x10],0x1f */
    "c4637961350001000069",  /* vpcmpestri xmm14,XMMWORD PTR [rip+0x100],0x69 */
    "c4437961c787",  /* vpcmpestri xmm8,xmm15,0x87 */
    "c4e37960142a3e",  /* vpcmpestrm xmm2,XMMWORD PTR [rdx+rbp*1],0x3e */
    "c4437960b44c00f0fffff1",  /* vpcmpestrm xmm14,XMMWORD PTR [r12+rcx*2-0x1000],0xf1 */
    "c4e3796062c09f",  /* vpcmpestrm xmm4,XMMWORD PTR [rdx-0x40],0x9f */
    "c4c15d649780000000",  /* vpcmpgtb ymm2,ymm4,YMMWORD PTR [r15+0x80] */
    "c4a14d6414f510000000",  /* vpcmpgtb ymm2,ymm6,YMMWORD PTR [r14*8+0x10] */
    "c4010564bc9800010000",  /* vpcmpgtb ymm15,ymm15,YMMWORD PTR [r8+r11*4+0x100] */
    "62d1154366ec",  /* vpcmpgtd k5{k3},zmm29,zmm12 */
    "c5e9666500",  /* vpcmpgtd xmm4,xmm2,XMMWORD PTR [rbp+0x0] */
    "c44135664708",  /* vpcmpgtd ymm8,ymm9,YMMWORD PTR [r15+0x8] */
    "6292852037fc",  /* vpcmpgtq k7,ymm31,ymm28 */
    "62d2ed4d374cceff",  /* vpcmpgtq k1{k5},zmm2,ZMMWORD PTR [r14+rcx*8-0x40] */
    "62f2ed4a37fb",  /* vpcmpgtq k7{k2},zmm2,zmm3 */
    "62b1552065246500100000",  /* vpcmpgtw k4,ymm21,YMMWORD PTR [r12*2+0x1000] */
    "62d1650565fe",  /* vpcmpgtw k7{k5},xmm19,xmm14 */
    "62b1554065ef",  /* vpcmpgtw k5,zmm21,zmm23 */
    "c4437963d6ed",  /* vpcmpistri xmm10,xmm14,0xed */
    "c4e379630d00010000eb",  /* vpcmpistri xmm1,XMMWORD PTR [rip+0x100],0xeb */
    "c42379633ca500100000b4",  /* vpcmpistri xmm15,XMMWORD PTR [r12*4+0x1000],0xb4 */
    "c4e37962250001000066",  /* vpcmpistrm xmm4,XMMWORD PTR [rip+0x100],0x66 */
    "c4c37962dbe3",  /* vpcmpistrm xmm3,xmm11,0xe3 */
    "c4e37962818000000029",  /* vpcmpistrm xmm0,XMMWORD PTR [rcx+0x80],0x29 */
    "62f325401fd102",  /* vpcmpled k2,zmm27,zmm1 */
    "62d365481fe902",  /* vpcmpled k5,zmm3,zmm9 */
    "62b365401fda02",  /* vpcmpled k3,zmm19,zmm18 */
    "62f3ad401fcb02",  /* vpcmpleq k1,zmm26,zmm3 */
    "62d32d481ee702",  /* vpcmpleud k4,zmm10,zmm15 */
    "62d305481ee802",  /* vpcmpleud k5,zmm15,zmm8 */
    "62d34d481ed402",  /* vpcmpleud k2,zmm6,zmm12 */
    "62d31d401fda01",  /* vpcmpltd k3,zmm28,zmm10 */
    "62f355481ff901",  /* vpcmpltd k7,zmm5,zmm1 */
    "62f335481fc901",  /* vpcmpltd k1,zmm9,zmm1 */
    "62d3fd481ffc01",  /* vpcmpltq k7,zmm0,zmm12 */
    "6293dd401fee01",  /* vpcmpltq k5,zmm20,zmm30 */
    "6293fd401fca01",  /* vpcmpltq k1,zmm16,zmm26 */
    "629345401ece01",  /* vpcmpltud k1,zmm23,zmm30 */
    "62f345481eea01",  /* vpcmpltud k5,zmm7,zmm2 */
    "62d35d481ee001",  /* vpcmpltud k4,zmm4,zmm8 */
    "62d335401fdc04",  /* vpcmpneqd k3,zmm25,zmm12 */
    "629365401fd004",  /* vpcmpneqd k2,zmm19,zmm24 */
    "62d32d481ff404",  /* vpcmpneqd k6,zmm10,zmm12 */
    "62d3dd481fcf04",  /* vpcmpneqq k1,zmm4,zmm15 */
    "62d3f5401fe604",  /* vpcmpneqq k4,zmm17,zmm14 */
    "62d39d401fd604",  /* vpcmpneqq k2,zmm28,zmm14 */
    "62b365481ec904",  /* vpcmpnequd k1,zmm3,zmm17 */
    "62d345401ee004",  /* vpcmpnequd k4,zmm23,zmm8 */
    "62b315401efe04",  /* vpcmpnequd k7,zmm29,zmm22 */
    "62f345481fdb06",  /* vpcmpnled k3,zmm7,zmm3 */
    "62f315401fe706",  /* vpcmpnled k4,zmm29,zmm7 */
    "62f365481ff206",  /* vpcmpnled k6,zmm3,zmm2 */
    "62b3bd481fe606",  /* vpcmpnleq k4,zmm8,zmm22 */
    "62d3b5481fca06",  /* vpcmpnleq k1,zmm9,zmm10 */
    "62d385481fd706",  /* vpcmpnleq k2,zmm15,zmm15 */
    "62933d401ede06",  /* vpcmpnleud k3,zmm24,zmm30 */
    "629335401ed006",  /* vpcmpnleud k2,zmm25,zmm24 */
    "62d325401ef906",  /* vpcmpnleud k7,zmm27,zmm9 */
    "62b345481fcd05",  /* vpcmpnltd k1,zmm7,zmm21 */
    "62937d481fd805",  /* vpcmpnltd k3,zmm0,zmm24 */
    "62f3cd401fd605",  /* vpcmpnltq k2,zmm22,zmm6 */
    "62f3bd481fea05",  /* vpcmpnltq k5,zmm8,zmm2 */
    "62b3fd401fcb05",  /* vpcmpnltq k1,zmm16,zmm19 */
    "62b325401ee505",  /* vpcmpnltud k4,zmm27,zmm21 */
    "62b315401edc05",  /* vpcmpnltud k3,zmm29,zmm20 */
    "62937d481ed205",  /* vpcmpnltud k2,zmm0,zmm26 */
    "62f3cd401fd303",  /* vpcmpq k2,zmm22,zmm3,0x3 */
    "62b3ed401f74860171",  /* vpcmpq k6,zmm18,ZMMWORD PTR [rsi+r8*4+0x40],0x71 */
    "62f39d381f250000000049",  /* vpcmpq k4,ymm12,QWORD BCST [rip+0x0],0x49 */
    "62b35d403e1cc500100000f2",  /* vpcmpub k3,zmm20,ZMMWORD PTR [r8*8+0x1000],0xf2 */
    "62f345003edc7a",  /* vpcmpub k3,xmm23,xmm4,0x7a */
    "62d37d483ed08b",  /* vpcmpub k2,zmm0,zmm8,0x8b */
    "62f30d481e6d02bf",  /* vpcmpud k5,zmm14,ZMMWORD PTR [rbp+0x80],0xbf */
    "62f31d101e807856341255",  /* vpcmpud k0,xmm28,DWORD BCST [rax+0x12345678],0x55 */
    "62b31d401ef403",  /* vpcmpud k6,zmm28,zmm20,0x3 */
    "62b3dd481e0ce5001000009d",  /* vpcmpuq k1,zmm4,ZMMWORD PTR [r12*8+0x1000],0x9d */
    "62f3c5401e3500010000ac",  /* vpcmpuq k6,zmm23,ZMMWORD PTR [rip+0x100],0xac */
    "62b3f5181e3c5d00100000ab",  /* vpcmpuq k7,xmm1,QWORD BCST [r11*2+0x1000],0xab */
    "62f38d483e050001000068",  /* vpcmpuw k0,zmm14,ZMMWORD PTR [rip+0x100],0x68 */
    "6293ed403eeb7f",  /* vpcmpuw k5,zmm18,zmm27,0x7f */
    "62d3e5283e4f029f",  /* vpcmpuw k1,ymm3,YMMWORD PTR [r15+0x40],0x9f */
    "6293ed083fc93e",  /* vpcmpw k1,xmm2,xmm25,0x3e */
    "62f3bd083feb4f",  /* vpcmpw k5,xmm8,xmm3,0x4f */
    "62f385283f2cfd10000000cb",  /* vpcmpw k5,ymm15,YMMWORD PTR [rdi*8+0x10],0xcb */
    "62927d8d8bc6",  /* vpcompressd xmm30{k5}{z},xmm0 */
    "62327d2a8bd7",  /* vpcompressd ymm23{k2},ymm10 */
    "62c27dab8bcd",  /* vpcompressd ymm13{k3}{z},ymm17 */
    "6242fd288bc1",  /* vpcompressq ymm9,ymm24 */
    "6262fd0d8b7101",  /* vpcompressq XMMWORD PTR [rcx+0x8]{k5},xmm30 */
    "62e2fd2c8b2df8ffffff",  /* vpcompressq YMMWORD PTR [rip+0xfffffffffffffff8]{k4},ymm21 */
    "62127d08c4e8",  /* vpconflictd xmm13,xmm24 */
    "62627dcfc41d00000000",  /* vpconflictd zmm27{k7}{z},ZMMWORD PTR [rip+0x0] */
    "62327d28c4646efc",  /* vpconflictd ymm12,YMMWORD PTR [rsi+r13*2-0x80] */
    "6212fdcec4e9",  /* vpconflictq zmm13{k6}{z},zmm25 */
    "6262fd29c41d00010000",  /* vpconflictq ymm27{k1},YMMWORD PTR [rip+0x100] */
    "6262fdacc4f4",  /* vpconflictq ymm30{k4}{z},ymm4 */
    "62f21d28500d00000000",  /* vpdpbusd ymm1,ymm12,YMMWORD PTR [rip+0x0] */
    "627255485025f8ffffff",  /* vpdpbusd zmm12,zmm5,ZMMWORD PTR [rip+0xfffffffffffffff8] */
    "62b27d08506caf10",  /* vpdpbusd xmm5,xmm0,XMMWORD PTR [rdi+r13*4+0x100] */
    "62822d2051c2",  /* vpdpbusds ymm16,ymm26,ymm26 */
    "6242151f515cc0f0",  /* vpdpbusds xmm27{k7},xmm13,DWORD BCST [r8+rax*8-0x40] */
    "62d26d30516ef0",  /* vpdpbusds ymm5,ymm18,DWORD BCST [r14-0x40] */
    "62c26558525020",  /* vpdpwssd zmm18,zmm3,DWORD BCST [r8+0x80] */
    "629235ac527452fe",  /* vpdpwssd ymm6{k4}{z},ymm9,YMMWORD PTR [r10+r10*2-0x40] */
    "6242550c520cf0",  /* vpdpwssd xmm25{k4},xmm5,XMMWORD PTR [r8+rsi*8] */
    "62d23d28533c6b",  /* vpdpwssds ymm7,ymm8,YMMWORD PTR [r11+rbp*2] */
    "6202054c53d3",  /* vpdpwssds zmm26{k4},zmm15,zmm27 */
    "62723d085314f500100000",  /* vpdpwssds xmm10,xmm8,XMMWORD PTR [rsi*8+0x1000] */
    "c4c31506face",  /* vperm2f128 ymm7,ymm13,ymm10,0xce */
    "c4e3350604bd10000000e9",  /* vperm2f128 ymm0,ymm9,YMMWORD PTR [rdi*4+0x10],0xe9 */
    "c4435506b4248000000087",  /* vperm2f128 ymm14,ymm5,YMMWORD PTR [r12+0x80],0x87 */
    "c4c37d46d174",  /* vperm2i128 ymm2,ymm0,ymm9,0x74 */
    "c4e36d4615000000007f",  /* vperm2i128 ymm2,ymm2,YMMWORD PTR [rip+0x0],0x7f */
    "c4430d465cf44099",  /* vperm2i128 ymm11,ymm14,YMMWORD PTR [r12+rsi*8+0x40],0x99 */
    "62a215c38dec",  /* vpermb zmm21{k3}{z},zmm29,zmm20 */
    "62f245228d3500010000",  /* vpermb ymm6{k2},ymm23,YMMWORD PTR [rip+0x100] */
    "623275288dd1",  /* vpermb ymm10,ymm1,ymm17 */
    "62220d38362c6d00100000",  /* vpermd ymm29,ymm14,DWORD BCST [r13*2+0x1000] */
    "62c205c336ef",  /* vpermd zmm21{k3}{z},zmm31,zmm15 */
    "c4c20536ab00100000",  /* vpermd ymm5,ymm15,YMMWORD PTR [r11+0x1000] */
    "62c205437566c0",  /* vpermi2b zmm20{k3},zmm31,ZMMWORD PTR [r14-0x1000] */
    "62c21da6754cfa02",  /* vpermi2b ymm17{k6}{z},ymm28,YMMWORD PTR [r10+rdi*8+0x40] */
    "62420d0375d6",  /* vpermi2b xmm26{k3},xmm30,xmm14 */
    "62620d50760df8ffffff",  /* vpermi2d zmm25,zmm30,DWORD BCST [rip+0xfffffffffffffff8] */
    "62c24510767710",  /* vpermi2d xmm22,xmm23,DWORD BCST [r15+0x40] */
    "62627508760cdd00100000",  /* vpermi2d xmm25,xmm1,XMMWORD PTR [rbx*8+0x1000] */
    "6242dd45777104",  /* vpermi2pd zmm30{k5},zmm20,ZMMWORD PTR [r9+0x100] */
    "62728548772df8ffffff",  /* vpermi2pd zmm13,zmm15,ZMMWORD PTR [rip+0xfffffffffffffff8] */
    "6202b52077c1",  /* vpermi2pd ymm24,ymm25,ymm25 */
    "6252154177df",  /* vpermi2ps zmm11{k1},zmm29,zmm15 */
    "62123dcd77d8",  /* vpermi2ps zmm11{k5}{z},zmm8,zmm24 */
    "62625ddb775740",  /* vpermi2ps zmm26{k3}{z},zmm4,DWORD BCST [rdi+0x100] */
    "6272fd207658fe",  /* vpermi2q ymm11,ymm16,YMMWORD PTR [rax-0x40] */
    "6222eda176d7",  /* vpermi2q ymm26{k1}{z},ymm18,ymm23 */
    "62629d4c76049510000000",  /* vpermi2q zmm24{k4},zmm12,ZMMWORD PTR [rdx*4+0x10] */
    "62f2e547751d00010000",  /* vpermi2w zmm3{k7},zmm19,ZMMWORD PTR [rip+0x100] */
    "6262e540753500010000",  /* vpermi2w zmm30,zmm19,ZMMWORD PTR [rip+0x100] */
    "6202fd4875c0",  /* vpermi2w zmm24,zmm0,zmm24 */
    "c462050de9",  /* vpermilpd ymm13,ymm15,ymm1 */
    "c4e37d05dc7f",  /* vpermilpd ymm3,ymm4,0x7f */
    "c422690d148d10000000",  /* vpermilpd xmm10,xmm2,XMMWORD PTR [r9*4+0x10] */
    "c4e2690c044d00100000",  /* vpermilps xmm0,xmm2,XMMWORD PTR [rcx*2+0x1000] */
    "c4637d04a578563412a1",  /* vpermilps ymm12,YMMWORD PTR [rbp+0x12345678],0xa1 */
    "c4e2250c3d00010000",  /* vpermilps ymm7,ymm11,YMMWORD PTR [rip+0x100] */
    "62c28dc416c5",  /* vpermpd zmm16{k4}{z},zmm30,zmm13 */
    "c423fd010c85100000004a",  /* vpermpd ymm9,YMMWORD PTR [r8*4+0x10],0x4a */
    "c483fd01a49178563412b5",  /* vpermpd ymm4,YMMWORD PTR [r9+r10*4+0x12345678],0xb5 */
    "c4c24d16dd",  /* vpermps ymm3,ymm6,ymm13 */
    "62c20d2d16df",  /* vpermps ymm19{k5},ymm14,ymm15 */
    "c4420516b280000000",  /* vpermps ymm14,ymm15,YMMWORD PTR [r10+0x80] */
    "62f2b54036d3",  /* vpermq zmm2,zmm25,zmm3 */
    "62b2e5cf362c4510000000",  /* vpermq zmm5{k7}{z},zmm3,ZMMWORD PTR [r8*2+0x10] */
    "62f3fd4e005c99c048",  /* vpermq zmm3{k6},ZMMWORD PTR [rcx+rbx*4-0x1000],0x48 */
    "62724d0f7d645104",  /* vpermt2b xmm12{k7},xmm6,XMMWORD PTR [rcx+rdx*2+0x40] */
    "62023d087d5cfef8",  /* vpermt2b xmm27,xmm8,XMMWORD PTR [r14+r15*8-0x80] */
    "62e2354b7dec",  /* vpermt2b zmm21{k3},zmm9,zmm4 */
    "6292454b7ecc",  /* vpermt2d zmm1{k3},zmm7,zmm28 */
    "62f255ce7ec0",  /* vpermt2d zmm0{k6}{z},zmm5,zmm0 */
    "626265867e247d10000000",  /* vpermt2d xmm28{k6}{z},xmm19,XMMWORD PTR [rdi*2+0x10] */
    "6202fd407f5446ff",  /* vpermt2pd zmm26,zmm16,ZMMWORD PTR [r14+r8*2-0x40] */
    "62f28d857f34dd10000000",  /* vpermt2pd xmm6{k5}{z},xmm30,XMMWORD PTR [rbx*8+0x10] */
    "62d2f5017f5308",  /* vpermt2pd xmm2{k1},xmm17,XMMWORD PTR [r11+0x80] */
    "62424d827f9c8600f0ffff",  /* vpermt2ps xmm27{k2}{z},xmm22,XMMWORD PTR [r14+rax*4-0x1000] */
    "62321d487ff4",  /* vpermt2ps zmm14,zmm12,zmm20 */
    "62223d2f7f84c878563412",  /* vpermt2ps ymm24{k7},ymm8,YMMWORD PTR [rax+r9*8+0x12345678] */
    "6262e5087e3d00000000",  /* vpermt2q xmm31,xmm3,XMMWORD PTR [rip+0x0] */
    "62a2bd0e7ef3",  /* vpermt2q xmm22{k6},xmm8,xmm19 */
    "6262edc57e2d00000000",  /* vpermt2q zmm29{k5}{z},zmm18,ZMMWORD PTR [rip+0x0] */
    "6252fd087d6c24f8",  /* vpermt2w xmm13,xmm0,XMMWORD PTR [r12-0x80] */
    "62c2954a7dc1",  /* vpermt2w zmm16{k2},zmm13,zmm9 */
    "62e2ed487d4901",  /* vpermt2w zmm17,zmm2,ZMMWORD PTR [rcx+0x40] */
    "62e2edae8d7c33fc",  /* vpermw ymm23{k6}{z},ymm2,YMMWORD PTR [rbx+rsi*1-0x80] */
    "626285408ddf",  /* vpermw zmm27,zmm31,zmm7 */
    "62d2bdae8df1",  /* vpermw ymm6{k6}{z},ymm8,ymm9 */
    "62327dc989ef",  /* vpexpandd zmm13{k1}{z},zmm23 */
    "62e27dab89af78563412",  /* vpexpandd ymm21{k3}{z},YMMWORD PTR [rdi+0x12345678] */
    "62627d8a8904ad00100000",  /* vpexpandd xmm24{k2}{z},XMMWORD PTR [rbp*4+0x1000] */
    "6252fd2889c6",  /* vpexpandq ymm8,ymm14 */
    "62f2fd28890d00000000",  /* vpexpandq ymm1,YMMWORD PTR [rip+0x0] */
    "6232fd2f89d5",  /* vpexpandq ymm10{k7},ymm21 */
    "c443791438aa",  /* vpextrb BYTE PTR [r8],xmm15,0xaa */
    "c4e3791425f8ffffffa3",  /* vpextrb BYTE PTR [rip+0xfffffffffffffff8],xmm4,0xa3 */
    "c4a37914049d1000000091",  /* vpextrb BYTE PTR [r11*4+0x10],xmm0,0x91 */
    "c46379162500010000d4",  /* vpextrd DWORD PTR [rip+0x100],xmm12,0xd4 */
    "c4c37916e202",  /* vpextrd r10d,xmm4,0x2 */
    "c4a379162ccd1000000004",  /* vpextrd DWORD PTR [r9*8+0x10],xmm5,0x4 */
    "c4e3f9162c850010000014",  /* vpextrq QWORD PTR [rax*4+0x1000],xmm5,0x14 */
    "c4c3f916f3b3",  /* vpextrq r11,xmm6,0xb3 */
    "c463f9160d00010000e8",  /* vpextrq QWORD PTR [rip+0x100],xmm9,0xe8 */
    "c5f9c5c29c",  /* vpextrw eax,xmm2,0x9c */
    "62927d29906c64e0",  /* vpgatherdd ymm5{k1},DWORD PTR [r12+ymm12*2-0x80] */
    "62d27d2a907cc140",  /* vpgatherdd ymm7{k2},DWORD PTR [r9+ymm0*8+0x100] */
    "c44275905c5780",  /* vpgatherdd ymm11,DWORD PTR [r15+ymm2*2-0x80],ymm1 */
    "c462bd902cf7",  /* vpgatherdq ymm13,QWORD PTR [rdi+xmm6*8],ymm8 */
    "6222fd4590b45200f0ffff",  /* vpgatherdq zmm30{k5},QWORD PTR [rdx+ymm26*2-0x1000] */
    "6282fd2b907c61f0",  /* vpgatherdq ymm23{k3},QWORD PTR [r9+xmm12*2-0x80] */
    "62327d239174fb10",  /* vpgatherqd xmm14{k3},DWORD PTR [rbx+ymm31*8+0x40] */
    "62827d22912c48",  /* vpgatherqd xmm21{k2},DWORD PTR [r8+ymm25*2] */
    "c4a2419134eb",  /* vpgatherqd xmm6,DWORD PTR [rbx+xmm13*8],xmm7 */
    "c4c29591a4c700100000",  /* vpgatherqq ymm4,QWORD PTR [r15+ymm0*8+0x1000],ymm13 */
    "62d2fd0e9154aef0",  /* vpgatherqq xmm2{k6},QWORD PTR [r14+xmm5*4-0x80] */
    "6252fd42916ca8f8",  /* vpgatherqq zmm13{k2},QWORD PTR [r8+zmm21*4-0x40] */
    "c44341205f0892",  /* vpinsrb xmm11,xmm7,BYTE PTR [r15+0x8],0x92 */
    "c4230920348ffc",  /* vpinsrb xmm14,xmm14,BYTE PTR [rdi+r9*4],0xfc */
    "c4c35120e203",  /* vpinsrb xmm4,xmm5,r10d,0x3 */
    "c4e30922ef59",  /* vpinsrd xmm5,xmm14,edi,0x59 */
    "c46371222d0000000021",  /* vpinsrd xmm13,xmm1,DWORD PTR [rip+0x0],0x21 */
    "c4832922a42a00010000c6",  /* vpinsrd xmm4,xmm10,DWORD PTR [r10+r13*1+0x100],0xc6 */
    "c443a1225bc026",  /* vpinsrq xmm11,xmm11,QWORD PTR [r11-0x40],0x26 */
    "c4e3a922e344",  /* vpinsrq xmm4,xmm10,rbx,0x44 */
    "c423912274c2c058",  /* vpinsrq xmm14,xmm13,QWORD PTR [rdx+r8*8-0x40],0x58 */
    "c5d9c415000100009d",  /* vpinsrw xmm2,xmm4,WORD PTR [rip+0x100],0x9d */
    "c44141c4e63d",  /* vpinsrw xmm12,xmm7,r14d,0x3d */
    "c5b1c4967856341208",  /* vpinsrw xmm2,xmm9,WORD PTR [rsi+0x12345678],0x8 */
    "62927d4b44ce",  /* vplzcntd zmm1{k3},zmm30 */
    "62627d494424fd00100000",  /* vplzcntd zmm28{k1},ZMMWORD PTR [rdi*8+0x1000] */
    "62b27d0d44f7",  /* vplzcntd xmm6{k5},xmm23 */
    "62e2fd48441cdd00100000",  /* vplzcntq zmm19,ZMMWORD PTR [rbx*8+0x1000] */
    "6262fd484405f8ffffff",  /* vplzcntq zmm24,ZMMWORD PTR [rip+0xfffffffffffffff8] */
    "6262fdda44247500100000",  /* vplzcntq zmm28{k2}{z},QWORD BCST [rsi*2+0x1000] */
    "62a2f5aeb50cbd10000000",  /* vpmadd52huq ymm17{k6}{z},ymm1,YMMWORD PTR [r15*4+0x10] */
    "6222f508b55c0af8",  /* vpmadd52huq xmm27,xmm1,XMMWORD PTR [rdx+r9*1-0x80] */
    "6262ad08b51d00010000",  /* vpmadd52huq xmm27,xmm10,XMMWORD PTR [rip+0x100] */
    "6222bd46b42c8d00100000",  /* vpmadd52luq zmm29{k6},zmm24,ZMMWORD PTR [r9*4+0x1000] */
    "62f2b520b41cdd00100000",  /* vpmadd52luq ymm3,ymm25,YMMWORD PTR [rbx*8+0x1000] */
    "6262c500b42df8ffffff",  /* vpmadd52luq xmm29,xmm23,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "62b1452af5ff",  /* vpmaddwd ymm7{k2},ymm7,ymm23 */
    "c4c17df500",  /* vpmaddwd ymm0,ymm0,YMMWORD PTR [r8] */
    "c4a16df5347500100000",  /* vpmaddwd ymm6,ymm2,YMMWORD PTR [r14*2+0x1000] */
    "c4e2453d1500000000",  /* vpmaxsd ymm2,ymm7,YMMWORD PTR [rip+0x0] */
    "62521d483d6e04",  /* vpmaxsd zmm13,zmm12,ZMMWORD PTR [r14+0x100] */
    "6242454e3d20",  /* vpmaxsd zmm28{k6},zmm7,ZMMWORD PTR [r8] */
    "c48141eea41f00f0ffff",  /* vpmaxsw xmm4,xmm7,XMMWORD PTR [r15+r11*1-0x1000] */
    "c4414dee0b",  /* vpmaxsw ymm9,ymm6,YMMWORD PTR [r11] */
    "c5c9ee5508",  /* vpmaxsw xmm2,xmm6,XMMWORD PTR [rbp+0x8] */
    "62717500de1df8ffffff",  /* vpmaxub xmm11,xmm17,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "62514d0ade5c4c10",  /* vpmaxub xmm11{k2},xmm6,XMMWORD PTR [r12+rcx*2+0x100] */
    "c4a105de3cfd00100000",  /* vpmaxub ymm7,ymm15,YMMWORD PTR [r15*8+0x1000] */
    "62e27d4e3ff8",  /* vpmaxud zmm23{k6},zmm0,zmm0 */
    "c422153f249d00100000",  /* vpmaxud ymm12,ymm13,YMMWORD PTR [r11*4+0x1000] */
    "c442693fa42400010000",  /* vpmaxud xmm12,xmm2,XMMWORD PTR [r12+0x100] */
    "62627d48390d00010000",  /* vpminsd zmm25,zmm0,ZMMWORD PTR [rip+0x100] */
    "c4421539aa00010000",  /* vpminsd ymm13,ymm13,YMMWORD PTR [r10+0x100] */
    "62b23d0c39d4",  /* vpminsd xmm2{k4},xmm8,xmm20 */
    "62717d00ea0d00000000",  /* vpminsw xmm9,xmm16,XMMWORD PTR [rip+0x0] */
    "62114d23ea74d204",  /* vpminsw ymm14{k3},ymm22,YMMWORD PTR [r10+r10*8+0x80] */
    "c4c14deac2",  /* vpminsw ymm0,ymm6,ymm10 */
    "c4216ddaa45e00010000",  /* vpminub ymm12,ymm2,YMMWORD PTR [rsi+r11*2+0x100] */
    "c48159da4c3080",  /* vpminub xmm1,xmm4,XMMWORD PTR [r8+r14*1-0x80] */
    "c581da1d00000000",  /* vpminub xmm3,xmm15,XMMWORD PTR [rip+0x0] */
    "62d225c33bfd",  /* vpminud zmm7{k3}{z},zmm27,zmm13 */
    "c4e2453b3500000000",  /* vpminud ymm6,ymm7,YMMWORD PTR [rip+0x0] */
    "62926d0c3bd9",  /* vpminud xmm3{k4},xmm2,xmm25 */
    "62927e08316cccf0",  /* vpmovdb DWORD PTR [r12+r9*8-0x40],xmm5 */
    "62c27e083184f578563412",  /* vpmovdb DWORD PTR [r13+rsi*8+0x12345678],xmm16 */
    "62027e2831cd",  /* vpmovdb xmm29,ymm25 */
    "62427ecc33da",  /* vpmovdw ymm10{k4}{z},zmm27 */
    "62227e28332cdd00100000",  /* vpmovdw XMMWORD PTR [r11*8+0x1000],ymm29 */
    "62127e0d33dd",  /* vpmovdw xmm29{k5},xmm11 */
    "c44179d7f5",  /* vpmovmskb r14d,xmm13 */
    "62727e283269e0",  /* vpmovqb DWORD PTR [rcx-0x80],ymm13 */
    "62627e4c32f1",  /* vpmovqb xmm1{k4},zmm30 */
    "62027e0a32540140",  /* vpmovqb WORD PTR [r9+r8*1+0x80]{k2},xmm26 */
    "62d27e0b35e6",  /* vpmovqd xmm14{k3},xmm4 */
    "62127e4d35e5",  /* vpmovqd ymm29{k5},zmm12 */
    "62927e8f35d0",  /* vpmovqd xmm24{k7}{z},xmm2 */
    "62327e4834d3",  /* vpmovqw xmm19,zmm10 */
    "62727e08343500010000",  /* vpmovqw DWORD PTR [rip+0x100],xmm14 */
    "62d27e283437",  /* vpmovqw QWORD PTR [r15],ymm6 */
    "62727e0e216459e0",  /* vpmovsdb DWORD PTR [rcx+rbx*2-0x80]{k6},xmm12 */
    "62427e2f21ef",  /* vpmovsdb xmm15{k7},ymm29 */
    "62627e4a210500010000",  /* vpmovsdb XMMWORD PTR [rip+0x100]{k2},zmm24 */
    "62927e0823f8",  /* vpmovsdw xmm24,xmm7 */
    "62e27e0823148d00100000",  /* vpmovsdw QWORD PTR [rcx*4+0x1000],xmm18 */
    "62627e4c23f7",  /* vpmovsdw ymm7{k4},zmm30 */
    "62727e28227a40",  /* vpmovsqb DWORD PTR [rdx+0x100],ymm15 */
    "62727e482234bd10000000",  /* vpmovsqb QWORD PTR [rdi*4+0x10],zmm14 */
    "62e27e08221500010000",  /* vpmovsqb WORD PTR [rip+0x100],xmm18 */
    "62327e0925147d10000000",  /* vpmovsqd QWORD PTR [r15*2+0x10]{k1},xmm10 */
    "62527e08259f00f0ffff",  /* vpmovsqd QWORD PTR [r15-0x1000],xmm11 */
    "62427e282553fc",  /* vpmovsqd XMMWORD PTR [r11-0x40],ymm26 */
    "62827e0824549b10",  /* vpmovsqw DWORD PTR [r11+r11*4+0x40],xmm18 */
    "62627e082435f8ffffff",  /* vpmovsqw DWORD PTR [rip+0xfffffffffffffff8],xmm30 */
    "62127ea924de",  /* vpmovsqw xmm30{k1}{z},ymm11 */
    "62e27e082025f8ffffff",  /* vpmovswb QWORD PTR [rip+0xfffffffffffffff8],xmm20 */
    "62e27e0a2005f8ffffff",  /* vpmovswb QWORD PTR [rip+0xfffffffffffffff8]{k2},xmm16 */
    "62027e4920d1",  /* vpmovswb ymm25{k1},zmm26 */
    "62027e2d11848300100000",  /* vpmovusdb QWORD PTR [r11+r8*4+0x1000]{k5},ymm24 */
    "62e27e48116b04",  /* vpmovusdb XMMWORD PTR [rbx+0x40],zmm21 */
    "62f27e28110500010000",  /* vpmovusdb QWORD PTR [rip+0x100],ymm0 */
    "62327e4d1324fd00100000",  /* vpmovusdw YMMWORD PTR [r15*8+0x1000]{k5},zmm12 */
    "62627e0a130c6d00100000",  /* vpmovusdw QWORD PTR [rbp*2+0x1000]{k2},xmm25 */
    "62427e2d13ba00100000",  /* vpmovusdw XMMWORD PTR [r10+0x1000]{k5},ymm31 */
    "62b27eac12e1",  /* vpmovusqb xmm17{k4}{z},ymm4 */
    "62f27e0a12fc",  /* vpmovusqb xmm4{k2},xmm7 */
    "62d27e4c12b600f0ffff",  /* vpmovusqb QWORD PTR [r14-0x1000]{k4},zmm6 */
    "62e27e8a15d7",  /* vpmovusqd xmm7{k2}{z},xmm18 */
    "62f27e0815e5",  /* vpmovusqd xmm5,xmm4 */
    "62927e2a15847500100000",  /* vpmovusqd XMMWORD PTR [r13+r14*2+0x1000]{k2},ymm0 */
    "62a27e0814147500100000",  /* vpmovusqw DWORD PTR [r14*2+0x1000],xmm18 */
    "62627e4d142d00010000",  /* vpmovusqw XMMWORD PTR [rip+0x100]{k5},zmm29 */
    "62927e8f14ed",  /* vpmovusqw xmm29{k7}{z},xmm5 */
    "62727e09106cb708",  /* vpmovuswb QWORD PTR [rdi+rsi*4+0x40]{k1},xmm13 */
    "62627e28103df8ffffff",  /* vpmovuswb XMMWORD PTR [rip+0xfffffffffffffff8],ymm31 */
    "62327e2b10cc",  /* vpmovuswb xmm20{k3},ymm9 */
    "62f27e2830fa",  /* vpmovwb xmm2,ymm7 */
    "62727e0e304a08",  /* vpmovwb QWORD PTR [rdx+0x40]{k6},xmm9 */
    "62f27eac30e1",  /* vpmovwb xmm1{k4}{z},ymm4 */
    "6222bd20282cbd10000000",  /* vpmuldq ymm29,ymm24,YMMWORD PTR [r15*4+0x10] */
    "6272fd44283df8ffffff",  /* vpmuldq zmm15{k4},zmm16,ZMMWORD PTR [rip+0xfffffffffffffff8] */
    "62e285d7281df8ffffff",  /* vpmuldq zmm19{k7}{z},zmm31,QWORD BCST [rip+0xfffffffffffffff8] */
    "62d1758be4e6",  /* vpmulhuw xmm4{k3}{z},xmm1,xmm14 */
    "c5a1e40c9510000000",  /* vpmulhuw xmm1,xmm11,XMMWORD PTR [rdx*4+0x10] */
    "62612528e40df8ffffff",  /* vpmulhuw ymm25,ymm11,YMMWORD PTR [rip+0xfffffffffffffff8] */
    "62715d05e57610",  /* vpmulhw xmm14{k5},xmm20,XMMWORD PTR [rsi+0x100] */
    "c559e50d00000000",  /* vpmulhw xmm9,xmm4,XMMWORD PTR [rip+0x0] */
    "62e13d4ae5c3",  /* vpmulhw zmm16{k2},zmm8,zmm3 */
    "c4c26d405491c0",  /* vpmulld ymm2,ymm2,YMMWORD PTR [r9+rdx*4-0x40] */
    "c4626940f4",  /* vpmulld xmm14,xmm2,xmm4 */
    "c44215409078563412",  /* vpmulld ymm10,ymm13,YMMWORD PTR [r8+0x12345678] */
    "62d10da9d5ef",  /* vpmullw ymm5{k1}{z},ymm14,ymm15 */
    "62e13d41d56cd940",  /* vpmullw zmm21{k1},zmm24,ZMMWORD PTR [rcx+rbx*8+0x1000] */
    "c42155d58c6680000000",  /* vpmullw ymm9,ymm5,YMMWORD PTR [rsi+r12*2+0x80] */
    "62129dce83ee",  /* vpmultishiftqb zmm13{k6}{z},zmm12,zmm30 */
    "6262a5488324dd10000000",  /* vpmultishiftqb zmm28,zmm11,ZMMWORD PTR [rbx*8+0x10] */
    "6282954883fa",  /* vpmultishiftqb zmm23,zmm13,zmm26 */
    "c44179f49300f0ffff",  /* vpmuludq xmm10,xmm0,XMMWORD PTR [r11-0x1000] */
    "c54df42d00010000",  /* vpmuludq ymm13,ymm6,YMMWORD PTR [rip+0x100] */
    "62b1ad58f43cd510000000",  /* vpmuludq zmm7,zmm10,QWORD BCST [r10*8+0x10] */
    "62227d2f547ce802",  /* vpopcntb ymm31{k7},YMMWORD PTR [rax+r13*8+0x40] */
    "62627d48540500000000",  /* vpopcntb zmm24,ZMMWORD PTR [rip+0x0] */
    "62727d28541500000000",  /* vpopcntb ymm10,YMMWORD PTR [rip+0x0] */
    "62d27d48558808000000",  /* vpopcntd zmm1,ZMMWORD PTR [r8+0x8] */
    "62727d48552d00010000",  /* vpopcntd zmm13,ZMMWORD PTR [rip+0x100] */
    "62c27d5c557240",  /* vpopcntd zmm22{k4},DWORD BCST [r10+0x100] */
    "6242fd9a556f08",  /* vpopcntq xmm29{k2}{z},QWORD BCST [r15+0x40] */
    "6282fdcd55e0",  /* vpopcntq zmm20{k5}{z},zmm24 */
    "6272fd38551d00000000",  /* vpopcntq ymm11,QWORD BCST [rip+0x0] */
    "6232fdcf54eb",  /* vpopcntw zmm13{k7}{z},zmm19 */
    "6222fd0e54f5",  /* vpopcntw xmm30{k6},xmm21 */
    "6282fd2d545c5cfe",  /* vpopcntw ymm19{k5},YMMWORD PTR [r12+r11*2-0x40] */
    "c541ebd7",  /* vpor xmm10,xmm7,xmm7 */
    "c44131eb1414",  /* vpor xmm10,xmm9,XMMWORD PTR [r12+rdx*1] */
    "c57deb2d00000000",  /* vpor ymm13,ymm0,YMMWORD PTR [rip+0x0] */
    "62211d28ebc6",  /* vpord ymm24,ymm12,ymm22 */
    "62713db1ebb500f0ffff",  /* vpord ymm14{k1}{z},ymm24,DWORD BCST [rbp-0x1000] */
    "628145c1ebc4",  /* vpord zmm16{k1}{z},zmm23,zmm28 */
    "6211858cebc0",  /* vporq xmm8{k4}{z},xmm15,xmm24 */
    "62f1ed28eb0d00010000",  /* vporq ymm1,ymm2,YMMWORD PTR [rip+0x100] */
    "6211fdc3ebe8",  /* vporq zmm13{k3}{z},zmm16,zmm24 */
    "c4c121f6df",  /* vpsadbw xmm3,xmm11,xmm15 */
    "c571f61500000000",  /* vpsadbw xmm10,xmm1,XMMWORD PTR [rip+0x0] */
    "c4c10df68b00010000",  /* vpsadbw ymm1,ymm14,YMMWORD PTR [r11+0x100] */
    "62d27d26a05c4520",  /* vpscatterdd DWORD PTR [r13+ymm16*2+0x80]{k6},ymm3 */
    "62b27d42a014bf",  /* vpscatterdd DWORD PTR [rdi+zmm31*4]{k2},zmm2 */
    "62127d45a07c8e40",  /* vpscatterdd DWORD PTR [r14+zmm25*4+0x100]{k5},zmm15 */
    "6252fd0ba0547d08",  /* vpscatterdq QWORD PTR [r13+xmm7*2+0x40]{k3},xmm10 */
    "6292fd0aa04c17f0",  /* vpscatterdq QWORD PTR [r15+xmm10*1-0x80]{k2},xmm1 */
    "6242fd05a07c6601",  /* vpscatterdq QWORD PTR [r14+xmm20*2+0x8]{k5},xmm31 */
    "62b27d2ea1742f10",  /* vpscatterqd DWORD PTR [rdi+ymm13*1+0x40]{k6},xmm6 */
    "62227d2ca12440",  /* vpscatterqd DWORD PTR [rax+ymm8*2]{k4},xmm28 */
    "62f27d41a17c5a40",  /* vpscatterqd DWORD PTR [rdx+zmm19*2+0x100]{k1},ymm7 */
    "6202fd4ea1945f78563412",  /* vpscatterqq QWORD PTR [r15+zmm11*2+0x12345678]{k6},zmm26 */
    "6202fd0da1646810",  /* vpscatterqq QWORD PTR [r8+xmm13*2+0x80]{k5},xmm28 */
    "62b2fd4ca17c76f0",  /* vpscatterqq QWORD PTR [rsi+zmm14*2-0x80]{k4},zmm7 */
    "62d2658f007204",  /* vpshufb xmm6{k7}{z},xmm3,XMMWORD PTR [r10+0x40] */
    "62826d8600645310",  /* vpshufb xmm20{k6}{z},xmm18,XMMWORD PTR [r11+r10*2+0x100] */
    "62d23da500d6",  /* vpshufb ymm2{k5}{z},ymm24,ymm14 */
    "62d2650e8f5810",  /* vpshufbitqmb k3{k6},xmm3,XMMWORD PTR [r8+0x100] */
    "62f27d2a8fd8",  /* vpshufbitqmb k3{k2},ymm0,ymm0 */
    "62f22d2b8feb",  /* vpshufbitqmb k5{k3},ymm10,ymm3 */
    "c57d701cad0010000004",  /* vpshufd ymm11,YMMWORD PTR [rbp*4+0x1000],0x4 */
    "c4c17970549680ce",  /* vpshufd xmm2,XMMWORD PTR [r14+rdx*4-0x80],0xce */
    "c57970216d",  /* vpshufd xmm12,XMMWORD PTR [rcx],0x6d */
    "c57a70458014",  /* vpshufhw xmm8,XMMWORD PTR [rbp-0x80],0x14 */
    "c4217a709ca100f0ffff47",  /* vpshufhw xmm11,XMMWORD PTR [rcx+r12*4-0x1000],0x47 */
    "c5fa701c4510000000a2",  /* vpshufhw xmm3,XMMWORD PTR [rax*2+0x10],0xa2 */
    "c4c17f70ea80",  /* vpshuflw ymm5,ymm10,0x80 */
    "c5ff701df8ffffff3e",  /* vpshuflw ymm3,YMMWORD PTR [rip+0xfffffffffffffff8],0x3e */
    "62d115b17273e025",  /* vpslld ymm29{k1}{z},DWORD BCST [r11-0x80],0x25 */
    "62b13dd97234b50010000086",  /* vpslld zmm8{k1}{z},DWORD BCST [r14*4+0x1000],0x86 */
    "c519f29300010000",  /* vpslld xmm10,xmm12,XMMWORD PTR [rbx+0x100] */
    "62f12d40737bfe08",  /* vpslldq zmm26,ZMMWORD PTR [rbx-0x80],0x8 */
    "c4c14d73f858",  /* vpslldq ymm6,ymm8,0x58 */
    "62b17d4073bcf108000000ab",  /* vpslldq zmm16,ZMMWORD PTR [rcx+r14*8+0x8],0xab */
    "62b1a500733475001000009a",  /* vpsllq xmm27,XMMWORD PTR [r14*2+0x1000],0x9a */
    "c4412df37471c0",  /* vpsllq ymm14,ymm10,XMMWORD PTR [r9+rsi*2-0x40] */
    "c5b5f33d00010000",  /* vpsllq ymm7,ymm9,XMMWORD PTR [rip+0x100] */
    "c4e22947b47900100000",  /* vpsllvd xmm6,xmm10,XMMWORD PTR [rcx+rdi*2+0x1000] */
    "62b2350547db",  /* vpsllvd xmm3{k5},xmm25,xmm19 */
    "6232758e47d5",  /* vpsllvd xmm10{k6}{z},xmm1,xmm21 */
    "62428520476c4e08",  /* vpsllvq ymm29,ymm31,YMMWORD PTR [r14+rcx*2+0x100] */
    "62e2f5834705f8ffffff",  /* vpsllvq xmm16{k3}{z},xmm17,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "6292d507474cabfc",  /* vpsllvq xmm1{k7},xmm21,XMMWORD PTR [r11+r13*4-0x40] */
    "62b17dc571f3e3",  /* vpsllw zmm16{k5}{z},zmm19,0xe3 */
    "c5b9f10df8ffffff",  /* vpsllw xmm1,xmm8,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "c571f1cf",  /* vpsllw xmm9,xmm1,xmm7 */
    "62f14d487225000000009b",  /* vpsrad zmm6,ZMMWORD PTR [rip+0x0],0x9b */
    "62917d4c72e637",  /* vpsrad zmm0{k4},zmm30,0x37 */
    "c5ede2247510000000",  /* vpsrad ymm4,ymm2,XMMWORD PTR [rsi*2+0x10] */
    "62b1f5a472e133",  /* vpsraq ymm17{k4}{z},ymm17,0x33 */
    "6291ed2d72e65a",  /* vpsraq ymm2{k5},ymm30,0x5a */
    "62f1fdb67266101c",  /* vpsraq ymm16{k6}{z},QWORD BCST [rsi+0x80],0x1c */
    "62621d30461d00000000",  /* vpsravd ymm27,ymm28,DWORD BCST [rip+0x0] */
    "6292450a46d6",  /* vpsravd xmm2{k2},xmm7,xmm30 */
    "6262754046ab08000000",  /* vpsravd zmm29,zmm17,ZMMWORD PTR [rbx+0x8] */
    "6272fdae46fa",  /* vpsravq ymm15{k6}{z},ymm0,ymm2 */
    "6212850046e7",  /* vpsravq xmm12,xmm31,xmm31 */
    "6252ed10465cf5f0",  /* vpsravq xmm11,xmm18,QWORD BCST [r13+rsi*8-0x80] */
    "6291354771e656",  /* vpsraw zmm25{k7},zmm30,0x56 */
    "62f15d2071647202f1",  /* vpsraw ymm20,YMMWORD PTR [rdx+rsi*2+0x40],0xf1 */
    "6291252171e243",  /* vpsraw ymm27{k1},ymm26,0x43 */
    "62f16dc2725340b7",  /* vpsrld zmm18{k2}{z},ZMMWORD PTR [rbx+0x1000],0xb7 */
    "c4211dd24472c0",  /* vpsrld ymm8,ymm12,XMMWORD PTR [rdx+r14*2-0x40] */
    "62d1058f72d010",  /* vpsrld xmm15{k7}{z},xmm8,0x10 */
    "62f14548731d00010000e2",  /* vpsrldq zmm7,ZMMWORD PTR [rip+0x100],0xe2 */
    "c5e173dbec",  /* vpsrldq xmm3,xmm3,0xec */
    "62d1b58273d441",  /* vpsrlq xmm25{k2}{z},xmm12,0x41 */
    "c5c9d305f8ffffff",  /* vpsrlq xmm0,xmm6,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "62f1ad487315f8ffffff1d",  /* vpsrlq zmm10,ZMMWORD PTR [rip+0xfffffffffffffff8],0x1d */
    "c4e2714535f8ffffff",  /* vpsrlvd xmm6,xmm1,XMMWORD PTR [rip+0xfffffffffffffff8] */
    "c4622945d6",  /* vpsrlvd xmm10,xmm10,xmm6 */
    "c4221d45b47778563412",  /* vpsrlvd ymm14,ymm12,YMMWORD PTR [rdi+r14*2+0x12345678] */
    "c4c28145c0",  /* vpsrlvq xmm0,xmm15,xmm8 */
    "62c2b58545f0",  /* vpsrlvq xmm22{k5}{z},xmm25,xmm8 */
    "6272cd5a455cadf8",  /* vpsrlvq zmm11{k2},zmm6,QWORD BCST [rbp+rbp*4-0x40] */
    "c5b1d10d00000000",  /* vpsrlw xmm1,xmm9,XMMWORD PTR [rip+0x0] */
    "c44131d15f40",  /* vpsrlw xmm11,xmm9,XMMWORD PTR [r15+0x40] */
    "c44169d1c4",  /* vpsrlw xmm8,xmm2,xmm12 */
    "62711d2ff85afc",  /* vpsubb ymm11{k7},ymm12,YMMWORD PTR [rdx-0x80] */
    "62715540f83d00000000",  /* vpsubb zmm15,zmm21,ZMMWORD PTR [rip+0x0] */
    "c59df8b900100000",  /* vpsubb ymm7,ymm12,YMMWORD PTR [rcx+0x1000] */
    "6261450dfaa45778563412",  /* vpsubd xmm28{k5},xmm7,XMMWORD PTR [rdi+rdx*2+0x12345678] */
    "c4c17dfae5",  /* vpsubd ymm4,ymm0,ymm13 */
    "62f145b9fa04b500100000",  /* vpsubd ymm0{k1}{z},ymm7,DWORD BCST [rsi*4+0x1000] */
    "c5e1fb5bc0",  /* vpsubq xmm3,xmm3,XMMWORD PTR [rbx-0x40] */
    "c5b5fb3d00000000",  /* vpsubq ymm7,ymm9,YMMWORD PTR [rip+0x0] */
    "62e1cd8afb9cd578563412",  /* vpsubq xmm19{k2}{z},xmm6,XMMWORD PTR [rbp+rdx*8+0x12345678] */
    "6261352ee80d00010000",  /* vpsubsb ymm25{k6},ymm9,YMMWORD PTR [rip+0x100] */
    "62e1252de835f8ffffff",  /* vpsubsb ymm22{k5},ymm11,YMMWORD PTR [rip+0xfffffffffffffff8] */
    "c40161e88c3d00010000",  /* vpsubsb xmm9,xmm3,XMMWORD PTR [r13+r15*1+0x100] */
    "62f15548e92b",  /* vpsubsw zmm5,zmm5,ZMMWORD PTR [rbx] */
    "62e14508e90d00010000",  /* vpsubsw xmm17,xmm7,XMMWORD PTR [rip+0x100] */
    "62112d40e98c8b78563412",  /* vpsubsw zmm9,zmm26,ZMMWORD PTR [r11+r9*4+0x12345678] */
    "62615548d80d00010000",  /* vpsubusb zmm25,zmm5,ZMMWORD PTR [rip+0x100] */
    "c4c161d8b900010000",  /* vpsubusb xmm7,xmm3,XMMWORD PTR [r9+0x100] */
    "62717d8ed8a4d700f0ffff",  /* vpsubusb xmm12{k6}{z},xmm0,XMMWORD PTR [rdi+rdx*8-0x1000] */
    "62a17d28d9647ffc",  /* vpsubusw ymm20,ymm0,YMMWORD PTR [rdi+r15*2-0x80] */
    "62313dc2d9de",  /* vpsubusw zmm11{k2}{z},zmm24,zmm22 */
    "c44111d96c2480",  /* vpsubusw xmm13,xmm13,XMMWORD PTR [r12-0x80] */
    "62c17dabf92f",  /* vpsubw ymm21{k3}{z},ymm0,YMMWORD PTR [r15] */
    "62714dc3f935f8ffffff",  /* vpsubw zmm14{k3}{z},zmm22,ZMMWORD PTR [rip+0xfffffffffffffff8] */
    "62710546f91cad10000000",  /* vpsubw zmm11{k6},zmm31,ZMMWORD PTR [rbp*4+0x10] */
    "62532dd3254cd7e08d",  /* vpternlogd zmm9{k3}{z},zmm26,DWORD BCST [r15+rdx*8-0x80],0x8d */
    "62334d8425bc4f7856341219",  /* vpternlogd xmm15{k4}{z},xmm22,XMMWORD PTR [rdi+r9*2+0x12345678],0x19 */
    "62e36d2825aa080000003a",  /* vpternlogd ymm21,ymm2,YMMWORD PTR [rdx+0x8],0x3a */
    "62e3a52d25ec51",  /* vpternlogq ymm21{k5},ymm11,ymm4,0x51 */
    "62e3c5182525f8ffffffa1",  /* vpternlogq xmm20,xmm7,QWORD BCST [rip+0xfffffffffffffff8],0xa1 */
    "6283b58a25d284",  /* vpternlogq xmm18{k2}{z},xmm9,xmm26,0x84 */
    "c55168d9",  /* vpunpckhbw xmm11,xmm5,xmm1 */
    "c5f168a678563412",  /* vpunpckhbw xmm4,xmm1,XMMWORD PTR [rsi+0x12345678] */
    "c5bd683df8ffffff",  /* vpunpckhbw ymm7,ymm8,YMMWORD PTR [rip+0xfffffffffffffff8] */
    "62d12d486a61c0",  /* vpunpckhdq zmm4,zmm10,ZMMWORD PTR [r9-0x1000] */
    "c5cd6a1df8ffffff",  /* vpunpckhdq ymm3,ymm6,YMMWORD PTR [rip+0xfffffffffffffff8] */
    "c4a17d6a4c8e40",  /* vpunpckhdq ymm1,ymm0,YMMWORD PTR [rsi+r9*4+0x40] */
    "6261cdca6d8c7b78563412",  /* vpunpckhqdq zmm25{k2}{z},zmm6,ZMMWORD PTR [rbx+rdi*2+0x12345678] */
    "c4a1516dbc0300100000",  /* vpunpckhqdq xmm7,xmm5,XMMWORD PTR [rbx+r8*1+0x1000] */
    "62e1a5486d4c49ff",  /* vpunpckhqdq zmm17,zmm11,ZMMWORD PTR [rcx+rcx*2-0x40] */
    "c519695b40",  /* vpunpckhwd xmm11,xmm12,XMMWORD PTR [rbx+0x40] */
    "62a1450a696ceefc",  /* vpunpckhwd xmm21{k2},xmm7,XMMWORD PTR [rsi+r13*8-0x40] */
    "62e15520692d00010000",  /* vpunpckhwd ymm21,ymm21,YMMWORD PTR [rip+0x100] */
    "c5b160048d00100000",  /* vpunpcklbw xmm0,xmm9,XMMWORD PTR [rcx*4+0x1000] */
    "c50560147500100000",  /* vpunpcklbw ymm10,ymm15,YMMWORD PTR [rsi*2+0x1000] */
    "62e1054f600df8ffffff",  /* vpunpcklbw zmm17{k7},zmm15,ZMMWORD PTR [rip+0xfffffffffffffff8] */
    "62e17d8f6224ed10000000",  /* vpunpckldq xmm20{k7}{z},xmm0,XMMWORD PTR [rbp*8+0x10] */
    "c54d62348500100000",  /* vpunpckldq ymm14,ymm6,YMMWORD PTR [rax*4+0x1000] */
    "c4c11562f7",  /* vpunpckldq ymm6,ymm13,ymm15 */
    "62d1a5006c02",  /* vpunpcklqdq xmm0,xmm27,XMMWORD PTR [r10] */
    "c5216c2d00000000",  /* vpunpcklqdq xmm13,xmm11,XMMWORD PTR [rip+0x0] */
    "c5bd6c0500000000",  /* vpunpcklqdq ymm0,ymm8,YMMWORD PTR [rip+0x0] */
    "62210d0d61f3",  /* vpunpcklwd xmm30{k5},xmm14,xmm19 */
    "626155a961e0",  /* vpunpcklwd ymm28{k1}{z},ymm5,ymm0 */
    "c50561bd78563412",  /* vpunpcklwd ymm15,ymm15,YMMWORD PTR [rbp+0x12345678] */
    "c4c13deffa",  /* vpxor ymm7,ymm8,ymm10 */
    "c42121ef145500100000",  /* vpxor xmm10,xmm11,XMMWORD PTR [r10*2+0x1000] */
    "c4214def1c9d10000000",  /* vpxor ymm11,ymm6,YMMWORD PTR [r11*4+0x10] */
    "62f16d48efd8",  /* vpxord zmm3,zmm2,zmm0 */
    "627105c2eff6",  /* vpxord zmm14{k2}{z},zmm31,zmm6 */
    "62e15d47efc3",  /* vpxord zmm16{k7},zmm20,zmm3 */
    "62b1ad55ef1cc500100000",  /* vpxorq zmm3{k5},zmm26,QWORD BCST [r8*8+0x1000] */
    "62a19d87efde",  /* vpxorq xmm19{k7}{z},xmm28,xmm22 */
    "6281fd51efb44578563412",  /* vpxorq zmm22{k1},zmm16,QWORD BCST [r13+r8*2+0x12345678] */
    "6252fd43a254ab10",  /* vscatterdpd QWORD PTR [r11+ymm21*4+0x80]{k3},zmm10 */
    "6272fd4fa2649120",  /* vscatterdpd QWORD PTR [rcx+ymm2*4+0x100]{k7},zmm12 */
    "62f2fd43a25c67f0",  /* vscatterdpd QWORD PTR [rdi+ymm20*2-0x80]{k3},zmm3 */
    "62227d03a24ceb20",  /* vscatterdps DWORD PTR [rbx+xmm29*8+0x80]{k3},xmm25 */
    "62427d45a2743a10",  /* vscatterdps DWORD PTR [r10+zmm23*1+0x40]{k5},zmm30 */
    "62127d4ea27c63e0",  /* vscatterdps DWORD PTR [r11+zmm12*2-0x80]{k6},zmm15 */
    "6202fd01a3bca278563412",  /* vscatterqpd QWORD PTR [r10+xmm28*4+0x12345678]{k1},xmm31 */
    "6282fd0aa39c3000f0ffff",  /* vscatterqpd QWORD PTR [r8+xmm14*1-0x1000]{k2},xmm19 */
    "6212fd4da31cc4",  /* vscatterqpd QWORD PTR [r12+zmm8*8]{k5},zmm11 */
    "62a27d24a3544e02",  /* vscatterqps DWORD PTR [rsi+ymm25*2+0x8]{k4},xmm18 */
    "62727d26a37495f0",  /* vscatterqps DWORD PTR [rbp+ymm18*4-0x40]{k6},xmm14 */
    "62827d2ba3b47a00f0ffff",  /* vscatterqps DWORD PTR [r10+ymm15*2-0x1000]{k3},xmm22 */
    "6241d5c5c6dc61",  /* vshufpd zmm27{k5}{z},zmm21,zmm12,0x61 */
    "62218d20c60c850010000036",  /* vshufpd ymm25,ymm30,YMMWORD PTR [r8*4+0x1000],0x36 */
    "62619d20c63d000000003b",  /* vshufpd ymm31,ymm28,YMMWORD PTR [rip+0x0],0x3b */
    "c40150c6a4df00f0ffff30",  /* vshufps xmm12,xmm5,XMMWORD PTR [r15+r11*8-0x1000],0x30 */
    "627114cfc62d000000001e",  /* vshufps zmm13{k7}{z},zmm13,ZMMWORD PTR [rip+0x0],0x1e */
    "62f14cc6c6d538",  /* vshufps zmm2{k6}{z},zmm22,zmm5,0x38 */
    "c4a17d51348500100000",  /* vsqrtpd ymm6,YMMWORD PTR [r8*4+0x1000] */
    "62d1fd8b51fd",  /* vsqrtpd xmm7{k3}{z},xmm13 */
    "c5fd5135f8ffffff",  /* vsqrtpd ymm6,YMMWORD PTR [rip+0xfffffffffffffff8] */
    "c4417c51740fac",  /* vsqrtps ymm14,YMMWORD PTR [r15+rcx*1-0x54] */
    "c4a17851a4f0d5000000",  /* vsqrtps xmm4,XMMWORD PTR [rax+r14*8+0xd5] */
    "62717c7851c7",  /* vsqrtps zmm8,zmm7{rz-sae} */
    "6281e70d51dd",  /* vsqrtsd xmm19{k5},xmm3,xmm29 */
    "c50351e0",  /* vsqrtsd xmm12,xmm15,xmm0 */
    "6241ff7851e3",  /* vsqrtsd xmm28,xmm0,xmm11{rz-sae} */
    "c582515d00",  /* vsqrtss xmm3,xmm15,DWORD PTR [rbp+0x0] */
    "62510e5051c1",  /* vsqrtss xmm8,xmm30,xmm9{ru-sae} */
    "c4c102514424c0",  /* vsqrtss xmm0,xmm15,DWORD PTR [r12-0x40] */
    "c54d5cd5",  /* vsubpd ymm10,ymm6,ymm5 */
    "62b1d5225c642204",  /* vsubpd ymm4{k2},ymm21,YMMWORD PTR [rdx+r12*1+0x80] */
    "c4417d5c6608",  /* vsubpd ymm12,ymm0,YMMWORD PTR [r14+0x8] */
    "62b1749a5c0c8d10000000",  /* vsubps xmm1{k2}{z},xmm1,DWORD BCST [r9*4+0x10] */
    "c4c1405cc6",  /* vsubps xmm0,xmm7,xmm14 */
    "623164205c8c9f00100000",  /* vsubps ymm9,ymm19,YMMWORD PTR [rdi+r11*4+0x1000] */
    "c4c13b5c6e08",  /* vsubsd xmm5,xmm8,QWORD PTR [r14+0x8] */
    "6241f7085c7cbaf8",  /* vsubsd xmm31,xmm1,QWORD PTR [r10+rdi*4-0x40] */
    "62b1a7305cd9",  /* vsubsd xmm3,xmm27,xmm17{rd-sae} */
    "c4814a5c948429ffffff",  /* vsubss xmm2,xmm6,DWORD PTR [r12+r8*4-0xd7] */
    "629166305cdc",  /* vsubss xmm3,xmm19,xmm28{rd-sae} */
    "62d17e595cc6",  /* vsubss xmm0{k1},xmm0,xmm14{ru-sae} */
    "62e1fd082e2ced10000000",  /* vucomisd xmm21,QWORD PTR [rbp*8+0x10] */
    "c5f92ea900100000",  /* vucomisd xmm5,QWORD PTR [rcx+0x1000] */
    "c441792e749340",  /* vucomisd xmm14,QWORD PTR [r11+rdx*4+0x40] */
    "c4c1782eb41680000000",  /* vucomiss xmm6,DWORD PTR [r14+rdx*1+0x80] */
    "62d17c182ecf",  /* vucomiss xmm1,xmm15{sae} */
    "c441782ef3",  /* vucomiss xmm14,xmm11 */
    "62e18dda157b20",  /* vunpckhpd zmm23{k2}{z},zmm14,QWORD BCST [rbx+0x100] */
    "c521159078563412",  /* vunpckhpd xmm10,xmm11,XMMWORD PTR [rax+0x12345678] */
    "c5051510",  /* vunpckhpd ymm10,ymm15,YMMWORD PTR [rax] */
    "62f16c3b156140",  /* vunpckhps ymm4{k3},ymm2,DWORD BCST [rcx+0x100] */
    "6221144015244d10000000",  /* vunpckhps zmm28,zmm29,ZMMWORD PTR [r9*2+0x10] */
    "c4217415448708",  /* vunpckhps ymm8,ymm1,YMMWORD PTR [rdi+r8*4+0x8] */
    "c5e914b49300010000",  /* vunpcklpd xmm6,xmm2,XMMWORD PTR [rbx+rdx*4+0x100] */
    "62319d4414d1",  /* vunpcklpd zmm10{k4},zmm28,zmm17 */
    "62f18d4b146ec0",  /* vunpcklpd zmm5{k3},zmm14,ZMMWORD PTR [rsi-0x1000] */
    "62a1742b14f2",  /* vunpcklps ymm22{k3},ymm1,ymm18 */
    "c4c108146f40",  /* vunpcklps xmm5,xmm14,XMMWORD PTR [r15+0x40] */
    "c548141a",  /* vunpcklps xmm11,xmm6,XMMWORD PTR [rdx] */
    "c5455784f300010000",  /* vxorpd ymm8,ymm7,YMMWORD PTR [rbx+rsi*8+0x100] */
    "62719510573500010000",  /* vxorpd xmm14,xmm29,QWORD BCST [rip+0x100] */
    "6271d58f57e8",  /* vxorpd xmm13{k7}{z},xmm5,xmm0 */
    "c598570c4500100000",  /* vxorps xmm1,xmm12,XMMWORD PTR [rax*2+0x1000] */
    "c53857d6",  /* vxorps xmm10,xmm8,xmm6 */
    "c5e0577cd500",  /* vxorps xmm7,xmm3,XMMWORD PTR [rbp+rdx*8+0x0] */
    "0f09",  /* wbinvd */
    "0f30",  /* wrmsr */
    "440f38f60500010000",  /* wrssd [rip+0x100],r8d */
    "0f38f68180000000",  /* wrssd [rcx+0x80],eax */
    "420f38f68c7000100000",  /* wrssd [rax+r14*2+0x1000],ecx */
    "490f38f6b45300010000",  /* wrssq [r11+rdx*2+0x100],rsi */
    "4c0f38f64608",  /* wrssq [rsi+0x8],r8 */
    "4c0f38f63d00000000",  /* wrssq [rip+0x0],r15 */
    "66470f38f59ca400010000",  /* wrussd [r12+r12*4+0x100],r11d */
    "66450f38f50c6e",  /* wrussd [r14+rbp*2],r9d */
    "66470f38f58c3400f0ffff",  /* wrussd [r12+r14*1-0x1000],r9d */
    "664e0f38f57c2a40",  /* wrussq [rdx+r13*1+0x40],r15 */
    "66480f38f50df8ffffff",  /* wrussq [rip+0xfffffffffffffff8],rcx */
    "66490f38f54b08",  /* wrussq [r11+0x8],rcx */
    "66440fc1f9",  /* xadd cx,r15w */
    "6748877b80",  /* xchg QWORD PTR [ebx-0x80],rdi */
    "6487a578563412",  /* xchg DWORD PTR fs:[rbp+0x12345678],esp */
    "4c878c7078563412",  /* xchg QWORD PTR [rax+rsi*2+0x12345678],r9 */
    "d7",  /* xlat BYTE PTR ds:[rbx] */
    "6766443138",  /* xor WORD PTR [eax+0x0],r15w */
    "664231542880",  /* xor WORD PTR [rax+r13*1-0x80],dx */
    "49817424c080000000",  /* xor QWORD PTR [r12-0x40],0x80 */
    "640f5701",  /* xorps xmm0,XMMWORD PTR fs:[rcx] */
    "0f570d00000000",  /* xorps xmm1,XMMWORD PTR [rip+0x0] */
    "67440f57607f",  /* xorps xmm12,XMMWORD PTR [eax+0x7f] */
    "410faeab00010000",  /* xrstor [r11+0x100] */
    "0fae2df8ffffff",  /* xrstor [rip+0xfffffffffffffff8] */
    "410fae6c0a40",  /* xrstor [r10+rcx*1+0x40] */
    "480fae2df8ffffff",  /* xrstor64 [rip+0xfffffffffffffff8] */
    "480faeac8d00f0ffff",  /* xrstor64 [rbp+rcx*4-0x1000] */
    "480fae2c4500100000",  /* xrstor64 [rax*2+0x1000] */
    "0fc75808",  /* xrstors [rax+0x8] */
    "420fc71cf500100000",  /* xrstors [r14*8+0x1000] */
    "0fc71d00000000",  /* xrstors [rip+0x0] */
    "4a0fc71c7d00100000",  /* xrstors64 [r15*2+0x1000] */
    "4a0fc79c1600100000",  /* xrstors64 [rsi+r10*1+0x1000] */
    "490fc75908",  /* xrstors64 [r9+0x8] */
    "410fae6180",  /* xsave [r9-0x80] */
    "420fae245d00100000",  /* xsave [r11*2+0x1000] */
    "0fae25f8ffffff",  /* xsave [rip+0xfffffffffffffff8] */
    "480fae2500000000",  /* xsave64 [rip+0x0] */
    "480fae248500100000",  /* xsave64 [rax*4+0x1000] */
    "490fae641d08",  /* xsave64 [r13+rbx*1+0x8] */
    "420fc7244d00100000",  /* xsavec [r9*2+0x1000] */
    "0fc725f8ffffff",  /* xsavec [rip+0xfffffffffffffff8] */
    "430fc7a4a300010000",  /* xsavec [r11+r12*4+0x100] */
    "4b0fc724e6",  /* xsavec64 [r14+r12*8] */
    "490fc76540",  /* xsavec64 [r13+0x40] */
    "480fc725f8ffffff",  /* xsavec64 [rip+0xfffffffffffffff8] */
    "410faeb300f0ffff",  /* xsaveopt [r11-0x1000] */
    "420fae746f08",  /* xsaveopt [rdi+r13*2+0x8] */
    "0fae35f8ffffff",  /* xsaveopt [rip+0xfffffffffffffff8] */
    "480fae73c0",  /* xsaveopt64 [rbx-0x40] */
    "480fae346d10000000",  /* xsaveopt64 [rbp*2+0x10] */
    "480faeb078563412",  /* xsaveopt64 [rax+0x12345678] */
    "0fc72cd510000000",  /* xsaves [rdx*8+0x10] */
    "410fc76dc0",  /* xsaves [r13-0x40] */
    "410fc7ac0700100000",  /* xsaves [r15+rax*1+0x1000] */
    "490fc76ec0",  /* xsaves64 [r14-0x40] */
    "480fc72d00000000",  /* xsaves64 [rip+0x0] */
    "480fc72c9510000000",  /* xsaves64 [rdx*4+0x10] */
    "62b1650872b40a7c0000001c",  /* {evex} vpslld xmm3,XMMWORD PTR [rdx+r9*1+0x7c],0x1c */
    "62f11528731d0000000032",  /* {evex} vpsrldq ymm13,YMMWORD PTR [rip+0x0],0x32 */
    "62f18d28731200",  /* {evex} vpsrlq ymm14,YMMWORD PTR [rdx],0x0 */
    "b805000000",  /* mov eax, 5 */
    "488b448b08",  /* mov rax, QWORD PTR [rbx+rcx*4+8] */
    "800001",  /* add BYTE PTR [rax], 1 */
    "0fb607",  /* movzx eax, BYTE PTR [rdi] */
    "480fbf4702",  /* movsx rax, WORD PTR [rdi+2] */
    "486307",  /* movsxd rax, DWORD PTR [rdi] */
    "488d0404",  /* lea rax, [rsp+rax] */
    "ffd0",  /* call rax */
    "ff24c510000000",  /* jmp QWORD PTR [rax*8+0x10] */
    "db28",  /* fld TBYTE PTR [rax] */
    "df28",  /* fild QWORD PTR [rax] */
    "dce9",  /* fsub st(1), st */
    "dee9",  /* fsubp st(1), st */
    "d8e1",  /* fsub st, st(1) */
    "62f174d95800",  /* vaddps zmm0{k1}{z}, zmm1, DWORD PTR [rax]{1to16} */
    "62f1741858c2",  /* vaddps zmm0, zmm1, zmm2, {rn-sae} */
    "f20f2a00",  /* cvtsi2sd xmm0, DWORD PTR [rax] */
    "64488b042528000000",  /* mov rax, QWORD PTR fs:0x28 */
    "648b00",  /* mov eax, DWORD PTR fs:[rax] */
    "a5",  /* movsd */
    "f348ab",  /* rep stosq */
    "6bc305",  /* imul eax, ebx, 5 */
    "d320",  /* shl DWORD PTR [rax], cl */
    "2e660f1f0400",  /* nop WORD PTR cs:[rax+rax*1+0x0] */
    "6a05",  /* push 5 */
    "c8100000",  /* enter 16, 0 */
    "62f27d29900488",  /* vpgatherdd ymm0{k1}, [rax+ymm1*4] */
    "660fc40001",  /* pinsrw xmm0, WORD PTR [rax], 1 */
    "66480f7ec0",  /* movq rax, xmm0 */
    "0f6e00",  /* movd mm0, DWORD PTR [rax] */
    "f20f38f000",  /* crc32 eax, BYTE PTR [rax] */
    "48b88877665544332211",  /* movabs rax, 0x1122334455667788 */
    "64a4",  /* movs byte ptr es:[rdi], byte ptr fs:[rsi] */
    "65ad",  /* lods dword ptr gs:[rsi] */
    "6e",  /* outs dx, byte ptr [rsi] */
    "6d",  /* ins dword ptr es:[rdi], dx */
    "d7",  /* xlat byte ptr [rbx] */
    "48a5",  /* movs qword ptr [rdi], qword ptr [rsi] */
    "f3aa",  /* rep stos byte ptr [rdi], al */
    "a6",  /* cmps byte ptr [rsi], byte ptr es:[rdi] */
    "ae",  /* scas al, byte ptr es:[rdi] */
    "c5e8c2cb01",  /* vcmpltps xmm1, xmm2, xmm3 */
    "c5edc20819",  /* vcmpnge_uqpd ymm1, ymm2, YMMWORD PTR [rax] */
    "c5eac248041f",  /* vcmptrue_usss xmm1, xmm2, DWORD PTR [rax+4] */
    "62f16c4ac2cb00",  /* vcmpeqps k1{k2}, zmm2, zmm3 */
    "62f36d481ecb01",  /* vpcmpltud k1, zmm2, zmm3 */
    "62f3dd591f1806",  /* vpcmpnleq k3{k1}, zmm4, QWORD PTR [rax]{1to8} */
    "62f36d483ecb00",  /* vpcmpequb k1, zmm2, zmm3 */
    "62f3ed283fcb04",  /* vpcmpneqw k1, ymm2, ymm3 */
    "b701",  /* mov bh, 1 */
    "b480",  /* mov ah, 0x80 */
    "8a28",  /* mov ch, BYTE PTR [rax] */
    "dee8",  /* fsubp st(0), st */
    "def1",  /* fdivrp */
    "62f16c1858d9",  /* vaddps zmm3, zmm2, zmm1{rn-sae} */
    "62f16c585818",  /* vaddps zmm3, zmm2, DWORD BCST [rax] */
    "8b041c",  /* mov eax, [rbx + rsp] */
    "8b448bf8",  /* mov eax, [4*rcx + rbx - 8] */
    "8b4014",  /* mov eax, DWORD PTR [rax + (2+3)*4] */
    "8b45f8",  /* mov eax, DWORD PTR -8[rbp] */
    "268b00",  /* mov eax, DWORD PTR es:[rax] */
    "89d8",  /* mov %eax, %ebx */
    "8b03",  /* mov eax, [%rbx] */
    "678d0448",  /* lea eax, [eax + ecx*2] */
    "ff10",  /* call [rax] */
    "64ff20",  /* jmp QWORD PTR fs:[rax] */
    "ff3500000000",  /* push QWORD PTR [rip] */
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
