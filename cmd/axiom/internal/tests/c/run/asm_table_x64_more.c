/* flags: --c-std=gnu17 */
/* native-only */
/* The x86-64 table forms of cmd/internal/tools/gen_x64_table_more.py (the rest of AVX-512 with FP16 and BF16,
   AVX-VNNI-INT8/INT16, AVX-IFMA, AVX-NE-CONVERT, SHA512, SM3, SM4, CMPccXADD, FMA4 ...), two shapes of each
   mnemonic, against a byte table checked against the system assembler in development. */
#include "host.h"

#ifdef __AXIOM_ASM_X86_64__
__asm__(".section .rodata.axiom_asm_table, \"a\"\n"
        ".globl axiom_asm_table\n"
        "axiom_asm_table:\n"
        "cmpbexadd %eax,%ecx,0x1(%rax)\n"
        "cmpbexadd %ebx,%ecx,0x1(%rax)\n"
        "cmpbxadd %eax,%ecx,0x1(%rax)\n"
        "cmpbxadd %ebx,%ecx,0x1(%rax)\n"
        "cmplexadd %eax,%ecx,0x1(%rax)\n"
        "cmplexadd %ebx,%ecx,0x1(%rax)\n"
        "cmplxadd %eax,%ecx,0x1(%rax)\n"
        "cmplxadd %ebx,%ecx,0x1(%rax)\n"
        "cmpnbexadd %eax,%ecx,0x1(%rax)\n"
        "cmpnbexadd %ebx,%ecx,0x1(%rax)\n"
        "cmpnbxadd %eax,%ecx,0x1(%rax)\n"
        "cmpnbxadd %ebx,%ecx,0x1(%rax)\n"
        "cmpnlexadd %eax,%ecx,0x1(%rax)\n"
        "cmpnlexadd %ebx,%ecx,0x1(%rax)\n"
        "cmpnlxadd %eax,%ecx,0x1(%rax)\n"
        "cmpnlxadd %ebx,%ecx,0x1(%rax)\n"
        "cmpnoxadd %eax,%ecx,0x1(%rax)\n"
        "cmpnoxadd %ebx,%ecx,0x1(%rax)\n"
        "cmpnpxadd %eax,%ecx,0x1(%rax)\n"
        "cmpnpxadd %ebx,%ecx,0x1(%rax)\n"
        "cmpnsxadd %eax,%ecx,0x1(%rax)\n"
        "cmpnsxadd %ebx,%ecx,0x1(%rax)\n"
        "cmpnzxadd %eax,%ecx,0x1(%rax)\n"
        "cmpnzxadd %ebx,%ecx,0x1(%rax)\n"
        "cmpoxadd %eax,%ecx,0x1(%rax)\n"
        "cmpoxadd %ebx,%ecx,0x1(%rax)\n"
        "cmppxadd %eax,%ecx,0x1(%rax)\n"
        "cmppxadd %ebx,%ecx,0x1(%rax)\n"
        "cmpsxadd %eax,%ecx,0x1(%rax)\n"
        "cmpsxadd %ebx,%ecx,0x1(%rax)\n"
        "cmpzxadd %eax,%ecx,0x1(%rax)\n"
        "cmpzxadd %ebx,%ecx,0x1(%rax)\n"
        "kmovb %edx,%k1\n"
        "kmovb %k1,0x1(%rax)\n"
        "kmovd %edx,%k1\n"
        "kmovd %k1,0x1(%rax)\n"
        "kmovq %k1,0x1(%rax)\n"
        "kmovq %k2,%k1\n"
        "kmovw %edx,%k1\n"
        "kmovw %k1,0x1(%rax)\n"
        "kshiftlb $0x47,%k2,%k1\n"
        "kshiftld $0x47,%k2,%k1\n"
        "kshiftlq $0x47,%k2,%k1\n"
        "kshiftlw $0x47,%k2,%k1\n"
        "kshiftrb $0x47,%k2,%k1\n"
        "kshiftrd $0x47,%k2,%k1\n"
        "kshiftrq $0x47,%k2,%k1\n"
        "kshiftrw $0x47,%k2,%k1\n"
        "v4fmaddps 0x10(%rax),%zmm0,%zmm1\n"
        "v4fmaddps 0x10(%rax),%zmm3,%zmm1\n"
        "v4fmaddss 0x10(%rax),%xmm0,%xmm1\n"
        "v4fmaddss 0x10(%rax),%xmm3,%xmm1\n"
        "v4fnmaddps 0x10(%rax),%zmm0,%zmm1\n"
        "v4fnmaddps 0x10(%rax),%zmm3,%zmm1\n"
        "v4fnmaddss 0x10(%rax),%xmm0,%xmm1\n"
        "v4fnmaddss 0x10(%rax),%xmm3,%xmm1\n"
        "vaddph %xmm2,%xmm0,%xmm1\n"
        "vaddph %xmm2,%xmm3,%xmm1\n"
        "vaddsh %xmm2,%xmm0,%xmm1\n"
        "vaddsh %xmm2,%xmm3,%xmm1\n"
        "valignd $0x47,%xmm2,%xmm0,%xmm1\n"
        "valignd $0x47,%xmm2,%xmm3,%xmm1\n"
        "valignq $0x47,%xmm2,%xmm0,%xmm1\n"
        "valignq $0x47,%xmm2,%xmm3,%xmm1\n"
        "vbcstnebf162ps 0x1(%rax),%xmm1\n"
        "vbcstnebf162ps 0x1(%rax),%ymm1\n"
        "vbcstnesh2ps 0x1(%rax),%xmm1\n"
        "vbcstnesh2ps 0x1(%rax),%ymm1\n"
        "vblendmpd %xmm2,%xmm0,%xmm1\n"
        "vblendmpd %xmm2,%xmm3,%xmm1\n"
        "vblendmps %xmm2,%xmm0,%xmm1\n"
        "vblendmps %xmm2,%xmm3,%xmm1\n"
        "vbroadcastf128 0x1(%rax),%ymm1\n"
        "vbroadcastf32x2 %xmm2,%ymm1\n"
        "vbroadcastf32x2 %xmm2,%zmm1\n"
        "vbroadcastf32x4 0x10(%rax),%ymm1\n"
        "vbroadcastf32x4 0x10(%rax),%zmm1\n"
        "vbroadcastf32x8 0x20(%rax),%zmm1\n"
        "vbroadcastf64x2 0x10(%rax),%ymm1\n"
        "vbroadcastf64x2 0x10(%rax),%zmm1\n"
        "vbroadcastf64x4 0x20(%rax),%zmm1\n"
        "vbroadcasti128 0x1(%rax),%ymm1\n"
        "vbroadcasti32x2 %xmm2,%xmm1\n"
        "vbroadcasti32x2 %xmm2,%ymm1\n"
        "vbroadcasti32x4 0x10(%rax),%ymm1\n"
        "vbroadcasti32x4 0x10(%rax),%zmm1\n"
        "vbroadcasti32x8 0x20(%rax),%zmm1\n"
        "vbroadcasti64x2 0x10(%rax),%ymm1\n"
        "vbroadcasti64x2 0x10(%rax),%zmm1\n"
        "vbroadcasti64x4 0x20(%rax),%zmm1\n"
        "vcmpph $0x47,%xmm2,%xmm0,%k1\n"
        "vcmpph $0x47,%xmm2,%xmm3,%k1\n"
        "vcmpsh $0x47,%xmm2,%xmm0,%k1\n"
        "vcmpsh $0x47,%xmm2,%xmm3,%k1\n"
        "vcomish %xmm2,%xmm1\n"
        "vcomish 0x2(%rax),%xmm1\n"
        "vcvtdq2pd %xmm2,%xmm1\n"
        "vcvtdq2pd %xmm2,%ymm1\n"
        "vcvtdq2ph %xmm2,%xmm1\n"
        "vcvtdq2ph %ymm2,%xmm1\n"
        "vcvtdq2phx 0x10(%rax),%xmm1\n"
        "vcvtdq2phy 0x20(%rax),%xmm1\n"
        "vcvtne2ps2bf16 %xmm2,%xmm0,%xmm1\n"
        "vcvtne2ps2bf16 %xmm2,%xmm3,%xmm1\n"
        "vcvtneebf162ps 0x1(%rax),%xmm1\n"
        "vcvtneebf162ps 0x1(%rax),%ymm1\n"
        "vcvtneeph2ps 0x1(%rax),%xmm1\n"
        "vcvtneeph2ps 0x1(%rax),%ymm1\n"
        "vcvtneobf162ps 0x1(%rax),%xmm1\n"
        "vcvtneobf162ps 0x1(%rax),%ymm1\n"
        "vcvtneoph2ps 0x1(%rax),%xmm1\n"
        "vcvtneoph2ps 0x1(%rax),%ymm1\n"
        "vcvtneps2bf16 %xmm2,%xmm1\n"
        "vcvtneps2bf16 %ymm2,%xmm1\n"
        "vcvtneps2bf16x 0x10(%rax),%xmm1\n"
        "vcvtneps2bf16y 0x20(%rax),%xmm1\n"
        "vcvtpd2dq %xmm2,%xmm1\n"
        "vcvtpd2dq %ymm2,%xmm1\n"
        "vcvtpd2dqx 0x1(%rax),%xmm1\n"
        "vcvtpd2dqy 0x1(%rax),%xmm1\n"
        "vcvtpd2ph %xmm2,%xmm1\n"
        "vcvtpd2ph %ymm2,%xmm1\n"
        "vcvtpd2phx 0x10(%rax),%xmm1\n"
        "vcvtpd2phy 0x20(%rax),%xmm1\n"
        "vcvtpd2phz 0x40(%rax),%xmm1\n"
        "vcvtpd2qq %xmm2,%xmm1\n"
        "vcvtpd2qq %ymm2,%ymm1\n"
        "vcvtpd2udq %xmm2,%xmm1\n"
        "vcvtpd2udq %ymm2,%xmm1\n"
        "vcvtpd2udqx 0x10(%rax),%xmm1\n"
        "vcvtpd2udqy 0x20(%rax),%xmm1\n"
        "vcvtpd2uqq %xmm2,%xmm1\n"
        "vcvtpd2uqq %ymm2,%ymm1\n"
        "vcvtph2dq %xmm2,%xmm1\n"
        "vcvtph2dq %xmm2,%ymm1\n"
        "vcvtph2pd %xmm2,%xmm1\n"
        "vcvtph2pd %xmm2,%ymm1\n"
        "vcvtph2psx %xmm2,%xmm1\n"
        "vcvtph2psx %xmm2,%ymm1\n"
        "vcvtph2qq %xmm2,%xmm1\n"
        "vcvtph2qq %xmm2,%ymm1\n"
        "vcvtph2udq %xmm2,%xmm1\n"
        "vcvtph2udq %xmm2,%ymm1\n"
        "vcvtph2uqq %xmm2,%xmm1\n"
        "vcvtph2uqq %xmm2,%ymm1\n"
        "vcvtph2uw %xmm2,%xmm1\n"
        "vcvtph2uw %ymm2,%ymm1\n"
        "vcvtph2w %xmm2,%xmm1\n"
        "vcvtph2w %ymm2,%ymm1\n"
        "vcvtps2phx %xmm2,%xmm1\n"
        "vcvtps2phx %ymm2,%xmm1\n"
        "vcvtps2phxx 0x10(%rax),%xmm1\n"
        "vcvtps2phxy 0x20(%rax),%xmm1\n"
        "vcvtps2qq %xmm2,%xmm1\n"
        "vcvtps2qq %xmm2,%ymm1\n"
        "vcvtps2udq %xmm2,%xmm1\n"
        "vcvtps2udq %ymm2,%ymm1\n"
        "vcvtps2uqq %xmm2,%xmm1\n"
        "vcvtps2uqq %xmm2,%ymm1\n"
        "vcvtqq2pd %xmm2,%xmm1\n"
        "vcvtqq2pd %ymm2,%ymm1\n"
        "vcvtqq2ph %xmm2,%xmm1\n"
        "vcvtqq2ph %ymm2,%xmm1\n"
        "vcvtqq2phx 0x10(%rax),%xmm1\n"
        "vcvtqq2phy 0x20(%rax),%xmm1\n"
        "vcvtqq2phz 0x40(%rax),%xmm1\n"
        "vcvtqq2ps %xmm2,%xmm1\n"
        "vcvtqq2ps %ymm2,%xmm1\n"
        "vcvtqq2psx 0x10(%rax),%xmm1\n"
        "vcvtqq2psy 0x20(%rax),%xmm1\n"
        "vcvtsd2sh %xmm2,%xmm0,%xmm1\n"
        "vcvtsd2sh %xmm2,%xmm3,%xmm1\n"
        "vcvtsd2si %xmm2,%ecx\n"
        "vcvtsd2si %xmm2,%rcx\n"
        "vcvtsd2usi %xmm2,%ecx\n"
        "vcvtsd2usi %xmm2,%rcx\n"
        "vcvtsh2sd %xmm2,%xmm0,%xmm1\n"
        "vcvtsh2sd %xmm2,%xmm3,%xmm1\n"
        "vcvtsh2si %xmm2,%ecx\n"
        "vcvtsh2si %xmm2,%rcx\n"
        "vcvtsh2ss %xmm2,%xmm0,%xmm1\n"
        "vcvtsh2ss %xmm2,%xmm3,%xmm1\n"
        "vcvtsh2usi %xmm2,%ecx\n"
        "vcvtsh2usi %xmm2,%rcx\n"
        "vcvtsi2sd %edx,%xmm0,%xmm1\n"
        "vcvtsi2sd %edx,%xmm3,%xmm1\n"
        "vcvtsi2sdl 0x1(%rax),%xmm0,%xmm1\n"
        "vcvtsi2sdl 0x1(%rax),%xmm3,%xmm1\n"
        "vcvtsi2sdq 0x1(%rax),%xmm0,%xmm1\n"
        "vcvtsi2sdq 0x1(%rax),%xmm3,%xmm1\n"
        "vcvtsi2sh %edx,%xmm0,%xmm1\n"
        "vcvtsi2sh %edx,%xmm3,%xmm1\n"
        "vcvtsi2shl 0x4(%rax),%xmm0,%xmm1\n"
        "vcvtsi2shl 0x4(%rax),%xmm3,%xmm1\n"
        "vcvtsi2shq 0x8(%rax),%xmm0,%xmm1\n"
        "vcvtsi2shq 0x8(%rax),%xmm3,%xmm1\n"
        "vcvtsi2ss %edx,%xmm0,%xmm1\n"
        "vcvtsi2ss %edx,%xmm3,%xmm1\n"
        "vcvtsi2ssl 0x1(%rax),%xmm0,%xmm1\n"
        "vcvtsi2ssl 0x1(%rax),%xmm3,%xmm1\n"
        "vcvtsi2ssq 0x1(%rax),%xmm0,%xmm1\n"
        "vcvtsi2ssq 0x1(%rax),%xmm3,%xmm1\n"
        "vcvtss2sh %xmm2,%xmm0,%xmm1\n"
        "vcvtss2sh %xmm2,%xmm3,%xmm1\n"
        "vcvtss2si %xmm2,%ecx\n"
        "vcvtss2si %xmm2,%rcx\n"
        "vcvtss2usi %xmm2,%ecx\n"
        "vcvtss2usi %xmm2,%rcx\n"
        "vcvttpd2dq %xmm2,%xmm1\n"
        "vcvttpd2dq %ymm2,%xmm1\n"
        "vcvttpd2dqx 0x1(%rax),%xmm1\n"
        "vcvttpd2dqy 0x1(%rax),%xmm1\n"
        "vcvttpd2qq %xmm2,%xmm1\n"
        "vcvttpd2qq %ymm2,%ymm1\n"
        "vcvttpd2udq %xmm2,%xmm1\n"
        "vcvttpd2udq %ymm2,%xmm1\n"
        "vcvttpd2udqx 0x10(%rax),%xmm1\n"
        "vcvttpd2udqy 0x20(%rax),%xmm1\n"
        "vcvttpd2uqq %xmm2,%xmm1\n"
        "vcvttpd2uqq %ymm2,%ymm1\n"
        "vcvttph2dq %xmm2,%xmm1\n"
        "vcvttph2dq %xmm2,%ymm1\n"
        "vcvttph2qq %xmm2,%xmm1\n"
        "vcvttph2qq %xmm2,%ymm1\n"
        "vcvttph2udq %xmm2,%xmm1\n"
        "vcvttph2udq %xmm2,%ymm1\n"
        "vcvttph2uqq %xmm2,%xmm1\n"
        "vcvttph2uqq %xmm2,%ymm1\n"
        "vcvttph2uw %xmm2,%xmm1\n"
        "vcvttph2uw %ymm2,%ymm1\n"
        "vcvttph2w %xmm2,%xmm1\n"
        "vcvttph2w %ymm2,%ymm1\n"
        "vcvttps2qq %xmm2,%xmm1\n"
        "vcvttps2qq %xmm2,%ymm1\n"
        "vcvttps2udq %xmm2,%xmm1\n"
        "vcvttps2udq %ymm2,%ymm1\n"
        "vcvttps2uqq %xmm2,%xmm1\n"
        "vcvttps2uqq %xmm2,%ymm1\n"
        "vcvttsd2si %xmm2,%ecx\n"
        "vcvttsd2si %xmm2,%rcx\n"
        "vcvttsd2usi %xmm2,%ecx\n"
        "vcvttsd2usi %xmm2,%rcx\n"
        "vcvttsh2si %xmm2,%ecx\n"
        "vcvttsh2si %xmm2,%rcx\n"
        "vcvttsh2usi %xmm2,%ecx\n"
        "vcvttsh2usi %xmm2,%rcx\n"
        "vcvttss2si %xmm2,%ecx\n"
        "vcvttss2si %xmm2,%rcx\n"
        "vcvttss2usi %xmm2,%ecx\n"
        "vcvttss2usi %xmm2,%rcx\n"
        "vcvtudq2pd %xmm2,%xmm1\n"
        "vcvtudq2pd %xmm2,%ymm1\n"
        "vcvtudq2ph %xmm2,%xmm1\n"
        "vcvtudq2ph %ymm2,%xmm1\n"
        "vcvtudq2phx 0x10(%rax),%xmm1\n"
        "vcvtudq2phy 0x20(%rax),%xmm1\n"
        "vcvtudq2ps %xmm2,%xmm1\n"
        "vcvtudq2ps %ymm2,%ymm1\n"
        "vcvtuqq2pd %xmm2,%xmm1\n"
        "vcvtuqq2pd %ymm2,%ymm1\n"
        "vcvtuqq2ph %xmm2,%xmm1\n"
        "vcvtuqq2ph %ymm2,%xmm1\n"
        "vcvtuqq2phx 0x10(%rax),%xmm1\n"
        "vcvtuqq2phy 0x20(%rax),%xmm1\n"
        "vcvtuqq2phz 0x40(%rax),%xmm1\n"
        "vcvtuqq2ps %xmm2,%xmm1\n"
        "vcvtuqq2ps %ymm2,%xmm1\n"
        "vcvtuqq2psx 0x10(%rax),%xmm1\n"
        "vcvtuqq2psy 0x20(%rax),%xmm1\n"
        "vcvtusi2sd %edx,%xmm0,%xmm1\n"
        "vcvtusi2sd %edx,%xmm3,%xmm1\n"
        "vcvtusi2sdl 0x4(%rax),%xmm0,%xmm1\n"
        "vcvtusi2sdl 0x4(%rax),%xmm3,%xmm1\n"
        "vcvtusi2sdq 0x8(%rax),%xmm0,%xmm1\n"
        "vcvtusi2sdq 0x8(%rax),%xmm3,%xmm1\n"
        "vcvtusi2sh %edx,%xmm0,%xmm1\n"
        "vcvtusi2sh %edx,%xmm3,%xmm1\n"
        "vcvtusi2shl 0x4(%rax),%xmm0,%xmm1\n"
        "vcvtusi2shl 0x4(%rax),%xmm3,%xmm1\n"
        "vcvtusi2shq 0x8(%rax),%xmm0,%xmm1\n"
        "vcvtusi2shq 0x8(%rax),%xmm3,%xmm1\n"
        "vcvtusi2ss %edx,%xmm0,%xmm1\n"
        "vcvtusi2ss %edx,%xmm3,%xmm1\n"
        "vcvtusi2ssl 0x4(%rax),%xmm0,%xmm1\n"
        "vcvtusi2ssl 0x4(%rax),%xmm3,%xmm1\n"
        "vcvtusi2ssq 0x8(%rax),%xmm0,%xmm1\n"
        "vcvtusi2ssq 0x8(%rax),%xmm3,%xmm1\n"
        "vcvtuw2ph %xmm2,%xmm1\n"
        "vcvtuw2ph %ymm2,%ymm1\n"
        "vcvtw2ph %xmm2,%xmm1\n"
        "vcvtw2ph %ymm2,%ymm1\n"
        "vdbpsadbw $0x47,%xmm2,%xmm0,%xmm1\n"
        "vdbpsadbw $0x47,%xmm2,%xmm3,%xmm1\n"
        "vdivph %xmm2,%xmm0,%xmm1\n"
        "vdivph %xmm2,%xmm3,%xmm1\n"
        "vdivsh %xmm2,%xmm0,%xmm1\n"
        "vdivsh %xmm2,%xmm3,%xmm1\n"
        "vdpbf16ps %xmm2,%xmm0,%xmm1\n"
        "vdpbf16ps %xmm2,%xmm3,%xmm1\n"
        "vexp2pd %zmm2,%zmm1\n"
        "vexp2pd 0x40(%rax),%zmm1\n"
        "vexp2ps %zmm2,%zmm1\n"
        "vexp2ps 0x40(%rax),%zmm1\n"
        "vfcmaddcph %xmm2,%xmm0,%xmm1\n"
        "vfcmaddcph %xmm2,%xmm3,%xmm1\n"
        "vfcmaddcsh %xmm2,%xmm0,%xmm1\n"
        "vfcmaddcsh %xmm2,%xmm3,%xmm1\n"
        "vfcmulcph %xmm2,%xmm0,%xmm1\n"
        "vfcmulcph %xmm2,%xmm3,%xmm1\n"
        "vfcmulcsh %xmm2,%xmm0,%xmm1\n"
        "vfcmulcsh %xmm2,%xmm3,%xmm1\n"
        "vfixupimmpd $0x47,%xmm2,%xmm0,%xmm1\n"
        "vfixupimmpd $0x47,%xmm2,%xmm3,%xmm1\n"
        "vfixupimmps $0x47,%xmm2,%xmm0,%xmm1\n"
        "vfixupimmps $0x47,%xmm2,%xmm3,%xmm1\n"
        "vfixupimmsd $0x47,%xmm2,%xmm0,%xmm1\n"
        "vfixupimmsd $0x47,%xmm2,%xmm3,%xmm1\n"
        "vfixupimmss $0x47,%xmm2,%xmm0,%xmm1\n"
        "vfixupimmss $0x47,%xmm2,%xmm3,%xmm1\n"
        "vfmadd132ph %xmm2,%xmm0,%xmm1\n"
        "vfmadd132ph %xmm2,%xmm3,%xmm1\n"
        "vfmadd132sh %xmm2,%xmm0,%xmm1\n"
        "vfmadd132sh %xmm2,%xmm3,%xmm1\n"
        "vfmadd213ph %xmm2,%xmm0,%xmm1\n"
        "vfmadd213ph %xmm2,%xmm3,%xmm1\n"
        "vfmadd213sh %xmm2,%xmm0,%xmm1\n"
        "vfmadd213sh %xmm2,%xmm3,%xmm1\n"
        "vfmadd231ph %xmm2,%xmm0,%xmm1\n"
        "vfmadd231ph %xmm2,%xmm3,%xmm1\n"
        "vfmadd231sh %xmm2,%xmm0,%xmm1\n"
        "vfmadd231sh %xmm2,%xmm3,%xmm1\n"
        "vfmaddcph %xmm2,%xmm0,%xmm1\n"
        "vfmaddcph %xmm2,%xmm3,%xmm1\n"
        "vfmaddcsh %xmm2,%xmm0,%xmm1\n"
        "vfmaddcsh %xmm2,%xmm3,%xmm1\n"
        "vfmaddpd %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfmaddpd %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfmaddps %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfmaddps %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfmaddsd %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfmaddsd %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfmaddss %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfmaddss %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfmaddsub132ph %xmm2,%xmm0,%xmm1\n"
        "vfmaddsub132ph %xmm2,%xmm3,%xmm1\n"
        "vfmaddsub213ph %xmm2,%xmm0,%xmm1\n"
        "vfmaddsub213ph %xmm2,%xmm3,%xmm1\n"
        "vfmaddsub231ph %xmm2,%xmm0,%xmm1\n"
        "vfmaddsub231ph %xmm2,%xmm3,%xmm1\n"
        "vfmaddsubpd %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfmaddsubpd %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfmaddsubps %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfmaddsubps %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfmsub132ph %xmm2,%xmm0,%xmm1\n"
        "vfmsub132ph %xmm2,%xmm3,%xmm1\n"
        "vfmsub132sh %xmm2,%xmm0,%xmm1\n"
        "vfmsub132sh %xmm2,%xmm3,%xmm1\n"
        "vfmsub213ph %xmm2,%xmm0,%xmm1\n"
        "vfmsub213ph %xmm2,%xmm3,%xmm1\n"
        "vfmsub213sh %xmm2,%xmm0,%xmm1\n"
        "vfmsub213sh %xmm2,%xmm3,%xmm1\n"
        "vfmsub231ph %xmm2,%xmm0,%xmm1\n"
        "vfmsub231ph %xmm2,%xmm3,%xmm1\n"
        "vfmsub231sh %xmm2,%xmm0,%xmm1\n"
        "vfmsub231sh %xmm2,%xmm3,%xmm1\n"
        "vfmsubadd132ph %xmm2,%xmm0,%xmm1\n"
        "vfmsubadd132ph %xmm2,%xmm3,%xmm1\n"
        "vfmsubadd213ph %xmm2,%xmm0,%xmm1\n"
        "vfmsubadd213ph %xmm2,%xmm3,%xmm1\n"
        "vfmsubadd231ph %xmm2,%xmm0,%xmm1\n"
        "vfmsubadd231ph %xmm2,%xmm3,%xmm1\n"
        "vfmsubaddpd %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfmsubaddpd %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfmsubaddps %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfmsubaddps %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfmsubpd %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfmsubpd %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfmsubps %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfmsubps %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfmsubsd %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfmsubsd %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfmsubss %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfmsubss %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfmulcph %xmm2,%xmm0,%xmm1\n"
        "vfmulcph %xmm2,%xmm3,%xmm1\n"
        "vfmulcsh %xmm2,%xmm0,%xmm1\n"
        "vfmulcsh %xmm2,%xmm3,%xmm1\n"
        "vfnmadd132ph %xmm2,%xmm0,%xmm1\n"
        "vfnmadd132ph %xmm2,%xmm3,%xmm1\n"
        "vfnmadd132sh %xmm2,%xmm0,%xmm1\n"
        "vfnmadd132sh %xmm2,%xmm3,%xmm1\n"
        "vfnmadd213ph %xmm2,%xmm0,%xmm1\n"
        "vfnmadd213ph %xmm2,%xmm3,%xmm1\n"
        "vfnmadd213sh %xmm2,%xmm0,%xmm1\n"
        "vfnmadd213sh %xmm2,%xmm3,%xmm1\n"
        "vfnmadd231ph %xmm2,%xmm0,%xmm1\n"
        "vfnmadd231ph %xmm2,%xmm3,%xmm1\n"
        "vfnmadd231sh %xmm2,%xmm0,%xmm1\n"
        "vfnmadd231sh %xmm2,%xmm3,%xmm1\n"
        "vfnmaddpd %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfnmaddpd %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfnmaddps %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfnmaddps %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfnmaddsd %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfnmaddsd %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfnmaddss %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfnmaddss %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfnmsub132ph %xmm2,%xmm0,%xmm1\n"
        "vfnmsub132ph %xmm2,%xmm3,%xmm1\n"
        "vfnmsub132sh %xmm2,%xmm0,%xmm1\n"
        "vfnmsub132sh %xmm2,%xmm3,%xmm1\n"
        "vfnmsub213ph %xmm2,%xmm0,%xmm1\n"
        "vfnmsub213ph %xmm2,%xmm3,%xmm1\n"
        "vfnmsub213sh %xmm2,%xmm0,%xmm1\n"
        "vfnmsub213sh %xmm2,%xmm3,%xmm1\n"
        "vfnmsub231ph %xmm2,%xmm0,%xmm1\n"
        "vfnmsub231ph %xmm2,%xmm3,%xmm1\n"
        "vfnmsub231sh %xmm2,%xmm0,%xmm1\n"
        "vfnmsub231sh %xmm2,%xmm3,%xmm1\n"
        "vfnmsubpd %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfnmsubpd %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfnmsubps %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfnmsubps %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfnmsubsd %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfnmsubsd %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfnmsubss %xmm2,%xmm4,%xmm0,%xmm1\n"
        "vfnmsubss %xmm2,%xmm4,%xmm3,%xmm1\n"
        "vfpclasspd $0x47,%xmm2,%k1\n"
        "vfpclasspd $0x47,%ymm2,%k1\n"
        "vfpclasspdx $0x47,0x10(%rax),%k1\n"
        "vfpclasspdy $0x47,0x20(%rax),%k1\n"
        "vfpclasspdz $0x47,0x40(%rax),%k1\n"
        "vfpclassph $0x47,%xmm2,%k1\n"
        "vfpclassph $0x47,%ymm2,%k1\n"
        "vfpclassphx $0x47,0x10(%rax),%k1\n"
        "vfpclassphy $0x47,0x20(%rax),%k1\n"
        "vfpclassphz $0x47,0x40(%rax),%k1\n"
        "vfpclassps $0x47,%xmm2,%k1\n"
        "vfpclassps $0x47,%ymm2,%k1\n"
        "vfpclasspsx $0x47,0x10(%rax),%k1\n"
        "vfpclasspsy $0x47,0x20(%rax),%k1\n"
        "vfpclasspsz $0x47,0x40(%rax),%k1\n"
        "vfpclasssd $0x47,%xmm2,%k1\n"
        "vfpclasssd $0x47,0x8(%rax),%k1\n"
        "vfpclasssh $0x47,%xmm2,%k1\n"
        "vfpclasssh $0x47,0x2(%rax),%k1\n"
        "vfpclassss $0x47,%xmm2,%k1\n"
        "vfpclassss $0x47,0x4(%rax),%k1\n"
        "vgetexppd %xmm2,%xmm1\n"
        "vgetexppd %ymm2,%ymm1\n"
        "vgetexpph %xmm2,%xmm1\n"
        "vgetexpph %ymm2,%ymm1\n"
        "vgetexpps %xmm2,%xmm1\n"
        "vgetexpps %ymm2,%ymm1\n"
        "vgetexpsd %xmm2,%xmm0,%xmm1\n"
        "vgetexpsd %xmm2,%xmm3,%xmm1\n"
        "vgetexpsh %xmm2,%xmm0,%xmm1\n"
        "vgetexpsh %xmm2,%xmm3,%xmm1\n"
        "vgetexpss %xmm2,%xmm0,%xmm1\n"
        "vgetexpss %xmm2,%xmm3,%xmm1\n"
        "vgetmantpd $0x47,%xmm2,%xmm1\n"
        "vgetmantpd $0x47,%ymm2,%ymm1\n"
        "vgetmantph $0x47,%xmm2,%xmm1\n"
        "vgetmantph $0x47,%ymm2,%ymm1\n"
        "vgetmantps $0x47,%xmm2,%xmm1\n"
        "vgetmantps $0x47,%ymm2,%ymm1\n"
        "vgetmantsd $0x47,%xmm2,%xmm0,%xmm1\n"
        "vgetmantsd $0x47,%xmm2,%xmm3,%xmm1\n"
        "vgetmantsh $0x47,%xmm2,%xmm0,%xmm1\n"
        "vgetmantsh $0x47,%xmm2,%xmm3,%xmm1\n"
        "vgetmantss $0x47,%xmm2,%xmm0,%xmm1\n"
        "vgetmantss $0x47,%xmm2,%xmm3,%xmm1\n"
        "vmaskmovdqu %xmm2,%xmm1\n"
        "vmaxph %xmm2,%xmm0,%xmm1\n"
        "vmaxph %xmm2,%xmm3,%xmm1\n"
        "vmaxsh %xmm2,%xmm0,%xmm1\n"
        "vmaxsh %xmm2,%xmm3,%xmm1\n"
        "vminph %xmm2,%xmm0,%xmm1\n"
        "vminph %xmm2,%xmm3,%xmm1\n"
        "vminsh %xmm2,%xmm0,%xmm1\n"
        "vminsh %xmm2,%xmm3,%xmm1\n"
        "vmovddup %xmm2,%xmm1\n"
        "vmovddup %ymm2,%ymm1\n"
        "vmovntdq %xmm1,0x1(%rax)\n"
        "vmovntdq %ymm1,0x1(%rax)\n"
        "vmovntdqa 0x1(%rax),%xmm1\n"
        "vmovntdqa 0x1(%rax),%ymm1\n"
        "vmovntpd %xmm1,0x1(%rax)\n"
        "vmovntpd %ymm1,0x1(%rax)\n"
        "vmovntps %xmm1,0x1(%rax)\n"
        "vmovntps %ymm1,0x1(%rax)\n"
        "vmovsd %xmm1,%xmm0,%xmm2\n"
        "vmovsd %xmm1,%xmm3,%xmm2\n"
        "vmovsh %xmm1,%xmm0,%xmm2\n"
        "vmovsh %xmm1,%xmm3,%xmm2\n"
        "vmovshdup %xmm2,%xmm1\n"
        "vmovshdup %ymm2,%ymm1\n"
        "vmovsldup %xmm2,%xmm1\n"
        "vmovsldup %ymm2,%ymm1\n"
        "vmovss %xmm1,%xmm0,%xmm2\n"
        "vmovss %xmm1,%xmm3,%xmm2\n"
        "vmovw %edx,%xmm1\n"
        "vmovw %xmm1,%edx\n"
        "vmulph %xmm2,%xmm0,%xmm1\n"
        "vmulph %xmm2,%xmm3,%xmm1\n"
        "vmulsh %xmm2,%xmm0,%xmm1\n"
        "vmulsh %xmm2,%xmm3,%xmm1\n"
        "vp2intersectd %xmm2,%xmm0,%k1\n"
        "vp2intersectd %xmm2,%xmm3,%k1\n"
        "vp2intersectq %xmm2,%xmm0,%k1\n"
        "vp2intersectq %xmm2,%xmm3,%k1\n"
        "vp4dpwssd 0x10(%rax),%zmm0,%zmm1\n"
        "vp4dpwssd 0x10(%rax),%zmm3,%zmm1\n"
        "vp4dpwssds 0x10(%rax),%zmm0,%zmm1\n"
        "vp4dpwssds 0x10(%rax),%zmm3,%zmm1\n"
        "vpackssdw %xmm2,%xmm0,%xmm1\n"
        "vpackssdw %xmm2,%xmm3,%xmm1\n"
        "vpblendmb %xmm2,%xmm0,%xmm1\n"
        "vpblendmb %xmm2,%xmm3,%xmm1\n"
        "vpblendmd %xmm2,%xmm0,%xmm1\n"
        "vpblendmd %xmm2,%xmm3,%xmm1\n"
        "vpblendmq %xmm2,%xmm0,%xmm1\n"
        "vpblendmq %xmm2,%xmm3,%xmm1\n"
        "vpblendmw %xmm2,%xmm0,%xmm1\n"
        "vpblendmw %xmm2,%xmm3,%xmm1\n"
        "vpcmpestriq $0x47,%xmm2,%xmm1\n"
        "vpcmpestriq $0x47,0x1(%rax),%xmm1\n"
        "vpcmpestrmq $0x47,%xmm2,%xmm1\n"
        "vpcmpestrmq $0x47,0x1(%rax),%xmm1\n"
        "vpcompressb %xmm1,%xmm2\n"
        "vpcompressb %xmm1,0x1(%rax)\n"
        "vpcompressw %xmm1,%xmm2\n"
        "vpcompressw %xmm1,0x2(%rax)\n"
        "vpdpbssd %xmm2,%xmm0,%xmm1\n"
        "vpdpbssd %xmm2,%xmm3,%xmm1\n"
        "vpdpbssds %xmm2,%xmm0,%xmm1\n"
        "vpdpbssds %xmm2,%xmm3,%xmm1\n"
        "vpdpbsud %xmm2,%xmm0,%xmm1\n"
        "vpdpbsud %xmm2,%xmm3,%xmm1\n"
        "vpdpbsuds %xmm2,%xmm0,%xmm1\n"
        "vpdpbsuds %xmm2,%xmm3,%xmm1\n"
        "vpdpbuud %xmm2,%xmm0,%xmm1\n"
        "vpdpbuud %xmm2,%xmm3,%xmm1\n"
        "vpdpbuuds %xmm2,%xmm0,%xmm1\n"
        "vpdpbuuds %xmm2,%xmm3,%xmm1\n"
        "vpdpwsud %xmm2,%xmm0,%xmm1\n"
        "vpdpwsud %xmm2,%xmm3,%xmm1\n"
        "vpdpwsuds %xmm2,%xmm0,%xmm1\n"
        "vpdpwsuds %xmm2,%xmm3,%xmm1\n"
        "vpdpwusd %xmm2,%xmm0,%xmm1\n"
        "vpdpwusd %xmm2,%xmm3,%xmm1\n"
        "vpdpwusds %xmm2,%xmm0,%xmm1\n"
        "vpdpwusds %xmm2,%xmm3,%xmm1\n"
        "vpdpwuud %xmm2,%xmm0,%xmm1\n"
        "vpdpwuud %xmm2,%xmm3,%xmm1\n"
        "vpdpwuuds %xmm2,%xmm0,%xmm1\n"
        "vpdpwuuds %xmm2,%xmm3,%xmm1\n"
        "vpermil2pd $0x7,%xmm2,%xmm4,%xmm0,%xmm1\n"
        "vpermil2pd $0x7,%xmm2,%xmm4,%xmm3,%xmm1\n"
        "vpermil2ps $0x7,%xmm2,%xmm4,%xmm0,%xmm1\n"
        "vpermil2ps $0x7,%xmm2,%xmm4,%xmm3,%xmm1\n"
        "vpermilpd $0x47,%xmm2,%xmm1\n"
        "vpermilpd $0x47,%ymm2,%ymm1\n"
        "vpermilps $0x47,%xmm2,%xmm1\n"
        "vpermilps $0x47,%ymm2,%ymm1\n"
        "vpexpandb %xmm2,%xmm1\n"
        "vpexpandb %ymm2,%ymm1\n"
        "vpexpandw %xmm2,%xmm1\n"
        "vpexpandw %ymm2,%ymm1\n"
        "vpextrw $0x47,%xmm1,%edx\n"
        "vpextrw $0x47,%xmm1,0x1(%rax)\n"
        "vpmaxsq %xmm2,%xmm0,%xmm1\n"
        "vpmaxsq %xmm2,%xmm3,%xmm1\n"
        "vpmaxuq %xmm2,%xmm0,%xmm1\n"
        "vpmaxuq %xmm2,%xmm3,%xmm1\n"
        "vpminsq %xmm2,%xmm0,%xmm1\n"
        "vpminsq %xmm2,%xmm3,%xmm1\n"
        "vpminuq %xmm2,%xmm0,%xmm1\n"
        "vpminuq %xmm2,%xmm3,%xmm1\n"
        "vpmovb2m %xmm2,%k1\n"
        "vpmovb2m %ymm2,%k1\n"
        "vpmovd2m %xmm2,%k1\n"
        "vpmovd2m %ymm2,%k1\n"
        "vpmovm2b %k2,%xmm1\n"
        "vpmovm2b %k2,%ymm1\n"
        "vpmovm2d %k2,%xmm1\n"
        "vpmovm2d %k2,%ymm1\n"
        "vpmovm2q %k2,%xmm1\n"
        "vpmovm2q %k2,%ymm1\n"
        "vpmovm2w %k2,%xmm1\n"
        "vpmovm2w %k2,%ymm1\n"
        "vpmovq2m %xmm2,%k1\n"
        "vpmovq2m %ymm2,%k1\n"
        "vpmovsxbd %xmm2,%xmm1\n"
        "vpmovsxbd %xmm2,%ymm1\n"
        "vpmovsxbq %xmm2,%xmm1\n"
        "vpmovsxbq %xmm2,%ymm1\n"
        "vpmovsxdq %xmm2,%xmm1\n"
        "vpmovsxdq %xmm2,%ymm1\n"
        "vpmovsxwd %xmm2,%xmm1\n"
        "vpmovsxwd %xmm2,%ymm1\n"
        "vpmovsxwq %xmm2,%xmm1\n"
        "vpmovsxwq %xmm2,%ymm1\n"
        "vpmovw2m %xmm2,%k1\n"
        "vpmovw2m %ymm2,%k1\n"
        "vpmovzxbd %xmm2,%xmm1\n"
        "vpmovzxbd %xmm2,%ymm1\n"
        "vpmovzxbq %xmm2,%xmm1\n"
        "vpmovzxbq %xmm2,%ymm1\n"
        "vpmovzxdq %xmm2,%xmm1\n"
        "vpmovzxdq %xmm2,%ymm1\n"
        "vpmovzxwd %xmm2,%xmm1\n"
        "vpmovzxwd %xmm2,%ymm1\n"
        "vpmovzxwq %xmm2,%xmm1\n"
        "vpmovzxwq %xmm2,%ymm1\n"
        "vpmullq %xmm2,%xmm0,%xmm1\n"
        "vpmullq %xmm2,%xmm3,%xmm1\n"
        "vprold $0x47,%xmm2,%xmm0\n"
        "vprold $0x47,%xmm2,%xmm3\n"
        "vprolq $0x47,%xmm2,%xmm0\n"
        "vprolq $0x47,%xmm2,%xmm3\n"
        "vprolvd %xmm2,%xmm0,%xmm1\n"
        "vprolvd %xmm2,%xmm3,%xmm1\n"
        "vprolvq %xmm2,%xmm0,%xmm1\n"
        "vprolvq %xmm2,%xmm3,%xmm1\n"
        "vprorvd %xmm2,%xmm0,%xmm1\n"
        "vprorvd %xmm2,%xmm3,%xmm1\n"
        "vprorvq %xmm2,%xmm0,%xmm1\n"
        "vprorvq %xmm2,%xmm3,%xmm1\n"
        "vpshldd $0x47,%xmm2,%xmm0,%xmm1\n"
        "vpshldd $0x47,%xmm2,%xmm3,%xmm1\n"
        "vpshldq $0x47,%xmm2,%xmm0,%xmm1\n"
        "vpshldq $0x47,%xmm2,%xmm3,%xmm1\n"
        "vpshldvd %xmm2,%xmm0,%xmm1\n"
        "vpshldvd %xmm2,%xmm3,%xmm1\n"
        "vpshldvq %xmm2,%xmm0,%xmm1\n"
        "vpshldvq %xmm2,%xmm3,%xmm1\n"
        "vpshldvw %xmm2,%xmm0,%xmm1\n"
        "vpshldvw %xmm2,%xmm3,%xmm1\n"
        "vpshldw $0x47,%xmm2,%xmm0,%xmm1\n"
        "vpshldw $0x47,%xmm2,%xmm3,%xmm1\n"
        "vpshrdd $0x47,%xmm2,%xmm0,%xmm1\n"
        "vpshrdd $0x47,%xmm2,%xmm3,%xmm1\n"
        "vpshrdq $0x47,%xmm2,%xmm0,%xmm1\n"
        "vpshrdq $0x47,%xmm2,%xmm3,%xmm1\n"
        "vpshrdvd %xmm2,%xmm0,%xmm1\n"
        "vpshrdvd %xmm2,%xmm3,%xmm1\n"
        "vpshrdvq %xmm2,%xmm0,%xmm1\n"
        "vpshrdvq %xmm2,%xmm3,%xmm1\n"
        "vpshrdvw %xmm2,%xmm0,%xmm1\n"
        "vpshrdvw %xmm2,%xmm3,%xmm1\n"
        "vpshrdw $0x47,%xmm2,%xmm0,%xmm1\n"
        "vpshrdw $0x47,%xmm2,%xmm3,%xmm1\n"
        "vpshufhw $0x47,%xmm2,%xmm1\n"
        "vpshufhw $0x47,%ymm2,%ymm1\n"
        "vpshuflw $0x47,%xmm2,%xmm1\n"
        "vpshuflw $0x47,%ymm2,%ymm1\n"
        "vpslld %xmm2,%xmm0,%xmm1\n"
        "vpslld %xmm2,%xmm3,%xmm1\n"
        "vpsllq %xmm2,%xmm0,%xmm1\n"
        "vpsllq %xmm2,%xmm3,%xmm1\n"
        "vpsllvw %xmm2,%xmm0,%xmm1\n"
        "vpsllvw %xmm2,%xmm3,%xmm1\n"
        "vpsllw %xmm2,%xmm0,%xmm1\n"
        "vpsllw %xmm2,%xmm3,%xmm1\n"
        "vpsrad %xmm2,%xmm0,%xmm1\n"
        "vpsrad %xmm2,%xmm3,%xmm1\n"
        "vpsraq %xmm2,%xmm0,%xmm1\n"
        "vpsraq %xmm2,%xmm3,%xmm1\n"
        "vpsravw %xmm2,%xmm0,%xmm1\n"
        "vpsravw %xmm2,%xmm3,%xmm1\n"
        "vpsraw %xmm2,%xmm0,%xmm1\n"
        "vpsraw %xmm2,%xmm3,%xmm1\n"
        "vpsrld %xmm2,%xmm0,%xmm1\n"
        "vpsrld %xmm2,%xmm3,%xmm1\n"
        "vpsrlq %xmm2,%xmm0,%xmm1\n"
        "vpsrlq %xmm2,%xmm3,%xmm1\n"
        "vpsrlvw %xmm2,%xmm0,%xmm1\n"
        "vpsrlvw %xmm2,%xmm3,%xmm1\n"
        "vpsrlw %xmm2,%xmm0,%xmm1\n"
        "vpsrlw %xmm2,%xmm3,%xmm1\n"
        "vptestmb %xmm2,%xmm0,%k1\n"
        "vptestmb %xmm2,%xmm3,%k1\n"
        "vptestmd %xmm2,%xmm0,%k1\n"
        "vptestmd %xmm2,%xmm3,%k1\n"
        "vptestmq %xmm2,%xmm0,%k1\n"
        "vptestmq %xmm2,%xmm3,%k1\n"
        "vptestmw %xmm2,%xmm0,%k1\n"
        "vptestmw %xmm2,%xmm3,%k1\n"
        "vptestnmb %xmm2,%xmm0,%k1\n"
        "vptestnmb %xmm2,%xmm3,%k1\n"
        "vptestnmd %xmm2,%xmm0,%k1\n"
        "vptestnmd %xmm2,%xmm3,%k1\n"
        "vptestnmq %xmm2,%xmm0,%k1\n"
        "vptestnmq %xmm2,%xmm3,%k1\n"
        "vptestnmw %xmm2,%xmm0,%k1\n"
        "vptestnmw %xmm2,%xmm3,%k1\n"
        "vrangepd $0x47,%xmm2,%xmm0,%xmm1\n"
        "vrangepd $0x47,%xmm2,%xmm3,%xmm1\n"
        "vrangeps $0x47,%xmm2,%xmm0,%xmm1\n"
        "vrangeps $0x47,%xmm2,%xmm3,%xmm1\n"
        "vrangesd $0x47,%xmm2,%xmm0,%xmm1\n"
        "vrangesd $0x47,%xmm2,%xmm3,%xmm1\n"
        "vrangess $0x47,%xmm2,%xmm0,%xmm1\n"
        "vrangess $0x47,%xmm2,%xmm3,%xmm1\n"
        "vrcp14pd %xmm2,%xmm1\n"
        "vrcp14pd %ymm2,%ymm1\n"
        "vrcp14ps %xmm2,%xmm1\n"
        "vrcp14ps %ymm2,%ymm1\n"
        "vrcp14sd %xmm2,%xmm0,%xmm1\n"
        "vrcp14sd %xmm2,%xmm3,%xmm1\n"
        "vrcp14ss %xmm2,%xmm0,%xmm1\n"
        "vrcp14ss %xmm2,%xmm3,%xmm1\n"
        "vrcp28pd %zmm2,%zmm1\n"
        "vrcp28pd 0x40(%rax),%zmm1\n"
        "vrcp28ps %zmm2,%zmm1\n"
        "vrcp28ps 0x40(%rax),%zmm1\n"
        "vrcp28sd %xmm2,%xmm0,%xmm1\n"
        "vrcp28sd %xmm2,%xmm3,%xmm1\n"
        "vrcp28ss %xmm2,%xmm0,%xmm1\n"
        "vrcp28ss %xmm2,%xmm3,%xmm1\n"
        "vrcpph %xmm2,%xmm1\n"
        "vrcpph %ymm2,%ymm1\n"
        "vrcpsh %xmm2,%xmm0,%xmm1\n"
        "vrcpsh %xmm2,%xmm3,%xmm1\n"
        "vreducepd $0x47,%xmm2,%xmm1\n"
        "vreducepd $0x47,%ymm2,%ymm1\n"
        "vreduceph $0x47,%xmm2,%xmm1\n"
        "vreduceph $0x47,%ymm2,%ymm1\n"
        "vreduceps $0x47,%xmm2,%xmm1\n"
        "vreduceps $0x47,%ymm2,%ymm1\n"
        "vreducesd $0x47,%xmm2,%xmm0,%xmm1\n"
        "vreducesd $0x47,%xmm2,%xmm3,%xmm1\n"
        "vreducesh $0x47,%xmm2,%xmm0,%xmm1\n"
        "vreducesh $0x47,%xmm2,%xmm3,%xmm1\n"
        "vreducess $0x47,%xmm2,%xmm0,%xmm1\n"
        "vreducess $0x47,%xmm2,%xmm3,%xmm1\n"
        "vrndscalepd $0x47,%xmm2,%xmm1\n"
        "vrndscalepd $0x47,%ymm2,%ymm1\n"
        "vrndscaleph $0x47,%xmm2,%xmm1\n"
        "vrndscaleph $0x47,%ymm2,%ymm1\n"
        "vrndscaleps $0x47,%xmm2,%xmm1\n"
        "vrndscaleps $0x47,%ymm2,%ymm1\n"
        "vrndscalesd $0x47,%xmm2,%xmm0,%xmm1\n"
        "vrndscalesd $0x47,%xmm2,%xmm3,%xmm1\n"
        "vrndscalesh $0x47,%xmm2,%xmm0,%xmm1\n"
        "vrndscalesh $0x47,%xmm2,%xmm3,%xmm1\n"
        "vrndscaless $0x47,%xmm2,%xmm0,%xmm1\n"
        "vrndscaless $0x47,%xmm2,%xmm3,%xmm1\n"
        "vrsqrt14pd %xmm2,%xmm1\n"
        "vrsqrt14pd %ymm2,%ymm1\n"
        "vrsqrt14ps %xmm2,%xmm1\n"
        "vrsqrt14ps %ymm2,%ymm1\n"
        "vrsqrt14sd %xmm2,%xmm0,%xmm1\n"
        "vrsqrt14sd %xmm2,%xmm3,%xmm1\n"
        "vrsqrt14ss %xmm2,%xmm0,%xmm1\n"
        "vrsqrt14ss %xmm2,%xmm3,%xmm1\n"
        "vrsqrt28pd %zmm2,%zmm1\n"
        "vrsqrt28pd 0x40(%rax),%zmm1\n"
        "vrsqrt28ps %zmm2,%zmm1\n"
        "vrsqrt28ps 0x40(%rax),%zmm1\n"
        "vrsqrt28sd %xmm2,%xmm0,%xmm1\n"
        "vrsqrt28sd %xmm2,%xmm3,%xmm1\n"
        "vrsqrt28ss %xmm2,%xmm0,%xmm1\n"
        "vrsqrt28ss %xmm2,%xmm3,%xmm1\n"
        "vrsqrtph %xmm2,%xmm1\n"
        "vrsqrtph %ymm2,%ymm1\n"
        "vrsqrtsh %xmm2,%xmm0,%xmm1\n"
        "vrsqrtsh %xmm2,%xmm3,%xmm1\n"
        "vscalefpd %xmm2,%xmm0,%xmm1\n"
        "vscalefpd %xmm2,%xmm3,%xmm1\n"
        "vscalefph %xmm2,%xmm0,%xmm1\n"
        "vscalefph %xmm2,%xmm3,%xmm1\n"
        "vscalefps %xmm2,%xmm0,%xmm1\n"
        "vscalefps %xmm2,%xmm3,%xmm1\n"
        "vscalefsd %xmm2,%xmm0,%xmm1\n"
        "vscalefsd %xmm2,%xmm3,%xmm1\n"
        "vscalefsh %xmm2,%xmm0,%xmm1\n"
        "vscalefsh %xmm2,%xmm3,%xmm1\n"
        "vscalefss %xmm2,%xmm0,%xmm1\n"
        "vscalefss %xmm2,%xmm3,%xmm1\n"
        "vsha512msg1 %xmm2,%ymm1\n"
        "vsha512msg2 %ymm2,%ymm1\n"
        "vsha512rnds2 %xmm2,%ymm0,%ymm1\n"
        "vsha512rnds2 %xmm2,%ymm3,%ymm1\n"
        "vshuff32x4 $0x47,%ymm2,%ymm0,%ymm1\n"
        "vshuff32x4 $0x47,%ymm2,%ymm3,%ymm1\n"
        "vshuff64x2 $0x47,%ymm2,%ymm0,%ymm1\n"
        "vshuff64x2 $0x47,%ymm2,%ymm3,%ymm1\n"
        "vshufi32x4 $0x47,%ymm2,%ymm0,%ymm1\n"
        "vshufi32x4 $0x47,%ymm2,%ymm3,%ymm1\n"
        "vshufi64x2 $0x47,%ymm2,%ymm0,%ymm1\n"
        "vshufi64x2 $0x47,%ymm2,%ymm3,%ymm1\n"
        "vsm3msg1 %xmm2,%xmm0,%xmm1\n"
        "vsm3msg1 %xmm2,%xmm3,%xmm1\n"
        "vsm3msg2 %xmm2,%xmm0,%xmm1\n"
        "vsm3msg2 %xmm2,%xmm3,%xmm1\n"
        "vsm3rnds2 $0x47,%xmm2,%xmm0,%xmm1\n"
        "vsm3rnds2 $0x47,%xmm2,%xmm3,%xmm1\n"
        "vsm4key4 %xmm2,%xmm0,%xmm1\n"
        "vsm4key4 %xmm2,%xmm3,%xmm1\n"
        "vsm4rnds4 %xmm2,%xmm0,%xmm1\n"
        "vsm4rnds4 %xmm2,%xmm3,%xmm1\n"
        "vsqrtph %xmm2,%xmm1\n"
        "vsqrtph %ymm2,%ymm1\n"
        "vsqrtsh %xmm2,%xmm0,%xmm1\n"
        "vsqrtsh %xmm2,%xmm3,%xmm1\n"
        "vsubph %xmm2,%xmm0,%xmm1\n"
        "vsubph %xmm2,%xmm3,%xmm1\n"
        "vsubsh %xmm2,%xmm0,%xmm1\n"
        "vsubsh %xmm2,%xmm3,%xmm1\n"
        "vtestpd %xmm2,%xmm1\n"
        "vtestpd %ymm2,%ymm1\n"
        "vtestps %xmm2,%xmm1\n"
        "vtestps %ymm2,%ymm1\n"
        "vucomish %xmm2,%xmm1\n"
        "vucomish 0x2(%rax),%xmm1\n"
        ".intel_syntax noprefix\n"
        "vcvtusi2ss xmm1,xmm0,eax,{rn-sae}\n"
        "vrangess xmm0{k1},xmm31,xmm28,{sae},0xc\n"
        "vaddph zmm1,zmm2,WORD BCST [rax+2]\n"
        "vcvtph2pd zmm1{k2}{z},WORD BCST [rax]\n"
        "vfmaddps xmm1,xmm2,xmm3,xmm4\n"
        "vpermil2ps xmm1,xmm2,xmm3,xmm4,2\n"
        ".att_syntax\n"
        ".globl axiom_asm_table_end\n"
        "axiom_asm_table_end:\n");

extern const unsigned char axiom_asm_table[], axiom_asm_table_end[];
static const char *const want[] = {
    "c4e279e64801",  /* cmpbexadd %eax,%ecx,0x1(%rax) */
    "c4e261e64801",  /* cmpbexadd %ebx,%ecx,0x1(%rax) */
    "c4e279e24801",  /* cmpbxadd %eax,%ecx,0x1(%rax) */
    "c4e261e24801",  /* cmpbxadd %ebx,%ecx,0x1(%rax) */
    "c4e279ee4801",  /* cmplexadd %eax,%ecx,0x1(%rax) */
    "c4e261ee4801",  /* cmplexadd %ebx,%ecx,0x1(%rax) */
    "c4e279ec4801",  /* cmplxadd %eax,%ecx,0x1(%rax) */
    "c4e261ec4801",  /* cmplxadd %ebx,%ecx,0x1(%rax) */
    "c4e279e74801",  /* cmpnbexadd %eax,%ecx,0x1(%rax) */
    "c4e261e74801",  /* cmpnbexadd %ebx,%ecx,0x1(%rax) */
    "c4e279e34801",  /* cmpnbxadd %eax,%ecx,0x1(%rax) */
    "c4e261e34801",  /* cmpnbxadd %ebx,%ecx,0x1(%rax) */
    "c4e279ef4801",  /* cmpnlexadd %eax,%ecx,0x1(%rax) */
    "c4e261ef4801",  /* cmpnlexadd %ebx,%ecx,0x1(%rax) */
    "c4e279ed4801",  /* cmpnlxadd %eax,%ecx,0x1(%rax) */
    "c4e261ed4801",  /* cmpnlxadd %ebx,%ecx,0x1(%rax) */
    "c4e279e14801",  /* cmpnoxadd %eax,%ecx,0x1(%rax) */
    "c4e261e14801",  /* cmpnoxadd %ebx,%ecx,0x1(%rax) */
    "c4e279eb4801",  /* cmpnpxadd %eax,%ecx,0x1(%rax) */
    "c4e261eb4801",  /* cmpnpxadd %ebx,%ecx,0x1(%rax) */
    "c4e279e94801",  /* cmpnsxadd %eax,%ecx,0x1(%rax) */
    "c4e261e94801",  /* cmpnsxadd %ebx,%ecx,0x1(%rax) */
    "c4e279e54801",  /* cmpnzxadd %eax,%ecx,0x1(%rax) */
    "c4e261e54801",  /* cmpnzxadd %ebx,%ecx,0x1(%rax) */
    "c4e279e04801",  /* cmpoxadd %eax,%ecx,0x1(%rax) */
    "c4e261e04801",  /* cmpoxadd %ebx,%ecx,0x1(%rax) */
    "c4e279ea4801",  /* cmppxadd %eax,%ecx,0x1(%rax) */
    "c4e261ea4801",  /* cmppxadd %ebx,%ecx,0x1(%rax) */
    "c4e279e84801",  /* cmpsxadd %eax,%ecx,0x1(%rax) */
    "c4e261e84801",  /* cmpsxadd %ebx,%ecx,0x1(%rax) */
    "c4e279e44801",  /* cmpzxadd %eax,%ecx,0x1(%rax) */
    "c4e261e44801",  /* cmpzxadd %ebx,%ecx,0x1(%rax) */
    "c5f992ca",  /* kmovb %edx,%k1 */
    "c5f9914801",  /* kmovb %k1,0x1(%rax) */
    "c5fb92ca",  /* kmovd %edx,%k1 */
    "c4e1f9914801",  /* kmovd %k1,0x1(%rax) */
    "c4e1f8914801",  /* kmovq %k1,0x1(%rax) */
    "c4e1f890ca",  /* kmovq %k2,%k1 */
    "c5f892ca",  /* kmovw %edx,%k1 */
    "c5f8914801",  /* kmovw %k1,0x1(%rax) */
    "c4e37932ca47",  /* kshiftlb $0x47,%k2,%k1 */
    "c4e37933ca47",  /* kshiftld $0x47,%k2,%k1 */
    "c4e3f933ca47",  /* kshiftlq $0x47,%k2,%k1 */
    "c4e3f932ca47",  /* kshiftlw $0x47,%k2,%k1 */
    "c4e37930ca47",  /* kshiftrb $0x47,%k2,%k1 */
    "c4e37931ca47",  /* kshiftrd $0x47,%k2,%k1 */
    "c4e3f931ca47",  /* kshiftrq $0x47,%k2,%k1 */
    "c4e3f930ca47",  /* kshiftrw $0x47,%k2,%k1 */
    "62f27f489a4801",  /* v4fmaddps 0x10(%rax),%zmm0,%zmm1 */
    "62f267489a4801",  /* v4fmaddps 0x10(%rax),%zmm3,%zmm1 */
    "62f27f089b4801",  /* v4fmaddss 0x10(%rax),%xmm0,%xmm1 */
    "62f267089b4801",  /* v4fmaddss 0x10(%rax),%xmm3,%xmm1 */
    "62f27f48aa4801",  /* v4fnmaddps 0x10(%rax),%zmm0,%zmm1 */
    "62f26748aa4801",  /* v4fnmaddps 0x10(%rax),%zmm3,%zmm1 */
    "62f27f08ab4801",  /* v4fnmaddss 0x10(%rax),%xmm0,%xmm1 */
    "62f26708ab4801",  /* v4fnmaddss 0x10(%rax),%xmm3,%xmm1 */
    "62f57c0858ca",  /* vaddph %xmm2,%xmm0,%xmm1 */
    "62f5640858ca",  /* vaddph %xmm2,%xmm3,%xmm1 */
    "62f57e0858ca",  /* vaddsh %xmm2,%xmm0,%xmm1 */
    "62f5660858ca",  /* vaddsh %xmm2,%xmm3,%xmm1 */
    "62f37d0803ca47",  /* valignd $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3650803ca47",  /* valignd $0x47,%xmm2,%xmm3,%xmm1 */
    "62f3fd0803ca47",  /* valignq $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3e50803ca47",  /* valignq $0x47,%xmm2,%xmm3,%xmm1 */
    "c4e27ab14801",  /* vbcstnebf162ps 0x1(%rax),%xmm1 */
    "c4e27eb14801",  /* vbcstnebf162ps 0x1(%rax),%ymm1 */
    "c4e279b14801",  /* vbcstnesh2ps 0x1(%rax),%xmm1 */
    "c4e27db14801",  /* vbcstnesh2ps 0x1(%rax),%ymm1 */
    "62f2fd0865ca",  /* vblendmpd %xmm2,%xmm0,%xmm1 */
    "62f2e50865ca",  /* vblendmpd %xmm2,%xmm3,%xmm1 */
    "62f27d0865ca",  /* vblendmps %xmm2,%xmm0,%xmm1 */
    "62f2650865ca",  /* vblendmps %xmm2,%xmm3,%xmm1 */
    "c4e27d1a4801",  /* vbroadcastf128 0x1(%rax),%ymm1 */
    "62f27d2819ca",  /* vbroadcastf32x2 %xmm2,%ymm1 */
    "62f27d4819ca",  /* vbroadcastf32x2 %xmm2,%zmm1 */
    "62f27d281a4801",  /* vbroadcastf32x4 0x10(%rax),%ymm1 */
    "62f27d481a4801",  /* vbroadcastf32x4 0x10(%rax),%zmm1 */
    "62f27d481b4801",  /* vbroadcastf32x8 0x20(%rax),%zmm1 */
    "62f2fd281a4801",  /* vbroadcastf64x2 0x10(%rax),%ymm1 */
    "62f2fd481a4801",  /* vbroadcastf64x2 0x10(%rax),%zmm1 */
    "62f2fd481b4801",  /* vbroadcastf64x4 0x20(%rax),%zmm1 */
    "c4e27d5a4801",  /* vbroadcasti128 0x1(%rax),%ymm1 */
    "62f27d0859ca",  /* vbroadcasti32x2 %xmm2,%xmm1 */
    "62f27d2859ca",  /* vbroadcasti32x2 %xmm2,%ymm1 */
    "62f27d285a4801",  /* vbroadcasti32x4 0x10(%rax),%ymm1 */
    "62f27d485a4801",  /* vbroadcasti32x4 0x10(%rax),%zmm1 */
    "62f27d485b4801",  /* vbroadcasti32x8 0x20(%rax),%zmm1 */
    "62f2fd285a4801",  /* vbroadcasti64x2 0x10(%rax),%ymm1 */
    "62f2fd485a4801",  /* vbroadcasti64x2 0x10(%rax),%zmm1 */
    "62f2fd485b4801",  /* vbroadcasti64x4 0x20(%rax),%zmm1 */
    "62f37c08c2ca47",  /* vcmpph $0x47,%xmm2,%xmm0,%k1 */
    "62f36408c2ca47",  /* vcmpph $0x47,%xmm2,%xmm3,%k1 */
    "62f37e08c2ca47",  /* vcmpsh $0x47,%xmm2,%xmm0,%k1 */
    "62f36608c2ca47",  /* vcmpsh $0x47,%xmm2,%xmm3,%k1 */
    "62f57c082fca",  /* vcomish %xmm2,%xmm1 */
    "62f57c082f4801",  /* vcomish 0x2(%rax),%xmm1 */
    "c5fae6ca",  /* vcvtdq2pd %xmm2,%xmm1 */
    "c5fee6ca",  /* vcvtdq2pd %xmm2,%ymm1 */
    "62f57c085bca",  /* vcvtdq2ph %xmm2,%xmm1 */
    "62f57c285bca",  /* vcvtdq2ph %ymm2,%xmm1 */
    "62f57c085b4801",  /* vcvtdq2phx 0x10(%rax),%xmm1 */
    "62f57c285b4801",  /* vcvtdq2phy 0x20(%rax),%xmm1 */
    "62f27f0872ca",  /* vcvtne2ps2bf16 %xmm2,%xmm0,%xmm1 */
    "62f2670872ca",  /* vcvtne2ps2bf16 %xmm2,%xmm3,%xmm1 */
    "c4e27ab04801",  /* vcvtneebf162ps 0x1(%rax),%xmm1 */
    "c4e27eb04801",  /* vcvtneebf162ps 0x1(%rax),%ymm1 */
    "c4e279b04801",  /* vcvtneeph2ps 0x1(%rax),%xmm1 */
    "c4e27db04801",  /* vcvtneeph2ps 0x1(%rax),%ymm1 */
    "c4e27bb04801",  /* vcvtneobf162ps 0x1(%rax),%xmm1 */
    "c4e27fb04801",  /* vcvtneobf162ps 0x1(%rax),%ymm1 */
    "c4e278b04801",  /* vcvtneoph2ps 0x1(%rax),%xmm1 */
    "c4e27cb04801",  /* vcvtneoph2ps 0x1(%rax),%ymm1 */
    "62f27e0872ca",  /* vcvtneps2bf16 %xmm2,%xmm1 */
    "62f27e2872ca",  /* vcvtneps2bf16 %ymm2,%xmm1 */
    "62f27e08724801",  /* vcvtneps2bf16x 0x10(%rax),%xmm1 */
    "62f27e28724801",  /* vcvtneps2bf16y 0x20(%rax),%xmm1 */
    "c5fbe6ca",  /* vcvtpd2dq %xmm2,%xmm1 */
    "c5ffe6ca",  /* vcvtpd2dq %ymm2,%xmm1 */
    "c5fbe64801",  /* vcvtpd2dqx 0x1(%rax),%xmm1 */
    "c5ffe64801",  /* vcvtpd2dqy 0x1(%rax),%xmm1 */
    "62f5fd085aca",  /* vcvtpd2ph %xmm2,%xmm1 */
    "62f5fd285aca",  /* vcvtpd2ph %ymm2,%xmm1 */
    "62f5fd085a4801",  /* vcvtpd2phx 0x10(%rax),%xmm1 */
    "62f5fd285a4801",  /* vcvtpd2phy 0x20(%rax),%xmm1 */
    "62f5fd485a4801",  /* vcvtpd2phz 0x40(%rax),%xmm1 */
    "62f1fd087bca",  /* vcvtpd2qq %xmm2,%xmm1 */
    "62f1fd287bca",  /* vcvtpd2qq %ymm2,%ymm1 */
    "62f1fc0879ca",  /* vcvtpd2udq %xmm2,%xmm1 */
    "62f1fc2879ca",  /* vcvtpd2udq %ymm2,%xmm1 */
    "62f1fc08794801",  /* vcvtpd2udqx 0x10(%rax),%xmm1 */
    "62f1fc28794801",  /* vcvtpd2udqy 0x20(%rax),%xmm1 */
    "62f1fd0879ca",  /* vcvtpd2uqq %xmm2,%xmm1 */
    "62f1fd2879ca",  /* vcvtpd2uqq %ymm2,%ymm1 */
    "62f57d085bca",  /* vcvtph2dq %xmm2,%xmm1 */
    "62f57d285bca",  /* vcvtph2dq %xmm2,%ymm1 */
    "62f57c085aca",  /* vcvtph2pd %xmm2,%xmm1 */
    "62f57c285aca",  /* vcvtph2pd %xmm2,%ymm1 */
    "62f67d0813ca",  /* vcvtph2psx %xmm2,%xmm1 */
    "62f67d2813ca",  /* vcvtph2psx %xmm2,%ymm1 */
    "62f57d087bca",  /* vcvtph2qq %xmm2,%xmm1 */
    "62f57d287bca",  /* vcvtph2qq %xmm2,%ymm1 */
    "62f57c0879ca",  /* vcvtph2udq %xmm2,%xmm1 */
    "62f57c2879ca",  /* vcvtph2udq %xmm2,%ymm1 */
    "62f57d0879ca",  /* vcvtph2uqq %xmm2,%xmm1 */
    "62f57d2879ca",  /* vcvtph2uqq %xmm2,%ymm1 */
    "62f57c087dca",  /* vcvtph2uw %xmm2,%xmm1 */
    "62f57c287dca",  /* vcvtph2uw %ymm2,%ymm1 */
    "62f57d087dca",  /* vcvtph2w %xmm2,%xmm1 */
    "62f57d287dca",  /* vcvtph2w %ymm2,%ymm1 */
    "62f57d081dca",  /* vcvtps2phx %xmm2,%xmm1 */
    "62f57d281dca",  /* vcvtps2phx %ymm2,%xmm1 */
    "62f57d081d4801",  /* vcvtps2phxx 0x10(%rax),%xmm1 */
    "62f57d281d4801",  /* vcvtps2phxy 0x20(%rax),%xmm1 */
    "62f17d087bca",  /* vcvtps2qq %xmm2,%xmm1 */
    "62f17d287bca",  /* vcvtps2qq %xmm2,%ymm1 */
    "62f17c0879ca",  /* vcvtps2udq %xmm2,%xmm1 */
    "62f17c2879ca",  /* vcvtps2udq %ymm2,%ymm1 */
    "62f17d0879ca",  /* vcvtps2uqq %xmm2,%xmm1 */
    "62f17d2879ca",  /* vcvtps2uqq %xmm2,%ymm1 */
    "62f1fe08e6ca",  /* vcvtqq2pd %xmm2,%xmm1 */
    "62f1fe28e6ca",  /* vcvtqq2pd %ymm2,%ymm1 */
    "62f5fc085bca",  /* vcvtqq2ph %xmm2,%xmm1 */
    "62f5fc285bca",  /* vcvtqq2ph %ymm2,%xmm1 */
    "62f5fc085b4801",  /* vcvtqq2phx 0x10(%rax),%xmm1 */
    "62f5fc285b4801",  /* vcvtqq2phy 0x20(%rax),%xmm1 */
    "62f5fc485b4801",  /* vcvtqq2phz 0x40(%rax),%xmm1 */
    "62f1fc085bca",  /* vcvtqq2ps %xmm2,%xmm1 */
    "62f1fc285bca",  /* vcvtqq2ps %ymm2,%xmm1 */
    "62f1fc085b4801",  /* vcvtqq2psx 0x10(%rax),%xmm1 */
    "62f1fc285b4801",  /* vcvtqq2psy 0x20(%rax),%xmm1 */
    "62f5ff085aca",  /* vcvtsd2sh %xmm2,%xmm0,%xmm1 */
    "62f5e7085aca",  /* vcvtsd2sh %xmm2,%xmm3,%xmm1 */
    "c5fb2dca",  /* vcvtsd2si %xmm2,%ecx */
    "c4e1fb2dca",  /* vcvtsd2si %xmm2,%rcx */
    "62f17f0879ca",  /* vcvtsd2usi %xmm2,%ecx */
    "62f1ff0879ca",  /* vcvtsd2usi %xmm2,%rcx */
    "62f57e085aca",  /* vcvtsh2sd %xmm2,%xmm0,%xmm1 */
    "62f566085aca",  /* vcvtsh2sd %xmm2,%xmm3,%xmm1 */
    "62f57e082dca",  /* vcvtsh2si %xmm2,%ecx */
    "62f5fe082dca",  /* vcvtsh2si %xmm2,%rcx */
    "62f67c0813ca",  /* vcvtsh2ss %xmm2,%xmm0,%xmm1 */
    "62f6640813ca",  /* vcvtsh2ss %xmm2,%xmm3,%xmm1 */
    "62f57e0879ca",  /* vcvtsh2usi %xmm2,%ecx */
    "62f5fe0879ca",  /* vcvtsh2usi %xmm2,%rcx */
    "c5fb2aca",  /* vcvtsi2sd %edx,%xmm0,%xmm1 */
    "c5e32aca",  /* vcvtsi2sd %edx,%xmm3,%xmm1 */
    "c5fb2a4801",  /* vcvtsi2sdl 0x1(%rax),%xmm0,%xmm1 */
    "c5e32a4801",  /* vcvtsi2sdl 0x1(%rax),%xmm3,%xmm1 */
    "c4e1fb2a4801",  /* vcvtsi2sdq 0x1(%rax),%xmm0,%xmm1 */
    "c4e1e32a4801",  /* vcvtsi2sdq 0x1(%rax),%xmm3,%xmm1 */
    "62f57e082aca",  /* vcvtsi2sh %edx,%xmm0,%xmm1 */
    "62f566082aca",  /* vcvtsi2sh %edx,%xmm3,%xmm1 */
    "62f57e082a4801",  /* vcvtsi2shl 0x4(%rax),%xmm0,%xmm1 */
    "62f566082a4801",  /* vcvtsi2shl 0x4(%rax),%xmm3,%xmm1 */
    "62f5fe082a4801",  /* vcvtsi2shq 0x8(%rax),%xmm0,%xmm1 */
    "62f5e6082a4801",  /* vcvtsi2shq 0x8(%rax),%xmm3,%xmm1 */
    "c5fa2aca",  /* vcvtsi2ss %edx,%xmm0,%xmm1 */
    "c5e22aca",  /* vcvtsi2ss %edx,%xmm3,%xmm1 */
    "c5fa2a4801",  /* vcvtsi2ssl 0x1(%rax),%xmm0,%xmm1 */
    "c5e22a4801",  /* vcvtsi2ssl 0x1(%rax),%xmm3,%xmm1 */
    "c4e1fa2a4801",  /* vcvtsi2ssq 0x1(%rax),%xmm0,%xmm1 */
    "c4e1e22a4801",  /* vcvtsi2ssq 0x1(%rax),%xmm3,%xmm1 */
    "62f57c081dca",  /* vcvtss2sh %xmm2,%xmm0,%xmm1 */
    "62f564081dca",  /* vcvtss2sh %xmm2,%xmm3,%xmm1 */
    "c5fa2dca",  /* vcvtss2si %xmm2,%ecx */
    "c4e1fa2dca",  /* vcvtss2si %xmm2,%rcx */
    "62f17e0879ca",  /* vcvtss2usi %xmm2,%ecx */
    "62f1fe0879ca",  /* vcvtss2usi %xmm2,%rcx */
    "c5f9e6ca",  /* vcvttpd2dq %xmm2,%xmm1 */
    "c5fde6ca",  /* vcvttpd2dq %ymm2,%xmm1 */
    "c5f9e64801",  /* vcvttpd2dqx 0x1(%rax),%xmm1 */
    "c5fde64801",  /* vcvttpd2dqy 0x1(%rax),%xmm1 */
    "62f1fd087aca",  /* vcvttpd2qq %xmm2,%xmm1 */
    "62f1fd287aca",  /* vcvttpd2qq %ymm2,%ymm1 */
    "62f1fc0878ca",  /* vcvttpd2udq %xmm2,%xmm1 */
    "62f1fc2878ca",  /* vcvttpd2udq %ymm2,%xmm1 */
    "62f1fc08784801",  /* vcvttpd2udqx 0x10(%rax),%xmm1 */
    "62f1fc28784801",  /* vcvttpd2udqy 0x20(%rax),%xmm1 */
    "62f1fd0878ca",  /* vcvttpd2uqq %xmm2,%xmm1 */
    "62f1fd2878ca",  /* vcvttpd2uqq %ymm2,%ymm1 */
    "62f57e085bca",  /* vcvttph2dq %xmm2,%xmm1 */
    "62f57e285bca",  /* vcvttph2dq %xmm2,%ymm1 */
    "62f57d087aca",  /* vcvttph2qq %xmm2,%xmm1 */
    "62f57d287aca",  /* vcvttph2qq %xmm2,%ymm1 */
    "62f57c0878ca",  /* vcvttph2udq %xmm2,%xmm1 */
    "62f57c2878ca",  /* vcvttph2udq %xmm2,%ymm1 */
    "62f57d0878ca",  /* vcvttph2uqq %xmm2,%xmm1 */
    "62f57d2878ca",  /* vcvttph2uqq %xmm2,%ymm1 */
    "62f57c087cca",  /* vcvttph2uw %xmm2,%xmm1 */
    "62f57c287cca",  /* vcvttph2uw %ymm2,%ymm1 */
    "62f57d087cca",  /* vcvttph2w %xmm2,%xmm1 */
    "62f57d287cca",  /* vcvttph2w %ymm2,%ymm1 */
    "62f17d087aca",  /* vcvttps2qq %xmm2,%xmm1 */
    "62f17d287aca",  /* vcvttps2qq %xmm2,%ymm1 */
    "62f17c0878ca",  /* vcvttps2udq %xmm2,%xmm1 */
    "62f17c2878ca",  /* vcvttps2udq %ymm2,%ymm1 */
    "62f17d0878ca",  /* vcvttps2uqq %xmm2,%xmm1 */
    "62f17d2878ca",  /* vcvttps2uqq %xmm2,%ymm1 */
    "c5fb2cca",  /* vcvttsd2si %xmm2,%ecx */
    "c4e1fb2cca",  /* vcvttsd2si %xmm2,%rcx */
    "62f17f0878ca",  /* vcvttsd2usi %xmm2,%ecx */
    "62f1ff0878ca",  /* vcvttsd2usi %xmm2,%rcx */
    "62f57e082cca",  /* vcvttsh2si %xmm2,%ecx */
    "62f5fe082cca",  /* vcvttsh2si %xmm2,%rcx */
    "62f57e0878ca",  /* vcvttsh2usi %xmm2,%ecx */
    "62f5fe0878ca",  /* vcvttsh2usi %xmm2,%rcx */
    "c5fa2cca",  /* vcvttss2si %xmm2,%ecx */
    "c4e1fa2cca",  /* vcvttss2si %xmm2,%rcx */
    "62f17e0878ca",  /* vcvttss2usi %xmm2,%ecx */
    "62f1fe0878ca",  /* vcvttss2usi %xmm2,%rcx */
    "62f17e087aca",  /* vcvtudq2pd %xmm2,%xmm1 */
    "62f17e287aca",  /* vcvtudq2pd %xmm2,%ymm1 */
    "62f57f087aca",  /* vcvtudq2ph %xmm2,%xmm1 */
    "62f57f287aca",  /* vcvtudq2ph %ymm2,%xmm1 */
    "62f57f087a4801",  /* vcvtudq2phx 0x10(%rax),%xmm1 */
    "62f57f287a4801",  /* vcvtudq2phy 0x20(%rax),%xmm1 */
    "62f17f087aca",  /* vcvtudq2ps %xmm2,%xmm1 */
    "62f17f287aca",  /* vcvtudq2ps %ymm2,%ymm1 */
    "62f1fe087aca",  /* vcvtuqq2pd %xmm2,%xmm1 */
    "62f1fe287aca",  /* vcvtuqq2pd %ymm2,%ymm1 */
    "62f5ff087aca",  /* vcvtuqq2ph %xmm2,%xmm1 */
    "62f5ff287aca",  /* vcvtuqq2ph %ymm2,%xmm1 */
    "62f5ff087a4801",  /* vcvtuqq2phx 0x10(%rax),%xmm1 */
    "62f5ff287a4801",  /* vcvtuqq2phy 0x20(%rax),%xmm1 */
    "62f5ff487a4801",  /* vcvtuqq2phz 0x40(%rax),%xmm1 */
    "62f1ff087aca",  /* vcvtuqq2ps %xmm2,%xmm1 */
    "62f1ff287aca",  /* vcvtuqq2ps %ymm2,%xmm1 */
    "62f1ff087a4801",  /* vcvtuqq2psx 0x10(%rax),%xmm1 */
    "62f1ff287a4801",  /* vcvtuqq2psy 0x20(%rax),%xmm1 */
    "62f17f087bca",  /* vcvtusi2sd %edx,%xmm0,%xmm1 */
    "62f167087bca",  /* vcvtusi2sd %edx,%xmm3,%xmm1 */
    "62f17f087b4801",  /* vcvtusi2sdl 0x4(%rax),%xmm0,%xmm1 */
    "62f167087b4801",  /* vcvtusi2sdl 0x4(%rax),%xmm3,%xmm1 */
    "62f1ff087b4801",  /* vcvtusi2sdq 0x8(%rax),%xmm0,%xmm1 */
    "62f1e7087b4801",  /* vcvtusi2sdq 0x8(%rax),%xmm3,%xmm1 */
    "62f57e087bca",  /* vcvtusi2sh %edx,%xmm0,%xmm1 */
    "62f566087bca",  /* vcvtusi2sh %edx,%xmm3,%xmm1 */
    "62f57e087b4801",  /* vcvtusi2shl 0x4(%rax),%xmm0,%xmm1 */
    "62f566087b4801",  /* vcvtusi2shl 0x4(%rax),%xmm3,%xmm1 */
    "62f5fe087b4801",  /* vcvtusi2shq 0x8(%rax),%xmm0,%xmm1 */
    "62f5e6087b4801",  /* vcvtusi2shq 0x8(%rax),%xmm3,%xmm1 */
    "62f17e087bca",  /* vcvtusi2ss %edx,%xmm0,%xmm1 */
    "62f166087bca",  /* vcvtusi2ss %edx,%xmm3,%xmm1 */
    "62f17e087b4801",  /* vcvtusi2ssl 0x4(%rax),%xmm0,%xmm1 */
    "62f166087b4801",  /* vcvtusi2ssl 0x4(%rax),%xmm3,%xmm1 */
    "62f1fe087b4801",  /* vcvtusi2ssq 0x8(%rax),%xmm0,%xmm1 */
    "62f1e6087b4801",  /* vcvtusi2ssq 0x8(%rax),%xmm3,%xmm1 */
    "62f57f087dca",  /* vcvtuw2ph %xmm2,%xmm1 */
    "62f57f287dca",  /* vcvtuw2ph %ymm2,%ymm1 */
    "62f57e087dca",  /* vcvtw2ph %xmm2,%xmm1 */
    "62f57e287dca",  /* vcvtw2ph %ymm2,%ymm1 */
    "62f37d0842ca47",  /* vdbpsadbw $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3650842ca47",  /* vdbpsadbw $0x47,%xmm2,%xmm3,%xmm1 */
    "62f57c085eca",  /* vdivph %xmm2,%xmm0,%xmm1 */
    "62f564085eca",  /* vdivph %xmm2,%xmm3,%xmm1 */
    "62f57e085eca",  /* vdivsh %xmm2,%xmm0,%xmm1 */
    "62f566085eca",  /* vdivsh %xmm2,%xmm3,%xmm1 */
    "62f27e0852ca",  /* vdpbf16ps %xmm2,%xmm0,%xmm1 */
    "62f2660852ca",  /* vdpbf16ps %xmm2,%xmm3,%xmm1 */
    "62f2fd48c8ca",  /* vexp2pd %zmm2,%zmm1 */
    "62f2fd48c84801",  /* vexp2pd 0x40(%rax),%zmm1 */
    "62f27d48c8ca",  /* vexp2ps %zmm2,%zmm1 */
    "62f27d48c84801",  /* vexp2ps 0x40(%rax),%zmm1 */
    "62f67f0856ca",  /* vfcmaddcph %xmm2,%xmm0,%xmm1 */
    "62f6670856ca",  /* vfcmaddcph %xmm2,%xmm3,%xmm1 */
    "62f67f0857ca",  /* vfcmaddcsh %xmm2,%xmm0,%xmm1 */
    "62f6670857ca",  /* vfcmaddcsh %xmm2,%xmm3,%xmm1 */
    "62f67f08d6ca",  /* vfcmulcph %xmm2,%xmm0,%xmm1 */
    "62f66708d6ca",  /* vfcmulcph %xmm2,%xmm3,%xmm1 */
    "62f67f08d7ca",  /* vfcmulcsh %xmm2,%xmm0,%xmm1 */
    "62f66708d7ca",  /* vfcmulcsh %xmm2,%xmm3,%xmm1 */
    "62f3fd0854ca47",  /* vfixupimmpd $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3e50854ca47",  /* vfixupimmpd $0x47,%xmm2,%xmm3,%xmm1 */
    "62f37d0854ca47",  /* vfixupimmps $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3650854ca47",  /* vfixupimmps $0x47,%xmm2,%xmm3,%xmm1 */
    "62f3fd0855ca47",  /* vfixupimmsd $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3e50855ca47",  /* vfixupimmsd $0x47,%xmm2,%xmm3,%xmm1 */
    "62f37d0855ca47",  /* vfixupimmss $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3650855ca47",  /* vfixupimmss $0x47,%xmm2,%xmm3,%xmm1 */
    "62f67d0898ca",  /* vfmadd132ph %xmm2,%xmm0,%xmm1 */
    "62f6650898ca",  /* vfmadd132ph %xmm2,%xmm3,%xmm1 */
    "62f67d0899ca",  /* vfmadd132sh %xmm2,%xmm0,%xmm1 */
    "62f6650899ca",  /* vfmadd132sh %xmm2,%xmm3,%xmm1 */
    "62f67d08a8ca",  /* vfmadd213ph %xmm2,%xmm0,%xmm1 */
    "62f66508a8ca",  /* vfmadd213ph %xmm2,%xmm3,%xmm1 */
    "62f67d08a9ca",  /* vfmadd213sh %xmm2,%xmm0,%xmm1 */
    "62f66508a9ca",  /* vfmadd213sh %xmm2,%xmm3,%xmm1 */
    "62f67d08b8ca",  /* vfmadd231ph %xmm2,%xmm0,%xmm1 */
    "62f66508b8ca",  /* vfmadd231ph %xmm2,%xmm3,%xmm1 */
    "62f67d08b9ca",  /* vfmadd231sh %xmm2,%xmm0,%xmm1 */
    "62f66508b9ca",  /* vfmadd231sh %xmm2,%xmm3,%xmm1 */
    "62f67e0856ca",  /* vfmaddcph %xmm2,%xmm0,%xmm1 */
    "62f6660856ca",  /* vfmaddcph %xmm2,%xmm3,%xmm1 */
    "62f67e0857ca",  /* vfmaddcsh %xmm2,%xmm0,%xmm1 */
    "62f6660857ca",  /* vfmaddcsh %xmm2,%xmm3,%xmm1 */
    "c4e3f969ca40",  /* vfmaddpd %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e169ca40",  /* vfmaddpd %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f968ca40",  /* vfmaddps %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e168ca40",  /* vfmaddps %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f96bca40",  /* vfmaddsd %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e16bca40",  /* vfmaddsd %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f96aca40",  /* vfmaddss %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e16aca40",  /* vfmaddss %xmm2,%xmm4,%xmm3,%xmm1 */
    "62f67d0896ca",  /* vfmaddsub132ph %xmm2,%xmm0,%xmm1 */
    "62f6650896ca",  /* vfmaddsub132ph %xmm2,%xmm3,%xmm1 */
    "62f67d08a6ca",  /* vfmaddsub213ph %xmm2,%xmm0,%xmm1 */
    "62f66508a6ca",  /* vfmaddsub213ph %xmm2,%xmm3,%xmm1 */
    "62f67d08b6ca",  /* vfmaddsub231ph %xmm2,%xmm0,%xmm1 */
    "62f66508b6ca",  /* vfmaddsub231ph %xmm2,%xmm3,%xmm1 */
    "c4e3f95dca40",  /* vfmaddsubpd %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e15dca40",  /* vfmaddsubpd %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f95cca40",  /* vfmaddsubps %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e15cca40",  /* vfmaddsubps %xmm2,%xmm4,%xmm3,%xmm1 */
    "62f67d089aca",  /* vfmsub132ph %xmm2,%xmm0,%xmm1 */
    "62f665089aca",  /* vfmsub132ph %xmm2,%xmm3,%xmm1 */
    "62f67d089bca",  /* vfmsub132sh %xmm2,%xmm0,%xmm1 */
    "62f665089bca",  /* vfmsub132sh %xmm2,%xmm3,%xmm1 */
    "62f67d08aaca",  /* vfmsub213ph %xmm2,%xmm0,%xmm1 */
    "62f66508aaca",  /* vfmsub213ph %xmm2,%xmm3,%xmm1 */
    "62f67d08abca",  /* vfmsub213sh %xmm2,%xmm0,%xmm1 */
    "62f66508abca",  /* vfmsub213sh %xmm2,%xmm3,%xmm1 */
    "62f67d08baca",  /* vfmsub231ph %xmm2,%xmm0,%xmm1 */
    "62f66508baca",  /* vfmsub231ph %xmm2,%xmm3,%xmm1 */
    "62f67d08bbca",  /* vfmsub231sh %xmm2,%xmm0,%xmm1 */
    "62f66508bbca",  /* vfmsub231sh %xmm2,%xmm3,%xmm1 */
    "62f67d0897ca",  /* vfmsubadd132ph %xmm2,%xmm0,%xmm1 */
    "62f6650897ca",  /* vfmsubadd132ph %xmm2,%xmm3,%xmm1 */
    "62f67d08a7ca",  /* vfmsubadd213ph %xmm2,%xmm0,%xmm1 */
    "62f66508a7ca",  /* vfmsubadd213ph %xmm2,%xmm3,%xmm1 */
    "62f67d08b7ca",  /* vfmsubadd231ph %xmm2,%xmm0,%xmm1 */
    "62f66508b7ca",  /* vfmsubadd231ph %xmm2,%xmm3,%xmm1 */
    "c4e3f95fca40",  /* vfmsubaddpd %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e15fca40",  /* vfmsubaddpd %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f95eca40",  /* vfmsubaddps %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e15eca40",  /* vfmsubaddps %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f96dca40",  /* vfmsubpd %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e16dca40",  /* vfmsubpd %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f96cca40",  /* vfmsubps %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e16cca40",  /* vfmsubps %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f96fca40",  /* vfmsubsd %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e16fca40",  /* vfmsubsd %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f96eca40",  /* vfmsubss %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e16eca40",  /* vfmsubss %xmm2,%xmm4,%xmm3,%xmm1 */
    "62f67e08d6ca",  /* vfmulcph %xmm2,%xmm0,%xmm1 */
    "62f66608d6ca",  /* vfmulcph %xmm2,%xmm3,%xmm1 */
    "62f67e08d7ca",  /* vfmulcsh %xmm2,%xmm0,%xmm1 */
    "62f66608d7ca",  /* vfmulcsh %xmm2,%xmm3,%xmm1 */
    "62f67d089cca",  /* vfnmadd132ph %xmm2,%xmm0,%xmm1 */
    "62f665089cca",  /* vfnmadd132ph %xmm2,%xmm3,%xmm1 */
    "62f67d089dca",  /* vfnmadd132sh %xmm2,%xmm0,%xmm1 */
    "62f665089dca",  /* vfnmadd132sh %xmm2,%xmm3,%xmm1 */
    "62f67d08acca",  /* vfnmadd213ph %xmm2,%xmm0,%xmm1 */
    "62f66508acca",  /* vfnmadd213ph %xmm2,%xmm3,%xmm1 */
    "62f67d08adca",  /* vfnmadd213sh %xmm2,%xmm0,%xmm1 */
    "62f66508adca",  /* vfnmadd213sh %xmm2,%xmm3,%xmm1 */
    "62f67d08bcca",  /* vfnmadd231ph %xmm2,%xmm0,%xmm1 */
    "62f66508bcca",  /* vfnmadd231ph %xmm2,%xmm3,%xmm1 */
    "62f67d08bdca",  /* vfnmadd231sh %xmm2,%xmm0,%xmm1 */
    "62f66508bdca",  /* vfnmadd231sh %xmm2,%xmm3,%xmm1 */
    "c4e3f979ca40",  /* vfnmaddpd %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e179ca40",  /* vfnmaddpd %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f978ca40",  /* vfnmaddps %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e178ca40",  /* vfnmaddps %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f97bca40",  /* vfnmaddsd %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e17bca40",  /* vfnmaddsd %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f97aca40",  /* vfnmaddss %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e17aca40",  /* vfnmaddss %xmm2,%xmm4,%xmm3,%xmm1 */
    "62f67d089eca",  /* vfnmsub132ph %xmm2,%xmm0,%xmm1 */
    "62f665089eca",  /* vfnmsub132ph %xmm2,%xmm3,%xmm1 */
    "62f67d089fca",  /* vfnmsub132sh %xmm2,%xmm0,%xmm1 */
    "62f665089fca",  /* vfnmsub132sh %xmm2,%xmm3,%xmm1 */
    "62f67d08aeca",  /* vfnmsub213ph %xmm2,%xmm0,%xmm1 */
    "62f66508aeca",  /* vfnmsub213ph %xmm2,%xmm3,%xmm1 */
    "62f67d08afca",  /* vfnmsub213sh %xmm2,%xmm0,%xmm1 */
    "62f66508afca",  /* vfnmsub213sh %xmm2,%xmm3,%xmm1 */
    "62f67d08beca",  /* vfnmsub231ph %xmm2,%xmm0,%xmm1 */
    "62f66508beca",  /* vfnmsub231ph %xmm2,%xmm3,%xmm1 */
    "62f67d08bfca",  /* vfnmsub231sh %xmm2,%xmm0,%xmm1 */
    "62f66508bfca",  /* vfnmsub231sh %xmm2,%xmm3,%xmm1 */
    "c4e3f97dca40",  /* vfnmsubpd %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e17dca40",  /* vfnmsubpd %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f97cca40",  /* vfnmsubps %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e17cca40",  /* vfnmsubps %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f97fca40",  /* vfnmsubsd %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e17fca40",  /* vfnmsubsd %xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e3f97eca40",  /* vfnmsubss %xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e3e17eca40",  /* vfnmsubss %xmm2,%xmm4,%xmm3,%xmm1 */
    "62f3fd0866ca47",  /* vfpclasspd $0x47,%xmm2,%k1 */
    "62f3fd2866ca47",  /* vfpclasspd $0x47,%ymm2,%k1 */
    "62f3fd0866480147",  /* vfpclasspdx $0x47,0x10(%rax),%k1 */
    "62f3fd2866480147",  /* vfpclasspdy $0x47,0x20(%rax),%k1 */
    "62f3fd4866480147",  /* vfpclasspdz $0x47,0x40(%rax),%k1 */
    "62f37c0866ca47",  /* vfpclassph $0x47,%xmm2,%k1 */
    "62f37c2866ca47",  /* vfpclassph $0x47,%ymm2,%k1 */
    "62f37c0866480147",  /* vfpclassphx $0x47,0x10(%rax),%k1 */
    "62f37c2866480147",  /* vfpclassphy $0x47,0x20(%rax),%k1 */
    "62f37c4866480147",  /* vfpclassphz $0x47,0x40(%rax),%k1 */
    "62f37d0866ca47",  /* vfpclassps $0x47,%xmm2,%k1 */
    "62f37d2866ca47",  /* vfpclassps $0x47,%ymm2,%k1 */
    "62f37d0866480147",  /* vfpclasspsx $0x47,0x10(%rax),%k1 */
    "62f37d2866480147",  /* vfpclasspsy $0x47,0x20(%rax),%k1 */
    "62f37d4866480147",  /* vfpclasspsz $0x47,0x40(%rax),%k1 */
    "62f3fd0867ca47",  /* vfpclasssd $0x47,%xmm2,%k1 */
    "62f3fd0867480147",  /* vfpclasssd $0x47,0x8(%rax),%k1 */
    "62f37c0867ca47",  /* vfpclasssh $0x47,%xmm2,%k1 */
    "62f37c0867480147",  /* vfpclasssh $0x47,0x2(%rax),%k1 */
    "62f37d0867ca47",  /* vfpclassss $0x47,%xmm2,%k1 */
    "62f37d0867480147",  /* vfpclassss $0x47,0x4(%rax),%k1 */
    "62f2fd0842ca",  /* vgetexppd %xmm2,%xmm1 */
    "62f2fd2842ca",  /* vgetexppd %ymm2,%ymm1 */
    "62f67d0842ca",  /* vgetexpph %xmm2,%xmm1 */
    "62f67d2842ca",  /* vgetexpph %ymm2,%ymm1 */
    "62f27d0842ca",  /* vgetexpps %xmm2,%xmm1 */
    "62f27d2842ca",  /* vgetexpps %ymm2,%ymm1 */
    "62f2fd0843ca",  /* vgetexpsd %xmm2,%xmm0,%xmm1 */
    "62f2e50843ca",  /* vgetexpsd %xmm2,%xmm3,%xmm1 */
    "62f67d0843ca",  /* vgetexpsh %xmm2,%xmm0,%xmm1 */
    "62f6650843ca",  /* vgetexpsh %xmm2,%xmm3,%xmm1 */
    "62f27d0843ca",  /* vgetexpss %xmm2,%xmm0,%xmm1 */
    "62f2650843ca",  /* vgetexpss %xmm2,%xmm3,%xmm1 */
    "62f3fd0826ca47",  /* vgetmantpd $0x47,%xmm2,%xmm1 */
    "62f3fd2826ca47",  /* vgetmantpd $0x47,%ymm2,%ymm1 */
    "62f37c0826ca47",  /* vgetmantph $0x47,%xmm2,%xmm1 */
    "62f37c2826ca47",  /* vgetmantph $0x47,%ymm2,%ymm1 */
    "62f37d0826ca47",  /* vgetmantps $0x47,%xmm2,%xmm1 */
    "62f37d2826ca47",  /* vgetmantps $0x47,%ymm2,%ymm1 */
    "62f3fd0827ca47",  /* vgetmantsd $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3e50827ca47",  /* vgetmantsd $0x47,%xmm2,%xmm3,%xmm1 */
    "62f37c0827ca47",  /* vgetmantsh $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3640827ca47",  /* vgetmantsh $0x47,%xmm2,%xmm3,%xmm1 */
    "62f37d0827ca47",  /* vgetmantss $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3650827ca47",  /* vgetmantss $0x47,%xmm2,%xmm3,%xmm1 */
    "c5f9f7ca",  /* vmaskmovdqu %xmm2,%xmm1 */
    "62f57c085fca",  /* vmaxph %xmm2,%xmm0,%xmm1 */
    "62f564085fca",  /* vmaxph %xmm2,%xmm3,%xmm1 */
    "62f57e085fca",  /* vmaxsh %xmm2,%xmm0,%xmm1 */
    "62f566085fca",  /* vmaxsh %xmm2,%xmm3,%xmm1 */
    "62f57c085dca",  /* vminph %xmm2,%xmm0,%xmm1 */
    "62f564085dca",  /* vminph %xmm2,%xmm3,%xmm1 */
    "62f57e085dca",  /* vminsh %xmm2,%xmm0,%xmm1 */
    "62f566085dca",  /* vminsh %xmm2,%xmm3,%xmm1 */
    "c5fb12ca",  /* vmovddup %xmm2,%xmm1 */
    "c5ff12ca",  /* vmovddup %ymm2,%ymm1 */
    "c5f9e74801",  /* vmovntdq %xmm1,0x1(%rax) */
    "c5fde74801",  /* vmovntdq %ymm1,0x1(%rax) */
    "c4e2792a4801",  /* vmovntdqa 0x1(%rax),%xmm1 */
    "c4e27d2a4801",  /* vmovntdqa 0x1(%rax),%ymm1 */
    "c5f92b4801",  /* vmovntpd %xmm1,0x1(%rax) */
    "c5fd2b4801",  /* vmovntpd %ymm1,0x1(%rax) */
    "c5f82b4801",  /* vmovntps %xmm1,0x1(%rax) */
    "c5fc2b4801",  /* vmovntps %ymm1,0x1(%rax) */
    "c5fb10d1",  /* vmovsd %xmm1,%xmm0,%xmm2 */
    "c5e310d1",  /* vmovsd %xmm1,%xmm3,%xmm2 */
    "62f57e0810d1",  /* vmovsh %xmm1,%xmm0,%xmm2 */
    "62f5660810d1",  /* vmovsh %xmm1,%xmm3,%xmm2 */
    "c5fa16ca",  /* vmovshdup %xmm2,%xmm1 */
    "c5fe16ca",  /* vmovshdup %ymm2,%ymm1 */
    "c5fa12ca",  /* vmovsldup %xmm2,%xmm1 */
    "c5fe12ca",  /* vmovsldup %ymm2,%ymm1 */
    "c5fa10d1",  /* vmovss %xmm1,%xmm0,%xmm2 */
    "c5e210d1",  /* vmovss %xmm1,%xmm3,%xmm2 */
    "62f57d086eca",  /* vmovw %edx,%xmm1 */
    "62f57d087eca",  /* vmovw %xmm1,%edx */
    "62f57c0859ca",  /* vmulph %xmm2,%xmm0,%xmm1 */
    "62f5640859ca",  /* vmulph %xmm2,%xmm3,%xmm1 */
    "62f57e0859ca",  /* vmulsh %xmm2,%xmm0,%xmm1 */
    "62f5660859ca",  /* vmulsh %xmm2,%xmm3,%xmm1 */
    "62f27f0868ca",  /* vp2intersectd %xmm2,%xmm0,%k1 */
    "62f2670868ca",  /* vp2intersectd %xmm2,%xmm3,%k1 */
    "62f2ff0868ca",  /* vp2intersectq %xmm2,%xmm0,%k1 */
    "62f2e70868ca",  /* vp2intersectq %xmm2,%xmm3,%k1 */
    "62f27f48524801",  /* vp4dpwssd 0x10(%rax),%zmm0,%zmm1 */
    "62f26748524801",  /* vp4dpwssd 0x10(%rax),%zmm3,%zmm1 */
    "62f27f48534801",  /* vp4dpwssds 0x10(%rax),%zmm0,%zmm1 */
    "62f26748534801",  /* vp4dpwssds 0x10(%rax),%zmm3,%zmm1 */
    "c5f96bca",  /* vpackssdw %xmm2,%xmm0,%xmm1 */
    "c5e16bca",  /* vpackssdw %xmm2,%xmm3,%xmm1 */
    "62f27d0866ca",  /* vpblendmb %xmm2,%xmm0,%xmm1 */
    "62f2650866ca",  /* vpblendmb %xmm2,%xmm3,%xmm1 */
    "62f27d0864ca",  /* vpblendmd %xmm2,%xmm0,%xmm1 */
    "62f2650864ca",  /* vpblendmd %xmm2,%xmm3,%xmm1 */
    "62f2fd0864ca",  /* vpblendmq %xmm2,%xmm0,%xmm1 */
    "62f2e50864ca",  /* vpblendmq %xmm2,%xmm3,%xmm1 */
    "62f2fd0866ca",  /* vpblendmw %xmm2,%xmm0,%xmm1 */
    "62f2e50866ca",  /* vpblendmw %xmm2,%xmm3,%xmm1 */
    "c4e3f961ca47",  /* vpcmpestriq $0x47,%xmm2,%xmm1 */
    "c4e3f961480147",  /* vpcmpestriq $0x47,0x1(%rax),%xmm1 */
    "c4e3f960ca47",  /* vpcmpestrmq $0x47,%xmm2,%xmm1 */
    "c4e3f960480147",  /* vpcmpestrmq $0x47,0x1(%rax),%xmm1 */
    "62f27d0863ca",  /* vpcompressb %xmm1,%xmm2 */
    "62f27d08634801",  /* vpcompressb %xmm1,0x1(%rax) */
    "62f2fd0863ca",  /* vpcompressw %xmm1,%xmm2 */
    "62f2fd08634801",  /* vpcompressw %xmm1,0x2(%rax) */
    "c4e27b50ca",  /* vpdpbssd %xmm2,%xmm0,%xmm1 */
    "c4e26350ca",  /* vpdpbssd %xmm2,%xmm3,%xmm1 */
    "c4e27b51ca",  /* vpdpbssds %xmm2,%xmm0,%xmm1 */
    "c4e26351ca",  /* vpdpbssds %xmm2,%xmm3,%xmm1 */
    "c4e27a50ca",  /* vpdpbsud %xmm2,%xmm0,%xmm1 */
    "c4e26250ca",  /* vpdpbsud %xmm2,%xmm3,%xmm1 */
    "c4e27a51ca",  /* vpdpbsuds %xmm2,%xmm0,%xmm1 */
    "c4e26251ca",  /* vpdpbsuds %xmm2,%xmm3,%xmm1 */
    "c4e27850ca",  /* vpdpbuud %xmm2,%xmm0,%xmm1 */
    "c4e26050ca",  /* vpdpbuud %xmm2,%xmm3,%xmm1 */
    "c4e27851ca",  /* vpdpbuuds %xmm2,%xmm0,%xmm1 */
    "c4e26051ca",  /* vpdpbuuds %xmm2,%xmm3,%xmm1 */
    "c4e27ad2ca",  /* vpdpwsud %xmm2,%xmm0,%xmm1 */
    "c4e262d2ca",  /* vpdpwsud %xmm2,%xmm3,%xmm1 */
    "c4e27ad3ca",  /* vpdpwsuds %xmm2,%xmm0,%xmm1 */
    "c4e262d3ca",  /* vpdpwsuds %xmm2,%xmm3,%xmm1 */
    "c4e279d2ca",  /* vpdpwusd %xmm2,%xmm0,%xmm1 */
    "c4e261d2ca",  /* vpdpwusd %xmm2,%xmm3,%xmm1 */
    "c4e279d3ca",  /* vpdpwusds %xmm2,%xmm0,%xmm1 */
    "c4e261d3ca",  /* vpdpwusds %xmm2,%xmm3,%xmm1 */
    "c4e278d2ca",  /* vpdpwuud %xmm2,%xmm0,%xmm1 */
    "c4e260d2ca",  /* vpdpwuud %xmm2,%xmm3,%xmm1 */
    "c4e278d3ca",  /* vpdpwuuds %xmm2,%xmm0,%xmm1 */
    "c4e260d3ca",  /* vpdpwuuds %xmm2,%xmm3,%xmm1 */
    "c4e37949cc27",  /* vpermil2pd $0x7,%xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e36149cc27",  /* vpermil2pd $0x7,%xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e37948cc27",  /* vpermil2ps $0x7,%xmm2,%xmm4,%xmm0,%xmm1 */
    "c4e36148cc27",  /* vpermil2ps $0x7,%xmm2,%xmm4,%xmm3,%xmm1 */
    "c4e37905ca47",  /* vpermilpd $0x47,%xmm2,%xmm1 */
    "c4e37d05ca47",  /* vpermilpd $0x47,%ymm2,%ymm1 */
    "c4e37904ca47",  /* vpermilps $0x47,%xmm2,%xmm1 */
    "c4e37d04ca47",  /* vpermilps $0x47,%ymm2,%ymm1 */
    "62f27d0862ca",  /* vpexpandb %xmm2,%xmm1 */
    "62f27d2862ca",  /* vpexpandb %ymm2,%ymm1 */
    "62f2fd0862ca",  /* vpexpandw %xmm2,%xmm1 */
    "62f2fd2862ca",  /* vpexpandw %ymm2,%ymm1 */
    "c5f9c5d147",  /* vpextrw $0x47,%xmm1,%edx */
    "c4e37915480147",  /* vpextrw $0x47,%xmm1,0x1(%rax) */
    "62f2fd083dca",  /* vpmaxsq %xmm2,%xmm0,%xmm1 */
    "62f2e5083dca",  /* vpmaxsq %xmm2,%xmm3,%xmm1 */
    "62f2fd083fca",  /* vpmaxuq %xmm2,%xmm0,%xmm1 */
    "62f2e5083fca",  /* vpmaxuq %xmm2,%xmm3,%xmm1 */
    "62f2fd0839ca",  /* vpminsq %xmm2,%xmm0,%xmm1 */
    "62f2e50839ca",  /* vpminsq %xmm2,%xmm3,%xmm1 */
    "62f2fd083bca",  /* vpminuq %xmm2,%xmm0,%xmm1 */
    "62f2e5083bca",  /* vpminuq %xmm2,%xmm3,%xmm1 */
    "62f27e0829ca",  /* vpmovb2m %xmm2,%k1 */
    "62f27e2829ca",  /* vpmovb2m %ymm2,%k1 */
    "62f27e0839ca",  /* vpmovd2m %xmm2,%k1 */
    "62f27e2839ca",  /* vpmovd2m %ymm2,%k1 */
    "62f27e0828ca",  /* vpmovm2b %k2,%xmm1 */
    "62f27e2828ca",  /* vpmovm2b %k2,%ymm1 */
    "62f27e0838ca",  /* vpmovm2d %k2,%xmm1 */
    "62f27e2838ca",  /* vpmovm2d %k2,%ymm1 */
    "62f2fe0838ca",  /* vpmovm2q %k2,%xmm1 */
    "62f2fe2838ca",  /* vpmovm2q %k2,%ymm1 */
    "62f2fe0828ca",  /* vpmovm2w %k2,%xmm1 */
    "62f2fe2828ca",  /* vpmovm2w %k2,%ymm1 */
    "62f2fe0839ca",  /* vpmovq2m %xmm2,%k1 */
    "62f2fe2839ca",  /* vpmovq2m %ymm2,%k1 */
    "c4e27921ca",  /* vpmovsxbd %xmm2,%xmm1 */
    "c4e27d21ca",  /* vpmovsxbd %xmm2,%ymm1 */
    "c4e27922ca",  /* vpmovsxbq %xmm2,%xmm1 */
    "c4e27d22ca",  /* vpmovsxbq %xmm2,%ymm1 */
    "c4e27925ca",  /* vpmovsxdq %xmm2,%xmm1 */
    "c4e27d25ca",  /* vpmovsxdq %xmm2,%ymm1 */
    "c4e27923ca",  /* vpmovsxwd %xmm2,%xmm1 */
    "c4e27d23ca",  /* vpmovsxwd %xmm2,%ymm1 */
    "c4e27924ca",  /* vpmovsxwq %xmm2,%xmm1 */
    "c4e27d24ca",  /* vpmovsxwq %xmm2,%ymm1 */
    "62f2fe0829ca",  /* vpmovw2m %xmm2,%k1 */
    "62f2fe2829ca",  /* vpmovw2m %ymm2,%k1 */
    "c4e27931ca",  /* vpmovzxbd %xmm2,%xmm1 */
    "c4e27d31ca",  /* vpmovzxbd %xmm2,%ymm1 */
    "c4e27932ca",  /* vpmovzxbq %xmm2,%xmm1 */
    "c4e27d32ca",  /* vpmovzxbq %xmm2,%ymm1 */
    "c4e27935ca",  /* vpmovzxdq %xmm2,%xmm1 */
    "c4e27d35ca",  /* vpmovzxdq %xmm2,%ymm1 */
    "c4e27933ca",  /* vpmovzxwd %xmm2,%xmm1 */
    "c4e27d33ca",  /* vpmovzxwd %xmm2,%ymm1 */
    "c4e27934ca",  /* vpmovzxwq %xmm2,%xmm1 */
    "c4e27d34ca",  /* vpmovzxwq %xmm2,%ymm1 */
    "62f2fd0840ca",  /* vpmullq %xmm2,%xmm0,%xmm1 */
    "62f2e50840ca",  /* vpmullq %xmm2,%xmm3,%xmm1 */
    "62f17d0872ca47",  /* vprold $0x47,%xmm2,%xmm0 */
    "62f1650872ca47",  /* vprold $0x47,%xmm2,%xmm3 */
    "62f1fd0872ca47",  /* vprolq $0x47,%xmm2,%xmm0 */
    "62f1e50872ca47",  /* vprolq $0x47,%xmm2,%xmm3 */
    "62f27d0815ca",  /* vprolvd %xmm2,%xmm0,%xmm1 */
    "62f2650815ca",  /* vprolvd %xmm2,%xmm3,%xmm1 */
    "62f2fd0815ca",  /* vprolvq %xmm2,%xmm0,%xmm1 */
    "62f2e50815ca",  /* vprolvq %xmm2,%xmm3,%xmm1 */
    "62f27d0814ca",  /* vprorvd %xmm2,%xmm0,%xmm1 */
    "62f2650814ca",  /* vprorvd %xmm2,%xmm3,%xmm1 */
    "62f2fd0814ca",  /* vprorvq %xmm2,%xmm0,%xmm1 */
    "62f2e50814ca",  /* vprorvq %xmm2,%xmm3,%xmm1 */
    "62f37d0871ca47",  /* vpshldd $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3650871ca47",  /* vpshldd $0x47,%xmm2,%xmm3,%xmm1 */
    "62f3fd0871ca47",  /* vpshldq $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3e50871ca47",  /* vpshldq $0x47,%xmm2,%xmm3,%xmm1 */
    "62f27d0871ca",  /* vpshldvd %xmm2,%xmm0,%xmm1 */
    "62f2650871ca",  /* vpshldvd %xmm2,%xmm3,%xmm1 */
    "62f2fd0871ca",  /* vpshldvq %xmm2,%xmm0,%xmm1 */
    "62f2e50871ca",  /* vpshldvq %xmm2,%xmm3,%xmm1 */
    "62f2fd0870ca",  /* vpshldvw %xmm2,%xmm0,%xmm1 */
    "62f2e50870ca",  /* vpshldvw %xmm2,%xmm3,%xmm1 */
    "62f3fd0870ca47",  /* vpshldw $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3e50870ca47",  /* vpshldw $0x47,%xmm2,%xmm3,%xmm1 */
    "62f37d0873ca47",  /* vpshrdd $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3650873ca47",  /* vpshrdd $0x47,%xmm2,%xmm3,%xmm1 */
    "62f3fd0873ca47",  /* vpshrdq $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3e50873ca47",  /* vpshrdq $0x47,%xmm2,%xmm3,%xmm1 */
    "62f27d0873ca",  /* vpshrdvd %xmm2,%xmm0,%xmm1 */
    "62f2650873ca",  /* vpshrdvd %xmm2,%xmm3,%xmm1 */
    "62f2fd0873ca",  /* vpshrdvq %xmm2,%xmm0,%xmm1 */
    "62f2e50873ca",  /* vpshrdvq %xmm2,%xmm3,%xmm1 */
    "62f2fd0872ca",  /* vpshrdvw %xmm2,%xmm0,%xmm1 */
    "62f2e50872ca",  /* vpshrdvw %xmm2,%xmm3,%xmm1 */
    "62f3fd0872ca47",  /* vpshrdw $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3e50872ca47",  /* vpshrdw $0x47,%xmm2,%xmm3,%xmm1 */
    "c5fa70ca47",  /* vpshufhw $0x47,%xmm2,%xmm1 */
    "c5fe70ca47",  /* vpshufhw $0x47,%ymm2,%ymm1 */
    "c5fb70ca47",  /* vpshuflw $0x47,%xmm2,%xmm1 */
    "c5ff70ca47",  /* vpshuflw $0x47,%ymm2,%ymm1 */
    "c5f9f2ca",  /* vpslld %xmm2,%xmm0,%xmm1 */
    "c5e1f2ca",  /* vpslld %xmm2,%xmm3,%xmm1 */
    "c5f9f3ca",  /* vpsllq %xmm2,%xmm0,%xmm1 */
    "c5e1f3ca",  /* vpsllq %xmm2,%xmm3,%xmm1 */
    "62f2fd0812ca",  /* vpsllvw %xmm2,%xmm0,%xmm1 */
    "62f2e50812ca",  /* vpsllvw %xmm2,%xmm3,%xmm1 */
    "c5f9f1ca",  /* vpsllw %xmm2,%xmm0,%xmm1 */
    "c5e1f1ca",  /* vpsllw %xmm2,%xmm3,%xmm1 */
    "c5f9e2ca",  /* vpsrad %xmm2,%xmm0,%xmm1 */
    "c5e1e2ca",  /* vpsrad %xmm2,%xmm3,%xmm1 */
    "62f1fd08e2ca",  /* vpsraq %xmm2,%xmm0,%xmm1 */
    "62f1e508e2ca",  /* vpsraq %xmm2,%xmm3,%xmm1 */
    "62f2fd0811ca",  /* vpsravw %xmm2,%xmm0,%xmm1 */
    "62f2e50811ca",  /* vpsravw %xmm2,%xmm3,%xmm1 */
    "c5f9e1ca",  /* vpsraw %xmm2,%xmm0,%xmm1 */
    "c5e1e1ca",  /* vpsraw %xmm2,%xmm3,%xmm1 */
    "c5f9d2ca",  /* vpsrld %xmm2,%xmm0,%xmm1 */
    "c5e1d2ca",  /* vpsrld %xmm2,%xmm3,%xmm1 */
    "c5f9d3ca",  /* vpsrlq %xmm2,%xmm0,%xmm1 */
    "c5e1d3ca",  /* vpsrlq %xmm2,%xmm3,%xmm1 */
    "62f2fd0810ca",  /* vpsrlvw %xmm2,%xmm0,%xmm1 */
    "62f2e50810ca",  /* vpsrlvw %xmm2,%xmm3,%xmm1 */
    "c5f9d1ca",  /* vpsrlw %xmm2,%xmm0,%xmm1 */
    "c5e1d1ca",  /* vpsrlw %xmm2,%xmm3,%xmm1 */
    "62f27d0826ca",  /* vptestmb %xmm2,%xmm0,%k1 */
    "62f2650826ca",  /* vptestmb %xmm2,%xmm3,%k1 */
    "62f27d0827ca",  /* vptestmd %xmm2,%xmm0,%k1 */
    "62f2650827ca",  /* vptestmd %xmm2,%xmm3,%k1 */
    "62f2fd0827ca",  /* vptestmq %xmm2,%xmm0,%k1 */
    "62f2e50827ca",  /* vptestmq %xmm2,%xmm3,%k1 */
    "62f2fd0826ca",  /* vptestmw %xmm2,%xmm0,%k1 */
    "62f2e50826ca",  /* vptestmw %xmm2,%xmm3,%k1 */
    "62f27e0826ca",  /* vptestnmb %xmm2,%xmm0,%k1 */
    "62f2660826ca",  /* vptestnmb %xmm2,%xmm3,%k1 */
    "62f27e0827ca",  /* vptestnmd %xmm2,%xmm0,%k1 */
    "62f2660827ca",  /* vptestnmd %xmm2,%xmm3,%k1 */
    "62f2fe0827ca",  /* vptestnmq %xmm2,%xmm0,%k1 */
    "62f2e60827ca",  /* vptestnmq %xmm2,%xmm3,%k1 */
    "62f2fe0826ca",  /* vptestnmw %xmm2,%xmm0,%k1 */
    "62f2e60826ca",  /* vptestnmw %xmm2,%xmm3,%k1 */
    "62f3fd0850ca47",  /* vrangepd $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3e50850ca47",  /* vrangepd $0x47,%xmm2,%xmm3,%xmm1 */
    "62f37d0850ca47",  /* vrangeps $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3650850ca47",  /* vrangeps $0x47,%xmm2,%xmm3,%xmm1 */
    "62f3fd0851ca47",  /* vrangesd $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3e50851ca47",  /* vrangesd $0x47,%xmm2,%xmm3,%xmm1 */
    "62f37d0851ca47",  /* vrangess $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3650851ca47",  /* vrangess $0x47,%xmm2,%xmm3,%xmm1 */
    "62f2fd084cca",  /* vrcp14pd %xmm2,%xmm1 */
    "62f2fd284cca",  /* vrcp14pd %ymm2,%ymm1 */
    "62f27d084cca",  /* vrcp14ps %xmm2,%xmm1 */
    "62f27d284cca",  /* vrcp14ps %ymm2,%ymm1 */
    "62f2fd084dca",  /* vrcp14sd %xmm2,%xmm0,%xmm1 */
    "62f2e5084dca",  /* vrcp14sd %xmm2,%xmm3,%xmm1 */
    "62f27d084dca",  /* vrcp14ss %xmm2,%xmm0,%xmm1 */
    "62f265084dca",  /* vrcp14ss %xmm2,%xmm3,%xmm1 */
    "62f2fd48caca",  /* vrcp28pd %zmm2,%zmm1 */
    "62f2fd48ca4801",  /* vrcp28pd 0x40(%rax),%zmm1 */
    "62f27d48caca",  /* vrcp28ps %zmm2,%zmm1 */
    "62f27d48ca4801",  /* vrcp28ps 0x40(%rax),%zmm1 */
    "62f2fd08cbca",  /* vrcp28sd %xmm2,%xmm0,%xmm1 */
    "62f2e508cbca",  /* vrcp28sd %xmm2,%xmm3,%xmm1 */
    "62f27d08cbca",  /* vrcp28ss %xmm2,%xmm0,%xmm1 */
    "62f26508cbca",  /* vrcp28ss %xmm2,%xmm3,%xmm1 */
    "62f67d084cca",  /* vrcpph %xmm2,%xmm1 */
    "62f67d284cca",  /* vrcpph %ymm2,%ymm1 */
    "62f67d084dca",  /* vrcpsh %xmm2,%xmm0,%xmm1 */
    "62f665084dca",  /* vrcpsh %xmm2,%xmm3,%xmm1 */
    "62f3fd0856ca47",  /* vreducepd $0x47,%xmm2,%xmm1 */
    "62f3fd2856ca47",  /* vreducepd $0x47,%ymm2,%ymm1 */
    "62f37c0856ca47",  /* vreduceph $0x47,%xmm2,%xmm1 */
    "62f37c2856ca47",  /* vreduceph $0x47,%ymm2,%ymm1 */
    "62f37d0856ca47",  /* vreduceps $0x47,%xmm2,%xmm1 */
    "62f37d2856ca47",  /* vreduceps $0x47,%ymm2,%ymm1 */
    "62f3fd0857ca47",  /* vreducesd $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3e50857ca47",  /* vreducesd $0x47,%xmm2,%xmm3,%xmm1 */
    "62f37c0857ca47",  /* vreducesh $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3640857ca47",  /* vreducesh $0x47,%xmm2,%xmm3,%xmm1 */
    "62f37d0857ca47",  /* vreducess $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3650857ca47",  /* vreducess $0x47,%xmm2,%xmm3,%xmm1 */
    "62f3fd0809ca47",  /* vrndscalepd $0x47,%xmm2,%xmm1 */
    "62f3fd2809ca47",  /* vrndscalepd $0x47,%ymm2,%ymm1 */
    "62f37c0808ca47",  /* vrndscaleph $0x47,%xmm2,%xmm1 */
    "62f37c2808ca47",  /* vrndscaleph $0x47,%ymm2,%ymm1 */
    "62f37d0808ca47",  /* vrndscaleps $0x47,%xmm2,%xmm1 */
    "62f37d2808ca47",  /* vrndscaleps $0x47,%ymm2,%ymm1 */
    "62f3fd080bca47",  /* vrndscalesd $0x47,%xmm2,%xmm0,%xmm1 */
    "62f3e5080bca47",  /* vrndscalesd $0x47,%xmm2,%xmm3,%xmm1 */
    "62f37c080aca47",  /* vrndscalesh $0x47,%xmm2,%xmm0,%xmm1 */
    "62f364080aca47",  /* vrndscalesh $0x47,%xmm2,%xmm3,%xmm1 */
    "62f37d080aca47",  /* vrndscaless $0x47,%xmm2,%xmm0,%xmm1 */
    "62f365080aca47",  /* vrndscaless $0x47,%xmm2,%xmm3,%xmm1 */
    "62f2fd084eca",  /* vrsqrt14pd %xmm2,%xmm1 */
    "62f2fd284eca",  /* vrsqrt14pd %ymm2,%ymm1 */
    "62f27d084eca",  /* vrsqrt14ps %xmm2,%xmm1 */
    "62f27d284eca",  /* vrsqrt14ps %ymm2,%ymm1 */
    "62f2fd084fca",  /* vrsqrt14sd %xmm2,%xmm0,%xmm1 */
    "62f2e5084fca",  /* vrsqrt14sd %xmm2,%xmm3,%xmm1 */
    "62f27d084fca",  /* vrsqrt14ss %xmm2,%xmm0,%xmm1 */
    "62f265084fca",  /* vrsqrt14ss %xmm2,%xmm3,%xmm1 */
    "62f2fd48ccca",  /* vrsqrt28pd %zmm2,%zmm1 */
    "62f2fd48cc4801",  /* vrsqrt28pd 0x40(%rax),%zmm1 */
    "62f27d48ccca",  /* vrsqrt28ps %zmm2,%zmm1 */
    "62f27d48cc4801",  /* vrsqrt28ps 0x40(%rax),%zmm1 */
    "62f2fd08cdca",  /* vrsqrt28sd %xmm2,%xmm0,%xmm1 */
    "62f2e508cdca",  /* vrsqrt28sd %xmm2,%xmm3,%xmm1 */
    "62f27d08cdca",  /* vrsqrt28ss %xmm2,%xmm0,%xmm1 */
    "62f26508cdca",  /* vrsqrt28ss %xmm2,%xmm3,%xmm1 */
    "62f67d084eca",  /* vrsqrtph %xmm2,%xmm1 */
    "62f67d284eca",  /* vrsqrtph %ymm2,%ymm1 */
    "62f67d084fca",  /* vrsqrtsh %xmm2,%xmm0,%xmm1 */
    "62f665084fca",  /* vrsqrtsh %xmm2,%xmm3,%xmm1 */
    "62f2fd082cca",  /* vscalefpd %xmm2,%xmm0,%xmm1 */
    "62f2e5082cca",  /* vscalefpd %xmm2,%xmm3,%xmm1 */
    "62f67d082cca",  /* vscalefph %xmm2,%xmm0,%xmm1 */
    "62f665082cca",  /* vscalefph %xmm2,%xmm3,%xmm1 */
    "62f27d082cca",  /* vscalefps %xmm2,%xmm0,%xmm1 */
    "62f265082cca",  /* vscalefps %xmm2,%xmm3,%xmm1 */
    "62f2fd082dca",  /* vscalefsd %xmm2,%xmm0,%xmm1 */
    "62f2e5082dca",  /* vscalefsd %xmm2,%xmm3,%xmm1 */
    "62f67d082dca",  /* vscalefsh %xmm2,%xmm0,%xmm1 */
    "62f665082dca",  /* vscalefsh %xmm2,%xmm3,%xmm1 */
    "62f27d082dca",  /* vscalefss %xmm2,%xmm0,%xmm1 */
    "62f265082dca",  /* vscalefss %xmm2,%xmm3,%xmm1 */
    "c4e27fccca",  /* vsha512msg1 %xmm2,%ymm1 */
    "c4e27fcdca",  /* vsha512msg2 %ymm2,%ymm1 */
    "c4e27fcbca",  /* vsha512rnds2 %xmm2,%ymm0,%ymm1 */
    "c4e267cbca",  /* vsha512rnds2 %xmm2,%ymm3,%ymm1 */
    "62f37d2823ca47",  /* vshuff32x4 $0x47,%ymm2,%ymm0,%ymm1 */
    "62f3652823ca47",  /* vshuff32x4 $0x47,%ymm2,%ymm3,%ymm1 */
    "62f3fd2823ca47",  /* vshuff64x2 $0x47,%ymm2,%ymm0,%ymm1 */
    "62f3e52823ca47",  /* vshuff64x2 $0x47,%ymm2,%ymm3,%ymm1 */
    "62f37d2843ca47",  /* vshufi32x4 $0x47,%ymm2,%ymm0,%ymm1 */
    "62f3652843ca47",  /* vshufi32x4 $0x47,%ymm2,%ymm3,%ymm1 */
    "62f3fd2843ca47",  /* vshufi64x2 $0x47,%ymm2,%ymm0,%ymm1 */
    "62f3e52843ca47",  /* vshufi64x2 $0x47,%ymm2,%ymm3,%ymm1 */
    "c4e278daca",  /* vsm3msg1 %xmm2,%xmm0,%xmm1 */
    "c4e260daca",  /* vsm3msg1 %xmm2,%xmm3,%xmm1 */
    "c4e279daca",  /* vsm3msg2 %xmm2,%xmm0,%xmm1 */
    "c4e261daca",  /* vsm3msg2 %xmm2,%xmm3,%xmm1 */
    "c4e379deca47",  /* vsm3rnds2 $0x47,%xmm2,%xmm0,%xmm1 */
    "c4e361deca47",  /* vsm3rnds2 $0x47,%xmm2,%xmm3,%xmm1 */
    "c4e27adaca",  /* vsm4key4 %xmm2,%xmm0,%xmm1 */
    "c4e262daca",  /* vsm4key4 %xmm2,%xmm3,%xmm1 */
    "c4e27bdaca",  /* vsm4rnds4 %xmm2,%xmm0,%xmm1 */
    "c4e263daca",  /* vsm4rnds4 %xmm2,%xmm3,%xmm1 */
    "62f57c0851ca",  /* vsqrtph %xmm2,%xmm1 */
    "62f57c2851ca",  /* vsqrtph %ymm2,%ymm1 */
    "62f57e0851ca",  /* vsqrtsh %xmm2,%xmm0,%xmm1 */
    "62f5660851ca",  /* vsqrtsh %xmm2,%xmm3,%xmm1 */
    "62f57c085cca",  /* vsubph %xmm2,%xmm0,%xmm1 */
    "62f564085cca",  /* vsubph %xmm2,%xmm3,%xmm1 */
    "62f57e085cca",  /* vsubsh %xmm2,%xmm0,%xmm1 */
    "62f566085cca",  /* vsubsh %xmm2,%xmm3,%xmm1 */
    "c4e2790fca",  /* vtestpd %xmm2,%xmm1 */
    "c4e27d0fca",  /* vtestpd %ymm2,%ymm1 */
    "c4e2790eca",  /* vtestps %xmm2,%xmm1 */
    "c4e27d0eca",  /* vtestps %ymm2,%ymm1 */
    "62f57c082eca",  /* vucomish %xmm2,%xmm1 */
    "62f57c082e4801",  /* vucomish 0x2(%rax),%xmm1 */
    "62f17e187bc8",  /* vcvtusi2ss xmm1,xmm0,eax,{rn-sae} */
    "6293051151c40c",  /* vrangess xmm0{k1},xmm31,xmm28,{sae},0xc */
    "62f56c58584801",  /* vaddph zmm1,zmm2,WORD BCST [rax+2] */
    "62f57cda5a08",  /* vcvtph2pd zmm1{k2}{z},WORD BCST [rax] */
    "c4e3e968cc30",  /* vfmaddps xmm1,xmm2,xmm3,xmm4 */
    "c4e36948cb42",  /* vpermil2ps xmm1,xmm2,xmm3,xmm4,2 */
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
