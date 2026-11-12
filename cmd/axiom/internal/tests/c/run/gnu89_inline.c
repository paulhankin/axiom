/* flags: --c-std=gnu89 */
/* In gnu89 (and gnu90) `inline` alone defines the external function (it
   may keep a static object: no inline definition, 6.7.4p3), as does a
   function with the attribute gnu_inline in a later mode; `extern inline`
   is the inline definition. Long long, // comments, mixed declarations
   and designated initializers are the dialect's too. */
#include "host.h"

inline int counter(void) {
    static int n;
    return ++n;
}

int main(void) {
    counter();
    long long big = 1LL << 40; // a comment
    int after = 2;
    struct { int a, b; } s = { .b = 3 };
    print_i64(counter() + (big >> 40) + after + s.b);
    return 0;
}
/* expect: 8 */
