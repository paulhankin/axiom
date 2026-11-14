// A frame of this compiler's at -O0 (ours-flags) with a variable length
// array, between C++ frames (main.c): the exception passes through it.
void cxx_throw(int, int);

void our_mid(int *s, int n, int kind) {
    volatile char vla[n];
    vla[0] = 1;
    vla[n - 1] = 4;
    cxx_throw(kind, 40 + vla[0] + vla[0]);
    s[2] = 99; // (not reached when it throws)
    s[3] += vla[0] + vla[n - 1] + n;
}
