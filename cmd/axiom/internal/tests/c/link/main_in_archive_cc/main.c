/* main in a member of an archive of the system's toolchain (-larch), and
   the program of objects alone: this compiler's object, which the member
   calls, and the archive. Members that nothing needs are not linked. */
/* main-object: ours */
/* flag-variants: -O2 | -O2 --c-lib=system */
#include <stdio.h>

int lib_input(void) { return 6; }

void lib_report(int x) { printf("report %d", x); }
/* expect: report 36 */
/* expect-status: 4 */
