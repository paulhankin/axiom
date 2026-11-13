/* flags: --c-std=gnu23 */
/* What the optimizer may and may not do with the calls of functions
   declared const, pure, [[unsequenced]] or [[reproducible]] (ir.Func.fx,
   ir.call_fx; gvn.ax): a pure call is made again after a store that may
   change what it reads (a global, a local array whose address it was
   given), and is not moved out of a loop that writes what it reads; a call
   in a loop's body is not made before the loop when the body does not run
   (its argument is a null pointer then), nor a call on a path some
   iterations do not take, nor one given a pointer based on a restrict
   parameter the loop writes through; C23's attributes on a function
   with a pointer parameter are not acted on (it may modify what that
   points to); a structure result, returned in the caller's memory, and one
   passed in memory, are not taken for values the call computes. */
#include "host.h"

int g = 1;
/* (the arguments, from volatile objects: no call is specialized for them) */
static volatile int v0 = 0, v3 = 3, v4 = 4, v7 = 7;
static const int *volatile null_p;
__attribute__((pure, noinline)) static int read_g(int k) { return g * k; }
__attribute__((pure, noinline)) static int read_p(const int *p) { return *p + 1; }
__attribute__((const, noinline)) static int sq(int x) { return x * x; }
__attribute__((noinline)) static int bump(int *p) [[reproducible]] {
    *p += 1;  /* (allowed: an object reached through a pointer parameter) */
    return *p;
}
__attribute__((noinline)) static int twice(int x) [[unsequenced]] { return 2 * x; }
__attribute__((noinline)) static int same(int x) [[reproducible]] { return x + g; }

__attribute__((noinline)) static int after_store(int k) {
    int a = read_g(k);
    g = 5;
    return a + read_g(k);  /* 1*k + 5*k */
}

__attribute__((noinline)) static int local_array(int v) {
    int a[2] = { v, 0 };
    int x = read_p(a);
    a[0] = v + 10;
    return x * 100 + read_p(a);
}

__attribute__((noinline)) static int loop_writes(int n) {
    int s = 0;
    for (int i = 0; i < n; i++) {
        s += read_g(1);
        g = g + 1;
    }
    return s;
}

__attribute__((noinline)) static int guarded(const int *p, int n) {
    int s = 0;
    for (int i = 0; i < n; i++)
        s += read_p(p);  /* p is null when n is 0 */
    return s;
}

__attribute__((noinline)) static int in_branch(const int *p, int n) {
    int s = 0;
    for (int i = 0; i < n; i++) {
        if (i == 5)
            s += read_p(p);  /* never reached for n <= 5 (p is null) */
        s += i;
    }
    return s;
}

__attribute__((noinline)) static int restrict_given(int *restrict p, int n) {
    int s = 0;
    for (int i = 0; i < n; i++) {
        p[i & 1] = i;
        s += read_p(p);  /* given p: it reads what the loop writes */
    }
    return s;
}

__attribute__((noinline)) static int restrict_apart(int *restrict p, int n) {
    int s = 0;
    for (int i = 0; i < n; i++) {
        p[i] = i;
        s += read_g(2);  /* g is not *p: may be made once */
    }
    return s + p[n - 1];
}

__attribute__((noinline)) static int c23_pointer(void) {
    int x = 0;
    int a = bump(&x);
    int b = bump(&x);
    return a * 10 + b;  /* 12 */
}

__attribute__((noinline)) static int c23_values(int v) {
    int a = twice(v) + twice(v);
    int b = same(v);
    g = 100;
    return a * 1000 + b + same(v);  /* g changed: same is called again */
}

struct big { long long a, b, c, d; };
__attribute__((pure, noinline)) static struct big make(int k) {
    struct big r = { k, 2 * k + g, 3, 4 };
    return r;  /* (returned in the caller's memory) */
}
__attribute__((const, noinline)) static long long first(struct big b) { return b.a + b.d; }  /* (passed in memory) */

__attribute__((noinline)) static long long aggregates(int k) {
    struct big r = make(k);
    struct big q = r;
    long long x = first(q);
    q.a = 100;
    return r.b * 10000 + x * 100 + first(q) + (make(k).a + make(k + 1).a * 10) * 100000000;
}

int main(void) {
    print_i64(after_store(v3));
    g = 1;
    print_i64(local_array(v4));
    g = 1;
    print_i64(loop_writes(v4));
    print_i64(g);
    print_i64(guarded(null_p, v0));
    int v = 6;
    print_i64(guarded(&v, v3));
    print_i64(in_branch(null_p, v4));
    int arr[4] = { 0, 0, 0, 0 };
    print_i64(restrict_given(arr, v4));
    print_i64(restrict_apart(arr, v4));
    print_i64(c23_pointer());
    g = 1;
    print_i64(c23_values(v7));
    int s = 0;
    for (int i = 0; i < 4; i++)
        s += sq(v + 1);
    print_i64(s);
    print_i64(aggregates(v3));
    return 0;
}
/* expect: 18 */
/* expect: 515 */
/* expect: 10 */
/* expect: 5 */
/* expect: 0 */
/* expect: 21 */
/* expect: 6 */
/* expect: 8 */
/* expect: 43 */
/* expect: 12 */
/* expect: 28115 */
/* expect: 196 */
/* expect: 4301060804 */
