/* --c-freestanding: __STDC_HOSTED__ is 0, and a header of the library is
   not available (C1081). */
/* flags: --c-freestanding */
/* expect-error: C1081 */
#if __STDC_HOSTED__
#error "hosted"
#endif
#include <stddef.h>
#include <stdlib.h>
int main(void) { return 0; }
