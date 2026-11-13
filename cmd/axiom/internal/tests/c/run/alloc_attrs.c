/* flags: --c-std=gnu17 -O2 */
/* What the attributes malloc and alloc_size, and variable length arrays,
   tell the optimizer (gvn.ax's fresh objects, ir.IN_OBJSIZE): a fresh
   allocation is apart from what the function was given, but not from a
   pointer read back from memory that holds its address; the object sizes
   of what an allocating function returned, at constant and at run-time
   sizes, past the end, through a function that hides the object; a
   variable length array's dynamic size. (At -O2: what -O0 does not find
   is the unknown answer.) */
#include "host.h"
#include <stddef.h>

__attribute__((malloc, alloc_size(1))) void *take(size_t n);
__attribute__((alloc_size(1, 2))) void *take2(size_t n, size_t m);
static _Alignas(16) char heap[4096];
static size_t used;
char gbuf[50];
void *take(size_t n) {
    void *p = heap + used;
    used += (n + 15) & ~(size_t)15;
    return p;
}
void *take2(size_t n, size_t m) { return take(n * m); }

static volatile size_t v40 = 40;
static int *volatile hold;

__attribute__((noinline)) static int apart(int *q) {
    int *p = take(16);
    *q = 1;
    p[0] = 2;
    return *q;  /* 1: p is a new object */
}

__attribute__((noinline)) static int read_back(void) {
    int *p = take(16);
    p[0] = 1;
    hold = p;
    int *r = hold;  /* p's value: based on it */
    *r = 5;
    return p[0];
}

__attribute__((noinline)) static size_t hidden(void *p) { return __builtin_object_size(p, 0); }

int main(void) {
    int x = 0;
    print_i64(apart(&x));
    print_i64(x);
    print_i64(read_back());
    char *a = take(100);
    print_u64(__builtin_object_size(a, 0));
    print_u64(__builtin_object_size(a + 10, 0));
    print_u64(__builtin_object_size(a + 10, 2));
    print_u64(__builtin_object_size(a + 10, 3));   /* (a subobject's lower bound: unknown) */
    print_u64(__builtin_object_size(a + 200, 0));  /* past the end */
    size_t n = v40;
    char *b = take(n);
    print_u64(__builtin_object_size(b, 0) == (size_t)-1);  /* (static form: not a constant) */
    print_u64(__builtin_object_size(b, 2));
    print_u64(__builtin_dynamic_object_size(b + 5, 0));
    print_u64(__builtin_dynamic_object_size(b + 50, 0));
    int *c = take2(n, sizeof(int));
    print_u64(__builtin_dynamic_object_size(c, 1));
    char *d = take2(3, 5);
    print_u64(__builtin_object_size(d, 0));
    print_u64(__builtin_object_size(take2(3, 5), 0) == (size_t)-1);  /* (a call in it: not looked into) */
    char vla[n + 1];
    vla[0] = 0;
    print_u64(__builtin_dynamic_object_size(vla, 0));
    print_u64(__builtin_dynamic_object_size(&vla[1], 2));
    print_u64(hidden(a) == (size_t)-1);
    char *g = gbuf;  /* (followed: the global's bytes from 3) */
    print_u64(__builtin_object_size(g + 3, 0));
    return vla[0];
}
/* expect: 1 */
/* expect: 1 */
/* expect: 5 */
/* expect: 100 */
/* expect: 90 */
/* expect: 90 */
/* expect: 0 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 35 */
/* expect: 0 */
/* expect: 160 */
/* expect: 15 */
/* expect: 1 */
/* expect: 41 */
/* expect: 40 */
/* expect: 1 */
/* expect: 47 */
