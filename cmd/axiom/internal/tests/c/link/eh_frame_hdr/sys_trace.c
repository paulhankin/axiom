// The platform runtime's unwinder walks code of the system's compiler
// through its call frame information, which it finds through
// PT_GNU_EH_FRAME (.eh_frame_hdr's sorted table): the frames from sys_c up
// to sys_a, each found by the function that encloses its address.
#include <unwind.h>

void sys_a(int *);
void sys_b(int *);
void sys_c(int *);

static _Unwind_Reason_Code step(struct _Unwind_Context *ctx, void *arg) {
    int *seen = arg;
    void *f = _Unwind_FindEnclosingFunction((void *)(_Unwind_GetIP(ctx) - 1));
    if (f == (void *)sys_c && seen[0] == 0) seen[0] = 1;
    if (f == (void *)sys_b && seen[0] == 1) seen[0] = 2;
    if (f == (void *)sys_a && seen[0] == 2) seen[0] = 3;
    seen[1]++;
    return _URC_NO_REASON;
}

__attribute__((noinline)) void sys_c(int *seen) {
    _Unwind_Backtrace(step, seen);
    __asm__ volatile("" ::: "memory");
}
__attribute__((noinline)) void sys_b(int *seen) {
    sys_c(seen);
    __asm__ volatile("" ::: "memory");
}
__attribute__((noinline)) void sys_a(int *seen) {
    sys_b(seen);
    __asm__ volatile("" ::: "memory");
}
