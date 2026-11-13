/* memset (C23 7.26.6.1): sets n bytes at s to (unsigned char)c. Words at a
   time past the first aligned address. */
#include <string.h>
#include "word.h"

#ifdef __AXIOM_WASM_BULK_MEMORY__
/* WebAssembly with the bulk-memory extension: memory.fill (std/c/rt/string_bulk.ax) */
void *__axc_mem_set(void *, unsigned char, size_t);
#endif

void *memset(void *s, int c, size_t n) {
#ifdef __AXIOM_WASM_BULK_MEMORY__
    return __axc_mem_set(s, (unsigned char)c, n);
#else
    unsigned char *d = s;
    unsigned char b = (unsigned char)c;
    if (n >= 4 * __AXIOM_WSIZE) {
        __axiom_word w = __AXIOM_ONES * b;
        while (!__AXIOM_ALIGNED(d)) {
            *d++ = b;
            n--;
        }
        __axiom_word *dw = (__axiom_word *)d;
        while (n >= 4 * __AXIOM_WSIZE) {
            dw[0] = w;
            dw[1] = w;
            dw[2] = w;
            dw[3] = w;
            dw += 4;
            n -= 4 * __AXIOM_WSIZE;
        }
        while (n >= __AXIOM_WSIZE) {
            *dw++ = w;
            n -= __AXIOM_WSIZE;
        }
        d = (unsigned char *)dw;
    }
    while (n--) {
        *d++ = b;
    }
    return s;
#endif
}
