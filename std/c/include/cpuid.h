/* <cpuid.h> - the x86-64 processor's identification (the cpuid instruction:
   Intel SDM vol. 2A, "CPUID"), as the conventional macros and functions of
   this header give it (the compiler's: docs/c.md, "Intrinsics"), and the
   feature bits of its leaves 1, 7 and 0x80000001. */
#ifndef __AXIOM_CPUID_H
#define __AXIOM_CPUID_H
#if !defined(__x86_64__)
#error "<cpuid.h> is for x86-64"
#endif

typedef unsigned int __axiom_cpuid_regs __axiom_attribute__((__vector_size__(16)));

/* leaf 1, ecx */
#define bit_SSE3 0x00000001u
#define bit_PCLMUL 0x00000002u
#define bit_SSSE3 0x00000200u
#define bit_FMA 0x00001000u
#define bit_CMPXCHG16B 0x00002000u
#define bit_SSE4_1 0x00080000u
#define bit_SSE4_2 0x00100000u
#define bit_MOVBE 0x00400000u
#define bit_POPCNT 0x00800000u
#define bit_AES 0x02000000u
#define bit_XSAVE 0x04000000u
#define bit_OSXSAVE 0x08000000u
#define bit_AVX 0x10000000u
#define bit_F16C 0x20000000u
#define bit_RDRND 0x40000000u
/* leaf 1, edx */
#define bit_CMPXCHG8B 0x00000100u
#define bit_CMOV 0x00008000u
#define bit_MMX 0x00800000u
#define bit_FXSAVE 0x01000000u
#define bit_SSE 0x02000000u
#define bit_SSE2 0x04000000u
/* leaf 7 (subleaf 0), ebx */
#define bit_FSGSBASE 0x00000001u
#define bit_BMI 0x00000008u
#define bit_AVX2 0x00000020u
#define bit_BMI2 0x00000100u
#define bit_AVX512F 0x00010000u
#define bit_AVX512DQ 0x00020000u
#define bit_RDSEED 0x00040000u
#define bit_ADX 0x00080000u
#define bit_AVX512IFMA 0x00200000u
#define bit_AVX512CD 0x10000000u
#define bit_SHA 0x20000000u
#define bit_AVX512BW 0x40000000u
#define bit_AVX512VL 0x80000000u
/* leaf 7 (subleaf 0), ecx */
#define bit_AVX512VBMI 0x00000002u
#define bit_AVX512VBMI2 0x00000040u
#define bit_GFNI 0x00000100u
#define bit_VAES 0x00000200u
#define bit_VPCLMULQDQ 0x00000400u
#define bit_AVX512VNNI 0x00000800u
#define bit_AVX512BITALG 0x00001000u
#define bit_AVX512VPOPCNTDQ 0x00004000u
/* leaf 7 (subleaf 0), edx */
#define bit_AVX512FP16 0x00800000u
/* leaf 0x80000001, ecx and edx */
#define bit_LAHF_LM 0x00000001u
#define bit_ABM 0x00000020u
#define bit_LZCNT 0x00000020u
#define bit_PRFCHW 0x00000100u
#define bit_LM 0x20000000u

static __axiom_attribute__((__always_inline__, __unused__)) __axiom_cpuid_regs __axiom_cpuid(unsigned int __axiom_leaf,
                                                                                          unsigned int __axiom_sub) {
    return (__axiom_cpuid_regs)__builtin_axiom_simd("cpuid", __axiom_leaf, __axiom_sub);
}

/* Leaf `l` (subleaf `s`) into the lvalues a, b, c, d (eax, ebx, ecx, edx). */
#define __cpuid_count(l, s, a, b, c, d)                                                      \
    do {                                                                                     \
        __axiom_cpuid_regs __axiom_r = __axiom_cpuid((unsigned int)(l), (unsigned int)(s)); \
        (a) = __axiom_r[0];                                                                  \
        (b) = __axiom_r[1];                                                                  \
        (c) = __axiom_r[2];                                                                  \
        (d) = __axiom_r[3];                                                                  \
    } while (0)
#define __cpuid(l, a, b, c, d) __cpuid_count((l), 0, a, b, c, d)

/* The highest leaf of the range of `ext` (0, or 0x80000000), and in *sig
   (unless null) leaf ext's ebx (the vendor's first four letters). */
static __axiom_attribute__((__unused__)) unsigned int __get_cpuid_max(unsigned int __axiom_ext,
                                                                     unsigned int *__axiom_sig) {
    __axiom_cpuid_regs __axiom_r = __axiom_cpuid(__axiom_ext, 0);
    if (__axiom_sig != 0) {
        *__axiom_sig = __axiom_r[1];
    }
    return __axiom_r[0];
}

/* Leaf `l` (subleaf `s`) into *a ... *d: 1, or 0 when the processor has no such leaf. */
static __axiom_attribute__((__unused__)) int __get_cpuid_count(unsigned int __axiom_l, unsigned int __axiom_s,
                                                              unsigned int *__axiom_a, unsigned int *__axiom_b,
                                                              unsigned int *__axiom_c, unsigned int *__axiom_d) {
    __axiom_cpuid_regs __axiom_r;
    if (__get_cpuid_max(__axiom_l & 0x80000000u, 0) < __axiom_l) {
        return 0;
    }
    __axiom_r = __axiom_cpuid(__axiom_l, __axiom_s);
    *__axiom_a = __axiom_r[0];
    *__axiom_b = __axiom_r[1];
    *__axiom_c = __axiom_r[2];
    *__axiom_d = __axiom_r[3];
    return 1;
}

static __axiom_attribute__((__unused__)) int __get_cpuid(unsigned int __axiom_l, unsigned int *__axiom_a,
                                                        unsigned int *__axiom_b, unsigned int *__axiom_c,
                                                        unsigned int *__axiom_d) {
    return __get_cpuid_count(__axiom_l, 0, __axiom_a, __axiom_b, __axiom_c, __axiom_d);
}
#endif
