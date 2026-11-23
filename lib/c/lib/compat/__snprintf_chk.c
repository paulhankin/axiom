/* __snprintf_chk (the system interface of Linux programs): snprintf of a fortified build: maxlen must not exceed the object's size.
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

int __snprintf_chk(char *s, size_t maxlen, int flag, size_t slen, const char *format, ...) {
    va_list ap;
    int r;
    (void)flag;
    if (maxlen > slen)
        __chk_fail();
    va_start(ap, format);
    r = vsnprintf(s, maxlen, format, ap);
    va_end(ap);
    return r;
}
