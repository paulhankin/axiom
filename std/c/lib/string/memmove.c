/* memmove (C23 7.26.2.3): copies n bytes from s2 to s1, which may overlap:
   forward when s1 is below s2, else backward. Words at a time when both are
   aligned alike. (The addresses are compared as integers: the objects may
   be different ones, which a relational operator does not allow.) */
#include <string.h>
#include "word.h"

#ifdef __AXIOM_WASM_BULK_MEMORY__
/* WebAssembly with the bulk-memory extension: memory.copy (std/c/rt/string_bulk.ax) */
void *__axc_mem_copy(void *, const void *, size_t);
#endif

void *memmove(void *s1, const void *s2, size_t n) {
#ifdef __AXIOM_WASM_BULK_MEMORY__
    return __axc_mem_copy(s1, s2, n);
#else
    unsigned char *d = s1;
    const unsigned char *s = s2;
    int words = n >= 2 * __AXIOM_WSIZE && (((size_t)d ^ (size_t)s) & (__AXIOM_WSIZE - 1)) == 0;
    if ((size_t)d == (size_t)s || n == 0) {
        return s1;
    }
    if ((size_t)d < (size_t)s) {
        if (words) {
            while (!__AXIOM_ALIGNED(d)) {
                *d++ = *s++;
                n--;
            }
            while (n >= __AXIOM_WSIZE) {
                *(__axiom_word *)d = *(const __axiom_word *)s;
                d += __AXIOM_WSIZE;
                s += __AXIOM_WSIZE;
                n -= __AXIOM_WSIZE;
            }
        }
        while (n--) {
            *d++ = *s++;
        }
        return s1;
    }
    d += n;
    s += n;
    if (words) {
        while (!__AXIOM_ALIGNED(d)) {
            *--d = *--s;
            n--;
        }
        while (n >= __AXIOM_WSIZE) {
            d -= __AXIOM_WSIZE;
            s -= __AXIOM_WSIZE;
            *(__axiom_word *)d = *(const __axiom_word *)s;
            n -= __AXIOM_WSIZE;
        }
    }
    while (n--) {
        *--d = *--s;
    }
    return s1;
#endif
}
