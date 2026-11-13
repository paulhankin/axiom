/* setjmp and longjmp (C17 7.13) in more shapes: longjmp across a thousand
   frames, past a frame that has a setjmp of its own (on another buffer),
   from a signal handler that raise called, back into a loop many times
   (setjmp in a for statement's body, and with goto around it), repeated
   setjmps on one buffer in one function, a variable length array made
   after the setjmp (freed by the jump, as the stack pointer goes back),
   recursion where each level has its own buffer, and threads each jumping
   on their own buffers at the same time. */
/* wasm-flags: --features=+exception-handling */
#include <setjmp.h>
#include <signal.h>
#include <threads.h>
#include "print.h"

static jmp_buf top, mid;

static void deep(int n) {
    if (n == 0)
        longjmp(top, 7);
    deep(n - 1);
}

/* has a setjmp on mid, but the jump is to top: it passes through */
static int middle(void) {
    volatile int here = 0;
    if (setjmp(mid) != 0)
        return -1;
    here = 1;
    deep(1000);
    return here;
}

static void on_signal(int sig) {
    longjmp(top, sig);
}

static int vla_jump(int n) {
    volatile int rounds = 0;
    if (setjmp(top) != 0)
        return rounds;
    {
        char a[n];
        a[0] = 1;
        a[n - 1] = 2;
        rounds = a[0] + a[n - 1];
        if (rounds == 3)
            longjmp(top, 1);
    }
    return -1;
}

/* each level its own buffer, the jump from the innermost to the outermost */
static int levels(int n, jmp_buf *outer) {
    jmp_buf mine;
    volatile int seen = n;
    if (n == 0)
        longjmp(*outer, 5);
    if (setjmp(mine) != 0)
        return -100;
    return seen + levels(n - 1, outer ? outer : &mine);
}

static int worker(void *arg) {
    jmp_buf b;
    volatile int i = 0;
    volatile long sum = 0;
    int r = setjmp(b);
    sum += r;
    if (i < 1000) {
        i++;
        longjmp(b, (int)(long)arg);
    }
    return (int)(sum / 1000);
}

int main(void) {
    volatile int count = 0, k;
    int r;
    if ((r = setjmp(top)) == 0)
        print_i64(middle());
    print_i64(r); /* expect: 7 */
    signal(SIGINT, on_signal);
    if ((r = setjmp(top)) == 0)
        raise(SIGINT);
    print_i64(r); /* expect: 2 */
    signal(SIGINT, SIG_DFL);
    for (k = 0; k < 5; k++) {
        if (setjmp(top) == 0)
            longjmp(top, 1);
        count++;
    }
    print_i64(count); /* expect: 5 */
    count = 0;
again:
    if (setjmp(mid) < 3) {
        count++;
        if (count < 50)
            longjmp(mid, count % 3);
    }
    if (count < 50)
        goto again;
    print_i64(count); /* expect: 50 */
    /* the same buffer set twice in a row: the second call is the one */
    count = 0;
    if (setjmp(top) == 0) {
        if (setjmp(top) == 0)
            longjmp(top, 9);
        count = 2;
    } else {
        count = 1;
    }
    print_i64(count); /* expect: 2 */
    print_i64(vla_jump(100000)); /* expect: 3 */
    {
        jmp_buf b0;
        if ((r = setjmp(b0)) == 0)
            r = levels(30, &b0);
        print_i64(r); /* expect: 5 */
        print_i64(levels(30, NULL)); /* expect: -100 */
    }
    {
        thrd_t t[4];
        int res, total = 0;
        for (k = 0; k < 4; k++)
            thrd_create(&t[k], worker, (void *)(long)(k + 1));
        for (k = 0; k < 4; k++) {
            thrd_join(t[k], &res);
            total += res;
        }
        print_i64(total); /* expect: 10 */
    }
    return 0;
}
