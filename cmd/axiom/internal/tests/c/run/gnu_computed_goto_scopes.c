/* flags: --c-std=gnu17 */
/* `goto *` where variable length arrays and objects with cleanups are in
   scope (a GNU dialect): the jump leaves the scopes a goto would, freeing
   the arrays (the stack does not grow over many jumps) and calling the
   cleanups, innermost first; labels in such scopes, jumped to from within
   them; a label's address taken in one. */
#include "host.h"

static int log_[16];
static int nlog;
static void done(int *p) { log_[nlog++] = *p; }

static long run(int n) {
    static void *const tab[] = { &&again, &&out };
    long sum = 0;
    int round = 0;
again:
    if (round == 100000)
        goto *tab[1];
    {
        int big[n];  /* freed by each jump back */
        for (int i = 0; i < n; i++)
            big[i] = i;
        sum += big[n - 1];
        round++;
        goto *tab[0];
    }
out:
    return sum;
}

static int cleanups(int k) {
    void *t = &&end;
    {
        int a __attribute__((cleanup(done))) = 1;
        {
            int b __attribute__((cleanup(done))) = 2;
            int v[k];
            v[0] = 3;
            if (k > 0)
                goto *t;
        }
    }
end:
    return nlog;
}

static int inside(int n) {
    int r = 0;
    int v[n];
    void *t[2] = { &&top, &&bottom };
    int i = 0;
top:
    v[i] = i * 3;
    i++;
    {
        int c __attribute__((cleanup(done))) = 10 + i;
        if (i < n)
            goto *t[0];
    }
    for (int j = 0; j < n; j++)
        r += v[j];
    if (r < 0)
        goto *t[1];
bottom:
    return r;
}

int main(void) {
    print_i64(run(1000));
    print_i64(cleanups(5));
    print_i64(log_[0] * 10 + log_[1]);
    nlog = 0;
    print_i64(inside(4));
    print_i64(nlog);
    print_i64(log_[0] * 1000 + log_[1] * 100 + log_[3]);
    return 0;
}
/* expect: 99900000 */
/* expect: 2 */
/* expect: 21 */
/* expect: 18 */
/* expect: 4 */
/* expect: 12214 */
