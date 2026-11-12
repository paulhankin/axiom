/* signal.h - signal handling (C89 4.7, C99 7.14, C11 7.14, C23 7.14), for
   the Axiom compiler's C library (std/c/lib/core/signal.c, raise.c). The
   numbers are the library's own, the same on every target (the library
   translates them for the system).

   A handler stays installed while it runs and its signal waits until it
   returns (7.14.1.1p3: the second choice); on Linux and macOS the system
   delivers the signals, and raise sends them. On Windows, SIGINT is
   console's Ctrl-C (and Ctrl-Break), raise calls a handler at once, and
   SIGSEGV, SIGILL and SIGFPE from the processor are the crash reporter's;
   on WebAssembly raise calls a handler at once and nothing else arrives.
   Where no system signal kills the process, the default action ends it
   with status 3 (SIGABRT: as abort does) or 128 + the signal's number. */
#ifndef __AXIOM_SIGNAL_H
#define __AXIOM_SIGNAL_H

typedef int sig_atomic_t;

#define SIG_DFL ((void (*)(int))0)
#define SIG_IGN ((void (*)(int))1)
#define SIG_ERR ((void (*)(int))-1)

#define SIGINT 2
#define SIGILL 4
#define SIGABRT 6
#define SIGFPE 8
#define SIGSEGV 11
#define SIGTERM 15

void (*signal(int sig, void (*func)(int)))(int);
int raise(int sig);

#endif
