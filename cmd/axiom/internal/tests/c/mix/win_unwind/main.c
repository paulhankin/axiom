/* Windows: the system's stack walk through a function with a live variable
   length array (whose stack pointer has moved: its unwind information finds
   its frame from rbp), and C's use of objects of a DLL (msvcrt.dll's
   _fmode, read through the import address table). */
/* windows-only */
/* uses: ../print.ax */
#include "../print.h"

/* ntdll.dll */
unsigned short RtlCaptureStackBackTrace(unsigned long skip, unsigned long n, void **frames, unsigned long *hash);
extern int _fmode;

static int walk(void) {
    void *frames[32];
    return RtlCaptureStackBackTrace(0, 32, frames, 0);
}

static int plain(int n) { return walk() + n * 0; }

static int with_vla(int n) {
    volatile int a[n];
    for (int i = 0; i < n; i++)
        a[i] = i;
    int r = walk();
    volatile int b[n * 3];
    b[0] = a[n - 1];
    return r + walk() - walk() + b[0] - (n - 1);
}

int main(void) {
    int p = plain(5), v = with_vla(100);
    /* the same depth: the walk goes through the VLA's frame as through the other */
    print_bool(p == v); /* expect: true */
    print_bool(p >= 4); /* expect: true */
    int m = _fmode;
    _fmode = m;
    print_bool(_fmode == m); /* expect: true */
    return 0;
}
