/* Conversions between pointers and integers (6.3.2.3p5-6): to integer
   types narrower or wider than a pointer and back, and _Bool. */
#include <stdint.h>
#include "host.h"

static char buf[16];

int main(void) {
    char *p = buf + 3;
    uintptr_t u = (uintptr_t)p;
    print_i64((char *)u == p);
    print_i64((unsigned char)(uintptr_t)p == (unsigned char)u);
    unsigned long long w = (unsigned long long)p;
    print_i64((char *)(uintptr_t)w == p);
    print_i64((char)p == (char)u);
    print_i64((_Bool)p);
    print_i64((_Bool)(char *)0);
    print_i64((uintptr_t)(void *)0 == 0);
    print_i64((intptr_t)(p + 1) - (intptr_t)p);
    void *q = (void *)(uintptr_t)(unsigned char)7;
    print_i64((uintptr_t)q);
    return 0;
}

/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 7 */
