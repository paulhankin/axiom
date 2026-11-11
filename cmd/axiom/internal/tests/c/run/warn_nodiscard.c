/* flags: --c-std=c23 */
/* expect-warning: C4051 */
/* [[nodiscard]] (6.7.13): a call of a function declared with it, or of
   one that returns a structure, union or enumeration declared with it,
   whose result is discarded (an expression statement, a clause of `for`,
   an operand of the comma operator) is warned about; a cast to void
   discards it on purpose. The program still does what it says. */
#include "host.h"

static int n;
[[nodiscard("it says how many")]] static int count(void) { return ++n; }
struct [[nodiscard]] Pair { int a, b; };
static struct Pair pair(void) { struct Pair p = { ++n, 0 }; return p; }
enum [[nodiscard]] Status { OK, FAIL };
static enum Status status(void) { ++n; return OK; }

int main(void) {
    count();
    (void)count();
    pair();
    status();
    int x = (count(), 10);
    for (count(); n < 8; count()) { }
    print_i64(n);
    print_i64(x);
    return 0;
}
/* expect: 8 */
/* expect: 10 */
