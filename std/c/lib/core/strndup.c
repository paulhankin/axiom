/* strndup (<string.h>, C23 7.26.2.7): a copy of at most n bytes of s, the
   terminating null added, in memory from malloc (for free), or a null
   pointer (errno ENOMEM) if there is none. s need not have a null within
   its first n bytes. */
#include <stdlib.h>
#include "axc/string_hosted.h"

char *strndup(const char *s, size_t n) {
    size_t len = 0;
    char *p;
    while (len < n && s[len])
        len++;
    p = malloc(len + 1);
    if (p) {
        size_t i;
        for (i = 0; i < len; i++)
            p[i] = s[i];
        p[len] = 0;
    }
    return p;
}
