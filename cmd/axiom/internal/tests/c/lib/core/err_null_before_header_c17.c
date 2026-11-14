/* flags: --c-std=c17 */
/* expect-error: C1060 */
/* Under an ISO standard a program's definition of NULL before a header that
   defines it (undefined behavior, 7.1.3p2) is a redefinition. */
#define NULL 0
#include <stddef.h>
int main(void) { return 0; }
