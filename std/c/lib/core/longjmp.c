/* longjmp (<setjmp.h>, C23 7.13.2.1): returns from the setjmp call that
   saved env again, with val (1 if val is 0: 7.13.2.1p4). The registers are
   restored by the linkers' stub (cmd/axiom/src/jstub.ax), on Windows x64
   after the system has unwound the frames in between (std/c/rt/core_jump.ax).
   A freestanding program has the stub's longjmp, which restores them at
   once. */
#include <setjmp.h>
#include "axc/core_impl.h"

void longjmp(jmp_buf env, int val) {
    __axc_longjmp(env, val == 0 ? 1 : val);
}
