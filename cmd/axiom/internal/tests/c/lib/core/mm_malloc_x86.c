/* _mm_malloc and _mm_free (<mm_malloc.h>): memory aligned to each power of
   two, which free frees, and _mm_free frees malloc's, as with the other
   compilers' headers (a library that allocates with malloc and frees with
   _mm_free, or the other way round); an alignment that is not a power of
   two gives a null pointer. */
/* native-only */
/* x86-64-only */
/* flags: --c-std=c17 */
#include <xmmintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    int aligned = 1;
    for (size_t al = 1; al <= 4096; al *= 2) {
        char *a = _mm_malloc(1000, al);
        aligned &= a != 0 && ((uintptr_t)a % al) == 0;
        memset(a, 7, 1000);
        free(a);
        char *b = malloc(100);
        memset(b, 7, 100);
        _mm_free(b);
        float *v = _mm_malloc(64 * sizeof(float), 16);
        for (int i = 0; i < 64; i += 4) _mm_store_ps(v + i, _mm_set1_ps((float)i));
        aligned &= v[60] == 60.0f;
        _mm_free(v);
    }
    _mm_free(0);
    printf("%d %d %d\n", aligned, _mm_malloc(10, 3) == 0, _mm_malloc(10, 24) == 0);
    return 0;
}
/* expect: 1 1 1 */
