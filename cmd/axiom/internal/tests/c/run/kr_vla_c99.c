/* flags: --c-std=c99 */
/* A K&R definition's parameters may be variable length arrays (C99): each
   parameter is in scope after its declarator in the declaration list, for
   the sizes of the later ones. */
#include "host.h"

int sum(n, a) int n; int a[n]; {
    int s = 0, i;
    for (i = 0; i < n; i++) s += a[i];
    return s;
}

int rows(n, m, a) int n, m; int a[n][m]; {
    return (int)sizeof(a[0]) / (int)sizeof(int) * 100 + a[1][2];
}

int whole(a, n) int n; int (*a)[n]; {
    return (int)sizeof(*a);
}

int main(void) {
    int a[3] = { 1, 2, 3 };
    int b[2][3] = { { 0 }, { 0, 0, 7 } };
    print_i64(sum(3, a));
    print_i64(rows(2, 3, b));
    print_i64(whole(b, 3) / (int)sizeof(int));
    return 0;
}

/* expect: 6 */
/* expect: 307 */
/* expect: 3 */
