/* flags: --c-std=c11 */
/* C11 6.7p3: a typedef name may be defined again with the same type (not a
   variably modified one), in the same scope too. */
#include "host.h"

typedef int T;
typedef int T;
typedef struct Node Node;
typedef struct Node { int v; Node *next; } Node;
typedef unsigned long U, U;
typedef int A[3];
typedef int A[3];

int main(void) {
    T t = 4;
    Node n = { 5, 0 };
    A a = { 1, 2, 3 };
    U u = 6;
    print_i64(t + n.v + a[2] + (T)u);
    return 0;
}
/* expect: 18 */
