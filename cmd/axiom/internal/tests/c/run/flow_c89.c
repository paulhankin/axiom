/* Statements in C89: declarations at the start of blocks, for with an
   expression first, goto, switch with a block in a case, break and continue. */
/* flags: --c-std=c89 */
#include "host.h"
static int f(int n) {
    int i, s;
    s = 0;
    for (i = 0; i < n; i++) {
        int k;
        k = i * i;
        if (k & 1) continue;
        s += k;
    }
    i = 0;
loop:
    if (i < 3) {
        s += 100;
        i++;
        goto loop;
    }
    switch (n) {
    case 1: s = -1; break;
    default:
        {
            int t;
            t = n;
            while (t > 0) {
                t--;
                if (t == 2) break;
                s++;
            }
        }
    }
    return s;
}
int main(void) {
    print_i64(f(1));
    print_i64(f(8));
    return 0;
}

/* expect: -1 */
/* expect: 361 */
