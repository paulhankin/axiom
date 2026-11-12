/* qsort (C17 7.22.5.2): sorted results for elements of several sizes and
   alignments and for the inputs that make simple quicksorts quadratic
   (sorted, reversed, all equal, organ pipe, sawtooth, a median-of-three
   killer), with at most about 3 n log2 n comparisons; and bsearch
   (7.22.5.1) over the result. */
#include <stdlib.h>
#include "print.h"

static long long ncmp;

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    ncmp++;
    return (x > y) - (x < y);
}

struct rec {
    char key[3];
};

static int keycmp(const struct rec *a, const struct rec *b) {
    int i;
    for (i = 0; i < 3; i++)
        if (a->key[i] != b->key[i])
            return (unsigned char)a->key[i] < (unsigned char)b->key[i] ? -1 : 1;
    return 0;
}

static int cmp_rec(const void *a, const void *b) {
    ncmp++;
    return keycmp(a, b);
}

struct big {
    long long k;
    double pad[2];
};

static int cmp_big(const void *a, const void *b) {
    long long x = ((const struct big *)a)->k, y = ((const struct big *)b)->k;
    ncmp++;
    return (x > y) - (x < y);
}

static int a[100000];

static int sorted(int n) {
    int i;
    for (i = 1; i < n; i++)
        if (a[i - 1] > a[i])
            return 0;
    return 1;
}

static long long sum(int n) {
    long long s = 0;
    int i;
    for (i = 0; i < n; i++)
        s += (long long)a[i] * 7 + (a[i] ^ 0x55);
    return s;
}

/* sorts a[0..n) of pattern p; true if sorted, the same elements, and
   within the comparison bound */
static int check(int p, int n) {
    int i;
    long long before, lg = 0, m;
    unsigned seed = 12345;
    for (i = 0; i < n; i++) {
        switch (p) {
        case 0: a[i] = i; break;
        case 1: a[i] = n - i; break;
        case 2: a[i] = 7; break;
        case 3: a[i] = i < n / 2 ? i : n - i; break;
        case 4: a[i] = i % 97; break;
        case 5:
            seed = seed * 1103515245u + 12345u;
            a[i] = (int)(seed >> 8);
            break;
        default:
            /* (an adversary of the median of three: smallest values at the
               ends and the middle, alternately) */
            a[i] = (i % 2) ? i : n - i;
            break;
        }
    }
    before = sum(n);
    ncmp = 0;
    qsort(a, (size_t)n, sizeof a[0], cmp_int);
    for (m = n; m > 1; m >>= 1)
        lg++;
    return sorted(n) && sum(n) == before && ncmp <= 3 * (long long)n * (lg + 1);
}

int main(void) {
    int p, ok = 1, i;
    static struct rec r[1000];
    static struct big b[2000];
    for (p = 0; p <= 6; p++) {
        ok &= check(p, 100000);
        ok &= check(p, 17);
        ok &= check(p, 1000);
    }
    print_bool(ok); /* expect: true */
    a[0] = 3;
    qsort(a, 1, sizeof a[0], cmp_int);
    qsort(a, 0, sizeof a[0], cmp_int);
    print_i64(a[0]); /* expect: 3 */
    /* 3-byte elements (bytes moved one at a time) */
    for (i = 0; i < 1000; i++) {
        r[i].key[0] = (char)('a' + (i * 7) % 26);
        r[i].key[1] = (char)('a' + (i * 13) % 26);
        r[i].key[2] = (char)('0' + i % 10);
    }
    qsort(r, 1000, sizeof r[0], cmp_rec);
    for (i = 1; i < 1000; i++)
        if (keycmp(&r[i - 1], &r[i]) > 0)
            ok = 0;
    print_bool(ok); /* expect: true */
    /* 24-byte elements */
    for (i = 0; i < 2000; i++) {
        b[i].k = (long long)((i * 7919) % 2000) - 1000;
        b[i].pad[0] = (double)b[i].k;
    }
    qsort(b, 2000, sizeof b[0], cmp_big);
    for (i = 0; i < 2000; i++)
        if (b[i].k != i - 1000 || b[i].pad[0] != (double)b[i].k)
            ok = 0;
    print_bool(ok); /* expect: true */
    /* bsearch */
    for (i = 0; i < 1000; i++)
        a[i] = i * 2;
    {
        int key = 500, miss = 501, lo = -1, hi = 1998;
        int *f = bsearch(&key, a, 1000, sizeof a[0], cmp_int);
        print_i64(f ? f - a : -1); /* expect: 250 */
        print_bool(bsearch(&miss, a, 1000, sizeof a[0], cmp_int) == NULL); /* expect: true */
        print_bool(bsearch(&lo, a, 1000, sizeof a[0], cmp_int) == NULL); /* expect: true */
        f = bsearch(&hi, a, 1000, sizeof a[0], cmp_int);
        print_i64(f ? f - a : -1); /* expect: 999 */
        print_bool(bsearch(&key, a, 0, sizeof a[0], cmp_int) == NULL); /* expect: true */
        ncmp = 0;
        for (key = 0; key < 2000; key++)
            ok &= (bsearch(&key, a, 1000, sizeof a[0], cmp_int) != NULL) == (key % 2 == 0);
        print_bool(ok && ncmp <= 2000 * 11); /* expect: true */
    }
    return 0;
}
