/* flags: --c-std=c11 */
/* expect-warning: C4090 */
/* 6.5.2.3p5: accessing a member of an atomic structure is undefined; a
   warning, and the member is accessed as an ordinary object. */
#include "host.h"
struct s { int a, b; };
_Atomic struct s x = { 4, 5 };
int main(void) {
    print_i64(x.b);
    return 0;
}
/* expect: 5 */
