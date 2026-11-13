/* A shared library of the system's toolchain (-L. -lso), and the system C
   library's POSIX functions: the program is linked dynamically. The
   library's data is referred to directly from position-dependent code (a
   copy relocation) and through the GOT, its thread-locals through the
   initial-exec model (relaxed from the others); errno is one object for
   both C libraries. */
/* cc-variants: -O2 | -O2 -fno-pie | -O2 -fPIC */
#include <stdio.h>
#include <errno.h>

int sys_posix(void);
int sys_lib(void);

int main(void) {
    int ok = sys_posix();
    printf("%d %d\n", ok, errno == EBADF); /* expect: 1 1 */
    printf("%d\n", sys_lib()); /* expect: 14299 */
    return 0;
}
