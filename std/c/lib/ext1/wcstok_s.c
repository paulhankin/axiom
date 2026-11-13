/* wcstok_s (C11 K.3.9.2.3.1): wcstok's tokens, with the state in *ptr and
   the wide characters left to search in *s1max, which each call decreases by
   those it examined (the separators before the token, the token, and the
   separator after it, which becomes the null wide character). Runtime-
   constraints: s1max, s2 and ptr not null, *ptr not null when s1 is,
   *s1max at most RSIZE_MAX, and the end of the token (its separator or
   the string's null) within the *s1max wide characters searched; after a
   violation the result is a null pointer, and neither *s1max nor *ptr has
   changed. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <wchar.h>

wchar_t *wcstok_s(wchar_t *__AXC_RESTRICT s1, rsize_t *__AXC_RESTRICT s1max, const wchar_t *__AXC_RESTRICT s2,
               wchar_t **__AXC_RESTRICT ptr) {
    const char *what = NULL;
    wchar_t *p, *tok;
    size_t left = 0;
    if (s1max == NULL)
        what = "s1max is a null pointer";
    else if (s2 == NULL)
        what = "s2 is a null pointer";
    else if (ptr == NULL)
        what = "ptr is a null pointer";
    else if (s1 == NULL && *ptr == NULL)
        what = "s1 and *ptr are null pointers";
    else if ((left = *s1max) > RSIZE_MAX)
        what = "*s1max is greater than RSIZE_MAX";
    if (what == NULL) {
        p = s1 != NULL ? s1 : *ptr;
        /* the separators before the token */
        while (left > 0 && *p != 0 && wcschr(s2, *p) != NULL) {
            p++;
            left--;
        }
        if (left == 0) {
            what = "no end of the token within *s1max wide characters";
        } else if (*p == 0) {
            *ptr = p;
            *s1max = left;
            return NULL;
        } else {
            tok = p;
            while (left > 0 && *p != 0 && wcschr(s2, *p) == NULL) {
                p++;
                left--;
            }
            if (left == 0) {
                what = "no end of the token within *s1max wide characters";
            } else {
                if (*p != 0) {
                    *p++ = 0;
                    left--;
                }
                *ptr = p;
                *s1max = left;
                return tok;
            }
        }
    }
    __axc_ext1_violation("wcstok_s", what, EINVAL);
    return NULL;
}
