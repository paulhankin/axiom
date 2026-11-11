/* A program that defines memcpy and memset itself, as freestanding programs
   do, and copies structures (which the backends may do with them). */
#include "host.h"
void *memcpy(void *d, const void *s, size_t n) {
    char *dd = d; const char *ss = s;
    while (n--) *dd++ = *ss++;
    return d;
}
void *memset(void *d, int c, size_t n) {
    unsigned char *p = d;
    while (n--) *p++ = (unsigned char)c;
    return d;
}
struct big { int a[40]; };
static struct big mk(int k) { struct big b = { { k } }; return b; }
int main(void) {
    char a[64], b[64];
    struct big x = mk(3), y;
    memset(a, 'x', 64);
    memcpy(b, a, 64);
    y = x;
    print_i64(b[63] + y.a[0]);
    return 0;
}

/* expect: 123 */
