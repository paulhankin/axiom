// The platform runtime's unwinder (sys_walk.c) walks out of a signal
// handler through the signal frame of the compiler's C library, whose
// return trampoline (ax_sigreturn on x86-64, the kernel's on AArch64) the
// unwinder recognizes by its instructions, to the interrupted code (the
// system call of raise) and on through this compiler's frames, one with a
// variable length array, up to main. The program is static: the unwinder
// finds the tables through the C library's _dl_find_object.
// cc-variants: -O2 | -O0 -fasynchronous-unwind-tables
// expect: 1 6 1
#include <signal.h>
#include <stdio.h>

void sys_walk(int *);
void sys_addrs(void **);

static int walk[4];

void our_handler(int sig) {
    if (sig == SIGINT) sys_walk(walk);
    walk[2]++; // (not a sibling call: the handler's frame is there)
}

void our_raiser(int n) {
    volatile char vla[n];
    vla[0] = 1;
    vla[n - 1] = 2;
    raise(SIGINT);
    walk[3] += vla[0] + vla[n - 1];
}

int main(void) {
    signal(SIGINT, our_handler);
    void (*volatile f)(int) = our_raiser;
    f(9);
    printf("%d %d %d\n", walk[0] == 4, walk[1] >= 6 ? 6 : walk[1], walk[3] == 3);
    return 0;
}
