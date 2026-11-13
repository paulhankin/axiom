/* flags: --c-std=gnu17 */
/* wasm-flags: --features=+exception-handling */
/* expect-trap */
/* A goto from a nested function to a label of its enclosing function in
   the scope of a variable length array that the call it jumps from is not
   in: a jump into that scope (6.8.6.1p1), which traps. */
#include "host.h"

__attribute__((noinline)) static void apply(void (*f)(void)) { f(); }

static int into(int n) {
    __label__ in;
    void f(void) { goto in; }
    apply(f);
    {
        int a[n];
        a[0] = 1;
    in:
        return a[0];
    }
}

int main(void) {
    return into(4);
}
