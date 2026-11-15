/* main in a member of an archive of this compiler (-lours), the program's
   other code a source: the start-up's call of main takes the member, as
   the platform's start-up does. */
#include <stdio.h>

int lib_input(void) { return 5; }

void lib_report(int x) { printf("report %d\n", x); }
/* expect: report 25 */
/* expect-status: 3 */
