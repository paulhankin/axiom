/* The object-size-checking builtins of the GNU dialects
   (__builtin___memcpy_chk ...), which the system's headers call for
   _FORTIFY_SOURCE: with the object's size (__builtin_object_size's), each
   does its function's work; with (size_t)-1 (a size not known) it is the
   function itself. */
/* flags: --c-std=gnu17 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define BOS(p) __builtin_object_size(p, 1)

static int vs(char *d, size_t n, const char *f, ...) {
    va_list ap;
    va_start(ap, f);
    int r = __builtin___vsnprintf_chk(d, n, 1, BOS(d), f, ap);
    va_end(ap);
    return r;
}
static int vs2(char *d, const char *f, ...) {
    va_list ap;
    va_start(ap, f);
    int r = __builtin___vsprintf_chk(d, 1, (size_t)-1, f, ap);
    va_end(ap);
    return r;
}

int main(void) {
    char a[32], b[32];
    char *p = __builtin___memcpy_chk(a, "hello", 6, BOS(a));
    __builtin___memmove_chk(a + 1, a, 5, BOS(a + 1));
    printf("%d %s\n", p == a, a);
    char *e = __builtin___mempcpy_chk(b, "abc", 3, BOS(b));
    *e = 0;
    __builtin___memset_chk(b + 3, 'x', 2, (size_t)-1);
    b[5] = 0;
    printf("%s %d\n", b, (int)(e - b));
    __builtin___strcpy_chk(a, "one", BOS(a));
    __builtin___strcat_chk(a, "two", BOS(a));
    __builtin___strncat_chk(a, "threeXX", 5, (size_t)-1);
    char *s = __builtin___stpcpy_chk(b, "four", BOS(b));
    printf("%s %s %d\n", a, b, (int)(s - b));
    __builtin___strncpy_chk(b, "fi", 4, BOS(b));
    char *t = __builtin___stpncpy_chk(a, "sixty", 3, BOS(a));
    printf("%s %d %d %d\n", b, b[3], (int)(t - a), a[2]);
    int n = __builtin___sprintf_chk(a, 1, BOS(a), "%d-%s", 7, "x");
    int m = __builtin___snprintf_chk(b, 4, 1, BOS(b), "%d", 123456);
    printf("%s %d %s %d\n", a, n, b, m);
    vs(a, sizeof a, "%s/%d", "v", 8);
    vs2(b, "%c%c", 'o', 'k');
    __builtin___printf_chk(1, "%s %s\n", a, b);
    __builtin___fprintf_chk(stdout, 1, "%d\n", 42);
    return 0;
}
/* expect: 1 hhello */
/* expect: abcxx 3 */
/* expect: onetwothree four 4 */
/* expect: fi 0 3 120 */
/* expect: 7-x 3 123 6 */
/* expect: v/8 ok */
/* expect: 42 */
