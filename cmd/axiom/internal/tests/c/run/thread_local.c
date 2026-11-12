/* flags: --c-std=c11 */
/* _Thread_local (C11): objects with thread storage duration, at file scope
   and in blocks with static or extern: the main thread's instances (each
   thread has its own: atomic_threads.c). There is no <threads.h> yet
   (__STDC_NO_THREADS__). */
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
    print_i64(__STDC_NO_THREADS__);
    return 0;
}

_Thread_local int later = 9;
/* expect: 6 */
/* expect: 0 */
/* expect: 3 */
/* expect: 9 */
/* expect: 1 */
