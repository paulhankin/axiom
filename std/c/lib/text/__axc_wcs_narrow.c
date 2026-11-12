/* __axc_wcs_narrow - the narrow string that the wide numeric conversions
   (wcstol, wcstod, ...) give to the narrow ones (strtol, strtod, ...), so
   that both read numbers by one implementation (axc/text.h): the leading
   white space (iswspace) is skipped, its length put in *lead, and the run
   of characters that may be in a number's subject sequence (C17
   7.22.1.3-4: ASCII letters, digits, '.', '+', '-', '(', ')' and '_') is
   copied, one byte for each wide character, into buf (cap bytes) or, if it
   is longer, into allocated memory (__axc_wcs_narrow_done frees it). A
   null pointer if that memory was not available. */
#include <axc/text.h>
#include <wctype.h>

static int in_number(wchar_t c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '.' || c == '+' ||
           c == '-' || c == '(' || c == ')' || c == '_';
}

char *__axc_wcs_narrow(const wchar_t *s, char *buf, size_t cap, size_t *lead) {
    size_t i = 0, n = 0, k;
    char *p = buf;
    while (iswspace((wint_t)s[i]))
        i++;
    *lead = i;
    s += i;
    while (in_number(s[n]))
        n++;
    if (n >= cap) {
        p = malloc(n + 1);
        if (!p)
            return 0;
    }
    for (k = 0; k < n; k++)
        p[k] = (char)s[k];
    p[n] = 0;
    return p;
}
