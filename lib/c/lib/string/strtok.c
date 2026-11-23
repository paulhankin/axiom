/* strtok (C23 7.26.5.9): the next token of the string s1 (or, if s1 is a
   null pointer, of the one a previous call was working through), the
   tokens being separated by bytes of s2; a null pointer when there is none.
   The token's end is overwritten with a null byte. */
#include <string.h>

static char *next;

char *strtok(char *restrict s1, const char *restrict s2) {
    if (s1 == NULL) {
        s1 = next;
        if (s1 == NULL) {
            return NULL;
        }
    }
    s1 += strspn(s1, s2);
    if (*s1 == 0) {
        next = NULL;
        return NULL;
    }
    char *end = s1 + strcspn(s1, s2);
    if (*end) {
        *end = 0;
        next = end + 1;
    } else {
        next = NULL;
    }
    return s1;
}
