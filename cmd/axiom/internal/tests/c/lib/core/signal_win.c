/* Windows x64: the processor's faults in the program's code, delivered to
   signal's handlers (std/c/rt/core_signal.ax): an access violation is
   SIGSEGV, an integer division by zero SIGFPE, an illegal instruction
   SIGILL. The handlers leave with longjmp (which unwinds from the handler
   through the faulting function: the trampoline's machine frame). With
   SIG_DFL back, the fault is the crash reporter's: its report, and the
   exception's code as the status (0xC0000094, whose low byte is what Wine
   reports). */
/* windows-only */
/* x86-64-only */
/* flags: --c-std=gnu11 */
/* expect-status: 148 */
/* expect-stderr: axiom: crash: arithmetic fault */
#include <setjmp.h>
#include <signal.h>
#include "print.h"

static jmp_buf jb;
static volatile int *volatile bad = 0;
static volatile int zero = 0;
static volatile int calls;

static void handler(int sig) {
    calls++;
    longjmp(jb, sig);
}

static int divide(int a) {
    return a / zero;
}

static void illegal(void) {
    __asm__ volatile("ud2");
}

int main(void) {
    volatile int r;
    print_bool(signal(SIGSEGV, handler) == SIG_DFL); /* expect: true */
    print_bool(signal(SIGFPE, handler) == SIG_DFL);  /* expect: true */
    print_bool(signal(SIGILL, handler) == SIG_DFL);  /* expect: true */
    r = setjmp(jb);
    if (r == 0) {
        *bad = 1;
        print_cstr("not reached");
    }
    print_i64(r); /* expect: 11 */
    r = setjmp(jb);
    if (r == 0) {
        print_i64(divide(7));
        print_cstr("not reached");
    }
    print_i64(r); /* expect: 8 */
    r = setjmp(jb);
    if (r == 0) {
        illegal();
        print_cstr("not reached");
    }
    print_i64(r); /* expect: 4 */
    /* again: the handler stays set */
    r = setjmp(jb);
    if (r == 0)
        *bad = 2;
    print_i64(r);     /* expect: 11 */
    print_i64(calls); /* expect: 4 */
    print_bool(signal(SIGFPE, SIG_DFL) == handler); /* expect: true */
    print_i64(divide(1));
    print_cstr("not reached");
    return 0;
}
