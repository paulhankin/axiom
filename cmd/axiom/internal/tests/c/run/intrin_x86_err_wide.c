/* A 256-bit intrinsic computed by an instruction of its own needs AVX2 as
   well as its extension (the code generator keeps 32-byte vectors in one
   register only with AVX2): _mm256_hadd_ps with AVX alone is C3240. */
/* flags: --target=x86_64 --features=+avx */
/* expect-error: C3240 */
#include <immintrin.h>

__m256 f(__m256 a, __m256 b) {
    return _mm256_hadd_ps(a, b);
}

int main(void) {
    return 0;
}
