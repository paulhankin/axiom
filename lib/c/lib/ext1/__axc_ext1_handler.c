/* __axc_ext1_handler - the runtime-constraint handler of Annex K (C11
   K.3.6.1.1, K.3.1.4): the one set_constraint_handler_s registered, or the
   default, abort_handler_s (the implementation's choice, K.3.6.1.1p3), and
   the call of it after a violation. The handler is one for the program,
   read and replaced atomically (any thread may register one). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <stdatomic.h>

static _Atomic(constraint_handler_t) handler;

constraint_handler_t __axc_ext1_get_handler(void) {
    constraint_handler_t h = atomic_load(&handler);
    return h != NULL ? h : abort_handler_s;
}

constraint_handler_t __axc_ext1_set_handler(constraint_handler_t h) {
    constraint_handler_t old = atomic_exchange(&handler, h);
    return old != NULL ? old : abort_handler_s;
}

void __axc_ext1_violation(const char *fn, const char *what, errno_t error) {
    /* the message: "fn: what" (cut to the buffer) */
    char msg[160];
    size_t n = 0;
    while (*fn != 0 && n < sizeof msg - 3)
        msg[n++] = *fn++;
    msg[n++] = ':';
    msg[n++] = ' ';
    while (*what != 0 && n < sizeof msg - 1)
        msg[n++] = *what++;
    msg[n] = 0;
    __axc_ext1_get_handler()(msg, NULL, error);
}
