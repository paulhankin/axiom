/* flags: --c-std=gnu17 */
/* The GNU dialects' __builtin_ forms of more C library functions: calls of
   the library's function of the name (fprintf, fputs, fputc, putc, fwrite,
   strcspn, strspn, strpbrk, imaxabs, sscanf). */
#include <stdio.h>
#include "print.h"

static const char hello[] = "hello";

int main(void) {
    int n = 0;
    __builtin_fputs("a", stdout);
    __builtin_fputc('b', stdout);
    __builtin_putc('c', stdout);
    __builtin_fwrite("d", 1, 1, stdout);
    __builtin_fprintf(stdout, "%d\n", 5); /* expect: abcd5 */
    fflush(stdout);
    print_i64(__builtin_strcspn("hello", "l") * 100 + __builtin_strspn("hello", "he") * 10 +
              (__builtin_strpbrk(hello, "lo") - hello == 2 ? 0 : 1)); /* expect: 220 */
    print_i64(__builtin_imaxabs(-7)); /* expect: 7 */
    __builtin_sscanf("42", "%d", &n);
    print_i64(n); /* expect: 42 */
    return 0;
}
