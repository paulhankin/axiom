/* flags: --c-std=c89 */
/* In C89 mode <math.h> declares the float and long double versions of the
   C89 functions too (cosf, sqrtl, ...), whose names C89 4.13.4 reserves to
   it: a call of one is to that function, not an implicit declaration of a
   function returning int. */
/* (print.h has _Bool, which C89 lacks) */
void print_i64(__axiom_int64 x);
#include <math.h>

int main(void) {
    float f = sqrtf(2.25f);
    long double l = fabsl(-3.0L);
    print_i64((long)(f * 10));
    print_i64((long)l);
    print_i64((long)(cosf(0.0f) + floorl(2.5L)));
    return 0;
}
/* expect: 15 */
/* expect: 3 */
/* expect: 3 */
