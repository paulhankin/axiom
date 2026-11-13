/* flags: --c-std=gnu17 */
/* wasm-flags: --features=+exception-handling */
/* A goto in a nested function to a local label (__label__) of its enclosing
   function: the nested function's call, and those between, are left, and
   the jump goes on as a goto in the enclosing function from the call it is
   making then: the cleanups of the scopes it leaves are called and their
   variable length arrays freed; the nested function's own cleanups too. */
#include "host.h"

__attribute__((noinline)) static int apply(int (*f)(int), int x) { return f(x) + 1; }

/* through a pointer, a call of another function */
static int jump(int x) {
    __label__ out;
    int check(int y) {
        if (y < 0)
            goto out;
        return y * 2;
    }
    return apply(check, x);
out:
    return -100;
}

/* a direct call; two levels of nesting */
static int deep(int x) {
    __label__ done;
    int r = 0;
    int outer(int y) {
        int inner(int z) {
            if (z > 3)
                goto done;
            return z;
        }
        return inner(y + 1) + inner(y + 2);
    }
    r = outer(x);
    return r;
done:
    return 1000 + r;
}

static int log_[16];
static int nlog;
static void note(int *p) {
    if (nlog < 16)
        log_[nlog] = *p;
    nlog++;
}

/* the cleanups of the scopes left, the innermost first; the nested
   function's own */
static int cleanups(int x) {
    __label__ out;
    int fail(int y) {
        int own __attribute__((cleanup(note))) = 30;
        if (y)
            goto out;
        return 0;
    }
    {
        int a __attribute__((cleanup(note))) = 10;
        {
            int b __attribute__((cleanup(note))) = 20;
            apply(fail, x);
            log_[nlog++] = 99;
        }
    }
    return 0;
out:
    return nlog;
}

/* variable length arrays: freed when their scope is left (many times, so
   that the stack would run out otherwise); one in the label's scope stays */
static long vla(int n) {
    __label__ next, inside;
    long sum = 0;
    int i = 0;
    int leave(int y) {
        if (y >= 0)
            goto next;
        goto inside;
    }
again:
    if (i < 3000) {
        int big[n];
        for (int k = 0; k < n; k++)
            big[k] = k;
        sum += big[n - 1];
        apply(leave, i);
    }
next:
    i++;
    if (i < 3000)
        goto again;
    {
        int keep[n];
        keep[0] = 7;
        keep[n - 1] = 9;
        apply(leave, -1);
        sum = -1;
    inside:
        sum += keep[0] + keep[n - 1];
    }
    return sum;
}

/* the jump goes to the activation the nested function belongs to */
static int rec(int depth, int (*up)(int)) {
    __label__ out;
    int hop(int y) {
        if (y == depth)
            goto out;
        return 0;
    }
    if (depth < 4)
        return rec(depth + 1, hop) + 100;
    up(depth - 1);
    return -1;
out:
    return depth;
}

/* the trampoline a jumping call held is given back when its enclosing
   function returns */
static int many(int x) {
    __label__ l;
    int f(int y) { goto l; }
    apply(f, x);
    return 0;
l:
    return 1;
}

/* a nested function's call between: its cleanups are called and its
   trampolines given back as the jump passes through it */
static int between(int x) {
    __label__ out;
    int leave(int y) {
        goto out;
    }
    int middle(int y) {
        int m __attribute__((cleanup(note))) = 40 + y;
        int own(int z) { return leave(z); }
        return apply(own, y);
    }
    apply(middle, x);
    return -1;
out:
    return 1;
}

int main(void) {
    print_i64(jump(3));
    print_i64(jump(-1));
    print_i64(deep(1));
    print_i64(deep(5));
    print_i64(cleanups(0));
    print_i64(cleanups(1));
    for (int i = 0; i < nlog; i++)
        print_i64(log_[i]);
    print_i64(vla(500));
    print_i64(rec(0, 0));
    int s = 0;
    for (int i = 0; i < 1000; i++)
        s += many(i);
    print_i64(s);
    nlog = 0;
    s = 0;
    for (int i = 0; i < 1000; i++)
        s += between(i % 2);
    print_i64(s);
    print_i64(nlog);
    print_i64(log_[0] + log_[1]);
    return 0;
}
/* expect: 7 */
/* expect: -100 */
/* expect: 5 */
/* expect: 1000 */
/* expect: 0 */
/* expect: 7 */
/* expect: 30 */
/* expect: 99 */
/* expect: 20 */
/* expect: 10 */
/* expect: 30 */
/* expect: 20 */
/* expect: 10 */
/* expect: 1497016 */
/* expect: 303 */
/* expect: 1000 */
/* expect: 1000 */
/* expect: 1000 */
/* expect: 81 */
