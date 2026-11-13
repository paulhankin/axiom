/* --c-lib=system: the system's C library is the program's, its headers the
   hosted headers: the program's code and an object of the system's C
   compiler use its streams (one order of output), its errno, its setjmp;
   and the object may call its POSIX functions on them (fileno). */
/* flags: --c-lib=system --c-std=gnu17 */
/* cc-variants: -O2 | -O2 -D_FORTIFY_SOURCE=2 -fstack-protector-all | -O0 -fno-pie */
#include <errno.h>
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int sys_fd(void);
void sys_print(const char *s);

static jmp_buf jb;

int main(void) {
    printf("main\n"); /* expect: main */
    sys_print("sys"); /* expect: sys */
    printf("%d\n", sys_fd()); /* expect: 1 */
    errno = 0;
    FILE *f = fopen("/nonexistent/x", "r");
    printf("%d %d\n", f == NULL, errno == ENOENT); /* expect: 1 1 */
    printf("%s\n", strerror(ENOENT)); /* expect: No such file or directory */
    if (setjmp(jb) == 0)
        longjmp(jb, 3);
    printf("%.4f %d\n", sqrt(2.0), getpid() > 0); /* expect: 1.4142 1 */
    return 0;
}
