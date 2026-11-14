// The platform runtime's unwinder walks from here through this compiler's
// frames: the functions a and b (b may be 0) in turn above sys_c, then main.
#include <unwind.h>

int main(void);
static void *want[4];

static _Unwind_Reason_Code step(struct _Unwind_Context *ctx, void *arg) {
    int *seen = arg;
    void *f = _Unwind_FindEnclosingFunction((void *)(_Unwind_GetIP(ctx) - 1));
    if (seen[1] < 4 && want[seen[1]] == 0) seen[1]++;
    if (seen[1] < 4 && f == want[seen[1]]) seen[1]++;
    seen[0] = seen[1] == 4;
    return _URC_NO_REASON;
}

__attribute__((noinline)) void sys_c(int *seen, void *a, void *b) {
    want[0] = (void *)sys_c;
    want[1] = a;
    want[2] = b;
    want[3] = (void *)main;
    _Unwind_Backtrace(step, seen);
    __asm__ volatile("" ::: "memory");
}
