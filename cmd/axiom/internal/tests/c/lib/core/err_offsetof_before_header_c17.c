/* flags: --c-std=c17 */
/* expect-error: C1060 */
/* Under an ISO standard a program's definition of offsetof before <stddef.h>
   (undefined behavior, 7.1.3p2) is a redefinition. */
#define offsetof(type, field) ((unsigned long) & ((type *)0)->field)
#include <stddef.h>
int main(void) { return 0; }
