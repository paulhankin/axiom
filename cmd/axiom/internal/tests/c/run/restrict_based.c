/* restrict on block-scope pointers against accesses through anything else
   (6.7.3.1p4): during an execution of the block, an object written through
   a restricted pointer is reached only through pointers based on it. Here
   every access that reaches such an object is based on the restricted
   pointer (through arithmetic, casts, a conditional, a loop's phi, memory,
   an integer, a call, a returned value, an inner block's restricted
   pointer), or it is not in the block's execution (after the block, in
   another iteration, in a block with a label), or nothing writes the
   object: the optimizer must not take any of them for apart. */
#include "host.h"

#include <stdint.h>
typedef uintptr_t uptr;

int *volatile g_ptr;
int g_obj;

static int arith(int *a, int *b) {
    int *restrict p = a;
    int *q = p + 1;
    p[1] = 1;
    q[0] = 2;
    return p[1] * 10 + *b;  /* (b is a + 2, which only b reads) */
}

static int casts(int *a) {
    int *restrict p = a;
    unsigned char *c = (unsigned char *)p;
    *p = 0;
    c[0] = 7;
    return *p & 0xff;
}

static int cond(int *a, int k) {
    int *restrict p = a;
    int *q = k ? p : p + 1;
    p[0] = 1;
    p[1] = 1;
    *q = 9;
    return p[0] + p[1];
}

static int walk(int *a, int n) {
    int *restrict p = a;
    int *q = p;
    int s = 0;
    for (int i = 0; i < n; i++) {
        p[i] = i;
        *q += 1;
        q++;
    }
    for (int i = 0; i < n; i++)
        s += p[i];
    return s;
}

static int memory(int *a) {
    int *box[2];
    int *restrict p = a;
    box[1] = p;
    int *volatile *vb = box;
    int *q = vb[1];
    *p = 1;
    *q = 2;
    return *p;
}

static int integer(int *a) {
    int *restrict p = a;
    uptr i = (uptr)p;
    int *q = (int *)(i + sizeof(int));
    p[1] = 1;
    *q = 4;
    return p[1];
}

static int *pass(int *x) { return x + 1; }
static int *id(int *x) {
    g_ptr = x;
    return g_ptr;
}

static int calls(int *a) {
    int *restrict p = a;
    int *q = pass(p);
    int *r = id(p);
    p[1] = 1;
    *q += 5;
    *r += 10;
    return p[1] * 100 + p[0];
}

static int *returned(int *a) {
    int *restrict p = a;
    *p = 1;
    return p + 1;
}

static int ret_after(int *a) {
    int *q = returned(a);
    q[-1] = 6;
    a[1] = 0;
    return a[0] + q[0];
}

static int stored(int *a) {
    int *c = 0;
    {
        int *restrict p = a;
        c = p;
        *p = 2;
    }
    *a = 3;
    *c += 4;
    return *a;
}

static int readonly(const int *a) {
    const int *restrict p = a;
    const int *restrict q = a;
    const int *r = a;
    return *p + *q + *r;
}

static int inner(int *a) {
    int *restrict p = a;
    p[0] = 1;
    {
        int *restrict q = p;
        q[0] = 2;
        {
            int *restrict r = q + 0;
            r[0] += 3;
        }
        q[0] += 4;
    }
    return p[0];
}

static int reassigned(int *a, int *b) {
    int *restrict p = a;
    int *q = p;
    p = b;  /* (b == a) */
    *p = 5;
    *q += 1;
    return *p;
}

static int address(int *a) {
    int *restrict p = a;
    int *restrict *pp = &p;
    int *q = p;
    *pp = a;  /* (p's value given through its address) */
    *p = 1;
    *q = 8;
    return *p;
}

/* a block in a loop: what its plain pointer reads is written through its
   restricted pointer only in other iterations (other executions of it) */
static int cycle(int *a, int *b, int n) {
    int t = *b;
    for (int i = 0; i < n; i++) {
        int *restrict p = a + (i & 1);
        if (i & 1)
            t += *b;
        else
            *p = i + 5;
    }
    return t;
}

/* an inlined function's restrict parameters, returned and stored: the
   caller's accesses after the call are outside its execution */
static int *pick(int *restrict x, int *restrict y, int **oy) {
    *x = 1;
    *oy = y;
    return x;
}

static int inl_escape(int *a) {
    int *qy;
    int *qx = pick(a, a + 1, &qy) + 1;
    *qx = 2;
    *qy = 3;
    return *qx;
}

static int iterations(int *a, int n) {
    int s = 0;
    for (int i = 0; i < n; i++) {
        {
            int *restrict p = a;
            *p += 1;
        }
        s += *a;  /* (after the block, in this iteration) */
        {
            int *restrict q = g_ptr;
            s += *q;
        }
    }
    return s;
}

static int global(void) {
    g_obj = 0;
    {
        int *restrict p = &g_obj;
        *p += 1;
    }
    g_obj += 1;
    return g_obj;
}

static int labelled(int *a, int k) {
    int *restrict p = a;
    if (k)
        goto out;
    *p = 1;
out:
    *a = 2;
    return *p;
}

static int again(int *a, int *b) {
    int n = 0;
    int t = 0;
top:
    {
        int *restrict p = a;
        t += *p;
        *p = n + 10;
    }
    *b = n + 20;  /* (b == a: after the block, before the next execution) */
    if (++n < 3)
        goto top;
    return t + *a;
}

static int member_addr(int *a) {
    int *q1, *q2;
    {
        struct rp { int *restrict m; int *restrict n; } s = { a, a };
        q1 = &s.m[0];
        q2 = &s.n[0];
    }
    *q1 = 1;  /* (after the block of the structure type: m and n alias) */
    *q2 = 2;
    return *q1;
}

/* restrict-qualified members of a structure type declared in a loop's
   body: each iteration is another execution of the block */
static int member_loop(int *a, int n) {
    for (int i = 0; i + 1 < n; i++) {
        struct { int *restrict m; int *restrict k; } s = { a, a };
        s.m[i + 1] = s.k[i] + 1;
    }
    return a[n - 1];
}

static void put(int *d, int v) { *d = v; }

static int helper(int *a) {
    int *restrict p = a;
    *p = 1;
    put(p, 2);
    return *p;
}

int main(void) {
    int a[8] = {0};
    a[2] = 3;
    print_i64(arith(a, a + 2));
    print_i64(casts(a));
    print_i64(cond(a, 1) * 10 + cond(a, 0));
    print_i64(walk(a, 5));
    print_i64(memory(a));
    print_i64(integer(a));
    a[0] = 3;
    print_i64(calls(a));
    print_i64(ret_after(a));
    print_i64(stored(a));
    a[0] = 5;
    print_i64(readonly(a));
    print_i64(inner(a));
    print_i64(reassigned(a, a));
    print_i64(address(a));
    a[0] = 0;
    print_i64(cycle(a, a, 4));
    print_i64(inl_escape(a));
    a[0] = 0;
    g_ptr = a;
    print_i64(iterations(a, 3));
    print_i64(global());
    print_i64(labelled(a, 0) * 10 + labelled(a, 1));
    a[0] = 0;
    print_i64(again(a, a));
    print_i64(member_addr(a));
    for (int i = 0; i < 8; i++)
        a[i] = 0;
    print_i64(member_loop(a, 8));
    print_i64(helper(a));
    return 0;
}
/* expect: 23 */
/* expect: 7 */
/* expect: 110 */
/* expect: 15 */
/* expect: 2 */
/* expect: 4 */
/* expect: 613 */
/* expect: 6 */
/* expect: 7 */
/* expect: 15 */
/* expect: 9 */
/* expect: 6 */
/* expect: 8 */
/* expect: 12 */
/* expect: 3 */
/* expect: 12 */
/* expect: 2 */
/* expect: 22 */
/* expect: 63 */
/* expect: 2 */
/* expect: 7 */
/* expect: 2 */
