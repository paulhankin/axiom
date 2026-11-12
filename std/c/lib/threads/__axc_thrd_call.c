/* __axc_thrd_call: calls a thread's start function (std/c/rt/
   threads_spawn.ax, which cannot call a function that returns int: its
   width is the build's), and gives its result as long long. */
#include "axc/threads_impl.h"

long long __axc_thrd_call(thrd_start_t func, void *arg) {
    return func(arg);
}
