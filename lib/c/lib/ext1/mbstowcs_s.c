/* mbstowcs_s (C11 K.3.6.5.1): the multibyte string src as wide characters
   into dst, an array of dstmax (or only counted, dst null), from the
   initial conversion state: their number into *retval. Runtime-
   constraints: retval and src not null, and those of __axc_ext1_mbs;
   after a violation *retval is (size_t)-1 (retval not null) and dst[0]
   the null wide character (dst not null, dstmax 1 to RSIZE_MAX /
   sizeof(wchar_t)). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <wchar.h>
#include <string.h>


errno_t mbstowcs_s(size_t *__AXC_RESTRICT retval, wchar_t *__AXC_RESTRICT dst, rsize_t dstmax,
                   const char *__AXC_RESTRICT src, rsize_t len) {
    mbstate_t st;
    const char *p = src;
    if (retval == NULL || src == NULL) {
        if (retval != NULL)
            *retval = (size_t)-1;
        if (dst != NULL && dstmax > 0 && dstmax <= RSIZE_MAX / sizeof(wchar_t))
            dst[0] = 0;
        __axc_ext1_violation("mbstowcs_s", retval == NULL ? "retval is a null pointer" : "src is a null pointer",
                             EINVAL);
        return EINVAL;
    }
    memset(&st, 0, sizeof st);
    return __axc_ext1_mbs("mbstowcs_s", retval, dst, dstmax, &p, len, &st);
}
