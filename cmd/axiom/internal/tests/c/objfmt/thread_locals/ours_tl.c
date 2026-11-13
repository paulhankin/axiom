// This compiler's object: an initialized thread-local, a zero one.
_Thread_local long ours_tl = 35;
static _Thread_local long ours_calls;

long ours_next(void) {
    ours_calls++;
    return ours_tl + ours_calls;
}
