/* intrin_x86_avx2 - x86-64 intrinsics checked against the instruction set reference's
   definitions (computed by cmd/internal/tools/gen_x86_intrin.py, which
   writes this file: do not edit): AVX, AVX2, FMA and F16C, where the processor and system have them. A mismatch prints the intrinsic
   and the result's bytes. */
/* x86-64-only */
/* flags: --c-std=gnu17 */
/* expect: intrin_x86_avx2 412 */
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

static __axiom_attribute__((__target__("avx2,fma,f16c"))) void run0(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m256i __r = _mm256_add_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_add_epi8", &__r, 32, "\xa4\x22\xf8\xa1\xd0\x7b\xa3\x5f\x20\x5a\x3e\xef\x1d\x35\xbc\x2b\xff\xff\xd6\xb4\xd7\x4e\xf8\x1e\xff\xff\xff\xff\x8b\x12\x01\x7b", 0);
    }
    {
        __m256i __r = _mm256_sub_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_sub_epi8", &__r, 32, "\x5c\x58\x22\x5d\x0a\x7b\x5b\x27\x22\xa6\xce\xf1\xe5\x7b\xbc\xd1\x01\x01\x2c\x58\xdb\xae\xb2\x1c\xff\xff\xff\xff\x3f\x52\x89\xf5", 0);
    }
    {
        __m256i __r = _mm256_add_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_add_epi16", &__r, 32, "\xa4\x22\xf8\xa1\xd0\x7c\xa3\x5f\x20\x5b\x3e\xef\x1d\x35\xbc\x2b\xff\xff\xd6\xb4\xd7\x4f\xf8\x1e\xff\xff\xff\xff\x8b\x12\x01\x7c", 0);
    }
    {
        __m256i __r = _mm256_sub_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_sub_epi16", &__r, 32, "\x5c\x57\x22\x5d\x0a\x7b\x5b\x27\x22\xa5\xce\xf0\xe5\x7a\xbc\xd0\x01\x00\x2c\x58\xdb\xad\xb2\x1c\xff\xff\xff\xff\x3f\x52\x89\xf4", 0);
    }
    {
        __m256i __r = _mm256_add_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_add_epi32", &__r, 32, "\xa4\x22\xf9\xa1\xd0\x7c\xa3\x5f\x20\x5b\x3f\xef\x1d\x35\xbd\x2b\xff\xff\xd6\xb4\xd7\x4f\xf9\x1e\xff\xff\xff\xff\x8b\x12\x02\x7c", 0);
    }
    {
        __m256i __r = _mm256_sub_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_sub_epi32", &__r, 32, "\x5c\x57\x21\x5d\x0a\x7b\x5b\x27\x22\xa5\xcd\xf0\xe5\x7a\xbc\xd0\x01\x00\x2c\x58\xdb\xad\xb1\x1c\xff\xff\xff\xff\x3f\x52\x89\xf4", 0);
    }
    {
        __m256i __r = _mm256_add_epi64(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_add_epi64", &__r, 32, "\xa4\x22\xf9\xa1\xd1\x7c\xa3\x5f\x20\x5b\x3f\xef\x1d\x35\xbd\x2b\xff\xff\xd6\xb4\xd7\x4f\xf9\x1e\xff\xff\xff\xff\x8b\x12\x02\x7c", 0);
    }
    {
        __m256i __r = _mm256_sub_epi64(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_sub_epi64", &__r, 32, "\x5c\x57\x21\x5d\x0a\x7b\x5b\x27\x22\xa5\xcd\xf0\xe4\x7a\xbc\xd0\x01\x00\x2c\x58\xda\xad\xb1\x1c\xff\xff\xff\xff\x3e\x52\x89\xf4", 0);
    }
    {
        __m256i __r = _mm256_adds_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_adds_epi8", &__r, 32, "\xa4\x22\xf8\xa1\xd0\x7b\x7f\x80\x20\x80\x3e\x7f\x1d\x35\xbc\x2b\xff\xff\xd6\xb4\xd7\x4e\xf8\x80\xff\xff\xff\xff\x7f\x12\x01\x7b", 0);
    }
    {
        __m256i __r = _mm256_subs_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_subs_epi8", &__r, 32, "\x5c\x58\x80\x5d\x0a\x7b\x5b\x27\x22\xa6\xce\xf1\xe5\x80\x7f\xd1\x01\x80\x80\x58\xdb\x7f\xb2\x1c\xff\xff\xff\x7f\x3f\x80\x7f\xf5", 0);
    }
    {
        __m256i __r = _mm256_adds_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_adds_epi16", &__r, 32, "\xa4\x22\xf8\xa1\xd0\x7c\x00\x80\x00\x80\xff\x7f\x1d\x35\xbc\x2b\xff\xff\xd6\xb4\xd7\x4f\x00\x80\xff\xff\xff\xff\x8b\x12\x01\x7c", 0);
    }
    {
        __m256i __r = _mm256_subs_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_subs_epi16", &__r, 32, "\x5c\x57\x22\x5d\x0a\x7b\x5b\x27\x22\xa5\xce\xf0\x00\x80\xbc\xd0\x00\x80\x2c\x58\xff\x7f\xb2\x1c\xff\xff\xff\x7f\x00\x80\x89\xf4", 0);
    }
    {
        __m256i __r = _mm256_adds_epu8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_adds_epu8", &__r, 32, "\xa4\xff\xf8\xff\xff\x7b\xa3\xff\xff\xff\x3e\xef\x1d\xff\xbc\xff\xff\xff\xd6\xb4\xff\xff\xf8\xff\xff\xff\xff\xff\x8b\xff\xff\x7b", 0);
    }
    {
        __m256i __r = _mm256_subs_epu8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_subs_epu8", &__r, 32, "\x00\x00\x22\x5d\x0a\x7b\x5b\x27\x00\x00\x00\x00\x00\x7b\x00\xd1\x00\x01\x2c\x00\x00\x00\xb2\x1c\xff\xff\xff\x00\x3f\x52\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_adds_epu16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_adds_epu16", &__r, 32, "\xff\xff\xff\xff\xd0\x7c\xff\xff\xff\xff\x3e\xef\xff\xff\xff\xff\xff\xff\xd6\xb4\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x01\x7c", 0);
    }
    {
        __m256i __r = _mm256_subs_epu16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_subs_epu16", &__r, 32, "\x00\x00\x22\x5d\x0a\x7b\x5b\x27\x00\x00\x00\x00\xe5\x7a\xbc\xd0\x01\x00\x00\x00\x00\x00\xb2\x1c\xff\xff\x00\x00\x3f\x52\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_avg_epu8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_avg_epu8", &__r, 32, "\x52\x91\x7c\xd1\xe8\x3e\x52\xb0\x90\xad\x1f\x78\x0f\x9b\x5e\x96\x80\x80\x6b\x5a\xec\xa7\x7c\x8f\x80\x80\x80\x80\x46\x89\x81\x3e", 0);
    }
    {
        __m256i __r = _mm256_avg_epu16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_avg_epu16", &__r, 32, "\x52\x91\xfc\xd0\x68\x3e\xd2\xaf\x90\xad\x9f\x77\x8f\x9a\xde\x95\x00\x80\x6b\x5a\xec\xa7\x7c\x8f\x00\x80\x00\x80\x46\x89\x01\x3e", 0);
    }
    {
        __m256i __r = _mm256_mulhi_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_mulhi_epi16", &__r, 32, "\xb8\xf9\x2a\x00\x6d\x00\x99\x17\x7b\x12\xab\x37\x73\xf1\xaf\xff\x00\xc0\xec\xfd\xb5\xe8\xa5\x30\x00\x00\x00\xc0\xda\xe2\xe3\x0e", 0);
    }
    {
        __m256i __r = _mm256_mulhi_epu16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_mulhi_epu16", &__r, 32, "\xb8\x36\x22\xa2\x6d\x00\x3c\x77\x9b\x6d\xab\x37\x8f\x4e\x2f\x2d\xff\x3f\x6d\x04\x8e\x67\x9d\x4f\x00\x00\xff\x3f\x00\x43\xe3\x0e", 0);
    }
    {
        __m256i __r = _mm256_mullo_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_mullo_epi16", &__r, 32, "\x00\x14\xef\x09\x27\xe3\xdc\xe1\xdf\xba\x50\x7b\x1c\xfd\x00\xaa\x00\x80\xd5\xd6\x4e\x2b\x1f\xe9\x00\x00\x00\x80\xfe\x5a\xac\x61", 0);
    }
    {
        __m256i __r = _mm256_mul_epu32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_mul_epu32", &__r, 32, "\x00\x14\x0c\x8d\xdf\x15\x23\xa2\xdf\xba\xcd\x85\xd1\x1a\xac\x37\x00\x80\x7e\x39\x40\x31\x6e\x04\x00\x00\x00\x80\xff\xff\xff\x3f", 0);
    }
    {
        __m256i __r = _mm256_madd_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_madd_epi16", &__r, 32, "\xef\x1d\xe2\xf9\x03\xc5\x07\x18\x2f\x36\x27\x4a\x1c\xa7\x23\xf1\xd5\x56\xed\xbd\x6d\x14\x5b\x19\x00\x80\x00\xc0\xaa\xbc\xbd\xf1", 0);
    }
    {
        __m256i __r = _mm256_sad_epu8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_sad_epu8", &__r, 32, "\xd2\x02\x00\x00\x00\x00\x00\x00\x24\x03\x00\x00\x00\x00\x00\x00\x19\x03\x00\x00\x00\x00\x00\x00\x11\x04\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_min_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_min_epi16", &__r, 32, "\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x55\xae\xfe\xd0\x23\x81\xff\xff\x00\x80\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_max_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_max_epi16", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\xff\xda\x38\x7f\x1c\x5d\x80\x2d\xff\x7f\x81\x06\xd9\x7e\xd5\x9d\x00\x00\xff\x7f\x26\x60\xbc\x43", 0);
    }
    {
        __m256i __r = _mm256_min_epu8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_min_epu8", &__r, 32, "\x00\x3d\x6b\xa2\xe3\x00\x24\x9c\x21\x80\x06\x70\x01\x5d\x3c\x2d\x00\x7f\x55\x06\xd9\x7e\x23\x81\x00\x00\x00\x7f\x26\x60\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_max_epu8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_max_epu8", &__r, 32, "\xa4\xe5\x8d\xff\xed\x7b\x7f\xc3\xff\xda\x38\x7f\x1c\xd8\x80\xfe\xff\x80\x81\xae\xfe\xd0\xd5\x9d\xff\xff\xff\x80\x65\xb2\xbc\x43", 0);
    }
    {
        __m256i __r = _mm256_cmpeq_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_cmpeq_epi8", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_cmpgt_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_cmpgt_epi8", &__r, 32, "\xff\xff\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\xff\x00\xff\x00\x00\xff\x00\xff\x00\xff\x00\x00\x00\xff\xff\x00\xff\x00", 0);
    }
    {
        __m256i __r = _mm256_cmpeq_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_cmpeq_epi16", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_cmpgt_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_cmpgt_epi16", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\xff\xff\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("avx2,fma,f16c"))) void run1(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m256i __r = _mm256_cmpeq_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_cmpeq_epi32", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_cmpgt_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_cmpgt_epi32", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_and_si256(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_and_si256", &__r, 32, "\x00\x25\x09\xa2\xe1\x00\x24\x80\x21\x80\x00\x70\x00\x58\x00\x2c\x00\x00\x01\x06\xd8\x50\x01\x81\x00\x00\x00\x00\x24\x20\x04\x00", 0);
    }
    {
        __m256i __r = _mm256_andnot_si256(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_andnot_si256", &__r, 32, "\xa4\xc0\x62\x00\x02\x00\x00\x1c\xde\x5a\x38\x0f\x1c\x05\x80\x01\xff\x7f\x54\xa8\x26\x80\x22\x00\x00\x00\x00\x80\x02\x40\xb8\x43", 0);
    }
    {
        __m256i __r = _mm256_or_si256(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_or_si256", &__r, 32, "\xa4\xfd\xef\xff\xef\x7b\x7f\xdf\xff\xda\x3e\x7f\x1d\xdd\xbc\xff\xff\xff\xd5\xae\xff\xfe\xf7\x9d\xff\xff\xff\xff\x67\xf2\xfd\x7b", 0);
    }
    {
        __m256i __r = _mm256_xor_si256(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_xor_si256", &__r, 32, "\xa4\xd8\xe6\x5d\x0e\x7b\x5b\x5f\xde\x5a\x3e\x0f\x1d\x85\xbc\xd3\xff\xff\xd4\xa8\x27\xae\xf6\x1c\xff\xff\xff\xff\x43\xd2\xf9\x7b", 0);
    }
    {
        __m256i __r = _mm256_packs_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_packs_epi16", &__r, 32, "\x7f\x8d\x7f\x80\x80\x7f\x80\x80\x80\x80\x7f\x80\x80\x7f\x7f\x7f\x80\x7f\x7f\x80\xff\x7f\x80\x7f\x7f\x80\x80\x80\x00\x80\x7f\x7f", 0);
    }
    {
        __m256i __r = _mm256_packs_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_packs_epi32", &__r, 32, "\x00\x80\x00\x80\xff\x7f\x00\x80\x00\x80\x00\x80\xff\x7f\xff\x7f\xff\x7f\x00\x80\xff\x7f\xff\x7f\x00\x80\x00\x80\x00\x80\xff\x7f", 0);
    }
    {
        __m256i __r = _mm256_packus_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_packus_epi16", &__r, 32, "\xff\x00\xff\x00\x00\xff\x00\x00\x00\x00\xe3\x00\x00\xff\xff\xff\x00\xff\xff\x00\x00\xff\x00\xff\xff\x00\x00\x00\x00\x00\xff\xff", 0);
    }
    {
        __m256i __r = _mm256_unpacklo_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_unpacklo_epi8", &__r, 32, "\x00\xa4\x3d\xe5\x8d\x6b\xff\xa2\xed\xe3\x7b\x00\x7f\x24\xc3\x9c\x00\xff\x80\x7f\x81\x55\x06\xae\xd9\xfe\x7e\xd0\xd5\x23\x9d\x81", 0);
    }
    {
        __m256i __r = _mm256_unpackhi_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_unpackhi_epi8", &__r, 32, "\x21\xff\x80\xda\x06\x38\x70\x7f\x01\x1c\xd8\x5d\x3c\x80\xfe\x2d\xff\x00\xff\x00\xff\x00\x7f\x80\x65\x26\xb2\x60\x45\xbc\x38\x43", 0);
    }
    {
        __m256i __r = _mm256_unpacklo_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_unpacklo_epi16", &__r, 32, "\x00\x3d\xa4\xe5\x8d\xff\x6b\xa2\xed\x7b\xe3\x00\x7f\xc3\x24\x9c\x00\x80\xff\x7f\x81\x06\x55\xae\xd9\x7e\xfe\xd0\xd5\x9d\x23\x81", 0);
    }
    {
        __m256i __r = _mm256_unpackhi_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_unpackhi_epi16", &__r, 32, "\x21\x80\xff\xda\x06\x70\x38\x7f\x01\xd8\x1c\x5d\x3c\xfe\x80\x2d\xff\xff\x00\x00\xff\x7f\x00\x80\x65\xb2\x26\x60\x45\x38\xbc\x43", 0);
    }
    {
        __m256i __r = _mm256_unpacklo_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_unpacklo_epi32", &__r, 32, "\x00\x3d\x8d\xff\xa4\xe5\x6b\xa2\xed\x7b\x7f\xc3\xe3\x00\x24\x9c\x00\x80\x81\x06\xff\x7f\x55\xae\xd9\x7e\xd5\x9d\xfe\xd0\x23\x81", 0);
    }
    {
        __m256i __r = _mm256_unpackhi_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_unpackhi_epi32", &__r, 32, "\x21\x80\x06\x70\xff\xda\x38\x7f\x01\xd8\x3c\xfe\x1c\x5d\x80\x2d\xff\xff\xff\x7f\x00\x00\x00\x80\x65\xb2\x45\x38\x26\x60\xbc\x43", 0);
    }
    {
        __m256i __r = _mm256_unpacklo_epi64(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_unpacklo_epi64", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\x7f\x55\xae\xfe\xd0\x23\x81", 0);
    }
    {
        __m256i __r = _mm256_unpackhi_epi64(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_unpackhi_epi64", &__r, 32, "\x21\x80\x06\x70\x01\xd8\x3c\xfe\xff\xda\x38\x7f\x1c\x5d\x80\x2d\xff\xff\xff\x7f\x65\xb2\x45\x38\x00\x00\x00\x80\x26\x60\xbc\x43", 0);
    }
    {
        int __r = _mm256_movemask_epi8(__L(__m256i, A, 0));
        chk("_mm256_movemask_epi8", &__r, 4, "\x9c\xa2\xd6\x27", 0);
    }
    {
        __m256i __r = _mm256_slli_epi16(__L(__m256i, A, 0), 0);
        chk("_mm256_slli_epi16/0", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_srli_epi16(__L(__m256i, A, 0), 0);
        chk("_mm256_srli_epi16/0", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_srai_epi16(__L(__m256i, A, 0), 0);
        chk("_mm256_srai_epi16/0", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_slli_epi16(__L(__m256i, A, 0), 1);
        chk("_mm256_slli_epi16/1", &__r, 32, "\x00\x7a\x1a\xff\xda\xf7\xfe\x86\x42\x00\x0c\xe0\x02\xb0\x78\xfc\x00\x00\x02\x0d\xb2\xfd\xaa\x3b\xfe\xff\xfe\xff\xca\x64\x8a\x70", 0);
    }
    {
        __m256i __r = _mm256_srli_epi16(__L(__m256i, A, 0), 1);
        chk("_mm256_srli_epi16/1", &__r, 32, "\x80\x1e\xc6\x7f\xf6\x3d\xbf\x61\x10\x40\x03\x38\x00\x6c\x1e\x7f\x00\x40\x40\x03\x6c\x3f\xea\x4e\xff\x7f\xff\x3f\x32\x59\x22\x1c", 0);
    }
    {
        __m256i __r = _mm256_srai_epi16(__L(__m256i, A, 0), 1);
        chk("_mm256_srai_epi16/1", &__r, 32, "\x80\x1e\xc6\xff\xf6\x3d\xbf\xe1\x10\xc0\x03\x38\x00\xec\x1e\xff\x00\xc0\x40\x03\x6c\x3f\xea\xce\xff\xff\xff\x3f\x32\xd9\x22\x1c", 0);
    }
    {
        __m256i __r = _mm256_slli_epi16(__L(__m256i, A, 0), 15);
        chk("_mm256_slli_epi16/15", &__r, 32, "\x00\x00\x00\x80\x00\x80\x00\x80\x00\x80\x00\x00\x00\x80\x00\x00\x00\x00\x00\x80\x00\x80\x00\x80\x00\x80\x00\x80\x00\x80\x00\x80", 0);
    }
    {
        __m256i __r = _mm256_srli_epi16(__L(__m256i, A, 0), 15);
        chk("_mm256_srli_epi16/15", &__r, 32, "\x00\x00\x01\x00\x00\x00\x01\x00\x01\x00\x00\x00\x01\x00\x01\x00\x01\x00\x00\x00\x00\x00\x01\x00\x01\x00\x00\x00\x01\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srai_epi16(__L(__m256i, A, 0), 15);
        chk("_mm256_srai_epi16/15", &__r, 32, "\x00\x00\xff\xff\x00\x00\xff\xff\xff\xff\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\xff\xff\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_slli_epi16(__L(__m256i, A, 0), 16);
        chk("_mm256_slli_epi16/16", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srli_epi16(__L(__m256i, A, 0), 16);
        chk("_mm256_srli_epi16/16", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srai_epi16(__L(__m256i, A, 0), 16);
        chk("_mm256_srai_epi16/16", &__r, 32, "\x00\x00\xff\xff\x00\x00\xff\xff\xff\xff\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\xff\xff\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_slli_epi16(__L(__m256i, A, 0), 255);
        chk("_mm256_slli_epi16/255", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srli_epi16(__L(__m256i, A, 0), 255);
        chk("_mm256_srli_epi16/255", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("avx2,fma,f16c"))) void run2(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m256i __r = _mm256_srai_epi16(__L(__m256i, A, 0), 255);
        chk("_mm256_srai_epi16/255", &__r, 32, "\x00\x00\xff\xff\x00\x00\xff\xff\xff\xff\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\xff\xff\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_sll_epi16(__L(__m256i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm256_sll_epi16/3", &__r, 32, "\x00\xe8\x68\xfc\x68\xdf\xf8\x1b\x08\x01\x30\x80\x08\xc0\xe0\xf1\x00\x00\x08\x34\xc8\xf6\xa8\xee\xf8\xff\xf8\xff\x28\x93\x28\xc2", 0);
    }
    {
        __m256i __r = _mm256_srl_epi16(__L(__m256i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm256_srl_epi16/3", &__r, 32, "\xa0\x07\xf1\x1f\x7d\x0f\x6f\x18\x04\x10\x00\x0e\x00\x1b\xc7\x1f\x00\x10\xd0\x00\xdb\x0f\xba\x13\xff\x1f\xff\x0f\x4c\x16\x08\x07", 0);
    }
    {
        __m256i __r = _mm256_sra_epi16(__L(__m256i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm256_sra_epi16/3", &__r, 32, "\xa0\x07\xf1\xff\x7d\x0f\x6f\xf8\x04\xf0\x00\x0e\x00\xfb\xc7\xff\x00\xf0\xd0\x00\xdb\x0f\xba\xf3\xff\xff\xff\x0f\x4c\xf6\x08\x07", 0);
    }
    {
        __m256i __r = _mm256_sll_epi16(__L(__m256i, A, 0), _mm_set_epi64x(0, 17));
        chk("_mm256_sll_epi16/17", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srl_epi16(__L(__m256i, A, 0), _mm_set_epi64x(0, 17));
        chk("_mm256_srl_epi16/17", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_sra_epi16(__L(__m256i, A, 0), _mm_set_epi64x(0, 17));
        chk("_mm256_sra_epi16/17", &__r, 32, "\x00\x00\xff\xff\x00\x00\xff\xff\xff\xff\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\xff\xff\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_slli_epi32(__L(__m256i, A, 0), 0);
        chk("_mm256_slli_epi32/0", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_srli_epi32(__L(__m256i, A, 0), 0);
        chk("_mm256_srli_epi32/0", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_srai_epi32(__L(__m256i, A, 0), 0);
        chk("_mm256_srai_epi32/0", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_slli_epi32(__L(__m256i, A, 0), 1);
        chk("_mm256_slli_epi32/1", &__r, 32, "\x00\x7a\x1a\xff\xda\xf7\xfe\x86\x42\x00\x0d\xe0\x02\xb0\x79\xfc\x00\x00\x03\x0d\xb2\xfd\xaa\x3b\xfe\xff\xff\xff\xca\x64\x8b\x70", 0);
    }
    {
        __m256i __r = _mm256_srli_epi32(__L(__m256i, A, 0), 1);
        chk("_mm256_srli_epi32/1", &__r, 32, "\x80\x9e\xc6\x7f\xf6\xbd\xbf\x61\x10\x40\x03\x38\x00\x6c\x1e\x7f\x00\xc0\x40\x03\x6c\xbf\xea\x4e\xff\xff\xff\x3f\x32\xd9\x22\x1c", 0);
    }
    {
        __m256i __r = _mm256_srai_epi32(__L(__m256i, A, 0), 1);
        chk("_mm256_srai_epi32/1", &__r, 32, "\x80\x9e\xc6\xff\xf6\xbd\xbf\xe1\x10\x40\x03\x38\x00\x6c\x1e\xff\x00\xc0\x40\x03\x6c\xbf\xea\xce\xff\xff\xff\x3f\x32\xd9\x22\x1c", 0);
    }
    {
        __m256i __r = _mm256_slli_epi32(__L(__m256i, A, 0), 31);
        chk("_mm256_slli_epi32/31", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x00\x80", 0);
    }
    {
        __m256i __r = _mm256_srli_epi32(__L(__m256i, A, 0), 31);
        chk("_mm256_srli_epi32/31", &__r, 32, "\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srai_epi32(__L(__m256i, A, 0), 31);
        chk("_mm256_srai_epi32/31", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_slli_epi32(__L(__m256i, A, 0), 32);
        chk("_mm256_slli_epi32/32", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srli_epi32(__L(__m256i, A, 0), 32);
        chk("_mm256_srli_epi32/32", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srai_epi32(__L(__m256i, A, 0), 32);
        chk("_mm256_srai_epi32/32", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_slli_epi32(__L(__m256i, A, 0), 255);
        chk("_mm256_slli_epi32/255", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srli_epi32(__L(__m256i, A, 0), 255);
        chk("_mm256_srli_epi32/255", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srai_epi32(__L(__m256i, A, 0), 255);
        chk("_mm256_srai_epi32/255", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_sll_epi32(__L(__m256i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm256_sll_epi32/3", &__r, 32, "\x00\xe8\x69\xfc\x68\xdf\xfb\x1b\x08\x01\x34\x80\x08\xc0\xe6\xf1\x00\x00\x0c\x34\xc8\xf6\xab\xee\xf8\xff\xff\xff\x28\x93\x2d\xc2", 0);
    }
    {
        __m256i __r = _mm256_srl_epi32(__L(__m256i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm256_srl_epi32/3", &__r, 32, "\xa0\xa7\xf1\x1f\x7d\xef\x6f\x18\x04\xd0\x00\x0e\x00\x9b\xc7\x1f\x00\x30\xd0\x00\xdb\xaf\xba\x13\xff\xff\xff\x0f\x4c\xb6\x08\x07", 0);
    }
    {
        __m256i __r = _mm256_sra_epi32(__L(__m256i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm256_sra_epi32/3", &__r, 32, "\xa0\xa7\xf1\xff\x7d\xef\x6f\xf8\x04\xd0\x00\x0e\x00\x9b\xc7\xff\x00\x30\xd0\x00\xdb\xaf\xba\xf3\xff\xff\xff\x0f\x4c\xb6\x08\x07", 0);
    }
    {
        __m256i __r = _mm256_sll_epi32(__L(__m256i, A, 0), _mm_set_epi64x(0, 33));
        chk("_mm256_sll_epi32/33", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srl_epi32(__L(__m256i, A, 0), _mm_set_epi64x(0, 33));
        chk("_mm256_srl_epi32/33", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_sra_epi32(__L(__m256i, A, 0), _mm_set_epi64x(0, 33));
        chk("_mm256_sra_epi32/33", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_slli_epi64(__L(__m256i, A, 0), 0);
        chk("_mm256_slli_epi64/0", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_srli_epi64(__L(__m256i, A, 0), 0);
        chk("_mm256_srli_epi64/0", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_slli_epi64(__L(__m256i, A, 0), 1);
        chk("_mm256_slli_epi64/1", &__r, 32, "\x00\x7a\x1a\xff\xdb\xf7\xfe\x86\x42\x00\x0d\xe0\x02\xb0\x79\xfc\x00\x00\x03\x0d\xb2\xfd\xaa\x3b\xfe\xff\xff\xff\xca\x64\x8b\x70", 0);
    }
    {
        __m256i __r = _mm256_srli_epi64(__L(__m256i, A, 0), 1);
        chk("_mm256_srli_epi64/1", &__r, 32, "\x80\x9e\xc6\xff\xf6\xbd\xbf\x61\x10\x40\x03\xb8\x00\x6c\x1e\x7f\x00\xc0\x40\x83\x6c\xbf\xea\x4e\xff\xff\xff\xbf\x32\xd9\x22\x1c", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("avx2,fma,f16c"))) void run3(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m256i __r = _mm256_slli_epi64(__L(__m256i, A, 0), 63);
        chk("_mm256_slli_epi64/63", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m256i __r = _mm256_srli_epi64(__L(__m256i, A, 0), 63);
        chk("_mm256_srli_epi64/63", &__r, 32, "\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_slli_epi64(__L(__m256i, A, 0), 64);
        chk("_mm256_slli_epi64/64", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srli_epi64(__L(__m256i, A, 0), 64);
        chk("_mm256_srli_epi64/64", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_slli_epi64(__L(__m256i, A, 0), 255);
        chk("_mm256_slli_epi64/255", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srli_epi64(__L(__m256i, A, 0), 255);
        chk("_mm256_srli_epi64/255", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_sll_epi64(__L(__m256i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm256_sll_epi64/3", &__r, 32, "\x00\xe8\x69\xfc\x6f\xdf\xfb\x1b\x08\x01\x34\x80\x0b\xc0\xe6\xf1\x00\x00\x0c\x34\xc8\xf6\xab\xee\xf8\xff\xff\xff\x2b\x93\x2d\xc2", 0);
    }
    {
        __m256i __r = _mm256_srl_epi64(__L(__m256i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm256_srl_epi64/3", &__r, 32, "\xa0\xa7\xf1\xbf\x7d\xef\x6f\x18\x04\xd0\x00\x2e\x00\x9b\xc7\x1f\x00\x30\xd0\x20\xdb\xaf\xba\x13\xff\xff\xff\xaf\x4c\xb6\x08\x07", 0);
    }
    {
        __m256i __r = _mm256_sll_epi64(__L(__m256i, A, 0), _mm_set_epi64x(0, 65));
        chk("_mm256_sll_epi64/65", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srl_epi64(__L(__m256i, A, 0), _mm_set_epi64x(0, 65));
        chk("_mm256_srl_epi64/65", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_slli_si256(__L(__m256i, A, 0), 0);
        chk("_mm256_slli_si256/0", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_srli_si256(__L(__m256i, A, 0), 0);
        chk("_mm256_srli_si256/0", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_slli_si256(__L(__m256i, A, 0), 3);
        chk("_mm256_slli_si256/3", &__r, 32, "\x00\x00\x00\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\x00\x00\x00\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65", 0);
    }
    {
        __m256i __r = _mm256_srli_si256(__L(__m256i, A, 0), 3);
        chk("_mm256_srli_si256/3", &__r, 32, "\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x00\x00\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_slli_si256(__L(__m256i, A, 0), 15);
        chk("_mm256_slli_si256/15", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srli_si256(__L(__m256i, A, 0), 15);
        chk("_mm256_srli_si256/15", &__r, 32, "\xfe\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x38\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_slli_si256(__L(__m256i, A, 0), 16);
        chk("_mm256_slli_si256/16", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srli_si256(__L(__m256i, A, 0), 16);
        chk("_mm256_srli_si256/16", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_shuffle_epi32(__L(__m256i, A, 0), 0x1b);
        chk("_mm256_shuffle_epi32/1b", &__r, 32, "\x01\xd8\x3c\xfe\x21\x80\x06\x70\xed\x7b\x7f\xc3\x00\x3d\x8d\xff\x65\xb2\x45\x38\xff\xff\xff\x7f\xd9\x7e\xd5\x9d\x00\x80\x81\x06", 0);
    }
    {
        __m256i __r = _mm256_shufflelo_epi16(__L(__m256i, A, 0), 0x1b);
        chk("_mm256_shufflelo_epi16/1b", &__r, 32, "\x7f\xc3\xed\x7b\x8d\xff\x00\x3d\x21\x80\x06\x70\x01\xd8\x3c\xfe\xd5\x9d\xd9\x7e\x81\x06\x00\x80\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_shufflehi_epi16(__L(__m256i, A, 0), 0x1b);
        chk("_mm256_shufflehi_epi16/1b", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x3c\xfe\x01\xd8\x06\x70\x21\x80\x00\x80\x81\x06\xd9\x7e\xd5\x9d\x45\x38\x65\xb2\xff\x7f\xff\xff", 0);
    }
    {
        __m256i __r = _mm256_shuffle_epi32(__L(__m256i, A, 0), 0xe4);
        chk("_mm256_shuffle_epi32/e4", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_shufflelo_epi16(__L(__m256i, A, 0), 0xe4);
        chk("_mm256_shufflelo_epi16/e4", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_shufflehi_epi16(__L(__m256i, A, 0), 0xe4);
        chk("_mm256_shufflehi_epi16/e4", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_shuffle_epi32(__L(__m256i, A, 0), 0x4e);
        chk("_mm256_shuffle_epi32/4e", &__r, 32, "\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\xff\xff\xff\x7f\x65\xb2\x45\x38\x00\x80\x81\x06\xd9\x7e\xd5\x9d", 0);
    }
    {
        __m256i __r = _mm256_shufflelo_epi16(__L(__m256i, A, 0), 0x4e);
        chk("_mm256_shufflelo_epi16/4e", &__r, 32, "\xed\x7b\x7f\xc3\x00\x3d\x8d\xff\x21\x80\x06\x70\x01\xd8\x3c\xfe\xd9\x7e\xd5\x9d\x00\x80\x81\x06\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_shufflehi_epi16(__L(__m256i, A, 0), 0x4e);
        chk("_mm256_shufflehi_epi16/4e", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x01\xd8\x3c\xfe\x21\x80\x06\x70\x00\x80\x81\x06\xd9\x7e\xd5\x9d\x65\xb2\x45\x38\xff\xff\xff\x7f", 0);
    }
    {
        __m256i __r = _mm256_abs_epi8(__L(__m256i, A, 0));
        chk("_mm256_abs_epi8", &__r, 32, "\x00\x3d\x73\x01\x13\x7b\x7f\x3d\x21\x80\x06\x70\x01\x28\x3c\x02\x00\x80\x7f\x06\x27\x7e\x2b\x63\x01\x01\x01\x7f\x65\x4e\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_sign_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_sign_epi8", &__r, 32, "\x00\xc3\x8d\x01\x13\x00\x7f\x3d\xdf\x80\x06\x70\x01\xd8\xc4\xfe\x00\x80\x81\xfa\x27\x82\xd5\x63\x00\x00\x00\x81\x65\xb2\xbb\x38", 0);
    }
    {
        __m256i __r = _mm256_abs_epi16(__L(__m256i, A, 0));
        chk("_mm256_abs_epi16", &__r, 32, "\x00\x3d\x73\x00\xed\x7b\x81\x3c\xdf\x7f\x06\x70\xff\x27\xc4\x01\x00\x80\x81\x06\xd9\x7e\x2b\x62\x01\x00\xff\x7f\x9b\x4d\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_sign_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_sign_epi16", &__r, 32, "\x00\xc3\x73\x00\xed\x7b\x81\x3c\xdf\x7f\x06\x70\x01\xd8\x3c\xfe\x00\x80\x7f\xf9\x27\x81\x2b\x62\x00\x00\x01\x80\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_abs_epi32(__L(__m256i, A, 0));
        chk("_mm256_abs_epi32", &__r, 32, "\x00\xc3\x72\x00\x13\x84\x80\x3c\x21\x80\x06\x70\xff\x27\xc3\x01\x00\x80\x81\x06\x27\x81\x2a\x62\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("avx2,fma,f16c"))) void run4(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m256i __r = _mm256_sign_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_sign_epi32", &__r, 32, "\x00\xc3\x72\x00\x13\x84\x80\x3c\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x7e\xf9\x27\x81\x2a\x62\x01\x00\x00\x80\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_hadd_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_hadd_epi16", &__r, 32, "\x8d\x3c\x6c\x3f\x27\xf0\x3d\xd6\x0f\x88\x07\x9d\x37\x5a\x9c\x8a\x81\x86\xae\x1c\xfe\x7f\xaa\xea\x54\x2e\x21\x52\x00\x80\xe2\xa3", 0);
    }
    {
        __m256i __r = _mm256_hadd_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_hadd_epi32", &__r, 32, "\xed\xb8\x0c\xc3\x22\x58\x43\x6e\x87\xe6\x8f\x3e\x1b\x38\xb9\xac\xd9\xfe\x56\xa4\x64\xb2\x45\xb8\xfd\x50\x79\x2f\x26\x60\xbc\xc3", 0);
    }
    {
        __m256i __r = _mm256_hadds_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_hadds_epi16", &__r, 32, "\x8d\x3c\x6c\x3f\x27\xf0\x3d\xd6\x0f\x88\x07\x9d\x37\x5a\xff\x7f\x81\x86\xae\x1c\xfe\x7f\xaa\xea\x54\x2e\x00\x80\x00\x80\xff\x7f", 0);
    }
    {
        __m256i __r = _mm256_hsub_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_hsub_epi16", &__r, 32, "\x73\x3d\x6e\xb8\x1b\x10\xc5\xd9\x39\x43\xbf\x64\xc7\x5b\x9c\x2f\x7f\x79\x04\xe1\x00\x80\x20\x7a\xaa\xd1\xdb\x4f\x00\x80\x6a\x1c", 0);
    }
    {
        __m256i __r = _mm256_hsub_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_hsub_epi32", &__r, 32, "\x13\xc1\x0d\x3c\x20\xa8\xc9\x71\xc1\xe4\x47\x06\xe3\x7d\xb8\x51\x27\x01\xac\x68\x9a\x4d\xba\x47\x01\xaf\x31\x2d\xda\x9f\x43\x3c", 0);
    }
    {
        __m256i __r = _mm256_hsubs_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_hsubs_epi16", &__r, 32, "\x73\x3d\xff\x7f\x00\x80\xc5\xd9\x39\x43\xbf\x64\x00\x80\x9c\x2f\x00\x80\xff\x7f\x00\x80\x00\x80\xff\x7f\xdb\x4f\xff\x7f\x6a\x1c", 0);
    }
    {
        __m256i __r = _mm256_maddubs_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_maddubs_epi16", &__r, 32, "\x91\xf9\x4d\xdd\x27\xe5\xb0\xc5\xdf\xec\xe0\x38\x94\x4e\xa6\x0e\x80\x3f\xe9\x28\xae\xe6\x3c\xcf\x00\x00\x80\xc0\xbe\x51\x54\xfc", 0);
    }
    {
        __m256i __r = _mm256_mulhrs_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_mulhrs_epi16", &__r, 32, "\x70\xf3\x54\x00\xdc\x00\x34\x2f\xf7\x24\x57\x6f\xe8\xe2\x5f\xff\x01\x80\xda\xfb\x6a\xd1\x4c\x61\x00\x00\x01\x80\xb5\xc5\xc7\x1d", 0);
    }
    {
        __m256i __r = _mm256_shuffle_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_shuffle_epi8", &__r, 32, "\x00\x00\x70\x00\x00\x00\xed\x00\x00\x00\x21\xfe\x01\xd8\x00\xd8\x00\x38\x7e\x00\x00\x00\x06\x00\x00\x00\x00\x00\xd5\x00\x00\x06", 0);
    }
    {
        __m256i __r = _mm256_alignr_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0), 0);
        chk("_mm256_alignr_epi8/0", &__r, 32, "\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c\xff\xda\x38\x7f\x1c\x5d\x80\x2d\xff\x7f\x55\xae\xfe\xd0\x23\x81\x00\x00\x00\x80\x26\x60\xbc\x43", 0);
    }
    {
        __m256i __r = _mm256_alignr_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0), 5);
        chk("_mm256_alignr_epi8/5", &__r, 32, "\x00\x24\x9c\xff\xda\x38\x7f\x1c\x5d\x80\x2d\x00\x3d\x8d\xff\xed\xd0\x23\x81\x00\x00\x00\x80\x26\x60\xbc\x43\x00\x80\x81\x06\xd9", 0);
    }
    {
        __m256i __r = _mm256_alignr_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0), 16);
        chk("_mm256_alignr_epi8/16", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_alignr_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0), 20);
        chk("_mm256_alignr_epi8/20", &__r, 32, "\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x00\x00\x00\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_alignr_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0), 32);
        chk("_mm256_alignr_epi8/32", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_cmpeq_epi64(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_cmpeq_epi64", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_min_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_min_epi8", &__r, 32, "\xa4\xe5\x8d\xa2\xe3\x00\x24\x9c\xff\x80\x06\x70\x01\xd8\x80\xfe\xff\x80\x81\xae\xd9\xd0\xd5\x81\xff\xff\xff\x80\x26\xb2\xbc\x38", 0);
    }
    {
        __m256i __r = _mm256_max_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_max_epi8", &__r, 32, "\x00\x3d\x6b\xff\xed\x7b\x7f\xc3\x21\xda\x38\x7f\x1c\x5d\x3c\x2d\x00\x7f\x55\x06\xfe\x7e\x23\x9d\x00\x00\x00\x7f\x65\x60\x45\x43", 0);
    }
    {
        __m256i __r = _mm256_min_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_min_epi32", &__r, 32, "\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c\x21\x80\x06\x70\x01\xd8\x3c\xfe\xff\x7f\x55\xae\xfe\xd0\x23\x81\x00\x00\x00\x80\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_max_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_max_epi32", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\xff\xda\x38\x7f\x1c\x5d\x80\x2d\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x26\x60\xbc\x43", 0);
    }
    {
        __m256i __r = _mm256_min_epu16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_min_epu16", &__r, 32, "\x00\x3d\x6b\xa2\xe3\x00\x24\x9c\x21\x80\x06\x70\x1c\x5d\x80\x2d\xff\x7f\x81\x06\xd9\x7e\x23\x81\x00\x00\xff\x7f\x26\x60\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_max_epu16(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_max_epu16", &__r, 32, "\xa4\xe5\x8d\xff\xed\x7b\x7f\xc3\xff\xda\x38\x7f\x01\xd8\x3c\xfe\x00\x80\x55\xae\xfe\xd0\xd5\x9d\xff\xff\x00\x80\x65\xb2\xbc\x43", 0);
    }
    {
        __m256i __r = _mm256_min_epu32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_min_epu32", &__r, 32, "\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c\x21\x80\x06\x70\x1c\x5d\x80\x2d\x00\x80\x81\x06\xfe\xd0\x23\x81\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_max_epu32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_max_epu32", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\xff\xda\x38\x7f\x01\xd8\x3c\xfe\xff\x7f\x55\xae\xd9\x7e\xd5\x9d\x00\x00\x00\x80\x26\x60\xbc\x43", 0);
    }
    {
        __m256i __r = _mm256_mul_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_mul_epi32", &__r, 32, "\x00\x14\x0c\x8d\x3b\xf3\x29\x00\xdf\xba\xcd\x85\xd1\x1a\xac\x37\x00\x80\x7e\x39\x40\xb1\xec\xfd\x00\x00\x00\x80\x00\x00\x00\xc0", 0);
    }
    {
        __m256i __r = _mm256_mullo_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_mullo_epi32", &__r, 32, "\x00\x14\x0c\x8d\x27\xe3\x5e\x33\xdf\xba\xcd\x85\x1c\xfd\x9f\x16\x00\x80\x7e\x39\x4e\x2b\x8f\xc1\x00\x00\x00\x80\xfe\x5a\x6a\xee", 0);
    }
    {
        __m256i __r = _mm256_packus_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_packus_epi32", &__r, 32, "\x00\x00\x00\x00\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff", 0);
    }
    {
        __m256i __r = _mm256_blendv_epi8(__L(__m256i, A, 0), __L(__m256i, B, 0), __L(__m256i, C, 0));
        chk("_mm256_blendv_epi8", &__r, 32, "\x00\xe5\x8d\xa2\xe3\x00\x24\x9c\xff\xda\x06\x70\x1c\x5d\x80\xfe\x00\x7f\x55\xae\xd9\x7e\xd5\x81\xff\xff\xff\x80\x26\x60\xbc\x38", 0);
    }
    {
        __m256i __r = _mm256_blend_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0), 0x0);
        chk("_mm256_blend_epi16/0", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_blend_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0), 0xa5);
        chk("_mm256_blend_epi16/a5", &__r, 32, "\xa4\xe5\x8d\xff\xe3\x00\x7f\xc3\x21\x80\x38\x7f\x01\xd8\x80\x2d\xff\x7f\x81\x06\xfe\xd0\xd5\x9d\xff\xff\x00\x80\x65\xb2\xbc\x43", 0);
    }
    {
        __m256i __r = _mm256_blend_epi16(__L(__m256i, A, 0), __L(__m256i, B, 0), 0xff);
        chk("_mm256_blend_epi16/ff", &__r, 32, "\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c\xff\xda\x38\x7f\x1c\x5d\x80\x2d\xff\x7f\x55\xae\xfe\xd0\x23\x81\x00\x00\x00\x80\x26\x60\xbc\x43", 0);
    }
    {
        __m256i __r = _mm256_mpsadbw_epu8(__L(__m256i, A, 0), __L(__m256i, B, 0), 0);
        chk("_mm256_mpsadbw_epu8/0", &__r, 32, "\xcb\x01\x9e\x01\xda\x00\x96\x00\xe8\x00\x68\x01\xb3\x00\x94\x01\xd4\x01\xfb\x00\xab\x01\xa3\x01\xb8\x00\x70\x01\x43\x01\xdd\x01", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("avx2,fma,f16c"))) void run5(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m256i __r = _mm256_mpsadbw_epu8(__L(__m256i, A, 0), __L(__m256i, B, 0), 5);
        chk("_mm256_mpsadbw_epu8/5", &__r, 32, "\x07\x01\x01\x02\x46\x01\x33\x01\x8c\x01\x50\x01\xac\x01\x88\x01\xd4\x01\xfb\x00\xab\x01\xa3\x01\xb8\x00\x70\x01\x43\x01\xdd\x01", 0);
    }
    {
        __m256i __r = _mm256_mpsadbw_epu8(__L(__m256i, A, 0), __L(__m256i, B, 0), 7);
        chk("_mm256_mpsadbw_epu8/7", &__r, 32, "\x86\x01\xd0\x00\x7b\x01\x0a\x01\xe5\x00\xf7\x00\x53\x01\x17\x01\xd4\x01\xfb\x00\xab\x01\xa3\x01\xb8\x00\x70\x01\x43\x01\xdd\x01", 0);
    }
    {
        __m256i __r = _mm256_cvtepi8_epi16(__L(__m128i, A, 0));
        chk("_mm256_cvtepi8_epi16", &__r, 32, "\x00\x00\x3d\x00\x8d\xff\xff\xff\xed\xff\x7b\x00\x7f\x00\xc3\xff\x21\x00\x80\xff\x06\x00\x70\x00\x01\x00\xd8\xff\x3c\x00\xfe\xff", 0);
    }
    {
        __m256i __r = _mm256_cvtepi8_epi32(__L(__m128i, A, 0));
        chk("_mm256_cvtepi8_epi32", &__r, 32, "\x00\x00\x00\x00\x3d\x00\x00\x00\x8d\xff\xff\xff\xff\xff\xff\xff\xed\xff\xff\xff\x7b\x00\x00\x00\x7f\x00\x00\x00\xc3\xff\xff\xff", 0);
    }
    {
        __m256i __r = _mm256_cvtepi8_epi64(__L(__m128i, A, 0));
        chk("_mm256_cvtepi8_epi64", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x3d\x00\x00\x00\x00\x00\x00\x00\x8d\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256i __r = _mm256_cvtepi16_epi32(__L(__m128i, A, 0));
        chk("_mm256_cvtepi16_epi32", &__r, 32, "\x00\x3d\x00\x00\x8d\xff\xff\xff\xed\x7b\x00\x00\x7f\xc3\xff\xff\x21\x80\xff\xff\x06\x70\x00\x00\x01\xd8\xff\xff\x3c\xfe\xff\xff", 0);
    }
    {
        __m256i __r = _mm256_cvtepi16_epi64(__L(__m128i, A, 0));
        chk("_mm256_cvtepi16_epi64", &__r, 32, "\x00\x3d\x00\x00\x00\x00\x00\x00\x8d\xff\xff\xff\xff\xff\xff\xff\xed\x7b\x00\x00\x00\x00\x00\x00\x7f\xc3\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256i __r = _mm256_cvtepi32_epi64(__L(__m128i, A, 0));
        chk("_mm256_cvtepi32_epi64", &__r, 32, "\x00\x3d\x8d\xff\xff\xff\xff\xff\xed\x7b\x7f\xc3\xff\xff\xff\xff\x21\x80\x06\x70\x00\x00\x00\x00\x01\xd8\x3c\xfe\xff\xff\xff\xff", 0);
    }
    {
        __m256i __r = _mm256_cvtepu8_epi16(__L(__m128i, A, 0));
        chk("_mm256_cvtepu8_epi16", &__r, 32, "\x00\x00\x3d\x00\x8d\x00\xff\x00\xed\x00\x7b\x00\x7f\x00\xc3\x00\x21\x00\x80\x00\x06\x00\x70\x00\x01\x00\xd8\x00\x3c\x00\xfe\x00", 0);
    }
    {
        __m256i __r = _mm256_cvtepu8_epi32(__L(__m128i, A, 0));
        chk("_mm256_cvtepu8_epi32", &__r, 32, "\x00\x00\x00\x00\x3d\x00\x00\x00\x8d\x00\x00\x00\xff\x00\x00\x00\xed\x00\x00\x00\x7b\x00\x00\x00\x7f\x00\x00\x00\xc3\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_cvtepu8_epi64(__L(__m128i, A, 0));
        chk("_mm256_cvtepu8_epi64", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x3d\x00\x00\x00\x00\x00\x00\x00\x8d\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_cvtepu16_epi32(__L(__m128i, A, 0));
        chk("_mm256_cvtepu16_epi32", &__r, 32, "\x00\x3d\x00\x00\x8d\xff\x00\x00\xed\x7b\x00\x00\x7f\xc3\x00\x00\x21\x80\x00\x00\x06\x70\x00\x00\x01\xd8\x00\x00\x3c\xfe\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_cvtepu16_epi64(__L(__m128i, A, 0));
        chk("_mm256_cvtepu16_epi64", &__r, 32, "\x00\x3d\x00\x00\x00\x00\x00\x00\x8d\xff\x00\x00\x00\x00\x00\x00\xed\x7b\x00\x00\x00\x00\x00\x00\x7f\xc3\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_cvtepu32_epi64(__L(__m128i, A, 0));
        chk("_mm256_cvtepu32_epi64", &__r, 32, "\x00\x3d\x8d\xff\x00\x00\x00\x00\xed\x7b\x7f\xc3\x00\x00\x00\x00\x21\x80\x06\x70\x00\x00\x00\x00\x01\xd8\x3c\xfe\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_add_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_add_ps", &__r, 32, "\x00\x00\x80\xff\x01\x00\xc0\x7f\x00\x00\x80\x3f\x9e\xc9\x7f\x7f\x00\x00\x80\x7f\x00\x00\x80\xff\x01\x00\xc0\x7f\xca\xf2\x49\x71", 0);
    }
    {
        __m256 __r = _mm256_sub_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_sub_ps", &__r, 32, "\x00\x00\x80\x7f\x01\x00\xc0\x7f\x00\x00\x80\x3f\x9e\xc9\x7f\xff\x00\x00\x80\x7f\x00\x00\x80\xff\x01\x00\xc0\x7f\xca\xf2\x49\xf1", 0);
    }
    {
        __m256 __r = _mm256_mul_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_mul_ps", &__r, 32, "\x00\x00\xc0\xff\x01\x00\xc0\x7f\x01\x00\x00\x00\x00\x00\x80\xff\x00\x00\x80\x7f\x00\x00\x80\x7f\x01\x00\xc0\x7f\xca\xf2\xc9\x26", 0);
    }
    {
        __m256 __r = _mm256_div_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_div_ps", &__r, 32, "\x00\x00\x00\x80\x01\x00\xc0\x7f\x00\x00\x80\x7f\x35\x0a\x30\x80\x00\x00\x80\x7f\x00\x00\x80\x7f\x01\x00\xc0\x7f\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_sqrt_ps(__L(__m256, FA, 0));
        chk("_mm256_sqrt_ps", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xff\x00\x00\x80\x7f\x00\x00\xc0\xff\x01\x00\xc0\x7f\xf3\x04\x35\x1a", 0);
    }
    {
        __m256 __r = _mm256_min_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_min_ps", &__r, 32, "\x00\x00\x80\xff\x01\x00\xc0\x7f\x01\x00\x00\x00\x00\x00\xc0\xbf\x00\x00\x20\x40\x00\x00\x80\xff\x00\x00\x00\x3f\x01\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_max_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_max_ps", &__r, 32, "\x00\x00\x00\x00\x01\x00\xc0\x7f\x00\x00\x80\x3f\x9e\xc9\x7f\x7f\x00\x00\x80\x7f\x00\x00\x20\xc0\x00\x00\x00\x3f\xca\xf2\x49\x71", 0);
    }
    {
        __m256 __r = _mm256_and_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_and_ps", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x40\x3f\x00\x00\x00\x40\x00\x00\x00\xc0\x00\x00\x00\x3f\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_or_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_or_ps", &__r, 32, "\x00\x00\x80\xff\x01\x00\xc0\xff\x01\x00\x80\x3f\x9e\xc9\xff\xff\x00\x00\xa0\x7f\x00\x00\xa0\xff\x01\x00\xc0\x7f\xcb\xf2\x49\x71", 0);
    }
    {
        __m256 __r = _mm256_xor_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_xor_ps", &__r, 32, "\x00\x00\x80\xff\x01\x00\xc0\xff\x01\x00\x80\x3f\x9e\xc9\xbf\xc0\x00\x00\xa0\x3f\x00\x00\xa0\x3f\x01\x00\xc0\x40\xcb\xf2\x49\x71", 0);
    }
    {
        __m256 __r = _mm256_andnot_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_andnot_ps", &__r, 32, "\x00\x00\x80\xff\x01\x00\xc0\x7f\x01\x00\x00\x00\x9e\xc9\x3f\x40\x00\x00\x20\x00\x00\x00\x20\x00\x00\x00\x00\x00\xca\xf2\x49\x71", 0);
    }
    {
        int __r = _mm256_movemask_ps(__L(__m256, FA, 0));
        chk("_mm256_movemask_ps", &__r, 4, "\x2a\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_unpacklo_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_unpacklo_ps", &__r, 32, "\x00\x00\x00\x00\x00\x00\x80\xff\x00\x00\x00\x80\x01\x00\xc0\x7f\x00\x00\x80\x7f\x00\x00\x20\x40\x00\x00\x80\xff\x00\x00\x20\xc0", 0);
    }
    {
        __m256 __r = _mm256_unpackhi_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_unpackhi_ps", &__r, 32, "\x00\x00\x80\x3f\x01\x00\x00\x00\x00\x00\xc0\xbf\x9e\xc9\x7f\x7f\x01\x00\xc0\x7f\x00\x00\x00\x3f\x01\x00\x00\x00\xca\xf2\x49\x71", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 0);
        chk("_mm256_cmp_ps/0", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 1);
        chk("_mm256_cmp_ps/1", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 2);
        chk("_mm256_cmp_ps/2", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 3);
        chk("_mm256_cmp_ps/3", &__r, 32, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("avx2,fma,f16c"))) void run6(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 4);
        chk("_mm256_cmp_ps/4", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 5);
        chk("_mm256_cmp_ps/5", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 6);
        chk("_mm256_cmp_ps/6", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 7);
        chk("_mm256_cmp_ps/7", &__r, 32, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 8);
        chk("_mm256_cmp_ps/8", &__r, 32, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 9);
        chk("_mm256_cmp_ps/9", &__r, 32, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 10);
        chk("_mm256_cmp_ps/10", &__r, 32, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 11);
        chk("_mm256_cmp_ps/11", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 12);
        chk("_mm256_cmp_ps/12", &__r, 32, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 13);
        chk("_mm256_cmp_ps/13", &__r, 32, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 14);
        chk("_mm256_cmp_ps/14", &__r, 32, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 15);
        chk("_mm256_cmp_ps/15", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 16);
        chk("_mm256_cmp_ps/16", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 17);
        chk("_mm256_cmp_ps/17", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 18);
        chk("_mm256_cmp_ps/18", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 19);
        chk("_mm256_cmp_ps/19", &__r, 32, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 20);
        chk("_mm256_cmp_ps/20", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 21);
        chk("_mm256_cmp_ps/21", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 22);
        chk("_mm256_cmp_ps/22", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 23);
        chk("_mm256_cmp_ps/23", &__r, 32, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 24);
        chk("_mm256_cmp_ps/24", &__r, 32, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 25);
        chk("_mm256_cmp_ps/25", &__r, 32, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 26);
        chk("_mm256_cmp_ps/26", &__r, 32, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 27);
        chk("_mm256_cmp_ps/27", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 28);
        chk("_mm256_cmp_ps/28", &__r, 32, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 29);
        chk("_mm256_cmp_ps/29", &__r, 32, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 30);
        chk("_mm256_cmp_ps/30", &__r, 32, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cmp_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), 31);
        chk("_mm256_cmp_ps/31", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256 __r = _mm256_addsub_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_addsub_ps", &__r, 32, "\x00\x00\x80\x7f\x01\x00\xc0\x7f\x00\x00\x80\x3f\x9e\xc9\x7f\x7f\x00\x00\x80\x7f\x00\x00\x80\xff\x01\x00\xc0\x7f\xca\xf2\x49\x71", 0);
    }
    {
        __m256 __r = _mm256_hadd_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_hadd_ps", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\xbf\x01\x00\xc0\x7f\x9e\xc9\x7f\x7f\x00\x00\xc0\xff\x01\x00\xc0\x7f\x00\x00\x00\x00\xca\xf2\x49\x71", 0);
    }
    {
        __m256 __r = _mm256_hsub_ps(__L(__m256, FA, 0), __L(__m256, FB, 0));
        chk("_mm256_hsub_ps", &__r, 32, "\x00\x00\x00\x00\x00\x00\x20\x40\x01\x00\xc0\x7f\x9e\xc9\x7f\xff\x00\x00\x80\x7f\x01\x00\xc0\x7f\x00\x00\xa0\x40\xca\xf2\x49\xf1", 0);
    }
    {
        __m256 __r = _mm256_round_ps(__L(__m256, FA, 0), 8);
        chk("_mm256_round_ps/0", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\x00\xc0\x00\x00\x80\x7f\x00\x00\x80\xff\x01\x00\xc0\x7f\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("avx2,fma,f16c"))) void run7(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m256 __r = _mm256_round_ps(__L(__m256, FA, 0), 9);
        chk("_mm256_round_ps/1", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\x00\xc0\x00\x00\x80\x7f\x00\x00\x80\xff\x01\x00\xc0\x7f\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_round_ps(__L(__m256, FA, 0), 10);
        chk("_mm256_round_ps/2", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\x80\xbf\x00\x00\x80\x7f\x00\x00\x80\xff\x01\x00\xc0\x7f\x00\x00\x80\x3f", 0);
    }
    {
        __m256 __r = _mm256_round_ps(__L(__m256, FA, 0), 11);
        chk("_mm256_round_ps/3", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\x80\xbf\x00\x00\x80\x7f\x00\x00\x80\xff\x01\x00\xc0\x7f\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_floor_ps(__L(__m256, FA, 0));
        chk("_mm256_floor_ps", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\x00\xc0\x00\x00\x80\x7f\x00\x00\x80\xff\x01\x00\xc0\x7f\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_ceil_ps(__L(__m256, FA, 0));
        chk("_mm256_ceil_ps", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\x80\xbf\x00\x00\x80\x7f\x00\x00\x80\xff\x01\x00\xc0\x7f\x00\x00\x80\x3f", 0);
    }
    {
        __m256 __r = _mm256_blendv_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), __L(__m256, FC, 0));
        chk("_mm256_blendv_ps", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x80\x01\x00\x00\x00\x00\x00\xc0\xbf\x00\x00\x80\x7f\x00\x00\x80\xff\x00\x00\x00\x3f\x01\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_fmadd_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), __L(__m256, FC, 0));
        chk("_mm256_fmadd_ps", &__r, 32, "\x00\x00\xc0\xff\x01\x00\xc0\x7f\x00\x00\xe8\xc0\x00\x00\x80\xff\x00\x00\x80\x7f\x00\x00\x80\x7f\x01\x00\xc0\x7f\x00\x00\x80\x3f", 0);
    }
    {
        __m256 __r = _mm256_fmsub_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), __L(__m256, FC, 0));
        chk("_mm256_fmsub_ps", &__r, 32, "\x00\x00\xc0\xff\x01\x00\xc0\x7f\x00\x00\xe8\x40\x00\x00\x80\xff\x00\x00\x80\x7f\x00\x00\x80\x7f\x01\x00\xc0\x7f\x00\x00\x80\xbf", 0);
    }
    {
        __m256 __r = _mm256_fnmadd_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), __L(__m256, FC, 0));
        chk("_mm256_fnmadd_ps", &__r, 32, "\x00\x00\xc0\xff\x01\x00\xc0\x7f\x00\x00\xe8\xc0\x00\x00\x80\x7f\x00\x00\x80\xff\x00\x00\x80\xff\x01\x00\xc0\x7f\x00\x00\x80\x3f", 0);
    }
    {
        __m256 __r = _mm256_fnmsub_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), __L(__m256, FC, 0));
        chk("_mm256_fnmsub_ps", &__r, 32, "\x00\x00\xc0\xff\x01\x00\xc0\x7f\x00\x00\xe8\x40\x00\x00\x80\x7f\x00\x00\x80\xff\x00\x00\x80\xff\x01\x00\xc0\x7f\x00\x00\x80\xbf", 0);
    }
    {
        __m256 __r = _mm256_fmaddsub_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), __L(__m256, FC, 0));
        chk("_mm256_fmaddsub_ps", &__r, 32, "\x00\x00\xc0\xff\x01\x00\xc0\x7f\x00\x00\xe8\x40\x00\x00\x80\xff\x00\x00\x80\x7f\x00\x00\x80\x7f\x01\x00\xc0\x7f\x00\x00\x80\x3f", 0);
    }
    {
        __m256 __r = _mm256_fmsubadd_ps(__L(__m256, FA, 0), __L(__m256, FB, 0), __L(__m256, FC, 0));
        chk("_mm256_fmsubadd_ps", &__r, 32, "\x00\x00\xc0\xff\x01\x00\xc0\x7f\x00\x00\xe8\xc0\x00\x00\x80\xff\x00\x00\x80\x7f\x00\x00\x80\x7f\x01\x00\xc0\x7f\x00\x00\x80\xbf", 0);
    }
    {
        __m256d __r = _mm256_add_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_add_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\xf8\xbf\x00\x00\x00\x00\x00\x00\xf0\x7f\x01\x00\x00\x00\x00\x00\xf8\x7f\x00\x00\x00\x00\x00\x00\xf8\xbf", 0);
    }
    {
        __m256d __r = _mm256_sub_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_sub_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\xf8\x3f\x00\x00\x00\x00\x00\x00\xf0\xff\x01\x00\x00\x00\x00\x00\xf8\x7f\x00\x00\x00\x00\x00\x00\xf8\xbf", 0);
    }
    {
        __m256d __r = _mm256_mul_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_mul_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf8\xff\x01\x00\x00\x00\x00\x00\xf8\x7f\x02\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m256d __r = _mm256_div_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_div_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x80\x01\x00\x00\x00\x00\x00\xf8\x7f\x00\x00\x00\x00\x00\x00\xf0\xff", 0);
    }
    {
        __m256d __r = _mm256_sqrt_pd(__L(__m256d, DA, 0));
        chk("_mm256_sqrt_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00\x00\xf8\xff", 0);
    }
    {
        __m256d __r = _mm256_min_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_min_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\xf8\xbf\x00\x00\x00\x00\x00\x00\x00\x80\x01\x00\x00\x00\x00\x00\xf8\x7f\x00\x00\x00\x00\x00\x00\xf8\xbf", 0);
    }
    {
        __m256d __r = _mm256_max_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_max_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x7f\x01\x00\x00\x00\x00\x00\xf8\x7f\x01\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_and_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_and_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_or_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_or_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\xf8\xbf\x00\x00\x00\x00\x00\x00\xf0\xff\x01\x00\x00\x00\x00\x00\xf8\x7f\x01\x00\x00\x00\x00\x00\xf8\xbf", 0);
    }
    {
        __m256d __r = _mm256_xor_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_xor_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\xf8\xbf\x00\x00\x00\x00\x00\x00\xf0\xff\x01\x00\x00\x00\x00\x00\x08\x40\x01\x00\x00\x00\x00\x00\xf8\xbf", 0);
    }
    {
        __m256d __r = _mm256_andnot_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_andnot_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\xf8\xbf\x00\x00\x00\x00\x00\x00\xf0\x7f\x01\x00\x00\x00\x00\x00\x08\x40\x01\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm256_movemask_pd(__L(__m256d, DA, 0));
        chk("_mm256_movemask_pd", &__r, 4, "\x0a\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_unpacklo_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_unpacklo_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf8\xbf\x00\x00\x00\x00\x00\x00\xf0\x3f\x01\x00\x00\x00\x00\x00\xf8\x7f", 0);
    }
    {
        __m256d __r = _mm256_unpackhi_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_unpackhi_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf0\x7f\x00\x00\x00\x00\x00\x00\xf8\xbf\x01\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 0);
        chk("_mm256_cmp_pd/0", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 1);
        chk("_mm256_cmp_pd/1", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 2);
        chk("_mm256_cmp_pd/2", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 3);
        chk("_mm256_cmp_pd/3", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 4);
        chk("_mm256_cmp_pd/4", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 5);
        chk("_mm256_cmp_pd/5", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("avx2,fma,f16c"))) void run8(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 6);
        chk("_mm256_cmp_pd/6", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 7);
        chk("_mm256_cmp_pd/7", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 8);
        chk("_mm256_cmp_pd/8", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 9);
        chk("_mm256_cmp_pd/9", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 10);
        chk("_mm256_cmp_pd/10", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 11);
        chk("_mm256_cmp_pd/11", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 12);
        chk("_mm256_cmp_pd/12", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 13);
        chk("_mm256_cmp_pd/13", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 14);
        chk("_mm256_cmp_pd/14", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 15);
        chk("_mm256_cmp_pd/15", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 16);
        chk("_mm256_cmp_pd/16", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 17);
        chk("_mm256_cmp_pd/17", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 18);
        chk("_mm256_cmp_pd/18", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 19);
        chk("_mm256_cmp_pd/19", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 20);
        chk("_mm256_cmp_pd/20", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 21);
        chk("_mm256_cmp_pd/21", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 22);
        chk("_mm256_cmp_pd/22", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 23);
        chk("_mm256_cmp_pd/23", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 24);
        chk("_mm256_cmp_pd/24", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 25);
        chk("_mm256_cmp_pd/25", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 26);
        chk("_mm256_cmp_pd/26", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 27);
        chk("_mm256_cmp_pd/27", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 28);
        chk("_mm256_cmp_pd/28", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 29);
        chk("_mm256_cmp_pd/29", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 30);
        chk("_mm256_cmp_pd/30", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_cmp_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), 31);
        chk("_mm256_cmp_pd/31", &__r, 32, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_addsub_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_addsub_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\xf8\x3f\x00\x00\x00\x00\x00\x00\xf0\x7f\x01\x00\x00\x00\x00\x00\xf8\x7f\x00\x00\x00\x00\x00\x00\xf8\xbf", 0);
    }
    {
        __m256d __r = _mm256_hadd_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_hadd_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x7f\x00\x00\x00\x00\x00\x00\xe0\xbf\x01\x00\x00\x00\x00\x00\xf8\x7f", 0);
    }
    {
        __m256d __r = _mm256_hsub_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0));
        chk("_mm256_hsub_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\xff\x00\x00\x00\x00\x00\x00\x04\x40\x01\x00\x00\x00\x00\x00\xf8\x7f", 0);
    }
    {
        __m256d __r = _mm256_round_pd(__L(__m256d, DA, 0), 8);
        chk("_mm256_round_pd/0", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00\x00\x00\xc0", 0);
    }
    {
        __m256d __r = _mm256_round_pd(__L(__m256d, DA, 0), 9);
        chk("_mm256_round_pd/1", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00\x00\x00\xc0", 0);
    }
    {
        __m256d __r = _mm256_round_pd(__L(__m256d, DA, 0), 10);
        chk("_mm256_round_pd/2", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00\x00\xf0\xbf", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("avx2,fma,f16c"))) void run9(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m256d __r = _mm256_round_pd(__L(__m256d, DA, 0), 11);
        chk("_mm256_round_pd/3", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00\x00\xf0\xbf", 0);
    }
    {
        __m256d __r = _mm256_floor_pd(__L(__m256d, DA, 0));
        chk("_mm256_floor_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00\x00\x00\xc0", 0);
    }
    {
        __m256d __r = _mm256_ceil_pd(__L(__m256d, DA, 0));
        chk("_mm256_ceil_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00\x00\xf0\xbf", 0);
    }
    {
        __m256d __r = _mm256_blendv_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), __L(__m256d, DC, 0));
        chk("_mm256_blendv_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf0\x3f\x01\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_fmadd_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), __L(__m256d, DC, 0));
        chk("_mm256_fmadd_pd", &__r, 32, "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf8\xff\x01\x00\x00\x00\x00\x00\xf8\x7f\x02\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m256d __r = _mm256_fmsub_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), __L(__m256d, DC, 0));
        chk("_mm256_fmsub_pd", &__r, 32, "\x01\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf8\xff\x01\x00\x00\x00\x00\x00\xf8\x7f\x02\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m256d __r = _mm256_fnmadd_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), __L(__m256d, DC, 0));
        chk("_mm256_fnmadd_pd", &__r, 32, "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf8\xff\x01\x00\x00\x00\x00\x00\xf8\x7f\x02\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_fnmsub_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), __L(__m256d, DC, 0));
        chk("_mm256_fnmsub_pd", &__r, 32, "\x01\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf8\xff\x01\x00\x00\x00\x00\x00\xf8\x7f\x02\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256d __r = _mm256_fmaddsub_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), __L(__m256d, DC, 0));
        chk("_mm256_fmaddsub_pd", &__r, 32, "\x01\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf8\xff\x01\x00\x00\x00\x00\x00\xf8\x7f\x02\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m256d __r = _mm256_fmsubadd_pd(__L(__m256d, DA, 0), __L(__m256d, DB, 0), __L(__m256d, DC, 0));
        chk("_mm256_fmsubadd_pd", &__r, 32, "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf8\xff\x01\x00\x00\x00\x00\x00\xf8\x7f\x02\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 0);
        chk("_mm_cmp_ps/0", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 1);
        chk("_mm_cmp_ps/1", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 2);
        chk("_mm_cmp_ps/2", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 3);
        chk("_mm_cmp_ps/3", &__r, 16, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 4);
        chk("_mm_cmp_ps/4", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 5);
        chk("_mm_cmp_ps/5", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 6);
        chk("_mm_cmp_ps/6", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 7);
        chk("_mm_cmp_ps/7", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 8);
        chk("_mm_cmp_ps/8", &__r, 16, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 9);
        chk("_mm_cmp_ps/9", &__r, 16, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 10);
        chk("_mm_cmp_ps/10", &__r, 16, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 11);
        chk("_mm_cmp_ps/11", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 12);
        chk("_mm_cmp_ps/12", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 13);
        chk("_mm_cmp_ps/13", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 14);
        chk("_mm_cmp_ps/14", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 15);
        chk("_mm_cmp_ps/15", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 16);
        chk("_mm_cmp_ps/16", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 17);
        chk("_mm_cmp_ps/17", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 18);
        chk("_mm_cmp_ps/18", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 19);
        chk("_mm_cmp_ps/19", &__r, 16, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 20);
        chk("_mm_cmp_ps/20", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 21);
        chk("_mm_cmp_ps/21", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("avx2,fma,f16c"))) void run10(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 22);
        chk("_mm_cmp_ps/22", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 23);
        chk("_mm_cmp_ps/23", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 24);
        chk("_mm_cmp_ps/24", &__r, 16, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 25);
        chk("_mm_cmp_ps/25", &__r, 16, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 26);
        chk("_mm_cmp_ps/26", &__r, 16, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 27);
        chk("_mm_cmp_ps/27", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 28);
        chk("_mm_cmp_ps/28", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 29);
        chk("_mm_cmp_ps/29", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 30);
        chk("_mm_cmp_ps/30", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmp_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 31);
        chk("_mm_cmp_ps/31", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_fmadd_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), __L(__m128, FC, 0));
        chk("_mm_fmadd_ps", &__r, 16, "\x00\x00\xc0\xff\x01\x00\xc0\x7f\x00\x00\xe8\xc0\x00\x00\x80\xff", 0);
    }
    {
        __m128 __r = _mm_fmsub_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), __L(__m128, FC, 0));
        chk("_mm_fmsub_ps", &__r, 16, "\x00\x00\xc0\xff\x01\x00\xc0\x7f\x00\x00\xe8\x40\x00\x00\x80\xff", 0);
    }
    {
        __m128 __r = _mm_fnmadd_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), __L(__m128, FC, 0));
        chk("_mm_fnmadd_ps", &__r, 16, "\x00\x00\xc0\xff\x01\x00\xc0\x7f\x00\x00\xe8\xc0\x00\x00\x80\x7f", 0);
    }
    {
        __m128 __r = _mm_fnmsub_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), __L(__m128, FC, 0));
        chk("_mm_fnmsub_ps", &__r, 16, "\x00\x00\xc0\xff\x01\x00\xc0\x7f\x00\x00\xe8\x40\x00\x00\x80\x7f", 0);
    }
    {
        __m128 __r = _mm_fmaddsub_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), __L(__m128, FC, 0));
        chk("_mm_fmaddsub_ps", &__r, 16, "\x00\x00\xc0\xff\x01\x00\xc0\x7f\x00\x00\xe8\x40\x00\x00\x80\xff", 0);
    }
    {
        __m128 __r = _mm_fmsubadd_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), __L(__m128, FC, 0));
        chk("_mm_fmsubadd_ps", &__r, 16, "\x00\x00\xc0\xff\x01\x00\xc0\x7f\x00\x00\xe8\xc0\x00\x00\x80\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 0);
        chk("_mm_cmp_pd/0", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 1);
        chk("_mm_cmp_pd/1", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 2);
        chk("_mm_cmp_pd/2", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 3);
        chk("_mm_cmp_pd/3", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 4);
        chk("_mm_cmp_pd/4", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 5);
        chk("_mm_cmp_pd/5", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 6);
        chk("_mm_cmp_pd/6", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 7);
        chk("_mm_cmp_pd/7", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 8);
        chk("_mm_cmp_pd/8", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 9);
        chk("_mm_cmp_pd/9", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 10);
        chk("_mm_cmp_pd/10", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 11);
        chk("_mm_cmp_pd/11", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 12);
        chk("_mm_cmp_pd/12", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 13);
        chk("_mm_cmp_pd/13", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 14);
        chk("_mm_cmp_pd/14", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 15);
        chk("_mm_cmp_pd/15", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("avx2,fma,f16c"))) void run11(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 16);
        chk("_mm_cmp_pd/16", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 17);
        chk("_mm_cmp_pd/17", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 18);
        chk("_mm_cmp_pd/18", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 19);
        chk("_mm_cmp_pd/19", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 20);
        chk("_mm_cmp_pd/20", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 21);
        chk("_mm_cmp_pd/21", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 22);
        chk("_mm_cmp_pd/22", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 23);
        chk("_mm_cmp_pd/23", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 24);
        chk("_mm_cmp_pd/24", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 25);
        chk("_mm_cmp_pd/25", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 26);
        chk("_mm_cmp_pd/26", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 27);
        chk("_mm_cmp_pd/27", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 28);
        chk("_mm_cmp_pd/28", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 29);
        chk("_mm_cmp_pd/29", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 30);
        chk("_mm_cmp_pd/30", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmp_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), 31);
        chk("_mm_cmp_pd/31", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_fmadd_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), __L(__m128d, DC, 0));
        chk("_mm_fmadd_pd", &__r, 16, "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf8\xff", 0);
    }
    {
        __m128d __r = _mm_fmsub_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), __L(__m128d, DC, 0));
        chk("_mm_fmsub_pd", &__r, 16, "\x01\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf8\xff", 0);
    }
    {
        __m128d __r = _mm_fnmadd_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), __L(__m128d, DC, 0));
        chk("_mm_fnmadd_pd", &__r, 16, "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf8\xff", 0);
    }
    {
        __m128d __r = _mm_fnmsub_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), __L(__m128d, DC, 0));
        chk("_mm_fnmsub_pd", &__r, 16, "\x01\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf8\xff", 0);
    }
    {
        __m128d __r = _mm_fmaddsub_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), __L(__m128d, DC, 0));
        chk("_mm_fmaddsub_pd", &__r, 16, "\x01\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf8\xff", 0);
    }
    {
        __m128d __r = _mm_fmsubadd_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), __L(__m128d, DC, 0));
        chk("_mm_fmsubadd_pd", &__r, 16, "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf8\xff", 0);
    }
    {
        __m256i __r = _mm256_cvtps_epi32(__L(__m256, FA, 0));
        chk("_mm256_cvtps_epi32", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\xfe\xff\xff\xff\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_cvttps_epi32(__L(__m256, FA, 0));
        chk("_mm256_cvttps_epi32", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cvtepi32_ps(__L(__m256i, A, 0));
        chk("_mm256_cvtepi32_ps", &__r, 32, "\x00\x86\xe5\xca\x10\x02\x72\xce\x00\x0d\xe0\x4e\x00\x94\xe1\xcb\x00\x30\xd0\x4c\x02\x55\xc4\xce\x00\x00\x00\x4f\xca\x16\x61\x4e", 0);
    }
    {
        __m128i __r = _mm256_cvtpd_epi32(__L(__m256d, DA, 0));
        chk("_mm256_cvtpd_epi32", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\xfe\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm256_cvttpd_epi32(__L(__m256d, DA, 0));
        chk("_mm256_cvttpd_epi32", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m256d __r = _mm256_cvtepi32_pd(__L(__m128i, A, 0));
        chk("_mm256_cvtepi32_pd", &__r, 32, "\x00\x00\x00\x00\xc0\xb0\x5c\xc1\x00\x00\x80\x09\x42\x40\xce\xc1\x00\x00\x40\x08\xa0\x01\xdc\x41\x00\x00\x00\xf0\x7f\x32\x7c\xc1", 0);
    }
    {
        __m256d __r = _mm256_cvtps_pd(__L(__m128, FA, 0));
        chk("_mm256_cvtps_pd", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00\x00\xf8\xbf", 0);
    }
    {
        __m128 __r = _mm256_cvtpd_ps(__L(__m256d, DA, 0));
        chk("_mm256_cvtpd_ps", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m256i __r = _mm256_permute4x64_epi64(__L(__m256i, A, 0), 0x1b);
        chk("_mm256_permute4x64_epi64/1b", &__r, 32, "\xff\xff\xff\x7f\x65\xb2\x45\x38\x00\x80\x81\x06\xd9\x7e\xd5\x9d\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x3d\x8d\xff\xed\x7b\x7f\xc3", 0);
    }
    {
        __m256i __r = _mm256_permute4x64_epi64(__L(__m256i, A, 0), 0x4e);
        chk("_mm256_permute4x64_epi64/4e", &__r, 32, "\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("avx2,fma,f16c"))) void run12(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m256i __r = _mm256_permute4x64_epi64(__L(__m256i, A, 0), 0xe4);
        chk("_mm256_permute4x64_epi64/e4", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_permute2x128_si256(__L(__m256i, A, 0), __L(__m256i, B, 0), 0x1);
        chk("_mm256_permute2x128_si256/1", &__r, 32, "\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m256i __r = _mm256_permute2x128_si256(__L(__m256i, A, 0), __L(__m256i, B, 0), 0x20);
        chk("_mm256_permute2x128_si256/20", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c\xff\xda\x38\x7f\x1c\x5d\x80\x2d", 0);
    }
    {
        __m256i __r = _mm256_permute2x128_si256(__L(__m256i, A, 0), __L(__m256i, B, 0), 0x31);
        chk("_mm256_permute2x128_si256/31", &__r, 32, "\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38\xff\x7f\x55\xae\xfe\xd0\x23\x81\x00\x00\x00\x80\x26\x60\xbc\x43", 0);
    }
    {
        __m256i __r = _mm256_permute2x128_si256(__L(__m256i, A, 0), __L(__m256i, B, 0), 0x8);
        chk("_mm256_permute2x128_si256/8", &__r, 32, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m256i __r = _mm256_permute2x128_si256(__L(__m256i, A, 0), __L(__m256i, B, 0), 0x83);
        chk("_mm256_permute2x128_si256/83", &__r, 32, "\xff\x7f\x55\xae\xfe\xd0\x23\x81\x00\x00\x00\x80\x26\x60\xbc\x43\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_permutevar8x32_epi32(__L(__m256i, A, 0), __L(__m256i, IX, 0));
        chk("_mm256_permutevar8x32_epi32", &__r, 32, "\xd9\x7e\xd5\x9d\x00\x3d\x8d\xff\x65\xb2\x45\x38\x21\x80\x06\x70\x21\x80\x06\x70\xed\x7b\x7f\xc3\x01\xd8\x3c\xfe\xed\x7b\x7f\xc3", 0);
    }
    {
        __m256i __r = _mm256_sllv_epi32(__L(__m256i, A, 0), __L(__m256i, SC32, 0));
        chk("_mm256_sllv_epi32", &__r, 32, "\x00\xe8\x69\xfc\xed\x7b\x7f\xc3\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x03\x0d\x00\x00\x00\x00\x80\xff\xff\xff\x00\x00\x65\xb2", 0);
    }
    {
        __m256i __r = _mm256_srlv_epi32(__L(__m256i, A, 0), __L(__m256i, SC32, 0));
        chk("_mm256_srlv_epi32", &__r, 32, "\xa0\xa7\xf1\x1f\xed\x7b\x7f\xc3\x00\x00\x00\x00\x01\x00\x00\x00\x00\xc0\x40\x03\x00\x00\x00\x00\xff\xff\xff\x00\x45\x38\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srav_epi32(__L(__m256i, A, 0), __L(__m256i, SC32, 0));
        chk("_mm256_srav_epi32", &__r, 32, "\xa0\xa7\xf1\xff\xed\x7b\x7f\xc3\x00\x00\x00\x00\xff\xff\xff\xff\x00\xc0\x40\x03\xff\xff\xff\xff\xff\xff\xff\x00\x45\x38\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_sllv_epi64(__L(__m256i, A, 0), __L(__m256i, SC64, 0));
        chk("_mm256_sllv_epi64", &__r, 32, "\x00\xe8\x69\xfc\x6f\xdf\xfb\x1b\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_srlv_epi64(__L(__m256i, A, 0), __L(__m256i, SC64, 0));
        chk("_mm256_srlv_epi64", &__r, 32, "\xa0\xa7\xf1\xbf\x7d\xef\x6f\x18\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_i32gather_epi32((const int *)(const void *)A, __L(__m256i, IG, 0), 4);
        chk("_mm256_i32gather_epi32", &__r, 32, "\x00\x3d\x8d\xff\xd9\x7e\xd5\x9d\x21\x80\x06\x70\x65\xb2\x45\x38\xed\x7b\x7f\xc3\xed\x7b\x7f\xc3\xff\xff\xff\x7f\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_i32gather_epi32((const int *)(const void *)A, __L(__m128i, IG, 0), 4);
        chk("_mm_i32gather_epi32", &__r, 16, "\x00\x3d\x8d\xff\xd9\x7e\xd5\x9d\x21\x80\x06\x70\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_i32gather_epi64((const __axiom_int64 *)(const void *)A, __L(__m128i, IG, 0), 8);
        chk("_mm256_i32gather_epi64", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\xe2\x5e\xd8\xe1\xea\x31\x65\xf8\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xfb\x90\xf8\xe8\x5b\xd2\x2d\x75", 0);
    }
    {
        __m256 __r = _mm256_i32gather_ps((const float *)(const void *)FA, __L(__m256i, IG, 0), 4);
        chk("_mm256_i32gather_ps", &__r, 32, "\x00\x00\x00\x00\x00\x00\x80\xff\x00\x00\x80\x3f\x01\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x80\x01\x00\xc0\x7f\x00\x00\xc0\xbf", 0);
    }
    {
        int __r = _mm256_movemask_epi8(__L(__m256i, A, 0));
        chk("_mm256_movemask_epi8", &__r, 4, "\x9c\xa2\xd6\x27", 0);
    }
    {
        __m128i __r = _mm256_extracti128_si256(__L(__m256i, A, 0), 1);
        chk("_mm256_extracti128_si256/1", &__r, 16, "\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_inserti128_si256(__L(__m256i, A, 0), __L(__m128i, B, 0), 0);
        chk("_mm256_inserti128_si256/0", &__r, 32, "\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c\xff\xda\x38\x7f\x1c\x5d\x80\x2d\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        __m256i __r = _mm256_broadcastd_epi32(__L(__m128i, B, 0));
        chk("_mm256_broadcastd_epi32", &__r, 32, "\xa4\xe5\x6b\xa2\xa4\xe5\x6b\xa2\xa4\xe5\x6b\xa2\xa4\xe5\x6b\xa2\xa4\xe5\x6b\xa2\xa4\xe5\x6b\xa2\xa4\xe5\x6b\xa2\xa4\xe5\x6b\xa2", 0);
    }
    {
        __m256i __r = _mm256_set_epi32(944091749, 2147483647, -1646952743, 109150208, -29566975, 1879474209, -1015055379, -7521024);
        chk("_mm256_set_epi32", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x65\xb2\x45\x38", 0);
    }
    {
        int __r = _mm256_testz_si256(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_testz_si256", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm256_testc_si256(__L(__m256i, A, 0), __L(__m256i, B, 0));
        chk("_mm256_testc_si256", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm256_testc_si256(__L(__m256i, A, 0), __L(__m256i, A, 0));
        chk("_mm256_testc_si256/self", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cvtps_ph(__L(__m128, FN, 0), 0);
        chk("_mm_cvtps_ph", &__r, 16, "\x00\xc8\x00\x48\x00\x00\x00\x3c\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m256 __r = _mm256_cvtph_ps(__L(__m128i, A, 0));
        chk("_mm256_cvtph_ps", &__r, 32, "\x00\x00\xa0\x3f\x00\xa0\xf1\xff\x00\xa0\x7d\x47\x00\xe0\x6f\xc0\x00\x00\x04\xb6\x00\xc0\x00\x46\x00\x20\x00\xc3\x00\x80\xc7\xff", 0);
    }
    {
        __m256i __r = _mm256_maskload_epi32((const int *)(const void *)A, __L(__m256i, B, 0));
        chk("_mm256_maskload_epi32", &__r, 32, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x81\x06\xd9\x7e\xd5\x9d\xff\xff\xff\x7f\x00\x00\x00\x00", 0);
    }
    {
        __m256i __r = _mm256_blend_epi32(__L(__m256i, A, 0), __L(__m256i, B, 0), 0xa5);
        chk("_mm256_blend_epi32/a5", &__r, 32, "\xa4\xe5\x6b\xa2\xed\x7b\x7f\xc3\xff\xda\x38\x7f\x01\xd8\x3c\xfe\x00\x80\x81\x06\xfe\xd0\x23\x81\xff\xff\xff\x7f\x26\x60\xbc\x43", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("avx2,fma,f16c"))) void run(void) {
    run0();
    run1();
    run2();
    run3();
    run4();
    run5();
    run6();
    run7();
    run8();
    run9();
    run10();
    run11();
    run12();
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
    if (__os_avx() && __cpu(7, 1, bit_AVX2) && __cpu(1, 2, bit_FMA) && __cpu(1, 2, bit_F16C)) {
        run();
    } else {
        __checks = 412;
    }
    if (__fails == 0) {
        say("intrin_x86_avx2 ");
        say_n(__checks);
        endl();
    }
    return 0;
}
