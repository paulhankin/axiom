// The platform's objects with personality routines and language-specific
// data (sys_cleanup.c: C's cleanup attribute with -fexceptions, whose call
// frame information names them in __eh_frame, a CIE's pointer to a GOT
// slot; sys_catch.cpp: C++, whose compact entries name them): the
// executable's __unwind_info lists the routines' GOT slots and the LSDA
// index, its __eh_frame points at the slots (macho.mjs checks them).
// targets: x86_64-macos aarch64-macos
// sys-flags: -fexceptions
// flags: -lc++
#include <stdio.h>

int sys_cleanup(int *);
int sys_catch(int *);

void sys_inner(int *s) { s[1] += 5; }

int main(void) {
    int s[2] = {0, 0};
    int a = sys_cleanup(s);
    int b = sys_catch(s);
    printf("%d %d %d\n", a, b, s[1]);
    return 0;
}
