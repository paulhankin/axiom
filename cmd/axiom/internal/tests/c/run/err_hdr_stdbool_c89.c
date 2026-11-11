/* flags: --c-std=c89 */
/* expect-error: C1040 */
/* <stdbool.h> is a header of C99 and later. */
#include <stdbool.h>
int main(void) { return 0; }
