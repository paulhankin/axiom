/* __errno_location (the system interface of Linux programs): the address of errno (the system library's own when it is in the program too: std/c/rt/core_errno.ax).
   An object compiled against the system C library's headers refers to it
   (docs/c.md, "Objects and libraries of other compilers"): defined here so
   that the program's C library is this one (--c-lib=axiom). The interface
   is the Linux Standard Base's (Core specification, the generic and the
   architecture parts); nothing of the system library's own layout is
   assumed beyond what that specification states. */
#include <errno.h>

int *__errno_location(void) {
    return __axc_errno();
}
