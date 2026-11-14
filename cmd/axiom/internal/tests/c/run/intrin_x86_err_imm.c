/* An immediate operand must be a constant (C3241): the instruction encodes it. */
/* flags: --target=x86_64 --features=+ssse3 */
/* expect-error: C3241 */
#include <tmmintrin.h>

__m128i f(__m128i a, __m128i b, int n) {
    return _mm_alignr_epi8(a, b, n);
}

int main(void) {
    return 0;
}
