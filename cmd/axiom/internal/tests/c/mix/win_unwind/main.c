/* Windows: the system's stack walk through a function with a live variable
   length array (whose stack pointer has moved: its unwind information finds
   its frame from rbp), and C's use of objects of a DLL (ntdll.dll's
   NlsAnsiCodePage, read through the import address table). */
/* windows-only */
/* uses: ../print.ax */
#include "../print.h"

/* ntdll.dll */
unsigned short RtlCaptureStackBackTrace(unsigned long skip, unsigned long n, void **frames, unsigned long *hash);
extern unsigned short NlsAnsiCodePage;
unsigned short *code_page(void) { return &NlsAnsiCodePage; }

static int walk(void) {
    void *frames[32];
    return RtlCaptureStackBackTrace(0, 32, frames, 0);
}

/* (not a sibling call at -O2, which would leave this frame out) */
static int plain(int n) {
    volatile int r = walk();
    return r + n * 0;
}

static int with_vla(int n) {
    volatile int a[n];
    for (int i = 0; i < n; i++)
        a[i] = i;
    int r = walk();
    volatile int b[n * 3];
    b[0] = a[n - 1];
    return r + walk() - walk() + b[0] - (n - 1);
}

/* (through pointers: neither is inlined into main at -O2) */
static int (*volatile plain_p)(int) = plain;
static int (*volatile vla_p)(int) = with_vla;

int main(void) {
    int p = plain_p(5), v = vla_p(100);
    /* the same depth: the walk goes through the VLA's frame as through the other */
    print_bool(p == v); /* expect: true */
    print_bool(p >= 4); /* expect: true */
    unsigned m = NlsAnsiCodePage;
    print_bool(*code_page() == m && m != 0); /* expect: true */
    return 0;
}
