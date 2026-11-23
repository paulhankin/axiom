/* wcsrtombs_s (C11 K.3.9.3.2.2): wcsrtombs with the bounds of dst (an
   array of dstmax), the count into *retval. Runtime-constraints: retval,
   src, *src and ps not null, and those of __axc_ext1_wcs; after a
   violation *retval is (size_t)-1 (retval not null) and dst[0] the null
   character (dst not null, dstmax 1 to RSIZE_MAX). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <wchar.h>


errno_t wcsrtombs_s(size_t *__AXC_RESTRICT retval, char *__AXC_RESTRICT dst, rsize_t dstmax,
                    const wchar_t **__AXC_RESTRICT src, rsize_t len, mbstate_t *__AXC_RESTRICT ps) {
    const char *what = retval == NULL ? "retval is a null pointer"
                       : src == NULL  ? "src is a null pointer"
                       : *src == NULL ? "*src is a null pointer"
                       : ps == NULL   ? "ps is a null pointer"
                                      : NULL;
    if (what != NULL) {
        if (retval != NULL)
            *retval = (size_t)-1;
        if (dst != NULL && dstmax > 0 && dstmax <= RSIZE_MAX)
            dst[0] = 0;
        __axc_ext1_violation("wcsrtombs_s", what, EINVAL);
        return EINVAL;
    }
    return __axc_ext1_wcs("wcsrtombs_s", retval, dst, dstmax, src, len, ps);
}
