/* intrin_x86_scalar - x86-64 intrinsics checked against the instruction set reference's
   definitions (computed by cmd/internal/tools/gen_x86_intrin.py, which
   writes this file: do not edit): AES, PCLMULQDQ, SHA, BMI1, BMI2, LZCNT and the carry chains, where the processor has them. A mismatch prints the intrinsic
   and the result's bytes. */
/* x86-64-only */
/* flags: --c-std=c17 */
/* expect: intrin_x86_scalar 77 */
#include <x86intrin.h>
#include <cpuid.h>
#include "host.h"

#define __L(t, p, o) (*(const t *)(const void *)((p) + (o)))
#define __I64(h, l) ((__axiom_int64)(((unsigned __axiom_int64)(h) << 32) | (unsigned __axiom_int64)(l)))
static __axiom_attribute__((__aligned__(64))) unsigned char A[64] = {0x00, 0x3d, 0x8d, 0xff, 0xed, 0x7b, 0x7f, 0xc3, 0x21, 0x80, 0x06, 0x70, 0x01, 0xd8, 0x3c, 0xfe, 0x00, 0x80, 0x81, 0x06, 0xd9, 0x7e, 0xd5, 0x9d, 0xff, 0xff, 0xff, 0x7f, 0x65, 0xb2, 0x45, 0x38, 0x3d, 0x74, 0x26, 0xa7, 0xf0, 0x85, 0x90, 0x36, 0xe2, 0x5e, 0xd8, 0xe1, 0xea, 0x31, 0x65, 0xf8, 0x7b, 0x4a, 0xc6, 0xf6, 0xbb, 0x1f, 0xa3, 0x2f, 0xfb, 0x90, 0xf8, 0xe8, 0x5b, 0xd2, 0x2d, 0x75};
static __axiom_attribute__((__aligned__(64))) unsigned char AZ[64] = {0x00, 0x3d, 0x8d, 0xff, 0xed, 0x7b, 0x7f, 0x00, 0x21, 0x80, 0x06, 0x70, 0x01, 0xd8, 0x3c, 0xfe, 0x00, 0x80, 0x81, 0x06, 0xd9, 0x7e, 0xd5, 0x9d, 0xff, 0xff, 0xff, 0x7f, 0x65, 0xb2, 0x45, 0x38, 0x3d, 0x74, 0x26, 0xa7, 0xf0, 0x85, 0x90, 0x36, 0xe2, 0x5e, 0xd8, 0xe1, 0xea, 0x31, 0x65, 0xf8, 0x7b, 0x4a, 0xc6, 0xf6, 0xbb, 0x1f, 0xa3, 0x2f, 0xfb, 0x90, 0xf8, 0xe8, 0x5b, 0xd2, 0x2d, 0x75};
static __axiom_attribute__((__aligned__(64))) unsigned char B[64] = {0xa4, 0xe5, 0x6b, 0xa2, 0xe3, 0x00, 0x24, 0x9c, 0xff, 0xda, 0x38, 0x7f, 0x1c, 0x5d, 0x80, 0x2d, 0xff, 0x7f, 0x55, 0xae, 0xfe, 0xd0, 0x23, 0x81, 0x00, 0x00, 0x00, 0x80, 0x26, 0x60, 0xbc, 0x43, 0xd0, 0x25, 0x39, 0x2e, 0x0f, 0x26, 0x6c, 0x7a, 0x81, 0x6c, 0x5b, 0x64, 0x9a, 0xf5, 0xd9, 0x26, 0x55, 0x9d, 0x5a, 0x9b, 0xc3, 0x5c, 0xc8, 0x86, 0xf5, 0x34, 0xff, 0x7c, 0x7e, 0xed, 0x27, 0x52};
static __axiom_attribute__((__aligned__(64))) unsigned char C[64] = {0x7e, 0xcd, 0x1f, 0xcd, 0xb1, 0x83, 0xa2, 0xfb, 0xbf, 0xd8, 0x00, 0x3e, 0xc4, 0xff, 0x84, 0x5e, 0x00, 0x80, 0x9b, 0x80, 0x7e, 0x74, 0x01, 0xac, 0x00, 0x00, 0x00, 0x80, 0x81, 0xd2, 0xb3, 0x7e, 0xb5, 0x62, 0x53, 0x30, 0xe3, 0x6e, 0xad, 0xd0, 0xf6, 0x6c, 0x27, 0xe7, 0xf6, 0x80, 0x41, 0x88, 0xd0, 0x59, 0x5f, 0x2c, 0xc4, 0xe7, 0x03, 0x1c, 0x67, 0xf8, 0xdf, 0x2d, 0xa8, 0x70, 0x6e, 0x1d};
static __axiom_attribute__((__aligned__(64))) unsigned char DA[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xbf, 0x10, 0x9e, 0x4c, 0x18, 0xbc, 0x42, 0x28, 0xc1, 0xa0, 0x8e, 0xcb, 0x33, 0x8b, 0x89, 0xd8, 0xc0, 0xf0, 0x23, 0x8e, 0x3c, 0xb3, 0x17, 0xf7, 0xc0, 0xd8, 0x2d, 0xfd, 0x67, 0x6e, 0x93, 0x0f, 0x41};
static __axiom_attribute__((__aligned__(64))) unsigned char DB[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x7f, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x7f, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc4, 0x66, 0xad, 0x9d, 0xe4, 0x06, 0x1b, 0x41, 0xb0, 0x02, 0x21, 0x0c, 0x00, 0x69, 0xfb, 0x40, 0x1f, 0xe7, 0xfc, 0x74, 0x95, 0x1b, 0x29, 0xc1, 0x04, 0xdf, 0xad, 0x7a, 0x5d, 0x65, 0x22, 0xc1};
static __axiom_attribute__((__aligned__(64))) unsigned char DC[64] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x3b, 0x77, 0x30, 0xd1, 0x42, 0xee, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xfe, 0x7d, 0xad, 0x40, 0x64, 0xf4, 0x18, 0xc1, 0xec, 0x93, 0x14, 0x10, 0x33, 0xd4, 0x0e, 0xc1, 0x45, 0x38, 0x2d, 0xbc, 0x62, 0xc1, 0x2b, 0xc1, 0x78, 0x0d, 0x40, 0x34, 0x39, 0x0e, 0x18, 0xc1};
static __axiom_attribute__((__aligned__(64))) unsigned char DN[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0xc0};
static __axiom_attribute__((__aligned__(64))) unsigned char FA[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0xc0, 0xbf, 0x00, 0x00, 0x80, 0x7f, 0x00, 0x00, 0x80, 0xff, 0x01, 0x00, 0xc0, 0x7f, 0x01, 0x00, 0x00, 0x00, 0x78, 0x28, 0x96, 0xc2, 0x93, 0x7c, 0x53, 0xc2, 0x5e, 0x17, 0x93, 0x42, 0x6b, 0xef, 0x6f, 0x40, 0x0b, 0x38, 0x40, 0x42, 0x61, 0x34, 0xc9, 0x3f, 0xb4, 0x9e, 0x8d, 0x42, 0xad, 0xf0, 0x20, 0x42};
static __axiom_attribute__((__aligned__(64))) unsigned char FB[64] = {0x00, 0x00, 0x80, 0xff, 0x01, 0x00, 0xc0, 0x7f, 0x01, 0x00, 0x00, 0x00, 0x9e, 0xc9, 0x7f, 0x7f, 0x00, 0x00, 0x20, 0x40, 0x00, 0x00, 0x20, 0xc0, 0x00, 0x00, 0x00, 0x3f, 0xca, 0xf2, 0x49, 0x71, 0xb3, 0x77, 0x30, 0xc2, 0xe4, 0x62, 0x3d, 0x42, 0xb9, 0xd9, 0x50, 0x42, 0x99, 0x85, 0x36, 0xc2, 0x78, 0x86, 0xc4, 0x42, 0xea, 0xd2, 0xaa, 0xc2, 0xd4, 0x6e, 0x98, 0x42, 0x59, 0x90, 0xbc, 0xc2};
static __axiom_attribute__((__aligned__(64))) unsigned char FC[64] = {0x00, 0x00, 0x00, 0x3f, 0xca, 0xf2, 0x49, 0x71, 0x00, 0x00, 0xe8, 0xc0, 0x00, 0xe0, 0x7f, 0x47, 0xab, 0xaa, 0xaa, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x3f, 0x3a, 0x5c, 0xd6, 0xc1, 0xda, 0xe6, 0x33, 0xc1, 0x6d, 0xd6, 0xaa, 0x42, 0x4e, 0xfb, 0xb0, 0xc2, 0xdb, 0x8c, 0x90, 0x42, 0x44, 0xed, 0x90, 0x42, 0x51, 0x70, 0x3b, 0xc2, 0x97, 0xb1, 0x75, 0x42};
static __axiom_attribute__((__aligned__(64))) unsigned char FN[64] = {0x00, 0x00, 0x00, 0xc1, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x40, 0xc0, 0x00, 0x00, 0x40, 0xc0, 0x00, 0x00, 0x40, 0xc0, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x40, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0xa0, 0xc0, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x00, 0xc1, 0x00, 0x00, 0x00, 0xc1, 0x00, 0x00, 0x00, 0x00};
static __axiom_attribute__((__aligned__(64))) unsigned char FP[16] = {0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x80, 0x40};
static __axiom_attribute__((__aligned__(64))) unsigned char IG[64] = {0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static __axiom_attribute__((__aligned__(64))) unsigned char IX[64] = {0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static __axiom_attribute__((__aligned__(64))) unsigned char SC32[32] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
static __axiom_attribute__((__aligned__(64))) unsigned char SC64[32] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static int __checks;
static int __fails;

static char __line[512];
static size_t __len;

static void say(const char *s) {
    while (*s != 0 && __len < sizeof __line) {
        __line[__len++] = *s++;
    }
}

static void endl(void) {
    print_str(__line, __len);
    __len = 0;
}

static void say_n(int v) {
    char b[16];
    int k = 15;
    b[15] = 0;
    do {
        b[--k] = (char)('0' + v % 10);
        v /= 10;
    } while (v != 0);
    say(b + k);
}

static void hex(const unsigned char *p, int n) {
    static const char d[] = "0123456789abcdef";
    char b[3];
    int i;
    b[2] = 0;
    for (i = 0; i < n; i++) {
        b[0] = d[p[i] >> 4];
        b[1] = d[p[i] & 15];
        say(b);
    }
}

/* The result's n bytes at r, against w's; approx: float lanes within a relative 1.5 * 2^-12 */
static void chk(const char *name, const void *r, int n, const char *w, int approx) {
    const unsigned char *p = (const unsigned char *)r;
    const unsigned char *q = (const unsigned char *)w;
    int i;
    int bad = 0;
    __checks++;
    for (i = 0; i < n; i++) {
        if (p[i] != q[i]) {
            bad = 1;
        }
    }
    if (bad && approx) {
        bad = 0;
        for (i = 0; i < n; i += 4) {
            float x;
            float y;
            float e;
            unsigned char *xb = (unsigned char *)&x;
            unsigned char *yb = (unsigned char *)&y;
            int j;
            for (j = 0; j < 4; j++) {
                xb[j] = p[i + j];
                yb[j] = q[i + j];
            }
            e = (x - y) / y;
            if (!(e < 0.000367f && e > -0.000367f) && !(x == y)) {
                bad = 1;
            }
        }
    }
    if (bad) {
        __fails++;
        say(name);
        say(": ");
        hex(p, n);
        say(" expected ");
        hex(q, n);
        endl();
    }
}

/* (the inputs, written where the compiler cannot see: no result is folded) */
static void (*volatile __touch)(unsigned char *) = 0;

static int __cpu(unsigned int leaf, int reg, unsigned int bit) {
    unsigned int r[4];
    if (!__get_cpuid_count(leaf, 0, &r[0], &r[1], &r[2], &r[3])) {
        return 0;
    }
    return (r[reg] & bit) != 0;
}

static int __os_avx(void) {
    return __cpu(1, 2, bit_OSXSAVE) && (_xgetbv(0) & 6) == 6;
}

static __axiom_attribute__((__target__("aes,pclmul,sha,bmi,bmi2,lzcnt,adx"))) void run0(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128i __r = _mm_aesenc_si128(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_aesenc_si128", &__r, 16, "\xd5\xce\x21\x24\xf8\xe4\x4b\x69\xce\x2c\x6c\x03\x0e\x53\x97\xfe", 0);
    }
    {
        __m128i __r = _mm_aesenclast_si128(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_aesenclast_si128", &__r, 16, "\xc7\xc4\x04\x19\xb6\xcd\xcf\x8a\x02\xbb\x65\x51\x60\x7a\x52\x7c", 0);
    }
    {
        __m128i __r = _mm_aesdec_si128(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_aesdec_si128", &__r, 16, "\x19\x5b\xe8\xcb\x83\x4b\xb0\x46\xff\x0a\x6c\x3b\xf6\xd4\x42\xa9", 0);
    }
    {
        __m128i __r = _mm_aesdeclast_si128(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_aesdeclast_si128", &__r, 16, "\xf6\xc8\xce\x91\xb0\x8b\x49\x4c\x84\xd9\x8c\x73\x15\x67\xeb\x50", 0);
    }
    {
        __m128i __r = _mm_aesimc_si128(__L(__m128i, A, 0));
        chk("_mm_aesimc_si128", &__r, 16, "\x79\x62\x6a\x3e\xc5\xc4\x03\x28\xd1\x4f\x43\x0a\x9f\x50\x94\x40", 0);
    }
    {
        __m128i __r = _mm_aeskeygenassist_si128(__L(__m128i, A, 0), 0x1);
        chk("_mm_aeskeygenassist_si128/1", &__r, 16, "\x55\x21\xd2\x2e\x20\xd2\x2e\x55\x7c\x61\xeb\xbb\x60\xeb\xbb\x7c", 0);
    }
    {
        __m128i __r = _mm_aeskeygenassist_si128(__L(__m128i, A, 0), 0x1b);
        chk("_mm_aeskeygenassist_si128/1b", &__r, 16, "\x55\x21\xd2\x2e\x3a\xd2\x2e\x55\x7c\x61\xeb\xbb\x7a\xeb\xbb\x7c", 0);
    }
    {
        __m128i __r = _mm_aeskeygenassist_si128(__L(__m128i, A, 0), 0x36);
        chk("_mm_aeskeygenassist_si128/36", &__r, 16, "\x55\x21\xd2\x2e\x17\xd2\x2e\x55\x7c\x61\xeb\xbb\x57\xeb\xbb\x7c", 0);
    }
    {
        __m128i __r = _mm_clmulepi64_si128(__L(__m128i, A, 0), __L(__m128i, B, 0), 0x0);
        chk("_mm_clmulepi64_si128/0", &__r, 16, "\x00\xd4\xa4\x49\x6e\xeb\xfb\x1d\x5a\xeb\x40\x58\x22\x2e\x85\x68", 0);
    }
    {
        __m128i __r = _mm_clmulepi64_si128(__L(__m128i, A, 0), __L(__m128i, B, 0), 0x1);
        chk("_mm_clmulepi64_si128/1", &__r, 16, "\x24\x51\x1d\x80\xf9\xbe\xba\x99\x9d\xc7\xef\xf4\x16\x8b\xe8\x74", 0);
    }
    {
        __m128i __r = _mm_clmulepi64_si128(__L(__m128i, A, 0), __L(__m128i, B, 0), 0x10);
        chk("_mm_clmulepi64_si128/10", &__r, 16, "\x00\xeb\xed\xc7\xa6\x51\xbf\x21\x12\xf2\xca\x2d\xbf\x10\xdb\x1d", 0);
    }
    {
        __m128i __r = _mm_clmulepi64_si128(__L(__m128i, A, 0), __L(__m128i, B, 0), 0x11);
        chk("_mm_clmulepi64_si128/11", &__r, 16, "\x1f\x05\x5e\xfb\x2d\xf1\x02\xa3\x3f\x33\x1f\x33\x84\x98\x4f\x1b", 0);
    }
    {
        __m128i __r = _mm_sha1nexte_epu32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_sha1nexte_epu32", &__r, 16, "\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c\xff\xda\x38\x7f\x1c\x93\x0f\xad", 0);
    }
    {
        __m128i __r = _mm_sha1msg1_epu32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_sha1msg1_epu32", &__r, 16, "\xff\xe7\xb5\x80\xf1\x26\xff\xee\x21\xbd\x8b\x8f\xec\xa3\x43\x3d", 0);
    }
    {
        __m128i __r = _mm_sha1msg2_epu32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_sha1msg2_epu32", &__r, 16, "\xfb\x71\x0a\xfb\x92\x3c\x29\xc2\x85\x01\x45\xd8\xfd\x05\x08\x02", 0);
    }
    {
        __m128i __r = _mm_sha256msg1_epu32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_sha256msg1_epu32", &__r, 16, "\x55\x9e\x9f\x1c\xf2\xfc\x67\xb0\x60\x1d\x42\x9a\xe6\x5b\x9d\xa2", 0);
    }
    {
        __m128i __r = _mm_sha256msg2_epu32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_sha256msg2_epu32", &__r, 16, "\x4d\xdb\xcc\x75\x54\x6f\xa6\xe8\x8a\xe7\xd8\x66\xdd\x98\xb7\xf8", 0);
    }
    {
        __m128i __r = _mm_sha1rnds4_epu32(__L(__m128i, A, 0), __L(__m128i, B, 0), 0);
        chk("_mm_sha1rnds4_epu32/0", &__r, 16, "\x05\x45\xcb\x47\x09\xae\x0b\xc7\x54\xaf\x0a\x9f\xdd\xef\xd0\xa1", 0);
    }
    {
        __m128i __r = _mm_sha1rnds4_epu32(__L(__m128i, A, 0), __L(__m128i, B, 0), 1);
        chk("_mm_sha1rnds4_epu32/1", &__r, 16, "\xf2\x83\x3a\x2c\x26\x00\x09\x9b\x7e\x15\x62\xe6\x91\xd6\x1b\x79", 0);
    }
    {
        __m128i __r = _mm_sha1rnds4_epu32(__L(__m128i, A, 0), __L(__m128i, B, 0), 2);
        chk("_mm_sha1rnds4_epu32/2", &__r, 16, "\xd6\x94\xd1\x1d\xfc\x8d\x3a\xb5\x0e\x6a\x0d\x0b\x1b\xbb\x01\x4d", 0);
    }
    {
        __m128i __r = _mm_sha1rnds4_epu32(__L(__m128i, A, 0), __L(__m128i, B, 0), 3);
        chk("_mm_sha1rnds4_epu32/3", &__r, 16, "\x7f\xb9\x1c\x43\x56\xe7\x31\x8e\xe2\x5d\xf6\x72\xf5\x15\xf0\x2b", 0);
    }
    {
        __m128i __r = _mm_sha256rnds2_epu32(__L(__m128i, A, 0), __L(__m128i, B, 0), __L(__m128i, C, 0));
        chk("_mm_sha256rnds2_epu32", &__r, 16, "\x98\x6b\x32\x80\xe6\x5f\xed\x67\x87\xcf\x59\x72\x6a\x64\x5d\xb0", 0);
    }
    {
        unsigned int __r = _andn_u32(0xff8d3d00u, 0xa26be5a4u);
        chk("_andn_u32", &__r, 4, "\xa4\xc0\x62\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _andn_u64(0xfe3cd80170068021ull, 0x2d805d1c7f38daffull);
        chk("_andn_u64", &__r, 8, "\xde\x5a\x38\x0f\x1c\x05\x80\x01", 0);
    }
    {
        unsigned int __r = _bextr_u32(0xff8d3d00u, 3, 9);
        chk("_bextr_u32/3/9", &__r, 4, "\xa0\x01\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _bextr_u64(0xfe3cd80170068021ull, 3, 9);
        chk("_bextr_u64/3/9", &__r, 8, "\x04\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _bextr_u32(0xff8d3d00u, 0, 32);
        chk("_bextr_u32/0/32", &__r, 4, "\x00\x3d\x8d\xff", 0);
    }
    {
        unsigned __axiom_int64 __r = _bextr_u64(0xfe3cd80170068021ull, 0, 32);
        chk("_bextr_u64/0/32", &__r, 8, "\x21\x80\x06\x70\x00\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _bextr_u32(0xff8d3d00u, 28, 8);
        chk("_bextr_u32/28/8", &__r, 4, "\x0f\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _bextr_u64(0xfe3cd80170068021ull, 28, 8);
        chk("_bextr_u64/28/8", &__r, 8, "\x17\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _bextr_u32(0xff8d3d00u, 40, 3);
        chk("_bextr_u32/40/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _bextr_u64(0xfe3cd80170068021ull, 40, 3);
        chk("_bextr_u64/40/3", &__r, 8, "\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("aes,pclmul,sha,bmi,bmi2,lzcnt,adx"))) void run1(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        unsigned int __r = _blsi_u32(0xff8d3d00u);
        chk("_blsi_u32", &__r, 4, "\x00\x01\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _blsi_u64(0xfe3cd80170068021ull);
        chk("_blsi_u64", &__r, 8, "\x01\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _blsmsk_u32(0xff8d3d00u);
        chk("_blsmsk_u32", &__r, 4, "\xff\x01\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _blsmsk_u64(0xfe3cd80170068021ull);
        chk("_blsmsk_u64", &__r, 8, "\x01\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _blsr_u32(0xff8d3d00u);
        chk("_blsr_u32", &__r, 4, "\x00\x3c\x8d\xff", 0);
    }
    {
        unsigned __axiom_int64 __r = _blsr_u64(0xfe3cd80170068021ull);
        chk("_blsr_u64", &__r, 8, "\x20\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        unsigned int __r = _tzcnt_u32(0xff8d3d00u);
        chk("_tzcnt_u32/ff8d3d00", &__r, 4, "\x08\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _tzcnt_u32(0x0u);
        chk("_tzcnt_u32/0", &__r, 4, "\x20\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _tzcnt_u32(0x80000000u);
        chk("_tzcnt_u32/80000000", &__r, 4, "\x1f\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _bzhi_u32(0xff8d3d00u, 0);
        chk("_bzhi_u32/0", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _bzhi_u64(0xfe3cd80170068021ull, 0);
        chk("_bzhi_u64/0", &__r, 8, "\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _bzhi_u32(0xff8d3d00u, 7);
        chk("_bzhi_u32/7", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _bzhi_u64(0xfe3cd80170068021ull, 7);
        chk("_bzhi_u64/7", &__r, 8, "\x21\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _bzhi_u32(0xff8d3d00u, 31);
        chk("_bzhi_u32/31", &__r, 4, "\x00\x3d\x8d\x7f", 0);
    }
    {
        unsigned __axiom_int64 __r = _bzhi_u64(0xfe3cd80170068021ull, 31);
        chk("_bzhi_u64/31", &__r, 8, "\x21\x80\x06\x70\x00\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _bzhi_u32(0xff8d3d00u, 32);
        chk("_bzhi_u32/32", &__r, 4, "\x00\x3d\x8d\xff", 0);
    }
    {
        unsigned __axiom_int64 __r = _bzhi_u64(0xfe3cd80170068021ull, 32);
        chk("_bzhi_u64/32", &__r, 8, "\x21\x80\x06\x70\x00\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _bzhi_u32(0xff8d3d00u, 200);
        chk("_bzhi_u32/200", &__r, 4, "\x00\x3d\x8d\xff", 0);
    }
    {
        unsigned __axiom_int64 __r = _bzhi_u64(0xfe3cd80170068021ull, 200);
        chk("_bzhi_u64/200", &__r, 8, "\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        unsigned int __r = _pdep_u32(0xff8d3d00u, 0xa26be5a4u);
        chk("_pdep_u32", &__r, 4, "\x00\x00\x69\x02", 0);
    }
    {
        unsigned int __r = _pext_u32(0xff8d3d00u, 0xa26be5a4u);
        chk("_pext_u32", &__r, 4, "\x38\xe5\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _pdep_u64(0xfe3cd80170068021ull, 0x2d805d1c7f38daffull);
        chk("_pdep_u64", &__r, 8, "\x21\x00\x20\x06\x00\x58\x00\x01", 0);
    }
    {
        unsigned __axiom_int64 __r = _pext_u64(0xfe3cd80170068021ull, 0x2d805d1c7f38daffull);
        chk("_pext_u64", &__r, 8, "\x21\x10\x70\x70\x0e\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _mulx_u64(0xfe3cd80170068021ull, 0x2d805d1c7f38daffull, &__hi) ^ __hi;
        chk("_mulx_u64", &__r, 8, "\x4a\x49\x2c\x2d\xb0\x32\x02\xad", 0);
    }
    {
        unsigned int __r = _mulx_u32(0xff8d3d00u, 0xa26be5a4u, &__hi32) ^ __hi32;
        chk("_mulx_u32", &__r, 4, "\xdf\x01\x2f\x2f", 0);
    }
    {
        unsigned int __r = _lzcnt_u32(0xff8d3d00u);
        chk("_lzcnt_u32/ff8d3d00", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _lzcnt_u32(0x0u);
        chk("_lzcnt_u32/0", &__r, 4, "\x20\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _lzcnt_u32(0x1u);
        chk("_lzcnt_u32/1", &__r, 4, "\x1f\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _lzcnt_u64(0xc37f7bedff8d3d00ull);
        chk("_lzcnt_u64/c37f7bedff8d3d00", &__r, 8, "\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _lzcnt_u64(0x0ull);
        chk("_lzcnt_u64/0", &__r, 8, "\x40\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _lzcnt_u64(0x1ull);
        chk("_lzcnt_u64/1", &__r, 8, "\x3f\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = (unsigned int)_addcarry_u32(0, 0xffffffffu, 0x1u, &__o32) * 0x100000000ull + __o32;
        chk("_addcarry_u32/0/ffffffff/1", &__r, 8, "\x00\x00\x00\x00\x01\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("aes,pclmul,sha,bmi,bmi2,lzcnt,adx"))) void run2(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        unsigned __axiom_int64 __r = (unsigned int)_subborrow_u32(0, 0xffffffffu, 0x1u, &__o32) * 0x100000000ull + __o32;
        chk("_subborrow_u32/0/ffffffff/1", &__r, 8, "\xfe\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = (unsigned int)_addcarryx_u32(0, 0xffffffffu, 0x1u, &__o32) * 0x100000000ull + __o32;
        chk("_addcarryx_u32/0/ffffffff/1", &__r, 8, "\x00\x00\x00\x00\x01\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = (unsigned int)_addcarry_u32(1, 0xffffffffu, 0x0u, &__o32) * 0x100000000ull + __o32;
        chk("_addcarry_u32/1/ffffffff/0", &__r, 8, "\x00\x00\x00\x00\x01\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = (unsigned int)_subborrow_u32(1, 0xffffffffu, 0x0u, &__o32) * 0x100000000ull + __o32;
        chk("_subborrow_u32/1/ffffffff/0", &__r, 8, "\xfe\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = (unsigned int)_addcarryx_u32(1, 0xffffffffu, 0x0u, &__o32) * 0x100000000ull + __o32;
        chk("_addcarryx_u32/1/ffffffff/0", &__r, 8, "\x00\x00\x00\x00\x01\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = (unsigned int)_addcarry_u32(1, 0x5u, 0x7u, &__o32) * 0x100000000ull + __o32;
        chk("_addcarry_u32/1/5/7", &__r, 8, "\x0d\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = (unsigned int)_subborrow_u32(1, 0x5u, 0x7u, &__o32) * 0x100000000ull + __o32;
        chk("_subborrow_u32/1/5/7", &__r, 8, "\xfd\xff\xff\xff\x01\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = (unsigned int)_addcarryx_u32(1, 0x5u, 0x7u, &__o32) * 0x100000000ull + __o32;
        chk("_addcarryx_u32/1/5/7", &__r, 8, "\x0d\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = (unsigned int)_addcarry_u32(0, 0x3u, 0x4u, &__o32) * 0x100000000ull + __o32;
        chk("_addcarry_u32/0/3/4", &__r, 8, "\x07\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = (unsigned int)_subborrow_u32(0, 0x3u, 0x4u, &__o32) * 0x100000000ull + __o32;
        chk("_subborrow_u32/0/3/4", &__r, 8, "\xff\xff\xff\xff\x01\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = (unsigned int)_addcarryx_u32(0, 0x3u, 0x4u, &__o32) * 0x100000000ull + __o32;
        chk("_addcarryx_u32/0/3/4", &__r, 8, "\x07\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _addcarry_u64(1, 0xffffffffffffffffull, 0x0ull, &__o64) + __o64;
        chk("_addcarry_u64/1", &__r, 8, "\x01\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned __axiom_int64 __r = _addcarry_u64(0, 0x8000000000000000ull, 0x8000000000000000ull, &__o64) + __o64;
        chk("_addcarry_u64/0", &__r, 8, "\x01\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("aes,pclmul,sha,bmi,bmi2,lzcnt,adx"))) void run(void) {
    run0();
    run1();
    run2();
}

int main(void) {
    if (__touch != 0) {
        __touch(A);
        __touch(AZ);
        __touch(B);
        __touch(C);
        __touch(DA);
        __touch(DB);
        __touch(DC);
        __touch(DN);
        __touch(FA);
        __touch(FB);
        __touch(FC);
        __touch(FN);
        __touch(FP);
        __touch(IG);
        __touch(IX);
        __touch(SC32);
        __touch(SC64);
    }
    if (__cpu(1, 2, bit_AES) && __cpu(1, 2, bit_PCLMUL) && __cpu(7, 1, bit_SHA) && __cpu(7, 1, bit_BMI) && __cpu(7, 1, bit_BMI2) && __cpu(0x80000001, 2, bit_LZCNT) && __cpu(7, 1, bit_ADX)) {
        run();
    } else {
        __checks = 77;
    }
    if (__fails == 0) {
        say("intrin_x86_scalar ");
        say_n(__checks);
        endl();
    }
    return 0;
}
