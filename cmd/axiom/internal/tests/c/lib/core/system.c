/* system (C17 7.22.4.8) where there is a command processor (/bin/sh): its
   status, as POSIX encodes it (exit status times 256). */
/* posix-only */
#include <stdlib.h>
#include "print.h"

int main(void) {
    print_bool(system(NULL) != 0); /* expect: true */
    print_i64(system("exit 3")); /* expect: 768 */
    print_i64(system("true")); /* expect: 0 */
    print_i64(system("echo from the shell")); /* expect: from the shell */ /* expect: 0 */
    return 0;
}
