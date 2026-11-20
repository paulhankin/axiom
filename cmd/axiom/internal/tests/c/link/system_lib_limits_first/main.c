/* --c-lib=system with the GNU compilers' macros: a program (a library's
   header) that defines one of the limits before <limits.h> keeps it, and
   the compiler's <limits.h> gives the others the system's leaves to it. */
/* flags: --c-lib=system --c-std=gnu17 --c-gnu-compat */
#ifndef CHAR_MAX
#define CHAR_MAX 127
#endif
#include <limits.h>
#include <stdio.h>
int main(void) {
    printf("%d %d %d %d\n", CHAR_MAX, CHAR_MIN, UCHAR_MAX, SHRT_MIN); /* expect: 127 -128 255 -32768 */
    return 0;
}
