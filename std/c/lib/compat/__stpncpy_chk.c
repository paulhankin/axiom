/* __stpncpy_chk (the system interface of Linux programs): stpncpy with the
   destination's size checked: what the object-size builtin
   __builtin___stpncpy_chk calls (docs/c.md, "Objects and libraries of other
   compilers"). The interface is the Linux Standard Base's. */
#include <stddef.h>
#include <string.h>

void __chk_fail(void);

/* stpncpy (POSIX): n bytes written, src's then 0s; the end of src's copy,
   or dest + n if src is not shorter than n */
char *__stpncpy_chk(char *dest, const char *src, size_t n, size_t destlen) {
    size_t k = 0;
    if (n > destlen)
        __chk_fail();
    while (k < n && src[k] != 0) {
        dest[k] = src[k];
        k++;
    }
    char *end = dest + k;
    while (k < n)
        dest[k++] = 0;
    return end;
}
