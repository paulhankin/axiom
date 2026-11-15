/* --c-lib=system: the program's own redefinition of a system header's macro
   stays an error (only the system's headers replace one another's). */
/* flags: --target=x86_64 --c-lib=system --c-system-include=sysredef */
/* expect-error: C1060 */
#include <first.h>
#define WIDTH_MAX 255
int w = WIDTH_MAX;
