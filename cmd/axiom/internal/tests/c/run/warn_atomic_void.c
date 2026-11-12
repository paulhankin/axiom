/* flags: --c-std=c11 */
/* expect-warning: C4091 */
/* 6.5.16.1p1: a pointer to void that does not have the _Atomic qualifier of
   the type pointed to: a warning (such pointers go to functions taking
   void *), and the conversion is made. */
#include "host.h"
_Atomic int x = 6;
int main(void) {
    void *p = &x;
    print_i64(p == (void *)&x);
    return 0;
}
/* expect: 1 */
