/* An intrinsic whose extension the calling function is not compiled for
   (here AVX2, in a build for the baseline) is C3240 at the call. */
/* flags: --target=x86_64 */
/* expect-error: C3240 */
#include <immintrin.h>

__m256i f(__m256i a, __m256i b) {
    return _mm256_add_epi32(a, b);
}

int main(void) {
    return 0;
}
