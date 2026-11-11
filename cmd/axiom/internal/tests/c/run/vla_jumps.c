/* Jumps out of the scopes of variable length arrays release just the arrays
   of the scopes they leave: a goto back into an enclosing VLA's scope, a goto
   out of two, break and continue from a block in a switch in a loop. */
#include "host.h"
static long long addr(void *p) { return (long long)(char *)p; }
int main(void) {
    int n = 1000;
    long long base = 0, s = 0;
    int i = 0;
    {
        int a[n];
        a[0] = 1;
    L:
        if (i == 1) base = addr(&i);  /* (just a value to keep) */
        {
            int b[n];
            b[0] = a[0] + i;
            s += b[0];
            if (++i < 5000) goto L;  /* releases b only */
        }
        s += a[0];
    }
    print_i64(s);
    s = 0;
    for (int r = 0; r < 4000; r++) {
        {
            int c[n];
            c[n - 1] = r;
            {
                int d[n];
                d[0] = c[n - 1];
                if (r & 1) goto out;  /* out of both */
                s += d[0];
            }
            s += 1;
        }
    out:
        s += 2;
    }
    print_i64(s);
    s = 0;
    for (int r = 0; r < 3000; r++) {
        int e[n];
        e[0] = r;
        switch (r % 3) {
        case 0: {
            int f[n];
            f[0] = e[0];
            if (r % 2) break;  /* out of the switch, f released */
            s += f[0];
            continue;  /* to the next round: e and f released */
        }
        case 1:
            s += 1;
            break;
        }
        s += e[0];
    }
    print_i64(s);
    (void)base;
    return 0;
}

/* expect: 12502501 */
/* expect: 4008000 */
/* expect: 4499500 */
