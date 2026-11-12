/* bsearch (<stdlib.h>, C23 7.24.5.1): an element of the sorted array base
   (nmemb elements of size bytes) that compar finds equal to *key, or a null
   pointer; by halving, at most about log2(nmemb) + 1 comparisons. */
#include <stdlib.h>

void *(bsearch)(const void *key, const void *base, size_t nmemb, size_t size,
                int (*compar)(const void *, const void *)) {
    const char *lo = base;
    while (nmemb > 0) {
        const char *mid = lo + (nmemb / 2) * size;
        int c = compar(key, mid);
        if (c == 0)
            return (void *)mid;
        if (c > 0) {
            lo = mid + size;
            nmemb -= nmemb / 2 + 1;
        } else {
            nmemb /= 2;
        }
    }
    return (void *)0;
}
