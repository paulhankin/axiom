/* <inttypes.h> is a header of C99: in C95, its #error. */
/* check-only */
/* flags: --c-std=c95 */
/* expect-error: C1040 */
#include <inttypes.h>
