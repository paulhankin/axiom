/* _IO_putc (the system interface of Linux programs): putc.
   An object compiled against the system C library's headers refers to it
   (docs/c.md, "Objects and libraries of other compilers"): defined here so
   that the program's C library is this one (--c-lib=axiom). The interface
   is the Linux Standard Base's (Core specification, the generic and the
   architecture parts); nothing of the system library's own layout is
   assumed beyond what that specification states. */
#include <stdio.h>

int _IO_putc(int c, FILE *stream) {
    return putc(c, stream);
}
