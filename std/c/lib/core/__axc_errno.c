/* __axc_errno (<errno.h>): the address of errno's storage, which is each
   thread's own (C11 7.5p2: errno has thread storage duration): the
   system C library's errno where that library is in the program too
   (std/c/rt/core_errno.ax), so that the two libraries' functions set one
   object, else the library's own. */
#include <errno.h>

int *__axc_sys_errno(void);

static _Thread_local int value;

int *__axc_errno(void) {
    int *p = __axc_sys_errno();
    return p ? p : &value;
}
