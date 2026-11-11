/* flags: --c-std=c99 */
/* Sorting through function pointers: generic algorithms over (base, count,
   size, compare) like a library sort — insertion, shell, heap, merge (top
   down with a buffer), quicksort (median of three, insertion for short
   ranges), and an LSD radix sort for keys — applied to integers, doubles,
   strings and records with several keys, with comparison callbacks that
   count their calls through a context pointer. Every result is checked to
   be sorted and to be a permutation; stable sorts are checked for
   stability. A table of algorithms by name selects them. */
#include "host.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef int (*cmp_fn)(const void *, const void *, void *);
typedef void (*sort_fn)(void *, size_t, size_t, cmp_fn, void *);

static void swap_bytes(void *a, void *b, size_t size) {
    unsigned char *p = a, *q = b;
    while (size--) {
        unsigned char t = *p;
        *p++ = *q;
        *q++ = t;
    }
}

static void copy_bytes(void *dst, const void *src, size_t size) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (size--) *d++ = *s++;
}

#define AT(base, i, size) ((unsigned char *)(base) + (size_t)(i) * (size))

static void insertion_sort(void *base, size_t n, size_t size, cmp_fn cmp, void *ctx) {
    unsigned char tmp[64];
    for (size_t i = 1; i < n; i++) {
        copy_bytes(tmp, AT(base, i, size), size);
        size_t j = i;
        while (j > 0 && cmp(AT(base, j - 1, size), tmp, ctx) > 0) {
            copy_bytes(AT(base, j, size), AT(base, j - 1, size), size);
            j--;
        }
        copy_bytes(AT(base, j, size), tmp, size);
    }
}

static void shell_sort(void *base, size_t n, size_t size, cmp_fn cmp, void *ctx) {
    static const size_t gaps[] = { 701, 301, 132, 57, 23, 10, 4, 1 };
    unsigned char tmp[64];
    for (size_t g = 0; g < sizeof gaps / sizeof *gaps; g++) {
        size_t gap = gaps[g];
        for (size_t i = gap; i < n; i++) {
            copy_bytes(tmp, AT(base, i, size), size);
            size_t j = i;
            for (; j >= gap && cmp(AT(base, j - gap, size), tmp, ctx) > 0; j -= gap)
                copy_bytes(AT(base, j, size), AT(base, j - gap, size), size);
            copy_bytes(AT(base, j, size), tmp, size);
        }
    }
}

static void sift_down(unsigned char *base, size_t start, size_t end, size_t size, cmp_fn cmp, void *ctx) {
    size_t root = start;
    while (2 * root + 1 < end) {
        size_t child = 2 * root + 1, sw = root;
        if (cmp(AT(base, sw, size), AT(base, child, size), ctx) < 0) sw = child;
        if (child + 1 < end && cmp(AT(base, sw, size), AT(base, child + 1, size), ctx) < 0) sw = child + 1;
        if (sw == root) return;
        swap_bytes(AT(base, root, size), AT(base, sw, size), size);
        root = sw;
    }
}

static void heap_sort(void *base, size_t n, size_t size, cmp_fn cmp, void *ctx) {
    if (n < 2) return;
    for (size_t s = n / 2; s-- > 0;) sift_down(base, s, n, size, cmp, ctx);
    for (size_t end = n - 1; end > 0; end--) {
        swap_bytes(AT(base, 0, size), AT(base, end, size), size);
        sift_down(base, 0, end, size, cmp, ctx);
    }
}

static unsigned char merge_buf[64 * 4096];

static void merge_rec(unsigned char *a, size_t n, size_t size, cmp_fn cmp, void *ctx, unsigned char *buf) {
    if (n < 2) return;
    size_t m = n / 2;
    merge_rec(a, m, size, cmp, ctx, buf);
    merge_rec(AT(a, m, size), n - m, size, cmp, ctx, buf);
    if (cmp(AT(a, m - 1, size), AT(a, m, size), ctx) <= 0) return;
    copy_bytes(buf, a, m * size);
    size_t i = 0, j = m, k = 0;
    while (i < m && j < n) {
        if (cmp(AT(a, j, size), AT(buf, i, size), ctx) < 0) copy_bytes(AT(a, k++, size), AT(a, j++, size), size);
        else copy_bytes(AT(a, k++, size), AT(buf, i++, size), size);
    }
    while (i < m) copy_bytes(AT(a, k++, size), AT(buf, i++, size), size);
}

static void merge_sort(void *base, size_t n, size_t size, cmp_fn cmp, void *ctx) {
    merge_rec(base, n, size, cmp, ctx, merge_buf);
}

static void quick_rec(unsigned char *a, size_t n, size_t size, cmp_fn cmp, void *ctx, int depth) {
    while (n > 12) {
        if (depth-- == 0) { heap_sort(a, n, size, cmp, ctx); return; }
        size_t mid = n / 2;
        unsigned char *lo = a, *mi = AT(a, mid, size), *hi = AT(a, n - 1, size);
        if (cmp(mi, lo, ctx) < 0) swap_bytes(mi, lo, size);
        if (cmp(hi, mi, ctx) < 0) {
            swap_bytes(hi, mi, size);
            if (cmp(mi, lo, ctx) < 0) swap_bytes(mi, lo, size);
        }
        unsigned char pivot[64];
        copy_bytes(pivot, mi, size);
        size_t i = 0, j = n - 1;
        for (;;) {
            while (cmp(AT(a, i, size), pivot, ctx) < 0) i++;
            while (cmp(pivot, AT(a, j, size), ctx) < 0) j--;
            if (i >= j) break;
            swap_bytes(AT(a, i, size), AT(a, j, size), size);
            i++;
            j--;
        }
        size_t left = j + 1;
        /* recurse into the smaller part */
        if (left < n - left) {
            quick_rec(a, left, size, cmp, ctx, depth);
            a = AT(a, left, size);
            n -= left;
        } else {
            quick_rec(AT(a, left, size), n - left, size, cmp, ctx, depth);
            n = left;
        }
    }
    insertion_sort(a, n, size, cmp, ctx);
}

static void quick_sort(void *base, size_t n, size_t size, cmp_fn cmp, void *ctx) {
    int depth = 0;
    for (size_t k = n; k; k >>= 1) depth += 2;
    quick_rec(base, n, size, cmp, ctx, depth);
}

static const struct {
    const char *name;
    sort_fn fn;
    bool stable;
} algorithms[] = {
    { "insertion", insertion_sort, true },
    { "shell", shell_sort, false },
    { "heap", heap_sort, false },
    { "merge", merge_sort, true },
    { "quick", quick_sort, false },
};
#define NALG (sizeof algorithms / sizeof algorithms[0])

/* ---- comparison callbacks ---- */

struct counter { long calls; int direction; };

static int cmp_int(const void *a, const void *b, void *ctx) {
    struct counter *c = ctx;
    c->calls++;
    int x = *(const int *)a, y = *(const int *)b;
    return c->direction * ((x > y) - (x < y));
}

static int cmp_double(const void *a, const void *b, void *ctx) {
    ((struct counter *)ctx)->calls++;
    double x = *(const double *)a, y = *(const double *)b;
    return x < y ? -1 : x > y;
}

static int str_compare(const char *s, const char *t) {
    while (*s && *s == *t) s++, t++;
    return (unsigned char)*s - (unsigned char)*t;
}

static int cmp_str(const void *a, const void *b, void *ctx) {
    ((struct counter *)ctx)->calls++;
    return str_compare(*(const char *const *)a, *(const char *const *)b);
}

struct record {
    char name[8];
    unsigned short dept;
    short age;
    int id;
    float score;
};

/* by department, then descending score; the id is the original position */
static int cmp_record(const void *a, const void *b, void *ctx) {
    const struct record *x = a, *y = b;
    ((struct counter *)ctx)->calls++;
    if (x->dept != y->dept) return x->dept < y->dept ? -1 : 1;
    if (x->score != y->score) return x->score > y->score ? -1 : 1;
    return 0;
}

/* ---- data ---- */

static uint32_t seed = 12345;
static uint32_t lcg(void) {
    seed = seed * 1103515245u + 12345u;
    return (seed >> 16) & 0x7fff;
}

#define N 1500
static int ints[N], work_ints[N];
static double dbls[N], work_dbls[N];
static struct record recs[600], work_recs[600];
static char strpool[N][8];
static const char *strs[N], *work_strs[N];

static bool sorted(const void *base, size_t n, size_t size, cmp_fn cmp, void *ctx) {
    for (size_t i = 1; i < n; i++)
        if (cmp(AT(base, i - 1, size), AT(base, i, size), ctx) > 0) return false;
    return true;
}

static uint64_t multiset_hash(const void *base, size_t n, size_t size) {
    /* a commutative hash of the elements: a sum of per-element hashes */
    uint64_t s = 0;
    for (size_t i = 0; i < n; i++) {
        uint64_t h = 1469598103934665603ull;
        const unsigned char *p = AT(base, i, size);
        for (size_t k = 0; k < size; k++) h = (h ^ p[k]) * 1099511628211ull;
        s += h;
    }
    return s;
}

/* LSD radix sort of unsigned keys derived from ints (sign bit flipped) */
static void radix_sort(int *a, size_t n) {
    static int tmp[N];
    for (int pass = 0; pass < 4; pass++) {
        size_t count[257] = { 0 };
        int shift = pass * 8;
        for (size_t i = 0; i < n; i++) count[(((uint32_t)a[i] ^ 0x80000000u) >> shift & 0xff) + 1]++;
        for (int i = 0; i < 256; i++) count[i + 1] += count[i];
        for (size_t i = 0; i < n; i++) tmp[count[((uint32_t)a[i] ^ 0x80000000u) >> shift & 0xff]++] = a[i];
        for (size_t i = 0; i < n; i++) a[i] = tmp[i];
    }
}

int main(void) {
    for (int i = 0; i < N; i++) {
        uint32_t hi = lcg();
        ints[i] = (int)(hi << 16 | lcg()) - 0x3fffffff;
        if (i % 7 == 0) ints[i] = ints[i / 2];   /* duplicates */
        dbls[i] = (double)(int)(lcg() % 2001 - 1000) / 8.0;
        for (int k = 0; k < 7; k++) strpool[i][k] = (char)('a' + lcg() % (k < 2 ? 3 : 26));
        strpool[i][lcg() % 3 + 4] = 0;
        strs[i] = strpool[i];
    }
    for (int i = 0; i < 600; i++) {
        unsigned short dept = (unsigned short)(lcg() % 5);
        short age = (short)(lcg() % 60 + 18);
        struct record r = { { 0 }, dept, age, i, (float)(lcg() % 40) * 0.25f };
        for (int k = 0; k < 5; k++) r.name[k] = (char)('A' + (i * 7 + k * 3) % 26);
        recs[i] = r;
    }

    uint64_t h = 0;
    for (size_t a = 0; a < NALG; a++) {
        struct counter c = { 0, 1 };
        int bad = 0;

        for (int i = 0; i < N; i++) work_ints[i] = ints[i];
        algorithms[a].fn(work_ints, N, sizeof(int), cmp_int, &c);
        bad += !sorted(work_ints, N, sizeof(int), cmp_int, &(struct counter){ 0, 1 });
        bad += multiset_hash(work_ints, N, sizeof(int)) != multiset_hash(ints, N, sizeof(int));
        h = h * 31 + (uint64_t)work_ints[N / 3];

        struct counter down = { 0, -1 };
        algorithms[a].fn(work_ints, N, sizeof(int), cmp_int, &down);
        bad += !sorted(work_ints, N, sizeof(int), cmp_int, &(struct counter){ 0, -1 });

        for (int i = 0; i < N; i++) work_dbls[i] = dbls[i];
        algorithms[a].fn(work_dbls, N, sizeof(double), cmp_double, &c);
        bad += !sorted(work_dbls, N, sizeof(double), cmp_double, &c);
        h = h * 31 + (uint64_t)(int64_t)(work_dbls[N - 10] * 8);

        for (int i = 0; i < N; i++) work_strs[i] = strs[i];
        algorithms[a].fn(work_strs, N, sizeof(char *), cmp_str, &c);
        bad += !sorted(work_strs, N, sizeof(char *), cmp_str, &c);
        for (const char *s = work_strs[N / 2]; *s; s++) h = h * 31 + (unsigned char)*s;

        for (int i = 0; i < 600; i++) work_recs[i] = recs[i];
        algorithms[a].fn(work_recs, 600, sizeof(struct record), cmp_record, &c);
        bad += !sorted(work_recs, 600, sizeof(struct record), cmp_record, &c);
        int unstable = 0;
        for (int i = 1; i < 600; i++)
            if (cmp_record(&work_recs[i - 1], &work_recs[i], &c) == 0 && work_recs[i - 1].id > work_recs[i].id) unstable++;
        if (algorithms[a].stable) bad += unstable != 0;
        else h = h * 31 + (unstable > 0);
        h = h * 31 + (uint64_t)work_recs[0].id + (uint64_t)work_recs[599].id;

        print_str(algorithms[a].name, 0 + (size_t)(algorithms[a].name[0] == 'i' ? 9 : algorithms[a].name[0] == 's' ? 5 : algorithms[a].name[0] == 'h' ? 4 : 5));
        print_i64(bad);
        print_bool(c.calls > N);
    }
    for (int i = 0; i < N; i++) work_ints[i] = ints[i];
    radix_sort(work_ints, N);
    struct counter c = { 0, 1 };
    print_bool(sorted(work_ints, N, sizeof(int), cmp_int, &c));
    print_i64(work_ints[0]);
    print_i64(work_ints[N - 1]);
    print_i64(work_ints[N / 2]);
    print_str(work_recs[0].name, 5);
    print_f64(work_recs[0].score);
    print_u64(h);
    /* the insertion sort's call count is exact (it depends only on the data) */
    struct counter ic = { 0, 1 };
    for (int i = 0; i < 200; i++) work_ints[i] = ints[i];
    insertion_sort(work_ints, 200, sizeof(int), cmp_int, &ic);
    print_i64(ic.calls);
    return 0;
}

/* expect: insertion */
/* expect: 0 */
/* expect: true */
/* expect: shell */
/* expect: 0 */
/* expect: true */
/* expect: heap */
/* expect: 0 */
/* expect: true */
/* expect: merge */
/* expect: 0 */
/* expect: true */
/* expect: quick */
/* expect: 0 */
/* expect: true */
/* expect: true */
/* expect: -1072280666 */
/* expect: 1072966834 */
/* expect: 21777469 */
/* expect: QTWZC */
/* expect: 9.75 */
/* expect: 7551502413506271156 */
/* expect: 10297 */
