/* towctrans (C17 7.30.3.2.1): a mapping from wctrans; 0 maps nothing. */
#include <wctype.h>

wint_t towctrans(wint_t wc, wctrans_t desc) {
    if (desc == 1)
        return towlower(wc);
    if (desc == 2)
        return towupper(wc);
    return wc;
}
