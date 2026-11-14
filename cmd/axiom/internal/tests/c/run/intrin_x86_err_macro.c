/* An intrinsic written as a macro (its operand an immediate) needs its
   extension too: _mm_alignr_epi8 is SSSE3's (C3240 in a baseline build). */
/* flags: --target=x86_64 --c-std=c99 */
/* expect-error: C3240 */
#include <tmmintrin.h>

__m128i f(__m128i a, __m128i b) {
    return _mm_alignr_epi8(a, b, 3);
}

int main(void) {
    return 0;
}
