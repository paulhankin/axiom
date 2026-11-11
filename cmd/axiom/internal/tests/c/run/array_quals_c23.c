/* flags: --c-std=c23 */
/* C23 6.7.4.1p3: an array type is qualified as its element type is, so a
   pointer to an array converts to a pointer to an array of qualified
   elements (in assignment, as an argument), and such pointers compare and
   subtract. */
#include "host.h"

static int last(int n, const int a[][3]) { return a[n - 1][2]; }

int main(void) {
    int x[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };
    const int (*p)[3] = x;
    const volatile int (*q)[3] = p;
    print_i64(last(2, x) + p[0][0] + q[1][1]);
    print_i64((x + 1) - p);
    print_bool(p == x && q != x + 1);
    return 0;
}

/* expect: 12 */
/* expect: 1 */
/* expect: true */
