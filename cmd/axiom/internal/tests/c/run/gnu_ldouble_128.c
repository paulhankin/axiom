/* flags: --c-std=gnu11 --c-long-double=128 */
/* linux-only */
/* gnu_ldouble.c with long double binary128 on every target that has an
   ABI for it (on x86-64 too). */
#include "gnu_ldouble.c"

/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: -1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 2 */
/* expect: 0 */
/* expect: 1 */
/* expect: 2 */
/* expect: 3 */
/* expect: 4 */
/* expect: 1 */
/* expect: 2 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
