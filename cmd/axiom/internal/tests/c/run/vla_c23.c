/* C23: variable length arrays initialized with {} (zeros), a label before a
   VLA declaration that a goto jumps back to, a label at the end of a block,
   and a large VLA whose pages are touched in order. */
/* flags: --c-std=c23 */
#include "host.h"
static int big(int n) {
    char a[n];
    a[0] = 5;
    a[n - 1] = 6;
    return a[0] + a[n - 1];
}
int main(void) {
    int m = 3;
    int z[m] = {};
    long long k[m][m + 1] = {};
    print_i64(z[0] + z[1] + z[2] + (int)k[2][3] + (int)sizeof k);
    int i = 0;
again:
    int w[m + i] = {};
    w[i] = i;
    if (++i < 4) goto again;
    print_i64(sizeof w);
    print_i64(big(400 * 1024));
    {
        int q[i];
        q[0] = 1;
        print_i64(q[0]);
    end:
    }
    return 0;
}

/* expect: 96 */
/* expect: 24 */
/* expect: 11 */
/* expect: 1 */
