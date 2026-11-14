// The same objects of this compiler's linked by the system's toolchain
// (-Wl,--gc-sections): its linker keeps their FDEs with their code and makes
// .eh_frame_hdr of them; the walk as sys_walk.c's.
#include <unwind.h>

int main(void);
int printf(const char *, ...);
int ours_a(int *);
int ours_b(int *);
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

int main(void) {
    int seen[3] = {0, 0, 0};
    int r = ours_a(seen);
    printf("%d %d %d\n", r, seen[0], seen[1]);
    int seen2[3] = {0, 0, 0};
    r = ours_b(seen2);
    printf("%d %d %d\n", r, seen2[0], seen2[1]);
    return 0;
}
