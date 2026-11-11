/* flags: --c-std=c17 */
/* expect-error: C1040 */
/* <stdckdint.h> is a header of C23. */
#include <stdckdint.h>
int main(void) { return 0; }
