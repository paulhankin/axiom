/* The size expressions of variable length arrays are evaluated once, in the
   order they are written (the standard leaves the order unspecified), each
   declarator's before the next declarator, a typedef's where it is declared. */
#include "host.h"
static int calls;
static int next(int x) { calls = calls * 10 + x; return x; }
int main(void) {
    int a[next(1)][next(2)][next(3)];
    print_i64(calls);
    calls = 0;
    int (*p[next(4)])[next(5)];
    print_i64(calls);
    calls = 0;
    print_i64(sizeof(int[next(6)][next(7)]));
    print_i64(calls);
    calls = 0;
    typedef int T[next(8)];
    T t[next(9)];
    print_i64(calls);
    print_i64(sizeof t);
    calls = 0;
    int n = next(1), b[n + next(2)], c[n + next(3)];
    print_i64(calls);
    print_i64(sizeof b + sizeof c);
    (void)a; (void)p;
    return 0;
}

/* expect: 123 */
/* expect: 45 */
/* expect: 168 */
/* expect: 67 */
/* expect: 89 */
/* expect: 288 */
/* expect: 123 */
/* expect: 28 */
