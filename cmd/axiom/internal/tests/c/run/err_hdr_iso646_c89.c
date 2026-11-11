/* flags: --c-std=c89 */
/* expect-error: C1040 */
/* <iso646.h> is a header of C95 and later. */
#include <iso646.h>
int main(void) { return 0; }
