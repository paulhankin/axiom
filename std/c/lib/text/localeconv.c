/* localeconv (C17 7.11.2.1): the "C" locale's conventions, which every
   locale has (7.11.2.1p7: "." as the decimal point, CHAR_MAX for the
   values that are not available, empty strings for the others). */
#include <limits.h>
#include <locale.h>

static char empty[] = "";
static char point[] = ".";

static struct lconv conventions = {
    point, empty, empty, empty, empty, empty, empty, empty, empty,
    CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
    empty, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX};

struct lconv *localeconv(void) {
    /* (the program may have changed the object: 7.11.2.1p8 forbids it) */
    return &conventions;
}
