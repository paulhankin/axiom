/* strdup (<string.h>, C23 7.26.2.6): a copy of s in memory from malloc (for
   free), or a null pointer (errno ENOMEM) if there is none. */
#include <stdlib.h>
#include "axc/string_hosted.h"

char *strdup(const char *s) {
    size_t n = 0;
    char *p;
    while (s[n])
        n++;
    p = malloc(n + 1);
    if (p) {
        size_t i;
        for (i = 0; i <= n; i++)
            p[i] = s[i];
    }
    return p;
}
