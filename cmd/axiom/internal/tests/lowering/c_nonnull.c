/* Tests of pointers against null that the attributes nonnull and
   returns_nonnull, and `[static n]`, say cannot fail are folded (ir.F_NONNULL,
   ir.Func.ret_nonnull: combine.ax): of the five tests below, those of q in
   one and of r in plain stay (2); with --c-keep-null-checks all do (5). */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 | \btest\s+%r\w+,%r\w+ | 2
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu17 --c-keep-null-checks | \btest\s+%r\w+,%r\w+ | 5
// lower: --target=aarch64 -O2 --c-freestanding --c-std=gnu17 | \bcmp\s+x\d+, #0x0 | 2
// lower: --target=aarch64 -O2 --c-freestanding --c-std=gnu17 --c-keep-null-checks | \bcmp\s+x\d+, #0x0 | 5
__attribute__((returns_nonnull)) int *get(int);
__attribute__((nonnull(1))) int one(int *p, int *q) {
    if (!p)
        return -1;
    if (!q)
        return -2;
    return *p + *q;
}
int fixed(int a[static 4]) { return a == 0 ? -1 : a[3]; }
int got(int x) { int *p = get(x); return p ? *p : 7; }
int plain(int *r) { return r ? *r : 9; }
