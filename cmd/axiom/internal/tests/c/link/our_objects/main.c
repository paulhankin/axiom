/* Objects of this compiler (--emit=obj) and a static library of them
   (--emit=lib, -lours) linked with each other and with a program: each
   has a copy of the runtime's helpers, which the link keeps one of, and
   constructors in its initialization array. The same objects are linked
   by the system's toolchain into a C program (cc_main.c), whose C library
   is then the system's. */
/* ours-flags: --c-std=gnu17 */
#include <stdio.h>

int ours_a(int);
int ours_b(const char *);
int ourlib_x(int);

int main(void) {
    /* expect: ctor b */
    printf("%d\n", ours_a(4)); /* expect: 5 */
    printf("%d\n", ours_b("abc")); /* expect: 3 */
    printf("%d\n", ourlib_x(6)); /* expect: 42 */
    return 0;
}
/* cc-expect: ctor b */
/* cc-expect: 5 3 42 */
