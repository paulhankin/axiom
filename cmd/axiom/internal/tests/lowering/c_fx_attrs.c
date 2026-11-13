/* Calls of functions that promise to write nothing (ir.call_fx): the same
   call twice is one (const, pure across a load, C23's unsequenced and
   reproducible), an unused one is none, the calls in a loop that writes
   nothing they read are made once before it (and the loop is then a vector
   loop); but a pure call after a store to a global is made again, and one in
   a loop that stores through a pointer stays in the loop. 9 calls in all
   (15 without the attributes, and no vector add). */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu23 | \bcall\b | 9
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu23 | \bpaddd\b | 1
// lower: --target=aarch64 -O2 --c-freestanding --c-std=gnu23 | \bbl\b | 9
int g;
__attribute__((const, noinline)) static int sq(int x) { return x * x; }
__attribute__((pure, noinline)) static int rd(int x) { return x + g; }
__attribute__((noinline)) static int sq23(int x) [[unsequenced]] { return x * 3; }
__attribute__((noinline)) static int rep23(int x) [[reproducible]] { return x + g; }
int cse_const(int a) { return sq(a) + sq(a); }                       /* 1 */
int cse_pure(int *p, int a) { return rd(a) + *p + rd(a); }           /* 1 */
int pure_store(int a) { int x = rd(a); g = 1; return x + rd(a); }    /* 2 */
int unused(int a) { sq(a); rd(a); return a; }                        /* none */
void hoist(int *restrict p, int n, int a) {                          /* 2, before the loop */
    for (int i = 0; i < n; i++)
        p[i] += sq(a) + rd(a);  /* (then a vector loop: paddd) */
}
int stays(int *p, int n, int a) {                                    /* 1, in the loop */
    for (int i = 0; i < n; i++)
        p[i] = rd(a);
    return n;
}
int c23(int a) { return sq23(a) + sq23(a) + rep23(a) + rep23(a); }   /* 2 */
