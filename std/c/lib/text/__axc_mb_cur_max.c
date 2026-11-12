/* __axc_mb_cur_max - MB_CUR_MAX (C17 7.22p3): 4 for UTF-8, 1 for the
   single-byte encoding (axc/locale.h). */
#include <axc/text.h>

int __axc_mb_cur_max(void) {
    return __axc_locale()->utf8 ? 4 : 1;
}
