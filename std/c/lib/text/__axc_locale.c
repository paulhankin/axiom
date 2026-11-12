/* __axc_locale - the current locale's state (axc/text.h): every category
   "C" at program startup (C17 7.11.1.1p4). */
#include <axc/text.h>

static struct __axc_locale current = {{"", "C", "C", "C", "C", "C"}, 0, ""};

struct __axc_locale *__axc_locale(void) {
    return &current;
}
