/* C23's assert (7.2.1.1) is variadic: an expression with commas outside
   parentheses (a compound literal) is one argument. */
/* flags: --c-std=c23 */
/* expect-status: 7 */
/* expect-stderr: assertion failed: (struct p){1, 2}.y == 3 */
#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include "print.h"

struct p {
    int x, y;
};

static void handler(int sig) {
    _Exit(sig == SIGABRT ? 7 : 1);
}

int main(void) {
    signal(SIGABRT, handler);
    assert((struct p){1, 2}.x == 1);
    static_assert(1);
    print_i64(__STDC_VERSION_ASSERT_H__); /* expect: 202311 */
    assert((struct p){1, 2}.y == 3);
    return 0;
}
