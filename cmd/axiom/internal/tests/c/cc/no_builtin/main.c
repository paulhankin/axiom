#include <stdio.h>
#include <string.h>

extern int calls;

int main(void) {
    char a[40] = "hello", b[40];
    volatile size_t n = 6;
    memcpy(b, a, n);
    memset(b + 10, 1, n);
    memmove(b + 20, b, n);
    int c = memcmp(a, b, 4);
    printf("%s %d %d\n", b + 20, c, calls);
    return 0;
}
