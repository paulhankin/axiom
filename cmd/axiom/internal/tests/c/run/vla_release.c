/* The stack space of variable length arrays is released when their scope is
   left: at the end of a loop's body, by continue, break and goto (backwards,
   to before the declaration), from nested blocks and switch statements, and
   from a for statement's declaration. Each loop allocates far more in total
   than the stack holds. */
#include "host.h"
static int use(char *p, int n) { p[0] = 1; p[n - 1] = 2; return p[0] + p[n - 1]; }
int main(void) {
    long s = 0;
    int n = 60000;
    for (int i = 0; i < 2000; i++) {
        char a[n];
        s += use(a, n);
        if (i % 3 == 0) continue;
        {
            char b[n];
            s += use(b, n);
            if (i % 5 == 0) continue;
            if (i == 1999) break;
        }
    }
    print_i64(s);
    int i = 0;
again:
    if (i < 3000) {
        char c[n];
        s += use(c, n);
        i++;
        goto again;
    }
    print_i64(s);
    i = 0;
    while (i < 3000) {
        char d[n + i];
        s += use(d, n + i);
        i++;
        switch (i % 4) {
        case 0: { char e[n]; s += use(e, n); break; }
        case 1: continue;
        default: break;
        }
    }
    print_i64(s);
    i = 0;
    do { char f[n]; s += use(f, n); } while (++i < 3000);
    for (int k = 0; k < 3000; k++) for (char g[n]; ; ) { s += use(g, n); break; }
    for (int k = 0; k < 3000; k++) for (char g[n], *q = g; q; q = 0) { s += use(g, n); }
    print_i64(s);
    i = 0;
loop2: ;
    char h[n];
    s += use(h, n);
    if (++i < 3000) goto loop2;
    print_i64(s);
    return 0;
}

/* expect: 9999 */
/* expect: 18999 */
/* expect: 30249 */
/* expect: 57249 */
/* expect: 66249 */
