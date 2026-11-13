/* __fprintf_chk (the system interface of Linux programs): fprintf of a fortified build.
   An object compiled against the system C library's headers refers to it
   (docs/c.md, "Objects and libraries of other compilers"): defined here so
   that the program's C library is this one (--c-lib=axiom). The interface
   is the Linux Standard Base's (Core specification, the generic and the
   architecture parts); nothing of the system library's own layout is
   assumed beyond what that specification states. */
#include <stdarg.h>
#include <stdio.h>

int __fprintf_chk(FILE *stream, int flag, const char *format, ...) {
    va_list ap;
    int r;
    (void)flag;
    va_start(ap, format);
    r = vfprintf(stream, format, ap);
    va_end(ap);
    return r;
}
