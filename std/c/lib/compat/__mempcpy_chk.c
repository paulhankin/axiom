/* __mempcpy_chk (the system interface of Linux programs): mempcpy, the GNU
   function, with the destination's size checked: what the object-size
   builtin __builtin___mempcpy_chk calls (docs/c.md, "Objects and libraries
   of other compilers"). The interface is the Linux Standard Base's. */
#include <stddef.h>
#include <string.h>

void __chk_fail(void);

/* mempcpy: the end of the copy */
void *__mempcpy_chk(void *dest, const void *src, size_t n, size_t destlen) {
    if (n > destlen)
        __chk_fail();
    memcpy(dest, src, n);
    return (char *)dest + n;
}
