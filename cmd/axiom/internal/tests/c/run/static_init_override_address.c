/* flags: --c-std=c99 */
/* Designators may initialize a subobject again (6.7.9p19): the last
   initializer for it is the one that counts, for addresses in an object
   with static storage duration as for numbers, an address replaced by a
   number and a number by an address too. */
#include "host.h"

static int one(void) { return 1; }
static int two(void) { return 2; }
static int three(void) { return 3; }
static int x, y;

typedef int (*fn)(void);
static const fn table[3] = { one, one, one, [0] = two, [2] = three };
static int *ptrs[3] = { &x, &x, &x, [1] = &y, [1] = 0, [2] = &y };
struct pair { int *p; long n; };
static struct pair pairs[2] = { { &x, 1 }, { &y, 2 }, [0].p = &y, [1] = { 0, 4 } };

int main(void) {
    print_i64(table[0]() * 100 + table[1]() * 10 + table[2]());
    print_i64((ptrs[0] == &x) + 2 * (ptrs[1] == 0) + 4 * (ptrs[2] == &y));
    print_i64((pairs[0].p == &y) + 2 * (pairs[1].p == 0) + 4 * (pairs[1].n == 4));
    print_i64(sizeof(table) / sizeof(table[0]));
    return 0;
}
/* expect: 213 */
/* expect: 7 */
/* expect: 7 */
/* expect: 3 */
