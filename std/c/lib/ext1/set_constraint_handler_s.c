/* set_constraint_handler_s (C11 K.3.6.1.1): registers handler as the
   runtime-constraint handler (a null pointer: the default, abort_handler_s)
   and returns the one registered before (abort_handler_s if none was). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>

constraint_handler_t set_constraint_handler_s(constraint_handler_t handler) {
    return __axc_ext1_set_handler(handler);
}
