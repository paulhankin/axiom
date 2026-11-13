/* __fread_chk (the system interface of Linux programs): fread of a fortified build (ptrlen: the object's size).
   An object compiled against the system C library's headers refers to it
   (docs/c.md, "Objects and libraries of other compilers"): defined here so
   that the program's C library is this one (--c-lib=axiom). The interface
   is the Linux Standard Base's (Core specification, the generic and the
   architecture parts); nothing of the system library's own layout is
   assumed beyond what that specification states. */
#include <stddef.h>
#include <stdio.h>

void __chk_fail(void);

size_t __fread_chk(void *ptr, size_t ptrlen, size_t size, size_t n, FILE *stream) {
    if (size != 0 && n > ptrlen / size)
        __chk_fail();
    return fread(ptr, size, n, stream);
}
