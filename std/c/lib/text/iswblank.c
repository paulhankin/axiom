/* iswblank (C17 7.30.2.1.3), by the locale's classes (__axc_wclass). */
#include <axc/text.h>
#include <wctype.h>

int iswblank(wint_t wc) {
    return (__axc_wclass((unsigned long)wc) & __AXC_W_BLANK) != 0;
}
