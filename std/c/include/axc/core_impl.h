/* axc/core_impl.h - the internal names of the C library's core part
   (std/c/lib/core, std/c/rt/core*.ax) that its sources share, and those
   the other parts use (the exit-time hook of the standard I/O). Each name
   X is defined by std/c/lib/core/X.c, or by an Axiom module of std/c/rt. */
#ifndef __AXIOM_AXC_CORE_IMPL_H
#define __AXIOM_AXC_CORE_IMPL_H

#define __AXIOM_WANT_SIZE_T
#define __AXIOM_WANT_WCHAR_T
#include <axc/types.h>

/* -- for the other parts -------------------------------------------------- */

/* Called by exit after the atexit handlers, when not null: the standard
   I/O part sets it (when it first opens or writes to a stream) to its
   function that flushes and closes every stream and removes the tmpfile
   files (7.24.4.4p4). quick_exit and _Exit do not call it. */
extern void (*__axc_at_exit_flush)(void);
/* exit's slot for the destructors (__axc_at_exit_dtors.c) */
extern void (*__axc_at_exit_dtors)(void);

/* The functions of std/c/rt (Axiom) take and return no int, whose width
   the build chooses (--c-int): long long is i64 there, size_t usize. */

/* Ends the process with the status at once, through the Axiom runtime
   (std/c/rt/core.ax: std/os.exit). */
__AXC_NORETURN void __axc_exit(long long status);
/* The start of exit: in a program with threads, a thread other than the
   first to call exit waits here for the end (std/c/rt/core.ax). */
void __axc_exit_begin(void);

/* Writes n bytes to the process's standard error, unbuffered (the
   messages of assert and abort; std/c/rt/core.ax). */
void __axc_write_err(const char *p, size_t n);

/* -- the core part's own ------------------------------------------------ */

/* The character classes of each value of unsigned char, and of EOF at
   index 0 (__axc_ctype_tab.c). */
#define __AXC_UPPER 0x01
#define __AXC_LOWER 0x02
#define __AXC_DIGIT 0x04
#define __AXC_SPACE 0x08 /* isspace */
#define __AXC_PUNCT 0x10
#define __AXC_CNTRL 0x20
#define __AXC_XDIGIT 0x40
#define __AXC_BLANK 0x80
#define __AXC_PRINT 0x100 /* graphic characters and space */
extern const unsigned short __axc_ctype_tab[257];
#define __AXC_CLASS(c) (__axc_ctype_tab[(unsigned)(c) + 1u <= 256u ? (unsigned)(c) + 1u : 0u])

/* A table of functions to call at exit (atexit) or quick_exit: 32 in the
   object, then blocks of 32 more from malloc (__axc_atexit_tab.c,
   __axc_quick_tab.c). */
struct __axc_fn_block {
    struct __axc_fn_block *prev;
    void (*fns[32])(void);
};
struct __axc_fn_table {
    struct __axc_fn_block first;
    struct __axc_fn_block *top; /* null: first */
    int n;                      /* functions in top */
    _Atomic int lock;           /* 1 while a thread changes the table */
};
/* Takes the table's lock (a spin: it is held for a few instructions, as
   atexit and at_quick_exit may be called from any thread); `t->lock = 0`
   (an atomic store) gives it back (__axc_fn_lock.c). */
void __axc_fn_lock(struct __axc_fn_table *t);
extern struct __axc_fn_table __axc_atexit_tab;
extern struct __axc_fn_table __axc_quick_tab;
int __axc_fn_push(struct __axc_fn_table *t, void (*f)(void));
void __axc_fn_run(struct __axc_fn_table *t);

/* The integer conversions' common part (__axc_strtoint.c): the value of the
   subject sequence of strtol and its kin (7.24.1.7) at s in base base,
   negated modulo 2^64 if it has a minus sign (*neg then 1), and where it
   ends in *end (s if there is none); *over is 1 if the magnitude is more
   than max. c23 admits the 0b prefixes. */
unsigned long long __axc_strtoint(const char *s, char **end, int base, unsigned long long max, int *neg,
                                  int *over, int c23);

/* The floating conversions' common part (__axc_strtofp.c): the value of the
   subject sequence of strtod (7.24.1.5) at s as binary64 (bits 64) or
   binary32 (bits 32) bits, correctly rounded; *end as for strtod; *range 1
   if it overflowed or underflowed. */
unsigned long long __axc_strtofp(const char *s, char **end, int bits, int *range);

/* The Axiom float parser (std/c/rt/core.ax: std/fmt.parse_float), for the
   n bytes at p: digits, at most one '.', an exponent part (all valid). */
double __axc_parse_f64(const char *p, size_t n);
float __axc_parse_f32(const char *p, size_t n);

/* strfromd and its kin (__axc_strfrom.c): the value formatted as snprintf
   does, for a format of strfromd's form (-1 for another). */
int __axc_strfrom(char *__AXC_RESTRICT s, size_t n, const char *__AXC_RESTRICT format, long double fp,
                  int long_double);

/* Sets errno (__axc_set_errno.c: for std/c/rt, which cannot know int's
   width). */
void __axc_set_errno(long long e);

/* system's command processor (std/c/rt/core.ax): with a null string, 1 if
   there is one; else the command's status, or -1. */
long long __axc_system(const char *string);

/* The state of rand (__axc_rand_state.c). */
extern unsigned long long __axc_rand_state;

/* signal's handlers, by the library's signal numbers (1..31), and what the
   system layer calls with a signal's number (__axc_sig_tab.c,
   __axc_sig_deliver.c, std/c/rt/core_signal.ax). */
extern void (*__axc_sig_tab[32])(int);
void __axc_sig_deliver(long long sig);
/* Sets the system's action for the library's signal sig: 0 default, 1
   ignore, 2 deliver (to __axc_sig_deliver); -1 if the system has no such
   signal (Windows, WebAssembly: only raise delivers), else 0. */
long long __axc_os_signal(long long sig, long long action);
/* Sends sig to the calling thread through the system: 0, or -1 where the
   system has no such signal (then raise delivers it itself). */
long long __axc_os_raise(long long sig);

/* strtold's scanner, the formatted input's (std/c/lib/stdio/scanfloat.c) */
long double __axc_strtold(const char *s, char **end, int *range);

#endif
