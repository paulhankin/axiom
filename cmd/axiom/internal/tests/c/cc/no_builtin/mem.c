/* The program's own memory functions, in a unit of their own. */
#include <stddef.h>

int calls;

void *memcpy(void *d, const void *s, size_t n) {
    calls += 1;
    char *dd = d;
    const char *ss = s;
    while (n--) *dd++ = *ss++;
    return d;
}

void *memset(void *d, int c, size_t n) {
    calls += 10;
    char *dd = d;
    while (n--) *dd++ = (char)c;
    return d;
}

void *memmove(void *d, const void *s, size_t n) {
    calls += 100;
    char *dd = d;
    const char *ss = s;
    if (dd < ss) {
        while (n--) *dd++ = *ss++;
    } else {
        while (n--) dd[n] = ss[n];
    }
    return d;
}

int memcmp(const void *a, const void *b, size_t n) {
    calls += 1000;
    const unsigned char *x = a, *y = b;
    for (; n; n--, x++, y++) {
        if (*x != *y) return *x - *y;
    }
    return 0;
}
