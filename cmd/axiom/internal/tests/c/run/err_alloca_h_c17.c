/* flags: --c-std=c17 */
/* expect-error: C3060 */
/* <alloca.h> is a GNU dialect's header: under an ISO standard, which has no
   alloca, it declares nothing. */
#include <alloca.h>
int main(void) { return *(char *)alloca(1); }
