/* mbsinit (C17 7.29.6.2.1): whether *ps is an initial conversion state
   (no character begun, no units still to give, no surrogate kept). */
#include <wchar.h>

int mbsinit(const mbstate_t *ps) {
    return !ps || (ps->__axiom_need == 0 && ps->__axiom_npend == 0 && ps->__axiom_high == 0);
}
