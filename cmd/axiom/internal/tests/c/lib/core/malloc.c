/* malloc, calloc, realloc, free, aligned_alloc (C17 7.22.3) and the
   library's choices (<stdlib.h>): a size of 0 is a unique object; memory is
   aligned to 16; calloc zeroes and checks its product for overflow;
   realloc keeps the contents; aligned_alloc takes any power of two; a
   failure is a null pointer with errno ENOMEM (EINVAL for an alignment). */
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include "print.h"

static void fill(char *p, int c, size_t n) {
    while (n-- > 0)
        *p++ = (char)c;
}

static int aligned(void *p, size_t a) {
    return ((uintptr_t)p & (a - 1)) == 0;
}

int main(void) {
    char *p, *q, *r;
    size_t i, a;
    int ok = 1;
    p = malloc(0);
    q = malloc(0);
    print_bool(p != NULL && q != NULL && p != q); /* expect: true */
    free(p);
    free(q);
    free(NULL);
    for (i = 1; i < 5000; i += 37) {
        p = malloc(i);
        ok &= p != NULL && aligned(p, 16);
        fill(p, 0x5A, i);
        free(p);
    }
    print_bool(ok); /* expect: true */
    p = calloc(1000, 7);
    for (i = 0; i < 7000; i++)
        ok &= p[i] == 0;
    free(p);
    print_bool(ok); /* expect: true */
    errno = 0;
    p = calloc(SIZE_MAX / 2, 3);
    print_bool(p == NULL && errno == ENOMEM); /* expect: true */
    errno = 0;
    p = malloc(SIZE_MAX - 4);
    print_bool(p == NULL && errno == ENOMEM); /* expect: true */
    p = calloc(0, 5);
    print_bool(p != NULL); /* expect: true */
    free(p);
    /* realloc: grows and shrinks keeping the contents; from NULL is malloc */
    p = realloc(NULL, 10);
    for (i = 0; i < 10; i++)
        p[i] = (char)i;
    p = realloc(p, 100000);
    for (i = 0; i < 10; i++)
        ok &= p[i] == (char)i;
    for (i = 0; i < 100000; i++)
        p[i] = (char)(i * 3);
    p = realloc(p, 20);
    for (i = 0; i < 20; i++)
        ok &= p[i] == (char)(i * 3);
    q = realloc(p, 0);
    print_bool(ok && q != NULL); /* expect: true */
    free(q);
    errno = 0;
    p = malloc(8);
    q = realloc(p, SIZE_MAX - 4);
    print_bool(q == NULL && errno == ENOMEM); /* expect: true */
    free(p);
    /* aligned_alloc */
    for (a = 1; a <= 65536; a *= 2) {
        p = aligned_alloc(a, 3 * a + 1);
        ok &= p != NULL && aligned(p, a);
        fill(p, 1, 3 * a + 1);
        r = realloc(p, 5 * a);
        ok &= r != NULL && r[3 * a] == 1;
        free(r);
        p = aligned_alloc(a, 0);
        ok &= p != NULL;
        free(p);
    }
    print_bool(ok); /* expect: true */
    errno = 0;
    p = aligned_alloc(24, 48);
    print_bool(p == NULL && errno == EINVAL); /* expect: true */
    errno = 0;
    print_bool(aligned_alloc(0, 8) == NULL && errno == EINVAL); /* expect: true */
    return 0;
}
