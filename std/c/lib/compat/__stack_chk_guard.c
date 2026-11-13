/* __stack_chk_guard (the system interface of Linux programs): the guard of stack protection.
   An object compiled against the system C library's headers refers to it
   (docs/c.md, "Objects and libraries of other compilers"): defined here so
   that the program's C library is this one (--c-lib=axiom). The interface
   is the Linux Standard Base's (Core specification, the generic and the
   architecture parts); nothing of the system library's own layout is
   assumed beyond what that specification states. */
#include <stdint.h>

/* the guard the stack protection compares on AArch64 (on x86-64 it is
   in the thread's control block: the thread pointer's offset 0x28, which
   the runtime keeps as it found it); a terminator value (a zero byte,
   CR, LF, 0xff), which a string overflow cannot write unchanged */
uintptr_t __stack_chk_guard = (uintptr_t)0x00ff0a0d00ff0a0dull;
