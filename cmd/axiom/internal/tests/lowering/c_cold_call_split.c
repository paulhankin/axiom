/* A value live across a call only where the call seldom runs (key and v,
   across the allocation at a tree's leaf) is split around that call, in
   memory across it, rather than taking a callee-saved register that every
   level of the recursion would save and restore (mach.ax save_dearer); one
   live across a call in a loop (s, i, n and k) keeps a callee-saved
   register. */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \bpush\s+%(rbx|r1[2-5])\b | 4
typedef struct node { struct node *l, *r; unsigned key; long v; } node;
void *grab(unsigned long n);
long tally(long x);

node *ins(node *t, unsigned key, long v) {
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

long loop_calls(long n, long k) {
    long s = 0;
    for (long i = 0; i < n; i++) s += tally(i * k);
    return s;
}
