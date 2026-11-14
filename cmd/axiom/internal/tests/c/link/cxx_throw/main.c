// Exceptions of C++ code (the system's compiler's, with exceptions:
// cxx-exceptions) thrown through this compiler's C functions and caught
// above them: our_top (at the suite's optimization level) and our_mid
// (ours_mid.c, at -O0), each with a variable length array, between the
// C++ frames, whose destructors run as the exception passes. The C++
// objects' language-specific data (a read-only section) and personality
// routine (a COMDAT data reference to it) are kept with their functions
// under --gc-sections; the C++ runtime library is a shared library built
// against the system's C library, so the program's is the system's too.
// needs: c++
// cxx-exceptions
// flags: --c-lib=system
// ours-flags: -O0
// cc-variants: -O0 | -O2 -ffunction-sections -fdata-sections
// flag-variants: | --gc-sections
// expect: 42 2 0 0
// expect: 7 2 0 0
// expect: -1 0 99 17
#include <stdio.h>

int cxx_top(int *, int, int);
void cxx_mid(int *, int, int);
void our_mid(int *, int, int);

void our_top(int *s, int n, int kind) {
    volatile char vla[n];
    vla[0] = 2;
    vla[n - 1] = 3;
    cxx_mid(s, n + vla[0], kind);
    s[2] = 99; // (not reached when it throws)
    s[3] += vla[0] + vla[n - 1];
}

int main(void) {
    for (int kind = 0; kind < 3; kind++) {
        int s[4] = {0, 0, 0, 0};
        int r = cxx_top(s, 5, kind);
        printf("%d %d %d %d\n", r, s[1], s[2], s[3]);
    }
    return 0;
}
