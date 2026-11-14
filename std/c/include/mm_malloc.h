/* <mm_malloc.h> - _mm_malloc and _mm_free: memory aligned to a power of two
   (the compiler's: docs/c.md, "Intrinsics"), from the C library's malloc
   and free. The block's own address is kept just before the aligned one. */
#ifndef __AXIOM_MM_MALLOC_H
#define __AXIOM_MM_MALLOC_H
#include <stddef.h>

void *malloc(size_t __axiom_n);
void free(void *__axiom_p);

static __axiom_attribute__((__unused__)) void *_mm_malloc(size_t __axiom_n, size_t __axiom_al) {
    char *__axiom_p;
    char *__axiom_q;
    if ((__axiom_al & (__axiom_al - 1)) != 0) {
        return 0;
    }
    if (__axiom_al < sizeof(void *)) {
        __axiom_al = sizeof(void *);
    }
    if (__axiom_n > (size_t)-1 - __axiom_al - sizeof(void *)) {
        return 0;
    }
    __axiom_p = (char *)malloc(__axiom_n + __axiom_al + sizeof(void *));
    if (__axiom_p == 0) {
        return 0;
    }
    __axiom_q = __axiom_p + sizeof(void *);
    __axiom_q += (__axiom_al - ((size_t)__axiom_q & (__axiom_al - 1))) & (__axiom_al - 1);
    ((void **)__axiom_q)[-1] = __axiom_p;
    return __axiom_q;
}

static __axiom_attribute__((__unused__)) void _mm_free(void *__axiom_p) {
    if (__axiom_p != 0) {
        free(((void **)__axiom_p)[-1]);
    }
}
#endif
