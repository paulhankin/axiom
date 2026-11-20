#include <stdio.h>
/* a nonnull parameter tested: -fno-delete-null-pointer-checks keeps the test */
__attribute__((nonnull)) __attribute__((noinline)) int len(const char *p) {
    if (!p) return -1;
    int n = 0;
    while (p[n]) n++;
    return n;
}
int (*volatile fp)(const char *) = len;
int main(void) { printf("%d\n", fp(0)); return 0; }
