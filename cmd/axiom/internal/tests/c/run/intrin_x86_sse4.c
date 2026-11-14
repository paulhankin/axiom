/* intrin_x86_sse4 - x86-64 intrinsics checked against the instruction set reference's
   definitions (computed by cmd/internal/tools/gen_x86_intrin.py, which
   writes this file: do not edit): SSE3, SSSE3, SSE4.1 and SSE4.2, where the processor has them. A mismatch prints the intrinsic
   and the result's bytes. */
/* x86-64-only */
/* flags: --c-std=c11 */
/* expect: intrin_x86_sse4 302 */
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

static __axiom_attribute__((__target__("sse4.2,popcnt"))) void run0(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128i __r = _mm_abs_epi8(__L(__m128i, A, 0));
        chk("_mm_abs_epi8", &__r, 16, "\x00\x3d\x73\x01\x13\x7b\x7f\x3d\x21\x80\x06\x70\x01\x28\x3c\x02", 0);
    }
    {
        __m128i __r = _mm_sign_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_sign_epi8", &__r, 16, "\x00\xc3\x8d\x01\x13\x00\x7f\x3d\xdf\x80\x06\x70\x01\xd8\xc4\xfe", 0);
    }
    {
        __m128i __r = _mm_abs_epi16(__L(__m128i, A, 0));
        chk("_mm_abs_epi16", &__r, 16, "\x00\x3d\x73\x00\xed\x7b\x81\x3c\xdf\x7f\x06\x70\xff\x27\xc4\x01", 0);
    }
    {
        __m128i __r = _mm_sign_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_sign_epi16", &__r, 16, "\x00\xc3\x73\x00\xed\x7b\x81\x3c\xdf\x7f\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_abs_epi32(__L(__m128i, A, 0));
        chk("_mm_abs_epi32", &__r, 16, "\x00\xc3\x72\x00\x13\x84\x80\x3c\x21\x80\x06\x70\xff\x27\xc3\x01", 0);
    }
    {
        __m128i __r = _mm_sign_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_sign_epi32", &__r, 16, "\x00\xc3\x72\x00\x13\x84\x80\x3c\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_hadd_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_hadd_epi16", &__r, 16, "\x8d\x3c\x6c\x3f\x27\xf0\x3d\xd6\x0f\x88\x07\x9d\x37\x5a\x9c\x8a", 0);
    }
    {
        __m128i __r = _mm_hadd_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_hadd_epi32", &__r, 16, "\xed\xb8\x0c\xc3\x22\x58\x43\x6e\x87\xe6\x8f\x3e\x1b\x38\xb9\xac", 0);
    }
    {
        __m128i __r = _mm_hadds_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_hadds_epi16", &__r, 16, "\x8d\x3c\x6c\x3f\x27\xf0\x3d\xd6\x0f\x88\x07\x9d\x37\x5a\xff\x7f", 0);
    }
    {
        __m128i __r = _mm_hsub_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_hsub_epi16", &__r, 16, "\x73\x3d\x6e\xb8\x1b\x10\xc5\xd9\x39\x43\xbf\x64\xc7\x5b\x9c\x2f", 0);
    }
    {
        __m128i __r = _mm_hsub_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_hsub_epi32", &__r, 16, "\x13\xc1\x0d\x3c\x20\xa8\xc9\x71\xc1\xe4\x47\x06\xe3\x7d\xb8\x51", 0);
    }
    {
        __m128i __r = _mm_hsubs_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_hsubs_epi16", &__r, 16, "\x73\x3d\xff\x7f\x00\x80\xc5\xd9\x39\x43\xbf\x64\x00\x80\x9c\x2f", 0);
    }
    {
        __m128i __r = _mm_maddubs_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_maddubs_epi16", &__r, 16, "\x91\xf9\x4d\xdd\x27\xe5\xb0\xc5\xdf\xec\xe0\x38\x94\x4e\xa6\x0e", 0);
    }
    {
        __m128i __r = _mm_mulhrs_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_mulhrs_epi16", &__r, 16, "\x70\xf3\x54\x00\xdc\x00\x34\x2f\xf7\x24\x57\x6f\xe8\xe2\x5f\xff", 0);
    }
    {
        __m128i __r = _mm_shuffle_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_shuffle_epi8", &__r, 16, "\x00\x00\x70\x00\x00\x00\xed\x00\x00\x00\x21\xfe\x01\xd8\x00\xd8", 0);
    }
    {
        __m128i __r = _mm_alignr_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0), 0);
        chk("_mm_alignr_epi8/0", &__r, 16, "\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c\xff\xda\x38\x7f\x1c\x5d\x80\x2d", 0);
    }
    {
        __m128i __r = _mm_alignr_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0), 5);
        chk("_mm_alignr_epi8/5", &__r, 16, "\x00\x24\x9c\xff\xda\x38\x7f\x1c\x5d\x80\x2d\x00\x3d\x8d\xff\xed", 0);
    }
    {
        __m128i __r = _mm_alignr_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0), 16);
        chk("_mm_alignr_epi8/16", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_alignr_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0), 20);
        chk("_mm_alignr_epi8/20", &__r, 16, "\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_alignr_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0), 32);
        chk("_mm_alignr_epi8/32", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpeq_epi64(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_cmpeq_epi64", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_min_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_min_epi8", &__r, 16, "\xa4\xe5\x8d\xa2\xe3\x00\x24\x9c\xff\x80\x06\x70\x01\xd8\x80\xfe", 0);
    }
    {
        __m128i __r = _mm_max_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_max_epi8", &__r, 16, "\x00\x3d\x6b\xff\xed\x7b\x7f\xc3\x21\xda\x38\x7f\x1c\x5d\x3c\x2d", 0);
    }
    {
        __m128i __r = _mm_min_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_min_epi32", &__r, 16, "\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_max_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_max_epi32", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\xff\xda\x38\x7f\x1c\x5d\x80\x2d", 0);
    }
    {
        __m128i __r = _mm_min_epu16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_min_epu16", &__r, 16, "\x00\x3d\x6b\xa2\xe3\x00\x24\x9c\x21\x80\x06\x70\x1c\x5d\x80\x2d", 0);
    }
    {
        __m128i __r = _mm_max_epu16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_max_epu16", &__r, 16, "\xa4\xe5\x8d\xff\xed\x7b\x7f\xc3\xff\xda\x38\x7f\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_min_epu32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_min_epu32", &__r, 16, "\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c\x21\x80\x06\x70\x1c\x5d\x80\x2d", 0);
    }
    {
        __m128i __r = _mm_max_epu32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_max_epu32", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\xff\xda\x38\x7f\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_mul_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_mul_epi32", &__r, 16, "\x00\x14\x0c\x8d\x3b\xf3\x29\x00\xdf\xba\xcd\x85\xd1\x1a\xac\x37", 0);
    }
    {
        __m128i __r = _mm_mullo_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_mullo_epi32", &__r, 16, "\x00\x14\x0c\x8d\x27\xe3\x5e\x33\xdf\xba\xcd\x85\x1c\xfd\x9f\x16", 0);
    }
    {
        __m128i __r = _mm_packus_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_packus_epi32", &__r, 16, "\x00\x00\x00\x00\xff\xff\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("sse4.2,popcnt"))) void run1(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128i __r = _mm_blendv_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0), __L(__m128i, C, 0));
        chk("_mm_blendv_epi8", &__r, 16, "\x00\xe5\x8d\xa2\xe3\x00\x24\x9c\xff\xda\x06\x70\x1c\x5d\x80\xfe", 0);
    }
    {
        __m128i __r = _mm_blend_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0), 0x0);
        chk("_mm_blend_epi16/0", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_blend_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0), 0xa5);
        chk("_mm_blend_epi16/a5", &__r, 16, "\xa4\xe5\x8d\xff\xe3\x00\x7f\xc3\x21\x80\x38\x7f\x01\xd8\x80\x2d", 0);
    }
    {
        __m128i __r = _mm_blend_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0), 0xff);
        chk("_mm_blend_epi16/ff", &__r, 16, "\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c\xff\xda\x38\x7f\x1c\x5d\x80\x2d", 0);
    }
    {
        __m128i __r = _mm_mpsadbw_epu8(__L(__m128i, A, 0), __L(__m128i, B, 0), 0);
        chk("_mm_mpsadbw_epu8/0", &__r, 16, "\xcb\x01\x9e\x01\xda\x00\x96\x00\xe8\x00\x68\x01\xb3\x00\x94\x01", 0);
    }
    {
        __m128i __r = _mm_mpsadbw_epu8(__L(__m128i, A, 0), __L(__m128i, B, 0), 5);
        chk("_mm_mpsadbw_epu8/5", &__r, 16, "\x07\x01\x01\x02\x46\x01\x33\x01\x8c\x01\x50\x01\xac\x01\x88\x01", 0);
    }
    {
        __m128i __r = _mm_mpsadbw_epu8(__L(__m128i, A, 0), __L(__m128i, B, 0), 7);
        chk("_mm_mpsadbw_epu8/7", &__r, 16, "\x86\x01\xd0\x00\x7b\x01\x0a\x01\xe5\x00\xf7\x00\x53\x01\x17\x01", 0);
    }
    {
        __m128i __r = _mm_cvtepi8_epi16(__L(__m128i, A, 0));
        chk("_mm_cvtepi8_epi16", &__r, 16, "\x00\x00\x3d\x00\x8d\xff\xff\xff\xed\xff\x7b\x00\x7f\x00\xc3\xff", 0);
    }
    {
        __m128i __r = _mm_cvtepi8_epi32(__L(__m128i, A, 0));
        chk("_mm_cvtepi8_epi32", &__r, 16, "\x00\x00\x00\x00\x3d\x00\x00\x00\x8d\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm_cvtepi8_epi64(__L(__m128i, A, 0));
        chk("_mm_cvtepi8_epi64", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x3d\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cvtepi16_epi32(__L(__m128i, A, 0));
        chk("_mm_cvtepi16_epi32", &__r, 16, "\x00\x3d\x00\x00\x8d\xff\xff\xff\xed\x7b\x00\x00\x7f\xc3\xff\xff", 0);
    }
    {
        __m128i __r = _mm_cvtepi16_epi64(__L(__m128i, A, 0));
        chk("_mm_cvtepi16_epi64", &__r, 16, "\x00\x3d\x00\x00\x00\x00\x00\x00\x8d\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm_cvtepi32_epi64(__L(__m128i, A, 0));
        chk("_mm_cvtepi32_epi64", &__r, 16, "\x00\x3d\x8d\xff\xff\xff\xff\xff\xed\x7b\x7f\xc3\xff\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm_cvtepu8_epi16(__L(__m128i, A, 0));
        chk("_mm_cvtepu8_epi16", &__r, 16, "\x00\x00\x3d\x00\x8d\x00\xff\x00\xed\x00\x7b\x00\x7f\x00\xc3\x00", 0);
    }
    {
        __m128i __r = _mm_cvtepu8_epi32(__L(__m128i, A, 0));
        chk("_mm_cvtepu8_epi32", &__r, 16, "\x00\x00\x00\x00\x3d\x00\x00\x00\x8d\x00\x00\x00\xff\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cvtepu8_epi64(__L(__m128i, A, 0));
        chk("_mm_cvtepu8_epi64", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x3d\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cvtepu16_epi32(__L(__m128i, A, 0));
        chk("_mm_cvtepu16_epi32", &__r, 16, "\x00\x3d\x00\x00\x8d\xff\x00\x00\xed\x7b\x00\x00\x7f\xc3\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cvtepu16_epi64(__L(__m128i, A, 0));
        chk("_mm_cvtepu16_epi64", &__r, 16, "\x00\x3d\x00\x00\x00\x00\x00\x00\x8d\xff\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cvtepu32_epi64(__L(__m128i, A, 0));
        chk("_mm_cvtepu32_epi64", &__r, 16, "\x00\x3d\x8d\xff\x00\x00\x00\x00\xed\x7b\x7f\xc3\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_addsub_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_addsub_ps", &__r, 16, "\x00\x00\x80\x7f\x01\x00\xc0\x7f\x00\x00\x80\x3f\x9e\xc9\x7f\x7f", 0);
    }
    {
        __m128 __r = _mm_hadd_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_hadd_ps", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\xbf\x01\x00\xc0\x7f\x9e\xc9\x7f\x7f", 0);
    }
    {
        __m128 __r = _mm_hsub_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_hsub_ps", &__r, 16, "\x00\x00\x00\x00\x00\x00\x20\x40\x01\x00\xc0\x7f\x9e\xc9\x7f\xff", 0);
    }
    {
        __m128 __r = _mm_round_ps(__L(__m128, FA, 0), 8);
        chk("_mm_round_ps/0", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\x00\xc0", 0);
    }
    {
        __m128 __r = _mm_round_ps(__L(__m128, FA, 0), 9);
        chk("_mm_round_ps/1", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\x00\xc0", 0);
    }
    {
        __m128 __r = _mm_round_ps(__L(__m128, FA, 0), 10);
        chk("_mm_round_ps/2", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\x80\xbf", 0);
    }
    {
        __m128 __r = _mm_round_ps(__L(__m128, FA, 0), 11);
        chk("_mm_round_ps/3", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\x80\xbf", 0);
    }
    {
        __m128 __r = _mm_floor_ps(__L(__m128, FA, 0));
        chk("_mm_floor_ps", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\x00\xc0", 0);
    }
    {
        __m128 __r = _mm_ceil_ps(__L(__m128, FA, 0));
        chk("_mm_ceil_ps", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\x80\xbf", 0);
    }
    {
        __m128 __r = _mm_blendv_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), __L(__m128, FC, 0));
        chk("_mm_blendv_ps", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x01\x00\x00\x00\x00\x00\xc0\xbf", 0);
    }
    {
        __m128d __r = _mm_addsub_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_addsub_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\xf8\x3f\x00\x00\x00\x00\x00\x00\xf0\x7f", 0);
    }
    {
        __m128d __r = _mm_hadd_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_hadd_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x7f", 0);
    }
    {
        __m128d __r = _mm_hsub_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_hsub_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\xff", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("sse4.2,popcnt"))) void run2(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128d __r = _mm_round_pd(__L(__m128d, DA, 0), 8);
        chk("_mm_round_pd/0", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_round_pd(__L(__m128d, DA, 0), 9);
        chk("_mm_round_pd/1", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_round_pd(__L(__m128d, DA, 0), 10);
        chk("_mm_round_pd/2", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_round_pd(__L(__m128d, DA, 0), 11);
        chk("_mm_round_pd/3", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_floor_pd(__L(__m128d, DA, 0));
        chk("_mm_floor_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_ceil_pd(__L(__m128d, DA, 0));
        chk("_mm_ceil_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_blendv_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0), __L(__m128d, DC, 0));
        chk("_mm_blendv_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128i __r = _mm_cmpgt_epi64(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_cmpgt_epi64", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        unsigned int __r = _mm_crc32_u8(0xa26be5a4u, (unsigned char)0x0u);
        chk("_mm_crc32_u8", &__r, 4, "\x5c\x49\x73\x65", 0);
    }
    {
        unsigned int __r = _mm_crc32_u16(0xa26be5a4u, (unsigned short)0x3d00u);
        chk("_mm_crc32_u16", &__r, 4, "\x28\x63\xc8\x93", 0);
    }
    {
        unsigned int __r = _mm_crc32_u32(0xa26be5a4u, (unsigned int)0xff8d3d00u);
        chk("_mm_crc32_u32", &__r, 4, "\x5f\x40\x8b\x06", 0);
    }
    {
        unsigned __axiom_int64 __r = _mm_crc32_u64(0xa26be5a4u, (unsigned __axiom_int64)0xc37f7bedff8d3d00u);
        chk("_mm_crc32_u64", &__r, 8, "\x5a\x4a\x73\x6c\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_popcnt_u32(0xff8d3d00u);
        chk("_mm_popcnt_u32", &__r, 4, "\x11\x00\x00\x00", 0);
    }
    {
        __axiom_int64 __r = _mm_popcnt_u64(0xc37f7bedff8d3d00ull);
        chk("_mm_popcnt_u64", &__r, 8, "\x28\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x0);
        chk("_mm_cmpestri/0/16/16", &__r, 4, "\x05\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x0);
        chk("_mm_cmpestrm/0/16/16", &__r, 16, "\x20\x49\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x0);
        chk("_mm_cmpestra/0/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x0);
        chk("_mm_cmpestrc/0/16/16", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x0);
        chk("_mm_cmpestro/0/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x0);
        chk("_mm_cmpestrs/0/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x0);
        chk("_mm_cmpestrz/0/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x0);
        chk("_mm_cmpestri/0/5/9", &__r, 4, "\x05\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x0);
        chk("_mm_cmpestrm/0/5/9", &__r, 16, "\x20\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x0);
        chk("_mm_cmpestra/0/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x0);
        chk("_mm_cmpestrc/0/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x0);
        chk("_mm_cmpestro/0/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x0);
        chk("_mm_cmpestrs/0/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x0);
        chk("_mm_cmpestrz/0/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x0);
        chk("_mm_cmpestri/0/0/3", &__r, 4, "\x10\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x0);
        chk("_mm_cmpestrm/0/0/3", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x0);
        chk("_mm_cmpestra/0/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x0);
        chk("_mm_cmpestrc/0/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("sse4.2,popcnt"))) void run3(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x0);
        chk("_mm_cmpestro/0/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x0);
        chk("_mm_cmpestrs/0/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x0);
        chk("_mm_cmpestrz/0/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistri(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x0);
        chk("_mm_cmpistri/0", &__r, 4, "\x10\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpistrm(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x0);
        chk("_mm_cmpistrm/0", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistra(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x0);
        chk("_mm_cmpistra/0", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrc(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x0);
        chk("_mm_cmpistrc/0", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistro(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x0);
        chk("_mm_cmpistro/0", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrs(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x0);
        chk("_mm_cmpistrs/0", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrz(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x0);
        chk("_mm_cmpistrz/0", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x4);
        chk("_mm_cmpestri/4/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x4);
        chk("_mm_cmpestrm/4/16/16", &__r, 16, "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x4);
        chk("_mm_cmpestra/4/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x4);
        chk("_mm_cmpestrc/4/16/16", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x4);
        chk("_mm_cmpestro/4/16/16", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x4);
        chk("_mm_cmpestrs/4/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x4);
        chk("_mm_cmpestrz/4/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x4);
        chk("_mm_cmpestri/4/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x4);
        chk("_mm_cmpestrm/4/5/9", &__r, 16, "\xfb\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x4);
        chk("_mm_cmpestra/4/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x4);
        chk("_mm_cmpestrc/4/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x4);
        chk("_mm_cmpestro/4/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x4);
        chk("_mm_cmpestrs/4/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x4);
        chk("_mm_cmpestrz/4/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x4);
        chk("_mm_cmpestri/4/0/3", &__r, 4, "\x10\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x4);
        chk("_mm_cmpestrm/4/0/3", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x4);
        chk("_mm_cmpestra/4/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x4);
        chk("_mm_cmpestrc/4/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x4);
        chk("_mm_cmpestro/4/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x4);
        chk("_mm_cmpestrs/4/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x4);
        chk("_mm_cmpestrz/4/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistri(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x4);
        chk("_mm_cmpistri/4", &__r, 4, "\x10\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("sse4.2,popcnt"))) void run4(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128i __r = _mm_cmpistrm(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x4);
        chk("_mm_cmpistrm/4", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistra(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x4);
        chk("_mm_cmpistra/4", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrc(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x4);
        chk("_mm_cmpistrc/4", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistro(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x4);
        chk("_mm_cmpistro/4", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrs(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x4);
        chk("_mm_cmpistrs/4", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrz(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x4);
        chk("_mm_cmpistrz/4", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x8);
        chk("_mm_cmpestri/8/16/16", &__r, 4, "\x10\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x8);
        chk("_mm_cmpestrm/8/16/16", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x8);
        chk("_mm_cmpestra/8/16/16", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x8);
        chk("_mm_cmpestrc/8/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x8);
        chk("_mm_cmpestro/8/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x8);
        chk("_mm_cmpestrs/8/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x8);
        chk("_mm_cmpestrz/8/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x8);
        chk("_mm_cmpestri/8/5/9", &__r, 4, "\x09\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x8);
        chk("_mm_cmpestrm/8/5/9", &__r, 16, "\x00\xfe\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x8);
        chk("_mm_cmpestra/8/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x8);
        chk("_mm_cmpestrc/8/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x8);
        chk("_mm_cmpestro/8/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x8);
        chk("_mm_cmpestrs/8/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x8);
        chk("_mm_cmpestrz/8/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x8);
        chk("_mm_cmpestri/8/0/3", &__r, 4, "\x03\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x8);
        chk("_mm_cmpestrm/8/0/3", &__r, 16, "\xf8\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x8);
        chk("_mm_cmpestra/8/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x8);
        chk("_mm_cmpestrc/8/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x8);
        chk("_mm_cmpestro/8/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x8);
        chk("_mm_cmpestrs/8/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x8);
        chk("_mm_cmpestrz/8/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistri(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x8);
        chk("_mm_cmpistri/8", &__r, 4, "\x05\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpistrm(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x8);
        chk("_mm_cmpistrm/8", &__r, 16, "\xe0\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistra(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x8);
        chk("_mm_cmpistra/8", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrc(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x8);
        chk("_mm_cmpistrc/8", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistro(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x8);
        chk("_mm_cmpistro/8", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("sse4.2,popcnt"))) void run5(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        int __r = _mm_cmpistrs(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x8);
        chk("_mm_cmpistrs/8", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrz(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x8);
        chk("_mm_cmpistrz/8", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0xc);
        chk("_mm_cmpestri/c/16/16", &__r, 4, "\x10\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0xc);
        chk("_mm_cmpestrm/c/16/16", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0xc);
        chk("_mm_cmpestra/c/16/16", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0xc);
        chk("_mm_cmpestrc/c/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0xc);
        chk("_mm_cmpestro/c/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0xc);
        chk("_mm_cmpestrs/c/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0xc);
        chk("_mm_cmpestrz/c/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0xc);
        chk("_mm_cmpestri/c/5/9", &__r, 4, "\x10\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0xc);
        chk("_mm_cmpestrm/c/5/9", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0xc);
        chk("_mm_cmpestra/c/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0xc);
        chk("_mm_cmpestrc/c/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0xc);
        chk("_mm_cmpestro/c/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0xc);
        chk("_mm_cmpestrs/c/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0xc);
        chk("_mm_cmpestrz/c/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0xc);
        chk("_mm_cmpestri/c/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0xc);
        chk("_mm_cmpestrm/c/0/3", &__r, 16, "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0xc);
        chk("_mm_cmpestra/c/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0xc);
        chk("_mm_cmpestrc/c/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0xc);
        chk("_mm_cmpestro/c/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0xc);
        chk("_mm_cmpestrs/c/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0xc);
        chk("_mm_cmpestrz/c/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistri(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0xc);
        chk("_mm_cmpistri/c", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpistrm(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0xc);
        chk("_mm_cmpistrm/c", &__r, 16, "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistra(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0xc);
        chk("_mm_cmpistra/c", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrc(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0xc);
        chk("_mm_cmpistrc/c", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistro(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0xc);
        chk("_mm_cmpistro/c", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrs(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0xc);
        chk("_mm_cmpistrs/c", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrz(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0xc);
        chk("_mm_cmpistrz/c", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x1);
        chk("_mm_cmpestri/1/16/16", &__r, 4, "\x08\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x1);
        chk("_mm_cmpestrm/1/16/16", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("sse4.2,popcnt"))) void run6(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x1);
        chk("_mm_cmpestra/1/16/16", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x1);
        chk("_mm_cmpestrc/1/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x1);
        chk("_mm_cmpestro/1/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x1);
        chk("_mm_cmpestrs/1/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x1);
        chk("_mm_cmpestrz/1/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x1);
        chk("_mm_cmpestri/1/5/9", &__r, 4, "\x08\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x1);
        chk("_mm_cmpestrm/1/5/9", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x1);
        chk("_mm_cmpestra/1/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x1);
        chk("_mm_cmpestrc/1/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x1);
        chk("_mm_cmpestro/1/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x1);
        chk("_mm_cmpestrs/1/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x1);
        chk("_mm_cmpestrz/1/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x1);
        chk("_mm_cmpestri/1/0/3", &__r, 4, "\x08\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x1);
        chk("_mm_cmpestrm/1/0/3", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x1);
        chk("_mm_cmpestra/1/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x1);
        chk("_mm_cmpestrc/1/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x1);
        chk("_mm_cmpestro/1/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x1);
        chk("_mm_cmpestrs/1/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x1);
        chk("_mm_cmpestrz/1/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistri(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x1);
        chk("_mm_cmpistri/1", &__r, 4, "\x08\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpistrm(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x1);
        chk("_mm_cmpistrm/1", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistra(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x1);
        chk("_mm_cmpistra/1", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrc(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x1);
        chk("_mm_cmpistrc/1", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistro(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x1);
        chk("_mm_cmpistro/1", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrs(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x1);
        chk("_mm_cmpistrs/1", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrz(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x1);
        chk("_mm_cmpistrz/1", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x12);
        chk("_mm_cmpestri/12/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x12);
        chk("_mm_cmpestrm/12/16/16", &__r, 16, "\xdf\xb6\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x12);
        chk("_mm_cmpestra/12/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x12);
        chk("_mm_cmpestrc/12/16/16", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x12);
        chk("_mm_cmpestro/12/16/16", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x12);
        chk("_mm_cmpestrs/12/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("sse4.2,popcnt"))) void run7(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x12);
        chk("_mm_cmpestrz/12/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x12);
        chk("_mm_cmpestri/12/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x12);
        chk("_mm_cmpestrm/12/5/9", &__r, 16, "\xdf\xfe\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x12);
        chk("_mm_cmpestra/12/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x12);
        chk("_mm_cmpestrc/12/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x12);
        chk("_mm_cmpestro/12/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x12);
        chk("_mm_cmpestrs/12/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x12);
        chk("_mm_cmpestrz/12/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x12);
        chk("_mm_cmpestri/12/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x12);
        chk("_mm_cmpestrm/12/0/3", &__r, 16, "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x12);
        chk("_mm_cmpestra/12/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x12);
        chk("_mm_cmpestrc/12/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x12);
        chk("_mm_cmpestro/12/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x12);
        chk("_mm_cmpestrs/12/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x12);
        chk("_mm_cmpestrz/12/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistri(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x12);
        chk("_mm_cmpistri/12", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpistrm(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x12);
        chk("_mm_cmpistrm/12", &__r, 16, "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistra(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x12);
        chk("_mm_cmpistra/12", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrc(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x12);
        chk("_mm_cmpistrc/12", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistro(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x12);
        chk("_mm_cmpistro/12", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrs(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x12);
        chk("_mm_cmpistrs/12", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrz(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x12);
        chk("_mm_cmpistrz/12", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x40);
        chk("_mm_cmpestri/40/16/16", &__r, 4, "\x0e\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x40);
        chk("_mm_cmpestrm/40/16/16", &__r, 16, "\x00\x00\x00\x00\x00\xff\x00\x00\xff\x00\x00\xff\x00\x00\xff\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x40);
        chk("_mm_cmpestra/40/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x40);
        chk("_mm_cmpestrc/40/16/16", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x40);
        chk("_mm_cmpestro/40/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x40);
        chk("_mm_cmpestrs/40/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x40);
        chk("_mm_cmpestrz/40/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x40);
        chk("_mm_cmpestri/40/5/9", &__r, 4, "\x08\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x40);
        chk("_mm_cmpestrm/40/5/9", &__r, 16, "\x00\x00\x00\x00\x00\xff\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x40);
        chk("_mm_cmpestra/40/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("sse4.2,popcnt"))) void run8(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x40);
        chk("_mm_cmpestrc/40/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x40);
        chk("_mm_cmpestro/40/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x40);
        chk("_mm_cmpestrs/40/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x40);
        chk("_mm_cmpestrz/40/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x40);
        chk("_mm_cmpestri/40/0/3", &__r, 4, "\x10\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x40);
        chk("_mm_cmpestrm/40/0/3", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x40);
        chk("_mm_cmpestra/40/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x40);
        chk("_mm_cmpestrc/40/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x40);
        chk("_mm_cmpestro/40/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x40);
        chk("_mm_cmpestrs/40/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x40);
        chk("_mm_cmpestrz/40/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistri(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x40);
        chk("_mm_cmpistri/40", &__r, 4, "\x10\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpistrm(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x40);
        chk("_mm_cmpistrm/40", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistra(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x40);
        chk("_mm_cmpistra/40", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrc(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x40);
        chk("_mm_cmpistrc/40", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistro(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x40);
        chk("_mm_cmpistro/40", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrs(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x40);
        chk("_mm_cmpistrs/40", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrz(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x40);
        chk("_mm_cmpistrz/40", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x3a);
        chk("_mm_cmpestri/3a/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x3a);
        chk("_mm_cmpestrm/3a/16/16", &__r, 16, "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x3a);
        chk("_mm_cmpestra/3a/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x3a);
        chk("_mm_cmpestrc/3a/16/16", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x3a);
        chk("_mm_cmpestro/3a/16/16", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x3a);
        chk("_mm_cmpestrs/3a/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 16, __L(__m128i, B, 0), 16, 0x3a);
        chk("_mm_cmpestrz/3a/16/16", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x3a);
        chk("_mm_cmpestri/3a/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x3a);
        chk("_mm_cmpestrm/3a/5/9", &__r, 16, "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x3a);
        chk("_mm_cmpestra/3a/5/9", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x3a);
        chk("_mm_cmpestrc/3a/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x3a);
        chk("_mm_cmpestro/3a/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x3a);
        chk("_mm_cmpestrs/3a/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 5, __L(__m128i, B, 0), 9, 0x3a);
        chk("_mm_cmpestrz/3a/5/9", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("sse4.2,popcnt"))) void run9(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        int __r = _mm_cmpestri(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x3a);
        chk("_mm_cmpestri/3a/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpestrm(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x3a);
        chk("_mm_cmpestrm/3a/0/3", &__r, 16, "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestra(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x3a);
        chk("_mm_cmpestra/3a/0/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrc(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x3a);
        chk("_mm_cmpestrc/3a/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestro(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x3a);
        chk("_mm_cmpestro/3a/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrs(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x3a);
        chk("_mm_cmpestrs/3a/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpestrz(__L(__m128i, A, 0), 0, __L(__m128i, B, 0), 3, 0x3a);
        chk("_mm_cmpestrz/3a/0/3", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistri(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x3a);
        chk("_mm_cmpistri/3a", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpistrm(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x3a);
        chk("_mm_cmpistrm/3a", &__r, 16, "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistra(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x3a);
        chk("_mm_cmpistra/3a", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrc(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x3a);
        chk("_mm_cmpistrc/3a", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistro(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x3a);
        chk("_mm_cmpistro/3a", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrs(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x3a);
        chk("_mm_cmpistrs/3a", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cmpistrz(__L(__m128i, AZ, 0), __L(__m128i, B, 0), 0x3a);
        chk("_mm_cmpistrz/3a", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static __axiom_attribute__((__target__("sse4.2,popcnt"))) void run(void) {
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
    if (__cpu(1, 2, bit_SSE4_2) && __cpu(1, 2, bit_POPCNT)) {
        run();
    } else {
        __checks = 302;
    }
    if (__fails == 0) {
        say("intrin_x86_sse4 ");
        say_n(__checks);
        endl();
    }
    return 0;
}
