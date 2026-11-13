/* bsearch_s (C11 K.3.6.3.1): bsearch with a context, which compar is
   given as its third argument. Runtime-constraints: nmemb and size at
   most RSIZE_MAX, and when nmemb is not 0, key, base and compar not null;
   after a violation the result is a null pointer (no search). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>

void *bsearch_s(const void *key, const void *base, rsize_t nmemb, rsize_t size,
                int (*compar)(const void *k, const void *y, void *context), void *context) {
    const char *lo = base, *what = NULL;
    if (nmemb > RSIZE_MAX)
        what = "nmemb is greater than RSIZE_MAX";
    else if (size > RSIZE_MAX)
        what = "size is greater than RSIZE_MAX";
    else if (nmemb > 0 && key == NULL)
        what = "key is a null pointer";
    else if (nmemb > 0 && base == NULL)
        what = "base is a null pointer";
    else if (nmemb > 0 && compar == NULL)
        what = "compar is a null pointer";
    if (what != NULL) {
        __axc_ext1_violation("bsearch_s", what, EINVAL);
        return NULL;
    }
    while (nmemb > 0) {
        const char *mid = lo + (nmemb / 2) * size;
        int c = compar(key, mid, context);
        if (c == 0)
            return (void *)mid;
        if (c > 0) {
            lo = mid + size;
            nmemb -= nmemb / 2 + 1;
        } else {
            nmemb /= 2;
        }
    }
    return NULL;
}
