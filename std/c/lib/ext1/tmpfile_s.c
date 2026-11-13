/* tmpfile_s (C11 K.3.5.1.1): tmpfile's file (exclusive, private to its
   owner, removed when closed: std/c/lib/stdio/open.c) into *streamptr, a
   null pointer if none could be created (the result then nonzero).
   Runtime-constraint: streamptr not null; after a violation no file is
   created. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <stdio.h>

errno_t tmpfile_s(FILE *__AXC_RESTRICT *__AXC_RESTRICT streamptr) {
    if (streamptr == NULL) {
        __axc_ext1_violation("tmpfile_s", "streamptr is a null pointer", EINVAL);
        return EINVAL;
    }
    *streamptr = tmpfile();
    if (*streamptr == NULL)
        return errno != 0 ? errno : EIO;
    return 0;
}
