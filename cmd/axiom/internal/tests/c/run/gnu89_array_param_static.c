/* flags: --c-std=gnu89 */
/* gnu89 takes C99's `static`, qualifiers and `[*]` in an array parameter's
   declarator, as GNU code built as C90 uses them (`char buf[static 1024]`). */
#include "host.h"
static int first(char buf[static 4]) { return buf[0]; }
static int second(int n, const int a[const __restrict 2]);
static int second(int n, const int a[const __restrict 2]) { return a[1] + n; }
int proto(int n, int a[*]);
int proto(int n, int a[n]) { return a[n - 1]; }
int main(void) {
    char b[4] = "xyz";
    int a[3] = { 1, 2, 3 };
    print_i64(first(b) + second(1, a) + proto(3, a));
    return 0;
}
/* expect: 126 */
