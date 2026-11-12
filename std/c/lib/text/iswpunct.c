/* iswpunct (C17 7.30.2.1.9), by the locale's classes (__axc_wclass). */
#include <axc/text.h>
#include <wctype.h>

int iswpunct(wint_t wc) {
    return (__axc_wclass((unsigned long)wc) & __AXC_W_PUNCT) != 0;
}
