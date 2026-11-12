/* flags: --c-long-double=128 */
/* linux-only */
/* ldouble_ops.c with long double binary128 on every target that has an
   ABI for it (on x86-64, the ABI's __float128: SSE and SSEUP). */
#include "ldouble_ops.c"

/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 10 */
/* expect: 1 */
/* expect: -100 */
/* expect: 200 */
/* expect: -30000 */
/* expect: 65535 */
/* expect: -2147483648 */
/* expect: -9223372036854775808 */
/* expect: 9223372036854775808 */
/* expect: 18446744073709549568 */
/* expect: 2 */
/* expect: 0.125 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 3 */
/* expect: 11 */
/* expect: 1 */
/* expect: -3 */
/* expect: 1.25 */
/* expect: -0.75 */
/* expect: 68.875 */
/* expect: 9.5 */
/* expect: -9 */
/* expect: -6 */
/* expect: 1 */
