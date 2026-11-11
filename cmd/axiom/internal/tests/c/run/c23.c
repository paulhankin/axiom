/* flags: --c-std=c23 */
/* C23: bool, true and false as keywords, static_assert without a message,
   empty initializers, unnamed parameters in definitions, attributes,
   labels before declarations and at the end of a block, () as (void). */
#include "host.h"

static_assert(sizeof(int) >= 2);

static int first(int a, int) { return a; }
static int noargs() { return 3; }

[[nodiscard]] static int attr(int x) { return x + 1; }

int main(void) {
    bool b = true;
    int z[3] = {};
    struct { int a; double d; } s = {};
    int i = 0;
    print_i64(b);
    print_i64(false + true * 2);
    print_i64(sizeof(bool));
    print_i64(z[0] + z[1] + z[2]);
    print_f64(s.a + s.d);
    print_i64(first(4, 5));
    print_i64(noargs());
    print_i64(attr(1));
again:
    int k = i++;
    if (k < 2) goto again;
    print_i64(i);
    {
        goto end;
    end:
    }
    return 0;
}

/* expect: 1 */
/* expect: 2 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */
/* expect: 4 */
/* expect: 3 */
/* expect: 2 */
/* expect: 3 */
