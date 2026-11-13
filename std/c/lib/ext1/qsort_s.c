/* qsort_s (C11 K.3.6.3.2): qsort with a context, which compar is given as
   its third argument: the same introspective sort (std/c/lib/core/
   qsort_impl.h). Runtime-constraints: nmemb and size at most RSIZE_MAX,
   and when nmemb is not 0, base and compar not null; after a violation
   nothing is sorted and the result is nonzero. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>

typedef int (*qs_cmp_fn)(const void *, const void *, void *);
#define QS_NAME qsort_ctx
#define QS_CMP(s, a, b) ((s)->cmp((a), (b), (s)->ctx))
#define QS_CTX , void *context
#define QS_CTX_VALUE context
#include "../core/qsort_impl.h"

errno_t qsort_s(void *base, rsize_t nmemb, rsize_t size, int (*compar)(const void *x, const void *y, void *context),
                void *context) {
    const char *what = NULL;
    errno_t e = EINVAL;
    if (nmemb > RSIZE_MAX)
        what = "nmemb is greater than RSIZE_MAX", e = ERANGE;
    else if (size > RSIZE_MAX)
        what = "size is greater than RSIZE_MAX", e = ERANGE;
    else if (nmemb > 0 && base == NULL)
        what = "base is a null pointer";
    else if (nmemb > 0 && compar == NULL)
        what = "compar is a null pointer";
    if (what != NULL) {
        __axc_ext1_violation("qsort_s", what, e);
        return e;
    }
    qsort_ctx(base, nmemb, size, compar, context);
    return 0;
}
