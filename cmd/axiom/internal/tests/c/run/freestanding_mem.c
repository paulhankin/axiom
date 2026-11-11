/* A program that defines memcpy, memmove, memset, memcmp and strlen itself,
   as freestanding programs do (the names are not reserved without the C
   library), and copies structures (which the backends may do with them). */
#include "host.h"
static int calls;
void *memcpy(void *d, const void *s, size_t n) {
    char *dd = d; const char *ss = s;
    calls++;
    while (n--) *dd++ = *ss++;
    return d;
}
void *memmove(void *d, const void *s, size_t n) {
    char *dd = d; const char *ss = s;
    if (dd < ss)
        while (n--) *dd++ = *ss++;
    else
        while (n--) dd[n] = ss[n];
    return d;
}
void *memset(void *d, int c, size_t n) {
    unsigned char *p = d;
    while (n--) *p++ = (unsigned char)c;
    return d;
}
int memcmp(const void *a, const void *b, size_t n) {
    const unsigned char *x = a, *y = b;
    for (; n; n--, x++, y++)
        if (*x != *y) return *x < *y ? -1 : 1;
    return 0;
}
size_t strlen(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n + 1000;  /* (not the C library's) */
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
    memmove(a + 1, a, 10);
    a[0] = 'a';
    print_i64(memcmp(a, b, 64) < 0);
    print_i64((long long)strlen("four"));
    print_i64(calls >= 1);
    return 0;
}

/* expect: 123 */
/* expect: 1 */
/* expect: 1004 */
/* expect: 1 */
