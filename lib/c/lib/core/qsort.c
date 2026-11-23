/* qsort (<stdlib.h>, C23 7.24.5.2): sorts nmemb elements of size bytes at
   base into the order compar gives (qsort_impl.h: an introspective sort,
   O(n log n) comparisons at worst; a comparison function that is not
   consistent gives an unspecified order but no access outside the array).
   Not stable. */
#include <stdlib.h>

typedef int (*qs_cmp_fn)(const void *, const void *);
#define QS_NAME sort
#define QS_CMP(s, a, b) ((s)->cmp((a), (b)))
#define QS_CTX
#define QS_CTX_VALUE NULL
#include "qsort_impl.h"

void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
    sort(base, nmemb, size, compar);
}
