/* __sprintf_chk (the system interface of Linux programs): sprintf of a fortified build: it fails rather than write past the object.
   An object compiled against the system C library's headers refers to it
   (docs/c.md, "Objects and libraries of other compilers"): defined here so
   that the program's C library is this one (--c-lib=axiom). The interface
   is the Linux Standard Base's (Core specification, the generic and the
   architecture parts); nothing of the system library's own layout is
   assumed beyond what that specification states. */
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

void __chk_fail(void);

/* slen: the size of the object s points to; writing past it fails */
int __sprintf_chk(char *s, int flag, size_t slen, const char *format, ...) {
    va_list ap;
    int r;
    (void)flag;
    va_start(ap, format);
    r = vsnprintf(s, slen, format, ap);
    va_end(ap);
    if (r >= 0 && (size_t)r >= slen)
        __chk_fail();
    return r;
}
