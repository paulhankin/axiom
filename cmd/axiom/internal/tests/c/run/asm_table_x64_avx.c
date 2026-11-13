/* flags: --c-std=gnu17 */
/* native-only */
/* The AVX (VEX) forms of the SSE3/SSSE3/SSE4.1/SSE4.2 instructions and their
   AVX-512BW/F (EVEX) forms added to asm/x64tab.ax, against a byte table whose
   bytes were checked against the system assembler in development. */
#include "host.h"

#ifdef __AXIOM_ASM_X86_64__
__asm__(".section .rodata.axiom_asm_table, \"a\"\n"
        ".globl axiom_asm_table\n"
        "axiom_asm_table:\n"
        "vhaddpd %xmm1,%xmm2,%xmm3\n"
        "vhaddps %ymm1,%ymm2,%ymm3\n"
        "vhsubpd 0x40(%rax),%xmm2,%xmm3\n"
        "vhsubps %xmm15,%xmm2,%xmm3\n"
        "vaddsubpd %ymm1,%ymm2,%ymm3\n"
        "vaddsubps %xmm1,%xmm2,%xmm3\n"
        "vrcpps %ymm11,%ymm2\n"
        "vrcpss %xmm1,%xmm2,%xmm3\n"
        "vrsqrtps (%rbx),%xmm2\n"
        "vrsqrtss %xmm1,%xmm2,%xmm3\n"
        "vmovddup %ymm1,%ymm2\n"
        "vmovddup (%rsp),%xmm2\n"
        "vmovshdup %xmm1,%xmm2\n"
        "vmovsldup %ymm1,%ymm2\n"
        "vlddqu (%rax),%ymm2\n"
        "vmovhps (%rax),%xmm2,%xmm3\n"
        "vmovhps %xmm2,0x8(%rax)\n"
        "vmovlps (%r10),%xmm2,%xmm3\n"
        "vmovhpd (%rax),%xmm2,%xmm3\n"
        "vmovlpd %xmm2,(%rax)\n"
        "vmovhlps %xmm1,%xmm2,%xmm3\n"
        "vmovlhps %xmm1,%xmm2,%xmm3\n"
        "vphaddw %ymm1,%ymm2,%ymm3\n"
        "vphaddd %xmm1,%xmm2,%xmm3\n"
        "vphaddsw %xmm1,%xmm2,%xmm3\n"
        "vpmaddubsw %ymm1,%ymm2,%ymm3\n"
        "vphsubw %xmm1,%xmm2,%xmm3\n"
        "vphsubd %xmm1,%xmm2,%xmm3\n"
        "vphsubsw (%rax),%xmm2,%xmm3\n"
        "vpsignb %ymm1,%ymm2,%ymm3\n"
        "vpsignw %xmm1,%xmm2,%xmm3\n"
        "vpsignd %xmm1,%xmm2,%xmm3\n"
        "vpmulhrsw %ymm1,%ymm2,%ymm3\n"
        "vpabsb %ymm1,%ymm2\n"
        "vpabsw %xmm1,%xmm2\n"
        "vpabsd (%rax),%ymm2\n"
        "vroundps $1,%ymm1,%ymm2\n"
        "vroundpd $2,%xmm1,%xmm2\n"
        "vroundss $3,%xmm1,%xmm2,%xmm3\n"
        "vroundsd $4,%xmm1,%xmm2,%xmm3\n"
        "vpalignr $3,%ymm1,%ymm2,%ymm3\n"
        "vptest %ymm1,%ymm2\n"
        "vphminposuw %xmm1,%xmm2\n"
        "vpackusdw %ymm1,%ymm2,%ymm3\n"
        "vpmovsxbw %xmm1,%ymm2\n"
        "vpmovzxbw (%rax),%ymm2\n"
        "vpminsb %ymm1,%ymm2,%ymm3\n"
        "vpminuw %xmm1,%xmm2,%xmm3\n"
        "vpmaxsb %ymm1,%ymm2,%ymm3\n"
        "vpmaxuw %xmm1,%xmm2,%xmm3\n"
        "vmpsadbw $2,%ymm1,%ymm2,%ymm3\n"
        "vmaskmovps (%rax),%ymm1,%ymm2\n"
        "vmaskmovps %ymm2,%ymm1,(%rax)\n"
        "vmaskmovpd (%rax),%xmm1,%xmm2\n"
        "vmaskmovpd %xmm2,%xmm1,(%rbx)\n"
        "vpmaskmovd (%rax),%ymm1,%ymm2\n"
        "vpmaskmovq %ymm2,%ymm1,(%rax)\n"
        "kunpckbw %k1,%k2,%k3\n"
        "kunpckwd %k1,%k2,%k3\n"
        "kunpckdq %k1,%k2,%k3\n"
        "vcvtsi2ssl (%rax),%xmm1,%xmm2\n"
        "vcvtsi2ssq (%rax),%xmm1,%xmm2\n"
        "vcvtsi2sdl %eax,%xmm1,%xmm2\n"
        "vcvtsi2sdq %rax,%xmm1,%xmm2\n"
        "vpabsb %zmm1,%zmm2{%k1}{z}\n"
        "vpabsw %ymm1,%ymm2{%k2}\n"
        "vpabsd 0x100(%rax),%zmm2{%k3}{z}\n"
        "vpabsq %zmm1,%zmm2{%k4}\n"
        "vpmaddubsw %zmm1,%zmm2,%zmm3{%k5}{z}\n"
        "vpmulhrsw %ymm1,%ymm2,%ymm3{%k6}\n"
        "vpalignr $3,%zmm1,%zmm2,%zmm3{%k7}{z}\n"
        "vpackusdw %zmm1,%zmm2,%zmm3{%k1}\n"
        "vpmovsxbw %ymm1,%zmm2{%k2}{z}\n"
        "vpmovzxbw (%rax),%zmm2{%k3}\n"
        "vpminsb %zmm1,%zmm2,%zmm3{%k4}\n"
        "vpminuw %ymm1,%ymm2,%ymm3{%k5}{z}\n"
        "vpmaxsb %zmm1,%zmm2,%zmm3{%k6}\n"
        "vpmaxuw %xmm1,%xmm2,%xmm3{%k7}{z}\n"
        "vpabsd 0x40(%rax){1to16},%zmm2\n"
        "vpackusdw 0x80(%rax){1to16},%zmm2,%zmm3\n"
        ".globl axiom_asm_table_end\n"
        "axiom_asm_table_end:\n");

extern const unsigned char axiom_asm_table[], axiom_asm_table_end[];
static const char *const want[] = {
    "c5e97cd9",  /* vhaddpd %xmm1,%xmm2,%xmm3 */
    "c5ef7cd9",  /* vhaddps %ymm1,%ymm2,%ymm3 */
    "c5e97d5840",  /* vhsubpd 0x40(%rax),%xmm2,%xmm3 */
    "c4c16b7ddf",  /* vhsubps %xmm15,%xmm2,%xmm3 */
    "c5edd0d9",  /* vaddsubpd %ymm1,%ymm2,%ymm3 */
    "c5ebd0d9",  /* vaddsubps %xmm1,%xmm2,%xmm3 */
    "c4c17c53d3",  /* vrcpps %ymm11,%ymm2 */
    "c5ea53d9",  /* vrcpss %xmm1,%xmm2,%xmm3 */
    "c5f85213",  /* vrsqrtps (%rbx),%xmm2 */
    "c5ea52d9",  /* vrsqrtss %xmm1,%xmm2,%xmm3 */
    "c5ff12d1",  /* vmovddup %ymm1,%ymm2 */
    "c5fb121424",  /* vmovddup (%rsp),%xmm2 */
    "c5fa16d1",  /* vmovshdup %xmm1,%xmm2 */
    "c5fe12d1",  /* vmovsldup %ymm1,%ymm2 */
    "c5fff010",  /* vlddqu (%rax),%ymm2 */
    "c5e81618",  /* vmovhps (%rax),%xmm2,%xmm3 */
    "c5f8175008",  /* vmovhps %xmm2,0x8(%rax) */
    "c4c168121a",  /* vmovlps (%r10),%xmm2,%xmm3 */
    "c5e91618",  /* vmovhpd (%rax),%xmm2,%xmm3 */
    "c5f91310",  /* vmovlpd %xmm2,(%rax) */
    "c5e812d9",  /* vmovhlps %xmm1,%xmm2,%xmm3 */
    "c5e816d9",  /* vmovlhps %xmm1,%xmm2,%xmm3 */
    "c4e26d01d9",  /* vphaddw %ymm1,%ymm2,%ymm3 */
    "c4e26902d9",  /* vphaddd %xmm1,%xmm2,%xmm3 */
    "c4e26903d9",  /* vphaddsw %xmm1,%xmm2,%xmm3 */
    "c4e26d04d9",  /* vpmaddubsw %ymm1,%ymm2,%ymm3 */
    "c4e26905d9",  /* vphsubw %xmm1,%xmm2,%xmm3 */
    "c4e26906d9",  /* vphsubd %xmm1,%xmm2,%xmm3 */
    "c4e2690718",  /* vphsubsw (%rax),%xmm2,%xmm3 */
    "c4e26d08d9",  /* vpsignb %ymm1,%ymm2,%ymm3 */
    "c4e26909d9",  /* vpsignw %xmm1,%xmm2,%xmm3 */
    "c4e2690ad9",  /* vpsignd %xmm1,%xmm2,%xmm3 */
    "c4e26d0bd9",  /* vpmulhrsw %ymm1,%ymm2,%ymm3 */
    "c4e27d1cd1",  /* vpabsb %ymm1,%ymm2 */
    "c4e2791dd1",  /* vpabsw %xmm1,%xmm2 */
    "c4e27d1e10",  /* vpabsd (%rax),%ymm2 */
    "c4e37d08d101",  /* vroundps $1,%ymm1,%ymm2 */
    "c4e37909d102",  /* vroundpd $2,%xmm1,%xmm2 */
    "c4e3690ad903",  /* vroundss $3,%xmm1,%xmm2,%xmm3 */
    "c4e3690bd904",  /* vroundsd $4,%xmm1,%xmm2,%xmm3 */
    "c4e36d0fd903",  /* vpalignr $3,%ymm1,%ymm2,%ymm3 */
    "c4e27d17d1",  /* vptest %ymm1,%ymm2 */
    "c4e27941d1",  /* vphminposuw %xmm1,%xmm2 */
    "c4e26d2bd9",  /* vpackusdw %ymm1,%ymm2,%ymm3 */
    "c4e27d20d1",  /* vpmovsxbw %xmm1,%ymm2 */
    "c4e27d3010",  /* vpmovzxbw (%rax),%ymm2 */
    "c4e26d38d9",  /* vpminsb %ymm1,%ymm2,%ymm3 */
    "c4e2693ad9",  /* vpminuw %xmm1,%xmm2,%xmm3 */
    "c4e26d3cd9",  /* vpmaxsb %ymm1,%ymm2,%ymm3 */
    "c4e2693ed9",  /* vpmaxuw %xmm1,%xmm2,%xmm3 */
    "c4e36d42d902",  /* vmpsadbw $2,%ymm1,%ymm2,%ymm3 */
    "c4e2752c10",  /* vmaskmovps (%rax),%ymm1,%ymm2 */
    "c4e2752e10",  /* vmaskmovps %ymm2,%ymm1,(%rax) */
    "c4e2712d10",  /* vmaskmovpd (%rax),%xmm1,%xmm2 */
    "c4e2712f13",  /* vmaskmovpd %xmm2,%xmm1,(%rbx) */
    "c4e2758c10",  /* vpmaskmovd (%rax),%ymm1,%ymm2 */
    "c4e2f58e10",  /* vpmaskmovq %ymm2,%ymm1,(%rax) */
    "c5ed4bd9",  /* kunpckbw %k1,%k2,%k3 */
    "c5ec4bd9",  /* kunpckwd %k1,%k2,%k3 */
    "c4e1ec4bd9",  /* kunpckdq %k1,%k2,%k3 */
    "c5f22a10",  /* vcvtsi2ssl (%rax),%xmm1,%xmm2 */
    "c4e1f22a10",  /* vcvtsi2ssq (%rax),%xmm1,%xmm2 */
    "c5f32ad0",  /* vcvtsi2sdl %eax,%xmm1,%xmm2 */
    "c4e1f32ad0",  /* vcvtsi2sdq %rax,%xmm1,%xmm2 */
    "62f27dc91cd1",  /* vpabsb %zmm1,%zmm2{%k1}{z} */
    "62f27d2a1dd1",  /* vpabsw %ymm1,%ymm2{%k2} */
    "62f27dcb1e5004",  /* vpabsd 0x100(%rax),%zmm2{%k3}{z} */
    "62f2fd4c1fd1",  /* vpabsq %zmm1,%zmm2{%k4} */
    "62f26dcd04d9",  /* vpmaddubsw %zmm1,%zmm2,%zmm3{%k5}{z} */
    "62f26d2e0bd9",  /* vpmulhrsw %ymm1,%ymm2,%ymm3{%k6} */
    "62f36dcf0fd903",  /* vpalignr $3,%zmm1,%zmm2,%zmm3{%k7}{z} */
    "62f26d492bd9",  /* vpackusdw %zmm1,%zmm2,%zmm3{%k1} */
    "62f27dca20d1",  /* vpmovsxbw %ymm1,%zmm2{%k2}{z} */
    "62f27d4b3010",  /* vpmovzxbw (%rax),%zmm2{%k3} */
    "62f26d4c38d9",  /* vpminsb %zmm1,%zmm2,%zmm3{%k4} */
    "62f26dad3ad9",  /* vpminuw %ymm1,%ymm2,%ymm3{%k5}{z} */
    "62f26d4e3cd9",  /* vpmaxsb %zmm1,%zmm2,%zmm3{%k6} */
    "62f26d8f3ed9",  /* vpmaxuw %xmm1,%xmm2,%xmm3{%k7}{z} */
    "62f27d581e5010",  /* vpabsd 0x40(%rax){1to16},%zmm2 */
    "62f26d582b5820",  /* vpackusdw 0x80(%rax){1to16},%zmm2,%zmm3 */
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
