// The platform runtime's unwinder walks code of the system's compiler
// through its call frame information, which it finds through
// PT_GNU_EH_FRAME (.eh_frame_hdr's sorted table): the frames from sys_c up
// to sys_a, each found by the function that encloses its address. Called
// from this compiler's code (main.c: our_top, ours_walk.c: our_mid,
// oursso_walk.c: so_mid, so_outer, so_inner), the walk goes through their frames
// up to main.
#include <unwind.h>

void sys_a(int *);
void sys_b(int *);
void sys_c(int *);
void sys_mid(int *);
void our_top(int *);
void our_mid(int *, int);
void so_addrs(void **);
int main(void);

static _Unwind_Reason_Code step(struct _Unwind_Context *ctx, void *arg) {
    int *seen = arg;
    void *f = _Unwind_FindEnclosingFunction((void *)(_Unwind_GetIP(ctx) - 1));
    if (f == (void *)sys_c && seen[0] == 0) seen[0] = 1;
    if (f == (void *)sys_b && seen[0] == 1) seen[0] = 2;
    if (f == (void *)sys_a && seen[0] == 2) seen[0] = 3;
    seen[1]++;
    return _URC_NO_REASON;
}

// The walk through this compiler's frames: each function in turn (0: the
// library's so_mid, whose address in the program is another)
static _Unwind_Reason_Code walk(struct _Unwind_Context *ctx, void *arg) {
    int *seen = arg;
    void *so[2];
    so_addrs(so);
    void *want[8] = {(void *)sys_c, so[0], so[1], 0, (void *)our_mid, (void *)sys_mid, (void *)our_top, (void *)main};
    void *f = _Unwind_FindEnclosingFunction((void *)(_Unwind_GetIP(ctx) - 1));
    if (seen[0] < 8 && (f == want[seen[0]] || (want[seen[0]] == 0 && f != 0))) seen[0]++;
    seen[1]++;
    return _URC_NO_REASON;
}

__attribute__((noinline)) void sys_c(int *seen) {
    if (seen[2] == 40) {
        _Unwind_Backtrace(walk, seen);
    } else {
        _Unwind_Backtrace(step, seen);
    }
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
__attribute__((noinline)) void sys_mid(int *seen) {
    our_mid(seen, seen[2]);
    __asm__ volatile("" ::: "memory");
}
