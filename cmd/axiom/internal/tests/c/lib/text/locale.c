/* setlocale and localeconv (C17 7.11): the locales "C", "POSIX", "C.UTF-8"
   and the names with a UTF-8 codeset; a query; a composite name for
   LC_ALL when the categories differ, which setlocale takes back; a name
   that is not a locale changes nothing; localeconv's "C" values; and
   MB_CUR_MAX in each encoding. */
#include <limits.h>
#include <locale.h>
#include "../../mix/print.h"
#include <stdlib.h>
#include <string.h>

static void show(const char *s) {
    print_cstr(s ? s : "(null)");
}

int main(void) {
    struct lconv *lc = localeconv();
    char saved[200];
    const char *p;
    int i;
    show(setlocale(LC_ALL, 0));
    print_i64((long long)MB_CUR_MAX);
    show(setlocale(LC_CTYPE, "C.UTF-8"));
    print_i64((long long)MB_CUR_MAX);
    show(setlocale(LC_ALL, 0));
    /* (the composite name back) */
    p = setlocale(LC_ALL, 0);
    for (i = 0; p[i]; i++)
        saved[i] = p[i];
    saved[i] = 0;
    show(setlocale(LC_ALL, "POSIX"));
    print_i64((long long)MB_CUR_MAX);
    show(setlocale(LC_ALL, saved));
    show(setlocale(LC_CTYPE, 0));
    show(setlocale(LC_TIME, "en_US.utf8"));
    show(setlocale(LC_NUMERIC, "de_DE.UTF-8@euro"));
    show(setlocale(LC_ALL, 0));
    /* not locales: nothing changes */
    show(setlocale(LC_ALL, "de_DE"));
    show(setlocale(LC_ALL, "C.ISO-8859-1"));
    show(setlocale(LC_ALL, "LC_CTYPE=C;LC_TIME=C"));
    show(setlocale(LC_ALL, "has space.UTF-8"));
    show(setlocale(99, "C"));
    show(setlocale(-1, 0));
    show(setlocale(LC_ALL, 0));
    show(setlocale(LC_ALL, "C"));
    print_i64((long long)MB_CUR_MAX);
    show(setlocale(LC_MONETARY, "C.UTF-8"));
    print_i64((long long)MB_CUR_MAX);
    /* "" is the environment's locale: one setlocale takes back */
    p = setlocale(LC_ALL, "");
    print_bool(!p || setlocale(LC_ALL, p) != 0);
    setlocale(LC_ALL, "C");
    show(lc->decimal_point);
    print_i64(*lc->thousands_sep + *lc->grouping + *lc->mon_decimal_point + *lc->mon_thousands_sep +
              *lc->mon_grouping + *lc->positive_sign + *lc->negative_sign + *lc->currency_symbol +
              *lc->int_curr_symbol);
    print_bool(lc->frac_digits == CHAR_MAX && lc->p_cs_precedes == CHAR_MAX && lc->n_cs_precedes == CHAR_MAX &&
               lc->p_sep_by_space == CHAR_MAX && lc->n_sep_by_space == CHAR_MAX && lc->p_sign_posn == CHAR_MAX &&
               lc->n_sign_posn == CHAR_MAX && lc->int_frac_digits == CHAR_MAX &&
               lc->int_p_cs_precedes == CHAR_MAX && lc->int_n_cs_precedes == CHAR_MAX &&
               lc->int_p_sep_by_space == CHAR_MAX && lc->int_n_sep_by_space == CHAR_MAX &&
               lc->int_p_sign_posn == CHAR_MAX && lc->int_n_sign_posn == CHAR_MAX);
    print_bool(localeconv() == lc);
    return 0;
    /* expect: C */
    /* expect: 1 */
    /* expect: C.UTF-8 */
    /* expect: 4 */
    /* expect: LC_COLLATE=C;LC_CTYPE=C.UTF-8;LC_MONETARY=C;LC_NUMERIC=C;LC_TIME=C */
    /* expect: POSIX */
    /* expect: 1 */
    /* expect: LC_COLLATE=C;LC_CTYPE=C.UTF-8;LC_MONETARY=C;LC_NUMERIC=C;LC_TIME=C */
    /* expect: C.UTF-8 */
    /* expect: en_US.utf8 */
    /* expect: de_DE.UTF-8@euro */
    /* expect: LC_COLLATE=C;LC_CTYPE=C.UTF-8;LC_MONETARY=C;LC_NUMERIC=de_DE.UTF-8@euro;LC_TIME=en_US.utf8 */
    /* expect: (null) */
    /* expect: (null) */
    /* expect: (null) */
    /* expect: (null) */
    /* expect: (null) */
    /* expect: (null) */
    /* expect: LC_COLLATE=C;LC_CTYPE=C.UTF-8;LC_MONETARY=C;LC_NUMERIC=de_DE.UTF-8@euro;LC_TIME=en_US.utf8 */
    /* expect: C */
    /* expect: 1 */
    /* expect: C.UTF-8 */
    /* expect: 1 */
    /* expect: true */
    /* expect: . */
    /* expect: 0 */
    /* expect: true */
    /* expect: true */
}
