/* A function compiled for an extension (the attribute target) may call its
   intrinsics, in an ISO mode too (spelled __axiom_attribute__ there): a
   program built for the baseline checks the processor first. Its code is
   the extension's instructions (here SSE4.1's pmulld and SSSE3's pshufb, in
   an AVX2 function their VEX forms), whether inlined or not (-O0). */
/* x86-64-only */
/* flags: --c-std=c11 */
/* expect: 1 */
/* expect: 16 9 4 1 */
#include <smmintrin.h>
#include <immintrin.h>
#include <cpuid.h>
#include "host.h"

static __axiom_attribute__((__target__("sse4.1"))) int square_rev(int *out, const int *in) {
    __m128i v = _mm_loadu_si128((const __m128i *)(const void *)in);
    v = _mm_mullo_epi32(v, v);
    v = _mm_shuffle_epi8(v, _mm_setr_epi8(12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3));
    _mm_storeu_si128((__m128i *)(void *)out, v);
    return 1;
}

static __axiom_attribute__((__target__("avx2"))) int sum8(const int *in) {
    __m256i v = _mm256_loadu_si256((const __m256i *)(const void *)in);
    __m128i s = _mm_add_epi32(_mm256_castsi256_si128(v), _mm256_extracti128_si256(v, 1));
    s = _mm_hadd_epi32(s, s);
    s = _mm_hadd_epi32(s, s);
    return _mm_cvtsi128_si32(s);
}

int main(void) {
    unsigned int a, b, c, d;
    int in[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int out[4];
    int ok = 1;
    if (__get_cpuid(1, &a, &b, &c, &d) && (c & bit_SSE4_1)) {
        ok = square_rev(out, in);
    } else {
        out[0] = 16, out[1] = 9, out[2] = 4, out[3] = 1;
    }
    if (__get_cpuid_count(7, 0, &a, &b, &c, &d) && (b & bit_AVX2)) {
        ok = ok && sum8(in) == 36;
    }
    print_i64(ok);
    {
        char line[32];
        int n = 0;
        int i;
        for (i = 0; i < 4; i++) {
            int v = out[i];
            if (v >= 10) {
                line[n++] = (char)('0' + v / 10);
            }
            line[n++] = (char)('0' + v % 10);
            line[n++] = i < 3 ? ' ' : 0;
        }
        print_str(line, (size_t)n - 1);
    }
    return 0;
}
