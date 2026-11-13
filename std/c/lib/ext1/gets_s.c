/* gets_s (C11 K.3.5.4.1): reads a line of the standard input into s, an
   array of n: at most n - 1 characters, the new-line (read) not stored, a
   null character after them; s, or a null pointer at the end of the
   input before any character, or after a read error (then s[0] is the
   null character). Runtime-constraints: s not null, n not 0 and at most
   RSIZE_MAX, and a new-line, the end of the input or a read error within
   n - 1 characters; after a violation s[0] is the null character (s not
   null, n 1 to RSIZE_MAX), and the rest of the line is read and
   discarded. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <stdio.h>

char *gets_s(char *s, rsize_t n) {
    const char *what = NULL;
    size_t k = 0;
    int c = 0;
    if (s == NULL)
        what = "s is a null pointer";
    else if (n == 0)
        what = "n is zero";
    else if (n > RSIZE_MAX)
        what = "n is greater than RSIZE_MAX";
    if (what == NULL) {
        for (;;) {
            c = getchar();
            if (c == EOF || c == '\n')
                break;
            if (k == n - 1) {
                what = "the line is longer than n - 1 characters";
                break;
            }
            s[k++] = (char)c;
        }
        if (what == NULL) {
            if (c == EOF && (ferror(stdin) || k == 0)) {
                s[0] = 0;
                return NULL;
            }
            s[k] = 0;
            return s;
        }
        /* (the rest of the line is read and discarded) */
        while (c != EOF && c != '\n')
            c = getchar();
    }
    if (s != NULL && n > 0 && n <= RSIZE_MAX)
        s[0] = 0;
    __axc_ext1_violation("gets_s", what, s == NULL || n == 0 ? EINVAL : ERANGE);
    return NULL;
}
