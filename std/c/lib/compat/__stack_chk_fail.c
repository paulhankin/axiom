/* __stack_chk_fail (the system interface of Linux programs): what code compiled with stack protection calls when a function's guard was overwritten: a message, then abort.
   An object compiled against the system C library's headers refers to it
   (docs/c.md, "Objects and libraries of other compilers"): defined here so
   that the program's C library is this one (--c-lib=axiom). The interface
   is the Linux Standard Base's (Core specification, the generic and the
   architecture parts); nothing of the system library's own layout is
   assumed beyond what that specification states. */
#include <stdio.h>
#include <stdlib.h>

void __stack_chk_fail(void) {
    fputs("*** stack smashing detected ***: terminated\n", stderr);
    abort();
}
