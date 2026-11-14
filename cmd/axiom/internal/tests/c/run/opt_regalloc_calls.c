/* flags: --c-std=gnu17 -O2 */
/* Register allocation around calls (mach.ax): a value live across a call
   only on a seldom-taken path (a tree insertion's allocation) is split
   there rather than given a callee-saved register; a piece of a split value
   reloaded after calls more often than it is read stays in its slot; a
   phi's new value is built in the phi's register (an accumulator's select,
   a flag computed before the old one's last use). Each with the edges:
   empty inputs, one element, odd and even lengths (an unrolled loop's
   remainder), duplicate keys, values changed through a pointer by the
   callee, extreme values. */
#include "host.h"

typedef struct node {
    struct node *l, *r;
    unsigned key;
    long long v;
} node;

static node pool[512];
static int used;
static int grabs;

__attribute__((noinline)) static void *grab(unsigned long long n) {
    (void)n;
    grabs++;
    return &pool[used++];
}

static node *ins(node *t, unsigned key, long long v) {
    if (!t) {
        node *n = grab(sizeof *n);
        n->l = n->r = 0;
        n->key = key;
        n->v = v;
        return n;
    }
    if (key < t->key) t->l = ins(t->l, key, v);
    else if (key > t->key) t->r = ins(t->r, key, v);
    else t->v = v;
    return t;
}

static long long fold(const node *t, long long h) {
    if (!t) return h;
    h = fold(t->l, h);
    h = h * 31 + (long long)t->key * 7 + t->v;
    return fold(t->r, h);
}

static long long calls;
__attribute__((noinline)) static long long tally(long long x) {
    calls++;
    return x ^ (x >> 3);
}

static long long loop_calls(long long n, long long k) {
    long long s = 0;
    for (long long i = 0; i < n; i++) s += tally(i * k);
    return s;
}

/* bad: words differing from the next, and two even words in a row; the
   even words are counted into *nfree */
__attribute__((noinline)) static unsigned long long walk(const unsigned long long *a, unsigned long long n, int *nfree) {
    int bad = 0, nf = 0;
    _Bool prev = 0;
    for (unsigned long long i = 0; i < n; i++) {
        unsigned long long w = a[i];
        if (w != a[i + 1]) bad++;
        _Bool f = !(w & 1);
        if (f && prev) bad++;
        if (f) nf++;
        prev = f;
    }
    *nfree = nf;
    return bad;
}

__attribute__((noinline)) static long long op_a(long long x) { return x * 3 + 1; }
__attribute__((noinline)) static long long op_b(long long x, long long y) { return x - y * 2; }
__attribute__((noinline)) static void op_c(long long *p) { *p += 100; }

static long long mix(const unsigned char *code, long long n) {
    long long a = 0, b = 1, c = 2, d = 3, e = 4, f = 5, g = 6, h = 7, cnt = 0;
    for (long long i = 0; i < n; i++) {
        switch (code[i] & 7) {
        case 0: a = op_a(a + i); break;
        case 1: b = op_b(b, i); cnt++; break;
        case 2: op_c(&cnt); break;
        case 3: c += d; d ^= i; break;
        case 4: e = e * 3 + f; break;
        case 5: f += g; g -= h; h += 1; break;
        case 6: a += b + c; break;
        default: h ^= a; break;
        }
    }
    return a + b + c + d + e + f + g + h + cnt;
}

/* a stack machine: sp and pc live across the calls of some arms, and the
   loop is left from its middle (stores of a split value at the start of
   the blocks an exit edge leads to) */
__attribute__((noinline)) static long long vm_bin(int op, long long a, long long b) { return op ? a - b : a + b; }
static long long vm(const unsigned char *code, int n) {
    long long stack[64];
    int sp = 0, pc = 0;
    for (;;) {
        if (pc >= n) return sp > 0 ? stack[sp - 1] : -1;
        unsigned char c = code[pc++];
        switch (c & 3) {
        case 0: stack[sp++] = c >> 2; break;
        case 1:
            if (sp < 2) return -2 - pc;
            sp--;
            stack[sp - 1] = vm_bin(c & 4, stack[sp - 1], stack[sp]);
            break;
        case 2: if (sp > 0 && stack[sp - 1] == 0) pc += c >> 4; break;
        default: if (sp > 0) stack[sp - 1] = tally(stack[sp - 1]); break;
        }
        if (sp >= 64) return -1000;
    }
}

/* x and y, live across the call only on the arm that seldom runs, are
   stored at that arm's start (not at the end of the block branching to it)
   and reloaded after the call */
__attribute__((noinline)) static long long cold_arm(long long x, long long y) {
    x = x * 3 + y;
    if ((x & 1023) == 7) {
        long long t = tally(y);
        calls += t;
        return x * 2 + y;
    }
    return x - y;
}

int main(void) {
    node *t = 0;
    unsigned x = 12345;
    for (int i = 0; i < 400; i++) {
        x = x * 1103515245u + 12345u;
        t = ins(t, (x >> 16) % 300, (long long)i - 200);
    }
    t = ins(t, 0, -9223372036854775807L - 1);
    t = ins(t, 4294967295u, 9223372036854775807L);
    t = ins(t, 4294967295u, 1);
    print_i64(grabs);
    print_i64(fold(t, 0));
    print_i64(fold(ins(0, 7, 8), 1));

    print_i64(loop_calls(0, 5));
    print_i64(loop_calls(1, 5));
    print_i64(loop_calls(1000, -7));
    print_i64(calls);

    static unsigned long long w[40];
    for (int i = 0; i < 40; i++) w[i] = (unsigned long long)(i * i / 3) | (i % 5 == 0);
    w[39] = ~0ul;
    int nf = -1;
    print_u64(walk(w, 0, &nf));
    print_i64(nf);
    for (unsigned long long n = 1; n <= 5; n++) {
        print_u64(walk(w, n, &nf));
        print_i64(nf);
    }
    print_u64(walk(w, 39, &nf));
    print_i64(nf);

    static unsigned char code[300];
    for (int i = 0; i < 300; i++) code[i] = (unsigned char)((i * 37) ^ (i >> 2));
    print_i64(mix(code, 0));
    print_i64(mix(code, 1));
    print_i64(mix(code, 7));
    print_i64(mix(code, 300));
    static const unsigned char prog[] = { 20, 36, 1, 8, 5, 3, 0, 2, 255, 12, 1, 3 };
    print_i64(vm(prog, 0));
    print_i64(vm(prog, 3));
    print_i64(vm(prog, 12));
    print_i64(vm(prog + 2, 10));
    print_i64(vm(code, 300));
    print_i64(cold_arm(4000, 2055));
    print_i64(cold_arm(2, 1));
    print_i64(cold_arm(-9, 4));
    print_i64(calls);
    return 0;
}
/* expect: 223 */
/* expect: 5043377370865780932 */
/* expect: 88 */
/* expect: 0 */
/* expect: 0 */
/* expect: 3514760 */
/* expect: 1001 */
/* expect: 0 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 2 */
/* expect: 1 */
/* expect: 3 */
/* expect: 1 */
/* expect: 4 */
/* expect: 1 */
/* expect: 5 */
/* expect: 1 */
/* expect: 46 */
/* expect: 15 */
/* expect: 28 */
/* expect: 29 */
/* expect: 154 */
/* expect: -8548828187617436595 */
/* expect: -1 */
/* expect: 14 */
/* expect: 3 */
/* expect: -3 */
/* expect: -4 */
/* expect: 12000 */
/* expect: 15 */
/* expect: -27 */
/* expect: 1006 */
