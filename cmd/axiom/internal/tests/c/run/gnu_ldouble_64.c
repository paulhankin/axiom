/* flags: --c-std=gnu11 --c-long-double=64 */
/* gnu_ldouble.c with long double as double, on every target: the
   builtins are the IR's operations, as for double. */
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
