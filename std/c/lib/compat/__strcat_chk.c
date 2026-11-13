/* __strcat_chk (the system interface of Linux programs): strcat of a fortified build.
   An object compiled against the system C library's headers refers to it
   (docs/c.md, "Objects and libraries of other compilers"): defined here so
   that the program's C library is this one (--c-lib=axiom). The interface
   is the Linux Standard Base's (Core specification, the generic and the
   architecture parts); nothing of the system library's own layout is
   assumed beyond what that specification states. */
#include <stddef.h>
#include <string.h>

void __chk_fail(void);

char *__strcat_chk(char *dest, const char *src, size_t destlen) {
    size_t d = strlen(dest), n = strlen(src);
    if (d + n >= destlen)
        __chk_fail();
    memcpy(dest + d, src, n + 1);
    return dest;
}
