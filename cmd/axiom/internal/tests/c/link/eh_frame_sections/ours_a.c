// A 16-byte atomic (the link-once helper, in a COMDAT group) and a call
// back to the walk, in sections of their own (--function-sections).
#include <stdatomic.h>

typedef struct {
    long a, b;
} pair;

static _Atomic pair P;
void sys_c(int *, void *, void *);

int ours_a(int *seen) {
    pair e = {0, 0}, d = {1, 2};
    int r = atomic_compare_exchange_strong(&P, &e, d);
    sys_c(seen, (void *)ours_a, 0);
    return r + seen[2];
}

// (nothing refers to it: --gc-sections leaves it out)
int ours_a_unused(int x) {
    return x * 3 + 1;
}
