/* The <string.h> calls a hosted program's code makes without calling (the
   compiler's memory operations, and memcmp of 1, 2, 4 or 8 bytes as two
   loads): memcpy, memmove and memset of constant and run-time sizes,
   overlapping either way, and memcmp of constant sizes at every alignment,
   against byte-by-byte references; the mismatches (none). */
#include <string.h>
#include "print.h"

static unsigned char a[400], b[400], ref[400];
static unsigned seed = 1;

static unsigned rnd(void) {
    seed = seed * 1103515245u + 12345u;
    return (seed >> 16) & 0x7fff;
}

static int sign(int x) { return (x > 0) - (x < 0); }

static int want_cmp(const unsigned char *x, const unsigned char *y, size_t n) {
    for (size_t i = 0; i < n; i++)
        if (x[i] != y[i])
            return x[i] < y[i] ? -1 : 1;
    return 0;
}

int main(void) {
    int bad = 0;
    for (int k = 0; k < 3000; k++) {
        size_t n = rnd() % ((k & 1) ? 300 : 40), i = rnd() % 64, j = rnd() % 64;
        for (size_t q = 0; q < 400; q++) {
            a[q] = ref[q] = (unsigned char)(rnd() & 3);
            b[q] = (unsigned char)(rnd() & 3);
        }
        /* memmove within a, either way */
        if (i <= j)
            for (size_t q = 0; q < n; q++) ref[i + q] = ref[j + q];
        else
            for (size_t q = n; q > 0; q--) ref[i + q - 1] = ref[j + q - 1];
        memmove(a + i, a + j, n);
        bad += memcmp(a, ref, 400) != 0;
        /* memcpy from b, memset */
        memcpy(a + j, b + i, n);
        for (size_t q = 0; q < n; q++) ref[j + q] = b[i + q];
        memset(a + i, (int)k, n / 2);
        for (size_t q = 0; q < n / 2; q++) ref[i + q] = (unsigned char)k;
        for (size_t q = 0; q < 400; q++) bad += a[q] != ref[q];
        /* memcmp of the constant sizes */
        bad += sign(memcmp(a + i, b + j, 1)) != want_cmp(a + i, b + j, 1);
        bad += sign(memcmp(a + i, b + j, 2)) != want_cmp(a + i, b + j, 2);
        bad += sign(memcmp(a + i, b + j, 4)) != want_cmp(a + i, b + j, 4);
        bad += sign(memcmp(a + i, b + j, 8)) != want_cmp(a + i, b + j, 8);
        bad += (memcmp(a + j, b + i, 4) == 0) != (want_cmp(a + j, b + i, 4) == 0);
        bad += sign(memcmp(a + i, b + j, 3)) != want_cmp(a + i, b + j, 3);
    }
    /* constant copies of the sizes the inline paths take */
    memcpy(a, b, 8);
    memcpy(a + 9, b + 1, 33);
    memmove(a + 1, a, 64);
    bad += want_cmp(a + 1, b, 8) != 0;
    print_i64(bad); /* expect: 0 */
    print_i64(sign(memcmp("\x80\x01", "\x7f\x02", 2))); /* expect: 1 */
    print_i64(memcmp("ab", "ab", 2)); /* expect: 0 */
    return 0;
}
