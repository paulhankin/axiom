/* flags: --c-std=c17 */
/* expect-error: C1040 */
/* <stdbit.h> is a header of C23. */
#include <stdbit.h>
int main(void) { return 0; }
