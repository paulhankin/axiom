// The same link-once helper as ours_a.c's (another COMDAT copy, which the
// link leaves out), and a frame between the walk and main.
#include <stdatomic.h>

typedef struct {
    long a, b;
} pair;

static _Atomic pair Q;
void sys_c(int *, void *, void *);
int ours_b(int *);

static int ours_b_inner(int *seen, int n) {
    pair e = {0, 0}, d = {3, 4};
    int r = atomic_compare_exchange_strong(&Q, &e, d);
    sys_c(seen, (void *)ours_b_inner, (void *)ours_b);
    return r + n + seen[2];
}

int ours_b(int *seen) {
    int (*volatile f)(int *, int) = ours_b_inner;
    int r = f(seen, 0);
    seen[2] = r;  // (after the call: not a sibling call)
    return r;
}
