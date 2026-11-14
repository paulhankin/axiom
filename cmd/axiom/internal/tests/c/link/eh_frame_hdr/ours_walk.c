// A frame of this compiler's at -O0 (ours-flags) with a variable length
// array between the system's frames (main.c).
void so_mid(int *, int, void (*)(int *));
void sys_c(int *);

void our_mid(int *seen, int n) {
    volatile char vla[n];
    vla[0] = 1;
    vla[n - 1] = 1;
    so_mid(seen, n, sys_c);
    seen[3] += vla[0] + vla[n - 1];
}
