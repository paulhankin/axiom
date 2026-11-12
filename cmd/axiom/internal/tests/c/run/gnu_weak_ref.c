/* flags: --c-std=gnu99 */
/* linux-only */
/* native-only */
/* A weak reference to a function no unit defines is a null pointer (the
   system's linker resolves it so); #pragma weak makes one too. */
#include "host.h"

extern int maybe_absent(void) __attribute__((weak));
extern int also_absent(void);
#pragma weak also_absent

int main(void) {
    print_i64(maybe_absent == 0);
    print_i64(also_absent == 0);
    return 0;
}
/* expect: 1 */
/* expect: 1 */
