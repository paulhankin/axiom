/* --c-lib=system with --c-gnu-compat (axiom cc's default): the GNU
   compilers' macros are defined, so the system's headers take their GNU
   paths: functions renamed by asm labels (_FILE_OFFSET_BITS=64: scandir
   and alphasort with their 64-bit names, the program's struct dirent
   their type), the compiler's <limits.h> giving the limits the system's
   leaves to it, the extern inline wrappers of _FORTIFY_SOURCE (with the
   object-size builtins), and the program's own tests of __GNUC__. */
/* flags: --c-lib=system --c-std=gnu17 --c-gnu-compat -O2 -D_FILE_OFFSET_BITS=64 -D_FORTIFY_SOURCE=2 */
#define _DEFAULT_SOURCE
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static int none(const struct dirent *d) { (void)d; return 0; }

int main(void) {
#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ == 2 && defined(__GNUC_STDC_INLINE__)
    puts("gnu 4.2"); /* expect: gnu 4.2 */
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
    return 0;
}
