// The walk: from the handler of a signal, which the compiler's C library
// calls, through the signal frame to the function that raised it and to
// main (main.c), each found by the function that encloses its address.
#include <unwind.h>

void sys_walk(int *);
void our_handler(int);
void our_raiser(int);
int main(void);

static _Unwind_Reason_Code step(struct _Unwind_Context *ctx, void *arg) {
    int *seen = arg;
    int before = 0;
    // a signal frame's address is the interrupted instruction's, a call's
    // the instruction after the call
    _Unwind_Ptr ip = _Unwind_GetIPInfo(ctx, &before);
    void *f = _Unwind_FindEnclosingFunction((void *)(before ? ip : ip - 1));
    void *want[4] = {(void *)sys_walk, (void *)our_handler, (void *)our_raiser, (void *)main};
    if (seen[0] < 4 && f == want[seen[0]]) seen[0]++;
    seen[1]++;
    return _URC_NO_REASON;
}

__attribute__((noinline)) void sys_walk(int *seen) {
    _Unwind_Backtrace(step, seen);
    __asm__ volatile("" ::: "memory");
}
