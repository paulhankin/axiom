/* --c-lib=system with --c-gnu-compat (axiom cc's default): the GNU
   compilers' macros are defined, so the system's headers take their GNU
   paths: functions renamed by asm labels (_FILE_OFFSET_BITS=64: scandir
   and alphasort with their 64-bit names, the program's struct dirent
   their type), the compiler's <limits.h> giving the limits the system's
   leaves to it, the extern inline wrappers of _FORTIFY_SOURCE (with the
   object-size builtins; those that forward their variable arguments,
   printf's and open's, left to the library's functions), _FloatN as the
   dialect's types, and the program's own tests of __GNUC__. */
/* flags: --c-lib=system --c-std=gnu17 --c-gnu-compat -O2 -D_FILE_OFFSET_BITS=64 -D_FORTIFY_SOURCE=2 -lm */
#define _GNU_SOURCE
#include <dirent.h>
#include <error.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static int none(const struct dirent *d) { (void)d; return 0; }

int main(void) {
#if defined(__GNUC__) && __GNUC__ == 9 && __GNUC_MINOR__ == 4 && defined(__GNUC_STDC_INLINE__)
    puts("gnu 9.4"); /* expect: gnu 9.4 */
#endif
    printf("%d %d %d\n", CHAR_BIT, SCHAR_MIN, INT_MAX == 2147483647); /* expect: 8 -128 1 */
    struct dirent **list;
    int n = scandir(".", &list, none, alphasort);
    printf("%d\n", n); /* expect: 0 */
    char buf[16];
    memcpy(buf, "fortified", 10);
    strcat(buf, "!");
    snprintf(buf + 10, sizeof buf - 10, "%d", 42);
    printf("%s %zu\n", buf, __builtin_object_size(buf, 1)); /* expect: fortified!42 16 */
    int fd = open(".", O_RDONLY);
    printf("%d %d\n", fd >= 0, close(fd)); /* expect: 1 0 */
    error(0, 0, "%s", "to stderr");
    _Float128 q = strtof128("0.5", 0);
    _Float64 d = sqrtf64(4.0);
    printf("%g %g %d\n", (double)q, (double)d, isinf(HUGE_VAL_F128)); /* expect: 0.5 2 1 */
    return 0;
}
