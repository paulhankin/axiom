/* flags: --c-std=c89 */
/* C89: implicit int, implicit function declarations, K&R definitions,
   declarations at the start of blocks, no long long (__axiom_int64 is the
   compiler's 64-bit type), and a return without a value. */
#include "host.h"

static count = 3;

mul(a, b)
    int a;
    int b;
{
    return a * b;
}

static int twice(x) int x; { return later(x) * 2; }

int later(int x) { return x + 1; }

void nothing(void) { return; }

int novalue() { return; }

int main(void) {
    int i;
    register int r = 4;
    __axiom_int64 big = 1;
    for (i = 0; i < 40; i++) big *= 2;
    print_i64(mul(6, 7));
    print_i64(count);
    print_i64(twice(4));
    print_i64(r);
    print_i64(big);
    nothing();
    {
        unsigned long ul = 4294967295UL;
        print_i64(ul == 4294967295UL);
    }
    return 0;
}

/* expect: 42 */
/* expect: 3 */
/* expect: 10 */
/* expect: 4 */
/* expect: 1099511627776 */
/* expect: 1 */
