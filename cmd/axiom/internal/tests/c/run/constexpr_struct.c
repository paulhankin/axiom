/* flags: --c-std=c23 */
/* constexpr structures (C23): named constants of structure type initialize
   other constexpr and static objects, whose members are named constants in
   turn; members not given are zero. */
#include "host.h"

struct S { int a; struct { short b, c; } in; long long big; };
constexpr struct S s = { 1, { .c = 3 } };
constexpr struct S t = s;
static struct S u = s;
static struct S arr[2] = { s, [1].in = s.in };
static_assert(t.in.c == 3 && t.big == 0);
static int sized[t.a + t.in.c];

int main(void) {
    struct S l = s;
    constexpr struct S m = t;
    print_i64(t.a + t.in.c + u.a + u.in.c);
    print_i64(arr[0].in.c + arr[1].in.c + arr[1].a);
    print_i64(l.in.c + l.in.b + l.big + m.a);
    print_i64(sizeof sized / sizeof sized[0]);
    return 0;
}
/* expect: 8 */
/* expect: 6 */
/* expect: 4 */
/* expect: 4 */
