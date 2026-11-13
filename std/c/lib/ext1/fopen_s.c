/* fopen_s (C11 K.3.5.2.1): fopen into *streamptr (a null pointer when the
   file cannot be opened: the result is then errno's value), with Annex
   K's modes: a mode may begin with u; a file opened for writing (w, a, or
   +) is opened for exclusive access where the system has it (Windows: no
   other handle may open it meanwhile); a file it creates gets permissions
   that keep other users out (owner-only on Linux and macOS, an access
   control list of its owner's on Windows) unless the mode begins with u
   (the system's default permissions). Runtime-constraints: streamptr,
   filename and mode not null; after a violation *streamptr is a null
   pointer (streamptr not null) and nothing is opened. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <stdio.h>

FILE *__axc_fopen_ext1(const char *filename, const char *mode);

errno_t fopen_s(FILE *__AXC_RESTRICT *__AXC_RESTRICT streamptr, const char *__AXC_RESTRICT filename,
                const char *__AXC_RESTRICT mode) {
    const char *what = streamptr == NULL ? "streamptr is a null pointer"
                       : filename == NULL ? "filename is a null pointer"
                       : mode == NULL ? "mode is a null pointer"
                                      : NULL;
    if (what != NULL) {
        if (streamptr != NULL)
            *streamptr = NULL;
        __axc_ext1_violation("fopen_s", what, EINVAL);
        return EINVAL;
    }
    errno = 0;
    *streamptr = __axc_fopen_ext1(filename, mode);
    if (*streamptr == NULL)
        return errno != 0 ? errno : EIO;
    return 0;
}
