/* flags: --c-std=gnu11 */
/* A GNU dialect's designator range fills a static table with one address,
   and later designators replace some of its elements (6.7.9p19). */
#include "host.h"

static int dflt(void) { return 0; }
static int a(void) { return 1; }
static int b(void) { return 2; }

typedef int (*fn)(void);
static const fn ops[5] = { [0 ... 4] = &dflt, [1] = a, [3] = b };

int main(void) {
    long r = 0;
    for (int i = 0; i < 5; i++) r = r * 10 + ops[i]();
    print_i64(r);
    print_i64(sizeof(ops) / sizeof(ops[0]));
    return 0;
}
/* expect: 1020 */
/* expect: 5 */
