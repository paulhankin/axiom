/* flags: --c-std=gnu17 */
/* A local structure built and then copied whole (`*r = t;`) is built in
   the copy's destination instead (copies.forward_into), at -O2, where
   nothing can tell: here both the cases where it is and the ones where it
   must not be, which would print other values if it were. */
#include "host.h"

typedef struct { int n; unsigned d[100]; } big;

static big G, H;

static unsigned sum(const big *a) {
    unsigned s = (unsigned)a->n * 7u;
    for (int i = 0; i < 100; i++) s = s * 31u + a->d[i];
    return s;
}

/* built from a, a global apart from the destination: forwarded once
   inlined into callers that copy into another global */
static void mul2(big *r, const big *a) {
    big t;
    for (int i = 0; i < 100; i++) t.d[i] = 0;
    for (int i = 0; i < a->n; i++) t.d[i] = a->d[i] * 2u + 1u;
    t.n = a->n;
    while (t.n > 0 && t.d[t.n - 1] == 1) t.n--;
    *r = t;
}

/* the destination read while t is built: not forwarded (r and a may be one) */
static void reverse(big *r, const big *a) {
    big t;
    for (int i = 0; i < 100; i++) t.d[i] = a->d[99 - i];
    t.n = a->n;
    *r = t;
}

/* t used after the copy */
static unsigned after(big *r, int k) {
    big t;
    for (int i = 0; i < 100; i++) t.d[i] = (unsigned)(i + k);
    t.n = k;
    *r = t;
    t.d[0] = 1000;
    return t.d[0] + r->d[0];
}

/* a way out that does not copy: the destination is not written on it */
static void maybe(big *r, int write) {
    big t;
    for (int i = 0; i < 100; i++) t.d[i] = 5000u + (unsigned)i;
    t.n = 9;
    if (!write) return;
    *r = t;
}

/* a call between: it reads the destination */
static unsigned seen;
static void peek(void) { seen += G.d[3]; }
static void (*volatile peekp)(void) = peek;  /* (not inlined) */
static void with_call(big *r) {
    big t;
    for (int i = 0; i < 100; i++) t.d[i] = 77u;
    t.n = 1;
    peekp();
    *r = t;
}

/* the copy in a loop, t read before it in the next round */
static unsigned in_loop(big *r) {
    big t;
    unsigned s = 0;
    t.n = 0;
    for (int i = 0; i < 100; i++) t.d[i] = 0;
    for (int k = 0; k < 3; k++) {
        s += t.d[k];
        for (int i = 0; i < 100; i++) t.d[i] = (unsigned)(k + 1) * 10u;
        r->d[k] = 99;
        *r = t;
    }
    return s;
}

/* a compound literal copied into the destination */
static void clear(big *r) { *r = (big){ 3 }; }

/* only part of t copied */
static void part(big *r) {
    big t;
    for (int i = 0; i < 100; i++) t.d[i] = 4u;
    t.n = 2;
    __builtin_memcpy(r, &t, sizeof(int) + 10 * sizeof(unsigned));
}

/* t copied from the destination itself, shifted by one element */
static void shift_up(big *r) {
    big t;
    __builtin_memcpy(&t, r, sizeof t);
    __builtin_memcpy(&t.d[1], &r->d[0], 99 * sizeof(unsigned));
    t.n = 5;
    *r = t;
}

int main(void) {
    for (int i = 0; i < 100; i++) G.d[i] = (unsigned)i * 3u;
    G.n = 60;
    mul2(&H, &G);
    print_u64(sum(&H));
    mul2(&G, &G);  /* the source is the destination */
    print_u64(sum(&G));
    reverse(&G, &G);
    print_u64(sum(&G));
    print_u64(after(&H, 4));
    H.d[0] = 1;
    maybe(&H, 0);
    print_u64(H.d[0]);
    maybe(&H, 1);
    print_u64(H.d[0] + (unsigned)H.n);
    G.d[3] = 11;
    with_call(&G);
    print_u64(seen * 1000u + G.d[3]);
    print_u64(in_loop(&H) * 1000u + H.d[0] + H.d[2]);
    clear(&H);
    print_u64(sum(&H));
    for (int i = 0; i < 100; i++) H.d[i] = 8u;
    part(&H);
    print_u64(H.d[9] * 10u + H.d[10] + (unsigned)H.n);
    for (int i = 0; i < 100; i++) H.d[i] = (unsigned)i;
    shift_up(&H);
    print_u64(sum(&H));
    return 0;
}

/* expect: 3814461464 */
/* expect: 3814461464 */
/* expect: 3187081776 */
/* expect: 1004 */
/* expect: 1 */
/* expect: 5009 */
/* expect: 11077 */
/* expect: 30060 */
/* expect: 1307390357 */
/* expect: 50 */
/* expect: 2134702324 */
