/* flags: --c-std=gnu99 */
/* wasm-only */
/* expect-trap */
/* A call of a weak function no unit defines (undefined: its address is
   null) reaches the linker's stub of its type, which traps. */
#include "host.h"

extern int maybe_absent(int) __attribute__((weak));

int main(void) {
    print_i64(1);
    return maybe_absent(2);
}
/* expect: 1 */
