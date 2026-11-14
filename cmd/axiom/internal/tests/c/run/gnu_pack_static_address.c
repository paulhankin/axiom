/* flags: --c-std=gnu17 */
/* An address in a static initializer at an offset its type does not align:
   a member of a packed structure, or of one under #pragma pack, holds the
   address at the member's own offset (the initial value has no padding
   before it). */
#include "host.h"

int x = 42, y = 7;
static int twice(int v) { return 2 * v; }

#pragma pack(2)
struct p2 { short a; int *b; };
#pragma pack()

struct __attribute__((packed)) p1 { char c; int *p; int (*f)(int); char d; };

struct p2 v2 = { .b = &x };
struct p1 v1[2] = { { 1, &y, twice, 3 }, { 4, &x + 0, 0, 5 } };

int main(void) {
    print_i64(sizeof(struct p1) == 2 + 2 * sizeof(void *));
    print_i64(*v2.b);
    print_i64(v1[0].c + *v1[0].p + v1[0].d);
    print_i64(v1[0].f(10));
    print_i64(v1[1].c + *v1[1].p + v1[1].d + (v1[1].f == 0));
    return 0;
}
/* expect: 1 */
/* expect: 42 */
/* expect: 11 */
/* expect: 20 */
/* expect: 52 */
