/* The variable length arrays of vla_basic.c, in C99. */
/* flags: --c-std=c99 */
#include "host.h"
static int sum(int n, int a[n]) { int s = 0; for (int i = 0; i < n; i++) s += a[i]; return s; }
static int sum2(int n, int m, int a[n][m]) {
    int s = 0;
    for (int i = 0; i < n; i++) for (int j = 0; j < m; j++) s += a[i][j] * (i + 1);
    return s;
}
int main(void) {
    int n = 5;
    int a[n];
    for (int i = 0; i < n; i++) a[i] = i * i;
    print_i64(sum(n, a));
    print_i64(sizeof a);
    int m = 3;
    int b[n][m];
    print_i64(sizeof b);
    print_i64(sizeof b[0]);
    for (int i = 0; i < n; i++) for (int j = 0; j < m; j++) b[i][j] = i + j;
    print_i64(sum2(n, m, b));
    int (*p)[m] = b;
    p++;
    print_i64((*p)[2]);
    print_i64(p - b);
    long total = 0;
    for (int k = 0; k < 100000; k++) { char big[1000 + k % 7]; big[0] = (char)k; big[999] = 1; total += big[999] + sizeof big; }
    print_i64(total);
    typedef int T[n];
    n = 100;
    T t;
    print_i64(sizeof(T));
    print_i64(sizeof t);
    int q = 2;
    print_i64(sizeof(int[q++]));
    print_i64(q);
    return 0;
}

/* expect: 30 */
/* expect: 20 */
/* expect: 60 */
/* expect: 12 */
/* expect: 165 */
/* expect: 3 */
/* expect: 1 */
/* expect: 100399995 */
/* expect: 20 */
/* expect: 20 */
/* expect: 8 */
/* expect: 3 */
