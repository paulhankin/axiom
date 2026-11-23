/* freopen_s (C11 K.3.5.2.2): freopen of stream, with fopen_s's modes (u,
   exclusive access, private permissions), the result into *newstreamptr
   (a null pointer when the file cannot be opened: the result is then
   errno's value). Runtime-constraints: newstreamptr, mode and stream not
   null; after a violation *newstreamptr is a null pointer
   (newstreamptr not null) and the stream is not closed. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <stdio.h>

FILE *__axc_freopen_ext1(const char *filename, const char *mode, FILE *stream);

errno_t freopen_s(FILE *__AXC_RESTRICT *__AXC_RESTRICT newstreamptr, const char *__AXC_RESTRICT filename,
                  const char *__AXC_RESTRICT mode, FILE *__AXC_RESTRICT stream) {
    const char *what = newstreamptr == NULL ? "newstreamptr is a null pointer"
                       : mode == NULL ? "mode is a null pointer"
                       : stream == NULL ? "stream is a null pointer"
                                        : NULL;
    if (what != NULL) {
        if (newstreamptr != NULL)
            *newstreamptr = NULL;
        __axc_ext1_violation("freopen_s", what, EINVAL);
        return EINVAL;
    }
    errno = 0;
    *newstreamptr = __axc_freopen_ext1(filename, mode, stream);
    if (*newstreamptr == NULL)
        return errno != 0 ? errno : EIO;
    return 0;
}
