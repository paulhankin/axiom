/* flags: --c-std=gnu99 */
/* wasm-only */
/* On WebAssembly too a weak reference to a function no unit defines is a
   null pointer (a call of it, made only when it is not null, is still an
   import of env), and one to an object a null address. */
#include "host.h"

extern int maybe_absent(void) __attribute__((weak));
extern int also_absent(void);
#pragma weak also_absent
extern int absent_object __attribute__((weak));

int main(void) {
    print_i64(maybe_absent == 0);
    print_i64(also_absent == 0);
    print_i64(&absent_object == 0);
    if (maybe_absent)
        return maybe_absent();
    return 0;
}
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
