/* iswctype (C17 7.30.2.2.1): desc is a class from wctype, its bit; 0 (no
   class) is in no character. */
#include <axc/text.h>
#include <wctype.h>

int iswctype(wint_t wc, wctype_t desc) {
    return (__axc_wclass((unsigned long)wc) & desc) != 0;
}
