/* flags: --c-std=c99 */
/* expect-error: C1040 */
/* <stdatomic.h> is a header of C11. */
#include <stdatomic.h>
int main(void) { return 0; }
