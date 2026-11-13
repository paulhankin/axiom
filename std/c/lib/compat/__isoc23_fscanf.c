/* __isoc23_fscanf (the system interface of Linux programs): C23's scanf (%b and 0b: the system's headers name it so in C23).
   An object compiled against the system C library's headers refers to it
   (docs/c.md, "Objects and libraries of other compilers"): defined here so
   that the program's C library is this one (--c-lib=axiom). The interface
   is the Linux Standard Base's (Core specification, the generic and the
   architecture parts); nothing of the system library's own layout is
   assumed beyond what that specification states. */
#include <stdarg.h>
#include <stdio.h>

int __axc_vscanf_c23(const char *format, va_list ap);
int __axc_vfscanf_c23(FILE *stream, const char *format, va_list ap);
int __axc_vsscanf_c23(const char *s, const char *format, va_list ap);

int __isoc23_fscanf(FILE *stream, const char *format, ...) {
    va_list ap;
    int r;
    va_start(ap, format);
    r = __axc_vfscanf_c23(stream, format, ap);
    va_end(ap);
    return r;
}
