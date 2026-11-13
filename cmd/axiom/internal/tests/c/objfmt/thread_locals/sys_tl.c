// The platform's compiler's thread-local, and its code reading this
// compiler's (ours_tl.c).
_Thread_local int sys_tl = 7;
extern _Thread_local long ours_tl;

long sys_sum(void) { return sys_tl + ours_tl; }
