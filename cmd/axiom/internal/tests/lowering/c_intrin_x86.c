/* The x86-64 intrinsics' instructions: generic vector operations are the
   instruction selector's (a saturating add paddusb, a multiply-add pmaddwd,
   a vector add paddd, an unaligned load movdqu), the others the instruction
   each names (pshufb, cvttps2dq, minps, packuswb, pmovmskb, crc32, aesenc,
   pclmulqdq, sha256rnds2); in a function compiled with AVX the VEX forms
   (vpshufb, vpmaddwd, vfmadd, vpgatherdd). The loops of a saturating blend
   and of a dot product have the instructions of hand-written assembly:
   two loads, the operation(s), a store or an add, and the loop's count. */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bpaddusb\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bpmaddwd\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bvpmaddwd\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bpshufb\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bvpshufb\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bcvttps2dq\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bminps\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bpackuswb\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bpmovmskb\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bcrc32\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \baesenc\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bpclmul\w*\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bsha256rnds2\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bvfmadd\d+ps\b | 1
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bvpgatherdd\b | 1
#include <x86intrin.h>
#include <stddef.h>

void blend(unsigned char *d, const unsigned char *a, const unsigned char *b, size_t n) {
    size_t i;
    for (i = 0; i < n; i += 16) {
        __m128i x = _mm_loadu_si128((const __m128i *)(const void *)(a + i));
        __m128i y = _mm_loadu_si128((const __m128i *)(const void *)(b + i));
        _mm_storeu_si128((__m128i *)(void *)(d + i), _mm_adds_epu8(x, y));
    }
}

int dot(const short *a, const short *b, size_t n) {
    __m128i acc = _mm_setzero_si128();
    size_t i;
    for (i = 0; i < n; i += 8) {
        __m128i x = _mm_loadu_si128((const __m128i *)(const void *)(a + i));
        __m128i y = _mm_loadu_si128((const __m128i *)(const void *)(b + i));
        acc = _mm_add_epi32(acc, _mm_madd_epi16(x, y));
    }
    return _mm_cvtsi128_si32(acc);
}

__axiom_attribute__((__target__("avx2"))) int dot8(const short *a, const short *b, size_t n) {
    __m256i acc = _mm256_setzero_si256();
    size_t i;
    for (i = 0; i < n; i += 16) {
        __m256i x = _mm256_loadu_si256((const __m256i *)(const void *)(a + i));
        __m256i y = _mm256_loadu_si256((const __m256i *)(const void *)(b + i));
        acc = _mm256_add_epi32(acc, _mm256_madd_epi16(x, y));
    }
    return _mm256_cvtsi256_si32(acc);
}

__axiom_attribute__((__target__("ssse3"))) __m128i shuf(__m128i a, __m128i b) {
    return _mm_shuffle_epi8(a, b);
}

__axiom_attribute__((__target__("avx2"))) __m128i shuf_vex(__m128i a, __m128i b) {
    return _mm_shuffle_epi8(a, b);
}

__m128i conv(__m128 a, __m128 b, __m128i c) {
    return _mm_packus_epi16(_mm_cvttps_epi32(_mm_min_ps(a, b)), c);
}

int mask(__m128i a) {
    return _mm_movemask_epi8(a);
}

__axiom_attribute__((__target__("sse4.2"))) unsigned long long crc(unsigned long long c, unsigned long long x) {
    return _mm_crc32_u64(c, x);
}

__axiom_attribute__((__target__("aes,pclmul,sha"))) __m128i crypto(__m128i a, __m128i b, __m128i k) {
    return _mm_xor_si128(_mm_aesenc_si128(a, b), _mm_xor_si128(_mm_clmulepi64_si128(a, b, 0x10),
                                                               _mm_sha256rnds2_epu32(a, b, k)));
}

__axiom_attribute__((__target__("avx2,fma"))) __m256 fma8(__m256 a, __m256 b, __m256 c) {
    return _mm256_fmadd_ps(a, b, c);
}

__axiom_attribute__((__target__("avx2"))) __m256i gather(const int *p, __m256i i) {
    return _mm256_i32gather_epi32(p, i, 4);
}
