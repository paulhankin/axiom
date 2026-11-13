/* qsort_impl.h - the sort of qsort (qsort.c) and Annex K's qsort_s
   (std/c/lib/ext1/qsort_s.c), one text for both: the includer declares
   the type qs_cmp_fn (compar's) and defines QS_NAME (the static function's
   name), QS_CMP(s, a, b) (a comparison), QS_CTX (the context parameter, or
   nothing) and QS_CTX_VALUE (context, or NULL).

   It sorts nmemb elements of size bytes at base into the order compar
   gives. Introspective sort (D. R. Musser,
   "Introspective Sorting and Selection Algorithms", Software: Practice and
   Experience 27(8), 1997): quicksort, partitioning about the median of three
   elements (of three medians of three past 128 elements: Bentley and
   McIlroy, "Engineering a Sort Function", 1993) with Sedgewick's two-sided
   scan, which stops at elements equal to the pivot so that many equal keys
   still split evenly; a part longer than 16 elements that has been
   partitioned 2 log2(nmemb) times without becoming short is heapsorted, so
   no input takes more than O(n log n) comparisons; short parts are sorted
   by insertion. The longer part of each partition waits on an explicit
   stack while the shorter is sorted: at most log2(nmemb) entries. Not
   stable. Elements are moved a word at a time when their size and address
   allow it. */
#include <stddef.h>

/* how elements are swapped: by words, by 32-bit units, or by bytes */
struct sorter {
    size_t size;
    int mode; /* 2: words, 1: 32-bit units, 0: bytes */
    qs_cmp_fn cmp;
    void *ctx; /* (qsort_s's context) */
};

static void swap(const struct sorter *s, char *a, char *b) {
    size_t n = s->size;
    if (a == b)
        return;
    if (s->mode == 2) {
        unsigned long long *x = (unsigned long long *)(void *)a, *y = (unsigned long long *)(void *)b;
        for (n /= sizeof *x; n > 0; n--, x++, y++) {
            unsigned long long t = *x;
            *x = *y;
            *y = t;
        }
    } else if (s->mode == 1) {
        unsigned int *x = (unsigned int *)(void *)a, *y = (unsigned int *)(void *)b;
        for (n /= sizeof *x; n > 0; n--, x++, y++) {
            unsigned int t = *x;
            *x = *y;
            *y = t;
        }
    } else {
        for (; n > 0; n--, a++, b++) {
            char t = *a;
            *a = *b;
            *b = t;
        }
    }
}

static char *median3(const struct sorter *s, char *a, char *b, char *c) {
    if (QS_CMP(s, a, b) < 0) {
        if (QS_CMP(s, b, c) < 0)
            return b;
        return QS_CMP(s, a, c) < 0 ? c : a;
    }
    if (QS_CMP(s, a, c) < 0)
        return a;
    return QS_CMP(s, b, c) < 0 ? c : b;
}

static void insertion(const struct sorter *s, char *a, size_t n) {
    size_t i;
    for (i = 1; i < n; i++) {
        char *p = a + i * s->size;
        while (p > a && QS_CMP(s, p - s->size, p) > 0) {
            swap(s, p - s->size, p);
            p -= s->size;
        }
    }
}

/* the heap of a[0..n) sifted down from element i (the children of i are
   2i + 1 and 2i + 2) */
static void sift(const struct sorter *s, char *a, size_t i, size_t n) {
    for (;;) {
        size_t c = 2 * i + 1;
        if (c >= n)
            return;
        if (c + 1 < n && QS_CMP(s, a + c * s->size, a + (c + 1) * s->size) < 0)
            c++;
        if (QS_CMP(s, a + i * s->size, a + c * s->size) >= 0)
            return;
        swap(s, a + i * s->size, a + c * s->size);
        i = c;
    }
}

static void heapsort(const struct sorter *s, char *a, size_t n) {
    size_t i = n / 2;
    while (i > 0)
        sift(s, a, --i, n);
    while (n > 1) {
        n--;
        swap(s, a, a + n * s->size);
        sift(s, a, 0, n);
    }
}

/* Partitions a[0..n) (n > 16) about a pivot it moves to a[0]: returns j,
   with a[0..j) <= a[j] <= a[j+1..n). */
static size_t partition(const struct sorter *s, char *a, size_t n) {
    size_t z = s->size;
    char *mid = a + (n / 2) * z, *last = a + (n - 1) * z;
    char *pv;
    size_t i = 0, j = n;
    if (n > 128) {
        size_t k = n / 8;
        char *x = median3(s, a, a + k * z, a + 2 * k * z);
        char *y = median3(s, mid - k * z, mid, mid + k * z);
        char *w = median3(s, last - 2 * k * z, last - k * z, last);
        pv = median3(s, x, y, w);
    } else {
        pv = median3(s, a, mid, last);
    }
    swap(s, a, pv);
    for (;;) {
        do
            i++;
        while (i < n && QS_CMP(s, a + i * z, a) < 0);
        /* (j > 0: a comparison function that is not consistent, which is
           undefined, still keeps every access inside the array) */
        do
            j--;
        while (j > 0 && QS_CMP(s, a + j * z, a) > 0);
        if (i >= j)
            break;
        swap(s, a + i * z, a + j * z);
    }
    swap(s, a, a + j * z);
    return j;
}

static void QS_NAME(void *base, size_t nmemb, size_t size, qs_cmp_fn compar QS_CTX) {
    struct sorter s;
    struct {
        char *a;
        size_t n;
        int depth;
    } stack[64];
    int top = 0;
    char *a = base;
    size_t n = nmemb;
    int depth = 0;
    size_t m;
    if (nmemb < 2 || size == 0)
        return;
    s.size = size;
    s.cmp = compar;
    s.ctx = QS_CTX_VALUE;
    s.mode = 0;
    if (size % sizeof(unsigned long long) == 0 && (size_t)base % sizeof(unsigned long long) == 0)
        s.mode = 2;
    else if (size % sizeof(unsigned int) == 0 && (size_t)base % sizeof(unsigned int) == 0)
        s.mode = 1;
    for (m = nmemb; m > 1; m >>= 1)
        depth += 2;
    for (;;) {
        while (n > 16) {
            size_t j, rn;
            if (depth == 0) {
                heapsort(&s, a, n);
                n = 0;
                break;
            }
            depth--;
            j = partition(&s, a, n);
            rn = n - j - 1;
            /* (the longer part waits; the shorter, at most half, goes on) */
            if (j < rn) {
                stack[top].a = a + (j + 1) * size;
                stack[top].n = rn;
                n = j;
            } else {
                stack[top].a = a;
                stack[top].n = j;
                a += (j + 1) * size;
                n = rn;
            }
            stack[top].depth = depth;
            top++;
        }
        insertion(&s, a, n);
        if (top == 0)
            return;
        top--;
        a = stack[top].a;
        n = stack[top].n;
        depth = stack[top].depth;
    }
}
