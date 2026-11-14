// The FDEs of this compiler's code in sections of its own: functions with
// --function-sections, the link-once helper of 16-byte atomics (a COMDAT
// group in each object: the link keeps one), with --gc-sections (an unused
// function's section left out, and its FDE with it). The platform's
// unwinder (sys_walk.c) walks through them up to main; cc_main.c, linked
// by the system's toolchain with the same objects, does too.
// flags: --c-lib=system --gc-sections
// ours-flags: --function-sections
// expect: 1 1 4
// expect: 1 1 4
// cc-flags: -Wl,--gc-sections
// cc-expect: 1 1 4
// cc-expect: 1 1 4
int printf(const char *, ...);
int ours_a(int *);
int ours_b(int *);

int main(void) {
    int seen[3] = {0, 0, 0};
    int r = ours_a(seen);
    printf("%d %d %d\n", r, seen[0], seen[1]);
    int seen2[3] = {0, 0, 0};
    r = ours_b(seen2);
    printf("%d %d %d\n", r, seen2[0], seen2[1]);
    return 0;
}
