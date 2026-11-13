/* A static archive of the system's toolchain (-L. -larch): the members that
   define what the program needs are linked, and what those need in turn
   (another member), but not the others (one of which refers to a function
   nothing defines). */
#include <stdio.h>

int arch_top(int x);

int main(void) {
    printf("%d\n", arch_top(4)); /* expect: 49 */
    return 0;
}
