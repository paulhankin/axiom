/* memcpy (C23 7.26.2.1): copies n bytes from s2 to s1, which do not
   overlap. Words at a time when both are aligned alike. */
#include <string.h>
#include "word.h"

#ifdef __AXIOM_WASM_BULK_MEMORY__
/* WebAssembly with the bulk-memory extension: memory.copy (std/c/rt/string_bulk.ax) */
void *__axc_mem_copy(void *, const void *, size_t);
#endif

void *memcpy(void *restrict s1, const void *restrict s2, size_t n) {
#ifdef __AXIOM_WASM_BULK_MEMORY__
    return __axc_mem_copy(s1, s2, n);
#else
    unsigned char *d = s1;
    const unsigned char *s = s2;
    if (n >= 4 * __AXIOM_WSIZE && (((size_t)d ^ (size_t)s) & (__AXIOM_WSIZE - 1)) == 0) {
        while (!__AXIOM_ALIGNED(d)) {
            *d++ = *s++;
            n--;
        }
        __axiom_word *dw = (__axiom_word *)d;
        const __axiom_word *sw = (const __axiom_word *)s;
        while (n >= 4 * __AXIOM_WSIZE) {
            dw[0] = sw[0];
            dw[1] = sw[1];
            dw[2] = sw[2];
            dw[3] = sw[3];
            dw += 4;
            sw += 4;
            n -= 4 * __AXIOM_WSIZE;
        }
        while (n >= __AXIOM_WSIZE) {
            *dw++ = *sw++;
            n -= __AXIOM_WSIZE;
        }
        d = (unsigned char *)dw;
        s = (const unsigned char *)sw;
    }
    while (n--) {
        *d++ = *s++;
    }
    return s1;
#endif
}
