#include "config.h"
#include <math.h>
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
int main(void) {
#if defined(HAVE_STDIO_H) && defined(HAVE_UNISTD_H) && !defined(HAVE_NO_SUCH_HEADER_H) && SIZEOF_LONG == 8
    char *s = strndup("configured", 6);
    printf("%s %.1f\n", s, sqrt(2.25));
    return 0;
#else
    return 1;
#endif
}
