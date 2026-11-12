/* flags: --c-std=gnu17 */
/* native-only */
/* The x86-64 assembler's extensions against a byte table (x87, MMX, SSE2-4.2, AVX/AVX2, FMA, F16C, AVX-512 F/VL/BW/DQ with masking, broadcast and rounding, and system instructions); the bytes were checked against the system assembler in development. */
#include "host.h"

#ifdef __AXIOM_ASM_X86_64__
__asm__(".section .rodata.axiom_asm_table, \"a\"\n"
        ".globl axiom_asm_table\n"
        "axiom_asm_table:\n"
        "fadd %st, %st(2)\n"
        "fadd %st(1), %st\n"
        "fadds (%rsi)\n"
        "faddl (%r8,%r8,8)\n"
        "fsub %st, %st(3)\n"
        "fsub %st(1), %st\n"
        "fsubs (%r8)\n"
        "fsubl (%rbp,%r12,1)\n"
        "fmul %st, %st(7)\n"
        "fmul %st(4), %st\n"
        "fmuls 256(%r15)\n"
        "fmull -225(%rbx,%rax,1)\n"
        "fdiv %st, %st(0)\n"
        "fdiv %st(6), %st\n"
        "fdivs 127(%r14)\n"
        "fdivl (%r15)\n"
        "fsubr %st, %st(3)\n"
        "fsubr %st(7), %st\n"
        "fsubrs (%r8)\n"
        "fsubrl -20(%rdx,%r14,2)\n"
        "fdivr %st, %st(7)\n"
        "fdivr %st(4), %st\n"
        "fdivrs (%rax,%r9,1)\n"
        "fdivrl -133(%rcx,%r15,4)\n"
        "fld %st(6)\n"
        "fst %st(3)\n"
        "fstp %st(4)\n"
        "fxch %st(4)\n"
        "ffree %st(7)\n"
        "flds (%r9,%r12,1)\n"
        "fldl 305419896(%r8)\n"
        "fsts (%rbp,%r14,2)\n"
        "fstl (%rdi)\n"
        "fstps -168(%r15,%r8,1)\n"
        "fstpl (%rcx)\n"
        "filds 245(%rbp,%r15,1)\n"
        "fildl (%r8)\n"
        "fistp (%rsi)\n"
        "fildll (%r12)\n"
        "faddp\n"
        "fmulp\n"
        "fcompp\n"
        "fucompp\n"
        "fcos\n"
        "fsin\n"
        "fsqrt\n"
        "fldpi\n"
        "fld1\n"
        "fldz\n"
        "fnstsw %ax\n"
        "fxam\n"
        "fprem\n"
        "paddb -8(%rbp), %mm3\n"
        "paddw 256(%rax), %mm3\n"
        "paddd %mm5, %mm5\n"
        "paddq -251(%r8,%rbp,2), %mm3\n"
        "psubb %mm7, %mm5\n"
        "pand %mm3, %mm6\n"
        "por 168(%r8,%rdi,1), %mm5\n"
        "pxor %mm0, %mm3\n"
        "pcmpeqb %mm2, %mm1\n"
        "pmullw %mm1, %mm0\n"
        "punpcklbw %mm4, %mm3\n"
        "pmaddwd %mm2, %mm5\n"
        "psllw $9, %mm1\n"
        "pslld $5, %mm2\n"
        "psrlq $8, %mm2\n"
        "psraw $8, %mm4\n"
        "movq %mm0, %mm4\n"
        "movd %r8d, %mm5\n"
        "pshufw $215, %mm6, %mm0\n"
        "vaddps %xmm5, %xmm14, %xmm13\n"
        "vsubps %xmm14, %xmm7, %xmm0\n"
        "vmulps %ymm10, %ymm13, %ymm1\n"
        "vdivps %ymm9, %ymm2, %ymm2\n"
        "vminps %ymm8, %ymm4, %ymm0\n"
        "vmaxps %xmm14, %xmm5, %xmm1\n"
        "vaddpd %ymm6, %ymm15, %ymm3\n"
        "vmulpd %ymm15, %ymm0, %ymm10\n"
        "vsubpd %ymm5, %ymm6, %ymm10\n"
        "vandps %xmm3, %xmm12, %xmm11\n"
        "vorpd %ymm1, %ymm2, %ymm4\n"
        "vxorps %xmm6, %xmm8, %xmm10\n"
        "vunpcklps %ymm7, %ymm15, %ymm4\n"
        "vunpckhpd %xmm13, %xmm2, %xmm12\n"
        "vaddss %xmm3, %xmm12, %xmm2\n"
        "vmulsd %xmm7, %xmm2, %xmm8\n"
        "vsubss %xmm3, %xmm1, %xmm9\n"
        "vdivsd %xmm0, %xmm2, %xmm13\n"
        "vminss %xmm6, %xmm7, %xmm13\n"
        "vmaxsd %xmm14, %xmm5, %xmm7\n"
        "vsqrtss %xmm13, %xmm12, %xmm9\n"
        "vsqrtps %ymm6, %ymm10\n"
        "vsqrtpd %xmm9, %xmm10\n"
        "vcmpps $25, %ymm10, %ymm14, %ymm3\n"
        "vcmppd $13, %ymm15, %ymm11, %ymm8\n"
        "vcmpss $2, %xmm6, %xmm7, %xmm11\n"
        "vshufps $143, %xmm7, %xmm12, %xmm9\n"
        "vshufpd $167, %xmm3, %xmm2, %xmm7\n"
        "vblendps $10, %xmm2, %xmm2, %xmm0\n"
        "vblendpd $148, %xmm3, %xmm10, %xmm2\n"
        "vpblendw $91, %xmm9, %xmm3, %xmm9\n"
        "vdpps $105, %xmm1, %xmm10, %xmm6\n"
        "vpaddb %xmm1, %xmm7, %xmm8\n"
        "vpaddd %xmm14, %xmm14, %xmm0\n"
        "vpaddq %ymm15, %ymm0, %ymm13\n"
        "vpsubw %xmm4, %xmm8, %xmm8\n"
        "vpand %ymm7, %ymm15, %ymm0\n"
        "vpor %xmm7, %xmm7, %xmm10\n"
        "vpxor %ymm10, %ymm8, %ymm7\n"
        "vpcmpeqb %xmm11, %xmm5, %xmm6\n"
        "vpcmpgtd %ymm2, %ymm3, %ymm12\n"
        "vpmaxsd %xmm15, %xmm12, %xmm11\n"
        "vpminud %ymm1, %ymm2, %ymm8\n"
        "vpmulld %xmm4, %xmm2, %xmm14\n"
        "vpunpcklbw %xmm10, %xmm14, %xmm4\n"
        "vpmaddwd %ymm13, %ymm13, %ymm3\n"
        "vpshufb %ymm0, %ymm6, %ymm14\n"
        "vpshufd $1, %xmm7, %xmm8\n"
        "vpslld $11, %xmm6, %xmm8\n"
        "vpsrlq $16, %ymm15, %ymm13\n"
        "vmovaps %xmm12, %xmm6\n"
        "vmovaps %xmm9, (%rbx)\n"
        "vmovups %xmm9, %xmm4\n"
        "vmovups %xmm2, 62(%r9,%rbp,4)\n"
        "vmovapd %ymm14, %ymm14\n"
        "vmovapd %ymm11, (%rsi)\n"
        "vmovdqa %ymm15, %ymm3\n"
        "vmovdqa %ymm12, 256(%rbp)\n"
        "vmovdqu (%rsi), %xmm6\n"
        "vmovdqu %xmm14, (%r12)\n"
        "vmovd %r8d, %xmm9\n"
        "vmovq %r15, %xmm5\n"
        "vmovss (%r8), %xmm6\n"
        "vmovsd %xmm11, %xmm0, %xmm12\n"
        "vfmadd132ps %ymm9, %ymm0, %ymm13\n"
        "vfmadd213pd %xmm2, %xmm0, %xmm11\n"
        "vfmsub231ps %ymm4, %ymm14, %ymm8\n"
        "vfnmadd213ps %ymm8, %ymm3, %ymm13\n"
        "vfmadd213ss %xmm0, %xmm5, %xmm5\n"
        "vfnmsub231sd %xmm12, %xmm8, %xmm9\n"
        "vcvtph2ps %xmm6, %ymm7\n"
        "vcvtps2ph $0, %ymm10, %xmm2\n"
        "vcvtdq2ps %ymm11, %ymm14\n"
        "vcvttps2dq %xmm1, %xmm5\n"
        "vcvtsi2sd %rsi, %xmm8, %xmm11\n"
        "vbroadcastss %xmm12, %ymm5\n"
        "vpbroadcastd %xmm15, %ymm8\n"
        "vpermq $168, %ymm7, %ymm0\n"
        "vperm2f128 $206, %ymm10, %ymm13, %ymm7\n"
        "vinsertf128 $1, %xmm8, %ymm6, %ymm2\n"
        "vextracti128 $1, %ymm5, %xmm8\n"
        "vpinsrb $3, %r10d, %xmm5, %xmm4\n"
        "vpextrd $2, %xmm4, %r10d\n"
        "vaddps %zmm13, %zmm19, %zmm4\n"
        "vaddps %zmm20, %zmm31, %zmm6{%k2}\n"
        "vaddps %zmm2, %zmm3, %zmm1{%k7}{z}\n"
        "vmulpd %zmm31, %zmm28, %zmm21\n"
        "vmulpd %zmm6, %zmm14, %zmm25{%k2}\n"
        "vmulpd %zmm31, %zmm28, %zmm24{%k7}{z}\n"
        "vpaddd %zmm18, %zmm29, %zmm24\n"
        "vpaddd %zmm21, %zmm31, %zmm7{%k2}\n"
        "vpaddd %zmm5, %zmm2, %zmm0{%k7}{z}\n"
        "vpaddq %zmm18, %zmm12, %zmm25\n"
        "vpaddq %zmm0, %zmm24, %zmm9{%k6}\n"
        "vpaddq %zmm3, %zmm24, %zmm16{%k2}{z}\n"
        "vpxord %zmm0, %zmm2, %zmm3\n"
        "vpxord %zmm17, %zmm7, %zmm27{%k1}\n"
        "vpxord %zmm12, %zmm1, %zmm31{%k6}{z}\n"
        "vpandq %zmm24, %zmm21, %zmm17\n"
        "vpandq %zmm3, %zmm11, %zmm22{%k4}\n"
        "vpandq %zmm3, %zmm22, %zmm26{%k5}{z}\n"
        "vpmaxsd %zmm27, %zmm4, %zmm17\n"
        "vpmaxsd %zmm4, %zmm16, %zmm11{%k1}\n"
        "vpmaxsd %zmm9, %zmm3, %zmm13{%k7}{z}\n"
        "vaddps {rn-sae}, %zmm1, %zmm2, %zmm3\n"
        "vmulpd {ru-sae}, %zmm4, %zmm5, %zmm6{%k2}\n"
        "vsqrtps {rz-sae}, %zmm7, %zmm8\n"
        "vaddps (%rax){1to16}, %zmm2, %zmm3\n"
        "vpaddd (%rbx){1to16}, %zmm2, %zmm3\n"
        "vaddpd 128(%rax){1to8}, %zmm2, %zmm3{%k1}\n"
        "vpcmpd $6, %zmm2, %zmm3, %k6\n"
        "vpcmpud $1, %zmm30, %zmm23, %k1\n"
        "vpcmpq $5, %zmm2, %zmm8, %k5\n"
        "vpternlogd $0x55, %zmm1, %zmm2, %zmm3\n"
        "vpternlogq $0xca, (%rax), %zmm2, %zmm3{%k1}\n"
        "vmovdqu32 %zmm2, %zmm28{%k6}{z}\n"
        "vmovdqa64 %zmm30, 64(%rax)\n"
        "vbroadcastss %xmm3, %zmm5\n"
        "vpbroadcastq %xmm1, %zmm2{%k3}\n"
        "vinsertf32x4 $1, %xmm2, %zmm3, %zmm4\n"
        "vextractf64x4 $1, %zmm5, %ymm6\n"
        "kmovw %k1, %k2\n"
        "kmovb %eax, %k3\n"
        "kandw %k1, %k2, %k3\n"
        "kortestq %k4, %k5\n"
        "knotb %k1, %k2\n"
        "vaddps %zmm20, %zmm21, %zmm22\n"
        "vpaddd %zmm31, %zmm30, %zmm29{%k7}\n"
        "vmulps %ymm18, %ymm19, %ymm20\n"
        "vaddsd %xmm16, %xmm17, %xmm18\n"
        "vfmadd213ps %zmm1, %zmm2, %zmm3\n"
        "vfnmadd231pd {rn-sae}, %zmm4, %zmm5, %zmm6\n"
        "cli\n"
        "sti\n"
        "hlt\n"
        "swapgs\n"
        "wrmsr\n"
        "rdmsr\n"
        "iretq\n"
        "sysretq\n"
        "invlpg (%rax)\n"
        "lgdt (%rbx)\n"
        "sidt 8(%rsp)\n"
        "mov %cr0, %rax\n"
        "mov %rax, %cr3\n"
        "mov %db0, %rcx\n"
        "mov %rdx, %dr7\n"
        "ltr %ax\n"
        "lldt %cx\n"
        "wbinvd\n"
        "clts\n"
        "xsave (%rdi)\n"
        "xrstor64 (%rsi)\n"
        "fxsave (%rax)\n"
        "endbr32\n"
        "endbr64\n"
        "rdpid %rax\n"
        "fadd %st, %st(0)\n"
        "fadds -83(%rbx,%r15,4)\n"
        "faddl (%rsi)\n"
        "fsub %st(0), %st\n"
        "fsubs 127(%r12)\n"
        "fsubl (%r14,%r15,4)\n"
        "fmuls (%rax)\n"
        "fmull (%rdi,%rdi,8)\n"
        "fdiv %st, %st(6)\n"
        "fdiv %st(2), %st\n"
        "fdivs -8(%r9)\n"
        "fdivl (%rdx)\n"
        "fsubr %st, %st(2)\n"
        "fsubr %st(5), %st\n"
        "fsubrs 256(%rcx)\n"
        "fsubrl -70(%r9,%r8,8)\n"
        "fdivr %st, %st(5)\n"
        "fdivr %st(5), %st\n"
        "fdivrs (%rdi,%rcx,8)\n"
        "fdivrl (%r15,%r14,2)\n"
        "fld %st(7)\n"
        "fst %st(4)\n"
        "fstp %st(7)\n"
        "fxch %st(5)\n"
        "flds 211(%r8,%r8,2)\n"
        "fldl 256(%rdi)\n"
        "fsts (%rsi,%rsi,4)\n"
        "fstl (%r15)\n"
        "fstps (%r9)\n"
        "fstpl (%r9)\n"
        "filds (%r12,%rsi,2)\n"
        "fistp (%rdi)\n"
        "fildll -248(%rbx,%rdx,1)\n"
        "paddb %mm0, %mm4\n"
        "paddw %mm1, %mm2\n"
        "paddd %mm0, %mm6\n"
        "paddq %mm0, %mm5\n"
        "psubb %mm0, %mm1\n"
        "pand %mm0, %mm0\n"
        "por %mm5, %mm4\n"
        "pxor %mm2, %mm0\n"
        "pcmpeqb %mm0, %mm3\n"
        "pmullw %mm0, %mm5\n"
        "punpcklbw %mm4, %mm5\n"
        "pmaddwd %mm4, %mm7\n"
        "psllw $1, %mm4\n"
        "pslld $12, %mm2\n"
        "psrlq $15, %mm3\n"
        "psraw $2, %mm5\n"
        "movq %mm7, %mm2\n"
        "movd %r13d, %mm6\n"
        "pshufw $249, %mm4, %mm6\n"
        "vaddps %xmm4, %xmm1, %xmm8\n"
        "vsubps %xmm3, %xmm14, %xmm7\n"
        "vmulps %xmm14, %xmm2, %xmm8\n"
        "vdivps %xmm11, %xmm8, %xmm13\n"
        "vminps %ymm4, %ymm1, %ymm12\n"
        "vmaxps (%rcx), %ymm2, %ymm7\n"
        "vaddpd %xmm5, %xmm7, %xmm3\n"
        "vmulpd %xmm14, %xmm14, %xmm9\n"
        "vsubpd %ymm13, %ymm13, %ymm0\n"
        "vandps %xmm5, %xmm3, %xmm15\n"
        "vorpd %ymm3, %ymm11, %ymm9\n"
        "vxorps %ymm13, %ymm3, %ymm3\n"
        "vunpcklps %ymm14, %ymm1, %ymm13\n"
        "vunpckhpd %ymm2, %ymm0, %ymm9\n"
        "vaddss %xmm7, %xmm15, %xmm6\n"
        "vmulsd %xmm11, %xmm12, %xmm14\n"
        "vsubss %xmm3, %xmm3, %xmm2\n"
        "vdivsd %xmm0, %xmm15, %xmm1\n"
        "vminss %xmm14, %xmm4, %xmm11\n"
        "vmaxsd %xmm13, %xmm15, %xmm9\n"
        "vsqrtss %xmm15, %xmm8, %xmm13\n"
        "vsqrtps %xmm3, %xmm2\n"
        "vsqrtpd %ymm4, %ymm13\n"
        "vcmpps $5, %xmm4, %xmm9, %xmm12\n"
        "vcmppd $21, %xmm9, %xmm3, %xmm4\n"
        "vcmpss $6, %xmm5, %xmm5, %xmm14\n"
        "vshufps $206, %xmm4, %xmm14, %xmm14\n"
        "vshufpd $196, 127(%r15), %xmm1, %xmm15\n"
        "vblendps $207, %ymm11, %ymm10, %ymm2\n"
        "vblendpd $96, %xmm10, %xmm14, %xmm14\n"
        "vpblendw $48, %xmm6, %xmm3, %xmm12\n"
        "vdpps $140, %xmm6, %xmm15, %xmm4\n"
        "vpaddb (%r12,%r8,4), %xmm5, %xmm14\n"
        "vpaddd %xmm15, %xmm10, %xmm14\n"
        "vpaddq %ymm11, %ymm5, %ymm12\n"
        "vpsubw %ymm5, %ymm15, %ymm12\n"
        "vpand %ymm14, %ymm0, %ymm11\n"
        "vpor %xmm6, %xmm9, %xmm15\n"
        "vpxor %xmm9, %xmm2, %xmm8\n"
        "vpcmpeqb 63(%rsi,%r14,8), %ymm2, %ymm6\n"
        "vpcmpgtd %ymm4, %ymm2, %ymm9\n"
        "vpmaxsd (%rdx,%r9,2), %xmm8, %xmm1\n"
        "vpminud %xmm10, %xmm11, %xmm2\n"
        "vpmulld %ymm9, %ymm14, %ymm4\n"
        "vpunpcklbw %ymm10, %ymm5, %ymm3\n"
        "vpmaddwd %xmm11, %xmm5, %xmm11\n"
        "vpshufb %xmm12, %xmm12, %xmm10\n"
        "vpshufd $20, %ymm2, %ymm11\n"
        "vpslld $25, %ymm11, %ymm14\n"
        "vpsrlq $5, %ymm3, %ymm11\n"
        "vmovaps %xmm13, %xmm0\n"
        "vmovaps %xmm10, (%rdx)\n"
        "vmovups %ymm11, %ymm10\n"
        "vmovups %ymm6, (%r8)\n"
        "vmovapd %xmm11, %xmm8\n"
        "vmovapd %xmm2, -8(%rdi)\n"
        "vmovdqa %xmm1, %xmm10\n"
        "vmovdqa %xmm11, (%r14)\n"
        "vmovdqu %xmm13, %xmm14\n"
        "vmovdqu %xmm8, -137(%rcx,%rdi,8)\n"
        "vmovd %edx, %xmm1\n"
        "vmovq %rbp, %xmm15\n"
        "vmovss (%r8), %xmm10\n"
        "vmovsd %xmm2, %xmm15, %xmm12\n"
        "vfmadd132ps %ymm1, %ymm3, %ymm14\n"
        "vfmadd213pd %xmm5, %xmm2, %xmm12\n"
        "vfmsub231ps %ymm8, %ymm3, %ymm11\n"
        "vfnmadd213ps %xmm4, %xmm13, %xmm2\n"
        "vfmadd213ss %xmm7, %xmm2, %xmm15\n"
        "vfnmsub231sd %xmm0, %xmm4, %xmm1\n"
        "vcvtph2ps %xmm0, %ymm10\n"
        "vcvtps2ph $0, %ymm7, %xmm3\n"
        "vcvtdq2ps %ymm14, %ymm6\n"
        "vcvttps2dq %xmm6, %xmm12\n"
        "vcvtsi2sd %rdi, %xmm12, %xmm5\n"
        "vbroadcastss %xmm4, %ymm6\n"
        "vpbroadcastd %xmm5, %ymm12\n"
        "vpermq $103, %ymm4, %ymm12\n"
        "vperm2f128 $160, %ymm9, %ymm3, %ymm3\n"
        "vinsertf128 $1, %xmm15, %ymm8, %ymm9\n"
        "vextracti128 $1, %ymm15, %xmm4\n"
        "vpinsrb $3, %r13d, %xmm3, %xmm0\n"
        "vpextrd $2, %xmm6, %edx\n"
        "vaddps %zmm8, %zmm1, %zmm16\n"
        "vaddps %zmm14, %zmm9, %zmm20{%k1}\n"
        "vaddps %zmm12, %zmm6, %zmm8{%k6}{z}\n"
        "vmulpd %zmm11, %zmm5, %zmm29\n"
        "vmulpd %zmm17, %zmm4, %zmm26{%k4}\n"
        "vmulpd %zmm2, %zmm29, %zmm19{%k6}{z}\n"
        "vpaddd %zmm21, %zmm16, %zmm25\n"
        "vpaddd %zmm12, %zmm27, %zmm8{%k6}\n"
        "vpaddd %zmm10, %zmm28, %zmm29{%k3}{z}\n"
        "vpaddq %zmm12, %zmm30, %zmm28\n"
        "vpaddq %zmm19, %zmm4, %zmm10{%k3}\n"
        "vpaddq %zmm30, %zmm14, %zmm8{%k6}{z}\n"
        "vpxord %zmm19, %zmm6, %zmm0\n"
        "vpxord %zmm3, %zmm20, %zmm16{%k7}\n"
        "vpxord %zmm21, %zmm28, %zmm4{%k4}{z}\n"
        "vpandq %zmm18, %zmm8, %zmm13\n"
        "vpandq %zmm24, %zmm4, %zmm28{%k3}\n"
        "vpandq %zmm5, %zmm31, %zmm30{%k7}{z}\n"
        "vpmaxsd %zmm19, %zmm14, %zmm12\n"
        "vpmaxsd %zmm15, %zmm13, %zmm3{%k3}\n"
        "vpmaxsd %zmm15, %zmm8, %zmm25{%k7}{z}\n"
        "vpcmpd $6, %zmm7, %zmm29, %k4\n"
        "vpcmpud $6, %zmm30, %zmm24, %k3\n"
        "vpcmpq $3, %zmm15, %zmm14, %k1\n"
        "vmovdqu32 %zmm5, %zmm0{%k1}{z}\n"
        "fadd %st, %st(3)\n"
        "fadd %st(2), %st\n"
        "fadds (%rdi)\n"
        "faddl (%r8)\n"
        "fsub %st, %st(1)\n"
        "fsubs -17(%r8,%rdx,8)\n"
        "fsubl (%r9)\n"
        "fmul %st(6), %st\n"
        "fmuls -8(%r14)\n"
        "fmull 256(%r14)\n"
        "fdiv %st(0), %st\n"
        "fdivs (%r14)\n"
        "fdivl (%rcx)\n"
        "fsubr %st, %st(0)\n"
        "fsubr %st(4), %st\n"
        "fsubrs 228(%rax,%r12,8)\n"
        "fsubrl (%r15,%rbp,8)\n"
        "fdivr %st, %st(2)\n"
        "fdivrs (%rbx)\n"
        "fdivrl (%rcx,%rdx,4)\n"
        "fstp %st(6)\n"
        "fxch %st(6)\n"
        "ffree %st(5)\n"
        "flds (%r9)\n"
        "fldl (%rbp)\n"
        "fsts -227(%rdx,%rsi,2)\n"
        "fstl -150(%r15,%r15,2)\n"
        "fstps (%r14)\n"
        "fstpl -129(%rsi,%rbx,8)\n"
        "filds (%r14,%rbx,4)\n"
        "fildl (%rbx,%rcx,1)\n"
        "fistp (%rsi,%rbp,1)\n"
        "fildll (%rax)\n"
        "paddb %mm6, %mm5\n"
        "paddw %mm0, %mm1\n"
        "paddd (%rbx), %mm0\n"
        "paddq %mm2, %mm0\n"
        "psubb %mm6, %mm7\n"
        "pand (%r9,%r14,1), %mm4\n"
        "por %mm6, %mm4\n"
        "pxor %mm5, %mm0\n"
        "pcmpeqb %mm2, %mm0\n"
        "pmullw %mm4, %mm7\n"
        "punpcklbw %mm0, %mm0\n"
        "psllw $5, %mm5\n"
        "pslld $11, %mm4\n"
        "psrlq $9, %mm6\n"
        "psraw $3, %mm0\n"
        "movq 8(%r12), %mm3\n"
        "movd %esi, %mm3\n"
        "pshufw $167, %mm1, %mm5\n"
        "vaddps %ymm5, %ymm2, %ymm10\n"
        "vsubps %xmm3, %xmm1, %xmm6\n"
        "vmulps %ymm10, %ymm2, %ymm11\n"
        "vdivps %xmm13, %xmm9, %xmm13\n"
        "vminps %ymm0, %ymm15, %ymm13\n"
        "vmaxps %xmm10, %xmm7, %xmm2\n"
        "vaddpd %ymm1, %ymm6, %ymm13\n"
        "vmulpd %xmm5, %xmm9, %xmm7\n"
        "vsubpd %xmm13, %xmm1, %xmm3\n"
        "vandps %ymm7, %ymm6, %ymm3\n"
        "vorpd %xmm4, %xmm0, %xmm15\n"
        "vxorps %ymm1, %ymm15, %ymm10\n"
        "vunpcklps %xmm3, %xmm1, %xmm2\n"
        "vunpckhpd (%rax), %ymm15, %ymm10\n"
        "vaddss %xmm12, %xmm9, %xmm11\n"
        "vmulsd %xmm13, %xmm3, %xmm4\n"
        "vsubss %xmm12, %xmm2, %xmm5\n"
        "vdivsd %xmm1, %xmm13, %xmm1\n"
        "vminss %xmm13, %xmm13, %xmm14\n"
        "vmaxsd %xmm8, %xmm2, %xmm13\n"
        "vsqrtss %xmm10, %xmm11, %xmm8\n"
        "vsqrtps (%r8), %xmm12\n"
        "vsqrtpd %xmm15, %xmm4\n"
        "vcmpps $24, %xmm2, %xmm3, %xmm12\n"
        "vcmppd $1, %xmm0, %xmm3, %xmm15\n"
        "vcmpss $4, %xmm3, %xmm3, %xmm1\n"
        "vshufps $92, %ymm7, %ymm14, %ymm12\n"
        "vshufpd $188, %ymm2, %ymm12, %ymm7\n"
        "vblendps $82, (%rbp), %ymm15, %ymm4\n"
        "vblendpd $76, %ymm15, %ymm15, %ymm14\n"
        "vpblendw $69, -8(%rsi), %xmm10, %xmm7\n"
        "vdpps $211, %ymm8, %ymm6, %ymm9\n"
        "vpaddb %xmm5, %xmm11, %xmm7\n"
        "vpaddd %ymm15, %ymm6, %ymm14\n"
        "vpaddq %xmm12, %xmm1, %xmm14\n"
        "vpsubw %xmm6, %xmm8, %xmm7\n"
        "vpand %xmm1, %xmm8, %xmm5\n"
        "vpor %xmm2, %xmm2, %xmm3\n"
        "vpxor %xmm10, %xmm0, %xmm0\n"
        "vpcmpeqb %ymm2, %ymm6, %ymm15\n"
        "vpcmpgtd %ymm10, %ymm3, %ymm8\n"
        "vpmaxsd (%r14,%rbx,8), %xmm8, %xmm3\n"
        "vpminud %ymm3, %ymm10, %ymm3\n"
        "vpmulld %ymm5, %ymm4, %ymm5\n"
        "vpunpcklbw %ymm4, %ymm10, %ymm13\n"
        "vpmaddwd %ymm15, %ymm9, %ymm5\n"
        "vpshufb %xmm12, %xmm11, %xmm3\n"
        "vpshufd $17, %ymm4, %ymm1\n"
        "vpslld $17, %ymm11, %ymm10\n"
        "vpsrlq $28, %ymm11, %ymm15\n"
        "vmovaps %xmm3, %xmm5\n"
        "vmovaps %xmm6, (%r12,%r14,8)\n"
        "vmovups %xmm4, %xmm12\n"
        "vmovups %xmm6, (%r9)\n"
        "vmovapd %xmm8, %xmm11\n"
        "vmovapd %xmm9, (%rax,%rbp,8)\n"
        "vmovdqa %ymm9, %ymm15\n"
        "vmovdqa %ymm9, (%r14,%rax,2)\n"
        "vmovdqu %xmm5, %xmm14\n"
        "vmovdqu %xmm6, (%rdx)\n"
        "vmovq %r9, %xmm14\n"
        "vmovss (%rbx), %xmm9\n"
        "vmovsd %xmm4, %xmm4, %xmm14\n"
        "vfmadd132ps %xmm0, %xmm11, %xmm7\n"
        "vfmadd213pd %xmm0, %xmm10, %xmm10\n"
        "vfmsub231ps %ymm1, %ymm2, %ymm10\n"
        "vfnmadd213ps %xmm7, %xmm15, %xmm10\n"
        "vfmadd213ss %xmm13, %xmm2, %xmm6\n"
        "vfnmsub231sd %xmm15, %xmm15, %xmm2\n"
        "vcvtph2ps %xmm9, %ymm0\n"
        "vcvtps2ph $0, %ymm14, %xmm14\n"
        "vcvtdq2ps 119(%rax,%rdi,8), %ymm11\n"
        "vcvttps2dq %xmm6, %xmm8\n"
        "vcvtsi2sd %rdi, %xmm4, %xmm14\n"
        "vbroadcastss %xmm6, %ymm0\n"
        "vpermq $86, %ymm4, %ymm3\n"
        "vperm2f128 $86, %ymm14, %ymm15, %ymm5\n"
        "vinsertf128 $1, %xmm1, %ymm0, %ymm12\n"
        "vextracti128 $1, %ymm14, %xmm7\n"
        "vpinsrb $3, %r8d, %xmm1, %xmm12\n"
        "vpextrd $2, %xmm15, %eax\n"
        "vaddps %zmm24, %zmm30, %zmm12\n"
        "vaddps %zmm3, %zmm18, %zmm17{%k7}\n"
        "vaddps %zmm29, %zmm19, %zmm31{%k2}{z}\n"
        "vmulpd %zmm20, %zmm5, %zmm3\n"
        "vmulpd %zmm6, %zmm1, %zmm5{%k1}\n"
        "vmulpd %zmm10, %zmm2, %zmm30{%k1}{z}\n"
        "vpaddd %zmm21, %zmm12, %zmm30\n"
        "vpaddd %zmm24, %zmm19, %zmm25{%k1}\n"
        "vpaddd %zmm18, %zmm11, %zmm26{%k1}{z}\n"
        "vpaddq %zmm25, %zmm11, %zmm24\n"
        "vpaddq %zmm28, %zmm14, %zmm28{%k7}\n"
        "vpaddq %zmm30, %zmm22, %zmm17{%k7}{z}\n"
        "vpxord %zmm24, %zmm31, %zmm2\n"
        "vpxord %zmm1, %zmm29, %zmm5{%k7}\n"
        "vpxord %zmm6, %zmm20, %zmm15{%k5}{z}\n"
        "vpandq %zmm3, %zmm28, %zmm29\n"
        "vpandq %zmm12, %zmm25, %zmm6{%k3}\n"
        "vpandq %zmm19, %zmm7, %zmm28{%k1}{z}\n"
        "vpmaxsd %zmm3, %zmm9, %zmm9\n"
        "vpmaxsd %zmm7, %zmm14, %zmm18{%k2}\n"
        "vpmaxsd %zmm14, %zmm26, %zmm20{%k7}{z}\n"
        "vpcmpd $3, %zmm29, %zmm11, %k5\n"
        "vpcmpud $1, %zmm2, %zmm7, %k5\n"
        "vpcmpq $0, %zmm6, %zmm12, %k3\n"
        "vmovdqu32 %zmm5, %zmm6{%k4}{z}\n"
        "fadd %st(4), %st\n"
        "fadds (%rbx,%r8,2)\n"
        "faddl (%rbx)\n"
        "fsub %st, %st(0)\n"
        "fsub %st(6), %st\n"
        "fsubs -196(%r9,%rdx,4)\n"
        "fsubl -128(%rsi)\n"
        "fmul %st, %st(4)\n"
        "fmul %st(3), %st\n"
        "fmuls 22(%rax,%rdx,2)\n"
        "fmull 125(%rsi,%rbx,4)\n"
        "fdiv %st(3), %st\n"
        "fdivs 127(%rcx)\n"
        "fdivl (%rsi)\n"
        "fsubr %st, %st(4)\n"
        "fsubr %st(0), %st\n"
        "fsubrs (%rsi)\n"
        "fsubrl -57(%r15,%rbp,8)\n"
        "fdivr %st, %st(4)\n"
        "fdivr %st(6), %st\n"
        "fdivrs -8(%r8)\n"
        "fdivrl -212(%rsi,%rbx,1)\n"
        "ffree %st(2)\n"
        "flds -128(%r14)\n"
        "fldl 305419896(%rbp)\n"
        "fsts (%r14,%rsi,2)\n"
        "fstl (%rdi,%r15,4)\n"
        "fstpl -153(%rdx,%rax,2)\n"
        "filds (%rsi)\n"
        "fildl -128(%r12)\n"
        "fistp 8(%rdi)\n"
        "fildll (%r8)\n"
        "paddb %mm0, %mm5\n"
        "paddw %mm1, %mm4\n"
        "paddd %mm4, %mm2\n"
        "paddq %mm5, %mm0\n"
        "psubb %mm7, %mm2\n"
        "pand %mm7, %mm3\n"
        "por %mm0, %mm0\n"
        "pcmpeqb %mm5, %mm0\n"
        "pmullw %mm4, %mm6\n"
        "punpcklbw %mm3, %mm3\n"
        "pmaddwd %mm3, %mm6\n"
        "psllw $14, %mm3\n"
        "pslld $13, %mm3\n"
        "psraw $1, %mm0\n"
        "movq %mm3, %mm6\n"
        "movd %esi, %mm2\n"
        "pshufw $166, %mm0, %mm7\n"
        "vaddps %ymm10, %ymm9, %ymm15\n"
        "vsubps %ymm6, %ymm8, %ymm10\n"
        "vmulps %ymm8, %ymm6, %ymm1\n"
        "vdivps %ymm1, %ymm5, %ymm14\n"
        "vminps %ymm0, %ymm6, %ymm5\n"
        "vmaxps %xmm7, %xmm1, %xmm6\n"
        "vaddpd %xmm10, %xmm15, %xmm14\n"
        "vmulpd %xmm3, %xmm15, %xmm8\n"
        "vsubpd %xmm9, %xmm11, %xmm2\n"
        "vandps %xmm9, %xmm12, %xmm7\n"
        "vorpd %ymm2, %ymm3, %ymm11\n"
        "vxorps %ymm9, %ymm15, %ymm13\n"
        "vunpcklps %xmm5, %xmm5, %xmm4\n"
        "vunpckhpd %ymm5, %ymm0, %ymm9\n"
        "vaddss %xmm3, %xmm15, %xmm15\n"
        "vmulsd (%r9), %xmm7, %xmm13\n"
        "vsubss %xmm1, %xmm10, %xmm14\n"
        "vdivsd %xmm12, %xmm4, %xmm8\n"
        "vminss %xmm2, %xmm4, %xmm5\n"
        "vmaxsd %xmm2, %xmm2, %xmm12\n"
        "vsqrtss %xmm11, %xmm10, %xmm5\n"
        "vsqrtps %ymm0, %ymm2\n"
        "vsqrtpd %ymm15, %ymm2\n"
        "vcmpps $11, %xmm9, %xmm12, %xmm7\n"
        "vcmppd $14, -21(%rsi,%r15,4), %ymm14, %ymm12\n"
        "vcmpss $6, %xmm8, %xmm5, %xmm14\n"
        "vshufps $206, %ymm0, %ymm4, %ymm2\n"
        "vshufpd $177, %ymm7, %ymm15, %ymm2\n"
        "vblendps $122, %xmm12, %xmm8, %xmm0\n"
        "vblendpd $80, %xmm10, %xmm2, %xmm3\n"
        "vpblendw $31, %ymm13, %ymm4, %ymm2\n"
        "vdpps $1, %ymm4, %ymm13, %ymm4\n"
        "vpaddb %xmm12, %xmm5, %xmm5\n"
        "vpaddd %ymm12, %ymm10, %ymm5\n"
        "vpaddq %ymm0, %ymm9, %ymm4\n"
        "vpsubw %xmm0, %xmm7, %xmm7\n"
        "vpand %xmm5, %xmm14, %xmm12\n"
        "vpor %ymm6, %ymm3, %ymm15\n"
        "vpxor %ymm6, %ymm8, %ymm9\n"
        "vpcmpeqb %ymm3, %ymm1, %ymm10\n"
        "vpcmpgtd %ymm5, %ymm1, %ymm15\n"
        "vpmaxsd %xmm2, %xmm10, %xmm4\n"
        "vpminud (%rbx), %ymm0, %ymm2\n"
        "vpmulld %xmm0, %xmm15, %xmm4\n"
        "vpunpcklbw %xmm11, %xmm9, %xmm12\n"
        "vpmaddwd %ymm15, %ymm2, %ymm9\n"
        "vpshufb %ymm6, %ymm0, %ymm15\n"
        "vpshufd $17, %ymm10, %ymm6\n"
        "vpslld $17, %ymm0, %ymm12\n"
        "vpsrlq $18, %ymm10, %ymm2\n"
        "vmovaps %xmm6, %xmm8\n"
        "vmovaps %xmm6, (%r14,%rbx,4)\n"
        "vmovups %xmm6, %xmm5\n"
        "vmovups %xmm2, (%rdx)\n"
        "vmovapd %xmm2, %xmm5\n"
        "vmovapd %xmm0, 8(%rcx)\n"
        "vmovdqa %xmm5, %xmm5\n"
        "vmovdqa %xmm6, (%rax)\n"
        "vmovdqu %xmm8, %xmm15\n"
        "vmovdqu %xmm3, 127(%rdx)\n"
        "vmovd %r10d, %xmm15\n"
        "vmovq %r9, %xmm10\n"
        "vmovss (%rdi,%r15,8), %xmm1\n"
        "vmovsd %xmm1, %xmm5, %xmm4\n"
        "vfmadd132ps %ymm11, %ymm12, %ymm11\n"
        "vfmadd213pd %xmm5, %xmm6, %xmm0\n"
        "vfmsub231ps %xmm10, %xmm15, %xmm1\n"
        "vfnmadd213ps %ymm12, %ymm14, %ymm11\n"
        "vfmadd213ss %xmm14, %xmm4, %xmm14\n"
        "vfnmsub231sd %xmm10, %xmm9, %xmm1\n"
        "vcvtph2ps %xmm5, %ymm7\n"
        "vcvtps2ph $0, %ymm10, %xmm0\n"
        "vcvtdq2ps %ymm1, %ymm4\n"
        "vcvttps2dq %xmm9, %xmm7\n"
        "vcvtsi2sd %rbp, %xmm10, %xmm11\n"
        "vbroadcastss %xmm1, %ymm4\n"
        "vpbroadcastd %xmm11, %ymm6\n"
        "vpermq $253, %ymm2, %ymm6\n"
        "vperm2f128 $81, %ymm4, %ymm4, %ymm1\n"
        "vinsertf128 $1, %xmm11, %ymm2, %ymm1\n"
        "vextracti128 $1, %ymm4, %xmm1\n"
        "vpinsrb $3, %esi, %xmm15, %xmm8\n"
        "vpextrd $2, %xmm6, %r10d\n"
        "vaddps %zmm3, %zmm8, %zmm18\n"
        "vaddps %zmm0, %zmm22, %zmm23{%k1}\n"
        "vaddps %zmm12, %zmm27, %zmm5{%k7}{z}\n"
        "vmulpd %zmm18, %zmm12, %zmm12\n"
        "vmulpd %zmm5, %zmm5, %zmm0{%k1}\n"
        "vmulpd %zmm18, %zmm27, %zmm4{%k1}{z}\n"
        "vpaddd -84(%r9,%r14,2), %zmm3, %zmm3\n"
        "vpaddd %zmm8, %zmm6, %zmm20{%k5}\n"
        "vpaddd %zmm21, %zmm17, %zmm8{%k5}{z}\n"
        "vpaddq %zmm4, %zmm0, %zmm10\n"
        "vpaddq %zmm9, %zmm23, %zmm29{%k2}\n"
        "vpaddq %zmm28, %zmm10, %zmm30{%k6}{z}\n"
        "vpxord %zmm1, %zmm12, %zmm16\n"
        "vpxord %zmm28, %zmm21, %zmm10{%k3}\n"
        "vpxord %zmm10, %zmm20, %zmm19{%k7}{z}\n"
        "vpandq %zmm30, %zmm18, %zmm17\n"
        "vpandq %zmm6, %zmm14, %zmm21{%k6}\n"
        "vpandq %zmm12, %zmm11, %zmm28{%k1}{z}\n"
        "vpmaxsd %zmm0, %zmm2, %zmm0\n"
        "vpmaxsd %zmm11, %zmm14, %zmm15{%k3}\n"
        "vpmaxsd %zmm25, %zmm24, %zmm15{%k6}{z}\n"
        "vpcmpd $7, %zmm27, %zmm18, %k5\n"
        "vpcmpud $1, %zmm21, %zmm2, %k1\n"
        "vpcmpq $2, %zmm3, %zmm26, %k1\n"
        "vmovdqu32 %zmm25, %zmm10{%k4}{z}\n"
        "fadd %st, %st(4)\n"
        "fadd %st(5), %st\n"
        "fadds (%r15)\n"
        "faddl (%rax,%rdx,1)\n"
        "fsub %st, %st(2)\n"
        "fsubs (%rdi,%rdx,8)\n"
        "fmul %st, %st(3)\n"
        "fmul %st(0), %st\n"
        "fmuls 127(%r15)\n"
        "fmull 127(%rsi)\n"
        "fdiv %st, %st(2)\n"
        "fdiv %st(1), %st\n"
        "fdivs (%rcx)\n"
        "fdivl (%r12,%rcx,2)\n"
        "fsubrs -8(%rdx)\n"
        "fsubrl 65(%rcx,%rdx,2)\n"
        "fdivr %st(3), %st\n"
        "fdivrs -234(%rbp,%rdi,8)\n"
        "fdivrl 8(%rcx)\n"
        "fld %st(1)\n"
        "fst %st(5)\n"
        "fxch %st(0)\n"
        "flds 107(%rbx,%rsi,8)\n"
        "fldl -67(%r15,%r8,8)\n"
        "fsts -128(%r15)\n"
        "fstl (%rsi)\n"
        "fstps 157(%r15,%rax,8)\n"
        "fstpl (%rdi,%r12,1)\n"
        "filds (%r8)\n"
        "fildl 256(%r15)\n"
        "fistp (%rdx)\n"
        "fildll (%rdx,%rdi,4)\n"
        "paddb %mm4, %mm0\n"
        "paddw %mm3, %mm5\n"
        "paddd %mm5, %mm2\n"
        "paddq %mm4, %mm2\n"
        "psubb %mm4, %mm7\n"
        "pand %mm1, %mm6\n"
        "pxor %mm0, %mm7\n"
        "pcmpeqb %mm2, %mm6\n"
        "pmullw %mm5, %mm2\n"
        "punpcklbw %mm7, %mm2\n"
        "pmaddwd %mm7, %mm4\n"
        "psllw $9, %mm7\n"
        "psrlq $3, %mm6\n"
        "psraw $5, %mm7\n"
        "movq %mm7, %mm4\n"
        "movd %r13d, %mm5\n"
        "pshufw $32, %mm0, %mm4\n"
        "vaddps %ymm10, %ymm5, %ymm0\n"
        "vsubps %ymm14, %ymm9, %ymm11\n"
        "vmulps %ymm14, %ymm11, %ymm10\n"
        "vdivps %xmm11, %xmm15, %xmm9\n"
        "vminps %xmm9, %xmm8, %xmm0\n"
        "vmaxps %xmm14, %xmm5, %xmm7\n"
        "vaddpd %xmm15, %xmm7, %xmm5\n"
        "vmulpd %xmm10, %xmm5, %xmm15\n"
        "vsubpd %xmm13, %xmm14, %xmm11\n"
        "vandps %ymm2, %ymm6, %ymm7\n"
        "vorpd %ymm3, %ymm11, %ymm1\n"
        "vxorps %ymm11, %ymm4, %ymm13\n"
        "vunpcklps %ymm11, %ymm14, %ymm4\n"
        "vunpckhpd %xmm6, %xmm4, %xmm2\n"
        "vaddss %xmm12, %xmm3, %xmm10\n"
        "vmulsd %xmm4, %xmm10, %xmm12\n"
        "vsubss %xmm9, %xmm15, %xmm12\n"
        "vdivsd %xmm8, %xmm9, %xmm15\n"
        "vminss %xmm9, %xmm15, %xmm9\n"
        "vmaxsd %xmm14, %xmm7, %xmm9\n"
        "vsqrtss %xmm0, %xmm15, %xmm8\n"
        "vsqrtps %xmm7, %xmm15\n"
        "vsqrtpd 44(%rcx,%r8,8), %ymm3\n"
        "vcmpps $8, 88(%rsi,%r12,4), %xmm0, %xmm14\n"
        "vcmppd $23, %ymm4, %ymm1, %ymm0\n"
        "vcmpss $4, %xmm6, %xmm0, %xmm13\n"
        "vshufps $246, %ymm9, %ymm14, %ymm2\n"
        "vshufpd $1, %ymm15, %ymm9, %ymm4\n"
        "vblendps $244, %xmm15, %xmm10, %xmm4\n"
        "vblendpd $24, %ymm2, %ymm2, %ymm0\n"
        "vpblendw $219, %ymm1, %ymm3, %ymm3\n"
        "vdpps $56, %xmm14, %xmm4, %xmm5\n"
        "vpaddb %xmm6, %xmm1, %xmm3\n"
        "vpaddd %ymm8, %ymm1, %ymm3\n"
        "vpaddq %ymm13, %ymm2, %ymm10\n"
        "vpsubw %ymm9, %ymm4, %ymm9\n"
        "vpand %xmm7, %xmm8, %xmm9\n"
        "vpor %xmm5, %xmm4, %xmm0\n"
        "vpxor %ymm1, %ymm2, %ymm3\n"
        "vpcmpeqb %ymm10, %ymm3, %ymm0\n"
        "vpcmpgtd %ymm7, %ymm3, %ymm2\n"
        "vpmaxsd %ymm12, %ymm11, %ymm7\n"
        "vpminud %ymm11, %ymm12, %ymm12\n"
        "vpmulld %xmm11, %xmm1, %xmm6\n"
        "vpunpcklbw %xmm3, %xmm1, %xmm1\n"
        "vpmaddwd %xmm15, %xmm15, %xmm11\n"
        "vpshufb %xmm10, %xmm14, %xmm14\n"
        "vpshufd $12, %xmm11, %xmm12\n"
        "vpslld $9, %ymm9, %ymm7\n"
        "vpsrlq $30, %xmm0, %xmm14\n"
        "vmovaps %ymm10, %ymm9\n"
        "vmovaps %ymm14, 305419896(%r14)\n"
        "vmovups %xmm0, %xmm7\n"
        "vmovups %xmm2, (%r12,%r15,4)\n"
        "vmovapd %ymm2, %ymm14\n"
        "vmovapd %ymm5, 305419896(%rdx)\n"
        "vmovdqa %xmm13, %xmm8\n"
        "vmovdqa %xmm0, (%r8)\n"
        "vmovdqu %ymm1, %ymm11\n"
        "vmovdqu %ymm3, (%r15)\n"
        "vmovd %edx, %xmm15\n"
        "vmovq %rbx, %xmm15\n"
        "vmovss 305419896(%rax), %xmm1\n"
        "vmovsd %xmm15, %xmm12, %xmm1\n"
        "vfmadd132ps %xmm1, %xmm1, %xmm6\n"
        "vfmadd213pd %xmm12, %xmm6, %xmm7\n"
        "vfmsub231ps %xmm2, %xmm5, %xmm2\n"
        "vfnmadd213ps %xmm14, %xmm11, %xmm15\n"
        "vfmadd213ss %xmm12, %xmm4, %xmm0\n"
        "vfnmsub231sd %xmm4, %xmm9, %xmm15\n"
        "vcvtph2ps %xmm4, %ymm2\n"
        "vcvtps2ph $0, %ymm15, %xmm13\n"
        "vcvtdq2ps %ymm1, %ymm15\n"
        "vcvttps2dq %xmm6, %xmm9\n"
        "vcvtsi2sd %rdi, %xmm3, %xmm2\n"
        "vbroadcastss %xmm9, %ymm4\n"
        "vpbroadcastd %xmm0, %ymm14\n"
        "vpermq $73, %ymm1, %ymm13\n"
        "vperm2f128 $137, %ymm3, %ymm12, %ymm4\n"
        "vinsertf128 $1, %xmm10, %ymm2, %ymm9\n"
        "vextracti128 $1, %ymm5, %xmm9\n"
        "vpinsrb $3, %eax, %xmm2, %xmm0\n"
        "vpextrd $2, %xmm0, %ebx\n"
        "vaddps %zmm27, %zmm16, %zmm1\n"
        "vaddps %zmm27, %zmm8, %zmm23{%k2}\n"
        "vaddps %zmm21, %zmm22, %zmm0{%k2}{z}\n"
        "vmulpd %zmm10, %zmm6, %zmm17\n"
        "vmulpd %zmm10, %zmm9, %zmm28{%k7}\n"
        "vmulpd %zmm27, %zmm27, %zmm7{%k3}{z}\n"
        "vpaddd %zmm27, %zmm10, %zmm28\n"
        "vpaddd %zmm30, %zmm31, %zmm0{%k1}\n"
        "vpaddd %zmm8, %zmm26, %zmm3{%k2}{z}\n"
        "vpaddq %zmm15, %zmm30, %zmm22\n"
        "vpaddq %zmm17, %zmm12, %zmm19{%k5}\n"
        "vpaddq %zmm22, %zmm15, %zmm23{%k3}{z}\n"
        "vpxord %zmm31, %zmm25, %zmm22\n"
        "vpxord %zmm31, %zmm24, %zmm2{%k1}\n"
        "vpxord %zmm10, %zmm25, %zmm6{%k7}{z}\n"
        "vpandq %zmm30, %zmm5, %zmm12\n"
        "vpandq %zmm6, %zmm11, %zmm24{%k1}\n"
        "vpandq %zmm20, %zmm1, %zmm19{%k5}{z}\n"
        "vpmaxsd %zmm30, %zmm25, %zmm28\n"
        "vpmaxsd %zmm3, %zmm9, %zmm26{%k3}\n"
        "vpmaxsd %zmm1, %zmm13, %zmm18{%k3}{z}\n"
        "vpcmpd $1, %zmm1, %zmm5, %k7\n"
        "vpcmpud $5, %zmm26, %zmm19, %k1\n"
        "vpcmpq $4, %zmm15, %zmm4, %k1\n"
        "vmovdqu32 %zmm5, %zmm19{%k3}{z}\n"
        "fadd %st, %st(1)\n"
        "fadd %st(7), %st\n"
        "faddl 305419896(%rax)\n"
        "fsub %st, %st(7)\n"
        "fsub %st(5), %st\n"
        "fsubs (%rdi)\n"
        "fsubl (%rsi,%rdx,8)\n"
        "fmul %st, %st(1)\n"
        "fmuls (%r12)\n"
        "fmull -166(%r15,%rbp,4)\n"
        "fdiv %st, %st(1)\n"
        "fdiv %st(5), %st\n"
        "fdivs 199(%rbp,%r15,1)\n"
        "fdivl -157(%rdx,%rax,2)\n"
        "fsubr %st, %st(5)\n"
        "fsubrs (%rdx)\n"
        "fsubrl (%r12)\n"
        "fdivr %st, %st(0)\n"
        "fdivrs (%rdx)\n"
        "fdivrl 109(%rbp,%r12,1)\n"
        "fstp %st(1)\n"
        "ffree %st(4)\n"
        "flds 127(%r15)\n"
        "fldl -8(%r8)\n"
        "fsts 305419896(%r12)\n"
        "fstl (%r9)\n"
        "fstps -128(%rdx)\n"
        "fstpl 57(%r14,%rdi,8)\n"
        "filds (%r9,%rsi,2)\n"
        "fildl (%r14,%rax,2)\n"
        "fistp (%rbp)\n"
        "fildll (%r15,%rbx,8)\n"
        "paddb %mm7, %mm7\n"
        "paddw (%rbp), %mm6\n"
        "paddd %mm4, %mm7\n"
        "paddq %mm0, %mm1\n"
        "psubb %mm5, %mm3\n"
        "pand %mm7, %mm5\n"
        "por %mm4, %mm7\n"
        "pxor %mm6, %mm7\n"
        "pmullw %mm4, %mm5\n"
        "punpcklbw %mm4, %mm0\n"
        "pmaddwd %mm0, %mm1\n"
        "psllw $11, %mm5\n"
        "pslld $15, %mm0\n"
        "psrlq $6, %mm2\n"
        "psraw $8, %mm0\n"
        "movq %mm7, %mm1\n"
        "movd %r10d, %mm3\n"
        "pshufw $51, %mm5, %mm2\n"
        "vaddps %xmm3, %xmm1, %xmm15\n"
        "vsubps %ymm0, %ymm4, %ymm2\n"
        "vmulps %xmm4, %xmm6, %xmm14\n"
        "vdivps %ymm15, %ymm2, %ymm9\n"
        "vminps %ymm8, %ymm8, %ymm15\n"
        "vmaxps %ymm7, %ymm7, %ymm1\n"
        "vaddpd %xmm13, %xmm10, %xmm2\n"
        "vmulpd %xmm12, %xmm4, %xmm14\n"
        "vsubpd %ymm4, %ymm2, %ymm11\n"
        "vandps %xmm4, %xmm5, %xmm9\n"
        "vorpd %xmm12, %xmm7, %xmm7\n"
        "vxorps %xmm7, %xmm0, %xmm12\n"
        "vunpcklps %xmm0, %xmm12, %xmm4\n"
        "vunpckhpd %xmm11, %xmm8, %xmm5\n"
        "vaddss %xmm4, %xmm12, %xmm2\n"
        "vmulsd %xmm1, %xmm15, %xmm1\n"
        "vsubss %xmm3, %xmm11, %xmm1\n"
        "vdivsd %xmm7, %xmm13, %xmm7\n"
        "vminss %xmm1, %xmm5, %xmm14\n"
        "vmaxsd %xmm5, %xmm1, %xmm0\n"
        "vsqrtss %xmm8, %xmm14, %xmm7\n"
        "vsqrtps %xmm6, %xmm8\n"
        "vsqrtpd %xmm10, %xmm2\n"
        "vcmpps $23, %xmm7, %xmm0, %xmm11\n"
        "vcmppd $27, %ymm11, %ymm4, %ymm0\n"
        "vcmpss $0, %xmm15, %xmm0, %xmm14\n"
        "vshufps $44, %ymm8, %ymm7, %ymm4\n"
        "vshufpd $214, %xmm12, %xmm13, %xmm15\n"
        "vblendps $129, %ymm3, %ymm5, %ymm3\n"
        "vblendpd $82, %xmm12, %xmm15, %xmm15\n"
        "vpblendw $110, %ymm0, %ymm10, %ymm13\n"
        "vdpps $163, %ymm7, %ymm0, %ymm9\n"
        "vpaddb %xmm2, %xmm6, %xmm12\n"
        "vpaddd %ymm1, %ymm15, %ymm11\n"
        "vpaddq %ymm12, %ymm8, %ymm2\n"
        "vpsubw %ymm3, %ymm4, %ymm3\n"
        "vpand %xmm6, %xmm11, %xmm10\n"
        "vpor %xmm5, %xmm7, %xmm4\n"
        "vpxor %ymm4, %ymm14, %ymm6\n"
        "vpcmpeqb %ymm8, %ymm2, %ymm8\n"
        "vpcmpgtd %xmm15, %xmm14, %xmm10\n"
        "vpmaxsd %xmm3, %xmm11, %xmm4\n"
        "vpminud %xmm0, %xmm4, %xmm3\n"
        "vpmulld %ymm4, %ymm15, %ymm8\n"
        "vpunpcklbw %xmm12, %xmm6, %xmm5\n"
        "vpmaddwd %ymm14, %ymm1, %ymm7\n"
        "vpshufb %xmm5, %xmm1, %xmm10\n"
        "vpshufd $13, %xmm1, %xmm8\n"
        "vpslld $23, %ymm0, %ymm2\n"
        "vpsrlq $11, %ymm7, %ymm15\n"
        "vmovaps %ymm11, %ymm3\n"
        "vmovaps %ymm2, (%rbp)\n"
        "vmovups %xmm1, %xmm15\n"
        "vmovups %xmm8, 113(%r9,%rsi,8)\n"
        "vmovapd %ymm13, %ymm5\n"
        "vmovapd %ymm8, (%rax)\n"
        "vmovdqa %ymm1, %ymm2\n"
        "vmovdqa %ymm3, (%rax,%rbp,1)\n"
        "vmovdqu %xmm8, %xmm9\n"
        "vmovdqu %xmm11, (%rbp,%r12,1)\n"
        "vmovd %ebx, %xmm9\n"
        "vmovq %r8, %xmm14\n"
        "vmovss 61(%r15,%rdx,1), %xmm1\n"
        "vmovsd %xmm12, %xmm4, %xmm7\n"
        "vfmadd132ps %ymm3, %ymm8, %ymm4\n"
        "vfmadd213pd %xmm6, %xmm2, %xmm8\n"
        "vfmsub231ps %xmm0, %xmm6, %xmm10\n"
        "vfnmadd213ps %ymm14, %ymm10, %ymm14\n"
        "vfmadd213ss %xmm3, %xmm7, %xmm4\n"
        "vfnmsub231sd %xmm11, %xmm14, %xmm8\n"
        "vcvtps2ph $0, %ymm0, %xmm3\n"
        "vcvtdq2ps %ymm10, %ymm11\n"
        "vcvttps2dq %xmm11, %xmm11\n"
        "vcvtsi2sd %r12, %xmm1, %xmm13\n"
        "vbroadcastss %xmm5, %ymm9\n"
        "vpbroadcastd %xmm9, %ymm15\n"
        "vpermq $123, %ymm11, %ymm13\n"
        "vperm2f128 $239, %ymm10, %ymm2, %ymm14\n"
        "vinsertf128 $1, %xmm13, %ymm15, %ymm0\n"
        "vextracti128 $1, %ymm3, %xmm15\n"
        "vpinsrb $3, %r8d, %xmm1, %xmm0\n"
        "vpextrd $2, %xmm11, %edi\n"
        "vaddps %zmm4, %zmm25, %zmm7\n"
        "vaddps %zmm16, %zmm24, %zmm23{%k6}\n"
        "vaddps %zmm0, %zmm22, %zmm26{%k3}{z}\n"
        "vmulpd %zmm1, %zmm13, %zmm0\n"
        "vmulpd %zmm30, %zmm6, %zmm26{%k4}\n"
        "vmulpd %zmm20, %zmm30, %zmm12{%k2}{z}\n"
        "vpaddd %zmm13, %zmm3, %zmm14\n"
        "vpaddd %zmm30, %zmm23, %zmm9{%k3}\n"
        "vpaddd %zmm17, %zmm12, %zmm3{%k5}{z}\n"
        "vpaddq %zmm2, %zmm27, %zmm1\n"
        "vpaddq %zmm5, %zmm7, %zmm12{%k5}\n"
        "vpaddq %zmm6, %zmm30, %zmm21{%k6}{z}\n"
        "vpxord %zmm14, %zmm20, %zmm13\n"
        "vpxord %zmm18, %zmm9, %zmm0{%k1}\n"
        "vpxord %zmm18, %zmm6, %zmm25{%k5}{z}\n"
        "vpandq %zmm26, %zmm31, %zmm5\n"
        "vpandq %zmm10, %zmm8, %zmm12{%k6}\n"
        "vpandq %zmm18, %zmm26, %zmm19{%k6}{z}\n"
        "vpmaxsd %zmm28, %zmm13, %zmm12\n"
        "vpmaxsd %zmm27, %zmm23, %zmm3{%k6}\n"
        "vpmaxsd %zmm20, %zmm23, %zmm22{%k4}{z}\n"
        "vpcmpd $6, %zmm22, %zmm26, %k7\n"
        "vpcmpud $4, %zmm9, %zmm16, %k6\n"
        "vpcmpq $3, %zmm29, %zmm11, %k6\n"
        "vmovdqu32 %zmm0, %zmm9{%k1}{z}\n"
        "fadd %st, %st(5)\n"
        "fadds (%rbp)\n"
        "fsubs (%r12)\n"
        "fsubl -128(%r9)\n"
        "fmuls (%rbp)\n"
        "fmull (%rdx)\n"
        "fdivs (%r12)\n"
        "fsubr %st(6), %st\n"
        "fsubrs -128(%rax)\n"
        "fsubrl 8(%r9)\n"
        "fdivr %st, %st(6)\n"
        "fdivr %st(2), %st\n"
        "fdivrs (%r9)\n"
        "fdivrl -71(%r12,%rbx,2)\n"
        "fld %st(5)\n"
        "fst %st(1)\n"
        "ffree %st(3)\n"
        "flds (%r8)\n"
        "fldl 220(%rbp,%r12,8)\n"
        "fsts -2(%rdi,%rcx,2)\n"
        "fstl (%rbx)\n"
        "fstps (%rsi)\n"
        "fstpl 203(%r8,%rsi,1)\n"
        "filds (%rbx)\n"
        "fildl 8(%rbp)\n"
        "fistp (%rcx,%rbp,1)\n"
        "fildll (%r14)\n"
        "paddb %mm5, %mm5\n"
        "paddw %mm1, %mm1\n"
        "paddd %mm0, %mm4\n"
        "paddq %mm7, %mm4\n"
        "psubb %mm0, %mm7\n"
        "pand %mm1, %mm7\n"
        "por %mm2, %mm3\n"
        "pxor %mm2, %mm7\n"
        "pcmpeqb %mm4, %mm2\n"
        "punpcklbw %mm1, %mm2\n"
        "pmaddwd %mm3, %mm0\n"
        "psllw $15, %mm2\n"
        "pslld $8, %mm4\n"
        "psrlq $0, %mm2\n"
        "psraw $13, %mm5\n"
        "movq %mm5, %mm2\n"
        "movd %r13d, %mm0\n"
        "pshufw $233, %mm6, %mm6\n"
        "vaddps %ymm15, %ymm12, %ymm1\n"
        "vsubps %xmm5, %xmm3, %xmm10\n"
        "vmulps %xmm4, %xmm3, %xmm11\n"
        "vdivps %xmm12, %xmm4, %xmm8\n"
        "vminps %ymm15, %ymm14, %ymm15\n"
        "vmaxps %ymm4, %ymm3, %ymm10\n"
        "vaddpd %ymm0, %ymm6, %ymm11\n"
        "vmulpd %xmm0, %xmm9, %xmm2\n"
        "vsubpd -85(%r9,%rdi,2), %ymm10, %ymm7\n"
        "vandps %xmm6, %xmm15, %xmm11\n"
        "vorpd %xmm11, %xmm14, %xmm11\n"
        "vxorps %ymm7, %ymm15, %ymm6\n"
        "vunpcklps %ymm15, %ymm11, %ymm2\n"
        "vunpckhpd %xmm5, %xmm13, %xmm10\n"
        "vaddss %xmm2, %xmm5, %xmm5\n"
        "vmulsd %xmm4, %xmm14, %xmm4\n"
        "vsubss %xmm15, %xmm11, %xmm4\n"
        "vdivsd %xmm4, %xmm0, %xmm0\n"
        "vminss (%r15), %xmm4, %xmm13\n"
        "vmaxsd %xmm8, %xmm6, %xmm9\n"
        "vsqrtss %xmm10, %xmm8, %xmm13\n"
        "vsqrtps 213(%rax,%r14,8), %xmm4\n"
        "vsqrtpd %xmm0, %xmm14\n"
        "vcmpps $0, %xmm15, %xmm3, %xmm1\n"
        "vcmppd $30, %ymm1, %ymm7, %ymm6\n"
        "vcmpss $4, (%rax), %xmm14, %xmm0\n"
        "vshufps $226, (%rdi), %xmm6, %xmm8\n"
        "vshufpd $244, 305419896(%r9), %ymm8, %ymm6\n"
        "vblendps $70, %ymm12, %ymm14, %ymm10\n"
        "vblendpd $123, %xmm6, %xmm9, %xmm3\n"
        "vpblendw $187, %xmm3, %xmm12, %xmm15\n"
        "vdpps $114, %xmm10, %xmm13, %xmm6\n"
        "vpaddb %ymm11, %ymm0, %ymm10\n"
        "vpaddd %ymm12, %ymm10, %ymm9\n"
        "vpaddq %xmm2, %xmm8, %xmm8\n"
        "vpsubw %xmm12, %xmm4, %xmm15\n"
        "vpand %ymm13, %ymm2, %ymm8\n"
        "vpor %xmm8, %xmm2, %xmm7\n"
        "vpxor %xmm14, %xmm0, %xmm10\n"
        "vpcmpeqb %ymm4, %ymm1, %ymm7\n"
        "vpcmpgtd %xmm9, %xmm9, %xmm6\n"
        "vpmaxsd %ymm5, %ymm8, %ymm11\n"
        "vpminud %xmm0, %xmm0, %xmm6\n"
        "vpmulld %ymm15, %ymm12, %ymm9\n"
        "vpunpcklbw %xmm12, %xmm11, %xmm1\n"
        "vpmaddwd %xmm8, %xmm13, %xmm5\n"
        "vpshufb %xmm7, %xmm9, %xmm1\n"
        "vpshufd $11, %ymm11, %ymm10\n"
        "vpslld $15, %ymm0, %ymm10\n"
        "vpsrlq $5, %ymm2, %ymm8\n"
        "vmovaps %xmm12, %xmm0\n"
        "vmovaps %xmm9, -128(%rsi)\n"
        "vmovups %xmm12, %xmm10\n"
        "vmovups %xmm15, -108(%rcx,%rax,8)\n"
        "vmovapd %xmm0, %xmm7\n"
        "vmovapd %xmm2, (%rax)\n"
        "vmovdqa %xmm1, %xmm0\n"
        "vmovdqa %xmm7, -253(%r8,%r8,1)\n"
        "vmovdqu %xmm2, %xmm15\n"
        "vmovdqu %xmm8, -16(%rbx,%r15,2)\n"
        "vmovd %edx, %xmm14\n"
        "vmovq %r8, %xmm12\n"
        "vmovss (%rbx,%r14,4), %xmm1\n"
        "vmovsd %xmm6, %xmm2, %xmm4\n"
        "vfmadd132ps %ymm1, %ymm15, %ymm8\n"
        "vfmadd213pd %xmm15, %xmm9, %xmm9\n"
        "vfmsub231ps %ymm9, %ymm2, %ymm15\n"
        "vfnmadd213ps %xmm8, %xmm12, %xmm6\n"
        "vfmadd213ss %xmm2, %xmm4, %xmm8\n"
        "vfnmsub231sd 256(%rdi), %xmm8, %xmm3\n"
        "vcvtph2ps %xmm12, %ymm0\n"
        "vcvtps2ph $0, %ymm5, %xmm12\n"
        "vcvtdq2ps %ymm11, %ymm12\n"
        "vcvttps2dq %xmm10, %xmm0\n"
        "vcvtsi2sd %rdi, %xmm10, %xmm12\n"
        "vbroadcastss %xmm0, %ymm9\n"
        "vpbroadcastd %xmm8, %ymm11\n"
        "vpermq $33, %ymm11, %ymm13\n"
        "vperm2f128 $140, %ymm1, %ymm8, %ymm3\n"
        "vinsertf128 $1, %xmm1, %ymm9, %ymm4\n"
        "vextracti128 $1, %ymm7, %xmm6\n"
        "vpinsrb $3, %edi, %xmm13, %xmm0\n"
        "vpextrd $2, %xmm12, %r13d\n"
        "vaddps %zmm5, %zmm3, %zmm26\n"
        "vaddps %zmm8, %zmm18, %zmm31{%k1}\n"
        "vaddps %zmm8, %zmm10, %zmm30{%k4}{z}\n"
        "vmulpd %zmm25, %zmm15, %zmm19\n"
        "vmulpd %zmm25, %zmm7, %zmm10{%k6}\n"
        "vmulpd %zmm10, %zmm4, %zmm13{%k5}{z}\n"
        "vpaddd %zmm14, %zmm28, %zmm21\n"
        "vpaddd %zmm15, %zmm5, %zmm11{%k3}\n"
        "vpaddd %zmm5, %zmm20, %zmm15{%k3}{z}\n"
        "vpaddq %zmm12, %zmm1, %zmm26\n"
        "vpaddq %zmm24, %zmm17, %zmm21{%k7}\n"
        "vpaddq %zmm3, %zmm31, %zmm17{%k5}{z}\n"
        "vpxord 305419896(%rdi), %zmm13, %zmm5\n"
        "vpxord %zmm25, %zmm28, %zmm27{%k3}\n"
        "vpxord %zmm1, %zmm8, %zmm2{%k4}{z}\n"
        "vpandq %zmm0, %zmm4, %zmm25\n"
        "vpandq %zmm6, %zmm14, %zmm9{%k2}\n"
        "vpandq %zmm6, %zmm29, %zmm5{%k5}{z}\n"
        "vpmaxsd %zmm8, %zmm14, %zmm2\n"
        "vpmaxsd %zmm27, %zmm7, %zmm6{%k1}\n"
        "vpmaxsd %zmm19, %zmm12, %zmm24{%k3}{z}\n"
        "vpcmpd $3, %zmm0, %zmm0, %k5\n"
        "vpcmpud $4, %zmm29, %zmm17, %k3\n"
        "vpcmpq $3, %zmm30, %zmm15, %k5\n"
        "vmovdqu32 %zmm15, %zmm1{%k4}{z}\n"
        "fadds -8(%rbp)\n"
        "faddl (%r15)\n"
        "fsub %st(2), %st\n"
        "fsubs (%rdx)\n"
        "fsubl (%rdx,%r14,1)\n"
        "fmul %st(7), %st\n"
        "fmuls (%r8,%r8,2)\n"
        "fmull (%rbp)\n"
        "fdiv %st, %st(7)\n"
        "fdivs 161(%rax,%r8,8)\n"
        "fsubr %st(1), %st\n"
        "fsubrs (%rbx,%r12,8)\n"
        "fsubrl (%rbx)\n"
        "fdivrs (%r15)\n"
        "fdivrl (%r8)\n"
        "fld %st(3)\n"
        "fst %st(2)\n"
        "flds -8(%r15)\n"
        "fldl (%r12,%r15,8)\n"
        "fsts (%rsi)\n"
        "fstl (%rdi,%rcx,2)\n"
        "fstps (%r12)\n"
        "fstpl 109(%r9,%r14,8)\n"
        "filds (%r9)\n"
        "fildl -103(%rdx,%rdi,4)\n"
        "fistp (%r9)\n"
        "fildll (%r9)\n"
        "paddb %mm5, %mm3\n"
        "paddw %mm6, %mm4\n"
        "paddd %mm1, %mm2\n"
        "paddq %mm3, %mm2\n"
        "psubb %mm0, %mm6\n"
        "pand %mm6, %mm3\n"
        "por %mm6, %mm6\n"
        "pxor %mm1, %mm4\n"
        "pcmpeqb %mm0, %mm1\n"
        "pmullw (%r14), %mm5\n"
        "psllw $4, %mm5\n"
        "pslld $12, %mm1\n"
        "psrlq $9, %mm5\n"
        "psraw $10, %mm5\n"
        "movq %mm0, %mm2\n"
        "movd %ecx, %mm5\n"
        "pshufw $220, %mm1, %mm3\n"
        "vaddps %xmm3, %xmm3, %xmm14\n"
        "vsubps %xmm15, %xmm12, %xmm11\n"
        "vmulps %ymm6, %ymm1, %ymm14\n"
        "vdivps %xmm0, %xmm0, %xmm11\n"
        "vminps (%r9), %ymm6, %ymm7\n"
        "vmaxps %ymm0, %ymm9, %ymm9\n"
        "vaddpd %xmm2, %xmm13, %xmm11\n"
        "vmulpd %ymm6, %ymm15, %ymm11\n"
        "vsubpd %ymm12, %ymm11, %ymm11\n"
        "vandps %ymm8, %ymm5, %ymm2\n"
        "vorpd %ymm9, %ymm8, %ymm8\n"
        "vxorps %ymm7, %ymm7, %ymm14\n"
        "vunpcklps (%rdx), %xmm6, %xmm11\n"
        "vunpckhpd %ymm12, %ymm2, %ymm5\n"
        "vaddss %xmm0, %xmm6, %xmm8\n"
        "vmulsd %xmm14, %xmm14, %xmm9\n"
        "vsubss %xmm4, %xmm11, %xmm2\n"
        "vdivsd %xmm2, %xmm4, %xmm11\n"
        "vminss %xmm1, %xmm10, %xmm7\n"
        "vmaxsd %xmm11, %xmm13, %xmm5\n"
        "vsqrtss %xmm2, %xmm3, %xmm7\n"
        "vsqrtps %xmm6, %xmm3\n"
        "vsqrtpd %xmm5, %xmm5\n"
        "vcmpps $18, %ymm15, %ymm3, %ymm2\n"
        "vcmppd $8, %xmm5, %xmm4, %xmm14\n"
        "vcmpss $0, %xmm13, %xmm13, %xmm11\n"
        "vshufps $187, %ymm5, %ymm9, %ymm8\n"
        "vshufpd $65, %ymm6, %ymm12, %ymm4\n"
        "vblendps $243, %ymm11, %ymm9, %ymm11\n"
        "vblendpd $14, %ymm0, %ymm2, %ymm5\n"
        "vpblendw $65, %ymm3, %ymm11, %ymm1\n"
        "vdpps $183, %ymm7, %ymm15, %ymm11\n"
        "vpaddb %ymm4, %ymm10, %ymm1\n"
        "vpaddd %xmm11, %xmm15, %xmm11\n"
        "vpaddq %ymm4, %ymm8, %ymm0\n"
        "vpsubw %ymm9, %ymm12, %ymm15\n"
        "vpand %ymm15, %ymm14, %ymm10\n"
        "vpor %xmm11, %xmm14, %xmm11\n"
        "vpxor %ymm4, %ymm1, %ymm2\n"
        "vpcmpeqb %ymm12, %ymm8, %ymm6\n"
        "vpcmpgtd %xmm14, %xmm6, %xmm10\n"
        "vpmaxsd %ymm0, %ymm4, %ymm9\n"
        "vpminud %ymm6, %ymm10, %ymm10\n"
        "vpmulld %xmm3, %xmm6, %xmm15\n"
        "vpunpcklbw %ymm2, %ymm4, %ymm11\n"
        "vpmaddwd %ymm8, %ymm13, %ymm6\n"
        "vpshufb %xmm15, %xmm10, %xmm5\n"
        "vpshufd $3, %xmm0, %xmm12\n"
        "vpslld $13, %xmm15, %xmm4\n"
        "vpsrlq $15, %ymm13, %ymm11\n"
        "vmovaps %ymm11, %ymm4\n"
        "vmovaps %ymm5, (%r8)\n"
        "vmovups %xmm7, %xmm13\n"
        "vmovups %xmm6, 256(%rdi)\n"
        "vmovapd %ymm7, %ymm12\n"
        "vmovapd %ymm0, (%rdi,%rdi,4)\n"
        "vmovdqa %xmm13, %xmm0\n"
        "vmovdqa %xmm15, (%rdi)\n"
        "vmovdqu (%rax), %xmm10\n"
        "vmovdqu %xmm11, (%r15,%r12,1)\n"
        "vmovd %r15d, %xmm5\n"
        "vmovq %rbx, %xmm1\n"
        "vmovss -173(%r8,%r9,1), %xmm6\n"
        "vmovsd %xmm7, %xmm2, %xmm1\n"
        "vfmadd132ps %ymm15, %ymm5, %ymm15\n"
        "vfmadd213pd %ymm15, %ymm14, %ymm7\n"
        "vfmsub231ps %xmm12, %xmm14, %xmm15\n"
        "vfnmadd213ps (%rbx), %xmm5, %xmm10\n"
        "vfmadd213ss %xmm14, %xmm5, %xmm9\n"
        "vfnmsub231sd %xmm12, %xmm8, %xmm2\n"
        "vcvtph2ps %xmm15, %ymm14\n"
        "vcvtps2ph $0, %ymm9, %xmm2\n"
        "vcvtdq2ps %ymm9, %ymm2\n"
        "vcvttps2dq %xmm5, %xmm5\n"
        "vcvtsi2sd %r15, %xmm15, %xmm10\n"
        "vbroadcastss %xmm6, %ymm14\n"
        "vpbroadcastd %xmm7, %ymm12\n"
        "vpermq $15, %ymm1, %ymm3\n"
        "vperm2f128 $227, %ymm11, %ymm8, %ymm10\n"
        "vinsertf128 $1, %xmm3, %ymm3, %ymm6\n"
        "vextracti128 $1, %ymm15, %xmm14\n"
        "vpinsrb $3, %ebx, %xmm4, %xmm0\n"
        "vpextrd $2, %xmm11, %ecx\n"
        "vaddps %zmm7, %zmm24, %zmm30\n"
        "vaddps %zmm11, %zmm15, %zmm28{%k2}\n"
        "vaddps %zmm17, %zmm9, %zmm13{%k5}{z}\n"
        "vmulpd 117(%rax,%rbx,2), %zmm20, %zmm26\n"
        "vmulpd %zmm0, %zmm6, %zmm9{%k2}\n"
        "vmulpd %zmm4, %zmm10, %zmm9{%k4}{z}\n"
        "vpaddd %zmm24, %zmm5, %zmm28\n"
        "vpaddd %zmm31, %zmm27, %zmm21{%k4}\n"
        "vpaddd %zmm14, %zmm19, %zmm2{%k5}{z}\n"
        "vpaddq %zmm1, %zmm23, %zmm27\n"
        "vpaddq %zmm4, %zmm31, %zmm3{%k5}\n"
        "vpaddq %zmm1, %zmm3, %zmm6{%k3}{z}\n"
        "vpxord %zmm22, %zmm22, %zmm5\n"
        "vpxord %zmm15, %zmm17, %zmm6{%k6}\n"
        "vpxord %zmm26, %zmm1, %zmm12{%k1}{z}\n"
        "vpandq (%rbx,%rdi,2), %zmm17, %zmm14\n"
        "vpandq %zmm10, %zmm20, %zmm19{%k2}\n"
        "vpandq %zmm1, %zmm17, %zmm1{%k5}{z}\n"
        "vpmaxsd %zmm18, %zmm9, %zmm19\n"
        "vpmaxsd %zmm13, %zmm1, %zmm8{%k6}\n"
        "vpmaxsd %zmm2, %zmm8, %zmm22{%k3}{z}\n"
        "vpcmpd $7, %zmm16, %zmm22, %k6\n"
        "vpcmpud $4, %zmm8, %zmm23, %k4\n"
        "vpcmpq $7, %zmm14, %zmm20, %k7\n"
        "vmovdqu32 %zmm5, %zmm16{%k5}{z}\n"
        ".globl axiom_asm_table_end\n"
        "axiom_asm_table_end:\n"
        ".text\n");

extern const unsigned char axiom_asm_table[], axiom_asm_table_end[];
static const char *const want[] = {
    "dcc2",  /* fadd %st, %st(2) */
    "d8c1",  /* fadd %st(1), %st */
    "d806",  /* fadds (%rsi) */
    "43dc04c0",  /* faddl (%r8,%r8,8) */
    "dce3",  /* fsub %st, %st(3) */
    "d8e1",  /* fsub %st(1), %st */
    "41d820",  /* fsubs (%r8) */
    "42dc642500",  /* fsubl (%rbp,%r12,1) */
    "dccf",  /* fmul %st, %st(7) */
    "d8cc",  /* fmul %st(4), %st */
    "41d88f00010000",  /* fmuls 256(%r15) */
    "dc8c031fffffff",  /* fmull -225(%rbx,%rax,1) */
    "d8f0",  /* fdiv %st, %st(0) */
    "d8f6",  /* fdiv %st(6), %st */
    "41d8767f",  /* fdivs 127(%r14) */
    "41dc37",  /* fdivl (%r15) */
    "dceb",  /* fsubr %st, %st(3) */
    "d8ef",  /* fsubr %st(7), %st */
    "41d828",  /* fsubrs (%r8) */
    "42dc6c72ec",  /* fsubrl -20(%rdx,%r14,2) */
    "dcff",  /* fdivr %st, %st(7) */
    "d8fc",  /* fdivr %st(4), %st */
    "42d83c08",  /* fdivrs (%rax,%r9,1) */
    "42dcbcb97bffffff",  /* fdivrl -133(%rcx,%r15,4) */
    "d9c6",  /* fld %st(6) */
    "ddd3",  /* fst %st(3) */
    "dddc",  /* fstp %st(4) */
    "d9cc",  /* fxch %st(4) */
    "ddc7",  /* ffree %st(7) */
    "43d90421",  /* flds (%r9,%r12,1) */
    "41dd8078563412",  /* fldl 305419896(%r8) */
    "42d9547500",  /* fsts (%rbp,%r14,2) */
    "dd17",  /* fstl (%rdi) */
    "43d99c0758ffffff",  /* fstps -168(%r15,%r8,1) */
    "dd19",  /* fstpl (%rcx) */
    "42df843df5000000",  /* filds 245(%rbp,%r15,1) */
    "41db00",  /* fildl (%r8) */
    "df1e",  /* fistp (%rsi) */
    "41df2c24",  /* fildll (%r12) */
    "dec1",  /* faddp */
    "dec9",  /* fmulp */
    "ded9",  /* fcompp */
    "dae9",  /* fucompp */
    "d9ff",  /* fcos */
    "d9fe",  /* fsin */
    "d9fa",  /* fsqrt */
    "d9eb",  /* fldpi */
    "d9e8",  /* fld1 */
    "d9ee",  /* fldz */
    "dfe0",  /* fnstsw %ax */
    "d9e5",  /* fxam */
    "d9f8",  /* fprem */
    "0ffc5df8",  /* paddb -8(%rbp), %mm3 */
    "0ffd9800010000",  /* paddw 256(%rax), %mm3 */
    "0ffeed",  /* paddd %mm5, %mm5 */
    "410fd49c6805ffffff",  /* paddq -251(%r8,%rbp,2), %mm3 */
    "0ff8ef",  /* psubb %mm7, %mm5 */
    "0fdbf3",  /* pand %mm3, %mm6 */
    "410febac38a8000000",  /* por 168(%r8,%rdi,1), %mm5 */
    "0fefd8",  /* pxor %mm0, %mm3 */
    "0f74ca",  /* pcmpeqb %mm2, %mm1 */
    "0fd5c1",  /* pmullw %mm1, %mm0 */
    "0f60dc",  /* punpcklbw %mm4, %mm3 */
    "0ff5ea",  /* pmaddwd %mm2, %mm5 */
    "0f71f109",  /* psllw $9, %mm1 */
    "0f72f205",  /* pslld $5, %mm2 */
    "0f73d208",  /* psrlq $8, %mm2 */
    "0f71e408",  /* psraw $8, %mm4 */
    "0f6fe0",  /* movq %mm0, %mm4 */
    "410f6ee8",  /* movd %r8d, %mm5 */
    "0f70c6d7",  /* pshufw $215, %mm6, %mm0 */
    "c50858ed",  /* vaddps %xmm5, %xmm14, %xmm13 */
    "c4c1405cc6",  /* vsubps %xmm14, %xmm7, %xmm0 */
    "c4c11459ca",  /* vmulps %ymm10, %ymm13, %ymm1 */
    "c4c16c5ed1",  /* vdivps %ymm9, %ymm2, %ymm2 */
    "c4c15c5dc0",  /* vminps %ymm8, %ymm4, %ymm0 */
    "c4c1505fce",  /* vmaxps %xmm14, %xmm5, %xmm1 */
    "c58558de",  /* vaddpd %ymm6, %ymm15, %ymm3 */
    "c4417d59d7",  /* vmulpd %ymm15, %ymm0, %ymm10 */
    "c54d5cd5",  /* vsubpd %ymm5, %ymm6, %ymm10 */
    "c51854db",  /* vandps %xmm3, %xmm12, %xmm11 */
    "c5ed56e1",  /* vorpd %ymm1, %ymm2, %ymm4 */
    "c53857d6",  /* vxorps %xmm6, %xmm8, %xmm10 */
    "c58414e7",  /* vunpcklps %ymm7, %ymm15, %ymm4 */
    "c4416915e5",  /* vunpckhpd %xmm13, %xmm2, %xmm12 */
    "c59a58d3",  /* vaddss %xmm3, %xmm12, %xmm2 */
    "c56b59c7",  /* vmulsd %xmm7, %xmm2, %xmm8 */
    "c5725ccb",  /* vsubss %xmm3, %xmm1, %xmm9 */
    "c56b5ee8",  /* vdivsd %xmm0, %xmm2, %xmm13 */
    "c5425dee",  /* vminss %xmm6, %xmm7, %xmm13 */
    "c4c1535ffe",  /* vmaxsd %xmm14, %xmm5, %xmm7 */
    "c4411a51cd",  /* vsqrtss %xmm13, %xmm12, %xmm9 */
    "c57c51d6",  /* vsqrtps %ymm6, %ymm10 */
    "c4417951d1",  /* vsqrtpd %xmm9, %xmm10 */
    "c4c10cc2da19",  /* vcmpps $25, %ymm10, %ymm14, %ymm3 */
    "c44125c2c70d",  /* vcmppd $13, %ymm15, %ymm11, %ymm8 */
    "c542c2de02",  /* vcmpss $2, %xmm6, %xmm7, %xmm11 */
    "c518c6cf8f",  /* vshufps $143, %xmm7, %xmm12, %xmm9 */
    "c5e9c6fba7",  /* vshufpd $167, %xmm3, %xmm2, %xmm7 */
    "c4e3690cc20a",  /* vblendps $10, %xmm2, %xmm2, %xmm0 */
    "c4e3290dd394",  /* vblendpd $148, %xmm3, %xmm10, %xmm2 */
    "c443610ec95b",  /* vpblendw $91, %xmm9, %xmm3, %xmm9 */
    "c4e32940f169",  /* vdpps $105, %xmm1, %xmm10, %xmm6 */
    "c541fcc1",  /* vpaddb %xmm1, %xmm7, %xmm8 */
    "c4c109fec6",  /* vpaddd %xmm14, %xmm14, %xmm0 */
    "c4417dd4ef",  /* vpaddq %ymm15, %ymm0, %ymm13 */
    "c539f9c4",  /* vpsubw %xmm4, %xmm8, %xmm8 */
    "c585dbc7",  /* vpand %ymm7, %ymm15, %ymm0 */
    "c541ebd7",  /* vpor %xmm7, %xmm7, %xmm10 */
    "c4c13deffa",  /* vpxor %ymm10, %ymm8, %ymm7 */
    "c4c15174f3",  /* vpcmpeqb %xmm11, %xmm5, %xmm6 */
    "c56566e2",  /* vpcmpgtd %ymm2, %ymm3, %ymm12 */
    "c442193ddf",  /* vpmaxsd %xmm15, %xmm12, %xmm11 */
    "c4626d3bc1",  /* vpminud %ymm1, %ymm2, %ymm8 */
    "c4626940f4",  /* vpmulld %xmm4, %xmm2, %xmm14 */
    "c4c10960e2",  /* vpunpcklbw %xmm10, %xmm14, %xmm4 */
    "c4c115f5dd",  /* vpmaddwd %ymm13, %ymm13, %ymm3 */
    "c4624d00f0",  /* vpshufb %ymm0, %ymm6, %ymm14 */
    "c57970c701",  /* vpshufd $1, %xmm7, %xmm8 */
    "c5b972f60b",  /* vpslld $11, %xmm6, %xmm8 */
    "c4c11573d710",  /* vpsrlq $16, %ymm15, %ymm13 */
    "c57829e6",  /* vmovaps %xmm12, %xmm6 */
    "c578290b",  /* vmovaps %xmm9, (%rbx) */
    "c57811cc",  /* vmovups %xmm9, %xmm4 */
    "c4c1781154a93e",  /* vmovups %xmm2, 62(%r9,%rbp,4) */
    "c4417d28f6",  /* vmovapd %ymm14, %ymm14 */
    "c57d291e",  /* vmovapd %ymm11, (%rsi) */
    "c57d7ffb",  /* vmovdqa %ymm15, %ymm3 */
    "c57d7fa500010000",  /* vmovdqa %ymm12, 256(%rbp) */
    "c5fa6f36",  /* vmovdqu (%rsi), %xmm6 */
    "c4417a7f3424",  /* vmovdqu %xmm14, (%r12) */
    "c441796ec8",  /* vmovd %r8d, %xmm9 */
    "c4c1f96eef",  /* vmovq %r15, %xmm5 */
    "c4c17a1030",  /* vmovss (%r8), %xmm6 */
    "c4417b10e3",  /* vmovsd %xmm11, %xmm0, %xmm12 */
    "c4427d98e9",  /* vfmadd132ps %ymm9, %ymm0, %ymm13 */
    "c462f9a8da",  /* vfmadd213pd %xmm2, %xmm0, %xmm11 */
    "c4620dbac4",  /* vfmsub231ps %ymm4, %ymm14, %ymm8 */
    "c44265ace8",  /* vfnmadd213ps %ymm8, %ymm3, %ymm13 */
    "c4e251a9e8",  /* vfmadd213ss %xmm0, %xmm5, %xmm5 */
    "c442b9bfcc",  /* vfnmsub231sd %xmm12, %xmm8, %xmm9 */
    "c4e27d13fe",  /* vcvtph2ps %xmm6, %ymm7 */
    "c4637d1dd200",  /* vcvtps2ph $0, %ymm10, %xmm2 */
    "c4417c5bf3",  /* vcvtdq2ps %ymm11, %ymm14 */
    "c5fa5be9",  /* vcvttps2dq %xmm1, %xmm5 */
    "c461bb2ade",  /* vcvtsi2sd %rsi, %xmm8, %xmm11 */
    "c4c27d18ec",  /* vbroadcastss %xmm12, %ymm5 */
    "c4427d58c7",  /* vpbroadcastd %xmm15, %ymm8 */
    "c4e3fd00c7a8",  /* vpermq $168, %ymm7, %ymm0 */
    "c4c31506face",  /* vperm2f128 $206, %ymm10, %ymm13, %ymm7 */
    "c4c34d18d001",  /* vinsertf128 $1, %xmm8, %ymm6, %ymm2 */
    "c4c37d39e801",  /* vextracti128 $1, %ymm5, %xmm8 */
    "c4c35120e203",  /* vpinsrb $3, %r10d, %xmm5, %xmm4 */
    "c4c37916e202",  /* vpextrd $2, %xmm4, %r10d */
    "62d1644058e5",  /* vaddps %zmm13, %zmm19, %zmm4 */
    "62b1044258f4",  /* vaddps %zmm20, %zmm31, %zmm6{%k2} */
    "62f164cf58ca",  /* vaddps %zmm2, %zmm3, %zmm1{%k7}{z} */
    "62819d4059ef",  /* vmulpd %zmm31, %zmm28, %zmm21 */
    "62618d4a59ce",  /* vmulpd %zmm6, %zmm14, %zmm25{%k2} */
    "62019dc759c7",  /* vmulpd %zmm31, %zmm28, %zmm24{%k7}{z} */
    "62211540fec2",  /* vpaddd %zmm18, %zmm29, %zmm24 */
    "62b10542fefd",  /* vpaddd %zmm21, %zmm31, %zmm7{%k2} */
    "62f16dcffec5",  /* vpaddd %zmm5, %zmm2, %zmm0{%k7}{z} */
    "62219d48d4ca",  /* vpaddq %zmm18, %zmm12, %zmm25 */
    "6271bd46d4c8",  /* vpaddq %zmm0, %zmm24, %zmm9{%k6} */
    "62e1bdc2d4c3",  /* vpaddq %zmm3, %zmm24, %zmm16{%k2}{z} */
    "62f16d48efd8",  /* vpxord %zmm0, %zmm2, %zmm3 */
    "62214549efd9",  /* vpxord %zmm17, %zmm7, %zmm27{%k1} */
    "624175ceeffc",  /* vpxord %zmm12, %zmm1, %zmm31{%k6}{z} */
    "6281d540dbc8",  /* vpandq %zmm24, %zmm21, %zmm17 */
    "62e1a54cdbf3",  /* vpandq %zmm3, %zmm11, %zmm22{%k4} */
    "6261cdc5dbd3",  /* vpandq %zmm3, %zmm22, %zmm26{%k5}{z} */
    "62825d483dcb",  /* vpmaxsd %zmm27, %zmm4, %zmm17 */
    "62727d413ddc",  /* vpmaxsd %zmm4, %zmm16, %zmm11{%k1} */
    "625265cf3de9",  /* vpmaxsd %zmm9, %zmm3, %zmm13{%k7}{z} */
    "62f16c1858d9",  /* vaddps {rn-sae}, %zmm1, %zmm2, %zmm3 */
    "62f1d55a59f4",  /* vmulpd {ru-sae}, %zmm4, %zmm5, %zmm6{%k2} */
    "62717c7851c7",  /* vsqrtps {rz-sae}, %zmm7, %zmm8 */
    "62f16c585818",  /* vaddps (%rax){1to16}, %zmm2, %zmm3 */
    "62f16d58fe1b",  /* vpaddd (%rbx){1to16}, %zmm2, %zmm3 */
    "62f1ed59585810",  /* vaddpd 128(%rax){1to8}, %zmm2, %zmm3{%k1} */
    "62f365481ff206",  /* vpcmpd $6, %zmm2, %zmm3, %k6 */
    "629345401ece01",  /* vpcmpud $1, %zmm30, %zmm23, %k1 */
    "62f3bd481fea05",  /* vpcmpq $5, %zmm2, %zmm8, %k5 */
    "62f36d4825d955",  /* vpternlogd $0x55, %zmm1, %zmm2, %zmm3 */
    "62f3ed492518ca",  /* vpternlogq $0xca, (%rax), %zmm2, %zmm3{%k1} */
    "62617ece6fe2",  /* vmovdqu32 %zmm2, %zmm28{%k6}{z} */
    "6261fd487f7001",  /* vmovdqa64 %zmm30, 64(%rax) */
    "62f27d4818eb",  /* vbroadcastss %xmm3, %zmm5 */
    "62f2fd4b59d1",  /* vpbroadcastq %xmm1, %zmm2{%k3} */
    "62f3654818e201",  /* vinsertf32x4 $1, %xmm2, %zmm3, %zmm4 */
    "62f3fd481bee01",  /* vextractf64x4 $1, %zmm5, %ymm6 */
    "c5f890d1",  /* kmovw %k1, %k2 */
    "c5f992d8",  /* kmovb %eax, %k3 */
    "c5ec41d9",  /* kandw %k1, %k2, %k3 */
    "c4e1f898ec",  /* kortestq %k4, %k5 */
    "c5f944d1",  /* knotb %k1, %k2 */
    "62a1544058f4",  /* vaddps %zmm20, %zmm21, %zmm22 */
    "62010d47feef",  /* vpaddd %zmm31, %zmm30, %zmm29{%k7} */
    "62a1642059e2",  /* vmulps %ymm18, %ymm19, %ymm20 */
    "62a1f70058d0",  /* vaddsd %xmm16, %xmm17, %xmm18 */
    "62f26d48a8d9",  /* vfmadd213ps %zmm1, %zmm2, %zmm3 */
    "62f2d518bcf4",  /* vfnmadd231pd {rn-sae}, %zmm4, %zmm5, %zmm6 */
    "fa",  /* cli */
    "fb",  /* sti */
    "f4",  /* hlt */
    "0f01f8",  /* swapgs */
    "0f30",  /* wrmsr */
    "0f32",  /* rdmsr */
    "48cf",  /* iretq */
    "480f07",  /* sysretq */
    "0f0138",  /* invlpg (%rax) */
    "0f0113",  /* lgdt (%rbx) */
    "0f014c2408",  /* sidt 8(%rsp) */
    "0f20c0",  /* mov %cr0, %rax */
    "0f22d8",  /* mov %rax, %cr3 */
    "0f21c1",  /* mov %db0, %rcx */
    "0f23fa",  /* mov %rdx, %dr7 */
    "0f00d8",  /* ltr %ax */
    "0f00d1",  /* lldt %cx */
    "0f09",  /* wbinvd */
    "0f06",  /* clts */
    "0fae27",  /* xsave (%rdi) */
    "480fae2e",  /* xrstor64 (%rsi) */
    "0fae00",  /* fxsave (%rax) */
    "f30f1efb",  /* endbr32 */
    "f30f1efa",  /* endbr64 */
    "f30fc7f8",  /* rdpid %rax */
    "d8c0",  /* fadd %st, %st(0) */
    "42d844bbad",  /* fadds -83(%rbx,%r15,4) */
    "dc06",  /* faddl (%rsi) */
    "d8e0",  /* fsub %st(0), %st */
    "41d864247f",  /* fsubs 127(%r12) */
    "43dc24be",  /* fsubl (%r14,%r15,4) */
    "d808",  /* fmuls (%rax) */
    "dc0cff",  /* fmull (%rdi,%rdi,8) */
    "dcf6",  /* fdiv %st, %st(6) */
    "d8f2",  /* fdiv %st(2), %st */
    "41d871f8",  /* fdivs -8(%r9) */
    "dc32",  /* fdivl (%rdx) */
    "dcea",  /* fsubr %st, %st(2) */
    "d8ed",  /* fsubr %st(5), %st */
    "d8a900010000",  /* fsubrs 256(%rcx) */
    "43dc6cc1ba",  /* fsubrl -70(%r9,%r8,8) */
    "dcfd",  /* fdivr %st, %st(5) */
    "d8fd",  /* fdivr %st(5), %st */
    "d83ccf",  /* fdivrs (%rdi,%rcx,8) */
    "43dc3c77",  /* fdivrl (%r15,%r14,2) */
    "d9c7",  /* fld %st(7) */
    "ddd4",  /* fst %st(4) */
    "dddf",  /* fstp %st(7) */
    "d9cd",  /* fxch %st(5) */
    "43d98440d3000000",  /* flds 211(%r8,%r8,2) */
    "dd8700010000",  /* fldl 256(%rdi) */
    "d914b6",  /* fsts (%rsi,%rsi,4) */
    "41dd17",  /* fstl (%r15) */
    "41d919",  /* fstps (%r9) */
    "41dd19",  /* fstpl (%r9) */
    "41df0474",  /* filds (%r12,%rsi,2) */
    "df1f",  /* fistp (%rdi) */
    "dfac1308ffffff",  /* fildll -248(%rbx,%rdx,1) */
    "0ffce0",  /* paddb %mm0, %mm4 */
    "0ffdd1",  /* paddw %mm1, %mm2 */
    "0ffef0",  /* paddd %mm0, %mm6 */
    "0fd4e8",  /* paddq %mm0, %mm5 */
    "0ff8c8",  /* psubb %mm0, %mm1 */
    "0fdbc0",  /* pand %mm0, %mm0 */
    "0febe5",  /* por %mm5, %mm4 */
    "0fefc2",  /* pxor %mm2, %mm0 */
    "0f74d8",  /* pcmpeqb %mm0, %mm3 */
    "0fd5e8",  /* pmullw %mm0, %mm5 */
    "0f60ec",  /* punpcklbw %mm4, %mm5 */
    "0ff5fc",  /* pmaddwd %mm4, %mm7 */
    "0f71f401",  /* psllw $1, %mm4 */
    "0f72f20c",  /* pslld $12, %mm2 */
    "0f73d30f",  /* psrlq $15, %mm3 */
    "0f71e502",  /* psraw $2, %mm5 */
    "0f6fd7",  /* movq %mm7, %mm2 */
    "410f6ef5",  /* movd %r13d, %mm6 */
    "0f70f4f9",  /* pshufw $249, %mm4, %mm6 */
    "c57058c4",  /* vaddps %xmm4, %xmm1, %xmm8 */
    "c5885cfb",  /* vsubps %xmm3, %xmm14, %xmm7 */
    "c4416859c6",  /* vmulps %xmm14, %xmm2, %xmm8 */
    "c441385eeb",  /* vdivps %xmm11, %xmm8, %xmm13 */
    "c5745de4",  /* vminps %ymm4, %ymm1, %ymm12 */
    "c5ec5f39",  /* vmaxps (%rcx), %ymm2, %ymm7 */
    "c5c158dd",  /* vaddpd %xmm5, %xmm7, %xmm3 */
    "c4410959ce",  /* vmulpd %xmm14, %xmm14, %xmm9 */
    "c4c1155cc5",  /* vsubpd %ymm13, %ymm13, %ymm0 */
    "c56054fd",  /* vandps %xmm5, %xmm3, %xmm15 */
    "c52556cb",  /* vorpd %ymm3, %ymm11, %ymm9 */
    "c4c16457dd",  /* vxorps %ymm13, %ymm3, %ymm3 */
    "c4417414ee",  /* vunpcklps %ymm14, %ymm1, %ymm13 */
    "c57d15ca",  /* vunpckhpd %ymm2, %ymm0, %ymm9 */
    "c58258f7",  /* vaddss %xmm7, %xmm15, %xmm6 */
    "c4411b59f3",  /* vmulsd %xmm11, %xmm12, %xmm14 */
    "c5e25cd3",  /* vsubss %xmm3, %xmm3, %xmm2 */
    "c5835ec8",  /* vdivsd %xmm0, %xmm15, %xmm1 */
    "c4415a5dde",  /* vminss %xmm14, %xmm4, %xmm11 */
    "c441035fcd",  /* vmaxsd %xmm13, %xmm15, %xmm9 */
    "c4413a51ef",  /* vsqrtss %xmm15, %xmm8, %xmm13 */
    "c5f851d3",  /* vsqrtps %xmm3, %xmm2 */
    "c57d51ec",  /* vsqrtpd %ymm4, %ymm13 */
    "c530c2e405",  /* vcmpps $5, %xmm4, %xmm9, %xmm12 */
    "c4c161c2e115",  /* vcmppd $21, %xmm9, %xmm3, %xmm4 */
    "c552c2f506",  /* vcmpss $6, %xmm5, %xmm5, %xmm14 */
    "c508c6f4ce",  /* vshufps $206, %xmm4, %xmm14, %xmm14 */
    "c44171c67f7fc4",  /* vshufpd $196, 127(%r15), %xmm1, %xmm15 */
    "c4c32d0cd3cf",  /* vblendps $207, %ymm11, %ymm10, %ymm2 */
    "c443090df260",  /* vblendpd $96, %xmm10, %xmm14, %xmm14 */
    "c463610ee630",  /* vpblendw $48, %xmm6, %xmm3, %xmm12 */
    "c4e30140e68c",  /* vdpps $140, %xmm6, %xmm15, %xmm4 */
    "c40151fc3484",  /* vpaddb (%r12,%r8,4), %xmm5, %xmm14 */
    "c44129fef7",  /* vpaddd %xmm15, %xmm10, %xmm14 */
    "c44155d4e3",  /* vpaddq %ymm11, %ymm5, %ymm12 */
    "c505f9e5",  /* vpsubw %ymm5, %ymm15, %ymm12 */
    "c4417ddbde",  /* vpand %ymm14, %ymm0, %ymm11 */
    "c531ebfe",  /* vpor %xmm6, %xmm9, %xmm15 */
    "c44169efc1",  /* vpxor %xmm9, %xmm2, %xmm8 */
    "c4a16d7474f63f",  /* vpcmpeqb 63(%rsi,%r14,8), %ymm2, %ymm6 */
    "c56d66cc",  /* vpcmpgtd %ymm4, %ymm2, %ymm9 */
    "c4a2393d0c4a",  /* vpmaxsd (%rdx,%r9,2), %xmm8, %xmm1 */
    "c4c2213bd2",  /* vpminud %xmm10, %xmm11, %xmm2 */
    "c4c20d40e1",  /* vpmulld %ymm9, %ymm14, %ymm4 */
    "c4c15560da",  /* vpunpcklbw %ymm10, %ymm5, %ymm3 */
    "c44151f5db",  /* vpmaddwd %xmm11, %xmm5, %xmm11 */
    "c4421900d4",  /* vpshufb %xmm12, %xmm12, %xmm10 */
    "c57d70da14",  /* vpshufd $20, %ymm2, %ymm11 */
    "c4c10d72f319",  /* vpslld $25, %ymm11, %ymm14 */
    "c5a573d305",  /* vpsrlq $5, %ymm3, %ymm11 */
    "c57829e8",  /* vmovaps %xmm13, %xmm0 */
    "c5782912",  /* vmovaps %xmm10, (%rdx) */
    "c4417c10d3",  /* vmovups %ymm11, %ymm10 */
    "c4c17c1130",  /* vmovups %ymm6, (%r8) */
    "c4417928c3",  /* vmovapd %xmm11, %xmm8 */
    "c5f92957f8",  /* vmovapd %xmm2, -8(%rdi) */
    "c5796fd1",  /* vmovdqa %xmm1, %xmm10 */
    "c441797f1e",  /* vmovdqa %xmm11, (%r14) */
    "c4417a6ff5",  /* vmovdqu %xmm13, %xmm14 */
    "c57a7f84f977ffffff",  /* vmovdqu %xmm8, -137(%rcx,%rdi,8) */
    "c5f96eca",  /* vmovd %edx, %xmm1 */
    "c461f96efd",  /* vmovq %rbp, %xmm15 */
    "c4417a1010",  /* vmovss (%r8), %xmm10 */
    "c50310e2",  /* vmovsd %xmm2, %xmm15, %xmm12 */
    "c4626598f1",  /* vfmadd132ps %ymm1, %ymm3, %ymm14 */
    "c462e9a8e5",  /* vfmadd213pd %xmm5, %xmm2, %xmm12 */
    "c44265bad8",  /* vfmsub231ps %ymm8, %ymm3, %ymm11 */
    "c4e211acd4",  /* vfnmadd213ps %xmm4, %xmm13, %xmm2 */
    "c46269a9ff",  /* vfmadd213ss %xmm7, %xmm2, %xmm15 */
    "c4e2d9bfc8",  /* vfnmsub231sd %xmm0, %xmm4, %xmm1 */
    "c4627d13d0",  /* vcvtph2ps %xmm0, %ymm10 */
    "c4e37d1dfb00",  /* vcvtps2ph $0, %ymm7, %xmm3 */
    "c4c17c5bf6",  /* vcvtdq2ps %ymm14, %ymm6 */
    "c57a5be6",  /* vcvttps2dq %xmm6, %xmm12 */
    "c4e19b2aef",  /* vcvtsi2sd %rdi, %xmm12, %xmm5 */
    "c4e27d18f4",  /* vbroadcastss %xmm4, %ymm6 */
    "c4627d58e5",  /* vpbroadcastd %xmm5, %ymm12 */
    "c463fd00e467",  /* vpermq $103, %ymm4, %ymm12 */
    "c4c36506d9a0",  /* vperm2f128 $160, %ymm9, %ymm3, %ymm3 */
    "c4433d18cf01",  /* vinsertf128 $1, %xmm15, %ymm8, %ymm9 */
    "c4637d39fc01",  /* vextracti128 $1, %ymm15, %xmm4 */
    "c4c36120c503",  /* vpinsrb $3, %r13d, %xmm3, %xmm0 */
    "c4e37916f202",  /* vpextrd $2, %xmm6, %edx */
    "62c1744858c0",  /* vaddps %zmm8, %zmm1, %zmm16 */
    "62c1344958e6",  /* vaddps %zmm14, %zmm9, %zmm20{%k1} */
    "62514cce58c4",  /* vaddps %zmm12, %zmm6, %zmm8{%k6}{z} */
    "6241d54859eb",  /* vmulpd %zmm11, %zmm5, %zmm29 */
    "6221dd4c59d1",  /* vmulpd %zmm17, %zmm4, %zmm26{%k4} */
    "62e195c659da",  /* vmulpd %zmm2, %zmm29, %zmm19{%k6}{z} */
    "62217d40fecd",  /* vpaddd %zmm21, %zmm16, %zmm25 */
    "62512546fec4",  /* vpaddd %zmm12, %zmm27, %zmm8{%k6} */
    "62411dc3feea",  /* vpaddd %zmm10, %zmm28, %zmm29{%k3}{z} */
    "62418d40d4e4",  /* vpaddq %zmm12, %zmm30, %zmm28 */
    "6231dd4bd4d3",  /* vpaddq %zmm19, %zmm4, %zmm10{%k3} */
    "62118dced4c6",  /* vpaddq %zmm30, %zmm14, %zmm8{%k6}{z} */
    "62b14d48efc3",  /* vpxord %zmm19, %zmm6, %zmm0 */
    "62e15d47efc3",  /* vpxord %zmm3, %zmm20, %zmm16{%k7} */
    "62b11dc4efe5",  /* vpxord %zmm21, %zmm28, %zmm4{%k4}{z} */
    "6231bd48dbea",  /* vpandq %zmm18, %zmm8, %zmm13 */
    "6201dd4bdbe0",  /* vpandq %zmm24, %zmm4, %zmm28{%k3} */
    "626185c7dbf5",  /* vpandq %zmm5, %zmm31, %zmm30{%k7}{z} */
    "62320d483de3",  /* vpmaxsd %zmm19, %zmm14, %zmm12 */
    "62d2154b3ddf",  /* vpmaxsd %zmm15, %zmm13, %zmm3{%k3} */
    "62423dcf3dcf",  /* vpmaxsd %zmm15, %zmm8, %zmm25{%k7}{z} */
    "62f315401fe706",  /* vpcmpd $6, %zmm7, %zmm29, %k4 */
    "62933d401ede06",  /* vpcmpud $6, %zmm30, %zmm24, %k3 */
    "62d38d481fcf03",  /* vpcmpq $3, %zmm15, %zmm14, %k1 */
    "62f17ec96fc5",  /* vmovdqu32 %zmm5, %zmm0{%k1}{z} */
    "dcc3",  /* fadd %st, %st(3) */
    "d8c2",  /* fadd %st(2), %st */
    "d807",  /* fadds (%rdi) */
    "41dc00",  /* faddl (%r8) */
    "dce1",  /* fsub %st, %st(1) */
    "41d864d0ef",  /* fsubs -17(%r8,%rdx,8) */
    "41dc21",  /* fsubl (%r9) */
    "d8ce",  /* fmul %st(6), %st */
    "41d84ef8",  /* fmuls -8(%r14) */
    "41dc8e00010000",  /* fmull 256(%r14) */
    "d8f0",  /* fdiv %st(0), %st */
    "41d836",  /* fdivs (%r14) */
    "dc31",  /* fdivl (%rcx) */
    "d8e8",  /* fsubr %st, %st(0) */
    "d8ec",  /* fsubr %st(4), %st */
    "42d8ace0e4000000",  /* fsubrs 228(%rax,%r12,8) */
    "41dc2cef",  /* fsubrl (%r15,%rbp,8) */
    "dcfa",  /* fdivr %st, %st(2) */
    "d83b",  /* fdivrs (%rbx) */
    "dc3c91",  /* fdivrl (%rcx,%rdx,4) */
    "ddde",  /* fstp %st(6) */
    "d9ce",  /* fxch %st(6) */
    "ddc5",  /* ffree %st(5) */
    "41d901",  /* flds (%r9) */
    "dd4500",  /* fldl (%rbp) */
    "d994721dffffff",  /* fsts -227(%rdx,%rsi,2) */
    "43dd947f6affffff",  /* fstl -150(%r15,%r15,2) */
    "41d91e",  /* fstps (%r14) */
    "dd9cde7fffffff",  /* fstpl -129(%rsi,%rbx,8) */
    "41df049e",  /* filds (%r14,%rbx,4) */
    "db040b",  /* fildl (%rbx,%rcx,1) */
    "df1c2e",  /* fistp (%rsi,%rbp,1) */
    "df28",  /* fildll (%rax) */
    "0ffcee",  /* paddb %mm6, %mm5 */
    "0ffdc8",  /* paddw %mm0, %mm1 */
    "0ffe03",  /* paddd (%rbx), %mm0 */
    "0fd4c2",  /* paddq %mm2, %mm0 */
    "0ff8fe",  /* psubb %mm6, %mm7 */
    "430fdb2431",  /* pand (%r9,%r14,1), %mm4 */
    "0febe6",  /* por %mm6, %mm4 */
    "0fefc5",  /* pxor %mm5, %mm0 */
    "0f74c2",  /* pcmpeqb %mm2, %mm0 */
    "0fd5fc",  /* pmullw %mm4, %mm7 */
    "0f60c0",  /* punpcklbw %mm0, %mm0 */
    "0f71f505",  /* psllw $5, %mm5 */
    "0f72f40b",  /* pslld $11, %mm4 */
    "0f73d609",  /* psrlq $9, %mm6 */
    "0f71e003",  /* psraw $3, %mm0 */
    "410f6f5c2408",  /* movq 8(%r12), %mm3 */
    "0f6ede",  /* movd %esi, %mm3 */
    "0f70e9a7",  /* pshufw $167, %mm1, %mm5 */
    "c56c58d5",  /* vaddps %ymm5, %ymm2, %ymm10 */
    "c5f05cf3",  /* vsubps %xmm3, %xmm1, %xmm6 */
    "c4416c59da",  /* vmulps %ymm10, %ymm2, %ymm11 */
    "c441305eed",  /* vdivps %xmm13, %xmm9, %xmm13 */
    "c5045de8",  /* vminps %ymm0, %ymm15, %ymm13 */
    "c4c1405fd2",  /* vmaxps %xmm10, %xmm7, %xmm2 */
    "c54d58e9",  /* vaddpd %ymm1, %ymm6, %ymm13 */
    "c5b159fd",  /* vmulpd %xmm5, %xmm9, %xmm7 */
    "c4c1715cdd",  /* vsubpd %xmm13, %xmm1, %xmm3 */
    "c5cc54df",  /* vandps %ymm7, %ymm6, %ymm3 */
    "c57956fc",  /* vorpd %xmm4, %xmm0, %xmm15 */
    "c50457d1",  /* vxorps %ymm1, %ymm15, %ymm10 */
    "c5f014d3",  /* vunpcklps %xmm3, %xmm1, %xmm2 */
    "c5051510",  /* vunpckhpd (%rax), %ymm15, %ymm10 */
    "c4413258dc",  /* vaddss %xmm12, %xmm9, %xmm11 */
    "c4c16359e5",  /* vmulsd %xmm13, %xmm3, %xmm4 */
    "c4c16a5cec",  /* vsubss %xmm12, %xmm2, %xmm5 */
    "c5935ec9",  /* vdivsd %xmm1, %xmm13, %xmm1 */
    "c441125df5",  /* vminss %xmm13, %xmm13, %xmm14 */
    "c4416b5fe8",  /* vmaxsd %xmm8, %xmm2, %xmm13 */
    "c4412251c2",  /* vsqrtss %xmm10, %xmm11, %xmm8 */
    "c441785120",  /* vsqrtps (%r8), %xmm12 */
    "c4c17951e7",  /* vsqrtpd %xmm15, %xmm4 */
    "c560c2e218",  /* vcmpps $24, %xmm2, %xmm3, %xmm12 */
    "c561c2f801",  /* vcmppd $1, %xmm0, %xmm3, %xmm15 */
    "c5e2c2cb04",  /* vcmpss $4, %xmm3, %xmm3, %xmm1 */
    "c50cc6e75c",  /* vshufps $92, %ymm7, %ymm14, %ymm12 */
    "c59dc6fabc",  /* vshufpd $188, %ymm2, %ymm12, %ymm7 */
    "c4e3050c650052",  /* vblendps $82, (%rbp), %ymm15, %ymm4 */
    "c443050df74c",  /* vblendpd $76, %ymm15, %ymm15, %ymm14 */
    "c4e3290e7ef845",  /* vpblendw $69, -8(%rsi), %xmm10, %xmm7 */
    "c4434d40c8d3",  /* vdpps $211, %ymm8, %ymm6, %ymm9 */
    "c5a1fcfd",  /* vpaddb %xmm5, %xmm11, %xmm7 */
    "c4414dfef7",  /* vpaddd %ymm15, %ymm6, %ymm14 */
    "c44171d4f4",  /* vpaddq %xmm12, %xmm1, %xmm14 */
    "c5b9f9fe",  /* vpsubw %xmm6, %xmm8, %xmm7 */
    "c5b9dbe9",  /* vpand %xmm1, %xmm8, %xmm5 */
    "c5e9ebda",  /* vpor %xmm2, %xmm2, %xmm3 */
    "c4c179efc2",  /* vpxor %xmm10, %xmm0, %xmm0 */
    "c54d74fa",  /* vpcmpeqb %ymm2, %ymm6, %ymm15 */
    "c4416566c2",  /* vpcmpgtd %ymm10, %ymm3, %ymm8 */
    "c4c2393d1cde",  /* vpmaxsd (%r14,%rbx,8), %xmm8, %xmm3 */
    "c4e22d3bdb",  /* vpminud %ymm3, %ymm10, %ymm3 */
    "c4e25d40ed",  /* vpmulld %ymm5, %ymm4, %ymm5 */
    "c52d60ec",  /* vpunpcklbw %ymm4, %ymm10, %ymm13 */
    "c4c135f5ef",  /* vpmaddwd %ymm15, %ymm9, %ymm5 */
    "c4c22100dc",  /* vpshufb %xmm12, %xmm11, %xmm3 */
    "c5fd70cc11",  /* vpshufd $17, %ymm4, %ymm1 */
    "c4c12d72f311",  /* vpslld $17, %ymm11, %ymm10 */
    "c4c10573d31c",  /* vpsrlq $28, %ymm11, %ymm15 */
    "c5f828eb",  /* vmovaps %xmm3, %xmm5 */
    "c481782934f4",  /* vmovaps %xmm6, (%r12,%r14,8) */
    "c57810e4",  /* vmovups %xmm4, %xmm12 */
    "c4c1781131",  /* vmovups %xmm6, (%r9) */
    "c4417928d8",  /* vmovapd %xmm8, %xmm11 */
    "c579290ce8",  /* vmovapd %xmm9, (%rax,%rbp,8) */
    "c4417d6ff9",  /* vmovdqa %ymm9, %ymm15 */
    "c4417d7f0c46",  /* vmovdqa %ymm9, (%r14,%rax,2) */
    "c57a6ff5",  /* vmovdqu %xmm5, %xmm14 */
    "c5fa7f32",  /* vmovdqu %xmm6, (%rdx) */
    "c441f96ef1",  /* vmovq %r9, %xmm14 */
    "c57a100b",  /* vmovss (%rbx), %xmm9 */
    "c55b10f4",  /* vmovsd %xmm4, %xmm4, %xmm14 */
    "c4e22198f8",  /* vfmadd132ps %xmm0, %xmm11, %xmm7 */
    "c462a9a8d0",  /* vfmadd213pd %xmm0, %xmm10, %xmm10 */
    "c4626dbad1",  /* vfmsub231ps %ymm1, %ymm2, %ymm10 */
    "c46201acd7",  /* vfnmadd213ps %xmm7, %xmm15, %xmm10 */
    "c4c269a9f5",  /* vfmadd213ss %xmm13, %xmm2, %xmm6 */
    "c4c281bfd7",  /* vfnmsub231sd %xmm15, %xmm15, %xmm2 */
    "c4c27d13c1",  /* vcvtph2ps %xmm9, %ymm0 */
    "c4437d1df600",  /* vcvtps2ph $0, %ymm14, %xmm14 */
    "c57c5b5cf877",  /* vcvtdq2ps 119(%rax,%rdi,8), %ymm11 */
    "c57a5bc6",  /* vcvttps2dq %xmm6, %xmm8 */
    "c461db2af7",  /* vcvtsi2sd %rdi, %xmm4, %xmm14 */
    "c4e27d18c6",  /* vbroadcastss %xmm6, %ymm0 */
    "c4e3fd00dc56",  /* vpermq $86, %ymm4, %ymm3 */
    "c4c30506ee56",  /* vperm2f128 $86, %ymm14, %ymm15, %ymm5 */
    "c4637d18e101",  /* vinsertf128 $1, %xmm1, %ymm0, %ymm12 */
    "c4637d39f701",  /* vextracti128 $1, %ymm14, %xmm7 */
    "c4437120e003",  /* vpinsrb $3, %r8d, %xmm1, %xmm12 */
    "c4637916f802",  /* vpextrd $2, %xmm15, %eax */
    "62110c4058e0",  /* vaddps %zmm24, %zmm30, %zmm12 */
    "62e16c4758cb",  /* vaddps %zmm3, %zmm18, %zmm17{%k7} */
    "620164c258fd",  /* vaddps %zmm29, %zmm19, %zmm31{%k2}{z} */
    "62b1d54859dc",  /* vmulpd %zmm20, %zmm5, %zmm3 */
    "62f1f54959ee",  /* vmulpd %zmm6, %zmm1, %zmm5{%k1} */
    "6241edc959f2",  /* vmulpd %zmm10, %zmm2, %zmm30{%k1}{z} */
    "62211d48fef5",  /* vpaddd %zmm21, %zmm12, %zmm30 */
    "62016541fec8",  /* vpaddd %zmm24, %zmm19, %zmm25{%k1} */
    "622125c9fed2",  /* vpaddd %zmm18, %zmm11, %zmm26{%k1}{z} */
    "6201a548d4c1",  /* vpaddq %zmm25, %zmm11, %zmm24 */
    "62018d4fd4e4",  /* vpaddq %zmm28, %zmm14, %zmm28{%k7} */
    "6281cdc7d4ce",  /* vpaddq %zmm30, %zmm22, %zmm17{%k7}{z} */
    "62910540efd0",  /* vpxord %zmm24, %zmm31, %zmm2 */
    "62f11547efe9",  /* vpxord %zmm1, %zmm29, %zmm5{%k7} */
    "62715dc5effe",  /* vpxord %zmm6, %zmm20, %zmm15{%k5}{z} */
    "62619d40dbeb",  /* vpandq %zmm3, %zmm28, %zmm29 */
    "62d1b543dbf4",  /* vpandq %zmm12, %zmm25, %zmm6{%k3} */
    "6221c5c9dbe3",  /* vpandq %zmm19, %zmm7, %zmm28{%k1}{z} */
    "627235483dcb",  /* vpmaxsd %zmm3, %zmm9, %zmm9 */
    "62e20d4a3dd7",  /* vpmaxsd %zmm7, %zmm14, %zmm18{%k2} */
    "62c22dc73de6",  /* vpmaxsd %zmm14, %zmm26, %zmm20{%k7}{z} */
    "629325481fed03",  /* vpcmpd $3, %zmm29, %zmm11, %k5 */
    "62f345481eea01",  /* vpcmpud $1, %zmm2, %zmm7, %k5 */
    "62f39d481fde00",  /* vpcmpq $0, %zmm6, %zmm12, %k3 */
    "62f17ecc6ff5",  /* vmovdqu32 %zmm5, %zmm6{%k4}{z} */
    "d8c4",  /* fadd %st(4), %st */
    "42d80443",  /* fadds (%rbx,%r8,2) */
    "dc03",  /* faddl (%rbx) */
    "d8e0",  /* fsub %st, %st(0) */
    "d8e6",  /* fsub %st(6), %st */
    "41d8a4913cffffff",  /* fsubs -196(%r9,%rdx,4) */
    "dc6680",  /* fsubl -128(%rsi) */
    "dccc",  /* fmul %st, %st(4) */
    "d8cb",  /* fmul %st(3), %st */
    "d84c5016",  /* fmuls 22(%rax,%rdx,2) */
    "dc4c9e7d",  /* fmull 125(%rsi,%rbx,4) */
    "d8f3",  /* fdiv %st(3), %st */
    "d8717f",  /* fdivs 127(%rcx) */
    "dc36",  /* fdivl (%rsi) */
    "dcec",  /* fsubr %st, %st(4) */
    "d8e8",  /* fsubr %st(0), %st */
    "d82e",  /* fsubrs (%rsi) */
    "41dc6cefc7",  /* fsubrl -57(%r15,%rbp,8) */
    "dcfc",  /* fdivr %st, %st(4) */
    "d8fe",  /* fdivr %st(6), %st */
    "41d878f8",  /* fdivrs -8(%r8) */
    "dcbc1e2cffffff",  /* fdivrl -212(%rsi,%rbx,1) */
    "ddc2",  /* ffree %st(2) */
    "41d94680",  /* flds -128(%r14) */
    "dd8578563412",  /* fldl 305419896(%rbp) */
    "41d91476",  /* fsts (%r14,%rsi,2) */
    "42dd14bf",  /* fstl (%rdi,%r15,4) */
    "dd9c4267ffffff",  /* fstpl -153(%rdx,%rax,2) */
    "df06",  /* filds (%rsi) */
    "41db442480",  /* fildl -128(%r12) */
    "df5f08",  /* fistp 8(%rdi) */
    "41df28",  /* fildll (%r8) */
    "0ffce8",  /* paddb %mm0, %mm5 */
    "0ffde1",  /* paddw %mm1, %mm4 */
    "0ffed4",  /* paddd %mm4, %mm2 */
    "0fd4c5",  /* paddq %mm5, %mm0 */
    "0ff8d7",  /* psubb %mm7, %mm2 */
    "0fdbdf",  /* pand %mm7, %mm3 */
    "0febc0",  /* por %mm0, %mm0 */
    "0f74c5",  /* pcmpeqb %mm5, %mm0 */
    "0fd5f4",  /* pmullw %mm4, %mm6 */
    "0f60db",  /* punpcklbw %mm3, %mm3 */
    "0ff5f3",  /* pmaddwd %mm3, %mm6 */
    "0f71f30e",  /* psllw $14, %mm3 */
    "0f72f30d",  /* pslld $13, %mm3 */
    "0f71e001",  /* psraw $1, %mm0 */
    "0f6ff3",  /* movq %mm3, %mm6 */
    "0f6ed6",  /* movd %esi, %mm2 */
    "0f70f8a6",  /* pshufw $166, %mm0, %mm7 */
    "c4413458fa",  /* vaddps %ymm10, %ymm9, %ymm15 */
    "c53c5cd6",  /* vsubps %ymm6, %ymm8, %ymm10 */
    "c4c14c59c8",  /* vmulps %ymm8, %ymm6, %ymm1 */
    "c5545ef1",  /* vdivps %ymm1, %ymm5, %ymm14 */
    "c5cc5de8",  /* vminps %ymm0, %ymm6, %ymm5 */
    "c5f05ff7",  /* vmaxps %xmm7, %xmm1, %xmm6 */
    "c4410158f2",  /* vaddpd %xmm10, %xmm15, %xmm14 */
    "c50159c3",  /* vmulpd %xmm3, %xmm15, %xmm8 */
    "c4c1215cd1",  /* vsubpd %xmm9, %xmm11, %xmm2 */
    "c4c11854f9",  /* vandps %xmm9, %xmm12, %xmm7 */
    "c56556da",  /* vorpd %ymm2, %ymm3, %ymm11 */
    "c4410457e9",  /* vxorps %ymm9, %ymm15, %ymm13 */
    "c5d014e5",  /* vunpcklps %xmm5, %xmm5, %xmm4 */
    "c57d15cd",  /* vunpckhpd %ymm5, %ymm0, %ymm9 */
    "c50258fb",  /* vaddss %xmm3, %xmm15, %xmm15 */
    "c441435929",  /* vmulsd (%r9), %xmm7, %xmm13 */
    "c52a5cf1",  /* vsubss %xmm1, %xmm10, %xmm14 */
    "c4415b5ec4",  /* vdivsd %xmm12, %xmm4, %xmm8 */
    "c5da5dea",  /* vminss %xmm2, %xmm4, %xmm5 */
    "c56b5fe2",  /* vmaxsd %xmm2, %xmm2, %xmm12 */
    "c4c12a51eb",  /* vsqrtss %xmm11, %xmm10, %xmm5 */
    "c5fc51d0",  /* vsqrtps %ymm0, %ymm2 */
    "c4c17d51d7",  /* vsqrtpd %ymm15, %ymm2 */
    "c4c118c2f90b",  /* vcmpps $11, %xmm9, %xmm12, %xmm7 */
    "c4210dc264beeb0e",  /* vcmppd $14, -21(%rsi,%r15,4), %ymm14, %ymm12 */
    "c44152c2f006",  /* vcmpss $6, %xmm8, %xmm5, %xmm14 */
    "c5dcc6d0ce",  /* vshufps $206, %ymm0, %ymm4, %ymm2 */
    "c585c6d7b1",  /* vshufpd $177, %ymm7, %ymm15, %ymm2 */
    "c4c3390cc47a",  /* vblendps $122, %xmm12, %xmm8, %xmm0 */
    "c4c3690dda50",  /* vblendpd $80, %xmm10, %xmm2, %xmm3 */
    "c4c35d0ed51f",  /* vpblendw $31, %ymm13, %ymm4, %ymm2 */
    "c4e31540e401",  /* vdpps $1, %ymm4, %ymm13, %ymm4 */
    "c4c151fcec",  /* vpaddb %xmm12, %xmm5, %xmm5 */
    "c4c12dfeec",  /* vpaddd %ymm12, %ymm10, %ymm5 */
    "c5b5d4e0",  /* vpaddq %ymm0, %ymm9, %ymm4 */
    "c5c1f9f8",  /* vpsubw %xmm0, %xmm7, %xmm7 */
    "c509dbe5",  /* vpand %xmm5, %xmm14, %xmm12 */
    "c565ebfe",  /* vpor %ymm6, %ymm3, %ymm15 */
    "c53defce",  /* vpxor %ymm6, %ymm8, %ymm9 */
    "c57574d3",  /* vpcmpeqb %ymm3, %ymm1, %ymm10 */
    "c57566fd",  /* vpcmpgtd %ymm5, %ymm1, %ymm15 */
    "c4e2293de2",  /* vpmaxsd %xmm2, %xmm10, %xmm4 */
    "c4e27d3b13",  /* vpminud (%rbx), %ymm0, %ymm2 */
    "c4e20140e0",  /* vpmulld %xmm0, %xmm15, %xmm4 */
    "c4413160e3",  /* vpunpcklbw %xmm11, %xmm9, %xmm12 */
    "c4416df5cf",  /* vpmaddwd %ymm15, %ymm2, %ymm9 */
    "c4627d00fe",  /* vpshufb %ymm6, %ymm0, %ymm15 */
    "c4c17d70f211",  /* vpshufd $17, %ymm10, %ymm6 */
    "c59d72f011",  /* vpslld $17, %ymm0, %ymm12 */
    "c4c16d73d212",  /* vpsrlq $18, %ymm10, %ymm2 */
    "c57828c6",  /* vmovaps %xmm6, %xmm8 */
    "c4c17829349e",  /* vmovaps %xmm6, (%r14,%rbx,4) */
    "c5f810ee",  /* vmovups %xmm6, %xmm5 */
    "c5f81112",  /* vmovups %xmm2, (%rdx) */
    "c5f928ea",  /* vmovapd %xmm2, %xmm5 */
    "c5f9294108",  /* vmovapd %xmm0, 8(%rcx) */
    "c5f96fed",  /* vmovdqa %xmm5, %xmm5 */
    "c5f97f30",  /* vmovdqa %xmm6, (%rax) */
    "c4417a6ff8",  /* vmovdqu %xmm8, %xmm15 */
    "c5fa7f5a7f",  /* vmovdqu %xmm3, 127(%rdx) */
    "c441796efa",  /* vmovd %r10d, %xmm15 */
    "c441f96ed1",  /* vmovq %r9, %xmm10 */
    "c4a17a100cff",  /* vmovss (%rdi,%r15,8), %xmm1 */
    "c5d310e1",  /* vmovsd %xmm1, %xmm5, %xmm4 */
    "c4421d98db",  /* vfmadd132ps %ymm11, %ymm12, %ymm11 */
    "c4e2c9a8c5",  /* vfmadd213pd %xmm5, %xmm6, %xmm0 */
    "c4c201baca",  /* vfmsub231ps %xmm10, %xmm15, %xmm1 */
    "c4420dacdc",  /* vfnmadd213ps %ymm12, %ymm14, %ymm11 */
    "c44259a9f6",  /* vfmadd213ss %xmm14, %xmm4, %xmm14 */
    "c4c2b1bfca",  /* vfnmsub231sd %xmm10, %xmm9, %xmm1 */
    "c4e27d13fd",  /* vcvtph2ps %xmm5, %ymm7 */
    "c4637d1dd000",  /* vcvtps2ph $0, %ymm10, %xmm0 */
    "c5fc5be1",  /* vcvtdq2ps %ymm1, %ymm4 */
    "c4c17a5bf9",  /* vcvttps2dq %xmm9, %xmm7 */
    "c461ab2add",  /* vcvtsi2sd %rbp, %xmm10, %xmm11 */
    "c4e27d18e1",  /* vbroadcastss %xmm1, %ymm4 */
    "c4c27d58f3",  /* vpbroadcastd %xmm11, %ymm6 */
    "c4e3fd00f2fd",  /* vpermq $253, %ymm2, %ymm6 */
    "c4e35d06cc51",  /* vperm2f128 $81, %ymm4, %ymm4, %ymm1 */
    "c4c36d18cb01",  /* vinsertf128 $1, %xmm11, %ymm2, %ymm1 */
    "c4e37d39e101",  /* vextracti128 $1, %ymm4, %xmm1 */
    "c4630120c603",  /* vpinsrb $3, %esi, %xmm15, %xmm8 */
    "c4c37916f202",  /* vpextrd $2, %xmm6, %r10d */
    "62e13c4858d3",  /* vaddps %zmm3, %zmm8, %zmm18 */
    "62e14c4158f8",  /* vaddps %zmm0, %zmm22, %zmm23{%k1} */
    "62d124c758ec",  /* vaddps %zmm12, %zmm27, %zmm5{%k7}{z} */
    "62319d4859e2",  /* vmulpd %zmm18, %zmm12, %zmm12 */
    "62f1d54959c5",  /* vmulpd %zmm5, %zmm5, %zmm0{%k1} */
    "62b1a5c159e2",  /* vmulpd %zmm18, %zmm27, %zmm4{%k1}{z} */
    "62916548fe9c71acffffff",  /* vpaddd -84(%r9,%r14,2), %zmm3, %zmm3 */
    "62c14d4dfee0",  /* vpaddd %zmm8, %zmm6, %zmm20{%k5} */
    "623175c5fec5",  /* vpaddd %zmm21, %zmm17, %zmm8{%k5}{z} */
    "6271fd48d4d4",  /* vpaddq %zmm4, %zmm0, %zmm10 */
    "6241c542d4e9",  /* vpaddq %zmm9, %zmm23, %zmm29{%k2} */
    "6201adced4f4",  /* vpaddq %zmm28, %zmm10, %zmm30{%k6}{z} */
    "62e11d48efc1",  /* vpxord %zmm1, %zmm12, %zmm16 */
    "62115543efd4",  /* vpxord %zmm28, %zmm21, %zmm10{%k3} */
    "62c15dc7efda",  /* vpxord %zmm10, %zmm20, %zmm19{%k7}{z} */
    "6281ed40dbce",  /* vpandq %zmm30, %zmm18, %zmm17 */
    "62e18d4edbee",  /* vpandq %zmm6, %zmm14, %zmm21{%k6} */
    "6241a5c9dbe4",  /* vpandq %zmm12, %zmm11, %zmm28{%k1}{z} */
    "62f26d483dc0",  /* vpmaxsd %zmm0, %zmm2, %zmm0 */
    "62520d4b3dfb",  /* vpmaxsd %zmm11, %zmm14, %zmm15{%k3} */
    "62123dc63df9",  /* vpmaxsd %zmm25, %zmm24, %zmm15{%k6}{z} */
    "62936d401feb07",  /* vpcmpd $7, %zmm27, %zmm18, %k5 */
    "62b36d481ecd01",  /* vpcmpud $1, %zmm21, %zmm2, %k1 */
    "62f3ad401fcb02",  /* vpcmpq $2, %zmm3, %zmm26, %k1 */
    "62117ecc6fd1",  /* vmovdqu32 %zmm25, %zmm10{%k4}{z} */
    "dcc4",  /* fadd %st, %st(4) */
    "d8c5",  /* fadd %st(5), %st */
    "41d807",  /* fadds (%r15) */
    "dc0410",  /* faddl (%rax,%rdx,1) */
    "dce2",  /* fsub %st, %st(2) */
    "d824d7",  /* fsubs (%rdi,%rdx,8) */
    "dccb",  /* fmul %st, %st(3) */
    "d8c8",  /* fmul %st(0), %st */
    "41d84f7f",  /* fmuls 127(%r15) */
    "dc4e7f",  /* fmull 127(%rsi) */
    "dcf2",  /* fdiv %st, %st(2) */
    "d8f1",  /* fdiv %st(1), %st */
    "d831",  /* fdivs (%rcx) */
    "41dc344c",  /* fdivl (%r12,%rcx,2) */
    "d86af8",  /* fsubrs -8(%rdx) */
    "dc6c5141",  /* fsubrl 65(%rcx,%rdx,2) */
    "d8fb",  /* fdivr %st(3), %st */
    "d8bcfd16ffffff",  /* fdivrs -234(%rbp,%rdi,8) */
    "dc7908",  /* fdivrl 8(%rcx) */
    "d9c1",  /* fld %st(1) */
    "ddd5",  /* fst %st(5) */
    "d9c8",  /* fxch %st(0) */
    "d944f36b",  /* flds 107(%rbx,%rsi,8) */
    "43dd44c7bd",  /* fldl -67(%r15,%r8,8) */
    "41d95780",  /* fsts -128(%r15) */
    "dd16",  /* fstl (%rsi) */
    "41d99cc79d000000",  /* fstps 157(%r15,%rax,8) */
    "42dd1c27",  /* fstpl (%rdi,%r12,1) */
    "41df00",  /* filds (%r8) */
    "41db8700010000",  /* fildl 256(%r15) */
    "df1a",  /* fistp (%rdx) */
    "df2cba",  /* fildll (%rdx,%rdi,4) */
    "0ffcc4",  /* paddb %mm4, %mm0 */
    "0ffdeb",  /* paddw %mm3, %mm5 */
    "0ffed5",  /* paddd %mm5, %mm2 */
    "0fd4d4",  /* paddq %mm4, %mm2 */
    "0ff8fc",  /* psubb %mm4, %mm7 */
    "0fdbf1",  /* pand %mm1, %mm6 */
    "0feff8",  /* pxor %mm0, %mm7 */
    "0f74f2",  /* pcmpeqb %mm2, %mm6 */
    "0fd5d5",  /* pmullw %mm5, %mm2 */
    "0f60d7",  /* punpcklbw %mm7, %mm2 */
    "0ff5e7",  /* pmaddwd %mm7, %mm4 */
    "0f71f709",  /* psllw $9, %mm7 */
    "0f73d603",  /* psrlq $3, %mm6 */
    "0f71e705",  /* psraw $5, %mm7 */
    "0f6fe7",  /* movq %mm7, %mm4 */
    "410f6eed",  /* movd %r13d, %mm5 */
    "0f70e020",  /* pshufw $32, %mm0, %mm4 */
    "c4c15458c2",  /* vaddps %ymm10, %ymm5, %ymm0 */
    "c441345cde",  /* vsubps %ymm14, %ymm9, %ymm11 */
    "c4412459d6",  /* vmulps %ymm14, %ymm11, %ymm10 */
    "c441005ecb",  /* vdivps %xmm11, %xmm15, %xmm9 */
    "c4c1385dc1",  /* vminps %xmm9, %xmm8, %xmm0 */
    "c4c1505ffe",  /* vmaxps %xmm14, %xmm5, %xmm7 */
    "c4c14158ef",  /* vaddpd %xmm15, %xmm7, %xmm5 */
    "c4415159fa",  /* vmulpd %xmm10, %xmm5, %xmm15 */
    "c441095cdd",  /* vsubpd %xmm13, %xmm14, %xmm11 */
    "c5cc54fa",  /* vandps %ymm2, %ymm6, %ymm7 */
    "c5a556cb",  /* vorpd %ymm3, %ymm11, %ymm1 */
    "c4415c57eb",  /* vxorps %ymm11, %ymm4, %ymm13 */
    "c4c10c14e3",  /* vunpcklps %ymm11, %ymm14, %ymm4 */
    "c5d915d6",  /* vunpckhpd %xmm6, %xmm4, %xmm2 */
    "c4416258d4",  /* vaddss %xmm12, %xmm3, %xmm10 */
    "c52b59e4",  /* vmulsd %xmm4, %xmm10, %xmm12 */
    "c441025ce1",  /* vsubss %xmm9, %xmm15, %xmm12 */
    "c441335ef8",  /* vdivsd %xmm8, %xmm9, %xmm15 */
    "c441025dc9",  /* vminss %xmm9, %xmm15, %xmm9 */
    "c441435fce",  /* vmaxsd %xmm14, %xmm7, %xmm9 */
    "c50251c0",  /* vsqrtss %xmm0, %xmm15, %xmm8 */
    "c57851ff",  /* vsqrtps %xmm7, %xmm15 */
    "c4a17d515cc12c",  /* vsqrtpd 44(%rcx,%r8,8), %ymm3 */
    "c42178c274a65808",  /* vcmpps $8, 88(%rsi,%r12,4), %xmm0, %xmm14 */
    "c5f5c2c417",  /* vcmppd $23, %ymm4, %ymm1, %ymm0 */
    "c57ac2ee04",  /* vcmpss $4, %xmm6, %xmm0, %xmm13 */
    "c4c10cc6d1f6",  /* vshufps $246, %ymm9, %ymm14, %ymm2 */
    "c4c135c6e701",  /* vshufpd $1, %ymm15, %ymm9, %ymm4 */
    "c4c3290ce7f4",  /* vblendps $244, %xmm15, %xmm10, %xmm4 */
    "c4e36d0dc218",  /* vblendpd $24, %ymm2, %ymm2, %ymm0 */
    "c4e3650ed9db",  /* vpblendw $219, %ymm1, %ymm3, %ymm3 */
    "c4c35940ee38",  /* vdpps $56, %xmm14, %xmm4, %xmm5 */
    "c5f1fcde",  /* vpaddb %xmm6, %xmm1, %xmm3 */
    "c4c175fed8",  /* vpaddd %ymm8, %ymm1, %ymm3 */
    "c4416dd4d5",  /* vpaddq %ymm13, %ymm2, %ymm10 */
    "c4415df9c9",  /* vpsubw %ymm9, %ymm4, %ymm9 */
    "c539dbcf",  /* vpand %xmm7, %xmm8, %xmm9 */
    "c5d9ebc5",  /* vpor %xmm5, %xmm4, %xmm0 */
    "c5edefd9",  /* vpxor %ymm1, %ymm2, %ymm3 */
    "c4c16574c2",  /* vpcmpeqb %ymm10, %ymm3, %ymm0 */
    "c5e566d7",  /* vpcmpgtd %ymm7, %ymm3, %ymm2 */
    "c4c2253dfc",  /* vpmaxsd %ymm12, %ymm11, %ymm7 */
    "c4421d3be3",  /* vpminud %ymm11, %ymm12, %ymm12 */
    "c4c27140f3",  /* vpmulld %xmm11, %xmm1, %xmm6 */
    "c5f160cb",  /* vpunpcklbw %xmm3, %xmm1, %xmm1 */
    "c44101f5df",  /* vpmaddwd %xmm15, %xmm15, %xmm11 */
    "c4420900f2",  /* vpshufb %xmm10, %xmm14, %xmm14 */
    "c4417970e30c",  /* vpshufd $12, %xmm11, %xmm12 */
    "c4c14572f109",  /* vpslld $9, %ymm9, %ymm7 */
    "c58973d01e",  /* vpsrlq $30, %xmm0, %xmm14 */
    "c4417c28ca",  /* vmovaps %ymm10, %ymm9 */
    "c4417c29b678563412",  /* vmovaps %ymm14, 305419896(%r14) */
    "c5f810f8",  /* vmovups %xmm0, %xmm7 */
    "c481781114bc",  /* vmovups %xmm2, (%r12,%r15,4) */
    "c57d28f2",  /* vmovapd %ymm2, %ymm14 */
    "c5fd29aa78563412",  /* vmovapd %ymm5, 305419896(%rdx) */
    "c441796fc5",  /* vmovdqa %xmm13, %xmm8 */
    "c4c1797f00",  /* vmovdqa %xmm0, (%r8) */
    "c57e6fd9",  /* vmovdqu %ymm1, %ymm11 */
    "c4c17e7f1f",  /* vmovdqu %ymm3, (%r15) */
    "c5796efa",  /* vmovd %edx, %xmm15 */
    "c461f96efb",  /* vmovq %rbx, %xmm15 */
    "c5fa108878563412",  /* vmovss 305419896(%rax), %xmm1 */
    "c51b11f9",  /* vmovsd %xmm15, %xmm12, %xmm1 */
    "c4e27198f1",  /* vfmadd132ps %xmm1, %xmm1, %xmm6 */
    "c4c2c9a8fc",  /* vfmadd213pd %xmm12, %xmm6, %xmm7 */
    "c4e251bad2",  /* vfmsub231ps %xmm2, %xmm5, %xmm2 */
    "c44221acfe",  /* vfnmadd213ps %xmm14, %xmm11, %xmm15 */
    "c4c259a9c4",  /* vfmadd213ss %xmm12, %xmm4, %xmm0 */
    "c462b1bffc",  /* vfnmsub231sd %xmm4, %xmm9, %xmm15 */
    "c4e27d13d4",  /* vcvtph2ps %xmm4, %ymm2 */
    "c4437d1dfd00",  /* vcvtps2ph $0, %ymm15, %xmm13 */
    "c57c5bf9",  /* vcvtdq2ps %ymm1, %ymm15 */
    "c57a5bce",  /* vcvttps2dq %xmm6, %xmm9 */
    "c4e1e32ad7",  /* vcvtsi2sd %rdi, %xmm3, %xmm2 */
    "c4c27d18e1",  /* vbroadcastss %xmm9, %ymm4 */
    "c4627d58f0",  /* vpbroadcastd %xmm0, %ymm14 */
    "c463fd00e949",  /* vpermq $73, %ymm1, %ymm13 */
    "c4e31d06e389",  /* vperm2f128 $137, %ymm3, %ymm12, %ymm4 */
    "c4436d18ca01",  /* vinsertf128 $1, %xmm10, %ymm2, %ymm9 */
    "c4c37d39e901",  /* vextracti128 $1, %ymm5, %xmm9 */
    "c4e36920c003",  /* vpinsrb $3, %eax, %xmm2, %xmm0 */
    "c4e37916c302",  /* vpextrd $2, %xmm0, %ebx */
    "62917c4058cb",  /* vaddps %zmm27, %zmm16, %zmm1 */
    "62813c4a58fb",  /* vaddps %zmm27, %zmm8, %zmm23{%k2} */
    "62b14cc258c5",  /* vaddps %zmm21, %zmm22, %zmm0{%k2}{z} */
    "62c1cd4859ca",  /* vmulpd %zmm10, %zmm6, %zmm17 */
    "6241b54f59e2",  /* vmulpd %zmm10, %zmm9, %zmm28{%k7} */
    "6291a5c359fb",  /* vmulpd %zmm27, %zmm27, %zmm7{%k3}{z} */
    "62012d48fee3",  /* vpaddd %zmm27, %zmm10, %zmm28 */
    "62910541fec6",  /* vpaddd %zmm30, %zmm31, %zmm0{%k1} */
    "62d12dc2fed8",  /* vpaddd %zmm8, %zmm26, %zmm3{%k2}{z} */
    "62c18d40d4f7",  /* vpaddq %zmm15, %zmm30, %zmm22 */
    "62a19d4dd4d9",  /* vpaddq %zmm17, %zmm12, %zmm19{%k5} */
    "62a185cbd4fe",  /* vpaddq %zmm22, %zmm15, %zmm23{%k3}{z} */
    "62813540eff7",  /* vpxord %zmm31, %zmm25, %zmm22 */
    "62913d41efd7",  /* vpxord %zmm31, %zmm24, %zmm2{%k1} */
    "62d135c7eff2",  /* vpxord %zmm10, %zmm25, %zmm6{%k7}{z} */
    "6211d548dbe6",  /* vpandq %zmm30, %zmm5, %zmm12 */
    "6261a549dbc6",  /* vpandq %zmm6, %zmm11, %zmm24{%k1} */
    "62a1f5cddbdc",  /* vpandq %zmm20, %zmm1, %zmm19{%k5}{z} */
    "620235403de6",  /* vpmaxsd %zmm30, %zmm25, %zmm28 */
    "6262354b3dd3",  /* vpmaxsd %zmm3, %zmm9, %zmm26{%k3} */
    "62e215cb3dd1",  /* vpmaxsd %zmm1, %zmm13, %zmm18{%k3}{z} */
    "62f355481ff901",  /* vpcmpd $1, %zmm1, %zmm5, %k7 */
    "629365401eca05",  /* vpcmpud $5, %zmm26, %zmm19, %k1 */
    "62d3dd481fcf04",  /* vpcmpq $4, %zmm15, %zmm4, %k1 */
    "62e17ecb6fdd",  /* vmovdqu32 %zmm5, %zmm19{%k3}{z} */
    "dcc1",  /* fadd %st, %st(1) */
    "d8c7",  /* fadd %st(7), %st */
    "dc8078563412",  /* faddl 305419896(%rax) */
    "dce7",  /* fsub %st, %st(7) */
    "d8e5",  /* fsub %st(5), %st */
    "d827",  /* fsubs (%rdi) */
    "dc24d6",  /* fsubl (%rsi,%rdx,8) */
    "dcc9",  /* fmul %st, %st(1) */
    "41d80c24",  /* fmuls (%r12) */
    "41dc8caf5affffff",  /* fmull -166(%r15,%rbp,4) */
    "dcf1",  /* fdiv %st, %st(1) */
    "d8f5",  /* fdiv %st(5), %st */
    "42d8b43dc7000000",  /* fdivs 199(%rbp,%r15,1) */
    "dcb44263ffffff",  /* fdivl -157(%rdx,%rax,2) */
    "dced",  /* fsubr %st, %st(5) */
    "d82a",  /* fsubrs (%rdx) */
    "41dc2c24",  /* fsubrl (%r12) */
    "d8f8",  /* fdivr %st, %st(0) */
    "d83a",  /* fdivrs (%rdx) */
    "42dc7c256d",  /* fdivrl 109(%rbp,%r12,1) */
    "ddd9",  /* fstp %st(1) */
    "ddc4",  /* ffree %st(4) */
    "41d9477f",  /* flds 127(%r15) */
    "41dd40f8",  /* fldl -8(%r8) */
    "41d9942478563412",  /* fsts 305419896(%r12) */
    "41dd11",  /* fstl (%r9) */
    "d95a80",  /* fstps -128(%rdx) */
    "41dd5cfe39",  /* fstpl 57(%r14,%rdi,8) */
    "41df0471",  /* filds (%r9,%rsi,2) */
    "41db0446",  /* fildl (%r14,%rax,2) */
    "df5d00",  /* fistp (%rbp) */
    "41df2cdf",  /* fildll (%r15,%rbx,8) */
    "0ffcff",  /* paddb %mm7, %mm7 */
    "0ffd7500",  /* paddw (%rbp), %mm6 */
    "0ffefc",  /* paddd %mm4, %mm7 */
    "0fd4c8",  /* paddq %mm0, %mm1 */
    "0ff8dd",  /* psubb %mm5, %mm3 */
    "0fdbef",  /* pand %mm7, %mm5 */
    "0febfc",  /* por %mm4, %mm7 */
    "0feffe",  /* pxor %mm6, %mm7 */
    "0fd5ec",  /* pmullw %mm4, %mm5 */
    "0f60c4",  /* punpcklbw %mm4, %mm0 */
    "0ff5c8",  /* pmaddwd %mm0, %mm1 */
    "0f71f50b",  /* psllw $11, %mm5 */
    "0f72f00f",  /* pslld $15, %mm0 */
    "0f73d206",  /* psrlq $6, %mm2 */
    "0f71e008",  /* psraw $8, %mm0 */
    "0f6fcf",  /* movq %mm7, %mm1 */
    "410f6eda",  /* movd %r10d, %mm3 */
    "0f70d533",  /* pshufw $51, %mm5, %mm2 */
    "c57058fb",  /* vaddps %xmm3, %xmm1, %xmm15 */
    "c5dc5cd0",  /* vsubps %ymm0, %ymm4, %ymm2 */
    "c54859f4",  /* vmulps %xmm4, %xmm6, %xmm14 */
    "c4416c5ecf",  /* vdivps %ymm15, %ymm2, %ymm9 */
    "c4413c5df8",  /* vminps %ymm8, %ymm8, %ymm15 */
    "c5c45fcf",  /* vmaxps %ymm7, %ymm7, %ymm1 */
    "c4c12958d5",  /* vaddpd %xmm13, %xmm10, %xmm2 */
    "c4415959f4",  /* vmulpd %xmm12, %xmm4, %xmm14 */
    "c56d5cdc",  /* vsubpd %ymm4, %ymm2, %ymm11 */
    "c55054cc",  /* vandps %xmm4, %xmm5, %xmm9 */
    "c4c14156fc",  /* vorpd %xmm12, %xmm7, %xmm7 */
    "c57857e7",  /* vxorps %xmm7, %xmm0, %xmm12 */
    "c59814e0",  /* vunpcklps %xmm0, %xmm12, %xmm4 */
    "c4c13915eb",  /* vunpckhpd %xmm11, %xmm8, %xmm5 */
    "c59a58d4",  /* vaddss %xmm4, %xmm12, %xmm2 */
    "c58359c9",  /* vmulsd %xmm1, %xmm15, %xmm1 */
    "c5a25ccb",  /* vsubss %xmm3, %xmm11, %xmm1 */
    "c5935eff",  /* vdivsd %xmm7, %xmm13, %xmm7 */
    "c5525df1",  /* vminss %xmm1, %xmm5, %xmm14 */
    "c5f35fc5",  /* vmaxsd %xmm5, %xmm1, %xmm0 */
    "c4c10a51f8",  /* vsqrtss %xmm8, %xmm14, %xmm7 */
    "c57851c6",  /* vsqrtps %xmm6, %xmm8 */
    "c4c17951d2",  /* vsqrtpd %xmm10, %xmm2 */
    "c578c2df17",  /* vcmpps $23, %xmm7, %xmm0, %xmm11 */
    "c4c15dc2c31b",  /* vcmppd $27, %ymm11, %ymm4, %ymm0 */
    "c4417ac2f700",  /* vcmpss $0, %xmm15, %xmm0, %xmm14 */
    "c4c144c6e02c",  /* vshufps $44, %ymm8, %ymm7, %ymm4 */
    "c44111c6fcd6",  /* vshufpd $214, %xmm12, %xmm13, %xmm15 */
    "c4e3550cdb81",  /* vblendps $129, %ymm3, %ymm5, %ymm3 */
    "c443010dfc52",  /* vblendpd $82, %xmm12, %xmm15, %xmm15 */
    "c4632d0ee86e",  /* vpblendw $110, %ymm0, %ymm10, %ymm13 */
    "c4637d40cfa3",  /* vdpps $163, %ymm7, %ymm0, %ymm9 */
    "c549fce2",  /* vpaddb %xmm2, %xmm6, %xmm12 */
    "c505fed9",  /* vpaddd %ymm1, %ymm15, %ymm11 */
    "c4c13dd4d4",  /* vpaddq %ymm12, %ymm8, %ymm2 */
    "c5ddf9db",  /* vpsubw %ymm3, %ymm4, %ymm3 */
    "c521dbd6",  /* vpand %xmm6, %xmm11, %xmm10 */
    "c5c1ebe5",  /* vpor %xmm5, %xmm7, %xmm4 */
    "c58deff4",  /* vpxor %ymm4, %ymm14, %ymm6 */
    "c4416d74c0",  /* vpcmpeqb %ymm8, %ymm2, %ymm8 */
    "c4410966d7",  /* vpcmpgtd %xmm15, %xmm14, %xmm10 */
    "c4e2213de3",  /* vpmaxsd %xmm3, %xmm11, %xmm4 */
    "c4e2593bd8",  /* vpminud %xmm0, %xmm4, %xmm3 */
    "c4620540c4",  /* vpmulld %ymm4, %ymm15, %ymm8 */
    "c4c14960ec",  /* vpunpcklbw %xmm12, %xmm6, %xmm5 */
    "c4c175f5fe",  /* vpmaddwd %ymm14, %ymm1, %ymm7 */
    "c4627100d5",  /* vpshufb %xmm5, %xmm1, %xmm10 */
    "c57970c10d",  /* vpshufd $13, %xmm1, %xmm8 */
    "c5ed72f017",  /* vpslld $23, %ymm0, %ymm2 */
    "c58573d70b",  /* vpsrlq $11, %ymm7, %ymm15 */
    "c57c29db",  /* vmovaps %ymm11, %ymm3 */
    "c5fc295500",  /* vmovaps %ymm2, (%rbp) */
    "c57810f9",  /* vmovups %xmm1, %xmm15 */
    "c441781144f171",  /* vmovups %xmm8, 113(%r9,%rsi,8) */
    "c57d29ed",  /* vmovapd %ymm13, %ymm5 */
    "c57d2900",  /* vmovapd %ymm8, (%rax) */
    "c5fd6fd1",  /* vmovdqa %ymm1, %ymm2 */
    "c5fd7f1c28",  /* vmovdqa %ymm3, (%rax,%rbp,1) */
    "c4417a6fc8",  /* vmovdqu %xmm8, %xmm9 */
    "c4217a7f5c2500",  /* vmovdqu %xmm11, (%rbp,%r12,1) */
    "c5796ecb",  /* vmovd %ebx, %xmm9 */
    "c441f96ef0",  /* vmovq %r8, %xmm14 */
    "c4c17a104c173d",  /* vmovss 61(%r15,%rdx,1), %xmm1 */
    "c55b11e7",  /* vmovsd %xmm12, %xmm4, %xmm7 */
    "c4e23d98e3",  /* vfmadd132ps %ymm3, %ymm8, %ymm4 */
    "c462e9a8c6",  /* vfmadd213pd %xmm6, %xmm2, %xmm8 */
    "c46249bad0",  /* vfmsub231ps %xmm0, %xmm6, %xmm10 */
    "c4422dacf6",  /* vfnmadd213ps %ymm14, %ymm10, %ymm14 */
    "c4e241a9e3",  /* vfmadd213ss %xmm3, %xmm7, %xmm4 */
    "c44289bfc3",  /* vfnmsub231sd %xmm11, %xmm14, %xmm8 */
    "c4e37d1dc300",  /* vcvtps2ph $0, %ymm0, %xmm3 */
    "c4417c5bda",  /* vcvtdq2ps %ymm10, %ymm11 */
    "c4417a5bdb",  /* vcvttps2dq %xmm11, %xmm11 */
    "c441f32aec",  /* vcvtsi2sd %r12, %xmm1, %xmm13 */
    "c4627d18cd",  /* vbroadcastss %xmm5, %ymm9 */
    "c4427d58f9",  /* vpbroadcastd %xmm9, %ymm15 */
    "c443fd00eb7b",  /* vpermq $123, %ymm11, %ymm13 */
    "c4436d06f2ef",  /* vperm2f128 $239, %ymm10, %ymm2, %ymm14 */
    "c4c30518c501",  /* vinsertf128 $1, %xmm13, %ymm15, %ymm0 */
    "c4c37d39df01",  /* vextracti128 $1, %ymm3, %xmm15 */
    "c4c37120c003",  /* vpinsrb $3, %r8d, %xmm1, %xmm0 */
    "c4637916df02",  /* vpextrd $2, %xmm11, %edi */
    "62f1344058fc",  /* vaddps %zmm4, %zmm25, %zmm7 */
    "62a13c4658f8",  /* vaddps %zmm16, %zmm24, %zmm23{%k6} */
    "62614cc358d0",  /* vaddps %zmm0, %zmm22, %zmm26{%k3}{z} */
    "62f1954859c1",  /* vmulpd %zmm1, %zmm13, %zmm0 */
    "6201cd4c59d6",  /* vmulpd %zmm30, %zmm6, %zmm26{%k4} */
    "62318dc259e4",  /* vmulpd %zmm20, %zmm30, %zmm12{%k2}{z} */
    "62516548fef5",  /* vpaddd %zmm13, %zmm3, %zmm14 */
    "62114543fece",  /* vpaddd %zmm30, %zmm23, %zmm9{%k3} */
    "62b11dcdfed9",  /* vpaddd %zmm17, %zmm12, %zmm3{%k5}{z} */
    "62f1a540d4ca",  /* vpaddq %zmm2, %zmm27, %zmm1 */
    "6271c54dd4e5",  /* vpaddq %zmm5, %zmm7, %zmm12{%k5} */
    "62e18dc6d4ee",  /* vpaddq %zmm6, %zmm30, %zmm21{%k6}{z} */
    "62515d40efee",  /* vpxord %zmm14, %zmm20, %zmm13 */
    "62b13549efc2",  /* vpxord %zmm18, %zmm9, %zmm0{%k1} */
    "62214dcdefca",  /* vpxord %zmm18, %zmm6, %zmm25{%k5}{z} */
    "62918540dbea",  /* vpandq %zmm26, %zmm31, %zmm5 */
    "6251bd4edbe2",  /* vpandq %zmm10, %zmm8, %zmm12{%k6} */
    "62a1adc6dbda",  /* vpandq %zmm18, %zmm26, %zmm19{%k6}{z} */
    "621215483de4",  /* vpmaxsd %zmm28, %zmm13, %zmm12 */
    "629245463ddb",  /* vpmaxsd %zmm27, %zmm23, %zmm3{%k6} */
    "62a245c43df4",  /* vpmaxsd %zmm20, %zmm23, %zmm22{%k4}{z} */
    "62b32d401ffe06",  /* vpcmpd $6, %zmm22, %zmm26, %k7 */
    "62d37d401ef104",  /* vpcmpud $4, %zmm9, %zmm16, %k6 */
    "6293a5481ff503",  /* vpcmpq $3, %zmm29, %zmm11, %k6 */
    "62717ec96fc8",  /* vmovdqu32 %zmm0, %zmm9{%k1}{z} */
    "dcc5",  /* fadd %st, %st(5) */
    "d84500",  /* fadds (%rbp) */
    "41d82424",  /* fsubs (%r12) */
    "41dc6180",  /* fsubl -128(%r9) */
    "d84d00",  /* fmuls (%rbp) */
    "dc0a",  /* fmull (%rdx) */
    "41d83424",  /* fdivs (%r12) */
    "d8ee",  /* fsubr %st(6), %st */
    "d86880",  /* fsubrs -128(%rax) */
    "41dc6908",  /* fsubrl 8(%r9) */
    "dcfe",  /* fdivr %st, %st(6) */
    "d8fa",  /* fdivr %st(2), %st */
    "41d839",  /* fdivrs (%r9) */
    "41dc7c5cb9",  /* fdivrl -71(%r12,%rbx,2) */
    "d9c5",  /* fld %st(5) */
    "ddd1",  /* fst %st(1) */
    "ddc3",  /* ffree %st(3) */
    "41d900",  /* flds (%r8) */
    "42dd84e5dc000000",  /* fldl 220(%rbp,%r12,8) */
    "d9544ffe",  /* fsts -2(%rdi,%rcx,2) */
    "dd13",  /* fstl (%rbx) */
    "d91e",  /* fstps (%rsi) */
    "41dd9c30cb000000",  /* fstpl 203(%r8,%rsi,1) */
    "df03",  /* filds (%rbx) */
    "db4508",  /* fildl 8(%rbp) */
    "df1c29",  /* fistp (%rcx,%rbp,1) */
    "41df2e",  /* fildll (%r14) */
    "0ffced",  /* paddb %mm5, %mm5 */
    "0ffdc9",  /* paddw %mm1, %mm1 */
    "0ffee0",  /* paddd %mm0, %mm4 */
    "0fd4e7",  /* paddq %mm7, %mm4 */
    "0ff8f8",  /* psubb %mm0, %mm7 */
    "0fdbf9",  /* pand %mm1, %mm7 */
    "0febda",  /* por %mm2, %mm3 */
    "0feffa",  /* pxor %mm2, %mm7 */
    "0f74d4",  /* pcmpeqb %mm4, %mm2 */
    "0f60d1",  /* punpcklbw %mm1, %mm2 */
    "0ff5c3",  /* pmaddwd %mm3, %mm0 */
    "0f71f20f",  /* psllw $15, %mm2 */
    "0f72f408",  /* pslld $8, %mm4 */
    "0f73d200",  /* psrlq $0, %mm2 */
    "0f71e50d",  /* psraw $13, %mm5 */
    "0f6fd5",  /* movq %mm5, %mm2 */
    "410f6ec5",  /* movd %r13d, %mm0 */
    "0f70f6e9",  /* pshufw $233, %mm6, %mm6 */
    "c4c11c58cf",  /* vaddps %ymm15, %ymm12, %ymm1 */
    "c5605cd5",  /* vsubps %xmm5, %xmm3, %xmm10 */
    "c56059dc",  /* vmulps %xmm4, %xmm3, %xmm11 */
    "c441585ec4",  /* vdivps %xmm12, %xmm4, %xmm8 */
    "c4410c5dff",  /* vminps %ymm15, %ymm14, %ymm15 */
    "c5645fd4",  /* vmaxps %ymm4, %ymm3, %ymm10 */
    "c54d58d8",  /* vaddpd %ymm0, %ymm6, %ymm11 */
    "c5b159d0",  /* vmulpd %xmm0, %xmm9, %xmm2 */
    "c4c12d5c7c79ab",  /* vsubpd -85(%r9,%rdi,2), %ymm10, %ymm7 */
    "c50054de",  /* vandps %xmm6, %xmm15, %xmm11 */
    "c4410956db",  /* vorpd %xmm11, %xmm14, %xmm11 */
    "c58457f7",  /* vxorps %ymm7, %ymm15, %ymm6 */
    "c4c12414d7",  /* vunpcklps %ymm15, %ymm11, %ymm2 */
    "c51115d5",  /* vunpckhpd %xmm5, %xmm13, %xmm10 */
    "c5d258ea",  /* vaddss %xmm2, %xmm5, %xmm5 */
    "c58b59e4",  /* vmulsd %xmm4, %xmm14, %xmm4 */
    "c4c1225ce7",  /* vsubss %xmm15, %xmm11, %xmm4 */
    "c5fb5ec4",  /* vdivsd %xmm4, %xmm0, %xmm0 */
    "c4415a5d2f",  /* vminss (%r15), %xmm4, %xmm13 */
    "c4414b5fc8",  /* vmaxsd %xmm8, %xmm6, %xmm9 */
    "c4413a51ea",  /* vsqrtss %xmm10, %xmm8, %xmm13 */
    "c4a17851a4f0d5000000",  /* vsqrtps 213(%rax,%r14,8), %xmm4 */
    "c57951f0",  /* vsqrtpd %xmm0, %xmm14 */
    "c4c160c2cf00",  /* vcmpps $0, %xmm15, %xmm3, %xmm1 */
    "c5c5c2f11e",  /* vcmppd $30, %ymm1, %ymm7, %ymm6 */
    "c58ac20004",  /* vcmpss $4, (%rax), %xmm14, %xmm0 */
    "c548c607e2",  /* vshufps $226, (%rdi), %xmm6, %xmm8 */
    "c4c13dc6b178563412f4",  /* vshufpd $244, 305419896(%r9), %ymm8, %ymm6 */
    "c4430d0cd446",  /* vblendps $70, %ymm12, %ymm14, %ymm10 */
    "c4e3310dde7b",  /* vblendpd $123, %xmm6, %xmm9, %xmm3 */
    "c463190efbbb",  /* vpblendw $187, %xmm3, %xmm12, %xmm15 */
    "c4c31140f272",  /* vdpps $114, %xmm10, %xmm13, %xmm6 */
    "c4417dfcd3",  /* vpaddb %ymm11, %ymm0, %ymm10 */
    "c4412dfecc",  /* vpaddd %ymm12, %ymm10, %ymm9 */
    "c539d4c2",  /* vpaddq %xmm2, %xmm8, %xmm8 */
    "c44159f9fc",  /* vpsubw %xmm12, %xmm4, %xmm15 */
    "c4416ddbc5",  /* vpand %ymm13, %ymm2, %ymm8 */
    "c4c169ebf8",  /* vpor %xmm8, %xmm2, %xmm7 */
    "c44179efd6",  /* vpxor %xmm14, %xmm0, %xmm10 */
    "c5f574fc",  /* vpcmpeqb %ymm4, %ymm1, %ymm7 */
    "c4c13166f1",  /* vpcmpgtd %xmm9, %xmm9, %xmm6 */
    "c4623d3ddd",  /* vpmaxsd %ymm5, %ymm8, %ymm11 */
    "c4e2793bf0",  /* vpminud %xmm0, %xmm0, %xmm6 */
    "c4421d40cf",  /* vpmulld %ymm15, %ymm12, %ymm9 */
    "c4c12160cc",  /* vpunpcklbw %xmm12, %xmm11, %xmm1 */
    "c4c111f5e8",  /* vpmaddwd %xmm8, %xmm13, %xmm5 */
    "c4e23100cf",  /* vpshufb %xmm7, %xmm9, %xmm1 */
    "c4417d70d30b",  /* vpshufd $11, %ymm11, %ymm10 */
    "c5ad72f00f",  /* vpslld $15, %ymm0, %ymm10 */
    "c5bd73d205",  /* vpsrlq $5, %ymm2, %ymm8 */
    "c57829e0",  /* vmovaps %xmm12, %xmm0 */
    "c578294e80",  /* vmovaps %xmm9, -128(%rsi) */
    "c4417810d4",  /* vmovups %xmm12, %xmm10 */
    "c578117cc194",  /* vmovups %xmm15, -108(%rcx,%rax,8) */
    "c5f928f8",  /* vmovapd %xmm0, %xmm7 */
    "c5f92910",  /* vmovapd %xmm2, (%rax) */
    "c5f96fc1",  /* vmovdqa %xmm1, %xmm0 */
    "c481797fbc0003ffffff",  /* vmovdqa %xmm7, -253(%r8,%r8,1) */
    "c57a6ffa",  /* vmovdqu %xmm2, %xmm15 */
    "c4217a7f447bf0",  /* vmovdqu %xmm8, -16(%rbx,%r15,2) */
    "c5796ef2",  /* vmovd %edx, %xmm14 */
    "c441f96ee0",  /* vmovq %r8, %xmm12 */
    "c4a17a100cb3",  /* vmovss (%rbx,%r14,4), %xmm1 */
    "c5eb10e6",  /* vmovsd %xmm6, %xmm2, %xmm4 */
    "c4620598c1",  /* vfmadd132ps %ymm1, %ymm15, %ymm8 */
    "c442b1a8cf",  /* vfmadd213pd %xmm15, %xmm9, %xmm9 */
    "c4426dbaf9",  /* vfmsub231ps %ymm9, %ymm2, %ymm15 */
    "c4c219acf0",  /* vfnmadd213ps %xmm8, %xmm12, %xmm6 */
    "c46259a9c2",  /* vfmadd213ss %xmm2, %xmm4, %xmm8 */
    "c4e2b9bf9f00010000",  /* vfnmsub231sd 256(%rdi), %xmm8, %xmm3 */
    "c4c27d13c4",  /* vcvtph2ps %xmm12, %ymm0 */
    "c4c37d1dec00",  /* vcvtps2ph $0, %ymm5, %xmm12 */
    "c4417c5be3",  /* vcvtdq2ps %ymm11, %ymm12 */
    "c4c17a5bc2",  /* vcvttps2dq %xmm10, %xmm0 */
    "c461ab2ae7",  /* vcvtsi2sd %rdi, %xmm10, %xmm12 */
    "c4627d18c8",  /* vbroadcastss %xmm0, %ymm9 */
    "c4427d58d8",  /* vpbroadcastd %xmm8, %ymm11 */
    "c443fd00eb21",  /* vpermq $33, %ymm11, %ymm13 */
    "c4e33d06d98c",  /* vperm2f128 $140, %ymm1, %ymm8, %ymm3 */
    "c4e33518e101",  /* vinsertf128 $1, %xmm1, %ymm9, %ymm4 */
    "c4e37d39fe01",  /* vextracti128 $1, %ymm7, %xmm6 */
    "c4e31120c703",  /* vpinsrb $3, %edi, %xmm13, %xmm0 */
    "c4437916e502",  /* vpextrd $2, %xmm12, %r13d */
    "6261644858d5",  /* vaddps %zmm5, %zmm3, %zmm26 */
    "62416c4158f8",  /* vaddps %zmm8, %zmm18, %zmm31{%k1} */
    "62412ccc58f0",  /* vaddps %zmm8, %zmm10, %zmm30{%k4}{z} */
    "6281854859d9",  /* vmulpd %zmm25, %zmm15, %zmm19 */
    "6211c54e59d1",  /* vmulpd %zmm25, %zmm7, %zmm10{%k6} */
    "6251ddcd59ea",  /* vmulpd %zmm10, %zmm4, %zmm13{%k5}{z} */
    "62c11d40feee",  /* vpaddd %zmm14, %zmm28, %zmm21 */
    "6251554bfedf",  /* vpaddd %zmm15, %zmm5, %zmm11{%k3} */
    "62715dc3fefd",  /* vpaddd %zmm5, %zmm20, %zmm15{%k3}{z} */
    "6241f548d4d4",  /* vpaddq %zmm12, %zmm1, %zmm26 */
    "6281f547d4e8",  /* vpaddq %zmm24, %zmm17, %zmm21{%k7} */
    "62e185c5d4cb",  /* vpaddq %zmm3, %zmm31, %zmm17{%k5}{z} */
    "62f11548efaf78563412",  /* vpxord 305419896(%rdi), %zmm13, %zmm5 */
    "62011d43efd9",  /* vpxord %zmm25, %zmm28, %zmm27{%k3} */
    "62f13dccefd1",  /* vpxord %zmm1, %zmm8, %zmm2{%k4}{z} */
    "6261dd48dbc8",  /* vpandq %zmm0, %zmm4, %zmm25 */
    "62718d4adbce",  /* vpandq %zmm6, %zmm14, %zmm9{%k2} */
    "62f195c5dbee",  /* vpandq %zmm6, %zmm29, %zmm5{%k5}{z} */
    "62d20d483dd0",  /* vpmaxsd %zmm8, %zmm14, %zmm2 */
    "629245493df3",  /* vpmaxsd %zmm27, %zmm7, %zmm6{%k1} */
    "62221dcb3dc3",  /* vpmaxsd %zmm19, %zmm12, %zmm24{%k3}{z} */
    "62f37d481fe803",  /* vpcmpd $3, %zmm0, %zmm0, %k5 */
    "629375401edd04",  /* vpcmpud $4, %zmm29, %zmm17, %k3 */
    "629385481fee03",  /* vpcmpq $3, %zmm30, %zmm15, %k5 */
    "62d17ecc6fcf",  /* vmovdqu32 %zmm15, %zmm1{%k4}{z} */
    "d845f8",  /* fadds -8(%rbp) */
    "41dc07",  /* faddl (%r15) */
    "d8e2",  /* fsub %st(2), %st */
    "d822",  /* fsubs (%rdx) */
    "42dc2432",  /* fsubl (%rdx,%r14,1) */
    "d8cf",  /* fmul %st(7), %st */
    "43d80c40",  /* fmuls (%r8,%r8,2) */
    "dc4d00",  /* fmull (%rbp) */
    "dcf7",  /* fdiv %st, %st(7) */
    "42d8b4c0a1000000",  /* fdivs 161(%rax,%r8,8) */
    "d8e9",  /* fsubr %st(1), %st */
    "42d82ce3",  /* fsubrs (%rbx,%r12,8) */
    "dc2b",  /* fsubrl (%rbx) */
    "41d83f",  /* fdivrs (%r15) */
    "41dc38",  /* fdivrl (%r8) */
    "d9c3",  /* fld %st(3) */
    "ddd2",  /* fst %st(2) */
    "41d947f8",  /* flds -8(%r15) */
    "43dd04fc",  /* fldl (%r12,%r15,8) */
    "d916",  /* fsts (%rsi) */
    "dd144f",  /* fstl (%rdi,%rcx,2) */
    "41d91c24",  /* fstps (%r12) */
    "43dd5cf16d",  /* fstpl 109(%r9,%r14,8) */
    "41df01",  /* filds (%r9) */
    "db44ba99",  /* fildl -103(%rdx,%rdi,4) */
    "41df19",  /* fistp (%r9) */
    "41df29",  /* fildll (%r9) */
    "0ffcdd",  /* paddb %mm5, %mm3 */
    "0ffde6",  /* paddw %mm6, %mm4 */
    "0ffed1",  /* paddd %mm1, %mm2 */
    "0fd4d3",  /* paddq %mm3, %mm2 */
    "0ff8f0",  /* psubb %mm0, %mm6 */
    "0fdbde",  /* pand %mm6, %mm3 */
    "0febf6",  /* por %mm6, %mm6 */
    "0fefe1",  /* pxor %mm1, %mm4 */
    "0f74c8",  /* pcmpeqb %mm0, %mm1 */
    "410fd52e",  /* pmullw (%r14), %mm5 */
    "0f71f504",  /* psllw $4, %mm5 */
    "0f72f10c",  /* pslld $12, %mm1 */
    "0f73d509",  /* psrlq $9, %mm5 */
    "0f71e50a",  /* psraw $10, %mm5 */
    "0f6fd0",  /* movq %mm0, %mm2 */
    "0f6ee9",  /* movd %ecx, %mm5 */
    "0f70d9dc",  /* pshufw $220, %mm1, %mm3 */
    "c56058f3",  /* vaddps %xmm3, %xmm3, %xmm14 */
    "c441185cdf",  /* vsubps %xmm15, %xmm12, %xmm11 */
    "c57459f6",  /* vmulps %ymm6, %ymm1, %ymm14 */
    "c5785ed8",  /* vdivps %xmm0, %xmm0, %xmm11 */
    "c4c14c5d39",  /* vminps (%r9), %ymm6, %ymm7 */
    "c5345fc8",  /* vmaxps %ymm0, %ymm9, %ymm9 */
    "c51158da",  /* vaddpd %xmm2, %xmm13, %xmm11 */
    "c50559de",  /* vmulpd %ymm6, %ymm15, %ymm11 */
    "c441255cdc",  /* vsubpd %ymm12, %ymm11, %ymm11 */
    "c4c15454d0",  /* vandps %ymm8, %ymm5, %ymm2 */
    "c4413d56c1",  /* vorpd %ymm9, %ymm8, %ymm8 */
    "c54457f7",  /* vxorps %ymm7, %ymm7, %ymm14 */
    "c548141a",  /* vunpcklps (%rdx), %xmm6, %xmm11 */
    "c4c16d15ec",  /* vunpckhpd %ymm12, %ymm2, %ymm5 */
    "c54a58c0",  /* vaddss %xmm0, %xmm6, %xmm8 */
    "c4410b59ce",  /* vmulsd %xmm14, %xmm14, %xmm9 */
    "c5a25cd4",  /* vsubss %xmm4, %xmm11, %xmm2 */
    "c55b5eda",  /* vdivsd %xmm2, %xmm4, %xmm11 */
    "c5aa5df9",  /* vminss %xmm1, %xmm10, %xmm7 */
    "c4c1135feb",  /* vmaxsd %xmm11, %xmm13, %xmm5 */
    "c5e251fa",  /* vsqrtss %xmm2, %xmm3, %xmm7 */
    "c5f851de",  /* vsqrtps %xmm6, %xmm3 */
    "c5f951ed",  /* vsqrtpd %xmm5, %xmm5 */
    "c4c164c2d712",  /* vcmpps $18, %ymm15, %ymm3, %ymm2 */
    "c559c2f508",  /* vcmppd $8, %xmm5, %xmm4, %xmm14 */
    "c44112c2dd00",  /* vcmpss $0, %xmm13, %xmm13, %xmm11 */
    "c534c6c5bb",  /* vshufps $187, %ymm5, %ymm9, %ymm8 */
    "c59dc6e641",  /* vshufpd $65, %ymm6, %ymm12, %ymm4 */
    "c443350cdbf3",  /* vblendps $243, %ymm11, %ymm9, %ymm11 */
    "c4e36d0de80e",  /* vblendpd $14, %ymm0, %ymm2, %ymm5 */
    "c4e3250ecb41",  /* vpblendw $65, %ymm3, %ymm11, %ymm1 */
    "c4630540dfb7",  /* vdpps $183, %ymm7, %ymm15, %ymm11 */
    "c5adfccc",  /* vpaddb %ymm4, %ymm10, %ymm1 */
    "c44101fedb",  /* vpaddd %xmm11, %xmm15, %xmm11 */
    "c5bdd4c4",  /* vpaddq %ymm4, %ymm8, %ymm0 */
    "c4411df9f9",  /* vpsubw %ymm9, %ymm12, %ymm15 */
    "c4410ddbd7",  /* vpand %ymm15, %ymm14, %ymm10 */
    "c44109ebdb",  /* vpor %xmm11, %xmm14, %xmm11 */
    "c5f5efd4",  /* vpxor %ymm4, %ymm1, %ymm2 */
    "c4c13d74f4",  /* vpcmpeqb %ymm12, %ymm8, %ymm6 */
    "c4414966d6",  /* vpcmpgtd %xmm14, %xmm6, %xmm10 */
    "c4625d3dc8",  /* vpmaxsd %ymm0, %ymm4, %ymm9 */
    "c4622d3bd6",  /* vpminud %ymm6, %ymm10, %ymm10 */
    "c4624940fb",  /* vpmulld %xmm3, %xmm6, %xmm15 */
    "c55d60da",  /* vpunpcklbw %ymm2, %ymm4, %ymm11 */
    "c4c115f5f0",  /* vpmaddwd %ymm8, %ymm13, %ymm6 */
    "c4c22900ef",  /* vpshufb %xmm15, %xmm10, %xmm5 */
    "c57970e003",  /* vpshufd $3, %xmm0, %xmm12 */
    "c4c15972f70d",  /* vpslld $13, %xmm15, %xmm4 */
    "c4c12573d50f",  /* vpsrlq $15, %ymm13, %ymm11 */
    "c57c29dc",  /* vmovaps %ymm11, %ymm4 */
    "c4c17c2928",  /* vmovaps %ymm5, (%r8) */
    "c57810ef",  /* vmovups %xmm7, %xmm13 */
    "c5f811b700010000",  /* vmovups %xmm6, 256(%rdi) */
    "c57d28e7",  /* vmovapd %ymm7, %ymm12 */
    "c5fd2904bf",  /* vmovapd %ymm0, (%rdi,%rdi,4) */
    "c5797fe8",  /* vmovdqa %xmm13, %xmm0 */
    "c5797f3f",  /* vmovdqa %xmm15, (%rdi) */
    "c57a6f10",  /* vmovdqu (%rax), %xmm10 */
    "c4017a7f1c27",  /* vmovdqu %xmm11, (%r15,%r12,1) */
    "c4c1796eef",  /* vmovd %r15d, %xmm5 */
    "c4e1f96ecb",  /* vmovq %rbx, %xmm1 */
    "c4817a10b40853ffffff",  /* vmovss -173(%r8,%r9,1), %xmm6 */
    "c5eb10cf",  /* vmovsd %xmm7, %xmm2, %xmm1 */
    "c4425598ff",  /* vfmadd132ps %ymm15, %ymm5, %ymm15 */
    "c4c28da8ff",  /* vfmadd213pd %ymm15, %ymm14, %ymm7 */
    "c44209bafc",  /* vfmsub231ps %xmm12, %xmm14, %xmm15 */
    "c46251ac13",  /* vfnmadd213ps (%rbx), %xmm5, %xmm10 */
    "c44251a9ce",  /* vfmadd213ss %xmm14, %xmm5, %xmm9 */
    "c4c2b9bfd4",  /* vfnmsub231sd %xmm12, %xmm8, %xmm2 */
    "c4427d13f7",  /* vcvtph2ps %xmm15, %ymm14 */
    "c4637d1dca00",  /* vcvtps2ph $0, %ymm9, %xmm2 */
    "c4c17c5bd1",  /* vcvtdq2ps %ymm9, %ymm2 */
    "c5fa5bed",  /* vcvttps2dq %xmm5, %xmm5 */
    "c441832ad7",  /* vcvtsi2sd %r15, %xmm15, %xmm10 */
    "c4627d18f6",  /* vbroadcastss %xmm6, %ymm14 */
    "c4627d58e7",  /* vpbroadcastd %xmm7, %ymm12 */
    "c4e3fd00d90f",  /* vpermq $15, %ymm1, %ymm3 */
    "c4433d06d3e3",  /* vperm2f128 $227, %ymm11, %ymm8, %ymm10 */
    "c4e36518f301",  /* vinsertf128 $1, %xmm3, %ymm3, %ymm6 */
    "c4437d39fe01",  /* vextracti128 $1, %ymm15, %xmm14 */
    "c4e35920c303",  /* vpinsrb $3, %ebx, %xmm4, %xmm0 */
    "c4637916d902",  /* vpextrd $2, %xmm11, %ecx */
    "62613c4058f7",  /* vaddps %zmm7, %zmm24, %zmm30 */
    "6241044a58e3",  /* vaddps %zmm11, %zmm15, %zmm28{%k2} */
    "623134cd58e9",  /* vaddps %zmm17, %zmm9, %zmm13{%k5}{z} */
    "6261dd4059945875000000",  /* vmulpd 117(%rax,%rbx,2), %zmm20, %zmm26 */
    "6271cd4a59c8",  /* vmulpd %zmm0, %zmm6, %zmm9{%k2} */
    "6271adcc59cc",  /* vmulpd %zmm4, %zmm10, %zmm9{%k4}{z} */
    "62015548fee0",  /* vpaddd %zmm24, %zmm5, %zmm28 */
    "62812544feef",  /* vpaddd %zmm31, %zmm27, %zmm21{%k4} */
    "62d165c5fed6",  /* vpaddd %zmm14, %zmm19, %zmm2{%k5}{z} */
    "6261c540d4d9",  /* vpaddq %zmm1, %zmm23, %zmm27 */
    "62f18545d4dc",  /* vpaddq %zmm4, %zmm31, %zmm3{%k5} */
    "62f1e5cbd4f1",  /* vpaddq %zmm1, %zmm3, %zmm6{%k3}{z} */
    "62b14d40efee",  /* vpxord %zmm22, %zmm22, %zmm5 */
    "62d17546eff7",  /* vpxord %zmm15, %zmm17, %zmm6{%k6} */
    "621175c9efe2",  /* vpxord %zmm26, %zmm1, %zmm12{%k1}{z} */
    "6271f540db347b",  /* vpandq (%rbx,%rdi,2), %zmm17, %zmm14 */
    "62c1dd42dbda",  /* vpandq %zmm10, %zmm20, %zmm19{%k2} */
    "62f1f5c5dbc9",  /* vpandq %zmm1, %zmm17, %zmm1{%k5}{z} */
    "62a235483dda",  /* vpmaxsd %zmm18, %zmm9, %zmm19 */
    "6252754e3dc5",  /* vpmaxsd %zmm13, %zmm1, %zmm8{%k6} */
    "62e23dcb3df2",  /* vpmaxsd %zmm2, %zmm8, %zmm22{%k3}{z} */
    "62b34d401ff007",  /* vpcmpd $7, %zmm16, %zmm22, %k6 */
    "62d345401ee004",  /* vpcmpud $4, %zmm8, %zmm23, %k4 */
    "62d3dd401ffe07",  /* vpcmpq $7, %zmm14, %zmm20, %k7 */
    "62e17ecd6fc5",  /* vmovdqu32 %zmm5, %zmm16{%k5}{z} */
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
