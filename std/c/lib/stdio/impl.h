/* impl.h - the internals of the C library's <stdio.h> (std/c/lib/stdio/):
   the members of FILE, the operating-system layer (std/c/rt/stdio.ax), and
   the functions the library's units share, all under reserved names
   (__axc_). Not a header of the implementation's: only the library's units
   include it.

   A stream (struct __axc_file, <stdio.h>) is in one of three states: idle,
   reading (F_RD: __buf holds bytes read from the file, __rpos..__rend those
   not yet taken) or writing (F_WR: __buf..__wpos holds bytes not yet
   written). The fast paths of getc and putc are a comparison of two
   pointers; everything else goes through __axc_refill and __axc_flush.
   Switching between reading and writing, which 7.21.5.3p7 lets a program
   do only after fflush or a seek, is done whenever it happens: the read
   buffer is given back to the file by a seek, the write buffer flushed.

   Text streams on Windows translate line ends (7.21.2p2): '\n' is written
   as "\r\n", and "\r\n" is read as '\n'. A read buffer keeps the file's
   bytes; __rend stops before each '\r' (__dend is the end of the bytes),
   so that the fast path never sees one and positions stay the file's. */
#ifndef __AXC_STDIO_IMPL_H
#define __AXC_STDIO_IMPL_H

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <wchar.h>
#include <stdint.h>

/* __flags */
#define F_READ 0x1u       /* opened for reading */
#define F_WRITE 0x2u      /* opened for writing */
#define F_APPEND 0x4u     /* every write at the end of the file */
#define F_EOF 0x8u        /* the end-of-file indicator */
#define F_ERR 0x10u       /* the error indicator */
#define F_LBF 0x20u       /* line buffered */
#define F_NBF 0x40u       /* unbuffered */
#define F_OWNBUF 0x80u    /* __buf was allocated by the library */
#define F_TEXT 0x100u     /* translates line ends (a text stream on Windows) */
#define F_STATIC 0x200u   /* stdin, stdout or stderr: not allocated */
#define F_RD 0x400u       /* reading: the buffer holds bytes of the file */
#define F_WR 0x800u       /* writing: the buffer holds bytes for the file */
#define F_UNSET 0x1000u   /* the buffering is decided at the first use (stdin, stdout) */
#define F_BACK 0x2000u    /* __rpos..__rend are pushed back bytes (ungetc) */
#define F_NOSEEK 0x4000u  /* the file cannot be positioned (a pipe or terminal) */
#define F_CLOSED 0x8000u  /* a stream that fclose closed (freopen's failure) */
#define F_SETBUF 0x10000u /* setvbuf was called: the first use keeps its buffering */

/* __orient (7.21.2p4) */
#define O_NONE 0
#define O_BYTE (-1)
#define O_WIDE 1

/* The pushed back bytes a stream holds: __back's size */
#define BACK_MAX 8

/* The operating-system layer: std/c/rt/stdio.ax. Negative results are the
   host ABI's codes (__axc_errno_of turns them into errno values). */
/* (integers, sizes included, cross as 64-bit ones: there is a 64-bit
   type with every --c-int and --c-long, not always a 32-bit one, and
   size_t follows int when that is as wide as a pointer) */
int64_t __axc_sys_open(const char *path, int64_t flags);
int64_t __axc_sys_read(int64_t fd, unsigned char *p, uint64_t n);
uint64_t __axc_sys_write(int64_t fd, const unsigned char *p, uint64_t n);
int64_t __axc_sys_seek(int64_t fd, int64_t off, int64_t whence);
void __axc_sys_close(int64_t fd);
int64_t __axc_sys_remove(const char *path);
int64_t __axc_sys_rename(const char *from, const char *to);
int64_t __axc_sys_isatty(int64_t fd);
int64_t __axc_sys_os(void);
int64_t __axc_sys_random(unsigned char *p, uint64_t n);
int64_t __axc_sys_tmpdir(char *buf, uint64_t cap);
void __axc_sys_lock(unsigned __axiom_int64 *word);
void __axc_sys_unlock(unsigned __axiom_int64 *word);
void *__axc_sys_alloc(uint64_t n);
void __axc_sys_free(void *p, uint64_t n);
void __axc_sys_exit(int64_t status);
void __axc_sys_copy(void *dst, const void *src, uint64_t n);

/* The first c in p[0..n), or NULL (a word at a time) */
const unsigned char *__axc_find_byte(const unsigned char *p, int c, size_t n);

/* errno (one place, so that errno's storage can change) */
#define SET_ERRNO(e) (errno = (e))
int __axc_errno_of(int code);

/* A stream's lock (7.21.2p8: each stream has one; the functions act as if
   they took it). The internal functions named _u expect it held. While
   the process has one thread (no thread was ever started) it is not
   taken: nothing can race, and the thread cannot start one in the middle
   of an operation. So UNLOCK releases the lock only if it is held. */
int64_t __axc_sys_threaded(void);
#define LOCK(f) (__axc_sys_threaded() ? __axc_sys_lock(&(f)->__lock) : (void)0)
#define UNLOCK(f) ((f)->__lock != 0 ? __axc_sys_unlock(&(f)->__lock) : (void)0)

/* The list of open streams (for fflush(NULL) and exit), and its lock */
extern FILE *__axc_files;
extern unsigned __axiom_int64 __axc_files_lock;

/* file.c */
int __axc_refill(FILE *f);
int __axc_flush_u(FILE *f);
int __axc_to_read(FILE *f);
int __axc_to_write(FILE *f);
int __axc_uflow(FILE *f);
int __axc_oflow(FILE *f, int c);
size_t __axc_write_u(FILE *f, const unsigned char *p, size_t n);
size_t __axc_read_u(FILE *f, unsigned char *p, size_t n);
void __axc_setup(FILE *f);
int __axc_orient(FILE *f, int o);
int __axc_parse_mode(const char *mode, unsigned int *flags, unsigned int *open_flags);
int __axc_seek_u(FILE *f, __axiom_int64 off, int whence);
__axiom_int64 __axc_tell_u(FILE *f);
void __axc_stdio_exit(void);
/* The exit path's slot for __axc_stdio_exit (the core part's exit calls
   it if set): set at a stream's first output and by tmpfile, so that a
   program without them has no flushing at exit linked in. */
extern void (*__axc_at_exit_flush)(void);
int __axc_close_u(FILE *f, int free_it);
void __axc_link(FILE *f);
void __axc_unlink(FILE *f);
FILE *__axc_new_file(void);
void __axc_free_file(FILE *f);
int __axc_is_windows(void);

/* getc and putc without the lock: the fast paths */
#define GETC_U(f) ((f)->__rpos < (f)->__rend ? (int)*(f)->__rpos++ : __axc_uflow(f))
#define PUTC_U(c, f) \
    ((f)->__wpos < (f)->__wend && ((c) != '\n' || !((f)->__flags & F_LBF)) \
        ? (int)(*(f)->__wpos++ = (unsigned char)(c)) : __axc_oflow((f), (c)))

/* Formatted output (printf.c): an output that takes runs of characters */
struct __axc_out {
    void (*put)(struct __axc_out *o, const void *p, size_t n);
    size_t count;      /* characters produced, whether or not they fit */
    int failed;        /* an output error (a stream's) */
    FILE *f;           /* a stream's */
    unsigned char *s;  /* a string's: the next character */
    size_t room;       /* a string's: characters that fit, not counting the null */
};

int __axc_vprintf_core(struct __axc_out *o, const char *fmt, va_list ap);
int __axc_vwprintf_core(struct __axc_out *o, const wchar_t *fmt, va_list ap);
int __axc_vfprintf_u(FILE *f, const char *fmt, va_list ap);

/* Floating-point formatting (fmtfloat.c): the conversion of a value of a
   format (binary64, x87 80-bit, binary128) given by its bits */
struct __axc_fp {
    unsigned __axiom_int64 hi, lo; /* the significand, with its leading bit where it is stored or implied */
    int exp;                       /* value = significand * 2^exp (when finite) */
    int neg;
    int kind; /* 0 zero, 1 finite, 2 infinity, 3 NaN */
    int mant_bits; /* the precision (53, 64, 113) */
    int min_exp;   /* the exponent of the smallest normal's leading bit (-1022, -16382) */
    int dec;       /* a decimal floating value (C23): value = significand * 10^exp, exp its quantum exponent */
};
void __axc_fp_double(struct __axc_fp *v, double x);
void __axc_fp_long_double(struct __axc_fp *v, long double x);
/* a decimal floating value's (_Decimal32, _Decimal64, _Decimal128: bytes
   4, 8 or 16 of BID at p) */
void __axc_fp_dec(struct __axc_fp *v, const void *p, int bytes);
/* Writes the conversion (spec 'f','e','g','a' and upper case) of v with the
   flags into buf (cap bytes), returning the length; *big, if the result
   needs more than cap, is a buffer of the library's (to be freed with
   __axc_sys_free(*big, *bigcap)). */
size_t __axc_fmt_fp(const struct __axc_fp *v, int spec, int prec, int alt, char *buf, size_t cap, char **big,
                    size_t *bigcap);

/* Formatted input (scanf.c): a source of characters with one pushed back */
struct __axc_in {
    int (*get)(struct __axc_in *i);
    void (*unget)(struct __axc_in *i, int c);
    size_t count;  /* characters read (for %n) */
    FILE *f;
    const unsigned char *s; /* a string's */
    const wchar_t *ws;      /* a wide string's */
    int eof;                /* the source has ended */
    int error;              /* a read error (a stream's) */
    int c23;                /* C23's rules (the call's: the header maps C23 mode's names) */
};
int __axc_vscanf_core(struct __axc_in *in, const char *fmt, va_list ap);
int __axc_vwscanf_core(struct __axc_in *in, const wchar_t *fmt, va_list ap);

/* Floating-point input (scanfloat.c): what the scanner read, 0.d[0]d[1]...
   d[nd-1] x 10^dexp (decimal) or mantissa x 2^bexp (hexadecimal) */
struct __axc_num {
    int kind; /* 0 decimal, 1 hexadecimal, 2 infinity, 3 NaN */
    int neg;
    char *d;  /* the significant digits' values (cap of them) */
    int nd, cap;
    int sticky; /* a dropped digit was not 0 (or a dropped bit 1) */
    long dexp;
    unsigned __axiom_int64 mhi, mlo; /* hexadecimal: up to 120 bits */
    long bexp;
    int nohex; /* set by the caller: no hexadecimal form (the decimal floating types') */
};
int __axc_scan_num(struct __axc_in *in, long width, struct __axc_num *n, size_t *taken, size_t *valid);
int __axc_num_digits(int ld);
long double __axc_strtold(const char *s, char **end, int *range);
double __axc_num_double(struct __axc_num *n, int *range);
float __axc_num_float(struct __axc_num *n, int *range);
long double __axc_num_long_double(struct __axc_num *n, int *range);
/* the value of n in decimal floating format dfmt (1 _Decimal32, 2
   _Decimal64, 3 _Decimal128), at dst (4, 8 or 16 bytes of BID); *range
   when it overflows or underflows */
void __axc_num_dec(struct __axc_num *n, int dfmt, void *dst, int *range);
/* strtod32, strtod64 and strtod128's work: as __axc_strtold for format dfmt */
void __axc_strtodec(const char *s, char **end, int dfmt, void *dst, int *range);

/* Pushing back (chario.c, wide.c), the lock held */
int __axc_ungetc_u(int c, FILE *f);
wint_t __axc_ungetwc_u(wint_t c, FILE *f);

/* Wide streams (wide.c) */
wint_t __axc_getwc_u(FILE *f);
wint_t __axc_putwc_u(wchar_t c, FILE *f);

#endif
