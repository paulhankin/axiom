/* raise of a signal whose action is the default (C17 7.14.1.1p4): the
   process ends abnormally (killed by the signal on Linux and macOS, with
   status 128 + 15 elsewhere). */
/* expect-signal: 15 */
#include <signal.h>
#include "print.h"

int main(void) {
    print_cstr("raise"); /* expect: raise */
    raise(SIGTERM);
    print_cstr("not reached");
    return 0;
}
