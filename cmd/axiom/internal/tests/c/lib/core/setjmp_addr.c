/* setjmp is only called: a call through a pointer would not be known to
   return twice, so taking the address of the function is an error
   (C3301). */
/* native-only */
/* expect-error: C3301 */
#include <setjmp.h>

int main(void) {
    int (*f)(__axiom_int64 *) = __axc_setjmp;
    return f != 0;
}
