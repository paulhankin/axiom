/* Sorting: a hand-written quicksort with insertion sort, and qsort with a comparator. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
static void isort(int32_t *a, int n) {
    for (int i = 1; i < n; i++) {
        int32_t x = a[i]; int j = i;
        while (j > 0 && a[j - 1] > x) { a[j] = a[j - 1]; j--; }
        a[j] = x;
    }
}
static void qs(int32_t *a, int n) {
    while (n > 24) {
        int32_t p = a[n / 2];
        int i = 0, j = n - 1;
        for (;;) {
            while (a[i] < p) i++;
            while (a[j] > p) j--;
            if (i >= j) break;
            int32_t t = a[i]; a[i] = a[j]; a[j] = t; i++; j--;
        }
        qs(a, j + 1);
        a += j + 1; n -= j + 1;
    }
    isort(a, n);
}
static int cmp(const void *x, const void *y) {
    uint32_t a = *(const uint32_t *)x, b = *(const uint32_t *)y;
    return (a > b) - (a < b);
}
int main(void) {
    int n = 1500000;
    int32_t *a = malloc(n * sizeof *a);
    uint32_t s = 2463534242u;
    uint64_t sum = 0;
    for (int r = 0; r < 3; r++) {
        for (int i = 0; i < n; i++) { s ^= s << 13; s ^= s >> 17; s ^= s << 5; a[i] = (int32_t)s; }
        qs(a, n);
        for (int i = 1; i < n; i++) if (a[i - 1] > a[i]) return 1;
        sum += (uint32_t)a[n / 3];
    }
    for (int i = 0; i < n; i++) { s ^= s << 13; s ^= s >> 17; s ^= s << 5; a[i] = (int32_t)s; }
    qsort(a, n / 2, sizeof *a, cmp);
    sum += (uint32_t)a[n / 5];
    printf("sort: %llu\n", (unsigned long long)sum);
    return 0;
}
