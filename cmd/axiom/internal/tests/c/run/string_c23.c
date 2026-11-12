/* flags: --c-std=c23 */
/* <string.h> in C23: memccpy, memset_explicit, and the search functions
   that are generic (their result points to const when their argument
   does: 7.26.5p1), with the functions themselves still there. */
#include "host.h"
#include <string.h>

int main(void) {
    char dst[10] = "........";
    print_i64((char *)memccpy(dst, "abc;def", ';', 7) - dst);
    print_str(dst, 8);
    print_i64(memccpy(dst, "abc", 'z', 3) == nullptr);
    print_i64(memset_explicit(dst, 'q', 3) == dst);
    print_str(dst, 8);
    const char *cs = "const";
    char ms[] = "mutable";
    static_assert(_Generic(strchr(cs, 'n'), const char *: 1, default: 0));
    static_assert(_Generic(strchr(ms, 'u'), char *: 1, default: 0));
    static_assert(_Generic(strrchr(cs, 'n'), const char *: 1, default: 0));
    static_assert(_Generic(strstr(ms, "ta"), char *: 1, default: 0));
    static_assert(_Generic(strpbrk(cs, "st"), const char *: 1, default: 0));
    static_assert(_Generic(memchr((const void *)cs, 's', 5), const void *: 1, default: 0));
    static_assert(_Generic(memchr(ms, 'b', 7), void *: 1, default: 0));
    print_i64(strchr(cs, 'n') - cs + (strstr(ms, "ta") - ms) * 10);
    char *(*f)(const char *, int) = (strchr);
    print_i64(f(ms, 'b') - ms);
    return 0;
}
/* expect: 4 */
/* expect: abc;.... */
/* expect: 1 */
/* expect: 1 */
/* expect: qqq;.... */
/* expect: 22 */
/* expect: 4 */
