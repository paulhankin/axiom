/* flags: --c-long-double=128 */
/* linux-only */
/* ldouble_bits.c with long double binary128 on every target that has an
   ABI for it (on x86-64, the ABI's __float128: SSE and SSEUP). */
#include "ldouble_bits.c"

/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
