/* flags: --c-std=c89 */
/* A program that defines a library function itself uses its own (the
   library's is not linked in: no second definition), and the library's
   functions it uses call it too (strcat calls strlen). In C89 mode. */
#include "host.h"
#include <string.h>

static int calls;

size_t strlen(const char *s) {
    size_t n = 0;
    calls++;
    while (s[n]) {
        n++;
    }
    return n;
}

int main(void) {
    char buf[16];
    strcpy(buf, "ab");
    strcat(buf, "cd");
    print_i64((long)strlen(buf));
    print_i64(calls);
    print_i64(memcmp(buf, "abcd", 5));
    return 0;
}
/* expect: 4 */
/* expect: 2 */
/* expect: 0 */
