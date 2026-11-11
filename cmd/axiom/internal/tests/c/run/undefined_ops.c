/* Operations whose behavior is undefined compile at every optimization
   level and run (their results are not checked): shift counts at least the
   width or negative, constant division by zero in code not executed,
   signed overflow in constants outside constant expressions. */
#include <limits.h>
#include "host.h"

volatile long long sink;
volatile int never = 0;

static int f(void) { return 1 << 40; }
static int g(void) { return -1 >> 99; }
static long long h(void) { return 1LL << 64; }
static unsigned k(void) { return 1u << -1; }
static int z(void) { int a = 0; return 5 / a + 7 % 0; }
static int o(void) { return INT_MAX + 1; }
static int m(void) { return INT_MIN % -1; }

int main(void) {
    sink = f() + g() + h() + k() + o();
    if (never) {
        sink = z() + m();
    }
    print_i64(1);
    return 0;
}

/* expect: 1 */
