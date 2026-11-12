/* wctype (C17 7.30.2.2.2): the classes' names, as in every locale; 0 for
   another name. */
#include <axc/text.h>
#include <wctype.h>

static const char *const names[12] = {"alnum", "alpha", "blank", "cntrl", "digit", "graph",
                                      "lower", "print", "punct", "space", "upper", "xdigit"};

wctype_t wctype(const char *property) {
    int i;
    for (i = 0; i < 12; i++) {
        const char *a = names[i], *b = property;
        while (*a && *a == *b) {
            a++;
            b++;
        }
        if (*a == 0 && *b == 0)
            return (wctype_t)1 << i;
    }
    return 0;
}
