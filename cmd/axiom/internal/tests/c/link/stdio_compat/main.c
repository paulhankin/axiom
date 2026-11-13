/* An object of the system's C compiler, compiled against the system C
   library's headers, uses the program's C library (the compiler's: there
   is one, whose streams both write in order): printf and the fortified
   functions its headers choose (_FORTIFY_SOURCE), stdout as an object,
   errno, the <ctype.h> macros' tables, assert, qsort with a callback,
   setjmp and longjmp, and stack protection. */
/* cc-variants: -O2 | -O2 -D_FORTIFY_SOURCE=2 -fstack-protector-all | -O0 -fPIC | -O2 -fno-pie */
#include <stdio.h>

void sys_print(const char *s);
int sys_errno(void);
int sys_ctype(const char *s);
int sys_sort(void);
int sys_jump(int v);
void sys_assert(int x);

int main(void) {
    printf("main first\n"); /* expect: main first */
    sys_print("hello"); /* expect: <hello> 7 */
    printf("main again\n"); /* expect: main again */
    printf("%d\n", sys_errno()); /* expect: 1 */
    printf("%d\n", sys_ctype("aB3x Y9-")); /* expect: 4 */
    printf("%d\n", sys_sort()); /* expect: 13459 */
    printf("%d\n", sys_jump(7)); /* expect: 7 */
    sys_assert(1);
    return 0;
}
