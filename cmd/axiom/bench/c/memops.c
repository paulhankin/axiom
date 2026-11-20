/* memcpy/memset/memcmp/memmove-heavy code: small fixed and variable sizes, and page-sized blocks. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
int main(void) {
    size_t n = 1 << 16;
    uint8_t *a = malloc(n + 64), *b = malloc(n + 64);
    for (size_t i = 0; i < n + 64; i++) a[i] = (uint8_t)(i * 31);
    uint64_t h = 0; uint32_t s = 5;
    for (int r = 0; r < 16000000; r++) {
        s = s * 1103515245 + 12345;
        size_t off = (s >> 8) & (n - 1), len = (s >> 4) & 63;
        uint64_t w; memcpy(&w, a + off, 8); h += w;
        memcpy(b + ((off * 7) & (n - 1)), a + off, len);
        if (memcmp(a + off, b + 32, 4) == 0) h++;
        memset(b + (off ^ 0x55), (int)r, 13);
    }
    for (int r = 0; r < 8000; r++) {
        memcpy(b, a, 16384); memset(a + 8192, r, 4096); memmove(a + 1, a, 8192);
        h += b[r & 16383] + a[(r * 3) & 16383];
    }
    for (size_t i = 0; i < n; i++) h += b[i];
    printf("memops: %llu\n", (unsigned long long)h);
    return 0;
}
