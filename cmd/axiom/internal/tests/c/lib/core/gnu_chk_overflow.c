/* An object-size-checking builtin whose object is too small ends the
   program as the C library's checked functions do (__chk_fail: a message,
   then abort). */
/* flags: --c-std=gnu17 */
/* expect-signal: 6 */
/* expect-stderr: buffer overflow detected */
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    (void)argv;
    char a[4];
    const char *src = argc > 0 ? "much too long" : "";
    puts("before");
    fflush(stdout);
    __builtin___strcpy_chk(a, src, __builtin_object_size(a, 1));
    puts(a);
    return 0;
}
/* expect: before */
