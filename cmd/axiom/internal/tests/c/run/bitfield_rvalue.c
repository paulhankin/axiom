/* flags: --c-std=c99 */
/* A bit-field of a structure that is not an lvalue (a function's result,
   a conditional's, an assignment's) promotes by its width like any other
   (6.3.1.1p2): to int when int holds every value. Found by differential
   testing. */
#include "host.h"

struct S { _Bool b : 1; unsigned int u : 30; unsigned int w : 32; signed int s : 5; unsigned long long l : 40; };
static struct S g = { 1, 513659352, 7, -3, 1 };
static struct S get(void) { return g; }

int main(void) {
    struct S a = g, b;
    print_i64(~get().u);
    print_i64(-(1 ? a : g).u < 0);
    print_i64((b = a).u - 513659353 < 0);
    print_i64(get().w - 8 > 0);
    print_i64(-get().u < 0);
    print_i64(get().s - get().b);
    print_f64((float)~get().u);
    return 0;
}

/* expect: -513659353 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: -4 */
/* expect: -513659360 */
