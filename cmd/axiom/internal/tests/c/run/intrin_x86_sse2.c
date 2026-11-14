/* intrin_x86_sse2 - x86-64 intrinsics checked against the instruction set reference's
   definitions (computed by cmd/internal/tools/gen_x86_intrin.py, which
   writes this file: do not edit): SSE, SSE2 and the __m64 forms (the baseline), in C89. A mismatch prints the intrinsic
   and the result's bytes. */
/* x86-64-only */
/* flags: --c-std=c89 */
/* expect: intrin_x86_sse2 325 */
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

static void run0(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128i __r = _mm_add_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_add_epi8", &__r, 16, "\xa4\x22\xf8\xa1\xd0\x7b\xa3\x5f\x20\x5a\x3e\xef\x1d\x35\xbc\x2b", 0);
    }
    {
        __m128i __r = _mm_sub_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_sub_epi8", &__r, 16, "\x5c\x58\x22\x5d\x0a\x7b\x5b\x27\x22\xa6\xce\xf1\xe5\x7b\xbc\xd1", 0);
    }
    {
        __m128i __r = _mm_add_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_add_epi16", &__r, 16, "\xa4\x22\xf8\xa1\xd0\x7c\xa3\x5f\x20\x5b\x3e\xef\x1d\x35\xbc\x2b", 0);
    }
    {
        __m128i __r = _mm_sub_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_sub_epi16", &__r, 16, "\x5c\x57\x22\x5d\x0a\x7b\x5b\x27\x22\xa5\xce\xf0\xe5\x7a\xbc\xd0", 0);
    }
    {
        __m128i __r = _mm_add_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_add_epi32", &__r, 16, "\xa4\x22\xf9\xa1\xd0\x7c\xa3\x5f\x20\x5b\x3f\xef\x1d\x35\xbd\x2b", 0);
    }
    {
        __m128i __r = _mm_sub_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_sub_epi32", &__r, 16, "\x5c\x57\x21\x5d\x0a\x7b\x5b\x27\x22\xa5\xcd\xf0\xe5\x7a\xbc\xd0", 0);
    }
    {
        __m128i __r = _mm_add_epi64(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_add_epi64", &__r, 16, "\xa4\x22\xf9\xa1\xd1\x7c\xa3\x5f\x20\x5b\x3f\xef\x1d\x35\xbd\x2b", 0);
    }
    {
        __m128i __r = _mm_sub_epi64(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_sub_epi64", &__r, 16, "\x5c\x57\x21\x5d\x0a\x7b\x5b\x27\x22\xa5\xcd\xf0\xe4\x7a\xbc\xd0", 0);
    }
    {
        __m128i __r = _mm_adds_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_adds_epi8", &__r, 16, "\xa4\x22\xf8\xa1\xd0\x7b\x7f\x80\x20\x80\x3e\x7f\x1d\x35\xbc\x2b", 0);
    }
    {
        __m128i __r = _mm_subs_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_subs_epi8", &__r, 16, "\x5c\x58\x80\x5d\x0a\x7b\x5b\x27\x22\xa6\xce\xf1\xe5\x80\x7f\xd1", 0);
    }
    {
        __m128i __r = _mm_adds_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_adds_epi16", &__r, 16, "\xa4\x22\xf8\xa1\xd0\x7c\x00\x80\x00\x80\xff\x7f\x1d\x35\xbc\x2b", 0);
    }
    {
        __m128i __r = _mm_subs_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_subs_epi16", &__r, 16, "\x5c\x57\x22\x5d\x0a\x7b\x5b\x27\x22\xa5\xce\xf0\x00\x80\xbc\xd0", 0);
    }
    {
        __m128i __r = _mm_adds_epu8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_adds_epu8", &__r, 16, "\xa4\xff\xf8\xff\xff\x7b\xa3\xff\xff\xff\x3e\xef\x1d\xff\xbc\xff", 0);
    }
    {
        __m128i __r = _mm_subs_epu8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_subs_epu8", &__r, 16, "\x00\x00\x22\x5d\x0a\x7b\x5b\x27\x00\x00\x00\x00\x00\x7b\x00\xd1", 0);
    }
    {
        __m128i __r = _mm_adds_epu16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_adds_epu16", &__r, 16, "\xff\xff\xff\xff\xd0\x7c\xff\xff\xff\xff\x3e\xef\xff\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm_subs_epu16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_subs_epu16", &__r, 16, "\x00\x00\x22\x5d\x0a\x7b\x5b\x27\x00\x00\x00\x00\xe5\x7a\xbc\xd0", 0);
    }
    {
        __m128i __r = _mm_avg_epu8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_avg_epu8", &__r, 16, "\x52\x91\x7c\xd1\xe8\x3e\x52\xb0\x90\xad\x1f\x78\x0f\x9b\x5e\x96", 0);
    }
    {
        __m128i __r = _mm_avg_epu16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_avg_epu16", &__r, 16, "\x52\x91\xfc\xd0\x68\x3e\xd2\xaf\x90\xad\x9f\x77\x8f\x9a\xde\x95", 0);
    }
    {
        __m128i __r = _mm_mulhi_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_mulhi_epi16", &__r, 16, "\xb8\xf9\x2a\x00\x6d\x00\x99\x17\x7b\x12\xab\x37\x73\xf1\xaf\xff", 0);
    }
    {
        __m128i __r = _mm_mulhi_epu16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_mulhi_epu16", &__r, 16, "\xb8\x36\x22\xa2\x6d\x00\x3c\x77\x9b\x6d\xab\x37\x8f\x4e\x2f\x2d", 0);
    }
    {
        __m128i __r = _mm_mullo_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_mullo_epi16", &__r, 16, "\x00\x14\xef\x09\x27\xe3\xdc\xe1\xdf\xba\x50\x7b\x1c\xfd\x00\xaa", 0);
    }
    {
        __m128i __r = _mm_mul_epu32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_mul_epu32", &__r, 16, "\x00\x14\x0c\x8d\xdf\x15\x23\xa2\xdf\xba\xcd\x85\xd1\x1a\xac\x37", 0);
    }
    {
        __m128i __r = _mm_madd_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_madd_epi16", &__r, 16, "\xef\x1d\xe2\xf9\x03\xc5\x07\x18\x2f\x36\x27\x4a\x1c\xa7\x23\xf1", 0);
    }
    {
        __m128i __r = _mm_sad_epu8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_sad_epu8", &__r, 16, "\xd2\x02\x00\x00\x00\x00\x00\x00\x24\x03\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_min_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_min_epi16", &__r, 16, "\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_max_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_max_epi16", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\xff\xda\x38\x7f\x1c\x5d\x80\x2d", 0);
    }
    {
        __m128i __r = _mm_min_epu8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_min_epu8", &__r, 16, "\x00\x3d\x6b\xa2\xe3\x00\x24\x9c\x21\x80\x06\x70\x01\x5d\x3c\x2d", 0);
    }
    {
        __m128i __r = _mm_max_epu8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_max_epu8", &__r, 16, "\xa4\xe5\x8d\xff\xed\x7b\x7f\xc3\xff\xda\x38\x7f\x1c\xd8\x80\xfe", 0);
    }
    {
        __m128i __r = _mm_cmpeq_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_cmpeq_epi8", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpgt_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_cmpgt_epi8", &__r, 16, "\xff\xff\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\xff\x00", 0);
    }
    {
        __m128i __r = _mm_cmpeq_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_cmpeq_epi16", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpgt_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_cmpgt_epi16", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static void run1(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128i __r = _mm_cmpeq_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_cmpeq_epi32", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cmpgt_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_cmpgt_epi32", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_and_si128(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_and_si128", &__r, 16, "\x00\x25\x09\xa2\xe1\x00\x24\x80\x21\x80\x00\x70\x00\x58\x00\x2c", 0);
    }
    {
        __m128i __r = _mm_andnot_si128(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_andnot_si128", &__r, 16, "\xa4\xc0\x62\x00\x02\x00\x00\x1c\xde\x5a\x38\x0f\x1c\x05\x80\x01", 0);
    }
    {
        __m128i __r = _mm_or_si128(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_or_si128", &__r, 16, "\xa4\xfd\xef\xff\xef\x7b\x7f\xdf\xff\xda\x3e\x7f\x1d\xdd\xbc\xff", 0);
    }
    {
        __m128i __r = _mm_xor_si128(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_xor_si128", &__r, 16, "\xa4\xd8\xe6\x5d\x0e\x7b\x5b\x5f\xde\x5a\x3e\x0f\x1d\x85\xbc\xd3", 0);
    }
    {
        __m128i __r = _mm_packs_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_packs_epi16", &__r, 16, "\x7f\x8d\x7f\x80\x80\x7f\x80\x80\x80\x80\x7f\x80\x80\x7f\x7f\x7f", 0);
    }
    {
        __m128i __r = _mm_packs_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_packs_epi32", &__r, 16, "\x00\x80\x00\x80\xff\x7f\x00\x80\x00\x80\x00\x80\xff\x7f\xff\x7f", 0);
    }
    {
        __m128i __r = _mm_packus_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_packus_epi16", &__r, 16, "\xff\x00\xff\x00\x00\xff\x00\x00\x00\x00\xe3\x00\x00\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm_unpacklo_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_unpacklo_epi8", &__r, 16, "\x00\xa4\x3d\xe5\x8d\x6b\xff\xa2\xed\xe3\x7b\x00\x7f\x24\xc3\x9c", 0);
    }
    {
        __m128i __r = _mm_unpackhi_epi8(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_unpackhi_epi8", &__r, 16, "\x21\xff\x80\xda\x06\x38\x70\x7f\x01\x1c\xd8\x5d\x3c\x80\xfe\x2d", 0);
    }
    {
        __m128i __r = _mm_unpacklo_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_unpacklo_epi16", &__r, 16, "\x00\x3d\xa4\xe5\x8d\xff\x6b\xa2\xed\x7b\xe3\x00\x7f\xc3\x24\x9c", 0);
    }
    {
        __m128i __r = _mm_unpackhi_epi16(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_unpackhi_epi16", &__r, 16, "\x21\x80\xff\xda\x06\x70\x38\x7f\x01\xd8\x1c\x5d\x3c\xfe\x80\x2d", 0);
    }
    {
        __m128i __r = _mm_unpacklo_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_unpacklo_epi32", &__r, 16, "\x00\x3d\x8d\xff\xa4\xe5\x6b\xa2\xed\x7b\x7f\xc3\xe3\x00\x24\x9c", 0);
    }
    {
        __m128i __r = _mm_unpackhi_epi32(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_unpackhi_epi32", &__r, 16, "\x21\x80\x06\x70\xff\xda\x38\x7f\x01\xd8\x3c\xfe\x1c\x5d\x80\x2d", 0);
    }
    {
        __m128i __r = _mm_unpacklo_epi64(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_unpacklo_epi64", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\xa4\xe5\x6b\xa2\xe3\x00\x24\x9c", 0);
    }
    {
        __m128i __r = _mm_unpackhi_epi64(__L(__m128i, A, 0), __L(__m128i, B, 0));
        chk("_mm_unpackhi_epi64", &__r, 16, "\x21\x80\x06\x70\x01\xd8\x3c\xfe\xff\xda\x38\x7f\x1c\x5d\x80\x2d", 0);
    }
    {
        int __r = _mm_movemask_epi8(__L(__m128i, A, 0));
        chk("_mm_movemask_epi8", &__r, 4, "\x9c\xa2\x00\x00", 0);
    }
    {
        __m128i __r = _mm_slli_epi16(__L(__m128i, A, 0), 0);
        chk("_mm_slli_epi16/0", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_srli_epi16(__L(__m128i, A, 0), 0);
        chk("_mm_srli_epi16/0", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_srai_epi16(__L(__m128i, A, 0), 0);
        chk("_mm_srai_epi16/0", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_slli_epi16(__L(__m128i, A, 0), 1);
        chk("_mm_slli_epi16/1", &__r, 16, "\x00\x7a\x1a\xff\xda\xf7\xfe\x86\x42\x00\x0c\xe0\x02\xb0\x78\xfc", 0);
    }
    {
        __m128i __r = _mm_srli_epi16(__L(__m128i, A, 0), 1);
        chk("_mm_srli_epi16/1", &__r, 16, "\x80\x1e\xc6\x7f\xf6\x3d\xbf\x61\x10\x40\x03\x38\x00\x6c\x1e\x7f", 0);
    }
    {
        __m128i __r = _mm_srai_epi16(__L(__m128i, A, 0), 1);
        chk("_mm_srai_epi16/1", &__r, 16, "\x80\x1e\xc6\xff\xf6\x3d\xbf\xe1\x10\xc0\x03\x38\x00\xec\x1e\xff", 0);
    }
    {
        __m128i __r = _mm_slli_epi16(__L(__m128i, A, 0), 15);
        chk("_mm_slli_epi16/15", &__r, 16, "\x00\x00\x00\x80\x00\x80\x00\x80\x00\x80\x00\x00\x00\x80\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srli_epi16(__L(__m128i, A, 0), 15);
        chk("_mm_srli_epi16/15", &__r, 16, "\x00\x00\x01\x00\x00\x00\x01\x00\x01\x00\x00\x00\x01\x00\x01\x00", 0);
    }
    {
        __m128i __r = _mm_srai_epi16(__L(__m128i, A, 0), 15);
        chk("_mm_srai_epi16/15", &__r, 16, "\x00\x00\xff\xff\x00\x00\xff\xff\xff\xff\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm_slli_epi16(__L(__m128i, A, 0), 16);
        chk("_mm_slli_epi16/16", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srli_epi16(__L(__m128i, A, 0), 16);
        chk("_mm_srli_epi16/16", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srai_epi16(__L(__m128i, A, 0), 16);
        chk("_mm_srai_epi16/16", &__r, 16, "\x00\x00\xff\xff\x00\x00\xff\xff\xff\xff\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm_slli_epi16(__L(__m128i, A, 0), 255);
        chk("_mm_slli_epi16/255", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srli_epi16(__L(__m128i, A, 0), 255);
        chk("_mm_srli_epi16/255", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static void run2(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128i __r = _mm_srai_epi16(__L(__m128i, A, 0), 255);
        chk("_mm_srai_epi16/255", &__r, 16, "\x00\x00\xff\xff\x00\x00\xff\xff\xff\xff\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm_sll_epi16(__L(__m128i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm_sll_epi16/3", &__r, 16, "\x00\xe8\x68\xfc\x68\xdf\xf8\x1b\x08\x01\x30\x80\x08\xc0\xe0\xf1", 0);
    }
    {
        __m128i __r = _mm_srl_epi16(__L(__m128i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm_srl_epi16/3", &__r, 16, "\xa0\x07\xf1\x1f\x7d\x0f\x6f\x18\x04\x10\x00\x0e\x00\x1b\xc7\x1f", 0);
    }
    {
        __m128i __r = _mm_sra_epi16(__L(__m128i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm_sra_epi16/3", &__r, 16, "\xa0\x07\xf1\xff\x7d\x0f\x6f\xf8\x04\xf0\x00\x0e\x00\xfb\xc7\xff", 0);
    }
    {
        __m128i __r = _mm_sll_epi16(__L(__m128i, A, 0), _mm_set_epi64x(0, 17));
        chk("_mm_sll_epi16/17", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srl_epi16(__L(__m128i, A, 0), _mm_set_epi64x(0, 17));
        chk("_mm_srl_epi16/17", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_sra_epi16(__L(__m128i, A, 0), _mm_set_epi64x(0, 17));
        chk("_mm_sra_epi16/17", &__r, 16, "\x00\x00\xff\xff\x00\x00\xff\xff\xff\xff\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm_slli_epi32(__L(__m128i, A, 0), 0);
        chk("_mm_slli_epi32/0", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_srli_epi32(__L(__m128i, A, 0), 0);
        chk("_mm_srli_epi32/0", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_srai_epi32(__L(__m128i, A, 0), 0);
        chk("_mm_srai_epi32/0", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_slli_epi32(__L(__m128i, A, 0), 1);
        chk("_mm_slli_epi32/1", &__r, 16, "\x00\x7a\x1a\xff\xda\xf7\xfe\x86\x42\x00\x0d\xe0\x02\xb0\x79\xfc", 0);
    }
    {
        __m128i __r = _mm_srli_epi32(__L(__m128i, A, 0), 1);
        chk("_mm_srli_epi32/1", &__r, 16, "\x80\x9e\xc6\x7f\xf6\xbd\xbf\x61\x10\x40\x03\x38\x00\x6c\x1e\x7f", 0);
    }
    {
        __m128i __r = _mm_srai_epi32(__L(__m128i, A, 0), 1);
        chk("_mm_srai_epi32/1", &__r, 16, "\x80\x9e\xc6\xff\xf6\xbd\xbf\xe1\x10\x40\x03\x38\x00\x6c\x1e\xff", 0);
    }
    {
        __m128i __r = _mm_slli_epi32(__L(__m128i, A, 0), 31);
        chk("_mm_slli_epi32/31", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x00\x80", 0);
    }
    {
        __m128i __r = _mm_srli_epi32(__L(__m128i, A, 0), 31);
        chk("_mm_srli_epi32/31", &__r, 16, "\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srai_epi32(__L(__m128i, A, 0), 31);
        chk("_mm_srai_epi32/31", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm_slli_epi32(__L(__m128i, A, 0), 32);
        chk("_mm_slli_epi32/32", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srli_epi32(__L(__m128i, A, 0), 32);
        chk("_mm_srli_epi32/32", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srai_epi32(__L(__m128i, A, 0), 32);
        chk("_mm_srai_epi32/32", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm_slli_epi32(__L(__m128i, A, 0), 255);
        chk("_mm_slli_epi32/255", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srli_epi32(__L(__m128i, A, 0), 255);
        chk("_mm_srli_epi32/255", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srai_epi32(__L(__m128i, A, 0), 255);
        chk("_mm_srai_epi32/255", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm_sll_epi32(__L(__m128i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm_sll_epi32/3", &__r, 16, "\x00\xe8\x69\xfc\x68\xdf\xfb\x1b\x08\x01\x34\x80\x08\xc0\xe6\xf1", 0);
    }
    {
        __m128i __r = _mm_srl_epi32(__L(__m128i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm_srl_epi32/3", &__r, 16, "\xa0\xa7\xf1\x1f\x7d\xef\x6f\x18\x04\xd0\x00\x0e\x00\x9b\xc7\x1f", 0);
    }
    {
        __m128i __r = _mm_sra_epi32(__L(__m128i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm_sra_epi32/3", &__r, 16, "\xa0\xa7\xf1\xff\x7d\xef\x6f\xf8\x04\xd0\x00\x0e\x00\x9b\xc7\xff", 0);
    }
    {
        __m128i __r = _mm_sll_epi32(__L(__m128i, A, 0), _mm_set_epi64x(0, 33));
        chk("_mm_sll_epi32/33", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srl_epi32(__L(__m128i, A, 0), _mm_set_epi64x(0, 33));
        chk("_mm_srl_epi32/33", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_sra_epi32(__L(__m128i, A, 0), _mm_set_epi64x(0, 33));
        chk("_mm_sra_epi32/33", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm_slli_epi64(__L(__m128i, A, 0), 0);
        chk("_mm_slli_epi64/0", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_srli_epi64(__L(__m128i, A, 0), 0);
        chk("_mm_srli_epi64/0", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_slli_epi64(__L(__m128i, A, 0), 1);
        chk("_mm_slli_epi64/1", &__r, 16, "\x00\x7a\x1a\xff\xdb\xf7\xfe\x86\x42\x00\x0d\xe0\x02\xb0\x79\xfc", 0);
    }
    {
        __m128i __r = _mm_srli_epi64(__L(__m128i, A, 0), 1);
        chk("_mm_srli_epi64/1", &__r, 16, "\x80\x9e\xc6\xff\xf6\xbd\xbf\x61\x10\x40\x03\xb8\x00\x6c\x1e\x7f", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static void run3(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128i __r = _mm_slli_epi64(__L(__m128i, A, 0), 63);
        chk("_mm_slli_epi64/63", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128i __r = _mm_srli_epi64(__L(__m128i, A, 0), 63);
        chk("_mm_srli_epi64/63", &__r, 16, "\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_slli_epi64(__L(__m128i, A, 0), 64);
        chk("_mm_slli_epi64/64", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srli_epi64(__L(__m128i, A, 0), 64);
        chk("_mm_srli_epi64/64", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_slli_epi64(__L(__m128i, A, 0), 255);
        chk("_mm_slli_epi64/255", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srli_epi64(__L(__m128i, A, 0), 255);
        chk("_mm_srli_epi64/255", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_sll_epi64(__L(__m128i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm_sll_epi64/3", &__r, 16, "\x00\xe8\x69\xfc\x6f\xdf\xfb\x1b\x08\x01\x34\x80\x0b\xc0\xe6\xf1", 0);
    }
    {
        __m128i __r = _mm_srl_epi64(__L(__m128i, A, 0), _mm_set_epi64x(0, 3));
        chk("_mm_srl_epi64/3", &__r, 16, "\xa0\xa7\xf1\xbf\x7d\xef\x6f\x18\x04\xd0\x00\x2e\x00\x9b\xc7\x1f", 0);
    }
    {
        __m128i __r = _mm_sll_epi64(__L(__m128i, A, 0), _mm_set_epi64x(0, 65));
        chk("_mm_sll_epi64/65", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srl_epi64(__L(__m128i, A, 0), _mm_set_epi64x(0, 65));
        chk("_mm_srl_epi64/65", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_slli_si128(__L(__m128i, A, 0), 0);
        chk("_mm_slli_si128/0", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_srli_si128(__L(__m128i, A, 0), 0);
        chk("_mm_srli_si128/0", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_slli_si128(__L(__m128i, A, 0), 3);
        chk("_mm_slli_si128/3", &__r, 16, "\x00\x00\x00\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01", 0);
    }
    {
        __m128i __r = _mm_srli_si128(__L(__m128i, A, 0), 3);
        chk("_mm_srli_si128/3", &__r, 16, "\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_slli_si128(__L(__m128i, A, 0), 15);
        chk("_mm_slli_si128/15", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srli_si128(__L(__m128i, A, 0), 15);
        chk("_mm_srli_si128/15", &__r, 16, "\xfe\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_slli_si128(__L(__m128i, A, 0), 16);
        chk("_mm_slli_si128/16", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_srli_si128(__L(__m128i, A, 0), 16);
        chk("_mm_srli_si128/16", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_shuffle_epi32(__L(__m128i, A, 0), 0x1b);
        chk("_mm_shuffle_epi32/1b", &__r, 16, "\x01\xd8\x3c\xfe\x21\x80\x06\x70\xed\x7b\x7f\xc3\x00\x3d\x8d\xff", 0);
    }
    {
        __m128i __r = _mm_shufflelo_epi16(__L(__m128i, A, 0), 0x1b);
        chk("_mm_shufflelo_epi16/1b", &__r, 16, "\x7f\xc3\xed\x7b\x8d\xff\x00\x3d\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_shufflehi_epi16(__L(__m128i, A, 0), 0x1b);
        chk("_mm_shufflehi_epi16/1b", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x3c\xfe\x01\xd8\x06\x70\x21\x80", 0);
    }
    {
        __m128i __r = _mm_shuffle_epi32(__L(__m128i, A, 0), 0xe4);
        chk("_mm_shuffle_epi32/e4", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_shufflelo_epi16(__L(__m128i, A, 0), 0xe4);
        chk("_mm_shufflelo_epi16/e4", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_shufflehi_epi16(__L(__m128i, A, 0), 0xe4);
        chk("_mm_shufflehi_epi16/e4", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_shuffle_epi32(__L(__m128i, A, 0), 0x4e);
        chk("_mm_shuffle_epi32/4e", &__r, 16, "\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x3d\x8d\xff\xed\x7b\x7f\xc3", 0);
    }
    {
        __m128i __r = _mm_shufflelo_epi16(__L(__m128i, A, 0), 0x4e);
        chk("_mm_shufflelo_epi16/4e", &__r, 16, "\xed\x7b\x7f\xc3\x00\x3d\x8d\xff\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_shufflehi_epi16(__L(__m128i, A, 0), 0x4e);
        chk("_mm_shufflehi_epi16/4e", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x01\xd8\x3c\xfe\x21\x80\x06\x70", 0);
    }
    {
        __m128 __r = _mm_add_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_add_ps", &__r, 16, "\x00\x00\x80\xff\x01\x00\xc0\x7f\x00\x00\x80\x3f\x9e\xc9\x7f\x7f", 0);
    }
    {
        __m128 __r = _mm_sub_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_sub_ps", &__r, 16, "\x00\x00\x80\x7f\x01\x00\xc0\x7f\x00\x00\x80\x3f\x9e\xc9\x7f\xff", 0);
    }
    {
        __m128 __r = _mm_mul_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_mul_ps", &__r, 16, "\x00\x00\xc0\xff\x01\x00\xc0\x7f\x01\x00\x00\x00\x00\x00\x80\xff", 0);
    }
    {
        __m128 __r = _mm_div_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_div_ps", &__r, 16, "\x00\x00\x00\x80\x01\x00\xc0\x7f\x00\x00\x80\x7f\x35\x0a\x30\x80", 0);
    }
    {
        __m128 __r = _mm_sqrt_ps(__L(__m128, FA, 0));
        chk("_mm_sqrt_ps", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xff", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static void run4(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128 __r = _mm_min_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_min_ps", &__r, 16, "\x00\x00\x80\xff\x01\x00\xc0\x7f\x01\x00\x00\x00\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_max_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_max_ps", &__r, 16, "\x00\x00\x00\x00\x01\x00\xc0\x7f\x00\x00\x80\x3f\x9e\xc9\x7f\x7f", 0);
    }
    {
        __m128 __r = _mm_and_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_and_ps", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x40\x3f", 0);
    }
    {
        __m128 __r = _mm_or_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_or_ps", &__r, 16, "\x00\x00\x80\xff\x01\x00\xc0\xff\x01\x00\x80\x3f\x9e\xc9\xff\xff", 0);
    }
    {
        __m128 __r = _mm_xor_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_xor_ps", &__r, 16, "\x00\x00\x80\xff\x01\x00\xc0\xff\x01\x00\x80\x3f\x9e\xc9\xbf\xc0", 0);
    }
    {
        __m128 __r = _mm_andnot_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_andnot_ps", &__r, 16, "\x00\x00\x80\xff\x01\x00\xc0\x7f\x01\x00\x00\x00\x9e\xc9\x3f\x40", 0);
    }
    {
        int __r = _mm_movemask_ps(__L(__m128, FA, 0));
        chk("_mm_movemask_ps", &__r, 4, "\x0a\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_unpacklo_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_unpacklo_ps", &__r, 16, "\x00\x00\x00\x00\x00\x00\x80\xff\x00\x00\x00\x80\x01\x00\xc0\x7f", 0);
    }
    {
        __m128 __r = _mm_unpackhi_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_unpackhi_ps", &__r, 16, "\x00\x00\x80\x3f\x01\x00\x00\x00\x00\x00\xc0\xbf\x9e\xc9\x7f\x7f", 0);
    }
    {
        __m128 __r = _mm_cmpeq_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpeq_ps", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmpeq_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpeq_ss", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_cmplt_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmplt_ps", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmplt_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmplt_ss", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_cmple_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmple_ps", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmple_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmple_ss", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_cmpgt_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpgt_ps", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmpgt_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpgt_ss", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_cmpge_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpge_ps", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmpge_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpge_ss", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_cmpunord_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpunord_ps", &__r, 16, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmpunord_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpunord_ss", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_cmpneq_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpneq_ps", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmpneq_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpneq_ss", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_cmpnlt_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpnlt_ps", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmpnlt_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpnlt_ss", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_cmpnle_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpnle_ps", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_cmpnle_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpnle_ss", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_cmpngt_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpngt_ps", &__r, 16, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmpngt_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpngt_ss", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_cmpnge_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpnge_ps", &__r, 16, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cmpnge_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpnge_ss", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_cmpord_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpord_ps", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static void run5(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128 __r = _mm_cmpord_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_cmpord_ss", &__r, 16, "\xff\xff\xff\xff\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        int __r = _mm_comieq_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_comieq_ss", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_ucomieq_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_ucomieq_ss", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_comilt_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_comilt_ss", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_ucomilt_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_ucomilt_ss", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_comile_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_comile_ss", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_ucomile_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_ucomile_ss", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_comigt_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_comigt_ss", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_ucomigt_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_ucomigt_ss", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_comige_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_comige_ss", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_ucomige_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_ucomige_ss", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_comineq_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_comineq_ss", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_ucomineq_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_ucomineq_ss", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_add_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_add_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\xf8\xbf\x00\x00\x00\x00\x00\x00\xf0\x7f", 0);
    }
    {
        __m128d __r = _mm_sub_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_sub_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\xf8\x3f\x00\x00\x00\x00\x00\x00\xf0\xff", 0);
    }
    {
        __m128d __r = _mm_mul_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_mul_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf8\xff", 0);
    }
    {
        __m128d __r = _mm_div_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_div_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_sqrt_pd(__L(__m128d, DA, 0));
        chk("_mm_sqrt_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_min_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_min_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\xf8\xbf\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_max_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_max_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x7f", 0);
    }
    {
        __m128d __r = _mm_and_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_and_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_or_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_or_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\xf8\xbf\x00\x00\x00\x00\x00\x00\xf0\xff", 0);
    }
    {
        __m128d __r = _mm_xor_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_xor_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\xf8\xbf\x00\x00\x00\x00\x00\x00\xf0\xff", 0);
    }
    {
        __m128d __r = _mm_andnot_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_andnot_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\xf8\xbf\x00\x00\x00\x00\x00\x00\xf0\x7f", 0);
    }
    {
        int __r = _mm_movemask_pd(__L(__m128d, DA, 0));
        chk("_mm_movemask_pd", &__r, 4, "\x02\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_unpacklo_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_unpacklo_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf8\xbf", 0);
    }
    {
        __m128d __r = _mm_unpackhi_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_unpackhi_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\xf0\x7f", 0);
    }
    {
        __m128d __r = _mm_cmpeq_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpeq_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmpeq_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpeq_sd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_cmplt_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmplt_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmplt_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmplt_sd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_cmple_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmple_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static void run6(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128d __r = _mm_cmple_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmple_sd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_cmpgt_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpgt_pd", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmpgt_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpgt_sd", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_cmpge_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpge_pd", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmpge_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpge_sd", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_cmpunord_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpunord_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmpunord_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpunord_sd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_cmpneq_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpneq_pd", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmpneq_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpneq_sd", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_cmpnlt_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpnlt_pd", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmpnlt_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpnlt_sd", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_cmpnle_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpnle_pd", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cmpnle_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpnle_sd", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_cmpngt_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpngt_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmpngt_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpngt_sd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_cmpnge_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpnge_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmpnge_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpnge_sd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128d __r = _mm_cmpord_pd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpord_pd", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128d __r = _mm_cmpord_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_cmpord_sd", &__r, 16, "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        int __r = _mm_comieq_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_comieq_sd", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_ucomieq_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_ucomieq_sd", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_comilt_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_comilt_sd", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_ucomilt_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_ucomilt_sd", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_comile_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_comile_sd", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_ucomile_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_ucomile_sd", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_comigt_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_comigt_sd", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_ucomigt_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_ucomigt_sd", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_comige_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_comige_sd", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_ucomige_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_ucomige_sd", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_comineq_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_comineq_sd", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_ucomineq_sd(__L(__m128d, DA, 0), __L(__m128d, DB, 0));
        chk("_mm_ucomineq_sd", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cvtps_epi32(__L(__m128, FA, 0));
        chk("_mm_cvtps_epi32", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\xfe\xff\xff\xff", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static void run7(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128i __r = _mm_cvttps_epi32(__L(__m128, FA, 0));
        chk("_mm_cvttps_epi32", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\xff\xff\xff\xff", 0);
    }
    {
        __m128 __r = _mm_cvtepi32_ps(__L(__m128i, A, 0));
        chk("_mm_cvtepi32_ps", &__r, 16, "\x00\x86\xe5\xca\x10\x02\x72\xce\x00\x0d\xe0\x4e\x00\x94\xe1\xcb", 0);
    }
    {
        __m128i __r = _mm_cvtpd_epi32(__L(__m128d, DA, 0));
        chk("_mm_cvtpd_epi32", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_cvttpd_epi32(__L(__m128d, DA, 0));
        chk("_mm_cvttpd_epi32", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128d __r = _mm_cvtepi32_pd(__L(__m128i, A, 0));
        chk("_mm_cvtepi32_pd", &__r, 16, "\x00\x00\x00\x00\xc0\xb0\x5c\xc1\x00\x00\x80\x09\x42\x40\xce\xc1", 0);
    }
    {
        __m128d __r = _mm_cvtps_pd(__L(__m128, FA, 0));
        chk("_mm_cvtps_pd", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80", 0);
    }
    {
        __m128 __r = _mm_cvtpd_ps(__L(__m128d, DA, 0));
        chk("_mm_cvtpd_ps", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cvtss_si32(__L(__m128, FA, 0));
        chk("_mm_cvtss_si32/0", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cvtss_si32(__L(__m128, FA, 3));
        chk("_mm_cvtss_si32/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cvtss_si32(__L(__m128, FA, 8));
        chk("_mm_cvtss_si32/8", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cvtss_si32(__L(__m128, FA, 12));
        chk("_mm_cvtss_si32/12", &__r, 4, "\xfe\xff\xff\xff", 0);
    }
    {
        int __r = _mm_cvttss_si32(__L(__m128, FA, 0));
        chk("_mm_cvttss_si32/0", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cvttss_si32(__L(__m128, FA, 3));
        chk("_mm_cvttss_si32/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cvttss_si32(__L(__m128, FA, 8));
        chk("_mm_cvttss_si32/8", &__r, 4, "\x01\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cvttss_si32(__L(__m128, FA, 12));
        chk("_mm_cvttss_si32/12", &__r, 4, "\xff\xff\xff\xff", 0);
    }
    {
        __axiom_int64 __r = _mm_cvtss_si64(__L(__m128, FA, 0));
        chk("_mm_cvtss_si64/0", &__r, 8, "\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __axiom_int64 __r = _mm_cvtss_si64(__L(__m128, FA, 3));
        chk("_mm_cvtss_si64/3", &__r, 8, "\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __axiom_int64 __r = _mm_cvtss_si64(__L(__m128, FA, 8));
        chk("_mm_cvtss_si64/8", &__r, 8, "\x01\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __axiom_int64 __r = _mm_cvtss_si64(__L(__m128, FA, 12));
        chk("_mm_cvtss_si64/12", &__r, 8, "\xfe\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __axiom_int64 __r = _mm_cvttss_si64(__L(__m128, FA, 0));
        chk("_mm_cvttss_si64/0", &__r, 8, "\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __axiom_int64 __r = _mm_cvttss_si64(__L(__m128, FA, 3));
        chk("_mm_cvttss_si64/3", &__r, 8, "\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __axiom_int64 __r = _mm_cvttss_si64(__L(__m128, FA, 8));
        chk("_mm_cvttss_si64/8", &__r, 8, "\x01\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __axiom_int64 __r = _mm_cvttss_si64(__L(__m128, FA, 12));
        chk("_mm_cvttss_si64/12", &__r, 8, "\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        int __r = _mm_cvtsd_si32(__L(__m128d, DA, 0));
        chk("_mm_cvtsd_si32/0", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cvtsd_si32(__L(__m128d, DA, 3));
        chk("_mm_cvtsd_si32/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cvtsd_si32(__L(__m128d, DA, 8));
        chk("_mm_cvtsd_si32/8", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cvtsd_si32(__L(__m128d, DA, 24));
        chk("_mm_cvtsd_si32/24", &__r, 4, "\xfe\xff\xff\xff", 0);
    }
    {
        int __r = _mm_cvttsd_si32(__L(__m128d, DA, 0));
        chk("_mm_cvttsd_si32/0", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cvttsd_si32(__L(__m128d, DA, 3));
        chk("_mm_cvttsd_si32/3", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cvttsd_si32(__L(__m128d, DA, 8));
        chk("_mm_cvttsd_si32/8", &__r, 4, "\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_cvttsd_si32(__L(__m128d, DA, 24));
        chk("_mm_cvttsd_si32/24", &__r, 4, "\xff\xff\xff\xff", 0);
    }
    {
        __axiom_int64 __r = _mm_cvtsd_si64(__L(__m128d, DA, 0));
        chk("_mm_cvtsd_si64/0", &__r, 8, "\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static void run8(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __axiom_int64 __r = _mm_cvtsd_si64(__L(__m128d, DA, 3));
        chk("_mm_cvtsd_si64/3", &__r, 8, "\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __axiom_int64 __r = _mm_cvtsd_si64(__L(__m128d, DA, 8));
        chk("_mm_cvtsd_si64/8", &__r, 8, "\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __axiom_int64 __r = _mm_cvtsd_si64(__L(__m128d, DA, 24));
        chk("_mm_cvtsd_si64/24", &__r, 8, "\xfe\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __axiom_int64 __r = _mm_cvttsd_si64(__L(__m128d, DA, 0));
        chk("_mm_cvttsd_si64/0", &__r, 8, "\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __axiom_int64 __r = _mm_cvttsd_si64(__L(__m128d, DA, 3));
        chk("_mm_cvttsd_si64/3", &__r, 8, "\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __axiom_int64 __r = _mm_cvttsd_si64(__L(__m128d, DA, 8));
        chk("_mm_cvttsd_si64/8", &__r, 8, "\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __axiom_int64 __r = _mm_cvttsd_si64(__L(__m128d, DA, 24));
        chk("_mm_cvttsd_si64/24", &__r, 8, "\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m128i __r = _mm_set_epi32(-29566975, 1879474209, -1015055379, -7521024);
        chk("_mm_set_epi32", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_setr_epi32(-7521024, -1015055379, 1879474209, -29566975);
        chk("_mm_setr_epi32", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_set1_epi8(-3);
        chk("_mm_set1_epi8", &__r, 16, "\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd\xfd", 0);
    }
    {
        __m128i __r = _mm_set1_epi16(-3);
        chk("_mm_set1_epi16", &__r, 16, "\xfd\xff\xfd\xff\xfd\xff\xfd\xff\xfd\xff\xfd\xff\xfd\xff\xfd\xff", 0);
    }
    {
        __m128i __r = _mm_set_epi8(-2, 60, -40, 1, 112, 6, -128, 33, -61, 127, 123, -19, -1, -115, 61, 0);
        chk("_mm_set_epi8", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_set_epi64x(__I64(0xfe3cd801u, 0x70068021u), __I64(0xc37f7bedu, 0xff8d3d00u));
        chk("_mm_set_epi64x", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        __m128i __r = _mm_setzero_si128();
        chk("_mm_setzero_si128", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_loadu_si128((const __m128i_u *)(const void *)(A + 3));
        chk("_mm_loadu_si128/3", &__r, 16, "\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe\x00\x80\x81", 0);
    }
    {
        __m128i __r = _mm_loadl_epi64((const __m128i_u *)(const void *)(A + 5));
        chk("_mm_loadl_epi64", &__r, 16, "\x7b\x7f\xc3\x21\x80\x06\x70\x01\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_loadu_si32(A + 1);
        chk("_mm_loadu_si32", &__r, 16, "\x3d\x8d\xff\xed\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        int __r = _mm_extract_epi16(__L(__m128i, A, 0), 0);
        chk("_mm_extract_epi16/0", &__r, 4, "\x00\x3d\x00\x00", 0);
    }
    {
        __m128i __r = _mm_insert_epi16(__L(__m128i, A, 0), -2, 0);
        chk("_mm_insert_epi16/0", &__r, 16, "\xfe\xff\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        int __r = _mm_extract_epi16(__L(__m128i, A, 0), 3);
        chk("_mm_extract_epi16/3", &__r, 4, "\x7f\xc3\x00\x00", 0);
    }
    {
        __m128i __r = _mm_insert_epi16(__L(__m128i, A, 0), -2, 3);
        chk("_mm_insert_epi16/3", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\xfe\xff\x21\x80\x06\x70\x01\xd8\x3c\xfe", 0);
    }
    {
        int __r = _mm_extract_epi16(__L(__m128i, A, 0), 7);
        chk("_mm_extract_epi16/7", &__r, 4, "\x3c\xfe\x00\x00", 0);
    }
    {
        __m128i __r = _mm_insert_epi16(__L(__m128i, A, 0), -2, 7);
        chk("_mm_insert_epi16/7", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x21\x80\x06\x70\x01\xd8\xfe\xff", 0);
    }
    {
        int __r = _mm_cvtsi128_si32(__L(__m128i, A, 0));
        chk("_mm_cvtsi128_si32", &__r, 4, "\x00\x3d\x8d\xff", 0);
    }
    {
        __m128i __r = _mm_cvtsi32_si128(1879474209);
        chk("_mm_cvtsi32_si128", &__r, 16, "\x21\x80\x06\x70\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128i __r = _mm_move_epi64(__L(__m128i, A, 0));
        chk("_mm_move_epi64", &__r, 16, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_shuffle_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 0x1b);
        chk("_mm_shuffle_ps/1b", &__r, 16, "\x00\x00\xc0\xbf\x00\x00\x80\x3f\x01\x00\xc0\x7f\x00\x00\x80\xff", 0);
    }
    {
        __m128 __r = _mm_shuffle_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 0x4e);
        chk("_mm_shuffle_ps/4e", &__r, 16, "\x00\x00\x80\x3f\x00\x00\xc0\xbf\x00\x00\x80\xff\x01\x00\xc0\x7f", 0);
    }
    {
        __m128 __r = _mm_shuffle_ps(__L(__m128, FA, 0), __L(__m128, FB, 0), 0xd8);
        chk("_mm_shuffle_ps/d8", &__r, 16, "\x00\x00\x00\x00\x00\x00\x80\x3f\x01\x00\xc0\x7f\x9e\xc9\x7f\x7f", 0);
    }
    {
        __m128 __r = _mm_movehl_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_movehl_ps", &__r, 16, "\x01\x00\x00\x00\x9e\xc9\x7f\x7f\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_movelh_ps(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_movelh_ps", &__r, 16, "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\xff\x01\x00\xc0\x7f", 0);
    }
    {
        __m128 __r = _mm_move_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_move_ss", &__r, 16, "\x00\x00\x80\xff\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static void run9(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m128 __r = _mm_add_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_add_ss/0", &__r, 16, "\x00\x00\x80\xff\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_add_ss(__L(__m128, FA, 4), __L(__m128, FB, 4));
        chk("_mm_add_ss/4", &__r, 16, "\x01\x00\xc0\x7f\x00\x00\x80\x3f\x00\x00\xc0\xbf\x00\x00\x80\x7f", 0);
    }
    {
        __m128 __r = _mm_add_ss(__L(__m128, FA, 16), __L(__m128, FB, 16));
        chk("_mm_add_ss/16", &__r, 16, "\x00\x00\x80\x7f\x00\x00\x80\xff\x01\x00\xc0\x7f\x01\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_sub_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_sub_ss/0", &__r, 16, "\x00\x00\x80\x7f\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_sub_ss(__L(__m128, FA, 4), __L(__m128, FB, 4));
        chk("_mm_sub_ss/4", &__r, 16, "\x01\x00\xc0\x7f\x00\x00\x80\x3f\x00\x00\xc0\xbf\x00\x00\x80\x7f", 0);
    }
    {
        __m128 __r = _mm_sub_ss(__L(__m128, FA, 16), __L(__m128, FB, 16));
        chk("_mm_sub_ss/16", &__r, 16, "\x00\x00\x80\x7f\x00\x00\x80\xff\x01\x00\xc0\x7f\x01\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_mul_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_mul_ss/0", &__r, 16, "\x00\x00\xc0\xff\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_mul_ss(__L(__m128, FA, 4), __L(__m128, FB, 4));
        chk("_mm_mul_ss/4", &__r, 16, "\x01\x00\xc0\x7f\x00\x00\x80\x3f\x00\x00\xc0\xbf\x00\x00\x80\x7f", 0);
    }
    {
        __m128 __r = _mm_mul_ss(__L(__m128, FA, 16), __L(__m128, FB, 16));
        chk("_mm_mul_ss/16", &__r, 16, "\x00\x00\x80\x7f\x00\x00\x80\xff\x01\x00\xc0\x7f\x01\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_div_ss(__L(__m128, FA, 0), __L(__m128, FB, 0));
        chk("_mm_div_ss/0", &__r, 16, "\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x80\x3f\x00\x00\xc0\xbf", 0);
    }
    {
        __m128 __r = _mm_div_ss(__L(__m128, FA, 4), __L(__m128, FB, 4));
        chk("_mm_div_ss/4", &__r, 16, "\x01\x00\xc0\x7f\x00\x00\x80\x3f\x00\x00\xc0\xbf\x00\x00\x80\x7f", 0);
    }
    {
        __m128 __r = _mm_div_ss(__L(__m128, FA, 16), __L(__m128, FB, 16));
        chk("_mm_div_ss/16", &__r, 16, "\x00\x00\x80\x7f\x00\x00\x80\xff\x01\x00\xc0\x7f\x01\x00\x00\x00", 0);
    }
    {
        __m128 __r = _mm_rcp_ps(__L(__m128, FC, 32));
        chk("_mm_rcp_ps", &__r, 16, "\x39\xdd\x18\xbd\xd3\x24\xb6\xbd\xd2\xce\x3f\x3c\x2d\x26\x39\xbc", 1);
    }
    {
        __m128 __r = _mm_rsqrt_ps(__L(__m128, FP, 0));
        chk("_mm_rsqrt_ps", &__r, 16, "\x00\x00\x80\x3f\xf3\x04\x35\x3f\x3a\xcd\x13\x3f\x00\x00\x00\x3f", 1);
    }
    {
        __m64 __r = _mm_add_pi8(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_add_pi8", &__r, 8, "\xa4\x22\xf8\xa1\xd0\x7b\xa3\x5f", 0);
    }
    {
        __m64 __r = _mm_adds_pu8(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_adds_pu8", &__r, 8, "\xa4\xff\xf8\xff\xff\x7b\xa3\xff", 0);
    }
    {
        __m64 __r = _mm_subs_pi16(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_subs_pi16", &__r, 8, "\x5c\x57\x22\x5d\x0a\x7b\x5b\x27", 0);
    }
    {
        __m64 __r = _mm_mulhi_pi16(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_mulhi_pi16", &__r, 8, "\xb8\xf9\x2a\x00\x6d\x00\x99\x17", 0);
    }
    {
        __m64 __r = _mm_madd_pi16(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_madd_pi16", &__r, 8, "\xef\x1d\xe2\xf9\x03\xc5\x07\x18", 0);
    }
    {
        __m64 __r = _mm_cmpgt_pi16(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_cmpgt_pi16", &__r, 8, "\xff\xff\xff\xff\xff\xff\xff\xff", 0);
    }
    {
        __m64 __r = _mm_xor_si64(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_xor_si64", &__r, 8, "\xa4\xd8\xe6\x5d\x0e\x7b\x5b\x5f", 0);
    }
    {
        __m64 __r = _mm_packs_pi16(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_packs_pi16", &__r, 8, "\x7f\x8d\x7f\x80\x80\x80\x7f\x80", 0);
    }
    {
        __m64 __r = _mm_packs_pu16(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_packs_pu16", &__r, 8, "\xff\x00\xff\x00\x00\x00\xe3\x00", 0);
    }
    {
        __m64 __r = _mm_unpacklo_pi8(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_unpacklo_pi8", &__r, 8, "\x00\xa4\x3d\xe5\x8d\x6b\xff\xa2", 0);
    }
    {
        __m64 __r = _mm_unpackhi_pi16(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_unpackhi_pi16", &__r, 8, "\xed\x7b\xe3\x00\x7f\xc3\x24\x9c", 0);
    }
    {
        __m64 __r = _mm_avg_pu8(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_avg_pu8", &__r, 8, "\x52\x91\x7c\xd1\xe8\x3e\x52\xb0", 0);
    }
    {
        __m64 __r = _mm_max_pi16(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_max_pi16", &__r, 8, "\x00\x3d\x8d\xff\xed\x7b\x7f\xc3", 0);
    }
    {
        __m64 __r = _mm_min_pu8(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_min_pu8", &__r, 8, "\x00\x3d\x6b\xa2\xe3\x00\x24\x9c", 0);
    }
    {
        __m64 __r = _mm_sad_pu8(__L(__m64, A, 0), __L(__m64, B, 0));
        chk("_mm_sad_pu8", &__r, 8, "\xd2\x02\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m64 __r = _mm_slli_pi16(__L(__m64, A, 0), 1);
        chk("_mm_slli_pi16/1", &__r, 8, "\x00\x7a\x1a\xff\xda\xf7\xfe\x86", 0);
    }
    {
        __m64 __r = _mm_srai_pi32(__L(__m64, A, 0), 1);
        chk("_mm_srai_pi32/1", &__r, 8, "\x80\x9e\xc6\xff\xf6\xbd\xbf\xe1", 0);
    }
    {
        __m64 __r = _mm_slli_pi16(__L(__m64, A, 0), 15);
        chk("_mm_slli_pi16/15", &__r, 8, "\x00\x00\x00\x80\x00\x80\x00\x80", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static void run10(void) {
    unsigned __axiom_int64 __hi = 0;
    unsigned int __hi32 = 0;
    unsigned int __o32 = 0;
    unsigned __axiom_int64 __o64 = 0;
    {
        __m64 __r = _mm_srai_pi32(__L(__m64, A, 0), 15);
        chk("_mm_srai_pi32/15", &__r, 8, "\x1a\xff\xff\xff\xfe\x86\xff\xff", 0);
    }
    {
        __m64 __r = _mm_slli_pi16(__L(__m64, A, 0), 16);
        chk("_mm_slli_pi16/16", &__r, 8, "\x00\x00\x00\x00\x00\x00\x00\x00", 0);
    }
    {
        __m64 __r = _mm_srai_pi32(__L(__m64, A, 0), 16);
        chk("_mm_srai_pi32/16", &__r, 8, "\x8d\xff\xff\xff\x7f\xc3\xff\xff", 0);
    }
    {
        int __r = _mm_movemask_pi8(__L(__m64, A, 0));
        chk("_mm_movemask_pi8", &__r, 4, "\x9c\x00\x00\x00", 0);
    }
    {
        __m64 __r = _mm_shuffle_pi16(__L(__m64, A, 0), 0x1b);
        chk("_mm_shuffle_pi16/1b", &__r, 8, "\x7f\xc3\xed\x7b\x8d\xff\x00\x3d", 0);
    }
    (void)__hi;
    (void)__hi32;
    (void)__o32;
    (void)__o64;
}

static void run(void) {
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
    if (1) {
        run();
    } else {
        __checks = 325;
    }
    if (__fails == 0) {
        say("intrin_x86_sse2 ");
        say_n(__checks);
        endl();
    }
    return 0;
}
