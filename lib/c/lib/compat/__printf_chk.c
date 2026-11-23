/* __printf_chk (the system interface of Linux programs): printf with the checks of a fortified build (flag: their level, which this one's checks do not need).
   An object compiled against the system C library's headers refers to it
   (docs/c.md, "Objects and libraries of other compilers"): defined here so
   that the program's C library is this one (--c-lib=axiom). The interface
   is the Linux Standard Base's (Core specification, the generic and the
   architecture parts); nothing of the system library's own layout is
   assumed beyond what that specification states. */
#include <stdarg.h>
#include <stdio.h>

int __printf_chk(int flag, const char *format, ...) {
    va_list ap;
    int r;
    (void)flag;
    va_start(ap, format);
    r = vprintf(format, ap);
    va_end(ap);
    return r;
}
