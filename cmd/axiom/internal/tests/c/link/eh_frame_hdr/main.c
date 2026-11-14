// The executable's .eh_frame_hdr and PT_GNU_EH_FRAME (sys_trace.c).
// flags: --c-lib=system
// cc-variants: -O2 | -O0 -fasynchronous-unwind-tables
// expect: 3 1
int printf(const char *, ...);
void sys_a(int *);

int main(void) {
    int seen[2] = {0, 0};
    sys_a(seen);
    printf("%d %d\n", seen[0], seen[1] >= 3);
    return 0;
}
