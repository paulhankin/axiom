/* stdin (the system interface of Linux programs): the standard input stream, as the object of type FILE * that the system's headers declare.
   An object compiled against the system C library's headers refers to it
   (docs/c.md, "Objects and libraries of other compilers"): defined here so
   that the program's C library is this one (--c-lib=axiom). The interface
   is the Linux Standard Base's (Core specification, the generic and the
   architecture parts); nothing of the system library's own layout is
   assumed beyond what that specification states. */
#include <stdio.h>

#undef stdin
FILE *stdin = &__axc_stdin;
