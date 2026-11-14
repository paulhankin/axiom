/* flags: --c-std=gnu17 */
/* Calls in tail position and the lifetimes of automatic objects whose
   address escaped (cmd/axiom/src/tailrec.ax: mark_frame_dead; ir.ax:
   IN_RSCOPE). At -O2 a call after the end of such an object's block jumps
   to its callee (the object is no longer alive, 6.2.4p6); a call while one
   is alive keeps the caller's frame. Each callee here reads the object
   through a pointer kept in a global after filling a frame of its own: had
   the call become a jump, that frame would lie over the object and the
   result would be wrong. The objects: of the function's body, of a block
   that the call is in, of a block in a loop (each iteration enters it
   again, the pointer stored outside), of a block with a label that a goto
   goes back to, of a for statement's declaration, a compound literal, one
   in a function inlined into the block, one whose bytes another object
   took over (a copy between them), and a variable-length array. */
#include "host.h"

static volatile int vs;
/* PAD keeps a function out of line (too big to inline, and not internal) */
#define PAD vs = 1; vs = 2; vs = 3; vs = 4; vs = 5; vs = 6; vs = 7; vs = 8; vs = 9; vs = 10; \
    vs = 11; vs = 12; vs = 13; vs = 14; vs = 15; vs = 16; vs = 17; vs = 18; vs = 19; vs = 20; \
    vs = 21; vs = 22; vs = 23; vs = 24; vs = 25; vs = 26; vs = 27; vs = 28; vs = 29; vs = 30; \
    vs = 31; vs = 32; vs = 33; vs = 34; vs = 35; vs = 36; vs = 37; vs = 38; vs = 39; vs = 40;

struct cont { int node; const struct cont *next; };
struct big { long v[8]; };

static const struct cont *saved;
static const int *saved_int;
static const struct big *saved_big;

/* a frame of 512 bytes, filled, then the object read */
int peek(int k) { PAD
    volatile unsigned char junk[512];
    for (int i = 0; i < 512; i++) junk[i] = (unsigned char)(0x5a + i);
    return saved->node + k + junk[k & 7] * 0;
}
int peek_int(int k) { PAD
    volatile unsigned char junk[512];
    for (int i = 0; i < 512; i++) junk[i] = (unsigned char)(0xa5 + i);
    return saved_int[k] + junk[k & 7] * 0;
}
long peek_big(int k) { PAD
    volatile unsigned char junk[512];
    for (int i = 0; i < 512; i++) junk[i] = (unsigned char)(0x3c + i);
    return saved_big->v[k & 7] + junk[k & 7] * 0;
}

/* the body's object: alive at the call */
int body_object(int n) { PAD
    struct cont c = { n * 3, 0 };
    saved = &c;
    return peek(1);
}

/* a block's object, the call in the block */
int block_object(int n) { PAD
    if (n > 0) {
        struct cont c = { n + 100, 0 };
        saved = &c;
        return peek(2);
    }
    return -1;
}

/* a block in a loop, entered again in each iteration */
int loop_object(int n) { PAD
    for (int i = 0;; i++) {
        struct cont c = { i * 10, 0 };
        saved = &c;
        if (i == n) return peek(3);
    }
}

/* a goto back into a block (which has no scope of its own then) */
int goto_object(int n) { PAD
    int k = 0;
    {
        struct cont c = { 7, 0 };
    again:
        saved = &c;
        c.node += k;
        if (++k < n) goto again;
        return peek(4);
    }
}

/* a for statement's object, the call in the loop */
int for_object(int n) { PAD
    for (struct cont c = { n, 0 }; c.node > 0; c.node--) {
        saved = &c;
        if (c.node == 3) return peek(5);
    }
    return -1;
}

/* a compound literal of the body */
int literal_object(int n) { PAD
    int m = n * 5;
    saved = &(struct cont){ m, 0 };
    return peek(6);
}

/* a function with scopes of its own, inlined into the block whose object
   it reads */
static inline int inlined(int n) {
    int t[4] = { n, n + 1, n + 2, n + 3 };
    vs = t[n & 3];
    return peek(t[1] & 1);
}
int inlined_object(int n) { PAD
    {
        struct cont c = { n * 7, 0 };
        saved = &c;
        return inlined(n);
    }
}

/* the object of a block whose bytes another object takes over (a copy of
   the whole of it into one not used before, after which it is not used) */
long copied_object(int n) { PAD
    struct big a;
    {
        struct big t;
        for (int i = 0; i < 8; i++) t.v[(i * 3) & 7] = n * 10 + ((i * 3) & 7);
        __builtin_memcpy(&a, &t, sizeof a);
    }
    saved_big = &a;
    return peek_big(3);
}

/* a variable-length array */
int vla_object(int n) { PAD
    int a[n];
    for (int i = 0; i < n; i++) a[i] = i * i;
    saved_int = a;
    return peek_int(n - 1);
}

/* the object's block has ended: the call may jump (and the result is the
   callee's alone) */
static long steps, leaves;
int visit(int n, const struct cont *k);
int star(int n, const struct cont *k) { PAD
    steps++;
    if (n > 1) {
        struct cont again = { n - 2, k };
        if (visit(n - 1, &again)) return 1;
    }
    if (!k) return ++leaves >= 40;
    return visit(k->node, k->next);
}
int visit(int n, const struct cont *k) { PAD
    steps++;
    return star(n, k);
}

int main(void) {
    print_i64(body_object(4));
    print_i64(block_object(5));
    print_i64(loop_object(6));
    print_i64(goto_object(4));
    print_i64(for_object(9));
    print_i64(literal_object(3));
    print_i64(inlined_object(2));
    print_i64(copied_object(4));
    print_i64(vla_object(9));
    print_i64(star(12, 0));
    print_i64(steps);
    print_i64(leaves);
    return 0;
}

/* expect: 13 */
/* expect: 107 */
/* expect: 63 */
/* expect: 17 */
/* expect: 8 */
/* expect: 21 */
/* expect: 15 */
/* expect: 43 */
/* expect: 64 */
/* expect: 1 */
/* expect: 1117 */
/* expect: 40 */
