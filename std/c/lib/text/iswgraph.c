/* iswgraph (C17 7.30.2.1.6), by the locale's classes (__axc_wclass). */
#include <axc/text.h>
#include <wctype.h>

int iswgraph(wint_t wc) {
    return (__axc_wclass((unsigned long)wc) & __AXC_W_GRAPH) != 0;
}
