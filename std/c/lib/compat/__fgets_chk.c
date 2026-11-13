/* __fgets_chk (the system interface of Linux programs): fgets of a fortified build (size: the object's).
   An object compiled against the system C library's headers refers to it
   (docs/c.md, "Objects and libraries of other compilers"): defined here so
   that the program's C library is this one (--c-lib=axiom). The interface
   is the Linux Standard Base's (Core specification, the generic and the
   architecture parts); nothing of the system library's own layout is
   assumed beyond what that specification states. */
#include <stddef.h>
#include <stdio.h>

void __chk_fail(void);

char *__fgets_chk(char *s, size_t size, int n, FILE *stream) {
    if (n > 0 && (size_t)n > size)
        __chk_fail();
    return fgets(s, n, stream);
}
