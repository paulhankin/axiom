/* flags: --c-std=c11 */
/* _Thread_local (C11): objects with thread storage duration, at file scope
   and in blocks with static or extern: the main thread's instances (each
   thread has its own: c/lib/threads/atomic_threads.c). <threads.h> is
   the C library's: __STDC_NO_THREADS__ is not defined. */
#include "host.h"

_Thread_local int counter = 5;
static _Thread_local int zeroed;
extern _Thread_local int later;

static int bump(void) {
    static _Thread_local int calls;
    return ++calls;
}

int main(void) {
    extern _Thread_local int counter;
    counter++;
    bump();
    bump();
    print_i64(counter);
    print_i64(zeroed);
    print_i64(bump());
    print_i64(later);
#ifdef __STDC_NO_THREADS__
    print_i64(1);
#else
    print_i64(0);
#endif
    return 0;
}

_Thread_local int later = 9;
/* expect: 6 */
/* expect: 0 */
/* expect: 3 */
/* expect: 9 */
/* expect: 0 */
