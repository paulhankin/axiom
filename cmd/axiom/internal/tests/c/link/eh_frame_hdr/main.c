// The executable's and a shared library's .eh_frame_hdr and
// PT_GNU_EH_FRAME, and the FDEs of this compiler's functions in them: the
// platform runtime's unwinder (sys_trace.c) walks from code of the
// system's compiler through this compiler's frames (these at the suite's
// optimization level, ours_walk.c's and oursso_walk.c's at -O0, one with a
// variable length array, whose frame the CFA follows from the frame
// pointer), the system's in between, up to main. With the system's C
// library and with the compiler's (whose dl_iterate_phdr and
// _dl_find_object the unwinder then uses, and whose program leaves the
// unwinder's weak references to the system library's threads null).
// flag-variants: --c-lib=system | --c-lib=axiom
// ours-flags: -O0
// cc-variants: -O2 | -O0 -fasynchronous-unwind-tables
// expect: 3 1
// expect: 8 1 42
int printf(const char *, ...);
void sys_a(int *);
void sys_mid(int *);

void our_top(int *seen) {
    sys_mid(seen);
    seen[3]++;
}

int main(void) {
    int seen[4] = {0, 0, 0, 0};
    sys_a(seen);
    printf("%d %d\n", seen[0], seen[1] >= 3);
    // the walk through this compiler's frames (sys_trace.c: sys_walk)
    int walk[4] = {0, 0, 40, 0};
    void (*volatile f)(int *) = our_top;
    f(walk);
    printf("%d %d %d\n", walk[0], walk[1] >= 8, walk[3]);
    return 0;
}
