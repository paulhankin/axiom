/* Sorting and searching big inputs: 200000 records from a linear
   congruential generator, sorted by qsort with a key of three fields and
   checked in order, the same keys searched by bsearch (each found, and as
   many misses not found), strings made with snprintf and sorted by strcmp,
   a stable merge sort written here compared with qsort's order where the
   keys are distinct, and the checksums of the orders. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct rec {
    unsigned key;
    int group;
    double weight;
    char name[12];
};

static unsigned long long seed = 12345;

static unsigned next(void) {
    seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
    return (unsigned)(seed >> 33);
}

static int cmp_rec(const void *a, const void *b) {
    const struct rec *x = a, *y = b;
    if (x->group != y->group)
        return x->group < y->group ? -1 : 1;
    if (x->key != y->key)
        return x->key < y->key ? -1 : 1;
    return strcmp(x->name, y->name);
}

static int cmp_uint(const void *a, const void *b) {
    unsigned x = *(const unsigned *)a, y = *(const unsigned *)b;
    return (x > y) - (x < y);
}

static int cmp_str(const void *a, const void *b) {
    return strcmp(*(char *const *)a, *(char *const *)b);
}

static void merge_sort(unsigned *a, unsigned *tmp, size_t n) {
    size_t m, i, j, k;
    if (n < 2)
        return;
    m = n / 2;
    merge_sort(a, tmp, m);
    merge_sort(a + m, tmp, n - m);
    for (i = 0, j = m, k = 0; k < n; k++)
        tmp[k] = (j >= n || (i < m && a[i] <= a[j])) ? a[i++] : a[j++];
    memcpy(a, tmp, n * sizeof *a);
}

int main(void) {
    enum { N = 200000 };
    struct rec *r = malloc(N * sizeof *r);
    unsigned *keys = malloc(N * sizeof *keys), *copy = malloc(N * sizeof *copy), *tmp = malloc(N * sizeof *tmp);
    char **names = malloc(20000 * sizeof *names);
    size_t i, found = 0, missed = 0, bad = 0;
    unsigned long long sum = 0;
    for (i = 0; i < N; i++) {
        r[i].key = next() % 1000000;
        r[i].group = (int)(next() % 17) - 8;
        r[i].weight = (next() % 10000) / 100.0;
        snprintf(r[i].name, sizeof r[i].name, "n%07u", next() % 9999999);
    }
    qsort(r, N, sizeof *r, cmp_rec);
    for (i = 1; i < N; i++)
        if (cmp_rec(&r[i - 1], &r[i]) > 0)
            bad++;
    for (i = 0; i < N; i++)
        sum = sum * 31 + r[i].key + (unsigned)r[i].group;
    printf("records: %d out of order, checksum %llu\n", (int)bad, sum);
    printf("first %d %u %s %.2f, last %d %u %s %.2f\n", r[0].group, r[0].key, r[0].name, r[0].weight, r[N - 1].group,
           r[N - 1].key, r[N - 1].name, r[N - 1].weight);
    /* distinct keys: qsort, bsearch, and a merge sort agree */
    for (i = 0; i < N; i++)
        keys[i] = (unsigned)(i * 2654435761u) | 1u;
    memcpy(copy, keys, N * sizeof *keys);
    qsort(keys, N, sizeof *keys, cmp_uint);
    merge_sort(copy, tmp, N);
    printf("merge sort agrees: %d\n", memcmp(keys, copy, N * sizeof *keys) == 0);
    for (i = 0; i < N; i += 7) {
        unsigned k = (unsigned)(i * 2654435761u) | 1u, miss = k & ~1u;
        found += bsearch(&k, keys, N, sizeof *keys, cmp_uint) != NULL;
        missed += bsearch(&miss, keys, N, sizeof *keys, cmp_uint) == NULL;
    }
    printf("bsearch: %d found, %d missed of %d\n", (int)found, (int)missed, (N + 6) / 7);
    /* strings */
    for (i = 0; i < 20000; i++) {
        char buf[32];
        unsigned hi = next(), lo = next() % 100; /* (in this order: arguments have none) */
        int len = snprintf(buf, sizeof buf, "%x-%u", hi, lo);
        names[i] = malloc((size_t)len + 1);
        memcpy(names[i], buf, (size_t)len + 1);
    }
    qsort(names, 20000, sizeof *names, cmp_str);
    bad = 0;
    for (i = 1; i < 20000; i++)
        bad += strcmp(names[i - 1], names[i]) > 0;
    printf("strings: %d out of order; %s .. %s\n", (int)bad, names[0], names[19999]);
    for (i = 0; i < 20000; i++)
        free(names[i]);
    free(names);
    free(r);
    free(keys);
    free(copy);
    free(tmp);
    return 0;
}

// expect: records: 0 out of order, checksum 10786236407421479628
// expect: first -8 102 n1236797 70.88, last 8 999811 n8926919 8.88
// expect: merge sort agrees: 1
// expect: bsearch: 28572 found, 28572 missed of 28572
// expect: strings: 0 out of order; 100008bf-70 .. ffbbd9d-74
