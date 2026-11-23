/* __axc_mb_utf8 - whether LC_CTYPE's multibyte encoding is UTF-8
   (axc/locale.h). */
#include <axc/text.h>

int __axc_mb_utf8(void) {
    return __axc_locale()->utf8;
}
