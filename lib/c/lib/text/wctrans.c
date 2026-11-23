/* wctrans (C17 7.30.3.2.2): "tolower" is 1, "toupper" 2, another name 0. */
#include <wctype.h>

wctrans_t wctrans(const char *property) {
    static const char lower[] = "tolower", upper[] = "toupper";
    int i;
    for (i = 0; i < 8 && property[i] == lower[i]; i++)
        if (lower[i] == 0)
            return 1;
    for (i = 0; i < 8 && property[i] == upper[i]; i++)
        if (upper[i] == 0)
            return 2;
    return 0;
}
