/* qsort with comparison functions that are not consistent (undefined by
   C17 7.22.5p4: the order is then unspecified here) touches nothing
   outside the array: guard elements on both sides stay as they were, and
   the array keeps its elements (only their order changes). And with no
   elements (C23 7.24.5p1), qsort and bsearch call no comparison function,
   with null pointers too. */
#include <stdio.h>
#include <stdlib.h>

static unsigned long long state = 1;
static int calls;

static int random_cmp(const void *a, const void *b) {
    (void)a;
    (void)b;
    state = state * 6364136223846793005ULL + 1442695040888963407ULL;
    return (int)(state >> 62) - 1;
}
static int always_greater(const void *a, const void *b) {
    (void)a;
    (void)b;
    return 1;
}
static int always_less(const void *a, const void *b) {
    (void)a;
    (void)b;
    return -1;
}
static int counted(const void *a, const void *b) {
    (void)a;
    (void)b;
    calls++;
    return 0;
}

static int check(int (*cmp)(const void *, const void *), size_t n, size_t size) {
    static unsigned char buf[4096 + 2 * 64];
    unsigned char *a = buf + 64;
    unsigned long sum = 0, after = 0;
    size_t i;
    for (i = 0; i < sizeof buf; i++)
        buf[i] = (unsigned char)(i < 64 || i >= 64 + n * size ? 0xA5 : (i * 7919) % 251);
    for (i = 0; i < n * size; i++)
        sum += a[i];
    qsort(a, n, size, cmp);
    for (i = 0; i < n * size; i++)
        after += a[i];
    for (i = 0; i < sizeof buf; i++)
        if ((i < 64 || i >= 64 + n * size) && buf[i] != 0xA5)
            return 0;
    return sum == after;
}

int main(void) {
    static const size_t sizes[] = {1, 3, 4, 8, 12, 16, 24};
    size_t s, n, ok = 0, total = 0;
    int (*cmps[3])(const void *, const void *) = {random_cmp, always_greater, always_less};
    int c;
    for (c = 0; c < 3; c++)
        for (s = 0; s < sizeof sizes / sizeof sizes[0]; s++)
            for (n = 0; n * sizes[s] <= 4096; n = n < 20 ? n + 1 : n * 3) {
                total++;
                ok += (size_t)check(cmps[c], n, sizes[s]);
            }
    printf("%d of %d\n", (int)ok, (int)total);
    qsort(NULL, 0, 8, counted);
    printf("bsearch %d, calls %d\n", bsearch(&c, NULL, 0, 4, counted) == NULL, calls);
    return 0;
}

// expect: 492 of 492
// expect: bsearch 1, calls 0
